#include <assert.h>
#include "arpa/inet.h"
#include "listeners.h"
#include "netinet/in.h"
#include "sys/socket.h"

#include "file_request_handler.h"
#include "fs.h"
#include "initialize.h"
#include "bsd_sockets.h"

#include "http_thread.h"

void http_thread(HttpConfig* config)
{
	initialize_global();
	initialize_thread();

	union AnySockAddr any_addr;
	any_addr.inet.sin_family = AF_INET;
	any_addr.inet.sin_port = htons(config->port);
	assert(inet_pton(AF_INET, "0.0.0.0", &any_addr.inet.sin_addr) == 1);

	BsdSocketConfig bsd_config = {
		.backlog = 0,
		.domain = AF_INET,
		.address = any_addr.storage,
	};

	st_bsd_start_listener(bsd_config, FILE_REQUEST_HANDLER, &config->continue_flag);
}

void webroot_chroot(const char* webroot)
{
	fs_chroot(webroot);
}
