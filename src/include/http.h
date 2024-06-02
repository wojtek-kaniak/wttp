#ifndef INCL_HTTP_H
#define INCL_HTTP_H
#include <stdint.h>

#include "types.h"

#define WTTP_P_METHOD_LIST \
	WTTP_P_X(GET)     \
	WTTP_P_X(HEAD)    \
	WTTP_P_X(POST)    \
	WTTP_P_X(PUT)     \
	WTTP_P_X(PATCH)   \
	WTTP_P_X(DELETE)  \
	WTTP_P_X(CONNECT) \
	WTTP_P_X(TRACE)   \
	WTTP_P_X(OPTIONS)

typedef enum HttpMethod
{
#define WTTP_P_X(ident) HTTP_ ## ident,
	WTTP_P_METHOD_LIST
#undef WTTP_P_X
} HttpMethod;

#define TYPE HttpMethod
#include "generic/option.h"
#undef TYPE

[[maybe_unused]] static Option_Str http_method_to_str(HttpMethod method)
{
	switch (method)
	{
#define WTTP_P_X(ident) \
		case HTTP_ ## ident: \
			return option_Str_some(str_from_cstr(#ident));

		WTTP_P_METHOD_LIST
#undef WTTP_P_X
		default:
			return option_Str_none;
	}
}

[[maybe_unused]] static Option_HttpMethod http_method_from_str(Str str)
{
#define WTTP_P_X(ident) \
	if (str_eq(str_from_cstr(#ident), str)) \
		return option_HttpMethod_some(HTTP_ ## ident);

	WTTP_P_METHOD_LIST
#undef WTTP_P_X
	return option_HttpMethod_none;
}

typedef struct HttpVersion
{
	unsigned int major;
	unsigned int minor;
} HttpVersion;

#define TYPE HttpVersion
#include "generic/option.h"
#undef TYPE

/// Parsed HTTP request, without the content
typedef struct HttpRequest
{
	HttpMethod method;
	HttpVersion version;
	StrBuffer uri;
	Hashmap_StrCI_StrBuffer headers;
	size_t head_length;
} HttpRequest;

#define TYPE HttpRequest
#include "generic/option.h"
#undef TYPE

#define WTTP_P_STATUSCODE_LIST \
	WTTP_P_X(100, CONTINUE, "Continue") \
	WTTP_P_X(200, OK, "OK") \
	WTTP_P_X(400, BAD_REQUEST, "Bad Request") \
	WTTP_P_X(403, FORBIDDEN, "Forbidden") \
	WTTP_P_X(404, NOT_FOUND, "Not Found") \
	WTTP_P_X(405, METHOD_NOT_ALLOWED, "Method Not Allowed") \
	WTTP_P_X(406, NOT_ACCEPTABLE, "Not Acceptable") \
	WTTP_P_X(408, REQUEST_TIMEOUT, "Request Timeout") \
	WTTP_P_X(414, REQUEST_URI_TOO_LONG, "Request URI Too Long") \
	WTTP_P_X(500, INTERNAL_SERVER_ERROR, "Internal Server Error") \
	WTTP_P_X(501, NOT_IMPLEMENTED, "Not Implemented") \
	WTTP_P_X(503, SERVICE_UNAVAILABLE, "Service Unavailable") \
	WTTP_P_X(505, HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported")

typedef enum HttpStatusCode : uint16_t
{
#define WTTP_P_X(code, ident, _name) HTTP_STATUS_ ## ident = code,
	WTTP_P_STATUSCODE_LIST
#undef WTTP_P_X
} HttpStatusCode;

/// Returns the default phrase for the status code
[[maybe_unused]] static Option_Str http_status_code_to_str(HttpStatusCode status_code)
{
	switch (status_code)
	{
#define WTTP_P_X(_code, ident, name) \
		case HTTP_STATUS_ ## ident: \
			return option_Str_some(str_from_cstr(name));
		
		WTTP_P_STATUSCODE_LIST
		default:
			return option_Str_none;
#undef WTTP_P_X
	}
}

#define WTTP_P_REQUESTPARSEERROR_LIST \
	WTTP_P_X(MALFORMED) \
	WTTP_P_X(UNKNOWN_METHOD) \
	WTTP_P_X(INVALID_URI) \
	WTTP_P_X(UNKNOWN_VERSION)

typedef enum HttpRequestParseError
{
#define WTTP_P_X(ident) HTTP_REQUEST_PARSE_ERROR_ ## ident,
	WTTP_P_REQUESTPARSEERROR_LIST
#undef WTTP_P_X
} HttpRequestParseError;

#define TYPE_OK HttpRequest
#define TYPE_ERR HttpRequestParseError
#include "generic/result.h"
#undef TYPE_OK
#undef TYPE_ERR

/// Parse the HTTP request head (without the content) from the entire request
Result_HttpRequest_HttpRequestParseError parse_request_head(Str request);

#endif
