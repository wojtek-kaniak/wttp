#if !defined (INCL_URI_H)
#define INCL_URI_H

#include "types.h"

/// Transform an absolute URI path into a filesystem path
/// Returns none on an invalid URI path
Option_StrBuffer uri_decode_absolute_path(Str uri);

#endif
