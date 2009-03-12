#ifndef PROTO_SCALOSMENUPLUGIN_H
#define PROTO_SCALOSMENUPLUGIN_H

#include <exec/types.h>
extern struct Library *ScalosMenuPluginBase ;
#ifdef __amigaos4__
#include <interfaces/scalosmenuplugin.h>
extern struct ScalosMenuPluginIFace *IScalosMenuPlugin;
#endif

#include <clib/scalosmenuplugin_protos.h>

#ifdef __amigaos4__
	#ifdef __USE_INLINE__
		#include <inline4/scalosmenuplugin.h>
	#endif
#elif defined(__GNUC__)
	#include <inline/scalosmenuplugin.h>
#elif defined(VBCC)
	#include <inline/scalosmenuplugin_protos.h>
#else
	#include <pragmas/scalosmenuplugin_pragmas.h>
#endif

#endif
