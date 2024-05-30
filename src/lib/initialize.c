#include "initialize.h"
#include "mime.h"
#include "random.h"

static bool initialized;
void initialize_global()
{
	assert(!initialized);
	initialized = true;
	mime_initialize();
}

static thread_local bool thread_initialized;
void initialize_thread()
{
	assert(!thread_initialized);
	thread_initialized = true;
	random_init_thread();
}
