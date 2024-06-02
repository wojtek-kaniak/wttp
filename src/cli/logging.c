#include "common.h"
#include <stdio.h>
#include <string.h>

[[noreturn]] void panic(const char* NONNULL message, const char* NONNULL file, size_t line)
{
	fprintf(stderr, "panicked '%s'\n\tat %s:%zu\n", message, file, line);

#if defined (DEBUG) && defined (__unix__)
	// force a core dump
	raise(SIGABRT);
#endif
	exit(1);
}

[[noreturn]] void alloc_fail(const char* NONNULL file, size_t line)
{
	fprintf(stderr, "allocation failed\n");

#ifdef DEBUG
	fprintf(stderr, "\tat %s:%zu\n", file, line);
#else
	UNUSED(file);
	UNUSED(line);
#endif

#if defined (DEBUG) && defined (__unix__)
	// force a core dump
	raise(SIGABRT);
#endif
	exit(1);
}

static const char* log_level_to_str(LogLevel level)
{
	switch (level)
	{
		case LOG_DEBUG:
			return "[DEBUG]";
			break;
		case LOG_INFO:
			return " [INFO]";
			break;
		case LOG_WARN:
			return " [WARN]";
			break;
		case LOG_ERROR:
			return "[ERROR]";
			break;
		case LOG_FATAL:
			return "[FATAL]";
			break;
	}
}

void log_msg(LogLevel level, const char * NONNULL message)
{
#if !defined (DEBUG)
	if (level == LOG_DEBUG)
		return;
#endif

	fprintf(stderr, "%s %s\n", log_level_to_str(level), message);
}

void log_with_errno(LogLevel level, const char * _Nonnull message, int err_no)
{
#if !defined (DEBUG)
	if (level == LOG_DEBUG)
		return;
#endif

	fprintf(stderr, "%s %s: %s\n", log_level_to_str(level), message, strerror(err_no));
}
