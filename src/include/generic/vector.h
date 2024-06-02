#include <stdlib.h>
#include <string.h>
#include "../common.h"

#define WTTP_P_VEC_CONCAT(a, b) WTTP_P_CONCAT(a, b)
#define WTTP_P_VEC_CONCAT3(a, b, c) WTTP_P_CONCAT3(a, b, c)

typedef struct WTTP_P_VEC_CONCAT(Vector_, TYPE) {
	size_t length;
	size_t capacity;
	TYPE* NULLABLE data;
} WTTP_P_VEC_CONCAT(Vector_, TYPE);

// vector_new
[[maybe_unused]] static WTTP_P_VEC_CONCAT(Vector_, TYPE) WTTP_P_VEC_CONCAT3(vector_, TYPE, _new)
	(size_t initial_capacity)
{
	TYPE* data = nullptr;

	if (initial_capacity != 0)
	{
		data = malloc(initial_capacity * sizeof(TYPE));
		
		if (data == nullptr)
			ALLOC_FAIL();
	}

	return (WTTP_P_VEC_CONCAT(Vector_, TYPE)) {
		.length = 0,
		.capacity = initial_capacity,
		.data = data,
	};
}

// vector_set_capacity
[[maybe_unused]] static void WTTP_P_VEC_CONCAT3(vector_, TYPE, _set_capacity)
	(WTTP_P_VEC_CONCAT(Vector_, TYPE)* NONNULL self, size_t new_capacity)
{
	if (new_capacity == 0)
	{
		free(self->data);
		self->data = nullptr;
	}
	else
	{
		TYPE* new_alloc = realloc(self->data, new_capacity * sizeof(TYPE));

		if (new_alloc == nullptr)
			ALLOC_FAIL();

		self->data = new_alloc;
	}

	self->capacity = new_capacity;

	if (self->length > self->capacity)
		self->length = self->capacity;
}

// vector_set_length
[[maybe_unused]] static void WTTP_P_VEC_CONCAT3(vector_, TYPE, _set_length)
	(WTTP_P_VEC_CONCAT(Vector_, TYPE)* NONNULL self, size_t new_length)
{
	const size_t DEFAULT_MIN_CAPACITY = 4;

	if (self->capacity < new_length)
	{
		size_t new_capacity = self->capacity == 0
			? DEFAULT_MIN_CAPACITY
			: 2 * self->capacity;

		if (new_capacity < new_length)
			new_capacity = new_length;

		WTTP_P_VEC_CONCAT3(vector_, TYPE, _set_capacity) (self, new_capacity);
	}

	self->length = new_length;
}

// vector_extend
[[maybe_unused]] static void WTTP_P_VEC_CONCAT3(vector_, TYPE, _extend)
	(WTTP_P_VEC_CONCAT(Vector_, TYPE)* NONNULL self, const TYPE* NONNULL elements, size_t count)
{
	// memcpy(null, _, 0) is UB
	if (count == 0)
		return;

	size_t new_length = self->length + count;
	size_t current_length = self->length;

	WTTP_P_VEC_CONCAT3(vector_, TYPE, _set_length) (self, new_length);
	
	memcpy(self->data + current_length, elements, count * sizeof(TYPE));
}

// vector_add
[[maybe_unused]] static void WTTP_P_VEC_CONCAT3(vector_, TYPE, _add)
	(WTTP_P_VEC_CONCAT(Vector_, TYPE)* NONNULL self, TYPE value)
{
	WTTP_P_VEC_CONCAT3(vector_, TYPE, _extend) (self, &value, 1);
}

// vector_free
[[maybe_unused]] static void WTTP_P_VEC_CONCAT3(vector_, TYPE, _free)
	(WTTP_P_VEC_CONCAT(Vector_, TYPE)* NONNULL self)
{
	free(self->data);
	self->data = nullptr;
	self->length = 0;
	self->capacity = 0;
}

#undef WTTP_P_VEC_CONCAT
#undef WTTP_P_VEC_CONCAT3
