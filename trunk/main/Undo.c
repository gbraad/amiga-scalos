// Undo.c
// $Date$
// $Revision$


#include <exec/types.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <utility/hooks.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <libraries/asl.h>
#include <dos/dos.h>

#define	__USE_SYSBASE

#include <proto/asl.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/iconobject.h>
#include <proto/utility.h>
#include <proto/wb.h>
#include "debug.h"
#include <proto/scalos.h>

#include <clib/alib_protos.h>

#include <defs.h>
#include <datatypes/iconobject.h>
#include <scalos/scalos.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

#include <DefIcons.h>

#include "scalos_structures.h"
#include "functions.h"
#include "Variables.h"

//----------------------------------------------------------------------------

// local data structures

struct UndoStep
	{
	struct Node ust_Node;
	struct List ust_EventList;	// List of events in this step
	};

//----------------------------------------------------------------------------

// local functions

static struct UndoStep *UndoCreateStep(void);
static void UndoDisposeStep(struct UndoStep *ust);
static struct UndoEvent *UndoCreateEvent(void);
static void UndoDisposeEvent(struct UndoEvent *uev);
static SAVEDS(LONG) UndoDummyFunc(struct Hook *hook, APTR object, struct UndoEvent *uev);
static void RedoCleanup(void);
static BOOL UndoAddCopyMoveEvent(struct UndoEvent *uev, struct TagItem *TagList);
static BOOL AddChangeIconPosEvent(struct UndoEvent *uev, struct TagItem *TagList);
static BOOL AddSnapshotEvent(struct UndoEvent *uev, struct TagItem *TagList);
static BOOL AddCleanupEvent(struct UndoEvent *uev, struct TagItem *TagList);
static SAVEDS(LONG) UndoCopyEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) UndoMoveEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) RedoCopyEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) RedoMoveEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) RedoCreateLinkEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static BOOL MoveIconTo(CONST_STRPTR DirName, CONST_STRPTR IconName, LONG x, LONG y);
static STRPTR UndoBuildIconName(CONST_STRPTR ObjName);
static struct internalScaWindowTask *UndoFindWindowByDir(CONST_STRPTR DirName);
static struct internalScaWindowTask *UndoFindWindowByWindowTask(const struct internalScaWindowTask *iwt);
static struct ScaIconNode *UndoFindIconByName(struct internalScaWindowTask *iwt, CONST_STRPTR IconName);
static BOOL UndoChangeIconPosEvent(struct UndoEvent *uev);
static BOOL RedoChangeIconPosEvent(struct UndoEvent *uev);
static SAVEDS(LONG) UndoDisposeCopyMoveData(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) UndoDisposeIconData(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) UndoDisposeCleanupData(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) UndoDisposeSnapshotData(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) UndoCleanupEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) UndoSnapshotEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static BOOL RedoCleanupEvent(struct UndoEvent *uev);
static const struct UndoCleanupIconEntry *UndoFindCleanupIconEntry(const struct UndoEvent *uev, const struct ScaIconNode *in);
static SAVEDS(LONG) UndoSnapshotEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) RedoSnapshotPosEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static BOOL UndoSnapshotIcon(struct internalScaWindowTask *iwt, struct ScaIconNode *in);
static BOOL UndoUnsnapshotIcon(struct internalScaWindowTask *iwt,
	struct ScaIconNode *in, BOOL SaveIcon);

//----------------------------------------------------------------------------

// local Data items

//----------------------------------------------------------------------------

// public data items

struct List globalUndoList;		// global Undo list for all file-related actions
SCALOSSEMAPHORE UndoListListSemaphore;	// Semaphore to protect globalUndoList
static ULONG UndoCount;			// Number of entries in globalUndoList

struct List globalRedoList;		// global Redo list for all file-related actions
SCALOSSEMAPHORE RedoListListSemaphore;	// Semaphore to protect globalRedoList

//----------------------------------------------------------------------------

void UndoCleanup(void)
{
	struct UndoStep *ust;

	ScalosObtainSemaphore(&UndoListListSemaphore);

	while ((ust = (struct UndoStep *) RemHead(&globalUndoList)))
		{
		UndoCount--;
		UndoDisposeStep(ust);
		}
	ScalosReleaseSemaphore(&UndoListListSemaphore);

	RedoCleanup();
}

//----------------------------------------------------------------------------

BOOL UndoAddEvent(enum ScalosUndoType type, ULONG FirstTag, ...)
{
	BOOL Success;
	struct TagItem *TagList;

	va_list args;

	va_start(args, FirstTag);

	TagList = ScalosVTagList(FirstTag, args);
	Success = UndoAddEventTagList(type, TagList);

	va_end(args);

	if (TagList)
		FreeTagItems(TagList);

	return Success;
}

//----------------------------------------------------------------------------

BOOL UndoAddEventTagList(enum ScalosUndoType type, struct TagItem *TagList)
{
	if (TagList)
		{
		struct UndoEvent *uev;

		uev = UndoCreateEvent();
		if (uev)
			{
			struct UndoStep *ustMulti;
			struct UndoStep *ust;

			ust = ustMulti = (struct UndoStep *) GetTagData(UNDOTAG_UndoMultiStep, (ULONG) NULL, TagList);
			d2(kprintf("%s/%s/%ld: ust=%08lx\n", __FILE__, __FUNC__, __LINE__, ust));
			if (NULL == ust)
				{
				ust = (struct UndoStep *) UndoBeginStep();
				}

			d2(kprintf("%s/%s/%ld: ust=%08lx\n", __FILE__, __FUNC__, __LINE__, ust));

			if (ust)
				{
				BOOL Success = FALSE;

				RedoCleanup();

				uev->uev_Type = type;

				uev->uev_CustomAddHook = (struct Hook *) GetTagData(UNDOTAG_CustomAddHook, (ULONG) NULL, TagList);

				uev->uev_NewPosX = GetTagData(UNDOTAG_IconPosX, NO_ICON_POSITION_SHORT, TagList);
				uev->uev_NewPosY = GetTagData(UNDOTAG_IconPosY, NO_ICON_POSITION_SHORT, TagList);

				d2(kprintf("%s/%s/%ld: uev=%08lx  uev_Type=%ld\n", __FILE__, __FUNC__, __LINE__, uev, uev->uev_Type));
				d2(kprintf("%s/%s/%ld: uev_NewPosX=%ld  uev_NewPosY=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_NewPosX, uev->uev_NewPosY));

				// TODO: fill uev contents
				switch (uev->uev_Type)
					{
				case UNDO_Copy:
				case UNDO_Move:
				case UNDO_CreateLink:
					Success = UndoAddCopyMoveEvent(uev, TagList);
					break;
				case UNDO_ChangeIconPos:
					Success = AddChangeIconPosEvent(uev, TagList);
					break;
				case UNDO_Cleanup:
					Success = AddCleanupEvent(uev, TagList);
					break;
				case UNDO_Snapshot:
				case UNDO_Unsnapshot:
					Success = AddSnapshotEvent(uev, TagList);
					break;
				default:
					if (uev->uev_CustomAddHook)
						{
						Success = CallHookPkt(uev->uev_CustomAddHook, NULL, uev);
						}
					break;
					}

				if (Success)
					{
					AddTail(&ust->ust_EventList, &uev->uev_Node);
					uev = NULL;

					if (NULL == ustMulti)
						{
						UndoEndStep((APTR) ust);
						}
					}
				}
			}

		if (uev)
			UndoDisposeEvent(uev);
		}

	return TRUE;
}

//----------------------------------------------------------------------------

APTR UndoBeginStep(void)
{
	struct UndoStep *ust;

	do	{
		ust = UndoCreateStep();
		if (NULL == ust)
			break;
		} while (0);

	return (APTR) ust;
}

//----------------------------------------------------------------------------

void UndoEndStep(APTR event)
{
	struct UndoStep *ust = (struct UndoStep *) event;

	do	{
		if (NULL == ust)
			break;

		if (IsListEmpty(&ust->ust_EventList))
			break;

		ScalosObtainSemaphore(&UndoListListSemaphore);
		AddTail(&globalUndoList, &ust->ust_Node);
		UndoCount++;
		ScalosReleaseSemaphore(&UndoListListSemaphore);

		ust = NULL;	// do not dipose ust here!
		} while (0);

	if (ust)
		UndoDisposeStep(ust);
}

//----------------------------------------------------------------------------

static struct UndoStep *UndoCreateStep(void)
{
	struct UndoStep *ust;

	do	{
		ust = ScalosAllocVecPooled(sizeof(struct UndoStep));
		if (NULL == ust)
			break;

		NewList(&ust->ust_EventList);
		} while (0);

	return ust;
}

//----------------------------------------------------------------------------

static void UndoDisposeStep(struct UndoStep *ust)
{
	if (ust)
		{
		struct UndoEvent *uev;

		while ((uev = (struct UndoEvent *) RemHead(&ust->ust_EventList)))
			{
			UndoDisposeEvent(uev);
			}

		ScalosFreeVecPooled(ust);
		}
}

//----------------------------------------------------------------------------

static struct UndoEvent *UndoCreateEvent(void)
{
	struct UndoEvent *uev;

	do	{
		static struct Hook DummyHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoDummyFunc),	// h_Entry + h_SubEntry
			NULL,				// h_Data
			};

		uev = ScalosAllocVecPooled(sizeof(struct UndoEvent));
		if (NULL == uev)
			break;

		uev->uev_UndoHook = &DummyHook;
		uev->uev_RedoHook = &DummyHook;
		uev->uev_DisposeHook = &DummyHook;
		} while (0);
	return uev;
}

//----------------------------------------------------------------------------

static void UndoDisposeEvent(struct UndoEvent *uev)
{
	if (uev)
		{
		if (uev->uev_DisposeHook)
			CallHookPkt(uev->uev_DisposeHook, NULL, uev);  // cleanup type-specific data
		ScalosFreeVecPooled(uev);
		}
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoDummyFunc(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	(void) hook;
	(void) object;
	(void) uev;

	return 0;
}

//----------------------------------------------------------------------------

static void RedoCleanup(void)
{
	struct UndoStep *ust;

	ScalosObtainSemaphore(&RedoListListSemaphore);

	while ((ust = (struct UndoStep *) RemHead(&globalRedoList)))
		{
		UndoDisposeStep(ust);
		}
	ScalosReleaseSemaphore(&RedoListListSemaphore);
}

//----------------------------------------------------------------------------

BOOL Undo(void)
{
	struct UndoStep *ust;

	ScalosObtainSemaphore(&UndoListListSemaphore);
	ust = (struct UndoStep *) RemTail(&globalUndoList);
	UndoCount--;
	ScalosReleaseSemaphore(&UndoListListSemaphore);

	d2(kprintf("%s/%s/%ld: ust=%08lx\n", __FILE__, __FUNC__, __LINE__, ust));

	if (ust)
		{
		struct UndoEvent *uev;

		for (uev = (struct UndoEvent *) ust->ust_EventList.lh_Head;
			uev != (struct UndoEvent *) &ust->ust_EventList.lh_Tail;
			uev = (struct UndoEvent *) uev->uev_Node.ln_Succ)
			{
			d2(kprintf("%s/%s/%ld: uev=%08lx  uev_Type=%ld  uev_UndoHook=%08lx\n", __FILE__, __FUNC__, __LINE__, uev, uev->uev_Type, uev->uev_UndoHook));
			if (uev->uev_UndoHook)
				CallHookPkt(uev->uev_UndoHook, NULL, uev);
			}

		// Move undo step to redo list
		ScalosObtainSemaphore(&RedoListListSemaphore);
		AddTail(&globalRedoList, &ust->ust_Node);
		ScalosReleaseSemaphore(&RedoListListSemaphore);
		}

	return TRUE;
}

//----------------------------------------------------------------------------

BOOL Redo(void)
{
	struct UndoStep *ust;

	ScalosObtainSemaphore(&RedoListListSemaphore);
	ust = (struct UndoStep *) RemTail(&globalRedoList);
	ScalosReleaseSemaphore(&RedoListListSemaphore);

	d2(kprintf("%s/%s/%ld: ust=%08lx\n", __FILE__, __FUNC__, __LINE__, ust));

	if (ust)
		{
		struct UndoEvent *uev;

		for (uev = (struct UndoEvent *) ust->ust_EventList.lh_Head;
			uev != (struct UndoEvent *) &ust->ust_EventList.lh_Tail;
			uev = (struct UndoEvent *) uev->uev_Node.ln_Succ)
			{
			d2(kprintf("%s/%s/%ld: uev=%08lx  uev_Type=%ld  uev_RedoHook=%08lx\n", __FILE__, __FUNC__, __LINE__, uev, uev->uev_Type, uev->uev_RedoHook));
			if (uev->uev_RedoHook)
				CallHookPkt(uev->uev_RedoHook, NULL, uev);
			}

		// Move redo step to undo list
		ScalosObtainSemaphore(&UndoListListSemaphore);
		AddTail(&globalUndoList, &ust->ust_Node);
		UndoCount++;
		ScalosReleaseSemaphore(&UndoListListSemaphore);
		}

	return TRUE;
}

//----------------------------------------------------------------------------

static BOOL UndoAddCopyMoveEvent(struct UndoEvent *uev, struct TagItem *TagList)
{
	BOOL Success = FALSE;
	STRPTR name = NULL;

	do	{
		BPTR dirLock;
		CONST_STRPTR fName;
		static struct Hook UndoDisposeCopyMoveDataHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoDisposeCopyMoveData),	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook UndoCopyEventHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoCopyEvent),      	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook RedoCopyEventHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(RedoCopyEvent),      	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook UndoMoveEventHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoMoveEvent),      	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook RedoMoveEventHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(RedoMoveEvent),      	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook RedoCreateLinkEventHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(RedoCreateLinkEvent),    	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};

		uev->uev_DisposeHook = (struct Hook *) GetTagData(UNDOTAG_DisposeHook, (ULONG) &UndoDisposeCopyMoveDataHook, TagList);

		if (UNDO_Copy == uev->uev_Type)
			{
			uev->uev_UndoHook = (struct Hook *) GetTagData(UNDOTAG_UndoHook, (ULONG) &UndoCopyEventHook, TagList);
			}
		else if (UNDO_Move == uev->uev_Type)
			{
			uev->uev_UndoHook = (struct Hook *) GetTagData(UNDOTAG_UndoHook, (ULONG) &UndoMoveEventHook, TagList);
			}
		else if (UNDO_CreateLink == uev->uev_Type)
			{
			uev->uev_UndoHook = (struct Hook *) GetTagData(UNDOTAG_UndoHook, (ULONG) &UndoCopyEventHook, TagList);
			}

		if (UNDO_Copy == uev->uev_Type)
			{
			uev->uev_RedoHook = (struct Hook *) GetTagData(UNDOTAG_RedoHook, (ULONG) &RedoCopyEventHook, TagList);
			}
		else if (UNDO_Move == uev->uev_Type)
			{
			uev->uev_RedoHook = (struct Hook *) GetTagData(UNDOTAG_RedoHook, (ULONG) &RedoMoveEventHook, TagList);
			}
		else if (UNDO_CreateLink == uev->uev_Type)
			{
			uev->uev_RedoHook = (struct Hook *) GetTagData(UNDOTAG_RedoHook, (ULONG) &RedoCreateLinkEventHook, TagList);
			}

		uev->uev_OldPosX = GetTagData(UNDOTAG_OldIconPosX, NO_ICON_POSITION_SHORT, TagList);
		uev->uev_OldPosY = GetTagData(UNDOTAG_OldIconPosY, NO_ICON_POSITION_SHORT, TagList);

		name = AllocPathBuffer();
		if (NULL == name)
			break;

		dirLock = (BPTR) GetTagData(UNDOTAG_CopySrcDirLock, BNULL, TagList);
		if (BNULL == dirLock)
			break;

		if (!NameFromLock(dirLock, name, Max_PathLen))
			break;

		d2(kprintf("%s/%s/%ld: UNDOTAG_CopySrcDirLock=<%s>\n", __FILE__, __FUNC__, __LINE__, name));

		uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName = AllocCopyString(name);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName)
			break;

		dirLock = (BPTR) GetTagData(UNDOTAG_CopyDestDirLock, BNULL, TagList);
		if (BNULL == dirLock)
			break;

		if (!NameFromLock(dirLock, name, Max_PathLen))
			break;

		d2(kprintf("%s/%s/%ld: UNDOTAG_CopyDestDirLock=<%s>\n", __FILE__, __FUNC__, __LINE__, name));

		uev->uev_Data.uev_CopyMoveData.ucmed_destDirName = AllocCopyString(name);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_destDirName)
			break;

		fName = (CONST_STRPTR) GetTagData(UNDOTAG_CopySrcName, (ULONG) NULL, TagList);
		if (NULL == fName)
			break;

		d2(kprintf("%s/%s/%ld: UNDOTAG_CopySrcName=<%s>\n", __FILE__, __FUNC__, __LINE__, fName));

		uev->uev_Data.uev_CopyMoveData.ucmed_srcName = AllocCopyString(fName);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_srcName)
			break;
	
		fName = (CONST_STRPTR) GetTagData(UNDOTAG_CopyDestName, (ULONG) uev->uev_Data.uev_CopyMoveData.ucmed_srcName, TagList);
		if (NULL == fName)
			fName = uev->uev_Data.uev_CopyMoveData.ucmed_srcName;

		d2(kprintf("%s/%s/%ld: UNDOTAG_CopyDestName=<%s>\n", __FILE__, __FUNC__, __LINE__, fName));

		uev->uev_Data.uev_CopyMoveData.ucmed_destName = AllocCopyString(fName);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_destName)
			break;

		if ((NO_ICON_POSITION_SHORT == uev->uev_OldPosX) || (NO_ICON_POSITION_SHORT == uev->uev_OldPosY))
			{
			struct internalScaWindowTask *iwt;
			struct ScaIconNode *in = NULL;

			do	{
				struct ExtGadget *gg;

				iwt = UndoFindWindowByDir(uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName);
				if (NULL == iwt)
					break;

				in = UndoFindIconByName(iwt, uev->uev_Data.uev_CopyMoveData.ucmed_srcName);
				if (NULL == in)
					break;

				gg = (struct ExtGadget *) in->in_Icon;

				uev->uev_OldPosX = gg->LeftEdge;
				uev->uev_OldPosY = gg->TopEdge;
				} while (0);

			if (iwt)
				{
				if (in)
					ScalosUnLockIconList(iwt);
				SCA_UnLockWindowList();
				}
			}

		Success = TRUE;
		} while (0);

	if (name)
		FreePathBuffer(name);

	return Success;
}

//----------------------------------------------------------------------------

static BOOL AddChangeIconPosEvent(struct UndoEvent *uev, struct TagItem *TagList)
{
	BOOL Success = FALSE;
	STRPTR name = NULL;

	do	{
		BPTR dirLock;
		struct ScaIconNode *in;
		const struct ExtGadget *gg;
		static struct Hook DisposeIconDataHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoDisposeIconData),	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook UndoChangeIconPosEventHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoChangeIconPosEvent),   // h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook RedoChangeIconPosEventHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(RedoChangeIconPosEvent),	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};

		uev->uev_DisposeHook = (struct Hook *) GetTagData(UNDOTAG_DisposeHook, (ULONG) &DisposeIconDataHook, TagList);
		uev->uev_UndoHook = (struct Hook *) GetTagData(UNDOTAG_UndoHook, (ULONG) &UndoChangeIconPosEventHook, TagList);
		uev->uev_RedoHook = (struct Hook *) GetTagData(UNDOTAG_RedoHook, (ULONG) &RedoChangeIconPosEventHook, TagList);

		name = AllocPathBuffer();
		if (NULL == name)
			break;

		dirLock = (BPTR) GetTagData(UNDOTAG_IconDirLock, BNULL, TagList);
		if (BNULL == dirLock)
			break;

		if (!NameFromLock(dirLock, name, Max_PathLen))
			break;

		d2(kprintf("%s/%s/%ld: UNDOTAG_IconDirLock=<%s>\n", __FILE__, __FUNC__, __LINE__, name));

		uev->uev_Data.uev_IconData.uid_DirName = AllocCopyString(name);
		if (NULL == uev->uev_Data.uev_IconData.uid_DirName)
			break;

		in  = (struct ScaIconNode *) GetTagData(UNDOTAG_IconNode, (ULONG) NULL, TagList);
		if (NULL == in)
			break;

		d2(kprintf("%s/%s/%ld: UNDOTAG_IconNode=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, in, GetIconName(in)));

		uev->uev_Data.uev_IconData.uid_IconName = AllocCopyString(GetIconName(in));
		if (NULL == uev->uev_Data.uev_IconData.uid_IconName)
			break;

		gg = (const struct ExtGadget *) in->in_Icon;

		uev->uev_OldPosX = gg->LeftEdge;
		uev->uev_OldPosY = gg->TopEdge;

		d2(kprintf("%s/%s/%ld: UNDOTAG_IconOldPosX=%ld  UNDOTAG_IconOldPosY=%ld\n", \
			__FILE__, __FUNC__, __LINE__, uev->uev_OldPosX, \
			uev->uev_OldPosY));

		Success = TRUE;
		} while (0);

	if (name)
		FreePathBuffer(name);

	return Success;
}

//----------------------------------------------------------------------------

static BOOL AddSnapshotEvent(struct UndoEvent *uev, struct TagItem *TagList)
{
	BOOL Success = FALSE;
	STRPTR name = NULL;

	do	{
		BPTR oldDir;
		BPTR dirLock;
		struct ScaIconNode *in;
		const struct ExtGadget *gg;
		static struct Hook DisposeIconDataHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoDisposeSnapshotData),	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook UndoSnapshotEventHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoSnapshotEvent),   	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook RedoSnapshotPosEventHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(RedoSnapshotPosEvent),	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};

		uev->uev_DisposeHook = (struct Hook *) GetTagData(UNDOTAG_DisposeHook, (ULONG) &DisposeIconDataHook, TagList);
		uev->uev_UndoHook = (struct Hook *) GetTagData(UNDOTAG_UndoHook, (ULONG) &UndoSnapshotEventHook, TagList);
		uev->uev_RedoHook = (struct Hook *) GetTagData(UNDOTAG_RedoHook, (ULONG) &RedoSnapshotPosEventHook, TagList);

		name = AllocPathBuffer();
		if (NULL == name)
			break;

		dirLock = (BPTR) GetTagData(UNDOTAG_IconDirLock, BNULL, TagList);
		if (BNULL == dirLock)
			break;

		if (!NameFromLock(dirLock, name, Max_PathLen))
			break;

		d2(kprintf("%s/%s/%ld: UNDOTAG_IconDirLock=<%s>\n", __FILE__, __FUNC__, __LINE__, name));

		uev->uev_Data.uev_SnapshotData.usid_DirName = AllocCopyString(name);
		if (NULL == uev->uev_Data.uev_SnapshotData.usid_DirName)
			break;

		uev->uev_Data.uev_SnapshotData.usid_SaveIcon = GetTagData(UNDOTAG_SaveIcon, FALSE, TagList);

		d2(kprintf("%s/%s/%ld: UNDOTAG_SaveIcon=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SnapshotData.usid_SaveIcon));

		in  = (struct ScaIconNode *) GetTagData(UNDOTAG_IconNode, (ULONG) NULL, TagList);
		if (NULL == in)
			break;

		d2(kprintf("%s/%s/%ld: UNDOTAG_IconNode=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, in, GetIconName(in)));

		uev->uev_Data.uev_SnapshotData.usid_IconName = AllocCopyString(GetIconName(in));
		if (NULL == uev->uev_Data.uev_SnapshotData.usid_IconName)
			break;

		oldDir = CurrentDir(dirLock);

		uev->uev_Data.uev_SnapshotData.usid_IconObj = (Object *) NewIconObjectTags(uev->uev_Data.uev_SnapshotData.usid_IconName,
			IDTA_SupportedIconTypes, CurrentPrefs.pref_SupportedIconTypes,
			IDTA_SizeConstraints, (ULONG) &CurrentPrefs.pref_IconSizeConstraints,
			IDTA_Text, (ULONG) uev->uev_Data.uev_SnapshotData.usid_IconName,
			DTA_Name, (ULONG) uev->uev_Data.uev_SnapshotData.usid_IconName,
			TAG_END);

		CurrentDir(oldDir);

		d2(kprintf("%s/%s/%ld: usid_IconObj=%08lx\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SnapshotData.usid_IconObj));

		if (NULL == uev->uev_Data.uev_SnapshotData.usid_IconObj)
			break;

		gg = (const struct ExtGadget *) in->in_Icon;

		uev->uev_OldPosX = gg->LeftEdge;
		uev->uev_OldPosY = gg->TopEdge;

		d2(kprintf("%s/%s/%ld: UNDOTAG_IconOldPosX=%ld  UNDOTAG_IconOldPosY=%ld\n", \
			__FILE__, __FUNC__, __LINE__, uev->uev_OldPosX, \
			uev->uev_OldPosY));

		Success = TRUE;
		} while (0);

	if (name)
		FreePathBuffer(name);

	return Success;
}
//----------------------------------------------------------------------------

static BOOL AddCleanupEvent(struct UndoEvent *uev, struct TagItem *TagList)
{
	BOOL Success = FALSE;

	d2(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	do	{
		const struct ScaIconNode *IconList;
		const struct ScaIconNode *in;
		struct UndoCleanupIconEntry *ucie;
		static struct Hook DisposeCleanupDataHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoDisposeCleanupData),	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook UndoCleanupEventHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoCleanupEvent),   	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook RedoCleanupEventHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(RedoCleanupEvent),		// h_Entry + h_SubEntry
			NULL,					// h_Data
			};

		uev->uev_DisposeHook = (struct Hook *) GetTagData(UNDOTAG_DisposeHook, (ULONG) &DisposeCleanupDataHook, TagList);
		uev->uev_UndoHook = (struct Hook *) GetTagData(UNDOTAG_UndoHook, (ULONG) &UndoCleanupEventHook, TagList);
		uev->uev_RedoHook = (struct Hook *) GetTagData(UNDOTAG_RedoHook, (ULONG) &RedoCleanupEventHook, TagList);

		uev->uev_Data.uev_CleanupData.ucd_CleanupMode = GetTagData(UNDOTag_CleanupMode, CLEANUP_Default, TagList);

		d2(kprintf("%s/%s/%ld: UNDOTag_CleanupMode=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CleanupData.ucd_CleanupMode));

		uev->uev_Data.uev_CleanupData.ucd_WindowTask = (struct internalScaWindowTask *) GetTagData(UNDOTag_WindowTask, (ULONG) NULL, TagList);
		if (NULL == uev->uev_Data.uev_CleanupData.ucd_WindowTask)
			break;

		d2(kprintf("%s/%s/%ld: UNDOTag_WindowTask=%08lx\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CleanupData.ucd_WindowTask));

		IconList = (struct ScaIconNode *) GetTagData(UNDOTAG_IconList, (ULONG) NULL, TagList);
		if (NULL == IconList)
			break;

		d2(kprintf("%s/%s/%ld: UNDOTAG_IconList=%08lx\n", __FILE__, __FUNC__, __LINE__, IconList));

		ScalosLockIconListShared(uev->uev_Data.uev_CleanupData.ucd_WindowTask);

		// count number of icons
		for (in = IconList; in; in = (const struct ScaIconNode *) in->in_Node.mln_Succ)
			{
			uev->uev_Data.uev_CleanupData.ucd_IconCount++;
			}

		d2(kprintf("%s/%s/%ld: ucd_IconCount=%lu\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CleanupData.ucd_IconCount));

		// create array to store icon positions
		uev->uev_Data.uev_CleanupData.ucd_Icons = ScalosAllocVecPooled(uev->uev_Data.uev_CleanupData.ucd_IconCount * sizeof(struct UndoCleanupIconEntry));
		if (NULL == uev->uev_Data.uev_CleanupData.ucd_Icons)
			break;

		// store icon positions in array
		for (ucie = uev->uev_Data.uev_CleanupData.ucd_Icons, in = IconList;
			in; in = (const struct ScaIconNode *) in->in_Node.mln_Succ, ucie++)
			{
			const struct ExtGadget *gg = (const struct ExtGadget *) in->in_Icon;

			ucie->ucin_IconNode = in;
			ucie->ucin_Left = gg->LeftEdge;
			ucie->ucin_Top = gg->TopEdge;

			d2(kprintf("%s/%s/%ld: in=%08lx <%s>  Left=%ld  Top=%ld\n", \
				__FILE__, __FUNC__, __LINE__, in, GetIconName(in), \
				ucie->ucin_Left, ucie->ucin_Top));
			}

		ScalosUnLockIconList(uev->uev_Data.uev_CleanupData.ucd_WindowTask);

		d2(kprintf("%s/%s/%ld: ucd_WindowTask=%08lx\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CleanupData.ucd_WindowTask));

		Success = TRUE;
		} while (0);

	d2(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoCopyEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	BOOL Success = FALSE;
	BPTR destDirLock = BNULL;
	STRPTR iconName = NULL;

	(void) hook;
	(void) object;

	d2(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	do	{
		STRPTR destName;
		BPTR oldDir;
		LONG Result;
		LONG IconResult;
		struct ScaUpdateIcon_IW upd;

		destDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_destDirName, ACCESS_READ);
		d2(kprintf("%s/%s/%ld: destDirLock=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, destDirLock, uev->uev_Data.uev_CopyMoveData.ucmed_destDirName));
		if (BNULL == destDirLock)
			break;

		destName = uev->uev_Data.uev_CopyMoveData.ucmed_destName;
		if (0 == strlen(destName))
			destName = uev->uev_Data.uev_CopyMoveData.ucmed_srcName;

		upd.ui_iw_Lock = destDirLock;
		upd.ui_iw_Name = destName;

		iconName = UndoBuildIconName(destName);
		if (NULL == iconName)
			break;

		oldDir = CurrentDir(destDirLock);

		Result = DeleteFile(destName);
		d2(kprintf("%s/%s/%ld: ucmed_destName=<%s>  Result=%ld  IoErr=%ld\n", __FILE__, __FUNC__, \
			__LINE__, destName, Result, IoErr()));

		// try to also to remove associated icon
		IconResult = DeleteFile(iconName);
		d2(kprintf("%s/%s/%ld: IconResult=%ld\n", __FILE__, __FUNC__, __LINE__, IconResult));

		SCA_UpdateIcon(WSV_Type_IconWindow, &upd, sizeof(upd));

		CurrentDir(oldDir);

		if (!Result)
			break;	// DeleteFile failed

		Success = TRUE;
		} while (0);

	if (BNULL != destDirLock)
		UnLock(destDirLock);
	if (iconName)
		FreePathBuffer(iconName);

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) RedoCopyEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	BOOL Success = FALSE;
	BPTR srcDirLock = BNULL;
	BPTR destDirLock = BNULL;
	Object *fileTransObj = NULL;

	(void) hook;
	(void) object;

	d2(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	do	{
		ULONG Result;

		srcDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName, ACCESS_READ);
		if (BNULL == srcDirLock)
			break;

		destDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_destDirName, ACCESS_READ);
		if (BNULL == destDirLock)
			break;

		fileTransObj = SCA_NewScalosObjectTags((STRPTR) "FileTransfer.sca",
			SCCA_FileTrans_Screen, (ULONG) iInfos.xii_iinfos.ii_Screen,
			SCCA_FileTrans_ReplaceMode, SCCV_ReplaceMode_Ask,
			TAG_END);
		if (NULL == fileTransObj)
			break;

		Result = DoMethod(fileTransObj, SCCM_FileTrans_Copy,
			srcDirLock, destDirLock,
			uev->uev_Data.uev_CopyMoveData.ucmed_srcName,
			uev->uev_Data.uev_CopyMoveData.ucmed_destName,
			uev->uev_NewPosX, uev->uev_NewPosY);
		if (RETURN_OK != Result)
			break;

		Success = TRUE;
		} while (0);

	if (fileTransObj)
		SCA_DisposeScalosObject(fileTransObj);

	if (BNULL != srcDirLock)
		UnLock(srcDirLock);
	if (BNULL != destDirLock)
		UnLock(destDirLock);

	d2(kprintf("%s/%s/%ld:  END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

// Move moved file system object back to original position
static SAVEDS(LONG) UndoMoveEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	BOOL Success = FALSE;
	BPTR srcDirLock = BNULL;
	BPTR destDirLock = BNULL;
	Object *fileTransObj = NULL;

	(void) hook;
	(void) object;

	d2(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	do	{
		ULONG Result;

		d2(kprintf("%s/%s/%ld: ucmed_srcDirName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName));

		srcDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName, ACCESS_READ);
		if (BNULL == srcDirLock)
			break;

		debugLock_d2(srcDirLock);

		d2(kprintf("%s/%s/%ld: ucmed_destDirName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CopyMoveData.ucmed_destDirName));

		destDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_destDirName, ACCESS_READ);
		if (BNULL == destDirLock)
			break;

		debugLock_d2(destDirLock);

		fileTransObj = SCA_NewScalosObjectTags((STRPTR) "FileTransfer.sca",
			SCCA_FileTrans_Screen, (ULONG) iInfos.xii_iinfos.ii_Screen,
			SCCA_FileTrans_ReplaceMode, SCCV_ReplaceMode_Ask,
			TAG_END);
		if (NULL == fileTransObj)
			break;

		d2(kprintf("%s/%s/%ld: fileTransObj=%08lx\n", __FILE__, __FUNC__, __LINE__, fileTransObj));
		d2(kprintf("%s/%s/%ld: .ucmed_srcName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CopyMoveData.ucmed_srcName));

		Result = DoMethod(fileTransObj, SCCM_FileTrans_Move,
			destDirLock, srcDirLock,
			uev->uev_Data.uev_CopyMoveData.ucmed_srcName,
			uev->uev_OldPosX, uev->uev_OldPosY);
		if (RETURN_OK != Result)
			break;

		d2(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

		Success = TRUE;
		} while (0);

	if (fileTransObj)
		SCA_DisposeScalosObject(fileTransObj);

	d2(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	if (BNULL != srcDirLock)
		UnLock(srcDirLock);
	if (BNULL != destDirLock)
		UnLock(destDirLock);

	d2(kprintf("%s/%s/%ld:  END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

// Repeat Move of moved file system object
static SAVEDS(LONG) RedoMoveEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	BOOL Success = FALSE;
	BPTR srcDirLock = BNULL;
	BPTR destDirLock = BNULL;
	Object *fileTransObj = NULL;

	(void) hook;
	(void) object;

	d2(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	do	{
		ULONG Result;

		srcDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName, ACCESS_READ);
		if (BNULL == srcDirLock)
			break;

		destDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_destDirName, ACCESS_READ);
		if (BNULL == destDirLock)
			break;

		fileTransObj = SCA_NewScalosObjectTags((STRPTR) "FileTransfer.sca",
			SCCA_FileTrans_Screen, (ULONG) iInfos.xii_iinfos.ii_Screen,
			SCCA_FileTrans_ReplaceMode, SCCV_ReplaceMode_Ask,
			TAG_END);
		if (NULL == fileTransObj)
			break;

		Result = DoMethod(fileTransObj, SCCM_FileTrans_Move,
			srcDirLock, destDirLock,
			uev->uev_Data.uev_CopyMoveData.ucmed_srcName,
			uev->uev_NewPosX, uev->uev_NewPosY);
		if (RETURN_OK != Result)
			break;

		Success = TRUE;
		} while (0);

	if (fileTransObj)
		SCA_DisposeScalosObject(fileTransObj);

	if (BNULL != srcDirLock)
		UnLock(srcDirLock);
	if (BNULL != destDirLock)
		UnLock(destDirLock);

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) RedoCreateLinkEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	BOOL Success = FALSE;
	BPTR srcDirLock = BNULL;
	BPTR destDirLock = BNULL;
	Object *fileTransObj = NULL;

	(void) hook;
	(void) object;

	d2(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	do	{
		ULONG Result;

		srcDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName, ACCESS_READ);
		if (BNULL == srcDirLock)
			break;

		destDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_destDirName, ACCESS_READ);
		if (BNULL == destDirLock)
			break;

		fileTransObj = SCA_NewScalosObjectTags((STRPTR) "FileTransfer.sca",
			SCCA_FileTrans_Screen, (ULONG) iInfos.xii_iinfos.ii_Screen,
			SCCA_FileTrans_ReplaceMode, SCCV_ReplaceMode_Ask,
			TAG_END);
		if (NULL == fileTransObj)
			break;

		Result = DoMethod(fileTransObj, SCCM_FileTrans_CreateLink,
			srcDirLock, destDirLock,
			uev->uev_Data.uev_CopyMoveData.ucmed_srcName,
			uev->uev_NewPosX, uev->uev_NewPosY);
		if (RETURN_OK != Result)
			break;

		Success = TRUE;
		} while (0);

	if (fileTransObj)
		SCA_DisposeScalosObject(fileTransObj);

	if (BNULL != srcDirLock)
		UnLock(srcDirLock);
	if (BNULL != destDirLock)
		UnLock(destDirLock);

	return Success;
}

//----------------------------------------------------------------------------

static STRPTR UndoBuildIconName(CONST_STRPTR ObjName)
{
	STRPTR iconName = AllocPathBuffer();

	if (iconName)
		{
		stccpy(iconName, ObjName, Max_PathLen);
		SafeStrCat(iconName, ".info", Max_PathLen);
		}

	return iconName;
}

//----------------------------------------------------------------------------

static BOOL MoveIconTo(CONST_STRPTR DirName, CONST_STRPTR IconName, LONG xNew, LONG yNew)
{
	BOOL Success = FALSE;
	struct internalScaWindowTask *iwt;
	struct ScaIconNode *in = NULL;

	do	{
		struct ScaIconNode *MovedIconList = NULL;
		LONG x0, y0;
		struct ExtGadget *gg;

		iwt = UndoFindWindowByDir(DirName);
		if (NULL == iwt)
			break;

		in = UndoFindIconByName(iwt, IconName);
		if (NULL == in)
			break;

		SCA_MoveIconNode(&iwt->iwt_WindowTask.wt_IconList, &MovedIconList, in);

		RemoveIcons(iwt, &MovedIconList);	// visibly remove icon from window

		gg = (struct ExtGadget *) in->in_Icon;

		// Adjust icon position
		x0 = gg->LeftEdge - gg->BoundsLeftEdge;

		gg->LeftEdge = xNew;
		gg->BoundsLeftEdge = gg->LeftEdge - x0;

		y0 = gg->TopEdge - gg->BoundsTopEdge;

		gg->TopEdge = yNew;
		gg->BoundsTopEdge = gg->TopEdge - y0;

		RefreshIconList(iwt, MovedIconList, NULL);	// draw icon at new position in window

		SCA_MoveIconNode(&MovedIconList, &iwt->iwt_WindowTask.wt_IconList, in);

		Success = TRUE;
		} while (0);

	if (iwt)
		{
		if (in)
			ScalosUnLockIconList(iwt);
		SCA_UnLockWindowList();
		}

	return Success;
}

//----------------------------------------------------------------------------

static struct internalScaWindowTask *UndoFindWindowByDir(CONST_STRPTR DirName)
{
	struct internalScaWindowTask *iwt = NULL;
	BPTR DirLock;

	DirLock = Lock(DirName, ACCESS_READ);
	if (DirLock)
		{
		struct ScaWindowStruct *ws;

		SCA_LockWindowList(SCA_LockWindowList_Shared);

		d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		debugLock_d1(DirLock);

		for (ws=winlist.wl_WindowStruct; (NULL == iwt) && ws; ws = (struct ScaWindowStruct *) ws->ws_Node.mln_Succ)
			{
			if (ws->ws_Lock && (LOCK_SAME == ScaSameLock(DirLock, ws->ws_Lock)))
				{
				iwt = (struct internalScaWindowTask *) ws->ws_WindowTask;
				d2(KPrintF("%s/%s/%ld: Found  iwt=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, iwt, iwt->iwt_WinTitle));
				break;
				}
			}

		if (NULL == iwt)
			SCA_UnLockWindowList();

		UnLock(DirLock);
		}

	d2(kprintf("%s/%s/%ld: iwt=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt));

	return iwt;
}

//----------------------------------------------------------------------------

static struct internalScaWindowTask *UndoFindWindowByWindowTask(const struct internalScaWindowTask *iwt)
{
	struct ScaWindowStruct *ws;

	SCA_LockWindowList(SCA_LockWindowList_Shared);

	d2(kprintf("%s/%s/%ld: START  iwt=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt));
	debugLock_d1(DirLock);

	for (ws=winlist.wl_WindowStruct; ws; ws = (struct ScaWindowStruct *) ws->ws_Node.mln_Succ)
		{
		if (iwt == (struct internalScaWindowTask *) ws->ws_WindowTask)
			{
			d2(KPrintF("%s/%s/%ld: Found  iwt=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, iwt, iwt->iwt_WinTitle));
			return (struct internalScaWindowTask *) ws->ws_WindowTask;
			}
		}

	SCA_UnLockWindowList();

	d2(kprintf("%s/%s/%ld: iwt=NULL\n", __FILE__, __FUNC__, __LINE__));

	return NULL;
}

//----------------------------------------------------------------------------

static struct ScaIconNode *UndoFindIconByName(struct internalScaWindowTask *iwt, CONST_STRPTR IconName)
{
	struct ScaIconNode *inFound = NULL;
	struct ScaIconNode *in;

	d1(KPrintF("%s/%s/%ld: START iwt=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, iwt, iwt->iwt_WinTitle));

	ScalosLockIconListShared(iwt);

	for (in=iwt->iwt_WindowTask.wt_IconList; in; in = (struct ScaIconNode *) in->in_Node.mln_Succ)
		{
		if (0 == Stricmp(GetIconName(in), IconName))
			{
			inFound = in;
			d2(kprintf("%s/%s/%ld: inFound=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, inFound, GetIconName(inFound)));
			break;
			}
		}

	if (NULL == inFound)
		ScalosUnLockIconList(iwt);

	d2(kprintf("%s/%s/%ld: inFound=%08lx\n", __FILE__, __FUNC__, __LINE__, inFound));

	return inFound;
}

//----------------------------------------------------------------------------

static BOOL UndoChangeIconPosEvent(struct UndoEvent *uev)
{
	d2(kprintf("%s/%s/%ld: uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));
	return MoveIconTo(uev->uev_Data.uev_IconData.uid_DirName,
		uev->uev_Data.uev_IconData.uid_IconName,
		uev->uev_OldPosX,
		uev->uev_OldPosY);
}

//----------------------------------------------------------------------------

static BOOL RedoChangeIconPosEvent(struct UndoEvent *uev)
{
	d2(kprintf("%s/%s/%ld: uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));
	return MoveIconTo(uev->uev_Data.uev_IconData.uid_DirName,
		uev->uev_Data.uev_IconData.uid_IconName,
		uev->uev_NewPosX,
		uev->uev_NewPosY);
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoDisposeCopyMoveData(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	(void) hook;
	(void) object;

	d2(kprintf("%s/%s/%ld: START  uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));
	if (uev)
		{
		if (uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName)
			{
			FreeCopyString(uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName);
			uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName = NULL;
			}
		if (uev->uev_Data.uev_CopyMoveData.ucmed_destDirName)
			{
			FreeCopyString(uev->uev_Data.uev_CopyMoveData.ucmed_destDirName);
			uev->uev_Data.uev_CopyMoveData.ucmed_destDirName = NULL;
			}
		if (uev->uev_Data.uev_CopyMoveData.ucmed_srcName)
			{
			FreeCopyString(uev->uev_Data.uev_CopyMoveData.ucmed_srcName);
			uev->uev_Data.uev_CopyMoveData.ucmed_srcName = NULL;
			}
		if (uev->uev_Data.uev_CopyMoveData.ucmed_destName)
			{
			FreeCopyString(uev->uev_Data.uev_CopyMoveData.ucmed_destName);
			uev->uev_Data.uev_CopyMoveData.ucmed_destName = NULL;
			}
		}
	d2(kprintf("%s/%s/%ld: END   uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));

	return 0;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoDisposeIconData(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	(void) hook;
	(void) object;

	d2(kprintf("%s/%s/%ld: START  uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));
	if (uev)
		{
		if (uev->uev_Data.uev_IconData.uid_DirName)
			{
			FreeCopyString(uev->uev_Data.uev_IconData.uid_DirName);
			uev->uev_Data.uev_IconData.uid_DirName = NULL;
			}
		if (uev->uev_Data.uev_IconData.uid_IconName)
			{
			FreeCopyString(uev->uev_Data.uev_IconData.uid_IconName);
			uev->uev_Data.uev_IconData.uid_IconName = NULL;
			}
		}
	d2(kprintf("%s/%s/%ld: END   uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));

	return 0;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoDisposeCleanupData(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	(void) hook;
	(void) object;

	d2(kprintf("%s/%s/%ld: START  uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));
	if (uev)
		{
		if (uev->uev_Data.uev_CleanupData.ucd_Icons)
			{
			ScalosFreeVecPooled(uev->uev_Data.uev_CleanupData.ucd_Icons);
			uev->uev_Data.uev_CleanupData.ucd_Icons = NULL;
			}
		}
	d2(kprintf("%s/%s/%ld: END   uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));

	return 0;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoDisposeSnapshotData(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	(void) hook;
	(void) object;

	d2(kprintf("%s/%s/%ld: START  uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));
	if (uev)
		{
		if (uev->uev_Data.uev_SnapshotData.usid_DirName)
			{
			FreeCopyString(uev->uev_Data.uev_SnapshotData.usid_DirName);
			uev->uev_Data.uev_SnapshotData.usid_DirName = NULL;
			}
		if (uev->uev_Data.uev_SnapshotData.usid_IconName)
			{
			FreeCopyString(uev->uev_Data.uev_SnapshotData.usid_IconName);
			uev->uev_Data.uev_SnapshotData.usid_IconName = NULL;
			}
		if (uev->uev_Data.uev_SnapshotData.usid_IconObj)
			{
			DisposeIconObject(uev->uev_Data.uev_SnapshotData.usid_IconObj);
			uev->uev_Data.uev_SnapshotData.usid_IconObj = NULL;
			}
		}
	d2(kprintf("%s/%s/%ld: END   uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));

	return 0;
}
//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoCleanupEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	struct internalScaWindowTask *iwt = NULL;
	BOOL Success = FALSE;

	(void) hook;
	(void) object;

	d2(kprintf("%s/%s/%ld: START  uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));

	do	{
		struct ScaIconNode *in;

		d2(kprintf("%s/%s/%ld: ucd_IconCount=%lu\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CleanupData.ucd_IconCount));
		d2(kprintf("%s/%s/%ld: ucd_WindowTask=%08lx\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CleanupData.ucd_WindowTask));

		iwt = UndoFindWindowByWindowTask(uev->uev_Data.uev_CleanupData.ucd_WindowTask);
		if (NULL == iwt)
			break;

		ScalosLockIconListShared(iwt);

		if (iwt->iwt_WindowTask.wt_Window)
			{
			EraseRect(iwt->iwt_WindowTask.wt_Window->RPort, 0, 0,
				iwt->iwt_WindowTask.wt_Window->Width - 1,
				iwt->iwt_WindowTask.wt_Window->Height -1);
			}

		for (in = iwt->iwt_WindowTask.wt_IconList; in; in = (struct ScaIconNode *) in->in_Node.mln_Succ)
			{
			const struct UndoCleanupIconEntry *ucie;

			ucie = UndoFindCleanupIconEntry(uev, in);
			if (ucie)
				{
				LONG x0, y0;
				struct ExtGadget *gg = (struct ExtGadget *) in->in_Icon;

				d2(kprintf("%s/%s/%ld: in=%08lx <%s>  Left=%ld  Top=%ld\n", \
					__FILE__, __FUNC__, __LINE__, in, GetIconName(in), \
					ucie->ucin_Left, ucie->ucin_Top));

				// Adjust icon position
				x0 = gg->LeftEdge - gg->BoundsLeftEdge;

				gg->LeftEdge = ucie->ucin_Left;
				gg->BoundsLeftEdge = gg->LeftEdge - x0;

				y0 = gg->TopEdge - gg->BoundsTopEdge;

				gg->TopEdge = ucie->ucin_Top;
				gg->BoundsTopEdge = gg->TopEdge - y0;
				}
			}

		RefreshIconList(iwt, iwt->iwt_WindowTask.wt_IconList, NULL);

		DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_SetVirtSize,
			SETVIRTF_AdjustBottomSlider | SETVIRTF_AdjustRightSlider);

		ScalosUnLockIconList(iwt);

		Success = TRUE;
		} while (0);

	if (iwt)
		SCA_UnLockWindowList();

	d2(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__));

	return Success;
}

//----------------------------------------------------------------------------

static BOOL RedoCleanupEvent(struct UndoEvent *uev)
{
	BOOL Success = FALSE;
	struct internalScaWindowTask *iwt = NULL;

	d2(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	do	{
		iwt = UndoFindWindowByWindowTask(uev->uev_Data.uev_CleanupData.ucd_WindowTask);
		if (NULL == iwt)
			break;

		switch (uev->uev_Data.uev_CleanupData.ucd_CleanupMode)
			{
		case CLEANUP_ByName:
			DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_CleanUpByName);
			break;
		case CLEANUP_ByDate:
			DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_CleanUpByDate);
			break;
		case CLEANUP_BySize:
			DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_CleanUpBySize);
			break;
		case CLEANUP_ByType:
			DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_CleanUpByType);
			break;
		case CLEANUP_Default:
		default:
			//TODO
			break;
			}
		} while (0);

	if (iwt)
		SCA_UnLockWindowList();

	d2(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__));

	return Success;
}

//----------------------------------------------------------------------------

static const struct UndoCleanupIconEntry *UndoFindCleanupIconEntry(const struct UndoEvent *uev, const struct ScaIconNode *in)
{
	const struct UndoCleanupIconEntry *ucie = uev->uev_Data.uev_CleanupData.ucd_Icons;
	ULONG n;

	for (n = 0; n < uev->uev_Data.uev_CleanupData.ucd_IconCount; n++, ucie++)
		{
		if (ucie->ucin_IconNode == in)
			return ucie;
		}

	return NULL;
}

//----------------------------------------------------------------------------


static SAVEDS(LONG) UndoSnapshotEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	struct internalScaWindowTask *iwt;
	struct ScaIconNode *in = NULL;
	BOOL Success = FALSE;

	d2(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	do	{
		const struct IBox *pWinRect;
		ULONG IconViewMode;
		ULONG ddFlags;
		LONG WinCurrentX, WinCurrentY;

		iwt = UndoFindWindowByDir(uev->uev_Data.uev_SnapshotData.usid_DirName);
		if (NULL == iwt)
			break;

		d2(kprintf("%s/%s/%ld: iwt=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt));
		d2(kprintf("%s/%s/%ld: usid_IconName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SnapshotData.usid_IconName));

		in = UndoFindIconByName(iwt, uev->uev_Data.uev_SnapshotData.usid_IconName);
		if (NULL == in)
			break;

		d2(kprintf("%s/%s/%ld: in=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, in, GetIconName(in)));


		if (uev->uev_OldPosX != uev->uev_NewPosX || uev->uev_OldPosY != uev->uev_NewPosY)
			{
			if (!MoveIconTo(uev->uev_Data.uev_SnapshotData.usid_DirName,
				uev->uev_Data.uev_SnapshotData.usid_IconName,
				uev->uev_OldPosX, uev->uev_OldPosY))
				break;
			}

		GetAttr(IDTA_ViewModes, uev->uev_Data.uev_SnapshotData.usid_IconObj, &IconViewMode);
		GetAttr(IDTA_Flags, uev->uev_Data.uev_SnapshotData.usid_IconObj, &ddFlags);
		GetAttr(IDTA_WindowRect, uev->uev_Data.uev_SnapshotData.usid_IconObj, (ULONG *) &pWinRect);
		GetAttr(IDTA_WinCurrentX, uev->uev_Data.uev_SnapshotData.usid_IconObj, (ULONG *) &WinCurrentX);
		GetAttr(IDTA_WinCurrentY, uev->uev_Data.uev_SnapshotData.usid_IconObj, (ULONG *) &WinCurrentY);

		SetAttrs(in->in_Icon,
			IDTA_ViewModes, IconViewMode,
			IDTA_Flags, ddFlags,
			pWinRect ? IDTA_WindowRect : TAG_IGNORE, (ULONG) pWinRect,
			IDTA_WinCurrentY, WinCurrentY,
			IDTA_WinCurrentX, WinCurrentX,
			TAG_END);

		if (uev->uev_Data.uev_SnapshotData.usid_SaveIcon)
			{
			ULONG Result;

			Result = SaveIconObject(uev->uev_Data.uev_SnapshotData.usid_IconObj,
				iwt->iwt_WindowTask.mt_WindowStruct->ws_Lock,
				uev->uev_Data.uev_SnapshotData.usid_IconName,
				FALSE,
				NULL);

			if (RETURN_OK != Result)
				break;
			}

		Success = TRUE;
		} while (0);

	if (iwt)
		{
		if (in)
			ScalosUnLockIconList(iwt);
		SCA_UnLockWindowList();
		}

	d2(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) RedoSnapshotPosEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	struct internalScaWindowTask *iwt;
	struct ScaIconNode *in = NULL;
	BOOL Success = FALSE;

	d2(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	do	{
		iwt = UndoFindWindowByDir(uev->uev_Data.uev_SnapshotData.usid_DirName);
		if (NULL == iwt)
			break;

		d2(kprintf("%s/%s/%ld: iwt=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt));
		d2(kprintf("%s/%s/%ld: usid_IconName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SnapshotData.usid_IconName));

		in = UndoFindIconByName(iwt, uev->uev_Data.uev_SnapshotData.usid_IconName);
		if (NULL == in)
			break;

		if (uev->uev_OldPosX != uev->uev_NewPosX || uev->uev_OldPosY != uev->uev_NewPosY)
			{
			if (!MoveIconTo(uev->uev_Data.uev_SnapshotData.usid_DirName,
				uev->uev_Data.uev_SnapshotData.usid_IconName,
				uev->uev_NewPosX, uev->uev_NewPosY))
				break;
			}

		SCA_LockWindowList(SCA_LockWindowList_Shared);

		if (UNDO_Snapshot == uev->uev_Type)
			Success = UndoSnapshotIcon(iwt, in);
		else
			{
			struct ScaIconNode *IconList;

			SCA_MoveIconNode(&iwt->iwt_WindowTask.wt_IconList, &IconList, in);

			RemoveIcons(iwt, &IconList);	   // visibly remove icon from window

			Success = UndoUnsnapshotIcon(iwt, in, uev->uev_Data.uev_SnapshotData.usid_SaveIcon);

			SCA_MoveIconNode(&IconList, &iwt->iwt_WindowTask.wt_LateIconList, in);

			if (IsIwtViewByIcon(iwt))
				{
				d1(KPrintF("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));
				DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_CleanUp);
				d1(KPrintF("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));
				DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_SetVirtSize,
					SETVIRTF_AdjustBottomSlider | SETVIRTF_AdjustRightSlider);
				}
			}

		SCA_UnLockWindowList();
		} while (0);

	if (iwt)
		{
		if (in)
			ScalosUnLockIconList(iwt);
		SCA_UnLockWindowList();
		}

	d2(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static BOOL UndoSnapshotIcon(struct internalScaWindowTask *iwt, struct ScaIconNode *in)
{
	BOOL Success = FALSE;
	ULONG IconType = 0;

	d2(KPrintF("%s/%s/%ld:  START iwt=%08lx  in=%08lx <%s>\n", __FILE__, __FUNC__, __LINE__, iwt, in, GetIconName(in)));

	GetAttr(IDTA_Type, in->in_Icon, &IconType);

	if (0 == IconType)
		return Success;

	if (WBAPPICON == IconType)
		{
		WindowSendAppIconMsg(iwt, AMCLASSICON_Snapshot, in);
		return TRUE;
		}
	else
		{
		BPTR destDirLock;

		SetIconWindowRect(in);

		if (in->in_DeviceIcon)
			{
			destDirLock = DiskInfoLock(in);

			if (destDirLock)
				{
				LONG Result;

				Result = ScalosPutIcon(in, destDirLock, in->in_Flags & INF_DefaultIcon);

				if (RETURN_OK == Result)
					Success = TRUE;

				UnLock(destDirLock);
				}
			}
		else
			{
			LONG Result;

			if (in->in_Lock)
				destDirLock = in->in_Lock;
			else
				destDirLock = iwt->iwt_WindowTask.mt_WindowStruct->ws_Lock;

			Result = ScalosPutIcon(in, destDirLock, in->in_Flags & INF_DefaultIcon);

			if (RETURN_OK == Result)
				Success = TRUE;
			}

		in->in_Flags &= ~INF_FreeIconPosition;
		in->in_SupportFlags &= ~INF_SupportsSnapshot;
		in->in_SupportFlags |= INF_SupportsUnSnapshot;
		}

	d2(KPrintF("%s/%s/%ld: END in=%08lx  <%s>  inFlags=%08lx  Success=%ld\n", __FILE__, __FUNC__, __LINE__, in, GetIconName(in), in->in_Flags, Success));

	return Success;
}

//----------------------------------------------------------------------------

static BOOL UndoUnsnapshotIcon(struct internalScaWindowTask *iwt,
	struct ScaIconNode *in, BOOL SaveIcon)
{
	BOOL Success = FALSE;
	ULONG IconType = 0;

	d2(KPrintF("%s/%s/%ld:  iwt=%08lx  in=%08lx <%s>\n", __FILE__, __FUNC__, __LINE__, iwt, in, GetIconName(in)));

	GetAttr(IDTA_Type, in->in_Icon, &IconType);

	if (0 == IconType)
		return Success;

	if (WBAPPICON == IconType)
		{
		WindowSendAppIconMsg(iwt, AMCLASSICON_UnSnapshot, in);
		return TRUE;
		}
	else
		{
		struct ExtGadget *gg = (struct ExtGadget *) in->in_Icon;
		BPTR destDirLock;
		WORD LeftEdge, TopEdge;

		d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

		SetIconWindowRect(in);

		d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

		LeftEdge = gg->LeftEdge;
		TopEdge = gg->TopEdge;

		if (in->in_DeviceIcon)
			{
			destDirLock = DiskInfoLock(in);

			d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

			if (destDirLock)
				{
		
				gg->LeftEdge = gg->TopEdge = NO_ICON_POSITION_SHORT;

				if (SaveIcon)
					{
					LONG Result;

					Result = ScalosPutIcon(in, destDirLock, in->in_Flags & INF_DefaultIcon);

					if (RETURN_OK == Result)
						Success = TRUE;
					}
				else
					{
					Success = TRUE;
					}

				UnLock(destDirLock);
				}
			}
		else
			{
			d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

			if (in->in_Lock)
				destDirLock = in->in_Lock;
			else
				destDirLock = iwt->iwt_WindowTask.mt_WindowStruct->ws_Lock;

			d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));
                   
			gg->LeftEdge = gg->TopEdge = NO_ICON_POSITION_SHORT;

			if (SaveIcon)
				{
				LONG Result;

				Result = ScalosPutIcon(in, destDirLock, in->in_Flags & INF_DefaultIcon);

				if (RETURN_OK == Result)
					Success = TRUE;
				}
			else
				{
				Success = TRUE;
				}

			d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));
			}

		gg->LeftEdge = LeftEdge;
		gg->TopEdge = TopEdge;

		in->in_Flags |= INF_FreeIconPosition;
		in->in_SupportFlags |= INF_SupportsSnapshot;
		in->in_SupportFlags &= ~INF_SupportsUnSnapshot;
		}

	d2(KPrintF("%s/%s/%ld: END in=%08lx  <%s>  inFlags=%08lx  Success=%ld\n", __FILE__, __FUNC__, __LINE__, in, GetIconName(in), in->in_Flags, Success));

	return Success;
}

//----------------------------------------------------------------------------


