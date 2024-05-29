#include "types.h"
#include "mime.h"

static bool initialized;
static Hashmap_Str_Str associations;

void mime_initialize()
{
	if (initialized)
		return;
	
	initialized = true;

	associations = hashmap_Str_Str_new(0);

	#define ASSOC(ext, mime) \
		hashmap_Str_Str_insert(&associations, str_from_cstr(ext), str_from_cstr(mime))

	// text
	// default to utf8?
	ASSOC("txt", "text/plain");
	ASSOC("html", "text/html");
	ASSOC("css", "text/css");
	ASSOC("js", "text/javascript");
	ASSOC("mjs", "text/javascript");

	// image
	ASSOC("png", "image/png");
	ASSOC("webp", "image/webp");
	ASSOC("gif", "image/gif");
	ASSOC("jpg", "image/jpeg");
	ASSOC("jpeg", "image/jpeg");
	ASSOC("ico", "image/vnd.microsoft.icon");
	ASSOC("svg", "image/svg+xml");

	// video
	ASSOC("mp4", "video/mp4");
	ASSOC("ogv", "video/ogg");
	ASSOC("webm", "video/webm");

	// audio
	ASSOC("mp3", "audio/mpeg");
	ASSOC("oga", "audio/ogg");

	// fonts
	ASSOC("ttf", "font/ttf");
	ASSOC("otf", "font/otf");
	ASSOC("woff", "font/woff");
	ASSOC("woff2", "font/woff2");
}

void mime_register_association(Str file_extension, Str mime)
{
	assert(initialized);
	hashmap_Str_Str_insert(&associations, file_extension, mime);
}

Option_Str mime(Str file_extension)
{
	assert(initialized);

	Str* result = hashmap_Str_Str_get(&associations, &file_extension);

	return result != nullptr
		? *result
		: option_Str_none;
}
