#if !defined (INCL_RECV_UTILS_H)
#define INCL_RECV_UTILS_H

#include <sys/socket.h>
#include "netinet/in.h"

#define RECV_BUFFER_SIZE 8192 /* 8 KiB */
#define RAW_REQUEST_MAX_SIZE 67108864 /* 64 MiB */

// type punning helper union to avoid pointer casts
union AnySockAddr
{
	struct sockaddr_storage storage;
	struct sockaddr_in inet;
	struct sockaddr_in6 inet6;
};

#endif
