// scalosgfx-classic.c
// $Date$
// $Revision$


#include <exec/types.h>
#include <exec/initializers.h>

#include <exec/resident.h>

#include <aros/libcall.h>

#include <proto/exec.h>

#include "scalosgfx.h"

//----------------------------------------------------------------------------
// Standard library functions

static AROS_UFP3 (struct Library *, Initlib,
		  AROS_UFPA(struct Library *, libbase, D0),
		  AROS_UFPA(struct SegList *, seglist, A0),
		  AROS_UFPA(struct ExecBase *, sysbase, A6)
);
static AROS_LD1 (struct Library *, Openlib,
		 AROS_LPA (__unused ULONG, version, D0),
		 struct Library *, libbase, 1, ScalosGfx
);
static AROS_LD0 (struct SegList *, Closelib,
		 struct Library *, base, 2, ScalosGfx
);
static AROS_LD1 (struct SegList *, Expungelib,
		 AROS_LPA(__unused struct Library *, __extrabase, D0),
		 struct Library *, libbase, 3, ScalosGfx
);
static AROS_LD0 (ULONG, Extfunclib,
		 __unused struct Library *, libbase, 4, ScalosGfx
);

//----------------------------------------------------------------------------

SAVEDS(LONG) ASM Libstart(void)
{
	return -1;
}

//----------------------------------------------------------------------------

/* OS3.x Library */

static APTR functable[] =
	{
	ScalosGfx_Openlib,
	ScalosGfx_Closelib,
	ScalosGfx_Expungelib,
	ScalosGfx_Extfunclib,
	ScalosGfxBase_LIBScalosGfxCreateEmptySAC,
	ScalosGfxBase_LIBScalosGfxCreateSAC,
	ScalosGfxBase_LIBScalosGfxFreeSAC,
	ScalosGfxBase_LIBScalosGfxCreateARGB,
	ScalosGfxBase_LIBScalosGfxFreeARGB,
	ScalosGfxBase_LIBScalosGfxARGBSetAlpha,
	ScalosGfxBase_LIBScalosGfxARGBSetAlphaMask,
	ScalosGfxBase_LIBScalosGfxCreateARGBFromBitMap,
	ScalosGfxBase_LIBScalosGfxFillARGBFromBitMap,
	ScalosGfxBase_LIBScalosGfxWriteARGBToBitMap,
	ScalosGfxBase_LIBScalosGfxMedianCut,
	ScalosGfxBase_LIBScalosGfxScaleARGBArray,
	ScalosGfxBase_LIBScalosGfxScaleBitMap,
	ScalosGfxBase_LIBScalosGfxCalculateScaleAspect,
	ScalosGfxBase_LIBScalosGfxBlitARGB,
	ScalosGfxBase_LIBScalosGfxFillRectARGB,
	ScalosGfxBase_LIBScalosGfxSetARGB,
	ScalosGfxBase_LIBScalosGfxNewColorMap,
	ScalosGfxBase_LIBScalosGfxARGBRectMult,
	ScalosGfxBase_LIBScalosGfxBlitARGBAlpha,
	ScalosGfxBase_LIBScalosGfxBlitARGBAlphaTagList,
	ScalosGfxBase_LIBScalosGfxBlitIcon,
	ScalosGfxBase_LIBScalosGfxDrawGradient,
	ScalosGfxBase_LIBScalosGfxDrawGradientRastPort,
	ScalosGfxBase_LIBScalosGfxDrawLine,
	ScalosGfxBase_LIBScalosGfxDrawLineRastPort,
	ScalosGfxBase_LIBScalosGfxDrawEllipse,
	ScalosGfxBase_LIBScalosGfxDrawEllipseRastPort,
	(APTR) -1
	};

/* Init table used in library initialization. */
static ULONG inittab[] =
	{
	sizeof(struct ScalosGfxBase),
	(ULONG) functable,
	0,
	(ULONG) Initlib
	};


/* The ROM tag */
struct Resident ALIGNED romtag =
	{
	RTC_MATCHWORD,
	&romtag,
	&romtag + 1,
	RTF_AUTOINIT,
	LIB_VERSION,
	NT_LIBRARY,
	0,
	libName,
	libIdString,
	inittab
	};


//----------------------------------------------------------------------------

static AROS_UFH3(struct Library *, Initlib,
		 AROS_UFHA(struct Library *, libbase, D0),
		 AROS_UFHA(struct SegList *, seglist, A0),
		 AROS_UFHA(struct ExecBase *, sysbase, A6)
)
{
	AROS_USERFUNC_INIT

	struct ScalosGfxBase *ScalosGfxLibBase = (struct ScalosGfxBase *) libbase;

	/* store pointer to execbase for global access */
	SysBase = sysbase;

	ScalosGfxLibBase->sgb_LibNode.lib_Revision = LIB_REVISION;
	ScalosGfxLibBase->sgb_SegList = seglist;

	ScalosGfxLibBase->sgb_Initialized = FALSE;

	if (!ScalosGfxInit(ScalosGfxLibBase))
		{
		ScalosGfx_Expungelib(NULL, &ScalosGfxLibBase->sgb_LibNode);
		ScalosGfxLibBase = NULL;
		}

	return ScalosGfxLibBase ? &ScalosGfxLibBase->sgb_LibNode : NULL;

	AROS_USERFUNC_EXIT
}


static AROS_LH1(struct Library *, Openlib,
		AROS_LHA(__unused ULONG, version, D0),
		struct Library *, libbase, 1, ScalosGfx
)
{
	AROS_LIBFUNC_INIT

	struct ScalosGfxBase *ScalosGfxLibBase = (struct ScalosGfxBase *) libbase;

	ScalosGfxLibBase->sgb_LibNode.lib_OpenCnt++;
	ScalosGfxLibBase->sgb_LibNode.lib_Flags &= ~LIBF_DELEXP;

	if (!ScalosGfxLibBase->sgb_Initialized)
		{
		if (!ScalosGfxOpen(ScalosGfxLibBase))
			{
			ScalosGfx_Closelib(&ScalosGfxLibBase->sgb_LibNode);
			return NULL;
			}

		ScalosGfxLibBase->sgb_Initialized = TRUE;
		}

	return &ScalosGfxLibBase->sgb_LibNode;

	AROS_LIBFUNC_EXIT
}


static AROS_LH0(struct SegList *, Closelib,
		struct Library *, libbase, 2, ScalosGfx
)
{
	AROS_LIBFUNC_INIT

	struct ScalosGfxBase *ScalosGfxLibBase = (struct ScalosGfxBase *) libbase;

	ScalosGfxLibBase->sgb_LibNode.lib_OpenCnt--;

	if (0 == ScalosGfxLibBase->sgb_LibNode.lib_OpenCnt)
		{
		if (ScalosGfxLibBase->sgb_LibNode.lib_Flags & LIBF_DELEXP)
			{
			return ScalosGfx_Expungelib(NULL, &ScalosGfxLibBase->sgb_LibNode);
			}
		}

	return NULL;

	AROS_LIBFUNC_EXIT
}


static AROS_LH1(struct SegList *, Expungelib,
		AROS_LHA(__unused struct Library *, __extrabase, D0),
		struct Library *, libbase, 3, ScalosGfx
)
{
	AROS_LIBFUNC_INIT

	struct ScalosGfxBase *ScalosGfxLibBase = (struct ScalosGfxBase *) libbase;

	if (0 == ScalosGfxLibBase->sgb_LibNode.lib_OpenCnt)
		{
		ULONG size = ScalosGfxLibBase->sgb_LibNode.lib_NegSize + ScalosGfxLibBase->sgb_LibNode.lib_PosSize;
		UBYTE *ptr = (UBYTE *) ScalosGfxLibBase - ScalosGfxLibBase->sgb_LibNode.lib_NegSize;
		struct SegList *libseglist = ScalosGfxLibBase->sgb_SegList;

		Remove((struct Node *) ScalosGfxLibBase);
		ScalosGfxCleanup(ScalosGfxLibBase);
		FreeMem(ptr,size);

		return libseglist;
		}

	ScalosGfxLibBase->sgb_LibNode.lib_Flags |= LIBF_DELEXP;

	return NULL;

	AROS_LIBFUNC_EXIT
}


static AROS_LH0(ULONG, Extfunclib,
		__unused struct Library *, libbase, 4, ScalosGfx
)
{
	AROS_LIBFUNC_INIT

	return 0;

	AROS_LIBFUNC_EXIT
}

//----------------------------------------------------------------------------
