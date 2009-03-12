#ifndef PROTO_GUIGFX_H
#define PROTO_GUIGFX_H
#include <exec/types.h>
extern struct Library *GuiGFXBase;

#ifdef __amigaos4__
#include <interfaces/guigfx.h>
extern struct GuiGFXIFace *IGuiGFX;
#endif

#include <clib/guigfx_protos.h>

#ifdef __amigaos4__
	#ifdef __USE_INLINE__
		#include <inline4/guigfx.h>
	#endif
#elif defined(__GNUC__)
	#include <inline/guigfx.h>
#elif defined(VBCC)
	#include <inline/guigfx_protos.h>
#else
	#include <pragmas/guigfx_pragmas.h>
#endif

#endif
