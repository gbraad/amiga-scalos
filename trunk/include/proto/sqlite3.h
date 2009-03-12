#ifndef PROTO_SQLITE3_H
#define PROTO_SQLITE3_H

#include <exec/types.h>
#include <sqlite3.h>
extern struct Library *SQLite3Base;
#ifdef __amigaos4__
#include <interfaces/sqlite3.h>
extern struct SQLite3IFace *ISQLite3;
#endif

#include <clib/sqlite3_protos.h>

#ifdef __amigaos4__
	#ifdef __USE_INLINE__
		#include <inline4/sqlite3.h>
	#endif
#elif defined(__GNUC__)
	#include <inline/sqlite3.h>
#elif defined(VBCC)
	#include <inline/sqlite3_protos.h>
#else
	#include <pragmas/sqlite3_pragmas.h>
#endif

#endif
