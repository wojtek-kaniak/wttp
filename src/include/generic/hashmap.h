#include <stdint.h>
#include "../common.h"

#define WTTP_P_MAP_CONCAT(a, b) WTTP_P_CONCAT(a, b)
#define WTTP_P_MAP_CONCAT3(a, b, c) WTTP_P_CONCAT3(a, b, c)
#define WTTP_P_MAP_CONCAT4(a, b, c, d) WTTP_P_CONCAT4(a, b, c, d)
#define WTTP_P_MAP_CONCAT5(a, b, c, d, e) WTTP_P_CONCAT5(a, b, c, d, e)
#define WTTP_P_MAP_STRINGIFY(a) WTTP_P_STRINGIFY(a)

#if !defined (WTTP_P_HASHMAP_BUCKET_COUNT)
#define WTTP_P_HASHMAP_BUCKET_COUNT 67
#endif

typedef struct WTTP_P_MAP_CONCAT4(KeyValuePair_, TYPE_KEY, _, TYPE_VALUE)
{
	TYPE_KEY key;
	TYPE_VALUE value;
} WTTP_P_MAP_CONCAT4(KeyValuePair_, TYPE_KEY, _, TYPE_VALUE);

#define TYPE WTTP_P_MAP_CONCAT4(KeyValuePair_, TYPE_KEY, _, TYPE_VALUE)
#include "vector.h"
#undef TYPE

typedef struct WTTP_P_MAP_CONCAT4(Hashmap_, TYPE_KEY, _, TYPE_VALUE)
{
	WTTP_P_MAP_CONCAT4(Vector_KeyValuePair_, TYPE_KEY, _, TYPE_VALUE) buckets[WTTP_P_HASHMAP_BUCKET_COUNT];
	uint64_t random_seed;
} WTTP_P_MAP_CONCAT4(Hashmap_, TYPE_KEY, _, TYPE_VALUE);

// Hashmap_new
WTTP_P_MAP_CONCAT4(Hashmap_, TYPE_KEY, _, TYPE_VALUE) WTTP_P_MAP_CONCAT5(hashmap_, TYPE_KEY, _, TYPE_VALUE, _new)
	(uint64_t random_seed)
{
	return (WTTP_P_MAP_CONCAT4(Hashmap_, TYPE_KEY, _, TYPE_VALUE)) {
		.random_seed = random_seed,
		// zero initialized vector is a valid 0 capacity vector
		.buckets = {0},
	};
}

// Hashmap_insert
WTTP_P_MAP_CONCAT(Option_, TYPE_VALUE) WTTP_P_MAP_CONCAT5(hashmap_, TYPE_KEY, _, TYPE_VALUE, _insert)
	(WTTP_P_MAP_CONCAT4(Hashmap_, TYPE_KEY, _, TYPE_VALUE)* self, TYPE_KEY key, TYPE_VALUE value)
{
	uint64_t hash = WTTP_P_MAP_CONCAT(TYPE_KEY, _HASH) (&key) ^ self->random_seed;

	WTTP_P_MAP_CONCAT4(Vector_KeyValuePair_, TYPE_KEY, _, TYPE_VALUE)* bucket
		= &self->buckets[hash % WTTP_P_HASHMAP_BUCKET_COUNT];

	WTTP_P_MAP_CONCAT(Option_, TYPE_VALUE) old_value =
		WTTP_P_MAP_CONCAT3(option_, TYPE_VALUE, _none);

	for (size_t i = 0; i < bucket->length; i++)
	{
		if (WTTP_P_MAP_CONCAT(TYPE_KEY, _EQ)(&key, &bucket->data[i].key))
		{
			old_value = WTTP_P_MAP_CONCAT3(option_, TYPE_VALUE, _some)(bucket->data[i].value);
			bucket->data[i].key = key;
			bucket->data[i].value = value;
			break;
		}
	}

	if (!old_value.has_value)
	{
		WTTP_P_MAP_CONCAT4(KeyValuePair_, TYPE_KEY, _, TYPE_VALUE) kvp = { .key = key, .value = value };
		WTTP_P_MAP_CONCAT5(vector_KeyValuePair_, TYPE_KEY, _, TYPE_VALUE, _add)(bucket, kvp);
	}

	return old_value;
}

// Hashmap_get
// The returned reference is valid as long as the hashmap is not modified
TYPE_VALUE* WTTP_P_MAP_CONCAT5(hashmap_, TYPE_KEY, _, TYPE_VALUE, _get)
	(const WTTP_P_MAP_CONCAT4(Hashmap_, TYPE_KEY, _, TYPE_VALUE)* self, TYPE_KEY* key)
{
	uint64_t hash = WTTP_P_MAP_CONCAT(TYPE_KEY, _HASH) (key) ^ self->random_seed;


	const WTTP_P_MAP_CONCAT4(Vector_KeyValuePair_, TYPE_KEY, _, TYPE_VALUE)* bucket
		= &self->buckets[hash % WTTP_P_HASHMAP_BUCKET_COUNT];

	for (size_t i = 0; i < bucket->length; i++)
	{
		if (WTTP_P_MAP_CONCAT(TYPE_KEY, _EQ)(key, &bucket->data[i].key))
			return &bucket->data[i].value;
	}

	return nullptr;
}

// Hashmap_free
void WTTP_P_MAP_CONCAT5(hashmap_, TYPE_KEY, _, TYPE_VALUE, _free)
	(WTTP_P_MAP_CONCAT4(Hashmap_, TYPE_KEY, _, TYPE_VALUE)* self)
{
	for (size_t i = 0; i < WTTP_P_HASHMAP_BUCKET_COUNT; i++)
		WTTP_P_MAP_CONCAT5(vector_KeyValuePair_, TYPE_KEY, _, TYPE_VALUE, _free)(&self->buckets[i]);
}
#undef WTTP_P_MAP_CONCAT
#undef WTTP_P_MAP_CONCAT3
#undef WTTP_P_MAP_CONCAT4
#undef WTTP_P_MAP_CONCAT5
#undef WTTP_P_MAP_STRINGIFY
