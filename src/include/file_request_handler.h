#if !defined (INCL_FILE_REQUEST_HANDLER_H)
#define INCL_FILE_REQUEST_HANDLER_H

#include "types.h"
#include "server.h"

extern RequestHandler FILE_REQUEST_HANDLER;

/// Set the file name to normalize paths ending with a slash to (i.e. "/example/" -> "/example/index.html") 
void set_index_filename(Str filename);

// TODO: directory listings
// /// Set whether to show directory listings
// void set_show_directory_listings(bool value);

#endif
