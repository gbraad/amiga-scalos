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
#include <proto/graphics.h>
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
#include <scalos/GadgetBar.h>
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

#define	ExAllBuffer_SIZE	10000		// Alloc size for rilc_ExAllBuffer

//----------------------------------------------------------------------------

// local functions

static enum ScanDirResult CheckCleanup(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc);
static enum ScanDirResult GenerateIcons(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc, BOOL Final);
static struct ScaIconNode *CreateDefaultIcon(struct internalScaWindowTask *iwt,
	const struct ReadIconListData *rild, struct BackDropList *bdl,
	struct ScaReadIconArg *ria, BOOL CheckForDuplicates);
static struct ScaIconNode *ScanDirInitIcon(struct internalScaWindowTask *iwt,
	const struct ReadIconListData *rild, struct BackDropList *bdl,
	struct ScaReadIconArg *ria, BOOL isDefIcon, Object *IconObj);
static struct ScaIconNode *CreateIcon(struct internalScaWindowTask *iwt,
	const struct ReadIconListData *rild, struct BackDropList *bdl, struct ScaReadIconArg *ria);
static SAVEDS(LONG) CompareNameFunc(struct Hook *hook, struct ScaIconNode *in2,
	struct ScaIconNode *in1);
static enum ScanDirResult BeginScan_ExAll(struct internalScaWindowTask *iwt, 
	struct ReadIconListControl *rilc);
static enum ScanDirResult ScanDir_ExAll(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc);
static enum ScanDirResult BeginScan_Examine(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc);
static enum ScanDirResult ScanDir_Examine(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc);
static LONG ReExamine(struct ReadIconListData *rild);
static struct IconScanEntry *NewIconScanEntry(const struct ReadIconListData *rild);
static void DisposeIconScanEntry(struct IconScanEntry *ise);
static void RilcDisposeData(void *data);
static void RilcDisposeKey(void *key);
static int RilcCompare(const void *data1, const void *data2);
static ULONG AdjustIconType(Object *IconObj, LONG DirEntryType);
static BOOL ScanDirFindIcon(struct internalScaWindowTask *iwt, CONST_STRPTR IconName);

//----------------------------------------------------------------------------

// local data

static struct Hook CompareNameHook =
	{
	{ NULL, NULL },
	HOOKFUNC_DEF(CompareNameFunc),	// h_Entry + h_SubEntry
	NULL,						// h_Data
	};

//----------------------------------------------------------------------------

// public data items

//----------------------------------------------------------------------------

// Result : Success
struct ScaIconNode *IconWindowReadIcon(struct internalScaWindowTask *iwt, 
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

	InitBackDropList(&bdl);

	do	{
		struct ReadIconListData rild;
		BPTR iLock;

		d1(KPrintF("%s/%s/%ld: Name=<%s>  ria=%08lx  iwt=%08lx\n", __FILE__, __FUNC__, __LINE__, Name, ria, iwt));

		// Always ignore "disk.info" in root windows
		if ((iwt->iwt_WindowTask.mt_WindowStruct->ws_Flags & WSV_FlagF_RootWindow)
			&& 0 == Stricmp(Name, "disk"))
			break;

		BackdropWait(iwt->iwt_WindowTask.mt_WindowStruct->ws_Lock);

		if (!AttemptSemaphoreNoNest(&iwt->iwt_ScanDirSemaphore))
			break;

		ScanDirSemaphoreLocked = TRUE;

		if (!CurrentPrefs.pref_ShowThumbnailsAsDefault && (THUMBNAILS_AsDefault == iwt->iwt_ThumbnailMode))
			{
			if (!IsShowAllType(iwt->iwt_OldShowType))
				{
				if (0 != IsIconName(Name))
					{
					d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
					break;	// Display thumbnail only if object has a icon
					}
				}
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


		d1(kprintf("%s/%s/%ld: Lock/Examine returned %ld\n", __FILE__, __FUNC__, __LINE__, err));

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

			// switch back to original object name (w/o ".info")
			stccpy(fib->fib_FileName, Name, sizeof(fib->fib_FileName));
			}

		if (IsFileHidden(fib))
			{
			d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
			break;
			}

		stccpy(rild.rild_Name, fib->fib_FileName, sizeof(rild.rild_Name));
		stccpy(rild.rild_Comment, fib->fib_Comment, sizeof(rild.rild_Comment));
		rild.rild_Protection = fib->fib_Protection;
		rild.rild_DateStamp = fib->fib_Date;
		rild.rild_DiskWriteProtected = iwt->iwt_ReadOnly;
		rild.rild_CheckOverlap = TRUE;

		if (rild.rild_SoloIcon)
			{
			// icon without object
			rild.rild_Type = 0;	// type of object is undefined
			rild.rild_Size64 = Make64(0);
			}
		else
			{
			rild.rild_Type = fib->fib_DirEntryType;
			rild.rild_Size64 = ScalosFibSize64(fib);
			}

		ReExamine(&rild);

		d1(kprintf("%s/%s/%ld:  Name=<%s>\n", __FILE__, __FUNC__, __LINE__, rild.rild_Name));

		in = CreateIcon(iwt, &rild, &bdl, ria);

		d1(KPrintF("%s/%s/%ld: in=%08lx\n", __FILE__, __FUNC__, __LINE__, in));

		if (NULL == in)
			{
			DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_CleanUp);
			DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_SetVirtSize, 
				SETVIRTF_AdjustRightSlider | SETVIRTF_AdjustBottomSlider);

			d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

			break;
			}

		d1(KPrintF("%s/%s/%ld: ViewByType\n", __FILE__, __FUNC__, __LINE__));

		SetIconSupportsFlags(in, VolumeIsWritable);

		if (ria && ria->ria_Lock && ((BPTR)NULL == iwt->iwt_WindowTask.mt_WindowStruct->ws_Lock))
			{
			in->in_Lock = DupLock(ria->ria_Lock);
			CreateSbiForIcon(in);
			}

		// in_Lock must be valid for SCCM_IconWin_GetIconFileType
		DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_GetIconFileType, in);

		DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_CleanUp);
		DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_SetVirtSize, 
			SETVIRTF_AdjustRightSlider | SETVIRTF_AdjustBottomSlider);

		d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		} while (0);

	FreeBackdropFile(&bdl);

	if (ScanDirSemaphoreLocked)
		ScalosReleaseSemaphore(&iwt->iwt_ScanDirSemaphore);

	if (in)
		GenerateThumbnails(iwt);

	if (IS_VALID_LOCK(oldDir))
		CurrentDir(oldDir);
	if (fib)
		FreeDosObject(DOS_FIB, fib);

	d1(KPrintF("%s/%s/%ld: in=%08lx\n", __FILE__, __FUNC__, __LINE__, in));

	return in;
}


void SetIconName(Object *IconObj, struct ScaIconNode *in)
{
	STRPTR IconName;

	in->in_Name = (STRPTR) "";

	GetAttr(DTA_Name, IconObj, (APTR) &IconName);
	if (IconName && *IconName)
		in->in_Name = IconName;
}


BOOL IsNoIconPosition(const struct ExtGadget *gg)
{
	return (BOOL) ((UWORD) NO_ICON_POSITION_SHORT == (UWORD) gg->LeftEdge);
}


// Result :
// ==0	Success
// !=0	Failure
ULONG ReadIconList(struct internalScaWindowTask *iwt)
{
	struct ReadIconListControl rilc;
	enum ScanDirResult sdResult;
	BOOL UpdateSemaphoreLocked = FALSE;
	BOOL ScanDirSemaphoreLocked = FALSE;
	ULONG Result = RETURN_OK;

	d1(KPrintF("\n" "%s/%s/%ld: START  iwt=%08lx ws=%08lx\n", \
		__FILE__, __FUNC__, __LINE__, iwt, iwt->iwt_WindowTask.mt_WindowStruct));
	TIMESTAMP_d1();

	do	{
		RilcInit(&rilc);

		FlushThumbnailEntries(iwt);

		rilc.rilc_OrigViewByType = iwt->iwt_WindowTask.mt_WindowStruct->ws_Viewmodes;
		rilc.rilc_TotalFound = 0;

		if (!AttemptSemaphoreNoNest(&iwt->iwt_UpdateSemaphore))
			{
			d1(kprintf("%s/%s/%ld: AttemptSemaphoreNoNest(iwt_UpdateSemaphore) failed\n", __FILE__, __FUNC__, __LINE__));
			break;
			}

		UpdateSemaphoreLocked = TRUE;

		if (!AttemptSemaphoreNoNest(&iwt->iwt_ScanDirSemaphore))
			{
			d1(kprintf("%s/%s/%ld: AttemptSemaphoreNoNest(iwt_ScanDirSemaphore) failed\n", __FILE__, __FUNC__, __LINE__));
			break;
			}

		ScanDirSemaphoreLocked = TRUE;

		d1(kprintf("%s/%s/%ld: UpdateSemaphore=%08lx  NestCount=%ld\n", \
			__FILE__, __FUNC__, __LINE__, &iwt->iwt_UpdateSemaphore, iwt->iwt_UpdateSemaphoreSema..ss_NestCount));

		ScalosLockIconListExclusive(iwt);
		FreeIconList(iwt, &iwt->iwt_WindowTask.wt_LateIconList);
		ScalosUnLockIconList(iwt);

		ScanDirUpdateStatusBarText(iwt, 0);

		BackdropWait(iwt->iwt_WindowTask.mt_WindowStruct->ws_Lock);

		if (iwt->iwt_WindowTask.wt_Window)
			{
			SetABPenDrMd(iwt->iwt_WindowTask.wt_Window->RPort, FontPrefs.fprf_FontFrontPen,
				FontPrefs.fprf_FontBackPen, FontPrefs.fprf_TextDrawMode);
			}

		TIMESTAMP_d1();

		sdResult = GetFileList(iwt, &rilc, CheckCleanup, CurrentPrefs.pref_UseExAll, FALSE, iwt->iwt_CheckOverlappingIcons);

		d1(KPrintF("%s/%s/%ld: sdResult=%ld\n", __FILE__, __FUNC__, __LINE__, sdResult));

		switch (sdResult)
			{
		case SCANDIR_VIEWMODE_CHANGE:
			Result = RETURN_OK;	// Success
			break;
		case SCANDIR_FINISHED:
			Result = RETURN_OK;	// Success
			break;
		default:		// Failure
			d1(kprintf("%s/%s/%ld: sdResult=%ld\n", __FILE__, __FUNC__, __LINE__, sdResult));
			Result = 1;
			break;
			}

		d1(KPrintF("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
		d1(KPrintF("%s/%s/%ld: ws_Viewmodes=%ld\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_WindowTask.mt_WindowStruct->ws_Viewmodes));
		TIMESTAMP_d1();

		if (rilc.rilc_OrigViewByType == iwt->iwt_WindowTask.mt_WindowStruct->ws_Viewmodes)
			{
			d1(kprintf("%s/%s/%ld: LateIconList=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_WindowTask.wt_LateIconList));

			ScanDirUpdateStatusBarText(iwt, rilc.rilc_TotalFound);

			d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

			sdResult = GenerateIcons(iwt, &rilc, TRUE);

			d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

			if (SCANDIR_OK != sdResult)
				{
				Result = 1;
				}
			else if (iwt->iwt_WindowTask.wt_Window)
				{
				d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

				SCA_SortNodes((struct ScalosNodeList *) &iwt->iwt_WindowTask.wt_LateIconList, &CompareNameHook, SCA_SortType_Best);

				d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

				DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_CleanUp);
				d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
				DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_SetVirtSize, 
					SETVIRTF_AdjustRightSlider | SETVIRTF_AdjustBottomSlider);
				d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
				}
			}
		TIMESTAMP_d1();
		d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		} while (0);

	RilcCleanup(&rilc);

	if (ScanDirSemaphoreLocked)
		ScalosReleaseSemaphore(&iwt->iwt_ScanDirSemaphore);
	if (UpdateSemaphoreLocked)
		ScalosReleaseSemaphore(&iwt->iwt_UpdateSemaphore);

	TIMESTAMP_d1();
	
	if (rilc.rilc_OrigViewByType != iwt->iwt_WindowTask.mt_WindowStruct->ws_Viewmodes)
		DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_Update);

	d1(KPrintF("%s/%s/%ld: END iwt=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, iwt, iwt->iwt_WinTitle));

	TIMESTAMP_d1();

	if (RETURN_OK == Result && !iwt->iwt_CloseWindow && THUMBNAILS_Never != iwt->iwt_ThumbnailMode)
		GenerateThumbnails(iwt);

	TIMESTAMP_d1();

	return Result;
}


static enum ScanDirResult BeginScan_ExAll(struct internalScaWindowTask *iwt, 
	struct ReadIconListControl *rilc)
{
	rilc->rilc_exallControl = AllocDosObject(DOS_EXALLCONTROL, NULL);
	if (NULL == rilc->rilc_exallControl)
		{
		d1(KPrintF("%s/%s/%ld: AllocDosObject(DOS_EXALLCONTROL) failed\n", __FILE__, __FUNC__, __LINE__));
		return SCANDIR_FAIL_ABORT;
		}

	rilc->rilc_exallControl->eac_MatchFunc = NULL;

	rilc->rilc_replyPort = CreateMsgPort();
	if (NULL == rilc->rilc_replyPort)
		{
		d1(kprintf("%s/%s/%ld: CreateMsgPort() failed\n", __FILE__, __FUNC__, __LINE__));
		return SCANDIR_FAIL_ABORT;
		}

	rilc->rilc_FileSysPort = GetFileSysTask();

	if (NULL == rilc->rilc_FileSysPort)
		{
		d1(kprintf("%s/%s/%ld: GetFileSysTask() failed\n", __FILE__, __FUNC__, __LINE__));
		return SCANDIR_FAIL_ABORT;
		}

	d1(kprintf("%s/%s/%ld: FileSysTask=%08lx\n", __FILE__, __FUNC__, __LINE__, rilc->rilc_FileSysPort));
	d1(kprintf("%s/%s/%ld: Lock Port=%08lx\n", __FILE__, __FUNC__, __LINE__, ((struct FileLock *)BADDR(rilc->rilc_dirLock))->fl_Task));

	rilc->rilc_pkt = AllocDosObject(DOS_STDPKT, NULL);
	if (NULL == rilc->rilc_pkt)
		{
		d1(kprintf("%s/%s/%ld: AllocDosObject(DOS_STDPKT) failed\n", __FILE__, __FUNC__, __LINE__));
		return SCANDIR_FAIL_ABORT;
		}

	rilc->rilc_pkt->dp_Type = ACTION_EXAMINE_ALL;
	rilc->rilc_pkt->dp_Arg1 = rilc->rilc_dirLock;
	rilc->rilc_pkt->dp_Arg2 = (LONG) rilc->rilc_ExAllBuffer;	// Buffer to store results
	rilc->rilc_pkt->dp_Arg3 = ExAllBuffer_SIZE;			// Length (in bytes) of buffer (ARG2)
	rilc->rilc_pkt->dp_Arg4 = rilc->rilc_ExAllType;			// Type of request
	rilc->rilc_pkt->dp_Arg5 = (LONG) rilc->rilc_exallControl;	// Control structure to store state information.

	rilc->rilc_PacketPending = TRUE;
	SendPkt(rilc->rilc_pkt, rilc->rilc_FileSysPort, rilc->rilc_replyPort);

	rilc->rilc_ExNextOk = FALSE;

	return SCANDIR_OK;
}


static enum ScanDirResult ScanDir_ExAll(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc)
{
	struct StandardPacket *replyPkt;
	struct ExAllData *ead = rilc->rilc_edNext;

	d1(kprintf("%s/%s/%ld: START edNext=%08lx\n", __FILE__, __FUNC__, __LINE__, rilc->rilc_edNext));

	if (NULL == rilc->rilc_edNext)
		{
		d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		WaitPort(rilc->rilc_replyPort);
		d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		replyPkt = (struct StandardPacket *) GetMsg(rilc->rilc_replyPort);

		if (NULL == replyPkt)
			return SCANDIR_OK;

		rilc->rilc_PacketPending = FALSE;
		rilc->rilc_FlagFinished = FALSE;

		d1(KPrintF("%s/%s/%ld: Entries=%ld  Res1=%ld  Res2=%ld\n", __FILE__, __FUNC__, __LINE__, \
			rilc->rilc_exallControl->eac_Entries, \
			replyPkt->sp_Pkt.dp_Res1, replyPkt->sp_Pkt.dp_Res2));

		if (!replyPkt->sp_Pkt.dp_Res1)
			{
			switch (replyPkt->sp_Pkt.dp_Res2)
				{
			case ERROR_BAD_NUMBER:
				return SCANDIR_EXALL_BADNUMBER;
				break;
			case ERROR_ACTION_NOT_KNOWN:
				return SCANDIR_EXALL_FAIL;
				break;
			case ERROR_NO_MORE_ENTRIES:
				rilc->rilc_FlagFinished = TRUE;
				break;
			default:
				d1(kprintf("%s/%s/%ld: Result2=%ld\n", __FILE__, __FUNC__, __LINE__, replyPkt->sp_Pkt.dp_Res2));
				return SCANDIR_FAIL_ABORT;
				break;
				}
			}

		ead = (struct ExAllData *) rilc->rilc_ExAllBuffer;
		rilc->rilc_edNext = ead->ed_Next;

		d1(kprintf("%s/%s/%ld: edNext=%08lx\n", __FILE__, __FUNC__, __LINE__, rilc->rilc_edNext));
		}

	d1(kprintf("%s/%s/%ld: Finished=%ld  Entries=%ld\n", __FILE__, __FUNC__, __LINE__, rilc->rilc_FlagFinished, \
		rilc->rilc_exallControl->eac_Entries));

	if (rilc->rilc_exallControl->eac_Entries)
		{
		struct IconScanEntry *ise;

		rilc->rilc_edNext = ead->ed_Next;

		d1(KPrintF("%s/%s/%ld: edNext=%08lx\n", __FILE__, __FUNC__, __LINE__, rilc->rilc_edNext));

		rilc->rilc_rd.rild_SoloIcon = FALSE;

		// !! be sure to copy the results from ead BEFORE starting next scan !!
		rilc->rilc_rd.rild_Type = ead->ed_Type;
		rilc->rilc_rd.rild_Protection = ead->ed_Prot;
		rilc->rilc_rd.rild_Size64 = ScalosExAllSize64(ead, rilc->rilc_ExAllType);

		rilc->rilc_rd.rild_DateStamp.ds_Days = ead->ed_Days;
		rilc->rilc_rd.rild_DateStamp.ds_Minute = ead->ed_Mins;
		rilc->rilc_rd.rild_DateStamp.ds_Tick = ead->ed_Ticks;

		rilc->rilc_rd.rild_SoloIcon = FALSE;

		stccpy(rilc->rilc_rd.rild_Name, ead->ed_Name, sizeof(rilc->rilc_rd.rild_Name));

		d1(KPrintF("%s/%s/%ld: <%s> ed_Comment=<%s>\n", \
			__FILE__, __FUNC__, __LINE__, ead->ed_Name, ead->ed_Comment));

		if (rilc->rilc_ExAllType >= ED_COMMENT && ead->ed_Comment)
			stccpy(rilc->rilc_rd.rild_Comment, ead->ed_Comment, sizeof(rilc->rilc_rd.rild_Comment));
		else
			strcpy(rilc->rilc_rd.rild_Comment, "");

		if (NULL == rilc->rilc_edNext && !rilc->rilc_FlagFinished)
			{
			// Start scanning for next entries - only if last call didn't return ERROR_NO_MORE_ENTRIES
			rilc->rilc_PacketPending = TRUE;
			SendPkt(rilc->rilc_pkt, rilc->rilc_FileSysPort, rilc->rilc_replyPort);

			rilc->rilc_edNext = NULL;
			}

		d1(KPrintF("%s/%s/%ld: Name=<%s>  Type=%ld  Prot=%08lx\n", __FILE__, __FUNC__, __LINE__, \
			rilc->rilc_rd.rild_Name, ead->ed_Type, ead->ed_Prot));
		d1(KPrintF("%s/%s/%ld: rild_Size64=%08lx-%08lx\n", __FILE__, __FUNC__, __LINE__, \
			ULONG64_HIGH(rilc->rilc_rd.rild_Size64), ULONG64_LOW(rilc->rilc_rd.rild_Size64)));

		ise = NewIconScanEntry(&rilc->rilc_rd);
		if (ise)
			{
			if (ise->ise_Flags & ISEFLG_IsIcon)
				{
				AddTail(&rilc->rilc_UnlinkedIconScanList, &ise->ise_Node);
				}
			else
				{
				AddTail(&rilc->rilc_NonIconScanList, &ise->ise_Node);
				d1(KPrintF("%s/%s/%ld: key=<%s>  ise=%08lx\n", __FILE__, __FUNC__, __LINE__, ise->ise_Fib.fib_FileName, ise));
				BTreeInsert(rilc->rilc_StdFilesTree,
					ise->ise_Fib.fib_FileName,
					ise);
				}

			rilc->rilc_TotalFound++;
			}

		if (rilc->rilc_Check)
			{
			enum ScanDirResult sdResult;

			sdResult = (*rilc->rilc_Check)(iwt, rilc);
			if (SCANDIR_OK != sdResult)
				return sdResult;
			}
		}
	else
		return SCANDIR_FINISHED;

	d1(kprintf("%s/%s/%ld: Finished=%ld  edNext=%08lx\n", __FILE__, __FUNC__, __LINE__, rilc->rilc_FlagFinished, \
		rilc->rilc_edNext));

	// Stop if last entry of last ExAllData has been read
	if (rilc->rilc_FlagFinished && (NULL == rilc->rilc_edNext))
		return SCANDIR_FINISHED;

	return SCANDIR_OK;
}


static enum ScanDirResult BeginScan_Examine(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc)
{
	rilc->rilc_fib = AllocDosObject(DOS_FIB, NULL);
	if (NULL == rilc->rilc_fib)
		return SCANDIR_FAIL_ABORT;

	ScalosExamine64(rilc->rilc_dirLock, rilc->rilc_fib);

	rilc->rilc_replyPort = CreateMsgPort();
	if (NULL == rilc->rilc_replyPort)
		return SCANDIR_FAIL_ABORT;

	rilc->rilc_FileSysPort = GetFileSysTask();
	if (NULL == rilc->rilc_FileSysPort)
		return SCANDIR_FAIL_ABORT;

	rilc->rilc_FlagLargeFileSupported = ScalosSupports64(rilc->rilc_FileSysPort,
		rilc->rilc_dirLock, rilc->rilc_fib);

	rilc->rilc_pkt = AllocDosObject(DOS_STDPKT, NULL);
	if (NULL == rilc->rilc_pkt)
		return SCANDIR_FAIL_ABORT;

	if (rilc->rilc_FlagLargeFileSupported)
		{
		ScalosDosPacketExNext64(rilc->rilc_pkt, rilc->rilc_dirLock, rilc->rilc_fib);
		}
	else
		{
		rilc->rilc_pkt->dp_Type = ACTION_EXAMINE_NEXT;
		rilc->rilc_pkt->dp_Arg1 = rilc->rilc_dirLock;
		rilc->rilc_pkt->dp_Arg2 = MKBADDR(rilc->rilc_fib);
		}

	rilc->rilc_PacketPending = TRUE;
	SendPkt(rilc->rilc_pkt, rilc->rilc_FileSysPort, rilc->rilc_replyPort);

	return SCANDIR_OK;
}


static enum ScanDirResult ScanDir_Examine(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc)
{
	struct IconScanEntry *ise;
	struct StandardPacket *replyPkt;
	size_t len;

	d1(kprintf("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
	WaitPort(rilc->rilc_replyPort);
	d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
	replyPkt = (struct StandardPacket *) GetMsg(rilc->rilc_replyPort);
	if (NULL == replyPkt)
		return SCANDIR_OK;

	rilc->rilc_PacketPending = FALSE;

	d1(kprintf("%s/%s/%ld: Res1=%ld\n", __FILE__, __FUNC__, __LINE__, replyPkt->sp_Pkt.dp_Res1));

	if (!replyPkt->sp_Pkt.dp_Res1)
		{
		switch (replyPkt->sp_Pkt.dp_Res2)
			{
		case ERROR_NO_MORE_ENTRIES:
			return SCANDIR_FINISHED;
			break;
		default:
			return SCANDIR_FAIL_ABORT;
			break;
			}
		}

	rilc->rilc_rd.rild_Type = rilc->rilc_fib->fib_DirEntryType;
	rilc->rilc_rd.rild_Protection = rilc->rilc_fib->fib_Protection;
	if (rilc->rilc_FlagLargeFileSupported)
		rilc->rilc_rd.rild_Size64 = ScalosFibSize64(rilc->rilc_fib);
	else
		rilc->rilc_rd.rild_Size64 = Make64(rilc->rilc_fib->fib_Size);
	rilc->rilc_rd.rild_DateStamp = rilc->rilc_fib->fib_Date;
	rilc->rilc_rd.rild_SoloIcon = FALSE;

	// when using Dos Packet interface,
	// fib_FileName is NOT 0-terminated and is preceded by length byte.

	len = (size_t) rilc->rilc_fib->fib_FileName[0];
	rilc->rilc_fib->fib_FileName[1 + len] = 0;

	stccpy(rilc->rilc_rd.rild_Name, &rilc->rilc_fib->fib_FileName[1], 
		sizeof(rilc->rilc_rd.rild_Name));

	d1(kprintf("%s/%s/%ld: fib_FileName=%02lx %02lx %02lx %02lx %02lx %02lx %02lx %02lx\n", __FILE__, __FUNC__, __LINE__, \
		rilc->rilc_fib->fib_FileName[0], rilc->rilc_fib->fib_FileName[1], \
		rilc->rilc_fib->fib_FileName[2], rilc->rilc_fib->fib_FileName[3], \
		rilc->rilc_fib->fib_FileName[4], rilc->rilc_fib->fib_FileName[5], \
		rilc->rilc_fib->fib_FileName[6], rilc->rilc_fib->fib_FileName[7] ));

	d1(kprintf("%s/%s/%ld: Name=<%s>  Type=%ld  Prot=%08lx\n", __FILE__, __FUNC__, __LINE__, \
		rilc->rilc_rd.rild_Name, rilc->rilc_rd.rild_Type, rilc->rilc_rd.rild_Protection));

	len = (size_t) rilc->rilc_fib->fib_Comment[0];
	rilc->rilc_fib->fib_Comment[1 + len] = 0;

	stccpy(rilc->rilc_rd.rild_Comment, &rilc->rilc_fib->fib_Comment[1], 
		sizeof(rilc->rilc_rd.rild_Comment));


	rilc->rilc_PacketPending = TRUE;
	SendPkt(rilc->rilc_pkt, rilc->rilc_FileSysPort, rilc->rilc_replyPort);

	ise = NewIconScanEntry(&rilc->rilc_rd);
	if (ise)
		{
		if (ise->ise_Flags & ISEFLG_IsIcon)
			{
			AddTail(&rilc->rilc_UnlinkedIconScanList, &ise->ise_Node);
			}
		else
			{
			AddTail(&rilc->rilc_NonIconScanList, &ise->ise_Node);
			d1(KPrintF("%s/%s/%ld: key=<%s>  ise=%08lx\n", __FILE__, __FUNC__, __LINE__, ise->ise_Fib.fib_FileName, ise));
			BTreeInsert(rilc->rilc_StdFilesTree,
				ise->ise_Fib.fib_FileName,
				ise);
			}

		rilc->rilc_TotalFound++;
		}

	d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

	if (rilc->rilc_Check)
		return (*rilc->rilc_Check)(iwt, rilc);

	return SCANDIR_OK;
}


BOOL ScanDirIsBackDropIcon(struct internalScaWindowTask *iwt, struct BackDropList *bdl,
	BPTR fLock, CONST_STRPTR FileName)
{
	struct ScaWindowStruct *wsMain = iInfos.xii_iinfos.ii_MainWindowStruct;

	if ((BPTR)NULL == fLock)
		return FALSE;
	if (NULL == wsMain)
		return FALSE;

	if (wsMain != iwt->iwt_WindowTask.mt_WindowStruct)
		{
		struct internalScaWindowTask *iwtMain = (struct internalScaWindowTask *) wsMain->ws_WindowTask;
		struct ScaIconNode *inx;

		d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		inx = FindBackdropIcon(fLock, FileName);

		if (inx)
			{
			d1(kprintf("%s/%s/%ld: iwtMain=%08lx  inx=%08lx\n", __FILE__, __FUNC__, __LINE__, iwtMain, inx));
			ScalosUnLockIconList(iwtMain);
			return TRUE;
			}
		d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		}

	return FALSE;
}


static struct ScaIconNode *ScanDirInitIcon(struct internalScaWindowTask *iwt,
	const struct ReadIconListData *rild, struct BackDropList *bdl,
	struct ScaReadIconArg *ria, BOOL isDefIcon, Object *IconObj)
{
	BPTR currentDirLock;
	struct ExtGadget *gg = (struct ExtGadget *) IconObj;
	struct ScaIconNode *in;
	ULONG IconType;
	STRPTR tt;
	BOOL FreePosition;

	if ('\0' == *rild->rild_Name)
		return NULL;

	d1(KPrintF("%s/%s/%ld: iwt=%08lx  ws=%08lx  rild_Name=<%s>\n", __FILE__, __FUNC__, __LINE__, \
		iwt, iwt->iwt_WindowTask.mt_WindowStruct, rild->rild_Name));

	d1(KPrintF("%s/%s/%ld: <%s> Type=%08lx\n", __FILE__, __FUNC__, __LINE__, rild->rild_Name, rild->rild_Type));

	if (NULL == IconObj)
		return NULL;

	IconType = AdjustIconType(IconObj, rild->rild_Type);
	d1(KPrintF("%s/%s/%ld: IconType=%ld\n", __FILE__, __FUNC__, __LINE__, IconType));

	if (WBDISK == IconType)
		{
		DisposeIconObject(IconObj);
		return NULL;
		}

	currentDirLock = CurrentDir((BPTR)NULL);
	CurrentDir(currentDirLock);

	if (ScanDirIsBackDropIcon(iwt, bdl, currentDirLock, rild->rild_Name))
		{
		DisposeIconObject(IconObj);
		return NULL;
		}

	d1(KPrintF("%s/%s/%ld:  iwt_ThumbnailMode=%ld\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_ThumbnailMode));

	if ((THUMBNAILS_Always == iwt->iwt_ThumbnailMode)
		|| (isDefIcon && THUMBNAILS_AsDefault == iwt->iwt_ThumbnailMode))
		{
		if (!AddThumbnailIcon(iwt, IconObj, currentDirLock, rild->rild_Name, 0)
			&& isDefIcon && !IsShowAllType(iwt->iwt_OldShowType))
			{
			// fail with default icons w/o thumbnails
			// in "show only icons" windows
			DisposeIconObject(IconObj);
			return NULL;
			}
		}

	FreePosition = IsNoIconPosition(gg);

	if (ria)
		{
		gg->LeftEdge = ria->ria_x;
		gg->TopEdge = ria->ria_y;
		}

	tt = NULL;
	if (DoMethod(IconObj, IDTM_FindToolType, "SCALOS_NOTEXT", &tt))
		{
		SetAttrs(IconObj,
			IDTA_Text, NULL,
			TAG_END);
		}

	d1(KPrintF("%s/%s/%ld: Left=%ld  Top=%ld\n", __FILE__, __FUNC__, __LINE__, gg->LeftEdge, gg->TopEdge));
	d1(KPrintF("%s/%s/%ld: mt_MainObject=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_WindowTask.mt_MainObject));

	if (iwt->iwt_WindowTask.mt_MainObject)
		{
		DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_LayoutIcon,
			IconObj, IOLAYOUTF_NormalImage);
		}

	ScalosLockIconListExclusive(iwt);

	if (IsNoIconPosition(gg))
		in = SCA_AllocIconNode(&iwt->iwt_WindowTask.wt_LateIconList);
	else
		in = SCA_AllocIconNode(&iwt->iwt_WindowTask.wt_IconList);

	d1(KPrintF("%s/%s/%ld: ScaIconNode=%08lx\n", __FILE__, __FUNC__, __LINE__, in));

	if (in)
		{
		SetIconName(IconObj, in);

		in->in_Icon = IconObj;
		in->in_FileType = (struct TypeNode *) IconType;

		in->in_SupportFlags = INF_SupportsOpen | INF_SupportsCopy | 
				INF_SupportsSnapshot |
				INF_SupportsInformation	| INF_SupportsLeaveOut;

		if (FreePosition)
			{
			in->in_Flags |= INF_FreeIconPosition;
			}
		else
			{
			in->in_SupportFlags |= INF_SupportsUnSnapshot;
			}

		if (!rild->rild_DiskWriteProtected)
			{
			if (IsPermanentBackDropIcon(iwt, bdl, currentDirLock, rild->rild_Name) )
				{
				d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
				in->in_SupportFlags |= INF_SupportsPutAway;
				in->in_SupportFlags &= ~INF_SupportsLeaveOut;

				SetAttrs(IconObj,
					IDTA_OverlayType, ICONOVERLAYF_LeftOut,
					TAG_END);
				}
			else
				{
				d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
				in->in_SupportFlags &= ~INF_SupportsPutAway;
				in->in_SupportFlags |= INF_SupportsLeaveOut;
				}
			}

		if (!(rild->rild_Protection & FIBF_DELETE))
			in->in_SupportFlags |= INF_SupportsDelete;
		if (!(rild->rild_Protection & FIBF_WRITE))
			in->in_SupportFlags |= INF_SupportsRename;

		in->in_FileDateStamp = rild->rild_DateStamp;
		in->in_FileSize = ULONG64_LOW(rild->rild_Size64);

		DateStamp(&in->in_DateStamp);

		d1(KPrintF("%s/%s/%ld: <%s> Protection=%08lx  SupportFlags=%08lx\n", \
			__FILE__, __FUNC__, __LINE__, rild->rild_Name, rild->rild_Protection, in->in_SupportFlags));

		d1(KPrintF("%s/%s/%ld: LeftEdge=%ld\n", __FILE__, __FUNC__, __LINE__, gg->LeftEdge));

		if (WBTOOL == IconType || WBPROJECT == IconType)
			{
			in->in_Flags |= INF_File;
			}

		tt = NULL;
		if (DoMethod(IconObj, IDTM_FindToolType, "SCALOS_NODRAG", &tt))
			in->in_Flags |= INF_Sticky;
		if (isDefIcon)
			ClassSetDefaultIconFlags(in, TRUE);
		if (rild->rild_DiskWriteProtected)
			{
			in->in_Flags |= INF_VolumeWriteProtected;
#if 0
			AddIconOverlay(IconObj, ICONOVERLAYF_ReadOnly);
#endif
			}
		ScalosUnLockIconList(iwt);

		d1(KPrintF("%s/%s/%ld: LeftEdge=%ld  TopEdge=%ld\n", __FILE__, __FUNC__, __LINE__, gg->LeftEdge, gg->TopEdge));

		if (!IsNoIconPosition(gg) && iwt->iwt_WindowTask.mt_MainObject)
			{
			d1(KPrintF("%s/%s/%ld: Fixed Icon Position\n", __FILE__, __FUNC__, __LINE__));

			if ((!iwt->iwt_BackDrop || !CheckPosition(iwt, in)) && (!rild->rild_CheckOverlap || !CheckOverlap(iwt, in)))
				{
				DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_DrawIcon, IconObj);
				}
			}
		}

	d1(KPrintF("%s/%s/%ld: in=%08lx\n", __FILE__, __FUNC__, __LINE__, in));

	return in;
}


static struct ScaIconNode *CreateIcon(struct internalScaWindowTask *iwt,
	const struct ReadIconListData *rild, struct BackDropList *bdl,
	struct ScaReadIconArg *ria)
{
	Object *IconObj;
	BOOL isLink;

	if ('\0' == *rild->rild_Name)
		return NULL;

	d1(KPrintF("%s/%s/%ld: iwt=%08lx  ws=%08lx  rild_Name=<%s>\n", __FILE__, __FUNC__, __LINE__, \
		iwt, iwt->iwt_WindowTask.mt_WindowStruct, rild->rild_Name));

	// If not root window, check for duplicate icon names
	if ((BPTR)NULL != iwt->iwt_WindowTask.mt_WindowStruct->ws_Lock)
		{
		if (ScanDirFindIcon(iwt, rild->rild_Name))
			{
			d1(KPrintF("%s/%s/%ld: Icon found\n", __FILE__, __FUNC__, __LINE__));
			return NULL;
			}
		}

	isLink = (ST_SOFTLINK == rild->rild_Type) || (ST_LINKDIR == rild->rild_Type) ||
		(ST_LINKFILE == rild->rild_Type) || IsSoftLink(rild->rild_Name);

	d1(KPrintF("%s/%s/%ld: <%s> Type=%08lx  isLink=%ld\n", __FILE__, __FUNC__, __LINE__, rild->rild_Name, rild->rild_Type, isLink));

	// NewIconObject()
	IconObj = (Object *) NewIconObjectTags((STRPTR) rild->rild_Name,
		IDTA_InnerBottom, CurrentPrefs.pref_ImageBorders.Bottom,
		IDTA_InnerRight, CurrentPrefs.pref_ImageBorders.Right,
		IDTA_InnerTop, CurrentPrefs.pref_ImageBorders.Top,
		IDTA_InnerLeft, CurrentPrefs.pref_ImageBorders.Left,
		IDTA_SupportedIconTypes, CurrentPrefs.pref_SupportedIconTypes,
		IDTA_SizeConstraints, (ULONG) &CurrentPrefs.pref_IconSizeConstraints,
		IDTA_Text, (ULONG) rild->rild_Name,
		DTA_Name, (ULONG) rild->rild_Name,
		IDTA_HalfShinePen, PalettePrefs.pal_PensList[PENIDX_HSHINEPEN],
		IDTA_HalfShadowPen, PalettePrefs.pal_PensList[PENIDX_HSHADOWPEN],
		IDTA_FrameTypeSel, CurrentPrefs.pref_FrameTypeSel,
		IDTA_FrameType, CurrentPrefs.pref_FrameType,
		IDTA_TextSkip, CurrentPrefs.pref_TextSkip,
		IDTA_MultiLineText, (ULONG) CurrentPrefs.pref_IconTextMuliLine,
		IDTA_TextPen, PalettePrefs.pal_PensList[PENIDX_ICONTEXTPEN],
		IDTA_TextPenSel, PalettePrefs.pal_PensList[PENIDX_ICONTEXTPENSEL],
		IDTA_TextPenShadow, PalettePrefs.pal_PensList[PENIDX_ICONTEXTSHADOWPEN],
		IDTA_TextPenOutline, PalettePrefs.pal_PensList[PENIDX_ICONTEXTOUTLINEPEN],
		IDTA_TextPenBgSel, PalettePrefs.pal_PensList[PENIDX_ICONTEXTPENBGSEL],
		IDTA_SelectedTextRectangle, CurrentPrefs.pref_SelectedTextRectangle,
		IDTA_SelTextRectBorderX, CurrentPrefs.pref_SelTextRectBorderX,
		IDTA_SelTextRectBorderY, CurrentPrefs.pref_SelTextRectBorderY,
		IDTA_SelTextRectRadius, CurrentPrefs.pref_SelTextRectRadius,
		IDTA_TextMode, CurrentPrefs.pref_TextMode,
		IDTA_TextDrawMode, FontPrefs.fprf_TextDrawMode,
		IDTA_TextBackPen, FontPrefs.fprf_FontBackPen,
		IDTA_TextStyle, isLink ? CurrentPrefs.pref_LinkTextStyle : FS_NORMAL,
		IDTA_Font, (ULONG) iwt->iwt_IconFont,
		IDTA_Fonthandle, (ULONG) &iwt->iwt_IconTTFont,
		IDTA_FontHook, (ULONG) (TTEngineBase ? &ScalosFontHook : NULL),
		TAG_END);

	d1(KPrintF("%s/%s/%ld: <%s>  IconObj=%08lx\n", __FILE__, __FUNC__, __LINE__, rild->rild_Name, IconObj));

	if (NULL == IconObj)
		{
		return CreateDefaultIcon(iwt, rild, bdl, ria, FALSE);
		}

	return ScanDirInitIcon(iwt, rild, bdl, ria, FALSE, IconObj);
}


static struct ScaIconNode *CreateDefaultIcon(struct internalScaWindowTask *iwt,
	const struct ReadIconListData *rild, struct BackDropList *bdl,
	struct ScaReadIconArg *ria, BOOL CheckForDuplicates)
{
	Object *IconObj = NULL;
	STRPTR PathBuffer = NULL;

	do	{
		CONST_STRPTR IconName = rild->rild_Name;
		ULONG IconViewMode;
		ULONG ddFlags;

		if (NULL == iwt->iwt_WindowTask.mt_MainObject || '\0' == *rild->rild_Name)
			break;;

		d1(KPrintF("%s/%s/%ld: iwt=%08lx  ws=%08lx  rild_Name=<%s>\n", __FILE__, __FUNC__, __LINE__, \
			iwt, iwt->iwt_WindowTask.mt_WindowStruct, rild->rild_Name));

		if (CheckForDuplicates)
			{
			// If not root window, check for duplicate icon names
			if ((BPTR)NULL != iwt->iwt_WindowTask.mt_WindowStruct->ws_Lock)
				{
				if (ScanDirFindIcon(iwt, rild->rild_Name))
					{
					d1(KPrintF("%s/%s/%ld: Icon found\n", __FILE__, __FUNC__, __LINE__));
					break;;
					}
				}
			}

		if (ria)
			{
			if (ria->ria_Lock && (BPTR)NULL == iwt->iwt_WindowTask.mt_WindowStruct->ws_Lock)
				{
				IconName = PathBuffer = AllocPathBuffer();
				if (NULL == PathBuffer)
					break;;

				NameFromLock(ria->ria_Lock, PathBuffer, Max_PathLen);
				AddPart(PathBuffer, (STRPTR) rild->rild_Name, Max_PathLen);
				}
			}

		d1(KPrintF("%s/%s/%ld: <%s> Type=%08lx\n", __FILE__, __FUNC__, __LINE__, rild->rild_Name, rild->rild_Type));

		// try to get default icon
		IconObj = (Object *) DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_GetDefIcon,
			IconName, rild->rild_Type, rild->rild_Protection);

		if (NULL == IconObj)
			break;

		// default icons inherit ShowAllFiles and ViewMode settings from parent window.
		IconViewMode = TranslateViewModesToIcon(iwt->iwt_WindowTask.mt_WindowStruct->ws_Viewmodes);

		ddFlags = iwt->iwt_WindowTask.mt_WindowStruct->ws_ViewAll;
		if (SortOrder_Ascending == iwt->iwt_WindowTask.mt_WindowStruct->ws_SortOrder)
			ddFlags |= DDFLAGS_SORTASC;
		else if (SortOrder_Descending == iwt->iwt_WindowTask.mt_WindowStruct->ws_SortOrder)
			ddFlags |= DDFLAGS_SORTDESC;

		SetAttrs(IconObj,
			IDTA_Flags, ddFlags,
			IDTA_ViewModes, IconViewMode,
			IDTA_InnerBottom, CurrentPrefs.pref_ImageBorders.Bottom,
			IDTA_InnerRight, CurrentPrefs.pref_ImageBorders.Right,
			IDTA_InnerTop, CurrentPrefs.pref_ImageBorders.Top,
			IDTA_InnerLeft, CurrentPrefs.pref_ImageBorders.Left,
			IDTA_SupportedIconTypes, CurrentPrefs.pref_SupportedIconTypes,
			IDTA_SizeConstraints, (ULONG) &CurrentPrefs.pref_IconSizeConstraints,
			TAG_END);
			} while (0);

	if (PathBuffer)
		FreePathBuffer(PathBuffer);

	return ScanDirInitIcon(iwt, rild, bdl, NULL, TRUE, IconObj);
}


static enum ScanDirResult CheckCleanup(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc)
{
	enum ScanDirResult sdResult = SCANDIR_OK;

	if (rilc->rilc_CleanupCount++ > 20)
		{
		rilc->rilc_CleanupCount = 0;

		sdResult = GenerateIcons(iwt, rilc, FALSE);

		d1(kprintf("%s/%s/%ld: IconList=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_WindowTask.wt_LateIconList));

		if (iwt->iwt_WindowTask.wt_Window)
			{
			SCA_SortNodes((struct ScalosNodeList *) &iwt->iwt_WindowTask.wt_LateIconList, &CompareNameHook, SCA_SortType_Best);

			DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_SetVirtSize, 
				SETVIRTF_AdjustRightSlider | SETVIRTF_AdjustBottomSlider);
			}

		ScanDirUpdateStatusBarText(iwt, rilc->rilc_TotalFound);
		}

	return sdResult;
}


static enum ScanDirResult GenerateIcons(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc, BOOL Final)
{
	struct IconScanEntry *ise;
	enum ScanDirResult sdResult;
	struct BackDropList bdl;
	ULONG n;

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

	d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

	for (n = 0, ise = (struct IconScanEntry *) rilc->rilc_IconScanList.lh_Head;
		SCANDIR_OK == sdResult && ise != (struct IconScanEntry *) &rilc->rilc_IconScanList.lh_Tail;
		ise = (struct IconScanEntry *) ise->ise_Node.ln_Succ)
		{
		BOOL ShowEntry = TRUE;

		if (ise->ise_Flags & ISEFLG_Used)
			continue;

		if (!Final && NULL == ise->ise_ObjPtr && NULL == ise->ise_IconPtr)
			continue;

		d1(KPrintF("%s/%s/%ld: ise=<%s>  Flags=%08lx  ObjPtr=%08lx  IconPtr=%08lx\n", \
			__FILE__, __FUNC__, __LINE__, ise->ise_Fib.fib_FileName, ise->ise_Flags, ise->ise_ObjPtr, ise->ise_IconPtr));

		if (!IsShowAllType(iwt->iwt_OldShowType))
			{
			ShowEntry = (NULL == ise->ise_ObjPtr);	// Icon w/o object

			if (WBDISK == ise->ise_IconType)
				ShowEntry = FALSE;	// don't show WBDISK icons
			}

		if (ShowEntry && IsFileHidden(&ise->ise_Fib))
			ShowEntry = FALSE;

		d1(kprintf("%s/%s/%ld: ShowEntry=%ld\n", __FILE__, __FUNC__, __LINE__, ShowEntry));

		if (ShowEntry)
			{
			struct ScaIconNode *in;
			struct ReadIconListData rild;

			rild.rild_DiskWriteProtected = !rilc->rilc_DiskWritable;

			if (ise->ise_ObjPtr)
				{
				ScanDirFillRildFromIse(&rild, ise->ise_ObjPtr);
				}
			else
				{
				// icon without object
				ScanDirFillRildFromIse(&rild,  ise);
				rild.rild_Type = 0;	// type of object is undefined

				rild.rild_Size64 = Make64(0);
				rild.rild_SoloIcon = TRUE;

				d1(kprintf("%s/%s/%ld: ise=<%s>  Flags=%08lx  ObjPtr=%08lx  IconPtr=%08lx  SOLOicon\n", \
					__FILE__, __FUNC__, __LINE__, ise->ise_Fib.fib_FileName, ise->ise_Flags, ise->ise_ObjPtr, ise->ise_IconPtr));
				}
			rild.rild_CheckOverlap = iwt->iwt_CheckOverlappingIcons;

			if (0 != ise->ise_Pos && ~0 != ise->ise_Pos)
				{
				// strip ".info" from name
				STRPTR ppos = rild.rild_Name + ((STRPTR) ise->ise_Pos - (STRPTR) ise->ise_Fib.fib_FileName);

				*ppos = '\0';
				}

			d1(KPrintF("%s/%s/%ld: CreateIcon(%s)\n", __FILE__, __FUNC__, __LINE__, rild.rild_Name));

			in = CreateIcon(iwt, &rild, &bdl, NULL);

			d1(kprintf("%s/%s/%ld: in=%08lx\n", __FILE__, __FUNC__, __LINE__, in));

			if (in)
				{
				d1(KPrintF("%s/%s/%ld: Name=<%s>  rild_IconType=%ld\n", \
					__FILE__, __FUNC__, __LINE__, rild.rild_Name, rild.rild_IconType));

				SetIconSupportsFlags(in, rilc->rilc_DiskWritable);
				}
			}

		if (iwt->iwt_WindowTask.mt_MainObject && n++ > 5)
			{
			n = 0;
			if (DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_CheckForMessages))
				sdResult = SCANDIR_ABORT;
			}

		ise->ise_Flags |= ISEFLG_Used;
		}

	d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

	for (n = 0, ise = (struct IconScanEntry *) rilc->rilc_NonIconScanList.lh_Head;
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

		if (!CurrentPrefs.pref_ShowThumbnailsAsDefault && (THUMBNAILS_AsDefault == iwt->iwt_ThumbnailMode))
			{
			if (!IsShowAllType(iwt->iwt_OldShowType))
				{
				if (NULL == ise->ise_IconPtr)
					ShowEntry = FALSE;	// Display thumbnail only if object has a icon
				}
			}

		if (THUMBNAILS_Never == iwt->iwt_ThumbnailMode)
			{
			if (!IsShowAllType(iwt->iwt_OldShowType))
				{
				ShowEntry = (NULL != ise->ise_IconPtr);	// Object with icon
				}
			else
				{
				if (NULL != ise->ise_IconPtr)
					ShowEntry = FALSE;	// suppress object if icon attached
				}
			}

		if (ShowEntry && IsFileHidden(&ise->ise_Fib))
			ShowEntry = FALSE;

		d1(kprintf("%s/%s/%ld: ShowEntry=%ld\n", __FILE__, __FUNC__, __LINE__, ShowEntry));

		if (ShowEntry)
			{
			struct ScaIconNode *in;
			struct ReadIconListData rild;

			rild.rild_DiskWriteProtected = !rilc->rilc_DiskWritable;

			ScanDirFillRildFromIse(&rild, ise);
			rild.rild_CheckOverlap = iwt->iwt_CheckOverlappingIcons;

			d1(KPrintF("%s/%s/%ld: CreateIcon(%s)  ObjPtr=%08lx  IconPtr=%08lx\n", \
				__FILE__, __FUNC__, __LINE__, rild.rild_Name, \
                                ise->ise_ObjPtr, ise->ise_IconPtr));

			if (ise->ise_IconPtr)
				in = CreateIcon(iwt, &rild, &bdl, NULL);
			else
				in = CreateDefaultIcon(iwt, &rild, &bdl, NULL, TRUE);

			d1(kprintf("%s/%s/%ld: in=%08lx\n", __FILE__, __FUNC__, __LINE__, in));

			if (in)
				{
				d1(KPrintF("%s/%s/%ld: Name=<%s>  rild_IconType=%ld\n", \
					__FILE__, __FUNC__, __LINE__, rild.rild_Name, rild.rild_IconType));

				SetIconSupportsFlags(in, rilc->rilc_DiskWritable);
				}
			}

		if (iwt->iwt_WindowTask.mt_MainObject && n++ > 5)
			{
			n = 0;
			if (DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_CheckForMessages))
				sdResult = SCANDIR_ABORT;
			}

		ise->ise_Flags |= ISEFLG_Used;
		}

	FreeBackdropFile(&bdl);

	d1(KPrintF("%s/%s/%ld: END\n", __FILE__, __FUNC__, __LINE__));

	return sdResult;
}


// compare function for icon list sorting
// drawers are sorted first
static SAVEDS(LONG) CompareNameFunc(
	struct Hook *hook,
	struct ScaIconNode *in2,
	struct ScaIconNode *in1)
{
	d1(kprintf("%s/%s/%ld: in1=%08lx  in2=%08lx\n", __FILE__, __FUNC__, __LINE__, in1, in2));

	(void) hook;

	if (IntuitionBase)
		{
		ULONG IconType1, IconType2;

		GetAttr(IDTA_Type, in1->in_Icon, &IconType1);
		GetAttr(IDTA_Type, in2->in_Icon, &IconType2);

		if (WBDRAWER == IconType2)
			{
			if (WBDRAWER != IconType1)
				return -1;
			}
		else
			{
			if (WBDRAWER == IconType1)
				return 1;
			}
		}

	return Stricmp(in2->in_Name, in1->in_Name);
}


void SetIconSupportsFlags(struct ScaIconNode *in, BOOL isDiskWritable)
{
	ULONG IconType;

	if (NULL == in || NULL == in->in_Icon)
		return;

	GetAttr(IDTA_Type, in->in_Icon, &IconType);

	d1(kprintf("%s/%s/%ld: Name=<%s>  IconType=%ld  in_SupportFlags=%08lx\n", __FILE__, __FUNC__, __LINE__, GetIconName(in), IconType, in->in_SupportFlags));

	if (WBGARBAGE == IconType)
		{
		in->in_SupportFlags |= INF_SupportsEmptyTrash;
		in->in_SupportFlags &= ~INF_SupportsDelete;
		}
	if (!isDiskWritable)
		{
		in->in_SupportFlags &= ~(INF_SupportsDelete | INF_SupportsRename | 
			INF_SupportsSnapshot | INF_SupportsUnSnapshot | 
			INF_SupportsPutAway | INF_SupportsLeaveOut |
			INF_SupportsEmptyTrash );
		}
	if (in->in_Flags & INF_DefaultIcon)
		{
		// forbid "Leave Out" for default icons
		in->in_SupportFlags &= ~(INF_SupportsLeaveOut);
		d1(kprintf("%s/%s/%ld: in=<%s>  supportFlags=%08lx\n", \
			__FILE__, __FUNC__, __LINE__, GetIconName(in), in->in_SupportFlags));
		}
	if (in->in_DeviceIcon)
		{
		// forbid "Leave Out" and "Put away" for device icons
		in->in_SupportFlags &= ~(INF_SupportsLeaveOut | INF_SupportsPutAway);
		}

	d1(kprintf("%s/%s/%ld: in=%08lx  <%s>  isDiskWritable=%ld  SupportFlags=%08lx\n", \
		__FILE__, __FUNC__, __LINE__, in, in->in_Name, isDiskWritable, in->in_SupportFlags));
}


BOOL IsSoftLink(CONST_STRPTR Name)
{
	BOOL isLink = FALSE;
	LONG Success;
	char *Buffer = NULL;
	BPTR fLock = (BPTR)NULL;

	d1(kprintf("%s/%s/%ld: Name=<%s>\n", __FILE__, __FUNC__, __LINE__, Name));

	do	{
		struct MsgPort *FileSysPort;
		BPTR dirLock;

		dirLock = CurrentDir((BPTR)NULL);
		CurrentDir(dirLock);

		if ((BPTR)NULL == dirLock)
			break;

		d1(kprintf("%s/%s/%ld: dirLock=%08lx\n", __FILE__, __FUNC__, __LINE__, dirLock));
		debugLock_d1(dirLock);

		FileSysPort = ((struct FileLock *) BADDR(dirLock))->fl_Task;

		Buffer = AllocPathBuffer();
		d1(kprintf("%s/%s/%ld: Buffer=%08lx\n", __FILE__, __FUNC__, __LINE__, Buffer));
		if (NULL == Buffer)
			break;

		// make BSTRING from Name (prepend length byte)
		stccpy(Buffer + 1, Name, Max_PathLen - 1);
		*Buffer = strlen(Name);

		Success = DoPkt(FileSysPort,
				ACTION_LOCATE_OBJECT,
				dirLock,
				MKBADDR(Buffer),
				ACCESS_READ,
				0, 0);

		d1(kprintf("%s/%s/%ld: Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

		if (Success)
			{
			fLock = (BPTR) Success;
			d1(kprintf("%s/%s/%ld: Success=%08lx  IoErr=%ld\n", __FILE__, __FUNC__, __LINE__, Success, IoErr()));
			}
		else
			{
			LONG errCode = IoErr();

			d1(kprintf("%s/%s/%ld: Success=%08lx  IoErr=%ld\n", __FILE__, __FUNC__, __LINE__, Success, errCode));

			if (ERROR_IS_SOFT_LINK == errCode)
				isLink = TRUE;
			}
		} while (0);

	d1(kprintf("%s/%s/%ld: Buffer=%08lx\n", __FILE__, __FUNC__, __LINE__, Buffer));
	if (fLock)
		UnLock(fLock);
	d1(kprintf("%s/%s/%ld: Buffer=%08lx\n", __FILE__, __FUNC__, __LINE__, Buffer));
	if (Buffer)
		FreePathBuffer(Buffer);

	d1(kprintf("%s/%s/%ld: END isLink=%ld\n", __FILE__, __FUNC__, __LINE__, isLink));

	return isLink;
}


static void RilcDisposeData(void *data)
{
	// No-op since data is only handed as reference to IconScanEntry node
	// and IconScanEntry nodes are managed within rilc_NonIconScanList
	(void) data;
}


static void RilcDisposeKey(void *key)
{
	// No-op since key is only handed as reference to IconScanEntry->ise_Fib.fib_FileName
	// and IconScanEntry nodes are managed within rilc_NonIconScanList
	(void) key;
}

static int RilcCompare(const void *key1, const void *key2)
{
	return Stricmp((CONST_STRPTR) key2, (CONST_STRPTR) key1);
}



void RilcInit(struct ReadIconListControl *rilc)
{
	memset(rilc, 0, sizeof(struct ReadIconListControl));

	NewList(&rilc->rilc_IconScanList);
	NewList(&rilc->rilc_NonIconScanList);
	NewList(&rilc->rilc_UnlinkedIconScanList);

	rilc->rilc_replyPort = NULL;
	rilc->rilc_pkt = NULL;
	rilc->rilc_exallControl = NULL;
	rilc->rilc_ExAllBuffer = NULL;
	rilc->rilc_dirLock = (BPTR)NULL;
	rilc->rilc_FlagFinished = FALSE;
	rilc->rilc_edNext = NULL;
	rilc->rilc_fib = NULL;
	rilc->rilc_CleanupCount = 0;
	rilc->rilc_PacketPending = FALSE;
	rilc->rilc_TotalFound = 0;

	rilc->rilc_StdFilesTree = BTreeCreate(RilcDisposeData, RilcDisposeKey, RilcCompare);
	d1(KPrintF("%s/%s/%ld: rilc_StdFilesTree=%08lx\n", __FILE__, __FUNC__, __LINE__, rilc->rilc_StdFilesTree));
}


void RilcCleanup(struct ReadIconListControl *rilc)
{
	struct IconScanEntry *ise;

	if (rilc->rilc_PacketPending && rilc->rilc_replyPort)
		{
		d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

		rilc->rilc_PacketPending = FALSE;

		d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		WaitPort(rilc->rilc_replyPort);
		d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		GetMsg(rilc->rilc_replyPort);
		}

	while ((ise = (struct IconScanEntry *) RemTail(&rilc->rilc_IconScanList)))
		{
		DisposeIconScanEntry(ise);
		}
	while ((ise = (struct IconScanEntry *) RemTail(&rilc->rilc_UnlinkedIconScanList)))
		{
		DisposeIconScanEntry(ise);
		}
	while ((ise = (struct IconScanEntry *) RemTail(&rilc->rilc_NonIconScanList)))
		{
		DisposeIconScanEntry(ise);
		}

	if (rilc->rilc_StdFilesTree)
		{
		BTreeDispose(rilc->rilc_StdFilesTree);
		rilc->rilc_StdFilesTree = NULL;
		}
	if (rilc->rilc_fib)
		{
		FreeDosObject(DOS_FIB, rilc->rilc_fib);
		rilc->rilc_fib = NULL;
		}
	if (rilc->rilc_exallControl)
		{
		FreeDosObject(DOS_EXALLCONTROL, rilc->rilc_exallControl);
		rilc->rilc_exallControl = NULL;
		}
	if (rilc->rilc_pkt)
		{
		FreeDosObject(DOS_STDPKT, rilc->rilc_pkt);
		rilc->rilc_pkt = NULL;
		}
	if (rilc->rilc_dirLock)
		{
		UnLock(rilc->rilc_dirLock);
		rilc->rilc_dirLock = (BPTR)NULL;
		}
	if (rilc->rilc_replyPort)
		{
		DeleteMsgPort(rilc->rilc_replyPort);
		rilc->rilc_replyPort = NULL;
		}
	if (rilc->rilc_ExAllBuffer)
		{
		FreeVec(rilc->rilc_ExAllBuffer);
		rilc->rilc_ExAllBuffer = NULL;
		}
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


static struct IconScanEntry *NewIconScanEntry(const struct ReadIconListData *rild)
{
	struct IconScanEntry *ise;

	ise = ScalosAllocVecPooled(sizeof(struct IconScanEntry));
	if (ise)
		{
		ise->ise_IconObj = NULL;

		d1(kprintf("%s/%s/%ld: <%s> comment=<%s>\n", __FILE__, __FUNC__, __LINE__, rild->rild_Name, rild->rild_Comment));

		stccpy(ise->ise_Fib.fib_FileName, rild->rild_Name, sizeof(ise->ise_Fib.fib_FileName));
		ise->ise_Fib.fib_DirEntryType = rild->rild_Type;
		ScalosSetFibSize64(&ise->ise_Fib, rild->rild_Size64);
		ise->ise_Fib.fib_Protection = rild->rild_Protection;
		ise->ise_Fib.fib_Date = rild->rild_DateStamp;
		stccpy(ise->ise_Fib.fib_Comment, rild->rild_Comment, sizeof(ise->ise_Fib.fib_Comment));

		// guess ise_IconType and set ISEFLG_IsLink flag
		switch (rild->rild_Type)
			{
		case ST_ROOT:
			ise->ise_IconType = WBDISK;
			break;
		case ST_USERDIR:
			ise->ise_IconType = WBDRAWER;
			break;
		case ST_LINKDIR:
			ise->ise_Flags |= ISEFLG_IsLink;
			ise->ise_IconType = WBDRAWER;
			break;
		case ST_SOFTLINK:
		case ST_LINKFILE:
			ise->ise_Flags |= ISEFLG_IsLink;
			ise->ise_IconType = WBPROJECT;
			break;
		default:
			if (IsSoftLink(rild->rild_Name))
				ise->ise_Flags |= ISEFLG_IsLink;

			if (!(ise->ise_Fib.fib_Protection & FIBF_EXECUTE))
				ise->ise_IconType = WBTOOL;
			else
				ise->ise_IconType = WBPROJECT;
			break;
			}

		ise->ise_Pos = IsIconName(ise->ise_Fib.fib_FileName);

		if (0 != ise->ise_Pos && ~0 != ise->ise_Pos)
			{
			ise->ise_Flags |= ISEFLG_IsIcon;

			if (rild->rild_FetchIconType)
				{
				ise->ise_PosChar = *((char *) ise->ise_Pos);
				*((char *) ise->ise_Pos) = '\0';		// strip ".info" from name

				// NewIconObject()
				ise->ise_IconObj = NewIconObjectTags(ise->ise_Fib.fib_FileName,
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
					IDTA_TextPen, PalettePrefs.pal_PensList[PENIDX_ICONTEXTPEN],
					IDTA_TextPenSel, PalettePrefs.pal_PensList[PENIDX_ICONTEXTPENSEL],
					IDTA_TextPenShadow, PalettePrefs.pal_PensList[PENIDX_ICONTEXTSHADOWPEN],
					IDTA_TextPenOutline, PalettePrefs.pal_PensList[PENIDX_ICONTEXTOUTLINEPEN],
					IDTA_TextPenBgSel, PalettePrefs.pal_PensList[PENIDX_ICONTEXTPENBGSEL],
					IDTA_SelectedTextRectangle, CurrentPrefs.pref_SelectedTextRectangle,
					IDTA_SelTextRectBorderX, CurrentPrefs.pref_SelTextRectBorderX,
					IDTA_SelTextRectBorderY, CurrentPrefs.pref_SelTextRectBorderY,
					IDTA_SelTextRectRadius, CurrentPrefs.pref_SelTextRectRadius,
					IDTA_TextMode, CurrentPrefs.pref_TextMode,
					IDTA_TextDrawMode, FontPrefs.fprf_TextDrawMode,
					IDTA_TextBackPen, FontPrefs.fprf_FontBackPen,
					IDTA_TextStyle, (ise->ise_Flags & ISEFLG_IsLink) ? CurrentPrefs.pref_LinkTextStyle : FS_NORMAL,
					IDTA_SupportedIconTypes, CurrentPrefs.pref_SupportedIconTypes,
					IDTA_SizeConstraints, (ULONG) &CurrentPrefs.pref_IconSizeConstraints,
					TAG_END);

				d1(KPrintF("%s/%s/%ld: Name=<%s>  IconObj=%08lx\n", \
						__FILE__, __FUNC__, __LINE__, ise->ise_Fib.fib_FileName, ise->ise_IconObj));
				if (ise->ise_IconObj)
					{
					ise->ise_IconType = AdjustIconType(ise->ise_IconObj, rild->rild_Type);

					d1(KPrintF("%s/%s/%ld: IconType=%ld\n", \
						__FILE__, __FUNC__, __LINE__, ise->ise_IconType));
					}

				*((char *) ise->ise_Pos) = ise->ise_PosChar;		// restore file name
				}
			}
		}

	return ise;
}


static void DisposeIconScanEntry(struct IconScanEntry *ise)
{
	if (ise)
		{
		if (ise->ise_IconObj)
			{
			DisposeIconObject(ise->ise_IconObj);
			ise->ise_IconObj = NULL;
			}

		ScalosFreeVecPooled(ise);
		}
}


// build links between icons and objects
enum ScanDirResult LinkIconScanList(struct internalScaWindowTask *iwt,
	struct ReadIconListControl *rilc)
{
	struct IconScanEntry *iseIcon;
	struct IconScanEntry *iseNext;
	enum ScanDirResult sdResult = SCANDIR_OK;
	ULONG n = 0;

	d1(KPrintF("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	for (iseIcon = (struct IconScanEntry *) rilc->rilc_UnlinkedIconScanList.lh_Head;
		iseIcon != (struct IconScanEntry *) &rilc->rilc_UnlinkedIconScanList.lh_Tail;
		iseIcon = iseNext)
		{
		d1(kprintf("%s/%s/%ld: Name=<%s>\n", \
				__FILE__, __FUNC__, __LINE__, iseIcon->ise_Fib.fib_FileName));

		iseNext = (struct IconScanEntry *) iseIcon->ise_Node.ln_Succ;

		if (NULL == iseIcon->ise_ObjPtr)
			{
			d1(kprintf("%s/%s/%ld: Pos=%08lx\n", __FILE__, __FUNC__, __LINE__, iseIcon->ise_Pos));

			if (iseIcon->ise_Pos != 0 && iseIcon->ise_Pos != ~0)
				{
				struct IconScanEntry *iseObject;
				size_t Length = ((STRPTR) iseIcon->ise_Pos) - (STRPTR) iseIcon->ise_Fib.fib_FileName;
				char key[sizeof(iseIcon->ise_Fib.fib_FileName)];

				strncpy(key, iseIcon->ise_Fib.fib_FileName, Length);
				key[Length] = '\0';

				iseObject = (struct IconScanEntry *) BTreeFind(rilc->rilc_StdFilesTree, key);
				d1(KPrintF("%s/%s/%ld: key=<%s>  iseObject=%08lx\n", __FILE__, __FUNC__, __LINE__, key, iseObject));

				if (iseObject && iseIcon != iseObject && NULL == iseObject->ise_IconPtr)
					{
					d1(kprintf("%s/%s/%ld: Name=<%s>\n", \
						__FILE__, __FUNC__, __LINE__, iseObject->ise_Fib.fib_FileName));

					BTreeHide(rilc->rilc_StdFilesTree, key);

					iseIcon->ise_ObjPtr = iseObject;
					iseObject->ise_IconPtr = iseIcon;
					iseObject->ise_IconType = iseIcon->ise_IconType;

					// Move completed icon node from rilc_UnlinkedIconScanList to rilc_IconScanList
					Remove(&iseIcon->ise_Node);
					AddTail(&rilc->rilc_IconScanList, &iseIcon->ise_Node);
					}
				}
			}
		if (iwt->iwt_WindowTask.mt_MainObject && n++ >= 20)
			{
			n = 0;
			if (DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_CheckForMessages))
				sdResult = SCANDIR_ABORT;
			}
		}

	d1(KPrintF("%s/%s/%ld:  END\n", __FILE__, __FUNC__, __LINE__));

	return sdResult;
}


enum ScanDirResult GetFileList(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc,
	enum ScanDirResult (*CheckFunc)(struct internalScaWindowTask *iwt, struct ReadIconListControl *rilc),
	BOOL UseExAll, BOOL FetchIconType, BOOL CheckOverlap)
{
	UBYTE OldViewType = iwt->iwt_WindowTask.mt_WindowStruct->ws_Viewmodes;
	enum ScanDirResult sdResult;
	struct MsgPort *oldFsTask;
	BPTR oldDir;

	rilc->rilc_Check = CheckFunc;

	DateStamp(&iwt->iwt_LastIconUpdateTime);		// remember time of last icon update

	d1(KPrintF("%s/%s/%ld: START  UseExAll=%ld  iwt=%08lx\n", __FILE__, __FUNC__, __LINE__, UseExAll, iwt));

	rilc->rilc_ExAllBuffer = AllocVec(ExAllBuffer_SIZE, MEMF_PUBLIC | MEMF_CLEAR);
	if (NULL == rilc->rilc_ExAllBuffer)
		return 0;

	d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

	rilc->rilc_rd.rild_FetchIconType = FetchIconType;
	rilc->rilc_rd.rild_Type = 0;
	rilc->rilc_rd.rild_CheckOverlap = CheckOverlap;
	rilc->rilc_rd.rild_SoloIcon = FALSE;

	d1(kprintf("%s/%s/%ld: iwt->iwt_WindowTask.mt_WindowStruct=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_WindowTask.mt_WindowStruct));

	if (iwt->iwt_WindowTask.mt_WindowStruct)
		{
		d1(kprintf("%s/%s/%ld: ws_Lock=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_WindowTask.mt_WindowStruct->ws_Lock));
		debugLock_d1(iwt->iwt_WindowTask.mt_WindowStruct->ws_Lock);
		rilc->rilc_dirLock = iwt->iwt_WindowTask.mt_WindowStruct->ws_Lock;
		}
	else
		{
		rilc->rilc_dirLock = CurrentDir((BPTR)NULL);
		CurrentDir(rilc->rilc_dirLock);
		d1(kprintf("%s/%s/%ld: DirLock=%08lx\n", __FILE__, __FUNC__, __LINE__, rilc->rilc_dirLock));
		}

	if ((BPTR)NULL == rilc->rilc_dirLock)
		return SCANDIR_FINISHED;

	rilc->rilc_dirLock = DupLock(rilc->rilc_dirLock);

	if ((BPTR)NULL == rilc->rilc_dirLock)
		return SCANDIR_FINISHED;

	rilc->rilc_DiskWritable = !iwt->iwt_ReadOnly;
	rilc->rilc_rd.rild_DiskWriteProtected = !rilc->rilc_DiskWritable;

#if defined(ED_SIZE64)
	rilc->rilc_ExAllType = ED_SIZE64;
#else /* ED_SIZE64 */
	rilc->rilc_ExAllType = ED_OWNER;
#endif /* ED_SIZE64 */

	d1(kprintf("%s/%s/%ld: Disk Writable=%ld\n", __FILE__, __FUNC__, __LINE__, rilc->rilc_DiskWritable));

	oldDir = CurrentDir(rilc->rilc_dirLock);

	oldFsTask = SetFileSysTask(((struct FileLock *) BADDR(rilc->rilc_dirLock))->fl_Task);

	debugLock_d1(rilc->rilc_dirLock);

	if (UseExAll)
		{
		sdResult = BeginScan_ExAll(iwt, rilc);
		}
	else
		{
		sdResult = BeginScan_Examine(iwt, rilc);
		}

	d1(kprintf("%s/%s/%ld: sdResult=%ld\n", __FILE__, __FUNC__, __LINE__, sdResult));

	while (SCANDIR_OK == sdResult || SCANDIR_FAIL_RETRY == sdResult)
		{
		if (UseExAll)
			{
			sdResult = ScanDir_ExAll(iwt, rilc);

			switch (sdResult)
				{
			case SCANDIR_EXALL_BADNUMBER:
				// failure because of unsupported rilc_ExAllType.
				// retry with ED_COMMENT because 
				// All filesystems supporting ExAll() must support through ED_COMMENT

				if (rilc->rilc_exallControl)
					{
					FreeDosObject(DOS_EXALLCONTROL, rilc->rilc_exallControl);
					rilc->rilc_exallControl = NULL;
					}
				if (rilc->rilc_pkt)
					{
					FreeDosObject(DOS_STDPKT, rilc->rilc_pkt);
					rilc->rilc_pkt = NULL;
					}
				if (rilc->rilc_replyPort)
					{
					DeleteMsgPort(rilc->rilc_replyPort);
					rilc->rilc_replyPort = NULL;
					}

				rilc->rilc_ExAllType = ED_COMMENT;
				sdResult = BeginScan_ExAll(iwt, rilc);
				break;

			case SCANDIR_EXALL_FAIL:
				UseExAll = FALSE;

				if (rilc->rilc_exallControl)
					{
					FreeDosObject(DOS_EXALLCONTROL, rilc->rilc_exallControl);
					rilc->rilc_exallControl = NULL;
					}
				if (rilc->rilc_pkt)
					{
					FreeDosObject(DOS_STDPKT, rilc->rilc_pkt);
					rilc->rilc_pkt = NULL;
					}
				if (rilc->rilc_replyPort)
					{
					DeleteMsgPort(rilc->rilc_replyPort);
					rilc->rilc_replyPort = NULL;
					}

				sdResult = BeginScan_Examine(iwt, rilc);
				break;

			default:
				break;
				}
			}
		else
			{
			sdResult = ScanDir_Examine(iwt, rilc);
			}

		d1(KPrintF("%s/%s/%ld: sdResult=%ld  OldViewType=%ld  ws_Viewmodes=%ld\n", \
			__FILE__, __FUNC__, __LINE__, sdResult, OldViewType, iwt->iwt_WindowTask.mt_WindowStruct->ws_Viewmodes));

		if (OldViewType != iwt->iwt_WindowTask.mt_WindowStruct->ws_Viewmodes)
			sdResult = SCANDIR_VIEWMODE_CHANGE;

		if (iwt->iwt_WindowTask.mt_MainObject && (SCANDIR_OK == sdResult || SCANDIR_FAIL_RETRY == sdResult))
			{
			if (DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_CheckForMessages))
				sdResult = SCANDIR_ABORT;
			}
		}

	d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

	CurrentDir(oldDir);
	SetFileSysTask(oldFsTask);

	d1(KPrintF("%s/%s/%ld: END\n", __FILE__, __FUNC__, __LINE__));

	return sdResult;
}


// add a single file/directory to rilc->rilc_IconScanList
LONG AddFileToFilesList(struct ReadIconListControl *rilc, BPTR dirLock, CONST_STRPTR Name)
{
	BPTR oldDir;
	BPTR fLock = (BPTR)NULL;
	LONG Result = RETURN_OK;

	debugLock_d1(dirLock);
	d1(kprintf("%s/%s/%ld: Name=<%s>\n", __FILE__, __FUNC__, __LINE__, Name));

	do	{
		struct IconScanEntry *ise;

		oldDir = CurrentDir(dirLock);

		if (NULL == rilc->rilc_fib)
			{
			rilc->rilc_fib = AllocDosObject(DOS_FIB, NULL);
			if (NULL == rilc->rilc_fib)
				{
				Result = IoErr();
				break;
				}
			}

		fLock = Lock((STRPTR) Name, ACCESS_READ);
		if ((BPTR)NULL == fLock)
			{
			Result = IoErr();
			break;
			}

		if (!ScalosExamine64(fLock, rilc->rilc_fib))
			{
			Result = IoErr();
			break;
			}

		rilc->rilc_rd.rild_Type = rilc->rilc_fib->fib_DirEntryType;
		rilc->rilc_rd.rild_Protection = rilc->rilc_fib->fib_Protection;
		rilc->rilc_rd.rild_Size64 = ScalosFibSize64(rilc->rilc_fib);
		rilc->rilc_rd.rild_DateStamp = rilc->rilc_fib->fib_Date;
		rilc->rilc_rd.rild_SoloIcon = FALSE;

		stccpy(rilc->rilc_rd.rild_Name, rilc->rilc_fib->fib_FileName, 
			sizeof(rilc->rilc_rd.rild_Name));

		d1(kprintf("%s/%s/%ld: Name=<%s>  Type=%ld  Prot=%08lx\n", __FILE__, __FUNC__, __LINE__, \
			rilc->rilc_rd.rild_Name, rilc->rilc_rd.rild_Type, rilc->rilc_rd.rild_Protection));

		stccpy(rilc->rilc_rd.rild_Comment, rilc->rilc_fib->fib_Comment, 
			sizeof(rilc->rilc_rd.rild_Comment));

		ise = NewIconScanEntry(&rilc->rilc_rd);
		d1(kprintf("%s/%s/%ld: ise=%08lx\n", __FILE__, __FUNC__, __LINE__, ise));
		if (ise)
			{
			if (IsIconName(Name))
				{
				ise->ise_Flags |= ISEFLG_IsIcon;
				AddTail(&rilc->rilc_IconScanList, &ise->ise_Node);
				}
			else
				AddTail(&rilc->rilc_NonIconScanList, &ise->ise_Node);

			rilc->rilc_TotalFound++;
			}
		} while (0);

	if (fLock)
		UnLock(fLock);

	CurrentDir(oldDir);

	d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	return Result;
}


void ScanDirFillRildFromIse(struct ReadIconListData *rild, const struct IconScanEntry *ise)
{
	rild->rild_Type = ise->ise_Fib.fib_DirEntryType;
	rild->rild_Protection = ise->ise_Fib.fib_Protection;
	rild->rild_Size64 = ScalosFibSize64(&ise->ise_Fib);
	rild->rild_DateStamp = ise->ise_Fib.fib_Date;
	rild->rild_IconType = ise->ise_IconType;
	rild->rild_TypeNode = NULL;
	rild->rild_SoloIcon = FALSE;

	d1(KPrintF("%s/%s/%ld: %08lx comment=%08lx  fib_FileName=<%s>...fib_Comment=<%s>\n", \
		__FILE__, __FUNC__, __LINE__, rild->rild_Name, rild->rild_Comment, ise->ise_Fib.fib_FileName, ise->ise_Fib.fib_Comment));

	stccpy(rild->rild_Name, ise->ise_Fib.fib_FileName, sizeof(rild->rild_Name));
	stccpy(rild->rild_Comment, ise->ise_Fib.fib_Comment, sizeof(rild->rild_Comment));

	d1(KPrintF("%s/%s/%ld: END <%s> comment=<%s>  IconType=%ld\n", \
		__FILE__, __FUNC__, __LINE__, rild->rild_Name, rild->rild_Comment, rild->rild_IconType));
}


void ScanDirUpdateStatusBarText(struct internalScaWindowTask *iwt, ULONG TotalIcons)
{
	if (iwt->iwt_StatusBar && iwt->iwt_StatusBarMembers[STATUSBARGADGET_StatusText])
		{
		char StatusBarText[256];

		ScaFormatStringMaxLength(StatusBarText, sizeof(StatusBarText), 
			GetLocString(MSGID_STATUSBARTEXT), TotalIcons, 0);

		d1(kprintf("%s/%s/%ld: StatusBarText=<%s>\n", __FILE__, __FUNC__, __LINE__, StatusBarText));

		DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_UpdateStatusBar,
			iwt->iwt_StatusBarMembers[STATUSBARGADGET_StatusText],
			GBTDTA_Text, StatusBarText,
			TAG_END);
		}
}


BOOL IsFileHidden(const struct FileInfoBlock *fib)
{
	if (CurrentPrefs.pref_HideHiddenFlag && ('.' == fib->fib_FileName[0]))
		return TRUE;

	if (CurrentPrefs.pref_HideProtectHiddenFlag && (fib->fib_Protection & FIBF_HIDDEN))
		return TRUE;

	return FALSE;
}


static ULONG AdjustIconType(Object *IconObj, LONG DirEntryType)
{
	ULONG IconType;
	ULONG NewIconType;

	GetAttr(IDTA_Type, IconObj, &IconType);
	NewIconType = IconType;

	d1(KPrintF("%s/%s/%ld: IconType=%lu DirEntryType=%ld\n", __FILE__, __FUNC__, __LINE__, IconType, DirEntryType));

	// Try to adjust IconType for icons with wrong type
	switch (DirEntryType)
		{
	case ST_ROOT:
		if (WBDISK != IconType)
			{
			NewIconType = WBDISK;
			}
		break;
	case ST_USERDIR:
	case ST_LINKDIR:
		if (WBDRAWER != IconType)
			{
			NewIconType = WBDRAWER;
			}
		break;
	case ST_FILE:
	case ST_LINKFILE:
		if (WBTOOL != IconType && WBPROJECT != IconType)
			{
			NewIconType = WBPROJECT;
			}
		break;
	default:
		break;
		}

	d1(KPrintF("%s/%s/%ld: NewIconType=%lu\n", __FILE__, __FUNC__, __LINE__, NewIconType));
	if (IconType != NewIconType)
		{
		SetAttrs(IconObj,
			IDTA_Type, NewIconType,
			TAG_END);
		}

	return NewIconType;
}


static BOOL ScanDirFindIcon(struct internalScaWindowTask *iwt, CONST_STRPTR IconName)
{
	struct ScaIconNode *in;
	BOOL IconFound = FALSE;

	ScalosLockIconListShared(iwt);

	for (in=iwt->iwt_WindowTask.wt_IconList; !IconFound && in; in = (struct ScaIconNode *) in->in_Node.mln_Succ)
		{
		if (0 == Stricmp((STRPTR) GetIconName(in), (STRPTR) IconName))
			{
			d1(kprintf("%s/%s/%ld: Icon found <%s>\n", __FILE__, __FUNC__, __LINE__, GetIconName(in)));
			IconFound = TRUE;
			break;
			}
		}

	for (in=iwt->iwt_WindowTask.wt_LateIconList; !IconFound && in; in = (struct ScaIconNode *) in->in_Node.mln_Succ)
		{
		if (0 == Stricmp((STRPTR) GetIconName(in), (STRPTR) IconName))
			{
			d1(kprintf("%s/%s/%ld: Icon found <%s>\n", __FILE__, __FUNC__, __LINE__, GetIconName(in)));
			IconFound = TRUE;
			break;
			}
		}

	ScalosUnLockIconList(iwt);

	return IconFound;
}


