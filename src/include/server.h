#if !defined (INCL_SERVER_H)
#define INCL_SERVER_H

#include <sys/socket.h>
#include "types.h"
#include "http.h"

typedef struct Response
{
	StrBuffer content;
	Str content_type;
	HttpStatusCode status_code;
	Option_Hashmap_StrCI_StrBuffer additional_headers;
} Response;

typedef struct RequestHandler
{
	Response(*ok_response)(HttpRequest);
	Response(*error_response)(HttpRequestParseError);
} RequestHandler;

typedef struct RawRequest
{
	Str buffer;
	Str remote_address;
	uint16_t remote_port;
} RawRequest;

/// Handle a raw (unparsed) request
/// `metadata` is a listener specific data associated with the request passed to `respond`
void handle_raw_request(
	RawRequest request,
	void(*respond)(Str, void*),
	void* metadata,
	RequestHandler request_handler
	);

#endif
