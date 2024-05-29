#include <sys/socket.h>

#include "bsd_sockets.h"
#include "common.h"
#include "types.h"
#include "st_iouring.h"

typedef struct Connection
{
	Vector_char buffer;
	struct sockaddr address;
} Connection;

static bool int_eq(int* a, int* b)
{
	return *a == *b;
}

static int int_hash(int* value)
{
	return *value;
}

#define TYPE Connection
#include "generic/option.h"
#undef TYPE

#define int_EQ int_eq
#define int_HASH int_hash
#define TYPE_KEY int
#define TYPE_VALUE Connection
#include "generic/hashmap.h"
#undef int_EQ
#undef int_HASH
#undef TYPE_KEY
#undef TYPE_VALUE

void st_iouring_start_listener(BsdSocketConfig config)
{
	// socket fd to connection map
	Hashmap_int_Connection connections;

	// TODO: iouring
	UNUSED(config);
	UNUSED(connections);
}
