// Patches-classic.c
// $Date$
// $Revision$


#include <exec/types.h>
#include <graphics/rastport.h>
#include <utility/hooks.h>
#include <intuition/gadgetclass.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <workbench/icon.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>
#include <dos/dostags.h>
#include <guigfx/guigfx.h>

#define	__USE_SYSBASE

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/layers.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/icon.h>
#include <proto/iconobject.h>
#include <proto/guigfx.h>
#include <proto/datatypes.h>
#include <proto/gadtools.h>
#include "debug.h"
#include <proto/scalos.h>

#include <clib/alib_protos.h>

#include <defs.h>
#include <datatypes/iconobject.h>
#include <scalos/palette.h>
#include <scalos/pattern.h>
#include <scalos/scalos.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scalos_structures.h"
#include "functions.h"
#include "Variables.h"
#include "Patches.h"

//----------------------------------------------------------------------------

// local data structures

#define	PPATCH_NEWFUNC(ple)	 ple->pe_NewFunction

struct PatchEntry
	{
	struct Library **pe_Library;
	ULONG pe_MinVersion;
	LONG pe_LibOffset;
	APTR pe_NewFunction;
	APTR *pe_OldFunction;
	};

//----------------------------------------------------------------------------

// local functions

static void SetPatches(const struct PatchEntry *PatchList);
static BOOL RemovePatches(const struct PatchEntry *PatchList);

//----------------------------------------------------------------------------

// public data items

//----------------------------------------------------------------------------

// local data items

//----------------------------------------------------------------------------

#define LVOCloseWorkBench 	-13  * LIB_VECTSIZE
#define LVOOpenWorkBench 	-35  * LIB_VECTSIZE
#define LVOOpenScreenTagList 	-102 * LIB_VECTSIZE
#define LVOOpenWindowTagList	-101 * LIB_VECTSIZE

#define LVOUpdateWorkbench	-5   * LIB_VECTSIZE	/* private1 */
#define LVOwbPrivate2 	        -6   * LIB_VECTSIZE
#define LVOAddAppWindowA 	-8   * LIB_VECTSIZE
#define LVORemoveAppWindow 	-9   * LIB_VECTSIZE
#define LVOAddAppIconA 		-10  * LIB_VECTSIZE
#define LVORemoveAppIcon 	-11  * LIB_VECTSIZE
#define LVOAddAppMenuItemA 	-12  * LIB_VECTSIZE
#define LVORemoveAppMenuItem 	-13  * LIB_VECTSIZE
#define LVOSetBackFill		-14  * LIB_VECTSIZE
#define LVOWBInfo		-15  * LIB_VECTSIZE

#define LVOPutDiskObject 	-14  * LIB_VECTSIZE
#define LVODeleteDiskObject 	-23  * LIB_VECTSIZE
#define LVOPutIconTagList	-31  * LIB_VECTSIZE

#define LVODeleteFile		-12  * LIB_VECTSIZE
#define LVORename 		-13  * LIB_VECTSIZE
#define LVOOpen			-5   * LIB_VECTSIZE
#define LVOClose		-6   * LIB_VECTSIZE
#define LVOCreateDir		-20  * LIB_VECTSIZE

//----------------------------------------------------------------------------

static const struct PatchEntry EmulationPatches[] =
	{
#ifdef TEST_OPENWINDOWTAGLIST
	{ (struct Library **) &IntuitionBase, 39,	LVOOpenWindowTagList,	AROS_SLIB_ENTRY(sca_OpenWindowTagList, iBase),	(APTR) &OldOpenWindowTagList },
#endif /* TEST_OPENWINDOWTAGLIST */
	{ (struct Library **) &IntuitionBase, 39,	LVOCloseWorkBench,	AROS_SLIB_ENTRY(sca_CloseWorkBench, iBase),	(APTR) &OldCloseWB },
	{ (struct Library **) &IntuitionBase, 39,	LVOOpenWorkBench,	AROS_SLIB_ENTRY(sca_OpenWorkBench, iBase),	(APTR) &OldOpenWB },
	{ (struct Library **) &IntuitionBase, 39,	LVOOpenScreenTagList,	AROS_SLIB_ENTRY(sca_OpenScreenTagList, iBase),	(APTR) &OldOpenScreenTagList },
	{ (struct Library **) &WorkbenchBase, 39,	LVOAddAppIconA,		AROS_SLIB_ENTRY(sca_AddAppIconA, wbBase),	(APTR) &OldAddAppIconA },
	{ (struct Library **) &WorkbenchBase, 39,	LVORemoveAppIcon,	AROS_SLIB_ENTRY(sca_RemoveAppIcon, wbBase),	(APTR) &OldRemoveAppIcon },
	{ (struct Library **) &WorkbenchBase, 39,	LVOAddAppWindowA,	AROS_SLIB_ENTRY(sca_AddAppWindowA, wbBase),	(APTR) &OldAddAppWindowA },
	{ (struct Library **) &WorkbenchBase, 39,	LVORemoveAppWindow,	AROS_SLIB_ENTRY(sca_RemoveAppWindow, wbBase),	(APTR) &OldRemoveAppWindow },
	{ (struct Library **) &WorkbenchBase, 39,	LVOAddAppMenuItemA,	AROS_SLIB_ENTRY(sca_AddAppMenuItemA, wbBase),	(APTR) &OldAppAppMenuItemA },
	{ (struct Library **) &WorkbenchBase, 39,	LVORemoveAppMenuItem,	AROS_SLIB_ENTRY(sca_RemoveAppMenuItem, wbBase),	(APTR) &OldRemoveAppMenuItem },
//	{ (struct Library **) &WorkbenchBase, 39,	LVOwbPrivate2,		AROS_SLIB_ENTRY(dummy,sca_GetLocString),	(APTR) &OldwbPrivate2 },
	{ NULL, 			      0,	0, 			NULL, 						NULL },
	};
static const struct PatchEntry StandardPatches[] =
	{
	{ (struct Library **) &IconBase, 39, 	LVOPutDiskObject,	AROS_SLIB_ENTRY(sca_PutDiskObject, IconBase),		(APTR) &OldPutDiskObject },
	{ (struct Library **) &IconBase, 39, 	LVODeleteDiskObject,	AROS_SLIB_ENTRY(sca_DeleteDiskObject, IconBase),	(APTR) &OldDeleteDiskObject },
	{ (struct Library **) &IconBase, 44, 	LVOPutIconTagList,	AROS_SLIB_ENTRY(sca_PutIconTagList, IconBase),		(APTR) &OldPutIconTagList},

	{ (struct Library **) &DOSBase,	 39,	LVODeleteFile,		AROS_SLIB_ENTRY(sca_DeleteFile, DOSBase),		(APTR) &OldDeleteFile },
	{ (struct Library **) &DOSBase,	 39,	LVORename,		AROS_SLIB_ENTRY(sca_Rename, DOSBase),			(APTR) &OldRename },
	{ (struct Library **) &DOSBase,	 39,	LVOCreateDir,		AROS_SLIB_ENTRY(sca_CreateDir, DOSBase),		(APTR) &OldCreateDir },
	{ (struct Library **) &DOSBase,	 39,	LVOOpen,		AROS_SLIB_ENTRY(sca_Open, DOSBase),			(APTR) &OldOpen },
	{ (struct Library **) &DOSBase,	 39,	LVOClose,		AROS_SLIB_ENTRY(sca_Close, DOSBase),			(APTR) &OldClose },
	{ NULL, 			 0, 	0,			NULL,							NULL },
	};
static const struct PatchEntry HardEmulationPatches[] =
	{
	{ (struct Library **) &WorkbenchBase, 39, 	LVOUpdateWorkbench,	AROS_SLIB_ENTRY(sca_UpdateWorkbench, wbBase),	(APTR) &OldUpdateWorkbench },
	{ (struct Library **) &WorkbenchBase, 39, 	LVOSetBackFill,		AROS_SLIB_ENTRY(sca_SetBackFill, wbBase),	(APTR) &OldSetBackFill },
	{ (struct Library **) &WorkbenchBase, 39, 	LVOWBInfo,		AROS_SLIB_ENTRY(sca_WBInfo, wbBase),		(APTR) &OldWBInfo },
	{ NULL, 			      0,	0, 			NULL,						NULL },
	};

//----------------------------------------------------------------------------

void SetAllPatches(struct MainTask *mt)
{
	PatchInit();

	SetPatches(StandardPatches);

	if (mt->emulation)
		{
		if (CurrentPrefs.pref_HardEmulation)
			SetPatches(HardEmulationPatches);

		SetPatches(EmulationPatches);
		}

}


ULONG RemoveAllPatches(struct MainTask *mt)
{
	if (!RemovePatches(StandardPatches))
		return FALSE;

	if (mt->emulation)
		{
		if (!RemovePatches(EmulationPatches))
			{
			SetPatches(StandardPatches);
			return FALSE;
			}

		if (CurrentPrefs.pref_HardEmulation)
			{
			if (!RemovePatches(HardEmulationPatches))
				{
				SetPatches(EmulationPatches);
				SetPatches(StandardPatches);
				}
			}

		}

	PatchCleanup();

	return TRUE;
}


static void SetPatches(const struct PatchEntry *PatchList)
{
	Forbid();

	d1(KPrintF("%s/%s/%ld: PatchList=%08lx\n", __FILE__, __FUNC__, __LINE__, PatchList));

	while (PatchList->pe_Library)
		{
		if (NULL == *PatchList->pe_OldFunction 
			&& (*PatchList->pe_Library)->lib_Version >= PatchList->pe_MinVersion)
			{
			*PatchList->pe_OldFunction = SetFunction(*PatchList->pe_Library, 
				PatchList->pe_LibOffset, PPATCH_NEWFUNC(PatchList));

			d1(KPrintF("%s/%s/%ld: Library=%08lx  LVO=%ld  NewFunc=%08lx  OldFunc=%08lx\n", \
				__FILE__, __FUNC__, __LINE__, *PatchList->pe_Library, \
				PatchList->pe_LibOffset, PPATCH_NEWFUNC(PatchList), \
				*PatchList->pe_OldFunction));
			}

		PatchList++;
		}

//	CacheClearU();
	Permit();
}


static BOOL RemovePatches(const struct PatchEntry *PatchList)
{
	const struct PatchEntry *pl = PatchList;
	BOOL Success = TRUE;

	d1(KPrintF("%s/%s/%ld: PatchList=%08lx\n", __FILE__, __FUNC__, __LINE__, PatchList));

	Forbid();

	while (pl->pe_Library)
		{
		if (pl->pe_OldFunction)
			{
			APTR patchedFunction;

			patchedFunction = SetFunction(*pl->pe_Library,
				pl->pe_LibOffset, *pl->pe_OldFunction);
			*pl->pe_OldFunction = NULL;

			d1(KPrintF("%s/%s/%ld: Library=%08lx  LVO=%ld  NewFunc=%08lx  patchedFunc=%08lx\n", \
				__FILE__, __FUNC__, __LINE__, *PatchList->pe_Library, \
				PatchList->pe_LibOffset, PPATCH_NEWFUNC(pl), patchedFunction));
			if (patchedFunction != PPATCH_NEWFUNC(pl))
				{
				*pl->pe_OldFunction = SetFunction(*pl->pe_Library,
					pl->pe_LibOffset, patchedFunction);

				Success = FALSE;
				}
			}

		pl++;
		}

//	CacheClearU();
	Permit();

	d1(KPrintF("%s/%s/%ld: Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	if (!Success)
		SetPatches(PatchList);

	return Success;
}

