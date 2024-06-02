// File extension to MIME type associations

#if !defined (INCL_MIME_H)
#define INCL_MIME_H

#include "types.h"

/// Initialize and register default MIME associations
void mime_initialize();

/// Register a new MIME association, overwriting a previous entry if present
void mime_register_association(Str file_extension, Str mime);

/// Get the associated MIME type from a file extension (without the leading period)
/// Lifetimes: `file_extension` has to be valid only for the `mime` invocation,
/// 	the returned Str has a static lifetime
Option_Str mime(Str file_extension);

#endif
