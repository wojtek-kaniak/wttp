#include "../common.h"

#define WTTP_P_RES_CONCAT(a, b) WTTP_P_CONCAT(a, b)
#define WTTP_P_RES_CONCAT3(a, b, c) WTTP_P_CONCAT3(a, b, c)
#define WTTP_P_RES_CONCAT4(a, b, c, d) WTTP_P_CONCAT4(a, b, c, d)
#define WTTP_P_RES_CONCAT5(a, b, c, d, e) WTTP_P_CONCAT5(a, b, c, d, e)
#define WTTP_P_RES_STRINGIFY(a) WTTP_P_STRINGIFY(a)

typedef struct WTTP_P_RES_CONCAT4(Result_, TYPE_OK, _, TYPE_ERR)
{
	bool is_ok;
	union {
		TYPE_OK ok;
		TYPE_ERR err;
	};
} WTTP_P_RES_CONCAT4(Result_, TYPE_OK, _, TYPE_ERR);

// Result_ok
[[maybe_unused]] static WTTP_P_RES_CONCAT4(Result_, TYPE_OK, _, TYPE_ERR) WTTP_P_RES_CONCAT5(result_, TYPE_OK, _, TYPE_ERR, _ok)
	(TYPE_OK value)
{
	return (WTTP_P_RES_CONCAT4(Result_, TYPE_OK, _, TYPE_ERR)) {
		.is_ok = true,
		.ok = value,
	};
}

// Result_err
[[maybe_unused]] static WTTP_P_RES_CONCAT4(Result_, TYPE_OK, _, TYPE_ERR) WTTP_P_RES_CONCAT5(result_, TYPE_OK, _, TYPE_ERR, _err)
	(TYPE_ERR error)
{
	return (WTTP_P_RES_CONCAT4(Result_, TYPE_OK, _, TYPE_ERR)) {
		.is_ok = false,
		.err = error,
	};
}

// Result_unwrap
[[maybe_unused]] static TYPE_OK WTTP_P_RES_CONCAT5(result_, TYPE_OK, _, TYPE_ERR, _unwrap)
	(WTTP_P_RES_CONCAT4(Result_, TYPE_OK, _, TYPE_ERR) self)
{
	if (self.is_ok)
		return self.ok;
	else
		PANIC("called result_" WTTP_P_RES_STRINGIFY(TYPE_OK) "_" WTTP_P_RES_STRINGIFY(TYPE_ERR) "_unwrap on err");
}

// Result_unwrap_err
[[maybe_unused]] static TYPE_ERR WTTP_P_RES_CONCAT5(result_, TYPE_OK, _, TYPE_ERR, _unwrap_err)
	(WTTP_P_RES_CONCAT4(Result_, TYPE_OK, _, TYPE_ERR) self)
{
	if (!self.is_ok)
		return self.err;
	else
		PANIC("called result_" WTTP_P_RES_STRINGIFY(TYPE_OK) "_" WTTP_P_RES_STRINGIFY(TYPE_ERR) "_unwrap_err on ok");
}

#undef WTTP_P_RES_CONCAT
#undef WTTP_P_RES_CONCAT3
#undef WTTP_P_RES_CONCAT4
#undef WTTP_P_RES_CONCAT5
#undef WTTP_P_RES_STRINGIFY
