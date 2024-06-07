#if !defined(INCL_HTTP_THREAD_H)
#define INCL_HTTP_THREAD_H

#include <stdint.h>
#include <stdatomic.h>

typedef struct HttpConfig
{
	const char* webroot;
	uint16_t port;
	volatile atomic_flag continue_flag;
} HttpConfig;

/// Start a listener using `HttpConfig` passed in `arg`
/// SAFETY: `arg` must be a pointer to `HttpConfig`
void http_thread(HttpConfig* config);

/// chroot into `webroot`, should be called before creating any threads
void webroot_chroot(const char* webroot);

#endif
