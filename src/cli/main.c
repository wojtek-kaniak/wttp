#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "common.h"
#include "bsd_sockets.h"
#include "initialize.h"

static const char* NULLABLE shift_args(int* NONNULL argc, char*** NONNULL argv)
{
	if (*argc == 0)
		return nullptr;
	
	char* current_arg = (*argv)[0];

	*argc -= 1;
	*argv += 1;

	return current_arg;
}

int main(int argc, char** argv)
{
	#define INVALID_ARG(message...) { fprintf(stderr, message); exit(2); }
	
	uint16_t port = 8080;

	const char* arg;
	assert(shift_args(&argc, &argv));
	
	while ((arg = shift_args(&argc, &argv)) != nullptr)
	{
		if (strcmp(arg, "-p") == 0)
		{
			const char* port_str = shift_args(&argc, &argv);

			if (port_str)
			{
				char* endptr;
				
				errno = 0;
				int parsed = strtol(port_str, &endptr, 10);

				if (*endptr == '\0' && errno == 0)
					port = parsed;
				else
				 	INVALID_ARG("invalid port `%s`", port_str);
			}
			else
				INVALID_ARG("option `-p` requires an argument")
		}
		else
			INVALID_ARG("unrecognized option `%s`", arg);
	}

	printf("port: %d\n", (int)port);

	initialize_global();
	initialize_thread();
}
