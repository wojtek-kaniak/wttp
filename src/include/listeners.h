#if !defined (INCL_LISTENERS_H)
#define INCL_LISTENERS_H

#include "bsd_sockets.h"
#include "server.h"

/// Start a single threaded listener on a socket, passing requests to `request_handler`
/// Supports AF_INET, AF_INET6 and AF_UNIX sockets
void st_bsd_start_listener(BsdSocketConfig config, RequestHandler request_handler);

#endif
