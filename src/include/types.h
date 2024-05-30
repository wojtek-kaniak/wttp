#ifndef INCL_TYPES_H
#define INCL_TYPES_H

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

#define TYPE size_t
#include "generic/option.h"
#undef TYPE

#define TYPE char
#include "generic/option.h"
#undef TYPE

// StrBuffer / Vector_char
#define TYPE char
#include "generic/vector.h"
#undef TYPE
typedef Vector_char StrBuffer;

/// A span of memory
/// Unlike C strings may contain NUL (zero) bytes
typedef struct Str
{
	size_t length;
	const char* NULLABLE data;
} Str;

[[maybe_unused]] static const Str STR_CRLF = (Str){
	.length = 2,
	.data = "\r\n",
};

#define TYPE Str
#include "generic/option.h"
#include "generic/vector.h"
#undef TYPE

/// Convert a C string to a `Str`
/// SAFETY: `cstr` has to be valid for the entire lifetime of the returned `Str`
[[maybe_unused]] static Str str_from_cstr(const char* NONNULL cstr)
{
	size_t length = strlen(cstr);
	return (Str){
		.length = length,
		.data = cstr,
	};
}

/// Convert a `StrBuffer` to a `Str`
/// SAFETY: the buffer may not be modified for the lifetime of the returned `Str`
/// 	(including appending, as the underlying memory may be reallocated and freed)
[[maybe_unused]] static Str str_from_strbuffer(StrBuffer buffer)
{
	return (Str){
		.length = buffer.length,
		.data = buffer.data,
	};
}

/// Check for bitwise equality
[[maybe_unused]] static bool str_eq(Str a, Str b)
{
	if (a.length != b.length)
		return false;

	for (size_t i = 0; i < a.length; i++)
	{
		// SAFETY: if a.length > 0 a.data / b.data is nonnull
		if (a.data[i] != b.data[i])
			return false;
	}

	return true;
}

/// Case insensitive equality check, ASCII only
[[maybe_unused]] static bool str_eq_ci(Str a, Str b)
{
	// TODO: utf8?
	if (a.length != b.length)
		return false;

	for (size_t i = 0; i < a.length; i++)
	{
		// SAFETY: if a.length > 0 a.data / b.data is nonnull
		char ac = a.data[i];
		char bc = b.data[i];

		if (ac >= 'A' && ac <= 'Z')
			ac = 'a' + (ac - 'A');

		if (bc >= 'A' && bc <= 'Z')
			bc = 'a' + (bc - 'A');

		if (ac != bc)
			return false;
	}

	return true;
}

/// Write the `Str` to a stream
[[maybe_unused]] static size_t str_fwrite(Str self, FILE* NONNULL stream)
{
	if (self.data != nullptr)
		return fwrite(self.data, 1, self.length, stream);
	else
		return 0;
}

/*
/// Find a substring in `self`
/// Returns an index of the first occurance if any
Option_size_t str_find(Str self, Str substr)
{
	if (self.length < substr.length)
		return option_size_t_none;

	for (size_t i = 0; i < (self.length - substr.length); i++)
	{
		bool match = true;
		for (size_t j = 0; j < substr.length && i + j < self.length; j++)
		{
			if (self.data[i + j] != substr.data[i + j])
			{
				match = false;
				break;
			}
		}

		if (match)
			return option_size_t_some(i);
	}

	return option_size_t_none;
}
*/

/// Slice `self`
/// Panics if start or end is invalid
[[maybe_unused]] static Str str_slice(Str self, size_t start, size_t end)
{
	if (start > end)
		PANIC("invalid range: start > end");

	if (start >= self.length)
		PANIC("invalid range: start >= length");

	if (end > self.length)
		PANIC("invalid range: end > length");

	size_t new_length = end - start;

	return (Str){
		.data = self.data + start,
		.length = new_length,
	};
}

/// Calculate a hash of `self`
/// str_eq(a, b) implies str_hash(a) == str_hash(b)
[[maybe_unused]] static int64_t str_hash(Str self)
{
	// algorithm from http://www.cse.yorku.ca/~oz/hash.html
	
	uint64_t hash = 5381;

	for (size_t i = 0; i < self.length; i++)
		hash = ((hash << 5) + hash) + self.data[i];

	return hash;
}

/// Calculate a case insensitive hash of `self`
/// str_eq_ci(a, b) implies str_hash_ci(a) == str_hash_ci(b)
[[maybe_unused]] static int64_t str_hash_ci(Str self)
{
	// algorithm from http://www.cse.yorku.ca/~oz/hash.html
	
	uint64_t hash = 5381;

	for (size_t i = 0; i < self.length; i++)
	{
		char c = self.data[i];

		if (c >= 'A' && c <= 'Z')
			c = 'a' + (c - 'A');

		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

/// Create a `StrBuffer` from raw parts
/// SAFETY:
/// 	`capacity == 0` <=> `data == nullptr`
/// 	`length <= capacity`
/// 	`data` must be allocated with the system allocator (`malloc` / `calloc`)
/// 	and may not be freed outside of the `StrBuffer`
[[maybe_unused]] static StrBuffer strbuffer_from_raw(char* NULLABLE data, size_t capacity, size_t length)
{
	DEBUG_ASSERT((capacity == 0) == (data == nullptr));
	DEBUG_ASSERT(capacity >= length);

	return (StrBuffer) {
		.data = data,
		.capacity = capacity,
		.length = length,
	};
}

/// Clones a Str to a new StrBuffer
[[maybe_unused]] static StrBuffer strbuffer_from_str(Str str)
{
	StrBuffer buffer = vector_char_new(str.length);

	vector_char_extend(&buffer, str.data, str.length);

	return buffer;
}

/// Append a `Str` at the end of `buffer`
[[maybe_unused]] static void strbuffer_extend(StrBuffer* NONNULL buffer, Str str)
{
	vector_char_extend(buffer, str.data, str.length);
}

/// Write a decimal representation of a `size_t` to the end of `buffer`
[[maybe_unused]] static void strbuffer_write_size_t(StrBuffer* NONNULL buffer, size_t value)
{
	char num_buf[32];
	snprintf(num_buf, 32, "%zu", value);
	size_t num_len = strlen(num_buf);
	vector_char_extend(buffer, num_buf, num_len);
}

/// Write a decimal representation of a `uint16_t` to the end of `buffer`
[[maybe_unused]] static void strbuffer_write_uint16_t(StrBuffer* NONNULL buffer, uint16_t value)
{
	char num_buf[32];
	snprintf(num_buf, 32, "%" PRIu16, value);
	size_t num_len = strlen(num_buf);
	vector_char_extend(buffer, num_buf, num_len);
}

typedef Str StrCI;

// Option_StrBuffer
#define TYPE StrBuffer
#include "generic/option.h"
#undef TYPE

[[maybe_unused]] static bool wttp_p_str_eq_hashmap(const Str* NONNULL a, const Str* NONNULL b)
{
	return str_eq(*a, *b);
}

[[maybe_unused]] static int64_t wttp_p_str_hash_hashmap(const Str* NONNULL self)
{
	return str_hash(*self);
}

// Hashmap_StrCI_StrBuffer (headers)
#define Str_EQ wttp_p_str_eq_hashmap
#define Str_HASH wttp_p_str_hash_hashmap
#define TYPE_KEY Str
#define TYPE_VALUE Str
#include "generic/hashmap.h"
#undef StrCI_EQ
#undef StrCI_HASH
#undef TYPE_KEY
#undef TYPE_VALUE

[[maybe_unused]] static bool wttp_p_str_eq_ci_hashmap(const Str* NONNULL a, const Str* NONNULL b)
{
	return str_eq_ci(*a, *b);
}

[[maybe_unused]] static int64_t wttp_p_str_hash_ci_hashmap(const Str* NONNULL self)
{
	return str_hash_ci(*self);
}

// Hashmap_StrCI_StrBuffer (headers)
#define StrCI_EQ wttp_p_str_eq_ci_hashmap
#define StrCI_HASH wttp_p_str_hash_ci_hashmap
#define TYPE_KEY StrCI
#define TYPE_VALUE StrBuffer
#include "generic/hashmap.h"
#undef StrCI_EQ
#undef StrCI_HASH
#undef TYPE_KEY
#undef TYPE_VALUE

// Option_Hashmap_StrCI_StrBuffers
#define TYPE Hashmap_StrCI_StrBuffer
#include "generic/option.h"
#undef TYPE

/// Free a header collection (freeing all values and the hashmap itself)
[[maybe_unused]] static void free_headers(Hashmap_StrCI_StrBuffer* NONNULL headers)
{
	for (size_t i = 0; i < COUNTOF(headers->buckets); i++)
	{
		for (size_t j = 0; j < headers->buckets[i].length; j++)
			vector_char_free(&headers->buckets[i].data[j].value);
	}

	hashmap_StrCI_StrBuffer_free(headers);
}

#endif
