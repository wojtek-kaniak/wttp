#include <stdio.h>
#include "http.h"
#include "types.h"
#define TYPE int
#include "generic/vector.h"
#undef TYPE

#define TYPE_OK int
#define TYPE_ERR char
#include "generic/result.h"
#undef TYPE_OK
#undef TYPE_ERR

int main()
{
	Result_int_char res = result_int_char_ok(2);
	char err = result_int_char_unwrap_err(res);

	puts("test");
	Vector_int vec = vector_int_new(0);
	vector_int_add(&vec, 5);
	printf("%zu %zu %d\n", vec.capacity, vec.length, vec.data[0]);

	Str method = option_Str_unwrap(http_method_to_str(option_HttpMethod_unwrap(http_method_from_str(str_from_cstr("HEAD")))));
	str_fwrite(method, stdout);
	putc('\n', stdout);
	// fwrite(method.data, 1, method.length, stdout);
}
