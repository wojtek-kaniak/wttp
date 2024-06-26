#include "../common.h"

#define WTTP_P_OPT_CONCAT(a, b) WTTP_P_CONCAT(a, b)
#define WTTP_P_OPT_CONCAT3(a, b, c) WTTP_P_CONCAT3(a, b, c)
#define WTTP_P_OPT_STRINGIFY(a) WTTP_P_STRINGIFY(a)

typedef struct WTTP_P_OPT_CONCAT(Option_, TYPE)
{
	bool has_value;
	TYPE value;
} WTTP_P_OPT_CONCAT(Option_, TYPE);

// option_some
[[maybe_unused]] static WTTP_P_OPT_CONCAT(Option_, TYPE) WTTP_P_OPT_CONCAT3(option_, TYPE, _some)
	(TYPE value)
{
	return (WTTP_P_OPT_CONCAT(Option_, TYPE)) {
		.has_value = true,
		.value = value,
	};
}

// option_unwrap
[[maybe_unused]] static TYPE WTTP_P_OPT_CONCAT3(option_, TYPE, _unwrap)
	(WTTP_P_OPT_CONCAT(Option_, TYPE) self)
{
	if (self.has_value)
		return self.value;
	else
		PANIC("called option_" WTTP_P_OPT_STRINGIFY(TYPE) "_unwrap on none");
}

// option_unwrap_or
[[maybe_unused]] static TYPE WTTP_P_OPT_CONCAT3(option_, TYPE, _unwrap_or)
	(WTTP_P_OPT_CONCAT(Option_, TYPE) self, TYPE default_value)
{
	return self.has_value ? self.value : default_value;
}

[[maybe_unused]] static const WTTP_P_OPT_CONCAT(Option_, TYPE) WTTP_P_OPT_CONCAT3(option_, TYPE, _none)
= (WTTP_P_OPT_CONCAT(Option_, TYPE)) {
	.has_value = false,
};

#undef WTTP_P_OPT_CONCAT
#undef WTTP_P_OPT_CONCAT3
#undef WTTP_P_OPT_STRINGIFY
