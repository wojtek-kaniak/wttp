#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "common.h"
#include "http.h"
#include "server.h"
#include "types.h"

#include "file_request_handler.h"

#if defined (DEBUG)
static bool SHOW_ERROR_MESSAGE = true;
#else
static bool SHOW_ERROR_MESSAGE = false;
#endif

#define LF "\n"

#define ERROR_HTML(message) \
	"<!DOCTYPE html>" LF \
	"<html>" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<title>" message "</title>" \
		"</head>" \
		"<body>" \
		"<h1>\"" message "\"</h1>" \
		"</body>" \
	"</html>" LF

static Response internal_error_response(const char* NONNULL message);
static Response not_found_response(const char* NONNULL path);

static Str index_filename;

void set_index_filename(Str filename)
{
	index_filename = filename;
}

Response ok_response(HttpRequest request)
{
	assert(request.uri.data != nullptr);
	
	if (request.uri.data[request.uri.length - 1] == '/')
		strbuffer_extend(&request.uri, index_filename);

	// null terminate the URI
	vector_char_add(&request.uri, '\0');

	const char* path = request.uri.data;

	int fd = open(path, O_RDONLY);
	if (fd != -1)
	{
		struct stat file_stat;
		if (fstat(fd, &file_stat) != 0)
		{
			const char* error_msg = "failed to stat the response file";
			log_with_errno(LOG_ERROR, error_msg, errno);
			return internal_error_response(error_msg);
		}
		
		if (request.method != HTTP_GET && request.method != HTTP_HEAD)
		{
			log_msg(LOG_DEBUG, "unsupported HTTP method");
			Hashmap_StrCI_StrBuffer headers = hashmap_StrCI_StrBuffer_new(0);
			hashmap_StrCI_StrBuffer_insert(
				&headers,
				str_from_cstr("Allow"),
				strbuffer_from_str(str_from_cstr("GET,HEAD"))
			);

			return (Response) {
				.status_code = HTTP_STATUS_METHOD_NOT_ALLOWED,
				.content_type = str_from_cstr("text/html"),
				.additional_headers = option_Hashmap_StrCI_StrBuffer_some(headers),
				.content = strbuffer_from_str(str_from_cstr(ERROR_HTML("405 - Method Not Allowed"))),
			};
		}

		if (request.method == HTTP_HEAD)
		{
			Hashmap_StrCI_StrBuffer headers = hashmap_StrCI_StrBuffer_new(0);
			StrBuffer content_length_value = vector_char_new(16);
			strbuffer_write_size_t(&content_length_value, file_stat.st_size);
			hashmap_StrCI_StrBuffer_insert(
				&headers,
				str_from_cstr("Content-Length"),
				content_length_value
			);

			return (Response) {
				.status_code = 200,
				.content_type = str_from_cstr("text/html"),
				.additional_headers = option_Hashmap_StrCI_StrBuffer_some(headers),
				.content = strbuffer_from_raw(nullptr, 0, 0),
			};
		}

		size_t buffer_size = file_stat.st_size;
		char* buffer = malloc(buffer_size);

		if (buffer == nullptr)
			ALLOC_FAIL();

		// file size can change between stat'ing and reading
		ssize_t bytes_read = pread(fd, buffer, file_stat.st_size, 0);

		if (bytes_read == -1)
		{
			free(buffer);
			log_with_errno(LOG_WARN, "failed to read the response file", errno);
			return not_found_response(path);
		}

		if (bytes_read < file_stat.st_size)
			log_msg(LOG_DEBUG, "response file size changed between stat and read calls");

		return (Response) {
			.status_code = 200,
			.content_type = str_from_cstr("text/html"),
			.content = strbuffer_from_raw(buffer, buffer_size, bytes_read),
			.additional_headers = option_Hashmap_StrCI_StrBuffer_none,
		};
	}
	else
	{
		if (errno == EACCES ||
			errno == EINVAL /* path unsupported by the fs */ ||
			errno == ENAMETOOLONG ||
			errno == ENOENT)
		{
			log_with_errno(LOG_DEBUG, "failed to open the response file (404 Not Found)", errno);
			return not_found_response(path);
		}
		else
		{
			const char* error_msg = "failed to open the response file";
			log_with_errno(LOG_ERROR, error_msg, errno);
			return internal_error_response(error_msg);
		}
	}
}

Response error_response(HttpRequestParseError error)
{
#define MSG_PREFIX "invalid request received: "
	HttpStatusCode status_code;
	const char* log_message;
	const char* content;

	// use http_status_code_to_str?
	switch (error)
	{
		case HTTP_REQUEST_PARSE_ERROR_MALFORMED:
			status_code = HTTP_STATUS_BAD_REQUEST;
			log_message = MSG_PREFIX "malformed request";
			content = ERROR_HTML("400 - Bad Request");
			break;
		case HTTP_REQUEST_PARSE_ERROR_UNKNOWN_METHOD:
			// 501 Not Implemented on an invalid method, see https://httpwg.org/specs/rfc9110.html#methods
			status_code = HTTP_STATUS_NOT_IMPLEMENTED;
			log_message = MSG_PREFIX "unknown HTTP method";
			content = ERROR_HTML("501 - Not Implemented - unknown HTTP method");
			break;
		case HTTP_REQUEST_PARSE_ERROR_INVALID_URI:
			status_code = HTTP_STATUS_BAD_REQUEST;
			log_message = MSG_PREFIX "invalid URI";
			content = ERROR_HTML("400 - Bad Request - invalid URI");
			break;
		case HTTP_REQUEST_PARSE_ERROR_UNKNOWN_VERSION:
			status_code = HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED;
			log_message = MSG_PREFIX "unknown HTTP version";
			content = ERROR_HTML("505 - HTTP Version Not Supported");
			break;
	};

	log_msg(LOG_DEBUG, log_message);

	return (Response) {
		.status_code = status_code,
		.content_type = str_from_cstr("text/html"),
		.content = strbuffer_from_str(str_from_cstr(content)),
		.additional_headers = option_Hashmap_StrCI_StrBuffer_none,
	};
#undef MSG_PREFIX
}

static Response not_found_response(const char* NONNULL path)
{
	const char template[] = "<!DOCTYPE html>" LF
		"<html>"
			"<head>"
				"<meta charset=\"UTF-8\">"
				"<title>Not Found</title>"
			"</head>"
			"<body>"
				"<h1>Not Found</h1>"
				"The requested page \"%s\" was not found on this server."
			"</body>"
		"</html>" LF;

	size_t buffer_len = sizeof(template) - 2 + strlen(path);
	char* buffer = malloc(buffer_len);

	if (buffer == nullptr)
		ALLOC_FAIL();

	assert(snprintf(buffer, buffer_len, template, path) >= 0);

	return (Response) {
		.status_code = HTTP_STATUS_NOT_FOUND,
		.content_type = str_from_cstr("text/html"),
		.content = strbuffer_from_raw(buffer, buffer_len, buffer_len),
	};
}

static Response internal_error_response(const char* NONNULL message)
{
	if (!SHOW_ERROR_MESSAGE)
	{
		return (Response) {
			.status_code = HTTP_STATUS_INTERNAL_SERVER_ERROR,
			.content_type = str_from_cstr("text/html"),
			.content = strbuffer_from_str(str_from_cstr(ERROR_HTML("Internal Server Error"))),
		};
	}
	else
	{
		const char template[] = "<!DOCTYPE html>" LF
			"<html>"
				"<head>"
					"<meta charset=\"UTF-8\">"
					"<title>Internal Server Error</title>"
				"</head>"
				"<body>"
					"<h1>Internal Server Error</h1>"
					"<code>\"%s\"</code>"
				"</body>"
			"</html>" LF;

		size_t buffer_len = sizeof(template) - 2 + strlen(message);
		char* buffer = malloc(buffer_len);

		if (buffer == nullptr)
			ALLOC_FAIL();

		assert(snprintf(buffer, buffer_len, template, message) >= 0);

		return (Response) {
			.status_code = HTTP_STATUS_INTERNAL_SERVER_ERROR,
			.content_type = str_from_cstr("text/html"),
			.content = strbuffer_from_raw(buffer, buffer_len, buffer_len),
			.additional_headers = option_Hashmap_StrCI_StrBuffer_none,
		};
	}
}

RequestHandler FILE_REQUEST_HANDLER = (RequestHandler) {
	.ok_response = ok_response,
	.error_response = error_response,
};