#if !defined (INCL_BSD_SOCKETS_H)
#define INCL_BSD_SOCKETS_H

#include <sys/socket.h>

typedef struct BsdSocketConfig {
	int domain;
	struct sockaddr_storage address;
	int backlog;
} BsdSocketConfig;

#endif
