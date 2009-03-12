// ScanDir.c
// $Date$
// $Revision$
// $Id$

#include <exec/types.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <utility/hooks.h>
#include <intuition/gadgetclass.h>
#include <datatypes/pictureclass.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/exall.h>

#define	__USE_SYSBASE

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/iconobject.h>
#include <proto/utility.h>
#include <proto/gadtools.h>
#include <proto/datatypes.h>
#include "debug.h"
#include <proto/scalos.h>

#include <clib/alib_protos.h>

#include <defs.h>
#include <datatypes/iconobject.h>
#include <scalos/scalos.h>

#include <DefIcons.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "scalos_structures.h"
#include "locale.h"
#include "functions.h"
#include "Variables.h"

//----------------------------------------------------------------------------

// local data definitions

//----------------------------------------------------------------------------

// local functions

static BOOL IsBackDropIconTextWindow(struct internalScaWindowTask *iwt, struct BackDropList *bdl,
	BPTR fLock, CONST_STRPTR FileName, ULONG pos);
static enum ScanDirResult TextCheckCleanup(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc);
static enum ScanDirResult GenerateTextIcons(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc, BOOL Final);
static struct ScaIconNode *GetTextIcon(struct internalScaWindowTask *iwt,
	struct Hook *ColumnChangeHook, struct IconScanEntry *ise, 
	struct BackDropList *bdl, const struct ReadIconListData *rild);
static void ResolveLink(struct ExAllData *ead);
static LONG ReExamine(struct ReadIconListData *rild);

//----------------------------------------------------------------------------

// local data

//----------------------------------------------------------------------------

// public data items

//----------------------------------------------------------------------------

// Result : Success
struct ScaIconNode *TextWindowReadIcon(struct internalScaWindowTask *iwt, 
	CONST_STRPTR Name, struct ScaReadIconArg *ria)
{
	struct ScaIconNode *in = NULL;
	ULONG err;
	struct FileInfoBlock *fib = NULL;
	BPTR oldDir = NOT_A_LOCK;
	BPTR dirLock;
	BOOL VolumeIsWritable;
	BOOL ScanDirSemaphoreLocked = FALSE;
	struct BackDropList bdl;


	do	{
		BPTR iLock;
		ULONG pos;
		BOOL hasIcon = FALSE;
		struct ReadIconListData rild;

		d1(KPrintF("%s/%s/%ld: Name=<%s>  ria=%08lx  iwt=%08lx\n", __FILE__, __FUNC__, __LINE__, Name, ria, iwt));

		InitBackDropList(&bdl);

		if (NULL == iwt->iwt_WindowTask.wt_Window)
			break;

		if (!AttemptSemaphoreNoNest(&iwt->iwt_ScanDirSemaphore))
			break;

		ScanDirSemaphoreLocked = TRUE;

		if (!IsShowAll(iwt->iwt_WindowTask.mt_WindowStruct) && 0 != IsIconName(Name))
			{
			d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
			break;
			}

		fib = AllocDosObject(DOS_FIB, NULL);
		if (NULL == fib)
			{
			d1(kprintf("%s/%s/%ld: AllocDosObject failed\n", __FILE__, __FUNC__, __LINE__));
			break;
			}

		if (ria && ria->ria_Lock)
			dirLock = ria->ria_Lock;
		else
			dirLock = iwt->iwt_WindowTask.mt_WindowStruct->ws_Lock;

		debugLock_d1(dirLock);
		if ((BPTR)NULL == dirLock)
			break;

		VolumeIsWritable = ClassIsDiskWritable(dirLock);

		oldDir = CurrentDir(dirLock);

		err = RETURN_OK;
		iLock = Lock((STRPTR) Name, ACCESS_READ);
		if (BNULL == iLock)
			err = IoErr();
		else
			{
			if (!ScalosExamine64(iLock, fib))
				err = IoErr();
			UnLock(iLock);
			}

		d1(kprintf("%s/%s/%ld: MatchFirst returned %ld\n", __FILE__, __FUNC__, __LINE__, err));

		if (RETURN_OK != err)
			{
			fib->fib_DirEntryType = MAKE_ID('E','R','R','O');
			stccpy(fib->fib_FileName, Name, sizeof(fib->fib_FileName));
			}

		rild.rild_SoloIcon = FALSE;

		if (ERROR_OBJECT_NOT_FOUND == err)
			{
			// object doesn't exist - check if associated icon is there
			BOOL iconExists = FALSE;
			STRPTR iconName = AllocPathBuffer();

			d1(kprintf("%s/%s/%ld: iconName=%08lx\n", __FILE__, __FUNC__, __LINE__, iconName));

			if (iconName)
				{
				BPTR iLock;

				stccpy(iconName, Name, Max_PathLen);
				SafeStrCat(iconName, ".info", Max_PathLen);

				iLock = Lock(iconName, ACCESS_READ);
				if (iLock)
					{
					rild.rild_SoloIcon = TRUE;
					ScalosExamine64(iLock, fib);
					iconExists = TRUE;
					UnLock(iLock);
					}

				FreePathBuffer(iconName);
				}

			if (!iconExists)
				{
				break;
				}
			}

		if (IsFileHidden(fib))
			{
			d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
			break;
			}

		stccpy(rild.rild_Name, fib->fib_FileName, sizeof(rild.rild_Name));
		stccpy(rild.rild_Comment, fib->fib_Comment, sizeof(rild.rild_Comment));
		rild.rild_Type = fib->fib_DirEntryType;
		rild.rild_Protection = fib->fib_Protection;
		rild.rild_Size64 = ScalosFibSize64(fib);
		rild.rild_DateStamp = fib->fib_Date;
		rild.rild_DiskWriteProtected = !VolumeIsWritable;
		rild.rild_CheckOverlap = TRUE;

		ReExamine(&rild);

		pos = IsIconName(rild.rild_Name);

		rild.rild_TypeNode = DefIconsIdentify(iwt->iwt_WindowTask.mt_WindowStruct->ws_Lock, rild.rild_Name);

		d1(kprintf("%s/%s/%ld:  Name=<%s>  pos=%08lx\n", __FILE__, __FUNC__, __LINE__, rild.rild_Name, pos));

		// Read Text Icon
		d1(kprintf("%s/%s/%ld: Name=<%s>\n", __FILE__, __FUNC__, __LINE__, fib->fib_FileName));

		if (IsShowAllType(iwt->iwt_OldShowType))
			{
			in = GetTextIcon(iwt, &iwt->iwt_ColumnWidthChangeHook, NULL, &bdl, &rild);

			d1(kprintf("%s/%s/%ld: in=%08lx\n", __FILE__, __FUNC__, __LINE__, in));

			if (0 != pos && ~0 != pos)
				{
				// rild.rild_Name IS an icon
				hasIcon = TRUE;
				}
			else
				{
				// we need to check whether an icon (.info) is present
				STRPTR IconFileName = AllocPathBuffer();

				if (IconFileName)
					{
					BPTR IconLock;

					stccpy(IconFileName, rild.rild_Name, MAX_FileName);
					SafeStrCat(IconFileName, ".info", MAX_FileName);
					if (IconLock = Lock(IconFileName, ACCESS_READ))
						{
						hasIcon = TRUE;
						UnLock(IconLock);
						}
					FreePathBuffer(IconFileName);
					}
				}

			if (in && 0 == pos)
				{
				ClassSetDefaultIconFlags(in, !hasIcon);
				d1(kprintf("%s/%s/%ld: in=<%s>  pos=%08lx  hasIcon=%ld\n", \
					__FILE__, __FUNC__, __LINE__, GetIconName(in), pos, hasIcon));
				}
			}
		else
			{
			d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

			if (rild.rild_SoloIcon && 0 != pos && ~0 != pos)
				{
				// strip ".info" from name
				*((char *) pos) = '\0';
				}

			in = GetTextIcon(iwt, &iwt->iwt_ColumnWidthChangeHook, NULL, &bdl, &rild);
			hasIcon = TRUE;

			d1(kprintf("%s/%s/%ld: in=%08lx\n", __FILE__, __FUNC__, __LINE__, in));
			}

		d1(KPrintF("%s/%s/%ld: in=%08lx\n", __FILE__, __FUNC__, __LINE__, in));

		if (in)
			{
			in->in_SupportFlags |= INF_SupportsOpen | INF_SupportsCopy | INF_SupportsInformation;

			d1(KPrintF("%s/%s/%ld: in=%08lx  hasIcon=%ld VolumeIsWritable=%ld\n", __FILE__, __FUNC__, __LINE__, in, hasIcon, VolumeIsWritable));

			if (hasIcon)
				in->in_SupportFlags |= INF_SupportsSnapshot | INF_SupportsUnSnapshot;

			if (VolumeIsWritable)
				{
				if (hasIcon)
					in->in_SupportFlags |= INF_SupportsSnapshot | INF_SupportsUnSnapshot;

				if (IsPermanentBackDropIcon(iwt, &bdl, dirLock, GetIconName(in)))
					{
					d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
					in->in_SupportFlags |= INF_SupportsPutAway;
					in->in_SupportFlags &= ~INF_SupportsLeaveOut;
					}
				else
					{
					d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
					in->in_SupportFlags &= ~INF_SupportsPutAway;
					in->in_SupportFlags |= INF_SupportsLeaveOut;
					}
				}
			else
				{
				in->in_SupportFlags &= ~(INF_SupportsSnapshot | INF_SupportsUnSnapshot);
				}

			if (!hasIcon)
				in->in_SupportFlags &= ~INF_SupportsLeaveOut;

			if (0 != pos)
				in->in_SupportFlags &= ~(INF_SupportsSnapshot | INF_SupportsUnSnapshot | INF_SupportsLeaveOut);

			d1(KPrintF("%s/%s/%ld: in=<%s>  supportFlags=%08lx  hasIcon=%ld  pos=%lu\n", \
				__FILE__, __FUNC__, __LINE__, GetIconName(in), in->in_SupportFlags, hasIcon, pos));

			in->in_FileType = rild.rild_TypeNode;

			SetIconSupportsFlags(in, VolumeIsWritable);

			if (!DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_TextWin_InsertIcon, in))
				{
				// SCCM_TextWin_InsertIcon failed
				struct ScaIconNode *IconList = NULL;

				SCA_MoveIconNode(&iwt->iwt_WindowTask.wt_IconList, &IconList, in);
				FreeIconList(iwt, &IconList);
				}
			}
		} while (0);

	FreeBackdropFile(&bdl);

	if (ScanDirSemaphoreLocked)
		ScalosReleaseSemaphore(&iwt->iwt_ScanDirSemaphore);

	if (IS_VALID_LOCK(oldDir))
		CurrentDir(oldDir);
	if (fib)
		FreeDosObject(DOS_FIB, fib);

	d1(KPrintF("%s/%s/%ld: in=%08lx\n", __FILE__, __FUNC__, __LINE__, in));

	return in;
}


static BOOL IsBackDropIconTextWindow(struct internalScaWindowTask *iwt, struct BackDropList *bdl,
	BPTR fLock, CONST_STRPTR FileName, ULONG pos)
{
	BOOL Result;

	if (0 != pos && ~0 != pos)
		{
		// FileName is an icon, generate name for non-icon
		STRPTR ObjectName = AllocCopyString(FileName);

		if (NULL == ObjectName)
			{
			Result = ScanDirIsBackDropIcon(iwt, bdl, fLock, FileName);
			}
		else
			{
			ObjectName[((STRPTR) pos) - FileName] = '\0';

			Result = ScanDirIsBackDropIcon(iwt, bdl, fLock, ObjectName);
			FreeCopyString(ObjectName);
			}
		}
	else
		{
		Result = ScanDirIsBackDropIcon(iwt, bdl, fLock, FileName);
		}

	return Result;
}


static enum ScanDirResult TextCheckCleanup(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc)
{
	enum ScanDirResult sdResult = SCANDIR_OK;

	if (rilc->rilc_CleanupCount++ > 20)
		{
		rilc->rilc_CleanupCount = 0;

		d1(kprintf("%s/%s/%ld: IconList=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_WindowTask.wt_LateIconList));

		ScanDirUpdateStatusBarText(iwt, rilc->rilc_TotalFound);
		}

	return sdResult;
}


static enum ScanDirResult GenerateTextIcons(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc, BOOL Final)
{
	struct IconScanEntry *ise;
	enum ScanDirResult sdResult;
	struct BackDropList bdl;


	d1(KPrintF("%s/%s/%ld: START  Final=%ld\n", __FILE__, __FUNC__, __LINE__, Final));

	InitBackDropList(&bdl);

	// build links between icons and objects
	sdResult = LinkIconScanList(iwt, rilc);
	if (SCANDIR_OK != sdResult)
		return sdResult;

	if (Final)
		{
		while ((ise = (struct IconScanEntry *) RemTail(&rilc->rilc_UnlinkedIconScanList)))
			{
			// Move all remaining unlinked icons from rilc_UnlinkedIconScanList to rilc_IconScanList
			Remove(&ise->ise_Node);
			AddTail(&rilc->rilc_IconScanList, &ise->ise_Node);
			}
		}

	for (ise = (struct IconScanEntry *) rilc->rilc_IconScanList.lh_Head;
		SCANDIR_OK == sdResult && ise != (struct IconScanEntry *) &rilc->rilc_IconScanList.lh_Tail;
		ise = (struct IconScanEntry *) ise->ise_Node.ln_Succ)
		{
		BOOL ShowEntry = TRUE;

		if (ise->ise_Flags & ISEFLG_Used)
			continue;

		if (rilc->rilc_OrigViewByType != iwt->iwt_WindowTask.mt_WindowStruct->ws_Viewmodes)
			break;

		if (!Final && NULL == ise->ise_ObjPtr && NULL == ise->ise_IconPtr)
			continue;

		d1(kprintf("%s/%s/%ld: ise=<%s>  Flags=%08lx  ObjPtr=%08lx  IconPtr=%08lx\n", \
			__FILE__, __FUNC__, __LINE__, ise->ise_Fib.fib_FileName, ise->ise_Flags, ise->ise_ObjPtr, ise->ise_IconPtr));

		if (!IsShowAllType(iwt->iwt_OldShowType))
			{
			ShowEntry = (NULL == ise->ise_ObjPtr);	// Icon w/o object

			if (WBDISK == ise->ise_IconType)
				ShowEntry = FALSE;	// don't show WBDISK icons
			}

		if (IsFileHidden(&ise->ise_Fib))
			ShowEntry = FALSE;

		if (ShowEntry)
			{
			struct ScaIconNode *in;
			struct ReadIconListData rild;

			rild.rild_DiskWriteProtected = !rilc->rilc_DiskWritable;

			ScanDirFillRildFromIse(&rild, ise);

			if (!IsShowAllType(iwt->iwt_OldShowType))
				{
				if (0 != ise->ise_Pos && ~0 != ise->ise_Pos)
					{
					// strip ".info" from name
					STRPTR ppos = rild.rild_Name + ((STRPTR) ise->ise_Pos - (STRPTR) ise->ise_Fib.fib_FileName);

					*ppos = '\0';
					}

				if (NULL == ise->ise_ObjPtr)
					{
					// icon w/o associated object
					rild.rild_Size64 = Make64(0);
					rild.rild_SoloIcon = TRUE;

					d1(kprintf("%s/%s/%ld: ise=<%s>  Flags=%08lx  ObjPtr=%08lx  IconPtr=%08lx  SOLOicon\n", \
						__FILE__, __FUNC__, __LINE__, ise->ise_Fib.fib_FileName, ise->ise_Flags, ise->ise_ObjPtr, ise->ise_IconPtr));
					}
				}

			in = GetTextIcon(iwt, NULL, ise, &bdl, &rild);

			d1(kprintf("%s/%s/%ld: in=%08lx\n", __FILE__, __FUNC__, __LINE__, in));

			if (in)
				{
				if (!(ise->ise_Flags & ISEFLG_IsIcon) && NULL == ise->ise_IconPtr)
				{
					ClassSetDefaultIconFlags(in, TRUE);
					d1(kprintf("%s/%s/%ld: ise=<%s>  Flags=%08lx  ObjPtr=%08lx  IconPtr=%08lx  SOLOicon\n", \
						__FILE__, __FUNC__, __LINE__, ise->ise_Fib.fib_FileName, ise->ise_Flags, ise->ise_ObjPtr, ise->ise_IconPtr));
				}

				d1(KPrintF("%s/%s/%ld: Name=<%s>  rild_IconType=%ld\n", \
					__FILE__, __FUNC__, __LINE__, rild.rild_Name, rild.rild_IconType));

				SetIconSupportsFlags(in, rilc->rilc_DiskWritable);
				}
			}

		if (iwt->iwt_WindowTask.mt_MainObject)
			{
			if (DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_CheckForMessages))
				sdResult = SCANDIR_ABORT;
			}

		ise->ise_Flags |= ISEFLG_Used;
		}

	for (ise = (struct IconScanEntry *) rilc->rilc_NonIconScanList.lh_Head;
		SCANDIR_OK == sdResult && ise != (struct IconScanEntry *) &rilc->rilc_NonIconScanList.lh_Tail;
		ise = (struct IconScanEntry *) ise->ise_Node.ln_Succ)
		{
		BOOL ShowEntry = TRUE;

		if (ise->ise_Flags & ISEFLG_Used)
			continue;

		if (!Final && NULL == ise->ise_ObjPtr && NULL == ise->ise_IconPtr)
			continue;

		d1(kprintf("%s/%s/%ld: ise=<%s>  Flags=%08lx  ObjPtr=%08lx  IconPtr=%08lx\n", \
			__FILE__, __FUNC__, __LINE__, ise->ise_Fib.fib_FileName, ise->ise_Flags, ise->ise_ObjPtr, ise->ise_IconPtr));

		if (!IsShowAllType(iwt->iwt_OldShowType))
			{
			ShowEntry = (NULL != ise->ise_IconPtr);	// Object with icon
			}

		if (IsFileHidden(&ise->ise_Fib))
			ShowEntry = FALSE;

		if (ShowEntry)
			{
			struct ScaIconNode *in;
			struct ReadIconListData rild;

			rild.rild_DiskWriteProtected = !rilc->rilc_DiskWritable;

			ScanDirFillRildFromIse(&rild, ise);

			in = GetTextIcon(iwt, NULL, ise, &bdl, &rild);

			d1(kprintf("%s/%s/%ld: in=%08lx\n", __FILE__, __FUNC__, __LINE__, in));

			if (in)
				{
				if (NULL == ise->ise_IconPtr)
					ClassSetDefaultIconFlags(in, TRUE);

				d1(KPrintF("%s/%s/%ld: Name=<%s>  rild_IconType=%ld\n", \
					__FILE__, __FUNC__, __LINE__, rild.rild_Name, rild.rild_IconType));

				SetIconSupportsFlags(in, rilc->rilc_DiskWritable);
				}
			}

		if (iwt->iwt_WindowTask.mt_MainObject)
			{
			if (DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_CheckForMessages))
				sdResult = SCANDIR_ABORT;
			}

		ise->ise_Flags |= ISEFLG_Used;
		}

	FreeBackdropFile(&bdl);

	d1(KPrintF("%s/%s/%ld: END\n", __FILE__, __FUNC__, __LINE__));

	return sdResult;
}


// Result :
// ==0	Success
// !=0	Failure
ULONG ReadTextWindowIconList(struct internalScaWindowTask *iwt)
{
	struct ReadIconListControl rilc;
	enum ScanDirResult sdResult;
	ULONG Result;
	ULONG Flags = REDRAWF_DontRefreshWindowFrame;

	d1(KPrintF("\n" "%s/%s/%ld: START  iwt=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt));

	RilcInit(&rilc);

	rilc.rilc_OrigViewByType = iwt->iwt_WindowTask.mt_WindowStruct->ws_Viewmodes;
	rilc.rilc_TotalFound = 0;

	if (!AttemptSemaphoreNoNest(&iwt->iwt_UpdateSemaphore))
		return 0;

	LockWindow(iwt);

	ScalosLockIconListExclusive(iwt);
	FreeIconList(iwt, &iwt->iwt_WindowTask.wt_IconList);
	FreeIconList(iwt, &iwt->iwt_WindowTask.wt_LateIconList);
	ScalosUnLockIconList(iwt);

	DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_Redraw, Flags);

	DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_SetVirtSize, 
		SETVIRTF_AdjustRightSlider | SETVIRTF_AdjustBottomSlider);

	// limit <wt_XOffset> and <wt_YOffset> to positive values
	if (iwt->iwt_WindowTask.wt_XOffset < 0)
		iwt->iwt_WindowTask.wt_XOffset = 0;
	if (iwt->iwt_WindowTask.wt_YOffset < 0)
		iwt->iwt_WindowTask.wt_YOffset = 0;

	d1(kprintf("%s/%s/%ld: xOffset=%ld  yOffset=%ld\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_WindowTask.wt_XOffset, iwt->iwt_WindowTask.wt_YOffset));

	sdResult = GetFileList(iwt, &rilc, TextCheckCleanup, CurrentPrefs.pref_UseExAll, TRUE, FALSE);

	d1(kprintf("%s/%s/%ld: sdResult=%ld\n", __FILE__, __FUNC__, __LINE__, sdResult));

	switch (sdResult)
		{
	case SCANDIR_VIEWMODE_CHANGE:
	case SCANDIR_FINISHED:
		Result = 0;	// Success
		break;
	default:		// Failure
		d1(kprintf("%s/%s/%ld: sdResult=%ld\n", __FILE__, __FUNC__, __LINE__, sdResult));
		Result = 1;
		break;
		}

	d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	if (rilc.rilc_OrigViewByType == iwt->iwt_WindowTask.mt_WindowStruct->ws_Viewmodes)
		{
		struct Hook SortHook;

		ScalosLockIconListExclusive(iwt);

		SETHOOKFUNC(SortHook, GetTextIconSortFunction(iwt));
		SortHook.h_Data = NULL;

		sdResult = GenerateTextIcons(iwt, &rilc, TRUE);

		d1(kprintf("%s/%s/%ld: IconList=%08lx  SortFunc=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_WindowTask.wt_IconList, SortHook.h_Entry));

		if (SCANDIR_OK != sdResult)
			{
			Result = 1;
			}
		else
			{
			if (SortHook.h_Entry)
				SCA_SortNodes((struct ScalosNodeList *) &iwt->iwt_WindowTask.wt_IconList, &SortHook, SCA_SortType_Best);

			if (iwt->iwt_WindowTask.wt_Window)
				{
				ReposTextIcons2(iwt);

				d1(kprintf("%s/%s/%ld: before RefreshIcons()\n", __FILE__, __FUNC__, __LINE__));
				RefreshIcons(iwt, NULL);
				}
			}

		ScalosUnLockIconList(iwt);
		}

	RilcCleanup(&rilc);

	ScalosReleaseSemaphore(&iwt->iwt_UpdateSemaphore);

	UnLockWindow(iwt);

	if (rilc.rilc_OrigViewByType != iwt->iwt_WindowTask.mt_WindowStruct->ws_Viewmodes)
		DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_Update);

	SetMenuOnOff(iwt);

	d1(KPrintF("%s/%s/%ld: END iwt=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, iwt, iwt->iwt_WinTitle));

	return Result;
}


static struct ScaIconNode *GetTextIcon(struct internalScaWindowTask *iwt,
	struct Hook *ColumnChangeHook, struct IconScanEntry *ise, 
	struct BackDropList *bdl, const struct ReadIconListData *rild)
{
	Object *TextIconObj;
	struct ScaIconNode *in = NULL;
	struct ExAllData ead;
	BOOL isLink;
	BPTR currentDirLock;
	ULONG TextStyle = FS_NORMAL;
	ULONG pos;
#if defined(ED_SIZE64)
	const ULONG ExAllType = ED_SIZE64;
#else /* ED_SIZE64 */
	const ULONG ExAllType = ED_OWNER;
#endif /* ED_SIZE64 */

	d1(kprintf("%s/%s/%ld: Name=<%s>  Type=%ld\n", __FILE__, __FUNC__, __LINE__, rild->rild_Name, rild->rild_Type));
	d1(kprintf("%s/%s/%ld: Comment=<%s>\n", __FILE__, __FUNC__, __LINE__, rild->rild_Comment));

	if (NULL == rild->rild_Name || '\0' == *rild->rild_Name)
		return NULL;

	ead.ed_Name = (STRPTR) rild->rild_Name;
	ead.ed_Type = rild->rild_Type;
	ScalosSetExAllSize64(&ead, ExAllType, rild->rild_Size64);
	ead.ed_Prot = rild->rild_Protection;
	ead.ed_Days = rild->rild_DateStamp.ds_Days;
	ead.ed_Mins = rild->rild_DateStamp.ds_Minute;
	ead.ed_Ticks = rild->rild_DateStamp.ds_Tick;
	ead.ed_Comment = (STRPTR) rild->rild_Comment;

	isLink = (ST_SOFTLINK == rild->rild_Type) || (ST_LINKDIR == rild->rild_Type) ||
		(ST_LINKFILE == rild->rild_Type) || IsSoftLink(rild->rild_Name);

	d1(KPrintF("%s/%s/%ld: <%s> Type=%ld  isLink=%ld\n", __FILE__, __FUNC__, __LINE__, ead.ed_Name, rild->rild_IconType, isLink));
	d1(KPrintF("%s/%s/%ld: <%s> comment=<%s>\n", __FILE__, __FUNC__, __LINE__, ead.ed_Name, ead.ed_Comment));

	d1(KPrintF("%s/%s/%ld: ExAllType=%ld  rild_Size64=%08lx-%08lx\n", __FILE__, __FUNC__, __LINE__, \
		ExAllType, ULONG64_HIGH(rild->rild_Size64), ULONG64_LOW(rild->rild_Size64)));
	d1(KPrintF("%s/%s/%ld: ed_Size64=%08lx-%08lx\n", __FILE__, __FUNC__, __LINE__, \
		ULONG64_HIGH(ead.ed_Size64), ULONG64_LOW(ead.ed_Size64)));

	currentDirLock = CurrentDir((BPTR)NULL);
	CurrentDir(currentDirLock);

	pos = IsIconName(rild->rild_Name);

	if (IsBackDropIconTextWindow(iwt, bdl, currentDirLock, rild->rild_Name, pos))
		{
		return NULL;
		}

	if (ST_SOFTLINK == ead.ed_Type)
		ResolveLink(&ead);

	if (isLink)
		TextStyle = CurrentPrefs.pref_LinkTextStyle;

	// Notice:
	// IDTA_TextMode is always IDTV_TextMode_Normal,
	// for shadowed drawing (ICONOBJ_USERFLAGF_DrawShadowed), IDTV_TextMode_Shadow is used.

	TextIconObj = NewObject(TextIconClass, NULL,
		TIDTA_ExAllData, (ULONG) &ead,
		TIDTA_ExAllType, ExAllType,
		TIDTA_WidthArray, (ULONG) iwt->iwt_WidthArray,
		TIDTA_TextStyle, TextStyle,
		TIDTA_Font, (ULONG) iwt->iwt_IconFont,
		TIDTA_TTFont, (ULONG) &iwt->iwt_IconTTFont,
		TIDTA_IconType, (0 != pos) ? WB_TEXTICON_TOOL : (ULONG) rild->rild_IconType,
		TIDTA_TypeNode, (ULONG) rild->rild_TypeNode,
		TIDTA_WindowTask, (ULONG) iwt,
//		TIDTA_IconObject, (ULONG) (ise ? ise->ise_IconObj : NULL),
		TIDTA_SoloIcon, rild->rild_SoloIcon,
		IDTA_InnerBottom, CurrentPrefs.pref_ImageBorders.Bottom,
		IDTA_InnerRight, CurrentPrefs.pref_ImageBorders.Right,
		IDTA_InnerTop, CurrentPrefs.pref_ImageBorders.Top,
		IDTA_InnerLeft, CurrentPrefs.pref_ImageBorders.Left,
		IDTA_Text, (ULONG) rild->rild_Name,
		IDTA_HalfShinePen, PalettePrefs.pal_PensList[PENIDX_HSHINEPEN],
		IDTA_HalfShadowPen, PalettePrefs.pal_PensList[PENIDX_HSHADOWPEN],
		IDTA_FrameTypeSel, CurrentPrefs.pref_FrameTypeSel,
		IDTA_FrameType, CurrentPrefs.pref_FrameType,
		IDTA_TextSkip, CurrentPrefs.pref_TextSkip,
		IDTA_MultiLineText, (ULONG) CurrentPrefs.pref_IconTextMuliLine,
		TIDTA_TextPenFileNormal, PalettePrefs.pal_PensList[PENIDX_FILETEXT],
		TIDTA_TextPenFileSelected, PalettePrefs.pal_PensList[PENIDX_FILETEXTSEL],
		IDTA_TextPenShadow, PalettePrefs.pal_PensList[PENIDX_ICONTEXTSHADOWPEN],
		IDTA_TextPenOutline, PalettePrefs.pal_PensList[PENIDX_ICONTEXTOUTLINEPEN],
		TIDTA_TextPenDrawerNormal, PalettePrefs.pal_PensList[PENIDX_DRAWERTEXT],
		TIDTA_TextPenDrawerSelected, PalettePrefs.pal_PensList[PENIDX_DRAWERTEXTSEL],
		IDTA_TextMode, IDTV_TextMode_Normal,
		IDTA_TextDrawMode, FontPrefs.fprf_TextDrawMode,
		IDTA_TextStyle, isLink ? CurrentPrefs.pref_LinkTextStyle : FS_NORMAL,
		IDTA_SupportedIconTypes, CurrentPrefs.pref_SupportedIconTypes,
		TIDTA_ColumnWidthChangeHook, (ULONG) ColumnChangeHook,
		TIDTA_SelectNameOnly, CurrentPrefs.pref_SelectTextIconName,
		TAG_END);

	d1(KPrintF("%s/%s/%ld: TextIconObj=%08lx  Name=<%s>  rild_IconType=%ld\n", __FILE__, __FUNC__, __LINE__, TextIconObj, ead.ed_Name, rild->rild_IconType));

	if (TextIconObj)
		{
		if (ise)
			ise->ise_IconObj = NULL;	// IconObject will be freed when TextIconObj is disposed!

		in = SCA_AllocIconNode(&iwt->iwt_WindowTask.wt_IconList);

		d1(kprintf("%s/%s/%ld: in=%08lx\n", __FILE__, __FUNC__, __LINE__, in));
		if (in)
			{
			in->in_Icon = TextIconObj;

			GetAttr(TIDTA_IconType, TextIconObj, (ULONG *) &in->in_FileType);

			SetIconName(TextIconObj, in);
			d1(kprintf("%s/%s/%ld: in_Name=<%s> %08lx\n", __FILE__, __FUNC__, __LINE__, in->in_Name, in->in_Name));

			switch (rild->rild_Type)
				{
			case ST_FILE:
			case ST_LINKFILE:
			case ST_PIPEFILE:
				in->in_Flags |= INF_File;
				break;
			case ST_SOFTLINK:
				if (ead.ed_Type < 0)
					in->in_Flags |= INF_File;
				break;
			default:
				if (rild->rild_Type < 0)
					in->in_Flags |= INF_File;
				break;
				}

			in->in_Flags |= INF_TextIcon;
			in->in_Flags |= INF_Sticky;

			if (rild->rild_DiskWriteProtected)
				in->in_Flags |= INF_VolumeWriteProtected;

			in->in_SupportFlags = INF_SupportsOpen | INF_SupportsCopy | INF_SupportsInformation;

			if (!rild->rild_DiskWriteProtected)
				{
				if (IsPermanentBackDropIcon(iwt, bdl, currentDirLock, rild->rild_Name))
					{
					d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
					in->in_SupportFlags |= INF_SupportsPutAway;
					in->in_SupportFlags &= ~INF_SupportsLeaveOut;
					}
				else
					{
					d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
					in->in_SupportFlags &= ~INF_SupportsPutAway;
					in->in_SupportFlags |= INF_SupportsLeaveOut;
					}
				}

			if (0 != pos)
				in->in_SupportFlags &= ~(INF_SupportsSnapshot | INF_SupportsUnSnapshot | INF_SupportsLeaveOut);

			if (!(rild->rild_Protection & FIBF_DELETE))
				in->in_SupportFlags |= INF_SupportsDelete;
			if (!(rild->rild_Protection & FIBF_WRITE))
				in->in_SupportFlags |= INF_SupportsRename;

			d1(kprintf("%s/%s/%ld: in=<%s>  supportFlags=%08lx  pos=%lu\n", \
				__FILE__, __FUNC__, __LINE__, GetIconName(in), in->in_SupportFlags, pos));

			in->in_FileDateStamp = rild->rild_DateStamp;
			in->in_FileSize = ULONG64_LOW(rild->rild_Size64);

			DateStamp(&in->in_DateStamp);

			d1(kprintf("%s/%s/%ld: <%s> Protection=%08lx  SupportFlags=%08lx\n", \
				__FILE__, __FUNC__, __LINE__, rild->rild_Name, rild->rild_Protection, in->in_SupportFlags));

			d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

			DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_LayoutIcon,
				in->in_Icon, IOLAYOUTF_NormalImage | IOLAYOUTF_SelectedImage);
			}
		}

	d1(kprintf("%s/%s/%ld: in=%08lx\n", __FILE__, __FUNC__, __LINE__, in));

	return in;
}


static void ResolveLink(struct ExAllData *ead)
{
	BPTR LinkLock;
	struct FileInfoBlock *fib = NULL;

	do	{
		LinkLock = Lock(ead->ed_Name, ACCESS_READ);
		d1(kprintf("%s/%s/%ld: LinkLock=%08lx\n", __FILE__, __FUNC__, __LINE__, LinkLock));
		if ((BPTR)NULL == LinkLock)
			break;

		debugLock_d1(LinkLock);

		fib = AllocDosObject(DOS_FIB, NULL);
		d1(kprintf("%s/%s/%ld: fib=%08lx\n", __FILE__, __FUNC__, __LINE__, fib));
		if (NULL == fib)
			break;

		if (!ScalosExamine64(LinkLock, fib))
			break;

		d1(kprintf("%s/%s/%ld: Name=<%s>  Type=%ld\n", __FILE__, __FUNC__, __LINE__, fib->fib_FileName, fib->fib_DirEntryType));

		ead->ed_Type = fib->fib_DirEntryType;
#if defined(ED_SIZE64)
		ScalosSetExAllSize64(ead, ED_SIZE64, ScalosFibSize64(fib));
#else /* ED_SIZE64 */
		ScalosSetExAllSize64(ead, ED_OWNER, ScalosFibSize64(fib));
#endif /* ED_SIZE64 */
		ead->ed_Prot = fib->fib_Protection;
		ead->ed_Days = fib->fib_Date.ds_Days;
		ead->ed_Mins = fib->fib_Date.ds_Minute;
		ead->ed_Ticks = fib->fib_Date.ds_Tick;
		ead->ed_Comment = fib->fib_Comment;
		} while (0);

	if (fib)
		FreeDosObject(DOS_FIB, fib);
	if (LinkLock)
		UnLock(LinkLock);
}


static LONG ReExamine(struct ReadIconListData *rild)
{
	struct FileInfoBlock *fib;
	BPTR fLock = (BPTR)NULL;
	BOOL Result = RETURN_OK;

	do	{
		fib = AllocDosObject(DOS_FIB, NULL);
		if (NULL == fib)
			{
			Result = IoErr();
			break;
			}

		fLock = Lock(rild->rild_Name, ACCESS_READ);
		if ((BPTR)NULL == fLock)
			{
			Result = IoErr();
			break;
			}

		if (!ScalosExamine64(fLock, fib))
			{
			Result = IoErr();
			break;
			}

		rild->rild_Type = fib->fib_DirEntryType;
		rild->rild_Protection = fib->fib_Protection;
		rild->rild_Size64 = ScalosFibSize64(fib);
		rild->rild_DateStamp = fib->fib_Date;

		stccpy(rild->rild_Comment, fib->fib_Comment, sizeof(rild->rild_Comment));
		stccpy(rild->rild_Name, fib->fib_FileName, sizeof(rild->rild_Name));

		} while (0);

	if (fib)
		FreeDosObject(DOS_FIB, fib);
	if (fLock)
		UnLock(fLock);

	return Result;
}


