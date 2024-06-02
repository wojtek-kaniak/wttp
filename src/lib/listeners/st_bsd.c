// A single-threaded BSD socket listener
// TODO: multithreading with https://lwn.net/Articles/542629/

// #define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "common.h"
#include "types.h"
#include "bsd_sockets.h"
#include "recv_utils.h"
#include "server.h"

#include "listeners.h"

static void respond(Str buffer, void* metadata);

static void shutdown_and_close_conn(int connection_fd);

[[nodiscard]] static StrBuffer sockaddr_to_remote_addr(struct sockaddr_storage sockaddr);

[[nodiscard]] static uint16_t sockaddr_to_remote_port(struct sockaddr_storage sockaddr);

typedef struct RequestMetadata
{
	int connection_fd;
	StrBuffer buffer;
	StrBuffer address_buf;
} RequestMetadata;

void st_bsd_start_listener(BsdSocketConfig config, RequestHandler request_handler)
{
	if (config.domain != AF_INET && config.domain != AF_INET6 && config.domain != AF_UNIX)
	{
		log_msg(LOG_FATAL, "unsupported socket domain (expected AF_INET, AF_INET6 or AF_UNIX)");
		return;
	}

	int socket_fd = socket(config.domain, SOCK_STREAM, IPPROTO_TCP);

	if (socket_fd == -1)
	{
		log_with_errno(LOG_FATAL, "failed to create a socket", errno);
		return;
	}

	// Disable Nagle's algorithm - the entire response is sent at once
	int tcp_nodelay = 1;
	if (setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &tcp_nodelay, sizeof(int)) == -1)
		log_with_errno(LOG_WARN, "failed to set TCP_NODELAY", errno);

	// TODO: is sockaddr_storage correct?
	if (bind(socket_fd, (struct sockaddr*)&config.address, sizeof(config.address)) == -1)
	{
		log_with_errno(LOG_FATAL, "failed to bind to a socket", errno);
		return;
	}

	if (listen(socket_fd, config.backlog) == -1)
	{
		log_with_errno(LOG_FATAL, "failed to listen on a socket", errno);
		return;
	}

	while (true)
	{
		struct sockaddr_storage connection_sockaddr;
		unsigned int conn_sockaddr_len;
		int connection_fd = accept(socket_fd, (struct sockaddr*)&connection_sockaddr, &conn_sockaddr_len);
		if (connection_fd == -1)
		{
			if (errno == ECONNABORTED || errno == EINTR)
				log_with_errno(LOG_DEBUG, "failed to accept a connection (continuing to listen on the socket)", errno);
			else
				log_with_errno(LOG_ERROR, "failed to accept a connection", errno);

			continue;
		}

		StrBuffer remote_addr = sockaddr_to_remote_addr(connection_sockaddr);
		uint16_t remote_port = sockaddr_to_remote_port(connection_sockaddr);

		log_msg(LOG_DEBUG, "accepted a connection");

		StrBuffer packet = vector_char_new(0);
		while (true)
		{
			if ((RAW_REQUEST_MAX_SIZE - packet.length) < RECV_BUFFER_SIZE)
			{
				log_msg(LOG_DEBUG, "HTTP request too long, closing the connection");
				goto abort_connection;
			}

			// TODO: support keep-alive & pipelining
			char buffer[RECV_BUFFER_SIZE];
			ssize_t bytes_read = recv(connection_fd, buffer, sizeof(buffer), 0);

			if (bytes_read == -1)
			{
				log_with_errno(LOG_DEBUG, "recv failed", errno);
				goto abort_connection;
			}

			if (bytes_read == 0)
				break;

			vector_char_extend(&packet, buffer, bytes_read);

			if (str_find(str_from_strbuffer(packet), STR_CRLF).has_value)
			{
				// TODO
				break;
			}
		}

		RawRequest req = {
			.buffer = (Str) { .data = packet.data, .length = packet.length },
			.remote_address = str_from_strbuffer(remote_addr),
			.remote_port = remote_port,
		};

		RequestMetadata* meta = malloc(sizeof(RequestMetadata));

		if (meta == nullptr)
			ALLOC_FAIL();

		*meta = (RequestMetadata){ .connection_fd = connection_fd, .buffer = packet, .address_buf = remote_addr };

		handle_raw_request(req, respond, (void*)meta, request_handler);
		continue;

		abort_connection:
		shutdown_and_close_conn(connection_fd);
		vector_char_free(&remote_addr);
		vector_char_free(&packet);
	}
}

/// Send a response to a request, invoked by [server.c]
static void respond(Str buffer, void* metadata)
{
	RequestMetadata meta = *(RequestMetadata*)metadata;

	ssize_t bytes_sent = send(meta.connection_fd, buffer.data, buffer.length, MSG_NOSIGNAL);

	if (bytes_sent == -1)
	{
		if (errno == ECONNRESET || errno == ENOTCONN || errno == EPIPE || errno == ENETUNREACH)
			log_with_errno(LOG_DEBUG, "failed to respond", errno);
		else
			log_with_errno(LOG_ERROR, "failed to respond", errno);
	}
	else if ((size_t)bytes_sent != buffer.length)
		log_msg(LOG_WARN, "failed to send an entire response");

	shutdown_and_close_conn(meta.connection_fd);

	vector_char_free(&meta.buffer);
	vector_char_free(&meta.address_buf);
	free(metadata);
}

static void shutdown_and_close_conn(int connection_fd)
{
	if (shutdown(connection_fd, SHUT_RDWR) == -1)
	{
		if (errno == ENOTCONN)
			log_with_errno(LOG_DEBUG, "failed to shutdown a connection, socket not connected", errno);
		else
			log_with_errno(LOG_ERROR, "failed to shutdown a connection", errno);
	}

	if (close(connection_fd) == -1)
		log_with_errno(LOG_ERROR, "failed to close a connection socket descriptor", errno);
}

/// Get a string representation of the remote address
[[nodiscard]] static StrBuffer sockaddr_to_remote_addr(struct sockaddr_storage sockaddr)
{
	// FIXME
	// if (sockaddr.ss_family == AF_UNIX)
		return strbuffer_from_str(str_from_cstr("[unix socket]"));

	union AnySockAddr any_sockaddr = { .storage = sockaddr };

	#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
	char address_str[MAX(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)];
	#undef MAX
	
	if (sockaddr.ss_family == AF_INET)
		inet_ntop(AF_INET, &any_sockaddr.inet.sin_addr, address_str, sizeof(address_str));
	else if (sockaddr.ss_family == AF_INET6)
		inet_ntop(AF_INET6, &any_sockaddr.inet6.sin6_addr, address_str, sizeof(address_str));
	else
	 	PANIC("unsupported address family");

	return strbuffer_from_str(str_from_cstr(address_str));
}

/// Get the remote port
[[nodiscard]] static uint16_t sockaddr_to_remote_port(struct sockaddr_storage sockaddr)
{
	// TODO
	union AnySockAddr any_sockaddr = { .storage = sockaddr };

	if (sockaddr.ss_family == AF_INET)
		return ntohs(any_sockaddr.inet.sin_port);
	else if (sockaddr.ss_family == AF_INET6)
		return ntohs(any_sockaddr.inet6.sin6_port);
	else
	 	return 0;
}
