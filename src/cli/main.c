#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sched.h>

#include "bsd_sockets.h"
#include "common.h"
#include "file_request_handler.h"
#include "initialize.h"
#include "listeners.h"
#include "server.h"

#include "fs.h"

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
	#define INVALID_OPT(message...) { fprintf(stderr, message); exit(2); }
	#define INVALID_OPT_ARG_REQUIRED(opt) INVALID_OPT("option `%s` requires an argument", opt)
	#define ARG_MATCH(arg, match) (strcmp(arg, match) == 0)
	#define ARG_MATCH2(arg, long, short) (strcmp(arg, long) == 0 || strcmp(arg, short) == 0)
	
	uint16_t port = 8080;
	const char* ipv4 = nullptr;
	const char* ipv6 = nullptr;
	const char* web_root = nullptr;

	assert(shift_args(&argc, &argv));
	
	// TODO: use getopt?
	const char* arg;
	while ((arg = shift_args(&argc, &argv)) != nullptr)
	{
		if (ARG_MATCH2(arg, "--port", "-p"))
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
				 	INVALID_OPT("invalid port `%s`", port_str);
			}
			else
				INVALID_OPT_ARG_REQUIRED(arg);
		}
		else if (ARG_MATCH(arg, "--ipv4"))
		{
			ipv4 = shift_args(&argc, &argv);

			if (ipv4 == nullptr)
				INVALID_OPT_ARG_REQUIRED(arg);
		}
		else if (ARG_MATCH(arg, "--ipv6"))
		{
			ipv6 = shift_args(&argc, &argv);

			if (ipv6 == nullptr)
				INVALID_OPT_ARG_REQUIRED(arg);
		}
		else if (ARG_MATCH2(arg, "--root-dir", "-d"))
		{
			web_root = shift_args(&argc, &argv);

			if (web_root == nullptr)
				INVALID_OPT_ARG_REQUIRED(arg);
		}
		else
			INVALID_OPT("unrecognized option `%s`", arg);
	}

	if (!web_root)
		web_root = ".";

	fs_chroot(web_root);

	initialize_global();
	initialize_thread();

	BsdSocketConfig socket_config = { .backlog = 0 };
	union AnySockAddr sockaddr;

	if (ipv4 && ipv6)
		INVALID_OPT("options `--ipv4` and `--ipv6` are mutually exclusive");

	if (ipv4)
	{
		socket_config.domain = AF_INET;
		sockaddr.inet.sin_family = AF_INET;
		sockaddr.inet.sin_port = htons(port);
		if (inet_pton(AF_INET, ipv4, &sockaddr.inet.sin_addr) != 1)
			INVALID_OPT("invalid IPv4");
	}
	else if (ipv6)
	{
		socket_config.domain = AF_INET6;
		sockaddr.inet6.sin6_family = AF_INET6;
		sockaddr.inet6.sin6_port = htons(port);
		if (inet_pton(AF_INET6, ipv6, &sockaddr.inet6.sin6_addr) != 1)
			INVALID_OPT("invalid IPv6");
	}
	else
	{
		socket_config.domain = AF_INET;
		sockaddr.inet.sin_family = AF_INET;
		sockaddr.inet.sin_port = htons(port);
		assert(inet_pton(AF_INET, "0.0.0.0", &sockaddr.inet.sin_addr) == 1);
	}
	socket_config.address = sockaddr.storage;

	RequestHandler req_handler = FILE_REQUEST_HANDLER;
	st_bsd_start_listener(socket_config, req_handler);
}
