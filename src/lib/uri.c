#include "uri.h"
#include "types.h"

static bool is_uri_safe(char c);
static Option_char from_percent_encoding(char ha, char hb);

Option_StrBuffer uri_decode_absolute_path(Str uri)
{
	StrBuffer result = vector_char_new(0);

	if (uri.length == 0 || uri.data[0] != '/')
		goto error;
	
	for (size_t i = 0; i < uri.length; i++)
	{
		char c = uri.data[i];

		if (c == '?')
			break;

		if (c == '%')
		{
			size_t bytes_left = uri.length - i - 1;

			if (bytes_left < 2)
				goto error;
			
			Option_char encoded_char = from_percent_encoding(uri.data[i + 1], uri.data[i + 2]);

			if (!encoded_char.has_value)
				goto error;

			vector_char_add(&result, encoded_char.value);
			i += 2;
		}
		else
		{
			if (!is_uri_safe(c))
				goto error;
			
			vector_char_add(&result, c);
		}
	}

	return option_StrBuffer_some(result);

	error:
	vector_char_free(&result);
	return option_StrBuffer_none;
}

static bool is_uri_safe(char c)
{
	// TODO: https://url.spec.whatwg.org/#url-code-points
	return c != '\0';
}

static Option_char from_hex(char c);

static Option_char from_percent_encoding(char ha, char hb)
{
	Option_char a = from_hex(ha);

	if (!a.has_value)
		return option_char_none;

	Option_char b = from_hex(hb);
	
	if (!b.has_value)
		return option_char_none;

	return option_char_some((a.value << 4) + b.value);
}

static Option_char from_hex(char c)
{
	if (c >= '0' && c <= '9')
		return option_char_some(c - '0');
	else if (c >= 'a' && c <= 'f')
		return option_char_some(c - 'a' + 10);
	else if (c >= 'A' && c <= 'F')
		return option_char_some(c - 'A' + 10);
	else
		return option_char_none;
}
