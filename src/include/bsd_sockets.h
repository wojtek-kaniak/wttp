#if !defined (INCL_BSD_SOCKETS_H)
#define INCL_BSD_SOCKETS_H

#include <sys/socket.h>
#include "netinet/in.h"

typedef struct BsdSocketConfig {
	int domain;
	struct sockaddr_storage address;
	int backlog;
} BsdSocketConfig;

// type punning helper union to avoid pointer casts
union AnySockAddr
{
	struct sockaddr_storage storage;
	struct sockaddr_in inet;
	struct sockaddr_in6 inet6;
};

#endif
