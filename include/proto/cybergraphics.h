#ifndef PROTO_CYBERGRAPHICS_H
#define PROTO_CYBERGRAPHICS_H 1
#include <exec/types.h>
extern struct Library *CyberGfxBase;
#ifdef __amigaos4__
#include <interfaces/cybergraphics.h>
extern struct CyberGfxIFace *ICyberGfx;
#endif /* __amigaos4__ */

#include <clib/cybergraphics_protos.h>

#ifdef __amigaos4__
	#ifdef __USE_INLINE__
		#include <inline4/cybergraphics.h>
	#endif
#elif defined(__GNUC__)
	#include <inline/cybergraphics.h>
#elif defined(VBCC)
	#include <inline/cybergraphics_protos.h>
#else
	#include <pragmas/cybergraphics_pragmas.h>
#endif

#endif /* !PROTO_CYBERGRAPHICS_H */
