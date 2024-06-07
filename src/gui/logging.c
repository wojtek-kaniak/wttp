#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

#include "logging.h"

#define LOG_PATH_SUFFIX "wttp-gui/http.log"

static FILE* log_file;

/// Append a relative path to a base path
/// `relative` should not start with a slash
static char* join_paths(const char* base, const char* relative)
{
	size_t base_len = strlen(base);
	size_t relative_len = strlen(relative);

	size_t new_len = base_len + relative_len + 1;

	bool insert_sep = base[base_len - 1] != '/';
	
	if (insert_sep)
		new_len++;

	char* new_path = malloc(new_len);

	memcpy(new_path, base, base_len);

	if (insert_sep)
	{
		new_path[base_len] = '/';
		base_len++;
	}

	memcpy(new_path + base_len, relative, relative_len);
	new_path[base_len + relative_len] = '\0';
	
	return new_path;
}

void logging_initialize()
{
	const char* log_path = getenv("WTTP_HTTP_LOG");

	if (!log_path)
	{
		char* xdg_state = getenv("XDG_STATE_HOME");

		// fallback if $XDG_STATE_HOME is not defined
		if (xdg_state)
			log_path = join_paths(xdg_state, LOG_PATH_SUFFIX);
		else if (getenv("HOME"))
			log_path = join_paths(getenv("HOME"), ".local/state/" LOG_PATH_SUFFIX);
	}

	if (log_path)
	{
		log_file = fopen(log_path, "a");

		if (!log_file)
			perror("failed to open a log file");

		// log_path memory is leaked if $WTTP_HTTP_LOG was not set, but this is acceptable
	}
}

[[noreturn]] void panic(const char* NONNULL message, const char* NONNULL file, size_t line)
{
	log_msg(LOG_FATAL, "panicked");
    fprintf(stderr, "panicked '%s'\n\tat %s:%zu\n", message, file, line);

#if defined (DEBUG) && defined (__unix__)
    // force a core dump
    raise(SIGABRT);
#endif
    exit(1);
}

[[noreturn]] void alloc_fail(const char* NONNULL file, size_t line)
{
	log_msg(LOG_FATAL, "allocation failed");

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

	if (log_file)
	    fprintf(log_file, "%s %s\n", log_level_to_str(level), message);

    fprintf(stderr, "%s %s\n", log_level_to_str(level), message);
}

void log_with_errno(LogLevel level, const char * _Nonnull message, int err_no)
{
#if !defined (DEBUG)
    if (level == LOG_DEBUG)
        return;
#endif

	if (log_file)
	    fprintf(log_file, "%s %s: %s\n", log_level_to_str(level), message, strerror(err_no));

    fprintf(stderr, "%s %s: %s\n", log_level_to_str(level), message, strerror(err_no));
}
