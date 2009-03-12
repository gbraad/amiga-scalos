#ifndef PROTO_SCALOS_H
#define PROTO_SCALOS_H

#include <exec/types.h>
#include <scalos/scalos.h>
#include <scalos/scalosgfx.h>
extern struct ScalosBase *ScalosBase ;
#ifdef __amigaos4__
#include <interfaces/scalos.h>
extern struct ScalosIFace *IScalos;
#endif

#include <clib/scalos_protos.h>

#ifdef __amigaos4__
	#ifdef __USE_INLINE__
		#include <inline4/scalos.h>
	#endif
#elif defined(__GNUC__)
	#include <inline/scalos.h>
#elif defined(VBCC)
	#include <inline/scalos_protos.h>
#else
	#include <pragmas/scalos_pragmas.h>
#endif

#endif
