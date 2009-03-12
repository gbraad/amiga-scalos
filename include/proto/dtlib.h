#ifndef PROTO_SCALOSPLUGIN_H
#define PROTO_SCALOSPLUGIN_H

#include <exec/types.h>
extern struct Library *DtLibBase ;

#include <clib/dtlib_protos.h>

#if defined(__GNUC__)
	#include <inline/dtlib.h>
#elif defined(VBCC)
	#include <inline/dtlib_protos.h>
#else
	#include <pragmas/dtlib_pragmas.h>
#endif

#endif
