// Dos64.c
// $Date$

/***********************************************************************
*
* Function:
*        64-bit File operations
*
***********************************************************************/

#include <exec/types.h>
#include <dos/exall.h>
#include <dos/dosextens.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define	__USE_SYSBASE

#include <proto/dos.h>

#include "int64.h"

/***********************************************************************
*        Public Routines
***********************************************************************/


LONG ScalosExamine64(BPTR lock, struct FileInfoBlock *fib)
{
#if defined(__MORPHOS__) && defined(ACTION_EXAMINE_NEXT64)
	if (DOSBase->dl_lib.lib_Version >= 51)
		return Examine64(lock, fib, NULL);
#endif /* __MORPHOS__ */
	return Examine(lock, fib);
}


LONG ScalosExNext64(BPTR lock, struct FileInfoBlock *fib)
{
#if defined(__MORPHOS__) && defined(ACTION_EXAMINE_NEXT64)
	if (DOSBase->dl_lib.lib_Version >= 51)
		return ExNext64(lock, fib, NULL);
#endif /* __MORPHOS__ */
	return ExNext(lock, fib);
}


ULONG64 ScalosFibSize64(const struct FileInfoBlock *fib)
{
#if defined(__MORPHOS__) && defined(ACTION_EXAMINE_NEXT64)
	if (DOSBase->dl_lib.lib_Version >= 51)
	{
		ULONG64 Size = (ULONG64) fib->fib_un.fib_un_ext.fib_un_ext_Size64;

		// workaround for MorphOS 2.1 sign-extenstion bug:
		// 64bit file size is negative for files with sizes between 2G and 4G
		if (0xffffffff == ULONG64_HIGH(Size))
			Size = MakeU64(ULONG64_LOW(Size));

		return Size;
	}
#endif /* __MORPHOS__ */
	return MakeU64(fib->fib_Size);
}


ULONG64 ScalosExAllSize64(const struct ExAllData *ead, ULONG edType)
{
#if defined(__MORPHOS__) && defined(ACTION_EXAMINE_NEXT64)
	if (DOSBase->dl_lib.lib_Version >= 51 && edType >= ED_SIZE64)
		return ead->ed_Size64;
#endif /* __MORPHOS__ */

	return MakeU64(ead->ed_Size);
}


void ScalosSetExAllSize64(struct ExAllData *ead, ULONG edType, ULONG64 Size)
{
#if defined(__MORPHOS__) && defined(ACTION_EXAMINE_NEXT64)
	if (DOSBase->dl_lib.lib_Version >= 51 && edType >= ED_SIZE64)
		ead->ed_Size64 = Size;
#endif /* __MORPHOS__ */
	if (edType >= ED_SIZE)
		ead->ed_Size = ULONG64_LOW(Size);
}

