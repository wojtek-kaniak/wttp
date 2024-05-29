#ifndef INCL_COMMON_H
#define INCL_COMMON_H

#if defined (__unix__) && defined (DEBUG)
#include <signal.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define WTTP_P_CONCAT(a, b) a ## b
#define WTTP_P_CONCAT3(a, b, c) a ## b ## c
#define WTTP_P_CONCAT4(a, b, c, d) a ## b ## c ## d
#define WTTP_P_CONCAT5(a, b, c, d, e) a ## b ## c ## d ## e
#define WTTP_P_STRINGIFY(a) #a

#if !defined (COUNTOF)
#define COUNTOF(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifndef UNUSED
#define UNUSED(var) (void)(var)
#endif

#ifndef NONNULL

#ifdef __clang__
#define NONNULL _Nonnull
#else
#define NONNULL [[gcc:nonnull]]
#endif

#endif

#ifndef NULLABLE

#ifdef __clang__
#define NULLABLE _Nullable
#else
#define NULLABLE
#endif

#endif

/// Display an allocation failure message and abort
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

#define ALLOC_FAIL() alloc_fail(__FILE__, __LINE__)

/// Display a panic message and abort
[[noreturn]] void panic(const char* NONNULL message, const char* NONNULL file, size_t line)
{
	fprintf(stderr, "panicked '%s'\n\tat %s:%zu\n", message, file, line);

#if defined (DEBUG) && defined (__unix__)
	// force a core dump
	raise(SIGABRT);
#endif
	exit(1);
}

#if defined (DEBUG)
#define DEBUG_ASSERT(condition) assert(condition)
#else
#define DEBUG_ASSERT(condition) UNUSED(condition)
#endif

typedef enum LogLevel
{
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,
} LogLevel;

/// Log a message
void log_msg(LogLevel level, const char* message);

/// Log a message with an associated OS error number
void log_with_errno(LogLevel level, const char* message, int err_no);

#define PANIC(message) panic(message, __FILE__, __LINE__)

#endif
