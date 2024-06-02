#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#include <types.h>
#include <http.h>
#include "common.h"
#include "random.h"
#include "uri.h"

static bool is_whitespace(char c)
{
	return c == ' ' || c == '\t';
}

static bool is_ctl(char c)
{
	return c < 32 || c == 127;
}

static bool is_separator(char c)
{
	return is_whitespace(c)
		|| c == '(' || c == ')'
		|| c == '<' || c == '>'
		|| c == '@' || c == ','
		|| c == ';' || c == ':'
		|| c == '\\' || c == '"'
		|| c == '/' || c == '['
		|| c == ']' || c == '?'
		|| c == '=' || c == '{'
		|| c == '}';
}

static bool skip_whitespace(Str* text)
{
	size_t i = 0;
	for (; i < text->length; i++)
	{
		if (!is_whitespace(text->data[i]))
			break;
	}

	*text = str_slice(*text, i, text->length);

	return i > 0;
}

static Option_Str next_token(Str* text)
{
	size_t i = 0;
	bool in_quote = false;

	// backslash escapes in quotes, i.e. "lorem \" ipsum"
	bool in_quoted_pair = false;

	for (; i < text->length; i++)
	{
		char current = text->data[i];

		// skip the next char after a backslash
		if (in_quoted_pair)
		{
			in_quoted_pair = false;
			continue;
		}

		if (current == '"')
		{
			in_quote = !in_quote;
			continue;
		}
		else if (in_quote)
		{
			if (current == '\\')
				in_quoted_pair = true;
			else if (current == '\n')
				break;

			continue;
		}

		if (is_separator(current) || is_ctl(current))
			break;
	}

	if (in_quote)
		// TODO: error
		return option_Str_none;

	if (i == 0)
		return option_Str_none;

	Str result = str_slice(*text, 0, i);
	*text = str_slice(*text, i, text->length);
	return option_Str_some(result);
}

/// RFC 2616 section 4.2 field-content
static Option_Str next_field_content(Str* text)
{
	size_t i = 0;
	bool in_quote = false;

	// backslash escapes in quotes, i.e. "lorem \" ipsum"
	bool in_quoted_pair = false;

	for (; i < text->length; i++)
	{
		char current = text->data[i];

		// skip the next char after a backslash
		if (in_quoted_pair)
		{
			in_quoted_pair = false;
			continue;
		}

		if (current == '"')
		{
			in_quote = !in_quote;
			continue;
		}
		else if (in_quote)
		{
			if (current == '\\')
				in_quoted_pair = true;
			else if (current == '\n')
				break;

			continue;
		}

		if (is_ctl(current))
			break;
	}

	if (in_quote)
		// TODO: error
		return option_Str_none;

	if (i == 0)
		return option_Str_none;

	Str result = str_slice(*text, 0, i);
	*text = str_slice(*text, i, text->length);
	return option_Str_some(result);
}

static Option_Str next_until_whitespace(Str* text)
{
	size_t i = 0;
	bool in_quote = false;

	// backslash escapes in quotes, i.e. "lorem \" ipsum"
	bool in_quoted_pair = false;

	for (; i < text->length; i++)
	{
		char current = text->data[i];

		// skip the next char after a backslash
		if (in_quoted_pair)
		{
			in_quoted_pair = false;
			continue;
		}

		if (current == '"')
		{
			in_quote = !in_quote;
			continue;
		}
		else if (in_quote)
		{
			if (current == '\\')
				in_quoted_pair = true;

			continue;
		}

		if (is_whitespace(current))
			break;
	}

	if (in_quote)
		// TODO: error
		return option_Str_none;

	if (i == 0)
		return option_Str_none;

	Str result = str_slice(*text, 0, i);
	*text = str_slice(*text, i, text->length);
	return option_Str_some(result);
}

static Option_HttpVersion next_http_version(Str* text)
{
	// at least "HTTP" "/" DIGIT "." DIGIT
	if (text->length < 8)
		return option_HttpVersion_none;

	if (!str_eq(str_slice(*text, 0, 5), str_from_cstr("HTTP/")))
		return option_HttpVersion_none;

	*text = str_slice(*text, 5, text->length);

	// Verify the first char is a digit since strtoul accepts leading whitespace and sign
	if (!isdigit(text->data[0]))
		return option_HttpVersion_none;
	
	char* major_end;
	unsigned long major = strtoul(text->data, &major_end, 10);
	*text = str_slice(*text, major_end - text->data, text->length);

	if (text->data[0] != '.')
		return option_HttpVersion_none;

	*text = str_slice(*text, 1, text->length);
	
	char* minor_end;
	unsigned long minor = strtoul(text->data, &minor_end, 10);
	*text = str_slice(*text, minor_end - text->data, text->length);

	return option_HttpVersion_some((HttpVersion) {
		.major = major < UINT_MAX ? major : UINT_MAX,
		.minor = minor < UINT_MAX ? minor : UINT_MAX,
	});
}

static bool skip_newline(Str* text)
{
	// Accept LF and ignore the leading CR, see RFC 2616 19.3 Tolerant Applications
	if (text->length > 0 && *text->data == '\r')
		*text = str_slice(*text, 1, text->length);

	if (text->length == 0 || *text->data != '\n')
		return false;

	*text = str_slice(*text, 1, text->length);
	return true;
}

Result_HttpRequest_HttpRequestParseError parse_request_head(Str request)
{
#define ERR(x) result_HttpRequest_HttpRequestParseError_err(HTTP_REQUEST_PARSE_ERROR_ ## x)
	HttpRequest result;
	Str text = request;
	
	// method:
	Option_Str method = next_token(&text);

	if (!method.has_value)
		return ERR(MALFORMED);

	if (!skip_whitespace(&text))
		return ERR(MALFORMED);

	// URI:
	Option_Str uri = next_until_whitespace(&text);

	if (!uri.has_value)
		return ERR(MALFORMED);

	if (!skip_whitespace(&text))
		return ERR(MALFORMED);

	// HTTP version:
	// TODO: not a token
	Option_HttpVersion http_version_opt = next_http_version(&text);

	if (!http_version_opt.has_value)
		return ERR(MALFORMED);

	skip_whitespace(&text);

	if (!skip_newline(&text))
		return ERR(MALFORMED);

	Option_HttpMethod parsed_method = http_method_from_str(method.value);
	if (!parsed_method.has_value)
		return ERR(UNKNOWN_METHOD);
	result.method = parsed_method.value;

	HttpVersion http_version = http_version_opt.value;
	if (http_version.major != 1 || (http_version.minor != 0 && http_version.minor != 1))
		return ERR(UNKNOWN_VERSION);
	result.version = http_version;

	Option_StrBuffer decoded_uri = uri_decode_absolute_path(uri.value);
	if (!decoded_uri.has_value)
		return ERR(INVALID_URI);
	result.uri = decoded_uri.value;

	// Headers:
	Hashmap_StrCI_StrBuffer headers = hashmap_StrCI_StrBuffer_new(random_next());
	
	while (true)
	{
		if (skip_newline(&text))
			break;

		Option_Str header_key = next_token(&text);

		if (!header_key.has_value)
		{
			log_msg(LOG_DEBUG, "header key invalid");
			return ERR(MALFORMED);
		}

		if (text.length > 0 && *text.data == ':')
			text = str_slice(text, 1, text.length);
		else
		{
			log_msg(LOG_DEBUG, "header definition missing a separator");
			return ERR(MALFORMED);
		}

		skip_whitespace(&text);

		// FIXME: fix - field-content not token, see RFC 2616 4.2 Message Headers
		Option_Str header_value = next_field_content(&text);

		if (!skip_newline(&text))
		{
			log_msg(LOG_DEBUG, "header definition missing a terminating newline");
			return ERR(MALFORMED);
		}

		StrBuffer* old = hashmap_StrCI_StrBuffer_get(&headers, &header_key.value);

		if (old != nullptr)
		{
			vector_char_extend(old, ",", 1);
			vector_char_extend(old, header_value.value.data, header_value.value.length);
		}
		else
		{
			StrBuffer value_buf = vector_char_new(header_value.value.length);
			vector_char_extend(&value_buf, header_value.value.data, header_value.value.length);
			hashmap_StrCI_StrBuffer_insert(&headers, header_key.value, value_buf);
		}
	}

	result.headers = headers;
	result.head_length = request.length - text.length;

	return result_HttpRequest_HttpRequestParseError_ok(result);
#undef ERR
}
