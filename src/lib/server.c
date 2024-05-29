#include "http.h"
#include "types.h"
#include "server.h"

#define SERVER_NAME "wttp"

#define CRLF "\r\n"

static StrBuffer format_response(Response response);

void handle_raw_request(RawRequest request, void(*respond)(Str, void*), void* metadata, RequestHandler request_handler)
{
	// TODO: request streaming
	Result_HttpRequest_HttpRequestParseError head_result = parse_request_head(request.buffer);

	Response response;
	if (head_result.is_ok)
		response = request_handler.ok_response(head_result.ok);
	else
		response = request_handler.error_response(head_result.err);

	StrBuffer response_text = format_response(response);
	respond(str_from_strbuffer(response_text), metadata);

	vector_char_free(&response.content);
	vector_char_free(&response_text);
	
	if (response.additional_headers.has_value)
		free_headers(&response.additional_headers.value);
}

static StrBuffer format_response(Response response)
{
	#define STRBUF_EXT_C(str) vector_char_extend(&text, str, sizeof(str) - 1)

	StrBuffer text = vector_char_new(response.content.length + 128);

	STRBUF_EXT_C("HTTP/1.1 ");
	strbuffer_write_uint16_t(&text, response.status_code);
	vector_char_add(&text, ' ');
	
	Option_Str status_code_msg = http_status_code_to_str(response.status_code);
	if (status_code_msg.has_value)
		strbuffer_extend(&text, status_code_msg.value);
	else
		// default status code message
		STRBUF_EXT_C("Internal Server Error");
	
	STRBUF_EXT_C(CRLF "Content-Type: ");
	strbuffer_extend(&text, response.content_type);

	// Content-Length can be overridden (HEAD requests)
	Str content_len_key = str_from_cstr("Content-Length");	
	StrBuffer* content_length = response.additional_headers.has_value
		? hashmap_StrCI_StrBuffer_get(&response.additional_headers.value, &content_len_key)
		: nullptr;
	STRBUF_EXT_C(CRLF "Content-Length: ");
	if (content_length == nullptr)
		strbuffer_write_size_t(&text, response.content.length);
	else
		strbuffer_extend(&text, str_from_strbuffer(*content_length));

	// TODO: handle keep-alive in listeners
	STRBUF_EXT_C(CRLF "Connection: close" CRLF);
	STRBUF_EXT_C(CRLF "Server: " SERVER_NAME CRLF);

	// Additional headers:
	if (response.additional_headers.has_value)
	{
		Hashmap_StrCI_StrBuffer headers = response.additional_headers.value;
		for (size_t bucket_ix = 0; bucket_ix < COUNTOF(headers.buckets); bucket_ix++)
		{
			for (size_t i = 0; i < headers.buckets[bucket_ix].length; i++)
			{
				KeyValuePair_StrCI_StrBuffer kvp = headers.buckets[bucket_ix].data[i];
				strbuffer_extend(&text, kvp.key);
				STRBUF_EXT_C(": ");
				strbuffer_extend(&text, str_from_strbuffer(kvp.value));
				STRBUF_EXT_C(CRLF);
			}
		}
	}

	STRBUF_EXT_C(CRLF);

	strbuffer_extend(&text, str_from_strbuffer(response.content));

	return text;
}
