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
	Object *ust_FileTransObj;	// valid during Undo and Redo processing
	};

//----------------------------------------------------------------------------

// local functions

static struct UndoStep *UndoCreateStep(void);
static void UndoDisposeStep(struct UndoStep *ust);
static struct UndoEvent *UndoCreateEvent(void);
static void UndoDisposeEvent(struct UndoEvent *uev);
static SAVEDS(LONG) UndoDummyFunc(struct Hook *hook, APTR object, struct UndoEvent *uev);
static void RedoCleanup(void);
static SAVEDS(ULONG) UndoTask(struct UndoStep **ust, struct SM_RunProcess *msg);
static SAVEDS(ULONG) RedoTask(struct UndoStep **ust, struct SM_RunProcess *msg);
static BOOL UndoAddCopyMoveEvent(struct UndoEvent *uev, struct TagItem *TagList);
static BOOL AddChangeIconPosEvent(struct UndoEvent *uev, struct TagItem *TagList);
static BOOL AddSnapshotEvent(struct UndoEvent *uev, struct TagItem *TagList);
static BOOL AddCleanupEvent(struct UndoEvent *uev, struct TagItem *TagList);
static BOOL AddRenameEvent(struct UndoEvent *uev, struct TagItem *TagList);
static BOOL AddRelabelEvent(struct UndoEvent *uev, struct TagItem *TagList);
static BOOL AddDeleteEvent(struct UndoEvent *uev, struct TagItem *TagList);
static BOOL AddSizeWindowEvent(struct UndoEvent *uev, struct TagItem *TagList);
static BOOL AddNewDrawerEvent(struct UndoEvent *uev, struct TagItem *TagList);
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
static SAVEDS(LONG) UndoDisposeNewDrawerData(struct Hook *hook, APTR object, struct UndoEvent *uev);
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
static SAVEDS(LONG) UndoRenameEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) RedoRenameEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) UndoRelabelEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) RedoRelabelEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) UndoSizeWindowEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) RedoSizeWindowEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) UndoNewDrawerEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);
static SAVEDS(LONG) RedoNewDrawerEvent(struct Hook *hook, APTR object, struct UndoEvent *uev);

//----------------------------------------------------------------------------

// local Data items

//----------------------------------------------------------------------------

// public data items

struct List globalUndoList;		// global Undo list for all file-related actions
SCALOSSEMAPHORE UndoListListSemaphore;	// Semaphore to protect globalUndoList
static ULONG UndoCount;			// Number of entries in globalUndoList
static ULONG maxUndoSteps = 10;

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
			d1(kprintf("%s/%s/%ld: ust=%08lx\n", __FILE__, __FUNC__, __LINE__, ust));
			if (NULL == ust)
				{
				ust = (struct UndoStep *) UndoBeginStep();
				}

			d1(kprintf("%s/%s/%ld: ust=%08lx\n", __FILE__, __FUNC__, __LINE__, ust));

			if (ust)
				{
				BOOL Success = FALSE;

				RedoCleanup();

				uev->uev_Type = type;

				uev->uev_CustomAddHook = (struct Hook *) GetTagData(UNDOTAG_CustomAddHook, (ULONG) NULL, TagList);

				uev->uev_NewPosX = GetTagData(UNDOTAG_IconPosX, NO_ICON_POSITION_SHORT, TagList);
				uev->uev_NewPosY = GetTagData(UNDOTAG_IconPosY, NO_ICON_POSITION_SHORT, TagList);

				d1(kprintf("%s/%s/%ld: uev=%08lx  uev_Type=%ld\n", __FILE__, __FUNC__, __LINE__, uev, uev->uev_Type));
				d1(kprintf("%s/%s/%ld: uev_NewPosX=%ld  uev_NewPosY=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_NewPosX, uev->uev_NewPosY));

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
				case UNDO_Rename:
					Success = AddRenameEvent(uev, TagList);
					break;
				case UNDO_Relabel:
					Success = AddRelabelEvent(uev, TagList);
					break;
				case UNDO_SizeToFit:
					Success = AddSizeWindowEvent(uev, TagList);
					break;
				case UNDO_Delete:
					Success = AddDeleteEvent(uev, TagList);
					break;
				case UNDO_NewDrawer:
					Success = AddNewDrawerEvent(uev, TagList);
					break;
				case UNDO_Leaveout:
				case UNDO_PutAway:
					//TODO
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
					uev->uev_UndoStep = ust;

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
		if (++UndoCount > maxUndoSteps)
			{
			// maxUndoSteps exceeded - dispose oldest list entry
			ust = (struct UndoStep *) RemHead(&globalUndoList);
			UndoCount--;
			}
		else
			{
			ust = NULL;	// do not dipose ust here!
			}

		ScalosReleaseSemaphore(&UndoListListSemaphore);

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

BOOL Undo(struct internalScaWindowTask *iwt)
{
	struct UndoStep *ust;
	BOOL Success = FALSE;

	ScalosObtainSemaphore(&UndoListListSemaphore);
	ust = (struct UndoStep *) RemTail(&globalUndoList);
	UndoCount--;
	ScalosReleaseSemaphore(&UndoListListSemaphore);

	d2(kprintf("%s/%s/%ld: ust=%08lx\n", __FILE__, __FUNC__, __LINE__, ust));

	if (ust)
		{
		Success = DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_RunProcess, UndoTask,
			&ust, sizeof(ust), iInfos.xii_iinfos.ii_MainMsgPort);
		}

	return Success;
}

//----------------------------------------------------------------------------

BOOL Redo(struct internalScaWindowTask *iwt)
{
	struct UndoStep *ust;
	BOOL Success = FALSE;

	ScalosObtainSemaphore(&RedoListListSemaphore);
	ust = (struct UndoStep *) RemTail(&globalRedoList);
	ScalosReleaseSemaphore(&RedoListListSemaphore);

	d1(kprintf("%s/%s/%ld: ust=%08lx\n", __FILE__, __FUNC__, __LINE__, ust));

	if (ust)
		{
		Success = DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_RunProcess, RedoTask,
			&ust, sizeof(ust), iInfos.xii_iinfos.ii_MainMsgPort);
		}

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(ULONG) UndoTask(struct UndoStep **ust, struct SM_RunProcess *msg)
{
	static CONST_STRPTR ProgTaskName = "Scalos_UndoTask";
	APTR prWindowPtr;
	struct Process *myProc = (struct Process *) FindTask(NULL);

	(void) msg;

	d2(kprintf("%s/%s/%ld: START  *ust=%08lx\n", __FILE__, __FUNC__, __LINE__, *ust));

	prWindowPtr = myProc->pr_WindowPtr;
	myProc->pr_WindowPtr = (APTR) ~0;    // suppress error requesters

	myProc->pr_Task.tc_Node.ln_Name = (STRPTR) ProgTaskName;
	SetProgramName(ProgTaskName);

	(*ust)->ust_FileTransObj = SCA_NewScalosObjectTags((STRPTR) "FileTransfer.sca",
		SCCA_FileTrans_Screen, (ULONG) iInfos.xii_iinfos.ii_Screen,
		SCCA_FileTrans_ReplaceMode, SCCV_ReplaceMode_Ask,
		TAG_END);

	if ((*ust)->ust_FileTransObj)
		{
		struct UndoEvent *uev;

		for (uev = (struct UndoEvent *) (*ust)->ust_EventList.lh_Head;
			uev != (struct UndoEvent *) &(*ust)->ust_EventList.lh_Tail;
			uev = (struct UndoEvent *) uev->uev_Node.ln_Succ)
			{
			d1(kprintf("%s/%s/%ld: uev=%08lx  uev_Type=%ld  uev_UndoHook=%08lx\n", __FILE__, __FUNC__, __LINE__, uev, uev->uev_Type, uev->uev_UndoHook));
			if (uev->uev_UndoHook)
				CallHookPkt(uev->uev_UndoHook, NULL, uev);
			}

		// Move undo step to redo list
		ScalosObtainSemaphore(&RedoListListSemaphore);
		AddTail(&globalRedoList, &(*ust)->ust_Node);
		ScalosReleaseSemaphore(&RedoListListSemaphore);

		SCA_DisposeScalosObject((*ust)->ust_FileTransObj);
		(*ust)->ust_FileTransObj = NULL;
		}
	else
		{
		UndoDisposeStep(*ust);
		}

	// restore pr_WindowPtr
	myProc->pr_WindowPtr = prWindowPtr;

	d2(kprintf("%s/%s/%ld: END\n", __FILE__, __FUNC__, __LINE__));

	return 0;
}

//----------------------------------------------------------------------------

static SAVEDS(ULONG) RedoTask(struct UndoStep **ust, struct SM_RunProcess *msg)
{
	static CONST_STRPTR ProgTaskName = "Scalos_RedoTask";
	APTR prWindowPtr;
	struct Process *myProc = (struct Process *) FindTask(NULL);

	(void) msg;

	d2(kprintf("%s/%s/%ld: START  *ust=%08lx\n", __FILE__, __FUNC__, __LINE__, *ust));

	prWindowPtr = myProc->pr_WindowPtr;
	myProc->pr_WindowPtr = (APTR) ~0;    // suppress error requesters

	myProc->pr_Task.tc_Node.ln_Name = (STRPTR) ProgTaskName;
	SetProgramName(ProgTaskName);

	(*ust)->ust_FileTransObj = SCA_NewScalosObjectTags((STRPTR) "FileTransfer.sca",
		SCCA_FileTrans_Screen, (ULONG) iInfos.xii_iinfos.ii_Screen,
		SCCA_FileTrans_ReplaceMode, SCCV_ReplaceMode_Ask,
		TAG_END);

	if ((*ust)->ust_FileTransObj)
		{
		struct UndoEvent *uev;

		for (uev = (struct UndoEvent *) (*ust)->ust_EventList.lh_Head;
			uev != (struct UndoEvent *) &(*ust)->ust_EventList.lh_Tail;
			uev = (struct UndoEvent *) uev->uev_Node.ln_Succ)
			{
			d1(kprintf("%s/%s/%ld: uev=%08lx  uev_Type=%ld  uev_RedoHook=%08lx\n", __FILE__, __FUNC__, __LINE__, uev, uev->uev_Type, uev->uev_RedoHook));
			if (uev->uev_RedoHook)
				CallHookPkt(uev->uev_RedoHook, NULL, uev);
			}

		// Move redo step to undo list
		ScalosObtainSemaphore(&UndoListListSemaphore);
		AddTail(&globalUndoList, &(*ust)->ust_Node);
		UndoCount++;
		ScalosReleaseSemaphore(&UndoListListSemaphore);

		SCA_DisposeScalosObject((*ust)->ust_FileTransObj);
		(*ust)->ust_FileTransObj = NULL;
		}
	else
		{
		UndoDisposeStep(*ust);
		}

	// restore pr_WindowPtr
	myProc->pr_WindowPtr = prWindowPtr;

	return 0;
}

//----------------------------------------------------------------------------

static BOOL UndoAddCopyMoveEvent(struct UndoEvent *uev, struct TagItem *TagList)
{
	BOOL Success = FALSE;
	STRPTR name;

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

		d1(kprintf("%s/%s/%ld: UNDOTAG_CopySrcDirLock=<%s>\n", __FILE__, __FUNC__, __LINE__, name));

		uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName = AllocCopyString(name);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName)
			break;

		dirLock = (BPTR) GetTagData(UNDOTAG_CopyDestDirLock, BNULL, TagList);
		if (BNULL == dirLock)
			break;

		if (!NameFromLock(dirLock, name, Max_PathLen))
			break;

		d1(kprintf("%s/%s/%ld: UNDOTAG_CopyDestDirLock=<%s>\n", __FILE__, __FUNC__, __LINE__, name));

		uev->uev_Data.uev_CopyMoveData.ucmed_destDirName = AllocCopyString(name);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_destDirName)
			break;

		fName = (CONST_STRPTR) GetTagData(UNDOTAG_CopySrcName, (ULONG) NULL, TagList);
		if (NULL == fName)
			break;

		d1(kprintf("%s/%s/%ld: UNDOTAG_CopySrcName=<%s>\n", __FILE__, __FUNC__, __LINE__, fName));

		uev->uev_Data.uev_CopyMoveData.ucmed_srcName = AllocCopyString(fName);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_srcName)
			break;
	
		fName = (CONST_STRPTR) GetTagData(UNDOTAG_CopyDestName, (ULONG) uev->uev_Data.uev_CopyMoveData.ucmed_srcName, TagList);
		if (NULL == fName)
			fName = uev->uev_Data.uev_CopyMoveData.ucmed_srcName;

		d1(kprintf("%s/%s/%ld: UNDOTAG_CopyDestName=<%s>\n", __FILE__, __FUNC__, __LINE__, fName));

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
	STRPTR name;

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

		d1(kprintf("%s/%s/%ld: UNDOTAG_IconDirLock=<%s>\n", __FILE__, __FUNC__, __LINE__, name));

		uev->uev_Data.uev_IconData.uid_DirName = AllocCopyString(name);
		if (NULL == uev->uev_Data.uev_IconData.uid_DirName)
			break;

		in  = (struct ScaIconNode *) GetTagData(UNDOTAG_IconNode, (ULONG) NULL, TagList);
		if (NULL == in)
			break;

		d1(kprintf("%s/%s/%ld: UNDOTAG_IconNode=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, in, GetIconName(in)));

		uev->uev_Data.uev_IconData.uid_IconName = AllocCopyString(GetIconName(in));
		if (NULL == uev->uev_Data.uev_IconData.uid_IconName)
			break;

		gg = (const struct ExtGadget *) in->in_Icon;

		uev->uev_OldPosX = gg->LeftEdge;
		uev->uev_OldPosY = gg->TopEdge;

		d1(kprintf("%s/%s/%ld: UNDOTAG_IconOldPosX=%ld  UNDOTAG_IconOldPosY=%ld\n", \
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
	STRPTR name;

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

		d1(kprintf("%s/%s/%ld: UNDOTAG_IconDirLock=<%s>\n", __FILE__, __FUNC__, __LINE__, name));

		uev->uev_Data.uev_SnapshotData.usid_DirName = AllocCopyString(name);
		if (NULL == uev->uev_Data.uev_SnapshotData.usid_DirName)
			break;

		uev->uev_Data.uev_SnapshotData.usid_SaveIcon = GetTagData(UNDOTAG_SaveIcon, FALSE, TagList);

		d1(kprintf("%s/%s/%ld: UNDOTAG_SaveIcon=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SnapshotData.usid_SaveIcon));

		in  = (struct ScaIconNode *) GetTagData(UNDOTAG_IconNode, (ULONG) NULL, TagList);
		if (NULL == in)
			break;

		d1(kprintf("%s/%s/%ld: UNDOTAG_IconNode=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, in, GetIconName(in)));

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

		d1(kprintf("%s/%s/%ld: usid_IconObj=%08lx\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SnapshotData.usid_IconObj));

		if (NULL == uev->uev_Data.uev_SnapshotData.usid_IconObj)
			break;

		gg = (const struct ExtGadget *) in->in_Icon;

		uev->uev_OldPosX = gg->LeftEdge;
		uev->uev_OldPosY = gg->TopEdge;

		d1(kprintf("%s/%s/%ld: UNDOTAG_IconOldPosX=%ld  UNDOTAG_IconOldPosY=%ld\n", \
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

	d1(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

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

		d1(kprintf("%s/%s/%ld: UNDOTag_CleanupMode=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CleanupData.ucd_CleanupMode));

		uev->uev_Data.uev_CleanupData.ucd_WindowTask = (struct internalScaWindowTask *) GetTagData(UNDOTag_WindowTask, (ULONG) NULL, TagList);
		if (NULL == uev->uev_Data.uev_CleanupData.ucd_WindowTask)
			break;

		d1(kprintf("%s/%s/%ld: UNDOTag_WindowTask=%08lx\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CleanupData.ucd_WindowTask));

		IconList = (struct ScaIconNode *) GetTagData(UNDOTAG_IconList, (ULONG) NULL, TagList);
		if (NULL == IconList)
			break;

		d1(kprintf("%s/%s/%ld: UNDOTAG_IconList=%08lx\n", __FILE__, __FUNC__, __LINE__, IconList));

		ScalosLockIconListShared(uev->uev_Data.uev_CleanupData.ucd_WindowTask);

		// count number of icons
		for (in = IconList; in; in = (const struct ScaIconNode *) in->in_Node.mln_Succ)
			{
			uev->uev_Data.uev_CleanupData.ucd_IconCount++;
			}

		d1(kprintf("%s/%s/%ld: ucd_IconCount=%lu\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CleanupData.ucd_IconCount));

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

			d1(kprintf("%s/%s/%ld: in=%08lx <%s>  Left=%ld  Top=%ld\n", \
				__FILE__, __FUNC__, __LINE__, in, GetIconName(in), \
				ucie->ucin_Left, ucie->ucin_Top));
			}

		ScalosUnLockIconList(uev->uev_Data.uev_CleanupData.ucd_WindowTask);

		d1(kprintf("%s/%s/%ld: ucd_WindowTask=%08lx\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CleanupData.ucd_WindowTask));

		Success = TRUE;
		} while (0);

	d1(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static BOOL AddRenameEvent(struct UndoEvent *uev, struct TagItem *TagList)
{
	BOOL Success = FALSE;
	STRPTR name;

	d1(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	do	{
		BPTR dirLock;
		CONST_STRPTR fName;
		static struct Hook UndoDisposeCopyMoveDataHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoDisposeCopyMoveData),	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook UndoRenameHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoRenameEvent),      	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook RedoRenameHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(RedoRenameEvent),      	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};

		uev->uev_DisposeHook = (struct Hook *) GetTagData(UNDOTAG_DisposeHook, (ULONG) &UndoDisposeCopyMoveDataHook, TagList);

		uev->uev_UndoHook = (struct Hook *) GetTagData(UNDOTAG_UndoHook, (ULONG) &UndoRenameHook, TagList);
		uev->uev_RedoHook = (struct Hook *) GetTagData(UNDOTAG_RedoHook, (ULONG) &RedoRenameHook, TagList);

		name = AllocPathBuffer();
		if (NULL == name)
			break;

		d1(kprintf("%s/%s/%ld: name=%08lx\n", __FILE__, __FUNC__, __LINE__, name));

		dirLock = (BPTR) GetTagData(UNDOTAG_CopySrcDirLock, BNULL, TagList);
		d1(kprintf("%s/%s/%ld: dirLock=%08lx\n", __FILE__, __FUNC__, __LINE__, dirLock));
		if (BNULL == dirLock)
			break;

		if (!NameFromLock(dirLock, name, Max_PathLen))
			break;

		d1(kprintf("%s/%s/%ld: UNDOTAG_CopySrcDirLock=<%s>\n", __FILE__, __FUNC__, __LINE__, name));

		uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName = AllocCopyString(name);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName)
			break;

		fName = (CONST_STRPTR) GetTagData(UNDOTAG_CopySrcName, (ULONG) NULL, TagList);
		if (NULL == fName)
			break;

		d1(kprintf("%s/%s/%ld: UNDOTAG_CopySrcName=<%s>\n", __FILE__, __FUNC__, __LINE__, fName));

		uev->uev_Data.uev_CopyMoveData.ucmed_srcName = AllocCopyString(fName);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_srcName)
			break;

		fName = (CONST_STRPTR) GetTagData(UNDOTAG_CopyDestName, (ULONG) NULL, TagList);
		if (NULL == fName)
			break;
		d1(kprintf("%s/%s/%ld: UNDOTAG_CopyDestName=<%s>\n", __FILE__, __FUNC__, __LINE__, fName));

		uev->uev_Data.uev_CopyMoveData.ucmed_destName = AllocCopyString(fName);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_destName)
			break;

		Success = TRUE;
		} while (0);

	if (name)
		FreePathBuffer(name);

	d1(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static BOOL AddRelabelEvent(struct UndoEvent *uev, struct TagItem *TagList)
{
	BOOL Success = FALSE;
	STRPTR name;

	d1(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	do	{
		CONST_STRPTR fName;
		static struct Hook UndoDisposeCopyMoveDataHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoDisposeCopyMoveData),	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook UndoRelabelHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoRelabelEvent),      	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook RedoRelabelHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(RedoRelabelEvent),      	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};

		uev->uev_DisposeHook = (struct Hook *) GetTagData(UNDOTAG_DisposeHook, (ULONG) &UndoDisposeCopyMoveDataHook, TagList);

		uev->uev_UndoHook = (struct Hook *) GetTagData(UNDOTAG_UndoHook, (ULONG) &UndoRelabelHook, TagList);
		uev->uev_RedoHook = (struct Hook *) GetTagData(UNDOTAG_RedoHook, (ULONG) &RedoRelabelHook, TagList);

		name = AllocPathBuffer();
		if (NULL == name)
			break;

		d1(kprintf("%s/%s/%ld: name=%08lx\n", __FILE__, __FUNC__, __LINE__, name));

		uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName = AllocCopyString(name);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName)
			break;

		fName = (CONST_STRPTR) GetTagData(UNDOTAG_CopySrcName, (ULONG) NULL, TagList);
		if (NULL == fName)
			break;

		// i.e. "Data1old:"
		d1(kprintf("%s/%s/%ld: UNDOTAG_CopySrcName=<%s>\n", __FILE__, __FUNC__, __LINE__, fName));

		uev->uev_Data.uev_CopyMoveData.ucmed_srcName = AllocCopyString(fName);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_srcName)
			break;

		fName = (CONST_STRPTR) GetTagData(UNDOTAG_CopyDestName, (ULONG) NULL, TagList);
		if (NULL == fName)
			break;

		// i.e. "Data1new"
		d1(kprintf("%s/%s/%ld: UNDOTAG_CopyDestName=<%s>\n", __FILE__, __FUNC__, __LINE__, fName));

		uev->uev_Data.uev_CopyMoveData.ucmed_destName = AllocCopyString(fName);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_destName)
			break;

		Success = TRUE;
		} while (0);

	if (name)
		FreePathBuffer(name);

	d1(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static BOOL AddDeleteEvent(struct UndoEvent *uev, struct TagItem *TagList)
{
	BOOL Success = FALSE;
	STRPTR name;

	d1(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	do	{
		BPTR dirLock;
		CONST_STRPTR fName;
		static struct Hook UndoDisposeCopyMoveDataHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoDisposeCopyMoveData),	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook UndoDeleteHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoMoveEvent),      	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook RedoDeleteHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(RedoMoveEvent),      	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};

		uev->uev_DisposeHook = (struct Hook *) GetTagData(UNDOTAG_DisposeHook, (ULONG) &UndoDisposeCopyMoveDataHook, TagList);

		uev->uev_UndoHook = (struct Hook *) GetTagData(UNDOTAG_UndoHook, (ULONG) &UndoDeleteHook, TagList);
		uev->uev_RedoHook = (struct Hook *) GetTagData(UNDOTAG_RedoHook, (ULONG) &RedoDeleteHook, TagList);

		name = AllocPathBuffer();
		if (NULL == name)
			break;

		d1(kprintf("%s/%s/%ld: name=%08lx\n", __FILE__, __FUNC__, __LINE__, name));

		dirLock = (BPTR) GetTagData(UNDOTAG_CopySrcDirLock, BNULL, TagList);
		d1(kprintf("%s/%s/%ld: dirLock=%08lx\n", __FILE__, __FUNC__, __LINE__, dirLock));
		if (BNULL == dirLock)
			break;

		if (!NameFromLock(dirLock, name, Max_PathLen))
			break;

		d1(kprintf("%s/%s/%ld: UNDOTAG_CopySrcDirLock=<%s>\n", __FILE__, __FUNC__, __LINE__, name));

		uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName = AllocCopyString(name);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName)
			break;

		fName = (CONST_STRPTR) GetTagData(UNDOTAG_CopySrcName, (ULONG) NULL, TagList);
		if (NULL == fName)
			break;

		d1(kprintf("%s/%s/%ld: UNDOTAG_CopySrcName=<%s>\n", __FILE__, __FUNC__, __LINE__, fName));

		uev->uev_Data.uev_CopyMoveData.ucmed_srcName = AllocCopyString(fName);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_srcName)
			break;

		fName = (CONST_STRPTR) GetTagData(UNDOTAG_CopyDestName, (ULONG) "SYS:Trashcan", TagList);
		if (NULL == fName)
			break;
		d1(kprintf("%s/%s/%ld: UNDOTAG_CopyDestName=<%s>\n", __FILE__, __FUNC__, __LINE__, fName));

		uev->uev_Data.uev_CopyMoveData.ucmed_destName = AllocCopyString(fName);
		if (NULL == uev->uev_Data.uev_CopyMoveData.ucmed_destName)
			break;

		Success = TRUE;
		} while (0);

	if (name)
		FreePathBuffer(name);

	d1(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static BOOL AddSizeWindowEvent(struct UndoEvent *uev, struct TagItem *TagList)
{
	BOOL Success = FALSE;

	d2(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	do	{
		static struct Hook UndoSizeWindowHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoSizeWindowEvent),  	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook RedoSizeWindowHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(RedoSizeWindowEvent),	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};

		uev->uev_UndoHook = (struct Hook *) GetTagData(UNDOTAG_UndoHook, (ULONG) &UndoSizeWindowHook, TagList);
		uev->uev_RedoHook = (struct Hook *) GetTagData(UNDOTAG_RedoHook, (ULONG) &RedoSizeWindowHook, TagList);

		uev->uev_Data.uev_SizeWindowData.uswd_WindowTask = (struct internalScaWindowTask *) GetTagData(UNDOTag_WindowTask, (ULONG) NULL, TagList);
		if (NULL == uev->uev_Data.uev_SizeWindowData.uswd_WindowTask)
			break;

		d2(kprintf("%s/%s/%ld: UNDOTag_WindowTask=%08lx\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SizeWindowData.uswd_WindowTask));

		uev->uev_Data.uev_SizeWindowData.uswd_OldLeft = GetTagData(UNDOTAG_OldWindowLeft, 0, TagList);
		uev->uev_Data.uev_SizeWindowData.uswd_OldTop = GetTagData(UNDOTAG_OldWindowTop, 0, TagList);
		uev->uev_Data.uev_SizeWindowData.uswd_OldWidth = GetTagData(UNDOTAG_OldWindowWidth, 0, TagList);
		uev->uev_Data.uev_SizeWindowData.uswd_OldHeight = GetTagData(UNDOTAG_OldWindowHeight, 0, TagList);
		uev->uev_Data.uev_SizeWindowData.uswd_OldVirtX = GetTagData(UNDOTAG_OldWindowVirtX, 0, TagList);
		uev->uev_Data.uev_SizeWindowData.uswd_OldVirtY = GetTagData(UNDOTAG_OldWindowVirtY, 0, TagList);

		uev->uev_Data.uev_SizeWindowData.uswd_NewLeft = GetTagData(UNDOTAG_NewWindowLeft, uev->uev_Data.uev_SizeWindowData.uswd_OldLeft, TagList);
		uev->uev_Data.uev_SizeWindowData.uswd_NewTop = GetTagData(UNDOTAG_NewWindowTop, uev->uev_Data.uev_SizeWindowData.uswd_OldTop, TagList);
		uev->uev_Data.uev_SizeWindowData.uswd_NewWidth = GetTagData(UNDOTAG_NewWindowWidth, uev->uev_Data.uev_SizeWindowData.uswd_OldWidth, TagList);
		uev->uev_Data.uev_SizeWindowData.uswd_NewHeight = GetTagData(UNDOTAG_NewWindowHeight, uev->uev_Data.uev_SizeWindowData.uswd_OldHeight, TagList);
		uev->uev_Data.uev_SizeWindowData.uswd_NewVirtX = GetTagData(UNDOTAG_NewWindowVirtX, uev->uev_Data.uev_SizeWindowData.uswd_OldVirtX, TagList);
		uev->uev_Data.uev_SizeWindowData.uswd_NewVirtY = GetTagData(UNDOTAG_NewWindowVirtY, uev->uev_Data.uev_SizeWindowData.uswd_OldVirtY, TagList);

		d2(kprintf("%s/%s/%ld: UNDOTAG_OldWindowLeft=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SizeWindowData.uswd_OldLeft));
		d2(kprintf("%s/%s/%ld: UNDOTAG_OldWindowTop=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SizeWindowData.uswd_OldTop));
		d2(kprintf("%s/%s/%ld: UNDOTAG_OldWindowWidth=%lu\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SizeWindowData.uswd_OldWidth));
		d2(kprintf("%s/%s/%ld: UNDOTAG_OldWindowHeight=%lu\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SizeWindowData.uswd_OldHeight));
		d2(kprintf("%s/%s/%ld: UNDOTAG_OldWindowVirtX=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SizeWindowData.uswd_OldVirtX));
		d2(kprintf("%s/%s/%ld: UNDOTAG_OldWindowVirtY=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SizeWindowData.uswd_OldVirtY));
		d2(kprintf("%s/%s/%ld: UNDOTAG_NewWindowLeft=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SizeWindowData.uswd_NewLeft));
		d2(kprintf("%s/%s/%ld: UNDOTAG_NewWindowTop=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SizeWindowData.uswd_NewTop));
		d2(kprintf("%s/%s/%ld: UNDOTAG_NewWindowWidth=%lu\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SizeWindowData.uswd_NewWidth));
		d2(kprintf("%s/%s/%ld: UNDOTAG_NewWindowHeight=%lu\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SizeWindowData.uswd_NewHeight));
		d2(kprintf("%s/%s/%ld: UNDOTAG_NewWindowVirtX=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SizeWindowData.uswd_NewVirtX));
		d2(kprintf("%s/%s/%ld: UNDOTAG_NewWindowVirtY=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SizeWindowData.uswd_NewVirtY));

		Success = TRUE;
		} while (0);

	d2(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static BOOL AddNewDrawerEvent(struct UndoEvent *uev, struct TagItem *TagList)
{
	BOOL Success = FALSE;
	STRPTR name;

	d2(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	do	{
		BPTR dirLock;
		CONST_STRPTR fName;
		static struct Hook UndoDisposeNewDrawerHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoDisposeNewDrawerData),	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook UndoNewDrawerHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(UndoNewDrawerEvent),      	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};
		static struct Hook RedoNewDrawerHook =
			{
			{ NULL, NULL },
			HOOKFUNC_DEF(RedoNewDrawerEvent),      	// h_Entry + h_SubEntry
			NULL,					// h_Data
			};

		uev->uev_DisposeHook = (struct Hook *) GetTagData(UNDOTAG_DisposeHook, (ULONG) &UndoDisposeNewDrawerHook, TagList);

		uev->uev_UndoHook = (struct Hook *) GetTagData(UNDOTAG_UndoHook, (ULONG) &UndoNewDrawerHook, TagList);
		uev->uev_RedoHook = (struct Hook *) GetTagData(UNDOTAG_RedoHook, (ULONG) &RedoNewDrawerHook, TagList);

		name = AllocPathBuffer();
		if (NULL == name)
			break;

		d2(kprintf("%s/%s/%ld: name=%08lx\n", __FILE__, __FUNC__, __LINE__, name));

		dirLock = (BPTR) GetTagData(UNDOTAG_CopySrcDirLock, BNULL, TagList);
		d2(kprintf("%s/%s/%ld: dirLock=%08lx\n", __FILE__, __FUNC__, __LINE__, dirLock));
		if (BNULL == dirLock)
			break;

		if (!NameFromLock(dirLock, name, Max_PathLen))
			break;

		d2(kprintf("%s/%s/%ld: UNDOTAG_CopySrcDirLock=<%s>\n", __FILE__, __FUNC__, __LINE__, name));

		uev->uev_Data.uev_NewDrawerData.und_DirName = AllocCopyString(name);
		if (NULL == uev->uev_Data.uev_NewDrawerData.und_DirName)
			break;

		fName = (CONST_STRPTR) GetTagData(UNDOTAG_CopySrcName, (ULONG) NULL, TagList);
		if (NULL == fName)
			break;

		d2(kprintf("%s/%s/%ld: UNDOTAG_CopySrcName=<%s>\n", __FILE__, __FUNC__, __LINE__, fName));

		uev->uev_Data.uev_NewDrawerData.und_srcName = AllocCopyString(fName);
		if (NULL == uev->uev_Data.uev_NewDrawerData.und_srcName)
			break;

		uev->uev_Data.uev_NewDrawerData.und_CreateIcon = GetTagData(UNDOTAG_CreateIcon, TRUE, TagList);

		d2(kprintf("%s/%s/%ld: UNDOTAG_CreateIcon=%ld\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_NewDrawerData.und_CreateIcon));

		Success = TRUE;
		} while (0);

	if (name)
		FreePathBuffer(name);

	d2(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoCopyEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	BOOL Success = FALSE;
	BPTR destDirLock;
	STRPTR iconName = NULL;

	(void) hook;
	(void) object;

	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	do	{
		STRPTR destName;
		BPTR oldDir;
		LONG Result;
		LONG IconResult;
		struct ScaUpdateIcon_IW upd;

		destDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_destDirName, ACCESS_READ);
		d1(kprintf("%s/%s/%ld: destDirLock=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, destDirLock, uev->uev_Data.uev_CopyMoveData.ucmed_destDirName));
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
		d1(kprintf("%s/%s/%ld: ucmed_destName=<%s>  Result=%ld  IoErr=%ld\n", __FILE__, __FUNC__, \
			__LINE__, destName, Result, IoErr()));

		// try to also to remove associated icon
		IconResult = DeleteFile(iconName);
		d1(kprintf("%s/%s/%ld: IconResult=%ld\n", __FILE__, __FUNC__, __LINE__, IconResult));

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
	BPTR srcDirLock;
	BPTR destDirLock = BNULL;

	(void) hook;
	(void) object;

	d1(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	do	{
		ULONG Result;

		srcDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName, ACCESS_READ);
		if (BNULL == srcDirLock)
			break;

		destDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_destDirName, ACCESS_READ);
		if (BNULL == destDirLock)
			break;

		Result = DoMethod(uev->uev_UndoStep->ust_FileTransObj, SCCM_FileTrans_Copy,
			srcDirLock, destDirLock,
			uev->uev_Data.uev_CopyMoveData.ucmed_srcName,
			uev->uev_Data.uev_CopyMoveData.ucmed_destName,
			uev->uev_NewPosX, uev->uev_NewPosY);
		if (RETURN_OK != Result)
			break;

		Success = TRUE;
		} while (0);

	if (BNULL != srcDirLock)
		UnLock(srcDirLock);
	if (BNULL != destDirLock)
		UnLock(destDirLock);

	d1(kprintf("%s/%s/%ld:  END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

// Move moved file system object back to original position
static SAVEDS(LONG) UndoMoveEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	BOOL Success = FALSE;
	BPTR srcDirLock;
	BPTR destDirLock = BNULL;

	(void) hook;
	(void) object;

	d1(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	do	{
		ULONG Result;

		d1(kprintf("%s/%s/%ld: ucmed_srcDirName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName));

		srcDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName, ACCESS_READ);
		if (BNULL == srcDirLock)
			break;

		debugLock_d1(srcDirLock);

		d1(kprintf("%s/%s/%ld: ucmed_destDirName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CopyMoveData.ucmed_destDirName));

		destDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_destDirName, ACCESS_READ);
		if (BNULL == destDirLock)
			break;

		debugLock_d1(destDirLock);

		d1(kprintf("%s/%s/%ld: .ucmed_srcName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CopyMoveData.ucmed_srcName));

		Result = DoMethod(uev->uev_UndoStep->ust_FileTransObj, SCCM_FileTrans_Move,
			destDirLock, srcDirLock,
			uev->uev_Data.uev_CopyMoveData.ucmed_srcName,
			uev->uev_OldPosX, uev->uev_OldPosY);
		if (RETURN_OK != Result)
			break;

		d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

		Success = TRUE;
		} while (0);

	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	if (BNULL != srcDirLock)
		UnLock(srcDirLock);
	if (BNULL != destDirLock)
		UnLock(destDirLock);

	d1(kprintf("%s/%s/%ld:  END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

// Repeat Move of moved file system object
static SAVEDS(LONG) RedoMoveEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	BOOL Success = FALSE;
	BPTR srcDirLock;
	BPTR destDirLock = BNULL;

	(void) hook;
	(void) object;

	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	do	{
		ULONG Result;

		srcDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName, ACCESS_READ);
		if (BNULL == srcDirLock)
			break;

		destDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_destDirName, ACCESS_READ);
		if (BNULL == destDirLock)
			break;

		Result = DoMethod(uev->uev_UndoStep->ust_FileTransObj, SCCM_FileTrans_Move,
			srcDirLock, destDirLock,
			uev->uev_Data.uev_CopyMoveData.ucmed_srcName,
			uev->uev_NewPosX, uev->uev_NewPosY);
		if (RETURN_OK != Result)
			break;

		Success = TRUE;
		} while (0);

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
	BPTR srcDirLock;
	BPTR destDirLock = BNULL;

	(void) hook;
	(void) object;

	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	do	{
		ULONG Result;

		srcDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName, ACCESS_READ);
		if (BNULL == srcDirLock)
			break;

		destDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_destDirName, ACCESS_READ);
		if (BNULL == destDirLock)
			break;

		Result = DoMethod(uev->uev_UndoStep->ust_FileTransObj, SCCM_FileTrans_CreateLink,
			srcDirLock, destDirLock,
			uev->uev_Data.uev_CopyMoveData.ucmed_srcName,
			uev->uev_NewPosX, uev->uev_NewPosY);
		if (RETURN_OK != Result)
			break;

		Success = TRUE;
		} while (0);

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
				d1(KPrintF("%s/%s/%ld: Found  iwt=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, iwt, iwt->iwt_WinTitle));
				break;
				}
			}

		if (NULL == iwt)
			SCA_UnLockWindowList();

		UnLock(DirLock);
		}

	d1(kprintf("%s/%s/%ld: iwt=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt));

	return iwt;
}

//----------------------------------------------------------------------------

static struct internalScaWindowTask *UndoFindWindowByWindowTask(const struct internalScaWindowTask *iwt)
{
	struct ScaWindowStruct *ws;

	SCA_LockWindowList(SCA_LockWindowList_Shared);

	d1(kprintf("%s/%s/%ld: START  iwt=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt));
	debugLock_d1(DirLock);

	for (ws=winlist.wl_WindowStruct; ws; ws = (struct ScaWindowStruct *) ws->ws_Node.mln_Succ)
		{
		if (iwt == (struct internalScaWindowTask *) ws->ws_WindowTask)
			{
			d1(KPrintF("%s/%s/%ld: Found  iwt=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, iwt, iwt->iwt_WinTitle));
			return (struct internalScaWindowTask *) ws->ws_WindowTask;
			}
		}

	SCA_UnLockWindowList();

	d1(kprintf("%s/%s/%ld: iwt=NULL\n", __FILE__, __FUNC__, __LINE__));

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
			d1(kprintf("%s/%s/%ld: inFound=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, inFound, GetIconName(inFound)));
			break;
			}
		}

	if (NULL == inFound)
		ScalosUnLockIconList(iwt);

	d1(kprintf("%s/%s/%ld: inFound=%08lx\n", __FILE__, __FUNC__, __LINE__, inFound));

	return inFound;
}

//----------------------------------------------------------------------------

static BOOL UndoChangeIconPosEvent(struct UndoEvent *uev)
{
	d1(kprintf("%s/%s/%ld: uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));
	return MoveIconTo(uev->uev_Data.uev_IconData.uid_DirName,
		uev->uev_Data.uev_IconData.uid_IconName,
		uev->uev_OldPosX,
		uev->uev_OldPosY);
}

//----------------------------------------------------------------------------

static BOOL RedoChangeIconPosEvent(struct UndoEvent *uev)
{
	d1(kprintf("%s/%s/%ld: uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));
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

	d1(kprintf("%s/%s/%ld: START  uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));
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
	d1(kprintf("%s/%s/%ld: END   uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));

	return 0;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoDisposeNewDrawerData(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	(void) hook;
	(void) object;

	d1(kprintf("%s/%s/%ld: START  uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));
	if (uev)
		{
		if (uev->uev_Data.uev_NewDrawerData.und_DirName)
			{
			FreeCopyString(uev->uev_Data.uev_NewDrawerData.und_DirName);
			uev->uev_Data.uev_NewDrawerData.und_DirName = NULL;
			}
		if (uev->uev_Data.uev_NewDrawerData.und_srcName)
			{
			FreeCopyString(uev->uev_Data.uev_NewDrawerData.und_srcName);
			uev->uev_Data.uev_NewDrawerData.und_srcName = NULL;
			}
		}
	d1(kprintf("%s/%s/%ld: END   uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));

	return 0;
}
//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoDisposeIconData(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	(void) hook;
	(void) object;

	d1(kprintf("%s/%s/%ld: START  uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));
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
	d1(kprintf("%s/%s/%ld: END   uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));

	return 0;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoDisposeCleanupData(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	(void) hook;
	(void) object;

	d1(kprintf("%s/%s/%ld: START  uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));
	if (uev)
		{
		if (uev->uev_Data.uev_CleanupData.ucd_Icons)
			{
			ScalosFreeVecPooled(uev->uev_Data.uev_CleanupData.ucd_Icons);
			uev->uev_Data.uev_CleanupData.ucd_Icons = NULL;
			}
		}
	d1(kprintf("%s/%s/%ld: END   uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));

	return 0;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoDisposeSnapshotData(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	(void) hook;
	(void) object;

	d1(kprintf("%s/%s/%ld: START  uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));
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
	d1(kprintf("%s/%s/%ld: END   uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));

	return 0;
}
//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoCleanupEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	struct internalScaWindowTask *iwt;
	BOOL Success = FALSE;

	(void) hook;
	(void) object;

	d1(kprintf("%s/%s/%ld: START  uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));

	do	{
		struct ScaIconNode *in;

		d1(kprintf("%s/%s/%ld: ucd_IconCount=%lu\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CleanupData.ucd_IconCount));
		d1(kprintf("%s/%s/%ld: ucd_WindowTask=%08lx\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CleanupData.ucd_WindowTask));

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

				d1(kprintf("%s/%s/%ld: in=%08lx <%s>  Left=%ld  Top=%ld\n", \
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

	d1(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__));

	return Success;
}

//----------------------------------------------------------------------------

static BOOL RedoCleanupEvent(struct UndoEvent *uev)
{
	BOOL Success = FALSE;
	struct internalScaWindowTask *iwt;

	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

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

	d1(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__));

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

	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	do	{
		const struct IBox *pWinRect;
		ULONG IconViewMode;
		ULONG ddFlags;
		LONG WinCurrentX, WinCurrentY;

		iwt = UndoFindWindowByDir(uev->uev_Data.uev_SnapshotData.usid_DirName);
		if (NULL == iwt)
			break;

		d1(kprintf("%s/%s/%ld: iwt=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt));
		d1(kprintf("%s/%s/%ld: usid_IconName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SnapshotData.usid_IconName));

		in = UndoFindIconByName(iwt, uev->uev_Data.uev_SnapshotData.usid_IconName);
		if (NULL == in)
			break;

		d1(kprintf("%s/%s/%ld: in=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, in, GetIconName(in)));


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

	d1(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) RedoSnapshotPosEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	struct internalScaWindowTask *iwt;
	struct ScaIconNode *in = NULL;
	BOOL Success = FALSE;

	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	do	{
		iwt = UndoFindWindowByDir(uev->uev_Data.uev_SnapshotData.usid_DirName);
		if (NULL == iwt)
			break;

		d1(kprintf("%s/%s/%ld: iwt=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt));
		d1(kprintf("%s/%s/%ld: usid_IconName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_SnapshotData.usid_IconName));

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

	d1(kprintf("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static BOOL UndoSnapshotIcon(struct internalScaWindowTask *iwt, struct ScaIconNode *in)
{
	BOOL Success = FALSE;
	ULONG IconType = 0;

	d1(KPrintF("%s/%s/%ld:  START iwt=%08lx  in=%08lx <%s>\n", __FILE__, __FUNC__, __LINE__, iwt, in, GetIconName(in)));

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

	d1(KPrintF("%s/%s/%ld: END in=%08lx  <%s>  inFlags=%08lx  Success=%ld\n", __FILE__, __FUNC__, __LINE__, in, GetIconName(in), in->in_Flags, Success));

	return Success;
}

//----------------------------------------------------------------------------

static BOOL UndoUnsnapshotIcon(struct internalScaWindowTask *iwt,
	struct ScaIconNode *in, BOOL SaveIcon)
{
	BOOL Success = FALSE;
	ULONG IconType = 0;

	d1(KPrintF("%s/%s/%ld:  iwt=%08lx  in=%08lx <%s>\n", __FILE__, __FUNC__, __LINE__, iwt, in, GetIconName(in)));

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

	d1(KPrintF("%s/%s/%ld: END in=%08lx  <%s>  inFlags=%08lx  Success=%ld\n", __FILE__, __FUNC__, __LINE__, in, GetIconName(in), in->in_Flags, Success));

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoRenameEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	BPTR srcDirLock;
	BOOL Success = FALSE;
	STRPTR destIconName = NULL;
	STRPTR srcIconName = NULL;

	(void) hook;
	(void) object;

	d1(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	do	{
		BPTR oldDir;

		d1(kprintf("%s/%s/%ld: ucmed_srcDirName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName));

		srcDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName, ACCESS_READ);
		if (BNULL == srcDirLock)
			break;

		debugLock_d1(srcDirLock);

		destIconName = UndoBuildIconName(uev->uev_Data.uev_CopyMoveData.ucmed_destName);
		if (NULL == destIconName)
			break;

		srcIconName = UndoBuildIconName(uev->uev_Data.uev_CopyMoveData.ucmed_srcName);
		if (NULL == srcIconName)
			break;

		d1(kprintf("%s/%s/%ld: .ucmed_srcName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CopyMoveData.ucmed_srcName));
		d1(kprintf("%s/%s/%ld: .ucmed_destName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CopyMoveData.ucmed_destName));

		oldDir = CurrentDir(srcDirLock);

		Success = Rename(uev->uev_Data.uev_CopyMoveData.ucmed_destName, uev->uev_Data.uev_CopyMoveData.ucmed_srcName);

		CurrentDir(oldDir);

		if (!Success)
			break;

		d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

		// Try to also rename icon
		oldDir = CurrentDir(srcDirLock);
		(void) Rename(destIconName, srcIconName);
		CurrentDir(oldDir);
		} while (0);

	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	if (BNULL != srcDirLock)
		UnLock(srcDirLock);
	if (srcIconName)
		FreePathBuffer(srcIconName);
	if (destIconName)
		FreePathBuffer(destIconName);

	d1(kprintf("%s/%s/%ld:  END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) RedoRenameEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	BPTR srcDirLock;
	BOOL Success = FALSE;
	STRPTR destIconName = NULL;
	STRPTR srcIconName = NULL;

	(void) hook;
	(void) object;

	d1(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	do	{
		BPTR oldDir;

		d1(kprintf("%s/%s/%ld: ucmed_srcDirName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName));

		srcDirLock = Lock(uev->uev_Data.uev_CopyMoveData.ucmed_srcDirName, ACCESS_READ);
		if (BNULL == srcDirLock)
			break;

		debugLock_d1(srcDirLock);

		destIconName = UndoBuildIconName(uev->uev_Data.uev_CopyMoveData.ucmed_destName);
		if (NULL == destIconName)
			break;

		srcIconName = UndoBuildIconName(uev->uev_Data.uev_CopyMoveData.ucmed_srcName);
		if (NULL == srcIconName)
			break;

		d1(kprintf("%s/%s/%ld: .ucmed_srcName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CopyMoveData.ucmed_srcName));
		d1(kprintf("%s/%s/%ld: .ucmed_destName=<%s>\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CopyMoveData.ucmed_destName));

		oldDir = CurrentDir(srcDirLock);

		Success = Rename(uev->uev_Data.uev_CopyMoveData.ucmed_srcName, uev->uev_Data.uev_CopyMoveData.ucmed_destName);

		CurrentDir(oldDir);

		if (!Success)
			break;

		d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

		// Try to also rename icon
		oldDir = CurrentDir(srcDirLock);
		(void) Rename(srcIconName, destIconName);
		CurrentDir(oldDir);
		} while (0);

	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	if (BNULL != srcDirLock)
		UnLock(srcDirLock);
	if (srcIconName)
		FreePathBuffer(srcIconName);
	if (destIconName)
		FreePathBuffer(destIconName);

	d1(kprintf("%s/%s/%ld:  END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoRelabelEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	BOOL Success = FALSE;
	STRPTR newName = NULL;
	STRPTR driveName;

	(void) hook;
	(void) object;

	d1(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	do	{
		driveName = AllocPathBuffer();
		if (NULL == driveName)
			break;

		newName = AllocPathBuffer();
		if (NULL == newName)
			break;

		stccpy(driveName, uev->uev_Data.uev_CopyMoveData.ucmed_destName, Max_PathLen);
		SafeStrCat(driveName, ":", Max_PathLen);

		stccpy(newName, uev->uev_Data.uev_CopyMoveData.ucmed_srcName, Max_PathLen);
		StripTrailingColon(newName);

		Success = Relabel(driveName, newName);
		} while (0);

	if (driveName)
		FreePathBuffer(driveName);
	if (newName)
		FreePathBuffer(newName);

	d1(kprintf("%s/%s/%ld:  END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) RedoRelabelEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	BOOL Success;

	(void) hook;
	(void) object;

	d2(kprintf("%s/%s/%ld: START  uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));

	do	{
		Success	= Relabel(uev->uev_Data.uev_CopyMoveData.ucmed_srcName,
			 uev->uev_Data.uev_CopyMoveData.ucmed_destName);
		} while (0);

	d2(kprintf("%s/%s/%ld:  END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoSizeWindowEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	struct internalScaWindowTask *iwt;
	BOOL Success = FALSE;

	(void) hook;
	(void) object;

	d2(kprintf("%s/%s/%ld: START  uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));

	do	{
		d2(kprintf("%s/%s/%ld: ucd_WindowTask=%08lx\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CleanupData.ucd_WindowTask));

		iwt = UndoFindWindowByWindowTask(uev->uev_Data.uev_SizeWindowData.uswd_WindowTask);
		if (NULL == iwt)
			break;

		ScalosObtainSemaphoreShared(iwt->iwt_WindowTask.wt_WindowSemaphore);

		d2(kprintf("%s/%s/%ld: XOffset=%ld  YOffset=%ld\n", __FILE__, __FUNC__, __LINE__, \
			iwt->iwt_WindowTask.wt_XOffset, iwt->iwt_WindowTask.wt_YOffset));

		DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_DeltaMove,
			uev->uev_Data.uev_SizeWindowData.uswd_OldVirtX - iwt->iwt_WindowTask.wt_XOffset,
			uev->uev_Data.uev_SizeWindowData.uswd_OldVirtY - iwt->iwt_WindowTask.wt_YOffset);

		d2(KPrintF("%s/%s/%ld: Left=%ld  Top=%ld  Width=%ld  Height=%ld\n", __FILE__, __FUNC__, __LINE__, \
			uev->uev_Data.uev_SizeWindowData.uswd_OldLeft, uev->uev_Data.uev_SizeWindowData.uswd_OldTop, \
			uev->uev_Data.uev_SizeWindowData.uswd_OldWidth, uev->uev_Data.uev_SizeWindowData.uswd_OldHeight));

		if (iwt->iwt_WindowTask.wt_Window)
			{
			ChangeWindowBox(iwt->iwt_WindowTask.wt_Window,
				uev->uev_Data.uev_SizeWindowData.uswd_OldLeft,
				uev->uev_Data.uev_SizeWindowData.uswd_OldTop,
				uev->uev_Data.uev_SizeWindowData.uswd_OldWidth,
				uev->uev_Data.uev_SizeWindowData.uswd_OldHeight);
			}

		ScalosReleaseSemaphore(iwt->iwt_WindowTask.wt_WindowSemaphore);

		Success = TRUE;
		} while (0);

	if (iwt)
		SCA_UnLockWindowList();

	d2(kprintf("%s/%s/%ld:  END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) RedoSizeWindowEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	struct internalScaWindowTask *iwt;
	BOOL Success = FALSE;

	(void) hook;
	(void) object;

	d2(kprintf("%s/%s/%ld: START  uev=%08lx\n", __FILE__, __FUNC__, __LINE__, uev));

	do	{
		d2(kprintf("%s/%s/%ld: ucd_WindowTask=%08lx\n", __FILE__, __FUNC__, __LINE__, uev->uev_Data.uev_CleanupData.ucd_WindowTask));

		iwt = UndoFindWindowByWindowTask(uev->uev_Data.uev_SizeWindowData.uswd_WindowTask);
		if (NULL == iwt)
			break;

		ScalosObtainSemaphoreShared(iwt->iwt_WindowTask.wt_WindowSemaphore);

		d2(kprintf("%s/%s/%ld: XOffset=%ld  YOffset=%ld\n", __FILE__, __FUNC__, __LINE__, \
			iwt->iwt_WindowTask.wt_XOffset, iwt->iwt_WindowTask.wt_YOffset));

		DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_DeltaMove,
			uev->uev_Data.uev_SizeWindowData.uswd_NewVirtX - iwt->iwt_WindowTask.wt_XOffset,
			uev->uev_Data.uev_SizeWindowData.uswd_NewVirtY - iwt->iwt_WindowTask.wt_YOffset);

		d2(KPrintF("%s/%s/%ld: Left=%ld  Top=%ld  Width=%ld  Height=%ld\n", __FILE__, __FUNC__, __LINE__, \
			uev->uev_Data.uev_SizeWindowData.uswd_NewLeft, uev->uev_Data.uev_SizeWindowData.uswd_NewTop, \
			uev->uev_Data.uev_SizeWindowData.uswd_NewWidth, uev->uev_Data.uev_SizeWindowData.uswd_NewHeight));

		if (iwt->iwt_WindowTask.wt_Window)
			{
			ChangeWindowBox(iwt->iwt_WindowTask.wt_Window,
				uev->uev_Data.uev_SizeWindowData.uswd_NewLeft,
				uev->uev_Data.uev_SizeWindowData.uswd_NewTop,
				uev->uev_Data.uev_SizeWindowData.uswd_NewWidth,
				uev->uev_Data.uev_SizeWindowData.uswd_NewHeight);
			}

		ScalosReleaseSemaphore(iwt->iwt_WindowTask.wt_WindowSemaphore);

		Success = TRUE;
		} while (0);

	if (iwt)
		SCA_UnLockWindowList();

	d2(kprintf("%s/%s/%ld:  END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) UndoNewDrawerEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	BOOL Success = FALSE;
	BPTR srcDirLock;
	STRPTR IconName = NULL;

	(void) hook;
	(void) object;

	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	do	{
		BPTR oldDir;
		LONG Result;

		srcDirLock = Lock(uev->uev_Data.uev_NewDrawerData.und_DirName, ACCESS_READ);
		d1(kprintf("%s/%s/%ld: srcDirLock=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, srcDirLock, uev->uev_Data.uev_NewDrawerData.und_DirName));
		if (BNULL == srcDirLock)
			break;

		if (0 == strlen(uev->uev_Data.uev_NewDrawerData.und_srcName))
			break;

		oldDir = CurrentDir(srcDirLock);

		Result = DeleteFile(uev->uev_Data.uev_NewDrawerData.und_srcName);
		if (!Result)
			break;

		if (uev->uev_Data.uev_NewDrawerData.und_CreateIcon)
			{
			// Remove associated icon
			struct ScaUpdateIcon_IW upd;

			IconName = UndoBuildIconName(uev->uev_Data.uev_NewDrawerData.und_srcName);
			if (NULL == IconName)
				break;

			Result = DeleteFile(IconName);
			if (!Result)
				break;

			upd.ui_iw_Lock = srcDirLock;
			upd.ui_iw_Name = uev->uev_Data.uev_NewDrawerData.und_srcName;

			SCA_UpdateIcon(WSV_Type_IconWindow, &upd, sizeof(upd));
			}

		CurrentDir(oldDir);

		if (!Result)
			break;	// DeleteFile failed

		Success = TRUE;
		} while (0);

	if (BNULL != srcDirLock)
		UnLock(srcDirLock);
	if (IconName)
		FreePathBuffer(IconName);

	return Success;
}

//----------------------------------------------------------------------------

static SAVEDS(LONG) RedoNewDrawerEvent(struct Hook *hook, APTR object, struct UndoEvent *uev)
{
	BOOL Success = FALSE;
	BPTR srcDirLock;
	BPTR newDirLock = BNULL;
	Object *IconObj = NULL;
	BPTR oldDir = BNULL;

	(void) hook;
	(void) object;

	d2(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	do	{
		srcDirLock = Lock(uev->uev_Data.uev_NewDrawerData.und_DirName, ACCESS_READ);
		if (BNULL == srcDirLock)
			break;

		oldDir = CurrentDir(srcDirLock);

		newDirLock = CreateDir(uev->uev_Data.uev_NewDrawerData.und_srcName);
		debugLock_d2(newDirLock);
		if (NULL == newDirLock)
			break;

		if (uev->uev_Data.uev_NewDrawerData.und_CreateIcon)
			{
			struct ScaUpdateIcon_IW upd;

			IconObj	= GetDefIconObject(WBDRAWER, NULL);

			d2(kprintf(__FILE__ "/%s/%ld: IconObj=%08lx\n", __FUNC__, __LINE__, IconObj));

			if (NULL == IconObj)
				break;

			upd.ui_iw_Lock = srcDirLock;
			upd.ui_iw_Name = uev->uev_Data.uev_NewDrawerData.und_srcName;

			PutIconObjectTags(IconObj, uev->uev_Data.uev_NewDrawerData.und_srcName,
				TAG_END);

			SCA_UpdateIcon(WSV_Type_IconWindow, &upd, sizeof(upd));
			}

		Success = TRUE;
		} while (0);

	if (oldDir)
		CurrentDir(oldDir);
	if (IconObj)
		DisposeIconObject(IconObj);
	if (BNULL != srcDirLock)
		UnLock(srcDirLock);
	if (BNULL != newDirLock)
		UnLock(newDirLock);

	d2(kprintf("%s/%s/%ld:  END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

