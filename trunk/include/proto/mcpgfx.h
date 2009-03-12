#ifndef PROTO_MCPGFX_H
#define PROTO_MCPGFX_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

extern struct Library	*MCPGfxBase;

#include <clib/mcpgfx_protos.h>

#if defined(__GNUC__)
	#include <inline/mcpgfx.h>
#elif defined(VBCC)
	#include <inline/mcpgfx_protos.h>
#else
	#include <pragmas/mcpgfx_pragmas.h>
#endif /* __VBCC__ */

#endif /* PROTO_MCPGFX_H */

