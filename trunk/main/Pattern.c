// Patterns.c
// $Date$
// $Revision$


#include <exec/types.h>
#include <graphics/rastport.h>
#include <utility/hooks.h>
#include <intuition/gadgetclass.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>
#include <dos/dostags.h>
#include <guigfx/guigfx.h>
#include <cybergraphx/cybergraphics.h>

#define	__USE_SYSBASE

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/layers.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/icon.h>
#include <proto/iconobject.h>
#include <proto/iffparse.h>
#include <proto/guigfx.h>
#include <proto/datatypes.h>
#include <proto/cybergraphics.h>
#include "debug.h"
#include <proto/scalos.h>
#include <proto/scalosgfx.h>

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
#include "locale.h"
#include "Variables.h"

#include "IconImageClass.h"

//----------------------------------------------------------------------------

// local data structures

//----------------------------------------------------------------------------

// local functions

static BOOL SetBackFill_OkRendered(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo);
static BOOL SetBackFill_AsyncRead(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo, struct Screen *scr);
static SAVEDS(void) AsyncBackFillFunc(APTR arg, struct SM_AsyncBackFill *msg);
static BOOL ReadBackFill(struct internalScaWindowTask *iwt, struct PatternNode *ptNode,
	struct PatternInfo *ptInfo, struct Screen *scr);
static BOOL ReadBackFill_NoGuiGfx(struct internalScaWindowTask *iwt, struct PatternNode *ptNode,
	struct PatternInfo *ptInfo, struct Screen *scr);
static BOOL ReadBackFill_Enhanced(struct internalScaWindowTask *iwt, struct PatternNode *ptNode,
	struct PatternInfo *ptInfo, struct Screen *scr);
static SAVEDS(ULONG) BackFillHookFunc(struct Hook *bfHook, struct RastPort *rp, struct BackFillMsg *msg);
static void PatternFreeNodeBitMap(struct PatternNode *ptNode);
static void PatternFreeInfoBitMap(struct PatternInfo *ptInfo);
static BOOL PatternCreateCenteredBitMap(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo);
static BOOL PatternCreateScaledBitMapMin(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo);
static BOOL PatternCreateScaledBitMapMax(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo);
static BOOL PatternCreateScaledBitMapGUIGfx(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo,
	ULONG ScaledWidth, ULONG ScaledHeight);
static BOOL PatternCreateScaledBitMapNoGuiGFX(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo,
	ULONG ScaledWidth, ULONG ScaledHeight);
static BOOL PatternBackgroundHorizontalGradient(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo, struct RastPort *rp);
static BOOL PatternBackgroundVerticalGradient(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo, struct RastPort *rp);
static BOOL PatternBackgroundSingleColor(struct PatternNode *ptNode,
	struct PatternInfo *ptInfo, struct RastPort *rp);

//----------------------------------------------------------------------------

// local data items

//----------------------------------------------------------------------------

void PatternsOff(struct MainTask *mt, struct MsgPort *ReplyPort)
{
	struct ScaWindowStruct *ws;
	ULONG MsgCount = 0;

	d1(KPrintF("%s/%s/%ld: START  <%s>\n", __FILE__, __FUNC__, __LINE__, ((struct Process *) FindTask(NULL))->pr_Task.tc_Node.ln_Name));

	SCA_LockWindowList(SCA_LockWindowList_Shared);

	for (ws=winlist.wl_WindowStruct; ws; ws = (struct ScaWindowStruct *) ws->ws_Node.mln_Succ)
		{
		struct ScalosMessage *sMsg = SCA_AllocMessage(MTYP_NewPattern, 0);
		if (sMsg)
			{
			sMsg->sm_Message.mn_ReplyPort = ReplyPort;

			MsgCount++;
			d1(KPrintF("%s/%s/%ld: PutMsg iwt=%08lx  Msg=%08lx \n", __FILE__, __FUNC__, __LINE__, ws->ws_WindowTask, &sMsg->sm_Message));
			PutMsg(ws->ws_MessagePort, &sMsg->sm_Message);
			}
		}

	SCA_UnLockWindowList();

	d1(KPrintF("%s/%s/%ld: before WaitReply()  MsgCount=%lu\n", __FILE__, __FUNC__, __LINE__, MsgCount));

	while (MsgCount--)
		WaitReply(ReplyPort, &mt->mwt, MTYP_NewPattern);

	d1(KPrintF("%s/%s/%ld: before WaitReply()  MsgCount=%lu\n", __FILE__, __FUNC__, __LINE__, MsgCount));

	if (mt->screenbackfill)
		{
		struct Layer *newLayer;

		d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		ScaLockScreenLayers();

		InstallLayerInfoHook(&iInfos.xii_iinfos.ii_Screen->LayerInfo, NULL);

		newLayer = CreateBehindLayer(&iInfos.xii_iinfos.ii_Screen->LayerInfo,
			iInfos.xii_iinfos.ii_Screen->RastPort.BitMap,
			iInfos.xii_iinfos.ii_Screen->LeftEdge,
			iInfos.xii_iinfos.ii_Screen->TopEdge,
			iInfos.xii_iinfos.ii_Screen->LeftEdge + iInfos.xii_iinfos.ii_Screen->Width - 1,
			iInfos.xii_iinfos.ii_Screen->TopEdge + iInfos.xii_iinfos.ii_Screen->Height - 1,
			LAYERBACKDROP | LAYERSIMPLE,
			NULL
			);

		DeleteLayer(0L, newLayer);

		ScaUnlockScreenLayers();

		mt->screenbackfill = FALSE;

		FreeBackFill(&mt->mwt.iwt_WindowTask.wt_PatternInfo);
		}

	d1(KPrintF("%s/%s/%ld: END", __FILE__, __FUNC__, __LINE__));
}


void PatternsOn(struct MainTask *mt)
{
	struct ScaWindowStruct *ws;
	APTR ScreenPatternNode;

	SCA_LockWindowList(SCA_LockWindowList_Shared);

	d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

	for (ws=winlist.wl_WindowStruct; ws; ws = (struct ScaWindowStruct *) ws->ws_Node.mln_Succ)
		{
		struct PatternNode *pNode;

		pNode = GetPatternNode(ws->ws_PatternNumber);
		if (pNode)
			{
			struct SM_NewPattern *sMsg = (struct SM_NewPattern *) SCA_AllocMessage(MTYP_NewPattern, 0);
			if (sMsg)
				{
				sMsg->smnp_PatternNode = pNode;

				d1(kprintf("%s/%s/%ld: PutMsg iwt=%08lx  Msg=%08lx \n", __FILE__, __FUNC__, __LINE__, ws->ws_WindowTask, &sMsg->ScalosMessage.sm_Message));
				PutMsg(ws->ws_MessagePort, &sMsg->ScalosMessage.sm_Message);
				}
			}
		}

	SCA_UnLockWindowList();

	ScreenPatternNode = GetPatternNode(PatternPrefs.patt_DefScreenPatternNr);
	if (ScreenPatternNode)
		{
		struct Layer *newLayer;

		mt->mwt.iwt_WindowTask.wt_PatternInfo.ptinf_width = iInfos.xii_iinfos.ii_Screen->Width;
		mt->mwt.iwt_WindowTask.wt_PatternInfo.ptinf_height = iInfos.xii_iinfos.ii_Screen->Height;

		mt->mwt.iwt_WindowTask.wt_PatternInfo.ptinf_hook.h_Data = mt;

		if (SetBackFill(NULL, ScreenPatternNode, &mt->mwt.iwt_WindowTask.wt_PatternInfo, SETBACKFILLF_NOASYNC, iInfos.xii_iinfos.ii_Screen))
			{
			mt->screenbackfill = TRUE;

			d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
			ScaLockScreenLayers();

			InstallLayerInfoHook(&iInfos.xii_iinfos.ii_Screen->LayerInfo, NULL);

			newLayer = CreateBehindLayer(&iInfos.xii_iinfos.ii_Screen->LayerInfo,
				iInfos.xii_iinfos.ii_Screen->RastPort.BitMap,
				iInfos.xii_iinfos.ii_Screen->LeftEdge,
				iInfos.xii_iinfos.ii_Screen->TopEdge,
				iInfos.xii_iinfos.ii_Screen->LeftEdge + iInfos.xii_iinfos.ii_Screen->Width - 1,
				iInfos.xii_iinfos.ii_Screen->TopEdge + iInfos.xii_iinfos.ii_Screen->Height - 1,
				LAYERBACKDROP | LAYERSIMPLE,
				NULL
				);

			DeleteLayer(0L, newLayer);

			ScaUnlockScreenLayers();
			}
		}
}


struct PatternNode *GetPatternNode(WORD PatternNo)
{
	struct PatternNode *pNode;

	d1(KPrintF("\n" "%s/%s/%ld: START PatternNo=%ld\n", __FILE__, __FUNC__, __LINE__, PatternNo));

	ScalosObtainSemaphoreShared(&PatternSema);

	switch (PatternNo)
		{
	case PATTERNNR_WorkbenchDefault:
		PatternNo = PatternPrefs.patt_DefWBPatternNr;
		break;
	case PATTERNNR_IconWindowDefault:
		PatternNo = PatternPrefs.patt_DefWindowPatternNr;
		break;
	case PATTERNNR_TextWindowDefault:
		PatternNo = PatternPrefs.patt_DefTextWinPatternNr;
		break;
		}

	d1(kprintf("%s/%s/%ld: PatternNo=%ld\n", __FILE__, __FUNC__, __LINE__, PatternNo));

	pNode = PatternNodes;
	while (pNode && pNode->ptn_number != PatternNo)
		{
		pNode = (struct PatternNode *) pNode->ptn_Node.mln_Succ;
		}

	d1(kprintf("%s/%s/%ld: pNode=%08lx\n", __FILE__, __FUNC__, __LINE__, pNode));

	if (pNode)
		{
		struct PatternNode *pNode2 = pNode;
		LONG ptnCount = 1;

		d1(kprintf("%s/%s/%ld: pNode=%08lx  type=%ld\n", __FILE__, __FUNC__, __LINE__, pNode, pNode->ptn_type));

		// count PatternNode's with given number
		while (pNode2)
			{
			if (pNode2->ptn_number == PatternNo)
				ptnCount++;
			pNode2 = (struct PatternNode *) pNode2->ptn_Node.mln_Succ;
			}

		d1(kprintf("%s/%s/%ld: ptnCount=%ld\n", __FILE__, __FUNC__, __LINE__, ptnCount));

		if (ptnCount > 1)
			{
			ptnCount = RandomNumber(ptnCount - 1) - 1;

			d1(kprintf("%s/%s/%ld: Random=%ld\n", __FILE__, __FUNC__, __LINE__, ptnCount));

			while (pNode && ptnCount > 0)
				{
				if (pNode->ptn_number == PatternNo)
					ptnCount--;
				pNode = (struct PatternNode *) pNode->ptn_Node.mln_Succ;
				}
			}

		d1(kprintf("%s/%s/%ld: pNode=%08lx  type=%ld\n", __FILE__, __FUNC__, __LINE__, pNode, pNode->ptn_type));
		}

	ScalosReleaseSemaphore(&PatternSema);

	d1(KPrintF("%s/%s/%ld: END pNode=%08lx\n", __FILE__, __FUNC__, __LINE__, pNode));

	return pNode;
}


ULONG NewWindowPatternMsg(struct internalScaWindowTask *iwt, struct Message *Msg, APTR p)
{
	d1(KPrintF("%s/%s/%ld: iwt=%08lx <%s>  iwt_AsyncLayoutPending=%ld\n", \
		__FILE__, __FUNC__, __LINE__, iwt, iwt->iwt_WinTitle, iwt->iwt_AsyncLayoutPending));

	if (!iwt->iwt_AsyncLayoutPending)
		{
		struct PatternNode **newPatternNode = (struct PatternNode **) p;

		d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		FreeBackFill(&iwt->iwt_WindowTask.wt_PatternInfo);

		iwt->iwt_WindowTask.mt_WindowStruct->ws_PatternNode = *newPatternNode;

		iwt->iwt_WindowTask.wt_PatternInfo.ptinf_width = iwt->iwt_WindowTask.mt_WindowStruct->ws_Width;
		iwt->iwt_WindowTask.wt_PatternInfo.ptinf_height = iwt->iwt_WindowTask.mt_WindowStruct->ws_Height;

		d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		SetBackFill(iwt, *newPatternNode, &iwt->iwt_WindowTask.wt_PatternInfo, 0, iwt->iwt_WinScreen);

		d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		DoMethod(iwt->iwt_WindowTask.mt_MainObject,
			SCCM_IconWin_Redraw,
			REDRAWF_DontRefreshWindowFrame);
		}

	d1(KPrintF("%s/%s/%ld: END\n", __FILE__, __FUNC__, __LINE__));
	return 0;
}


void FreeBackFill(struct PatternInfo *ptinf)
{
	ScalosObtainSemaphoreShared(&PatternSema);

	if (ptinf->ptinf_message)
		{
		d1(KPrintF("%s/%s/%ld: msg=%08lx  ReplyPort=%08lx\n", \
			__FILE__, __FUNC__, __LINE__, ptinf->ptinf_message, ptinf->ptinf_message->mn_ReplyPort));
		ptinf->ptinf_message->mn_ReplyPort = NULL;
		ptinf->ptinf_message = NULL;
		}

	PatternFreeInfoBitMap(ptinf);

	ScalosReleaseSemaphore(&PatternSema);
}


BOOL SetBackFill(struct internalScaWindowTask *iwt, struct PatternNode *ptNode,
	struct PatternInfo *ptInfo, ULONG Flags, struct Screen *Scr)
{
	BOOL Success = FALSE;

	d1(KPrintF("\n" "%s/%s/%ld: iwt=%08lx  ptn=%08lx  ptinf=%08lx  Flags=%04lx  scr=%08lx\n", \
		__FILE__, __FUNC__, __LINE__, iwt, ptNode, ptInfo, Flags, Scr));

	SETHOOKFUNC(ptInfo->ptinf_hook, BackFillHookFunc);
	ptInfo->ptinf_hook.h_Data = iwt;

	ptInfo->ptinf_bitmap = NULL;

	if (ptNode)
		{
		d1(KPrintF("%s/%s/%ld: type=%ld\n", __FILE__, __FUNC__, __LINE__, ptNode->ptn_type));

		if (ptNode->ptn_flags & PTNF_Ready)
			{
			d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
			Success = SetBackFill_OkRendered(iwt, ptNode, ptInfo);
			}
		else
			{
			d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

			if (NULL == ptNode->ptn_filename)
				{
				d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
				ptNode->ptn_flags |= PTNF_Ready;
				Success = SetBackFill_OkRendered(iwt, ptNode, ptInfo);
				}
			else
				{
				d1(KPrintF("%s/%s/%ld: ptn_filename=<%s>\n", __FILE__, __FUNC__, __LINE__, ptNode->ptn_filename));

				if (!(Flags & SETBACKFILLF_NOASYNC) && PatternPrefs.patt_AsyncBackFill)
					{
					d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
					Success = SetBackFill_AsyncRead(iwt, ptNode, ptInfo, Scr);
					}
				else
					{
					d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
					Success = ReadBackFill(iwt, ptNode, ptInfo, Scr);
					}
				}
			}
		}

	d1(KPrintF("%s/%s/%ld: Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}


static BOOL SetBackFill_OkRendered(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo)
{
	d1(KPrintF("%s/%s/%ld: ptn=%08lx  type=%ld  ptinf=%08lx\n", __FILE__, __FUNC__, __LINE__, ptNode, ptNode->ptn_type, ptInfo));

	PatternFreeInfoBitMap(ptInfo);

	if ((SCP_RenderType_FitSize == ptNode->ptn_type) && GuiGFXBase)
		{
		d1(KPrintF("%s/%s/%ld: SCP_RenderType_FitSize\n", __FILE__, __FUNC__, __LINE__));

		ptInfo->ptinf_bitmap = CreatePictureBitMap(ptNode->ptn_drawhandle, 
			ptNode->ptn_picture, 
			GGFX_DestWidth, ptInfo->ptinf_width,
			GGFX_DestHeight, ptInfo->ptinf_height,
			TAG_END);

		if (ptInfo->ptinf_bitmap)
			{
			ptInfo->ptinf_flags |= PTINFF_FreeBitMap;
			return TRUE;
			}
		}
	else if (SCP_RenderType_Centered == ptNode->ptn_type)
		{
		d1(KPrintF("%s/%s/%ld: SCP_RenderType_Centered\n", __FILE__, __FUNC__, __LINE__));

		if (PatternCreateCenteredBitMap(iwt, ptNode, ptInfo))
			{
			ptInfo->ptinf_flags |= PTINFF_FreeBitMap;
			return TRUE;
			}
		}
	else if (SCP_RenderType_ScaledMin == ptNode->ptn_type)
		{
		d1(KPrintF("%s/%s/%ld: SCP_RenderType_ScaledMin\n", __FILE__, __FUNC__, __LINE__));

		if (PatternCreateScaledBitMapMin(iwt, ptNode, ptInfo))
			{
			ptInfo->ptinf_flags |= PTINFF_FreeBitMap;
			return TRUE;
			}
		}
	else if (SCP_RenderType_ScaledMax == ptNode->ptn_type)
		{
		d1(KPrintF("%s/%s/%ld: SCP_RenderType_ScaledMax\n", __FILE__, __FUNC__, __LINE__));

		if (PatternCreateScaledBitMapMax(iwt, ptNode, ptInfo))
			{
			ptInfo->ptinf_flags |= PTINFF_FreeBitMap;
			return TRUE;
			}
		}

	// SCP_RenderType_Tiled
	ptInfo->ptinf_width = ptNode->ptn_width;
	ptInfo->ptinf_height = ptNode->ptn_height;
	ptInfo->ptinf_bitmap = ptNode->ptn_bitmap;

	d1(kprintf("%s/%s/%ld: bitmap=%08lx\n", __FILE__, __FUNC__, __LINE__, ptInfo->ptinf_bitmap));

	return (BOOL) (NULL != ptInfo->ptinf_bitmap);
}


static BOOL SetBackFill_AsyncRead(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo, struct Screen *scr)
{
	struct SM_AsyncBackFill *msg = NULL;
	BOOL Success = FALSE;
	struct Process *bfProc;

	d1(KPrintF("%s/%s/%ld: iwt=%08lx <%s>  ptn=%08lx  ptinf=%08lx  scr=%08lx\n", \
		__FILE__, __FUNC__, __LINE__, iwt, iwt->iwt_WinTitle, ptNode, ptInfo, scr));

	if (ScalosAttemptSemaphoreShared(&iwt->iwt_ChildProcessSemaphore))
		{
		do	{
			STATIC_PATCHFUNC(ProcRunnerTask);

			if (iwt->iwt_AsyncLayoutPending)
				break;

			msg = (struct SM_AsyncBackFill *) SCA_AllocMessage(MTYP_AsyncBackFill, sizeof(struct PatternInfo));
			if (NULL == msg)
				break;

			iwt->iwt_AsyncLayoutPending = TRUE;
			d1(kprintf("%s/%s/%ld: iwt=%08lx  AsyncLayoutPending=%lu\n", \
				__FILE__, __FUNC__, __LINE__, iwt, iwt->iwt_AsyncLayoutPending));

			ptInfo->ptinf_message = &msg->ScalosMessage.sm_Message;
			msg->ScalosMessage.sm_Message.mn_ReplyPort = iwt->iwt_WindowTask.wt_IconPort;

			msg->smab_BackfillFunc = (RUNPROCFUNC) AsyncBackFillFunc;
			msg->smab_PatternNode = ptNode;
			msg->smab_PatternInfo = ptInfo;
			msg->smab_Screen = scr;
			msg->smab_WindowTask = &iwt->iwt_WindowTask;

			CopyMem(ptInfo, msg->smab_PatternInfoCopy, sizeof(struct PatternInfo));

			d1(KPrintF("%s/%s/%ld: msg=%08lx  ReplyPort=%08lx  Len=%lu\n", \
				__FILE__, __FUNC__, __LINE__, msg, msg->ScalosMessage.sm_Message.mn_ReplyPort, msg->ScalosMessage.sm_Message.mn_Length));
			d1(KPrintF("%s/%s/%ld: iwt=%08lx\n", __FILE__, __FUNC__, __LINE__, iwt));
			d1(kprintf("%s/%s/%ld: msg=%08lx  ptinf=%08lx  PatternInfoCopy=%08lx\n", \
				__FILE__, __FUNC__, __LINE__, msg, ptInfo, msg->smab_PatternInfoCopy));

			bfProc = CreateNewProcTags(NP_StackSize, CurrentPrefs.pref_DefaultStackSize,
					NP_Priority, PatternPrefs.patt_AsyncProcTaskPri,
					NP_CommandName, (ULONG) "Scalos_Async_Backfill",
					NP_Name, (ULONG) "Scalos_Async_Backfill",
					NP_Entry, (ULONG) PATCH_NEWFUNC(ProcRunnerTask),
					TAG_END);
			if (NULL == bfProc)
				{
				iwt->iwt_AsyncLayoutPending = FALSE;
				break;
				}

			d1(KPrintF("%s/%s/%ld: msg=%08lx  ReplyPort=%08lx  Len=%lu\n", \
				__FILE__, __FUNC__, __LINE__, msg, msg->ScalosMessage.sm_Message.mn_ReplyPort, msg->ScalosMessage.sm_Message.mn_Length));

			PutMsg(&bfProc->pr_MsgPort, &msg->ScalosMessage.sm_Message);
			msg = NULL;
			Success = TRUE;
			} while (0);

		if (msg)
			SCA_FreeMessage(&msg->ScalosMessage);

		ScalosReleaseSemaphore(&iwt->iwt_ChildProcessSemaphore);
		}

	d1(KPrintF("%s/%s/%ld: Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}


static SAVEDS(void) AsyncBackFillFunc(APTR arg, struct SM_AsyncBackFill *msg)
{
	struct internalScaWindowTask *iwt = (struct internalScaWindowTask *) msg->smab_WindowTask;

	d1(KPrintF("%s/%s/%ld: START iwt=%08lx ReplyPort=%08lx\n", \
		__FILE__, __FUNC__, __LINE__, iwt, msg->ScalosMessage.sm_Message.mn_ReplyPort));

	(void) arg;

	ReadBackFill(iwt, msg->smab_PatternNode, (struct PatternInfo *) msg->smab_PatternInfoCopy, msg->smab_Screen);

	d1(KPrintF("%s/%s/%ld: END iwt=%08lx  ReplyPort=%08lx\n",
		__FILE__, __FUNC__, __LINE__, iwt, msg->ScalosMessage.sm_Message.mn_ReplyPort));
}


static BOOL ReadBackFill(struct internalScaWindowTask *iwt, struct PatternNode *ptNode,
	struct PatternInfo *ptInfo, struct Screen *scr)
{
	BOOL Success;

	d1(KPrintF("%s/%s/%ld: START ptn=%08lx  ptinf=%08lx  scr=%08lx\n", __FILE__, __FUNC__, __LINE__, ptNode, ptInfo, scr));

	if (ptNode->ptn_flags & PTNF_Ready)
		{
		d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		return SetBackFill_OkRendered(iwt, ptNode, ptInfo);
		}
	if (NULL == ptNode->ptn_filename)
		{
		d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		ptNode->ptn_flags |= PTNF_Ready;
		return SetBackFill_OkRendered(iwt, ptNode, ptInfo);
		}

	d1(KPrintF("%s/%s/%ld: ptn_filename=<%s>\n", __FILE__, __FUNC__, __LINE__, ptNode->ptn_filename));

	ScalosObtainSemaphoreShared(&PatternSema);
	ScalosObtainSemaphore(ptNode->ptn_semaphore);

	d1(KPrintF("%s/%s/%ld: ptn_bitmap=%08lx\n", __FILE__, __FUNC__, __LINE__, ptNode->ptn_bitmap));

	if (ptNode->ptn_bitmap)
		{
		ptInfo->ptinf_width = ptNode->ptn_width;
		ptInfo->ptinf_height = ptNode->ptn_height;
		ptInfo->ptinf_bitmap = ptNode->ptn_bitmap;

		ptNode->ptn_flags |= PTNF_Ready;
		Success = TRUE;
		}
	else
		{
		if (!(ptNode->ptn_flags & PTNF_NoRemap) && GuiGFXBase && (ptNode->ptn_flags & PTNF_Enhanced))
			{
			//.enhanced
			d1(KPrintF("%s/%s/%ld: ptInfo=%08lx\n", __FILE__, __FUNC__, __LINE__, ptInfo));
			Success = ReadBackFill_Enhanced(iwt, ptNode, ptInfo, scr);
			}
		else
			{
			//.noremap
			d1(KPrintF("%s/%s/%ld: ptInfo=%08lx\n", __FILE__, __FUNC__, __LINE__, ptInfo));
			Success = ReadBackFill_NoGuiGfx(iwt, ptNode, ptInfo, scr);
			}
		}

	ScalosReleaseSemaphore(ptNode->ptn_semaphore);
	ScalosReleaseSemaphore(&PatternSema);

	d1(KPrintF("%s/%s/%ld: END Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}


static BOOL ReadBackFill_NoGuiGfx(struct internalScaWindowTask *iwt, struct PatternNode *ptNode,
	struct PatternInfo *ptInfo, struct Screen *scr)
{
	struct BitMap *destBitMap = NULL;
	struct BitMapHeader *bmhd = NULL;
	ULONG ReallyUseFriendBitMap;
	BOOL Success = FALSE;

	d1(KPrintF("%s/%s/%ld: START ptn=%08lx  ptn_flags=%04lx  ptinf=%08lx  scr=%08lx\n", \
		__FILE__, __FUNC__, __LINE__, ptNode, ptNode->ptn_flags, ptInfo, scr));

	do	{
		d1(KPrintF("%s/%s/%ld: PDTA_Remap=%ld\n", __FILE__, __FUNC__, __LINE__, !(ptNode->ptn_flags & PTNF_NoRemap)));

		if (ptNode->ptn_object)
			DisposeDTObject(ptNode->ptn_object);

		ptNode->ptn_object = NewDTObject(ptNode->ptn_filename,
			PDTA_DestMode, PMODE_V43,
			DTA_SourceType, DTST_FILE,
			DTA_GroupID, GID_PICTURE,
			PDTA_Remap, !(ptNode->ptn_flags & PTNF_NoRemap),
			PDTA_Screen, (ULONG) scr,
			PDTA_FreeSourceBitMap, TRUE,
			OBP_Precision, ptNode->ptn_prec,
			TAG_END);
		if (NULL == ptNode->ptn_object)
			break;

		SetAttrs(ptNode->ptn_object,
			PDTA_UseFriendBitMap, TRUE,
			TAG_END);

		DoMethod(ptNode->ptn_object, DTM_PROCLAYOUT, NULL, TRUE);

		GetDTAttrs(ptNode->ptn_object, 
			PDTA_DestBitMap, (ULONG) &destBitMap,
			PDTA_BitMapHeader, (ULONG) &bmhd,
			TAG_END);
		d1(KPrintF("%s/%s/%ld: destBitMap=%08lx bmhd=%08lx\n", __FILE__, __FUNC__, __LINE__, destBitMap, bmhd));
		if (NULL == destBitMap || NULL == bmhd)
			break;

		ptNode->ptn_width = bmhd->bmh_Width;
		ptNode->ptn_height = bmhd->bmh_Height;
		ptNode->ptn_bitmap = destBitMap;

		if (SCP_RenderType_FitSize == ptNode->ptn_type)
			{
			ULONG ScaledWidth = ptInfo->ptinf_width;
			ULONG ScaledHeight = ptInfo->ptinf_height;

			d1(KPrintF("%s/%s/%ld: ScaledWidth=%ld  ScaledHeight=%ld\n", __FILE__, __FUNC__, __LINE__, ScaledWidth, ScaledHeight));

			if (!PatternCreateScaledBitMapNoGuiGFX(iwt, ptNode, ptInfo, ScaledWidth, ScaledHeight))
				break;
			}
		else if (SCP_RenderType_ScaledMin == ptNode->ptn_type)
			{
			LONG ScaledWidth, ScaledHeight;

			ScaledWidth = (iwt->iwt_InnerHeight * ptNode->ptn_width) / ptNode->ptn_height;

			if (ScaledWidth > iwt->iwt_InnerWidth)
				{
				ScaledWidth = iwt->iwt_InnerWidth;
				ScaledHeight = (iwt->iwt_InnerWidth * ptNode->ptn_height) / ptNode->ptn_width;
				}
			else
				{
				ScaledHeight = iwt->iwt_InnerHeight;
				}

			d1(KPrintF("%s/%s/%ld: ScaledWidth=%ld  ScaledHeight=%ld\n", __FILE__, __FUNC__, __LINE__, ScaledWidth, ScaledHeight));

			if (!PatternCreateScaledBitMapNoGuiGFX(iwt, ptNode, ptInfo, ScaledWidth, ScaledHeight))
				break;
			}
		else if (SCP_RenderType_ScaledMax == ptNode->ptn_type)
			{
			LONG ScaledWidth, ScaledHeight;

			ScaledWidth = (iwt->iwt_InnerHeight * ptNode->ptn_width) / ptNode->ptn_height;

			if (ScaledWidth < iwt->iwt_InnerWidth)
				{
				ScaledWidth = iwt->iwt_InnerWidth;
				ScaledHeight = (iwt->iwt_InnerWidth * ptNode->ptn_height) / ptNode->ptn_width;
				}
			else
				{
				ScaledHeight = iwt->iwt_InnerHeight;
				}

			d1(KPrintF("%s/%s/%ld: ScaledWidth=%ld  ScaledHeight=%ld\n", __FILE__, __FUNC__, __LINE__, ScaledWidth, ScaledHeight));

			if (!PatternCreateScaledBitMapNoGuiGFX(iwt, ptNode, ptInfo, ScaledWidth, ScaledHeight))
				break;
			}

		GetAttr(PDTA_UseFriendBitMap, ptNode->ptn_object, &ReallyUseFriendBitMap);

		d1(KPrintF("%s/%s/%ld: pref_UseFriendBitMap=%ld  ReallyUseFriendBitMap=%ld\n", \
			__FILE__, __FUNC__, __LINE__, PatternPrefs.patt_UseFriendBitMap, ReallyUseFriendBitMap));

		if (!ReallyUseFriendBitMap && PatternPrefs.patt_UseFriendBitMap)
			{
			struct BitMap *newBitMap = AllocBitMap(ptNode->ptn_width, 
				ptNode->ptn_height,
				scr->BitMap.Depth, 
				BMF_DISPLAYABLE | BMB_INTERLEAVED | BMB_MINPLANES,
				scr->RastPort.BitMap);
			d1(KPrintF("%s/%s/%ld: newBitMap=%08lx\n", __FILE__, __FUNC__, __LINE__, newBitMap));
			if (NULL == newBitMap)
				break;

			BltBitMap(ptNode->ptn_bitmap, 0, 0, 
				newBitMap, 0, 0,
				ptNode->ptn_width, ptNode->ptn_height,
				ABC | ABNC,
				~0,
				NULL);

			PatternFreeNodeBitMap(ptNode);

			ptNode->ptn_flags |= PTNF_FreeBitmap;
			ptNode->ptn_bitmap = newBitMap;
			}

		ptInfo->ptinf_width = ptNode->ptn_width;
		ptInfo->ptinf_height = ptNode->ptn_height;
		ptInfo->ptinf_bitmap = ptNode->ptn_bitmap;

		ptNode->ptn_flags |= PTNF_Ready;
		Success = TRUE;
		} while (0);

	d1(KPrintF("%s/%s/%ld: Success=%ld  bitmap=%08lx\n", __FILE__, __FUNC__, __LINE__, Success, ptInfo->ptinf_bitmap));

	if (!Success)
		{
		if (ptNode->ptn_object)
			{
			DisposeDTObject(ptNode->ptn_object);
			ptNode->ptn_object = NULL;
			}
		}

	d1(KPrintF("%s/%s/%ld: END Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}


static BOOL ReadBackFill_Enhanced(struct internalScaWindowTask *iwt, struct PatternNode *ptNode,
	struct PatternInfo *ptInfo, struct Screen *scr)
{
	BOOL Success = FALSE;

	d1(KPrintF("%s/%s/%ld: START ptn=%08lx  ptinf=%08lx  scr=%08lx\n", __FILE__, __FUNC__, __LINE__, ptNode, ptInfo, scr));

	do	{
		ULONG picWidth, picHeight;

		if (ptNode->ptn_drawhandle)
			{
			ReleaseDrawHandle(ptNode->ptn_drawhandle);
			ptNode->ptn_drawhandle = NULL;
			}
		if (ptNode->ptn_colorhandle)
			{
			RemColorHandle(ptNode->ptn_colorhandle);
			ptNode->ptn_colorhandle = NULL;
			}
		if (ptNode->ptn_picture)
			{
			DeletePicture(ptNode->ptn_picture);
			ptNode->ptn_picture = NULL;
			}

		ptNode->ptn_picture = LoadPictureA(ptNode->ptn_filename, NULL);
		d1(KPrintF("%s/%s/%ld: ptn_filename=<%s>  ptn_picture=%08lx\n", \
			__FILE__, __FUNC__, __LINE__, ptNode->ptn_filename, ptNode->ptn_picture));
		if (NULL == ptNode->ptn_picture)
			{
			// Create empty picture
			ptNode->ptn_picture = MakePictureA(NULL,
				1, //ptInfo->ptinf_width,
				1, //ptInfo->ptinf_height,
				NULL);
			}
		d1(KPrintF("%s/%s/%ld: ptn_filename=<%s>  ptn_picture=%08lx\n", \
			__FILE__, __FUNC__, __LINE__, ptNode->ptn_filename, ptNode->ptn_picture));
		if (NULL == ptNode->ptn_picture)
			break;

		ptNode->ptn_colorhandle = AddPictureA(PenShareMap, ptNode->ptn_picture, NULL);
		d1(KPrintF("%s/%s/%ld: ptn_colorhandle=%08lx\n", __FILE__, __FUNC__, __LINE__, ptNode->ptn_colorhandle));
		if (NULL == ptNode->ptn_colorhandle)
			break;

		ptNode->ptn_drawhandle = ObtainDrawHandle(PenShareMap, &scr->RastPort, scr->ViewPort.ColorMap,
			GGFX_Weight, ptNode->ptn_weight,
			GGFX_AutoDither, 0 != (ptNode->ptn_flags & PTNF_AutoDither),
			GGFX_DitherAmount, ptNode->ptn_ditheramount,
			GGFX_DitherMode, ptNode->ptn_dithermode,
			OBP_Precision, ptNode->ptn_prec,
			TAG_END);
		d1(KPrintF("%s/%s/%ld: ptn_drawhandle=%08lx\n", __FILE__, __FUNC__, __LINE__, ptNode->ptn_drawhandle));
		if (NULL == ptNode->ptn_drawhandle)
			break;

		if (2 != GetPictureAttrs(ptNode->ptn_picture,
			PICATTR_Width, (ULONG) &picWidth,
			PICATTR_Height, (ULONG) &picHeight,
			TAG_END))
			break;

		ptNode->ptn_width = picWidth;
		ptNode->ptn_height = picHeight;

		d1(KPrintF("%s/%s/%ld: ptn_type=%ld\n", __FILE__, __FUNC__, __LINE__, ptNode->ptn_type));

		PatternFreeInfoBitMap(ptInfo);

		switch (ptNode->ptn_type)
			{
		case SCP_RenderType_FitSize:
			ptInfo->ptinf_bitmap = CreatePictureBitMap(ptNode->ptn_drawhandle, ptNode->ptn_picture,
				GGFX_DestWidth, ptInfo->ptinf_width,
				GGFX_DestHeight, ptInfo->ptinf_height,
				TAG_END);
			d1(KPrintF("%s/%s/%ld: bitmap=%08lx\n", __FILE__, __FUNC__, __LINE__, ptInfo->ptinf_bitmap));
			if (NULL == ptInfo->ptinf_bitmap)
				break;

			ptInfo->ptinf_flags |= PTINFF_FreeBitMap;
			break;
		
		case SCP_RenderType_Tiled:
			ptNode->ptn_bitmap = CreatePictureBitMap(ptNode->ptn_drawhandle, ptNode->ptn_picture,
				TAG_END);
			d1(KPrintF("%s/%s/%ld: ptNode=%08lx  bitmap=%08lx\n", __FILE__, __FUNC__, __LINE__, ptNode, ptInfo->ptinf_bitmap));
			if (NULL == ptNode->ptn_bitmap)
				break;

			DeletePicture(ptNode->ptn_picture);
			ptNode->ptn_picture = NULL;

			ptInfo->ptinf_width = ptNode->ptn_width;
			ptInfo->ptinf_height = ptNode->ptn_height;
			ptInfo->ptinf_bitmap = ptNode->ptn_bitmap;

			ptNode->ptn_flags |= PTNF_FreeBitmap;
			break;

		case SCP_RenderType_Centered:
			ptNode->ptn_bitmap = CreatePictureBitMap(ptNode->ptn_drawhandle, ptNode->ptn_picture,
				TAG_END);
			d1(KPrintF("%s/%s/%ld: ptNode=%08lx  bitmap=%08lx\n", __FILE__, __FUNC__, __LINE__, ptNode, ptInfo->ptinf_bitmap));
			if (NULL == ptNode->ptn_bitmap)
				break;

			DeletePicture(ptNode->ptn_picture);
			ptNode->ptn_picture = NULL;

			if (!PatternCreateCenteredBitMap(iwt, ptNode, ptInfo))
				{
				ptInfo->ptinf_width = ptNode->ptn_width;
				ptInfo->ptinf_height = ptNode->ptn_height;
				ptInfo->ptinf_bitmap = ptNode->ptn_bitmap;

				ptNode->ptn_flags &= ~PTNF_FreeBitmap;
				}
			break;

		case SCP_RenderType_ScaledMin:
			if (!PatternCreateScaledBitMapMin(iwt, ptNode, ptInfo))
				{
				ptInfo->ptinf_width = ptNode->ptn_width;
				ptInfo->ptinf_height = ptNode->ptn_height;
				ptInfo->ptinf_bitmap = ptNode->ptn_bitmap;

				ptNode->ptn_flags &= ~PTNF_FreeBitmap;
				}
			break;

		case SCP_RenderType_ScaledMax:
			if (!PatternCreateScaledBitMapMax(iwt, ptNode, ptInfo))
				{
				ptInfo->ptinf_width = ptNode->ptn_width;
				ptInfo->ptinf_height = ptNode->ptn_height;
				ptInfo->ptinf_bitmap = ptNode->ptn_bitmap;

				ptNode->ptn_flags &= ~PTNF_FreeBitmap;
				}
			break;

		default:
			break;
			}

		ptNode->ptn_flags |= PTNF_Ready;
		Success = TRUE;
		} while (0);

	if (!Success)
		{
		d1(KPrintF("%s/%s/%ld: bitmap=%08lx\n", __FILE__, __FUNC__, __LINE__, ptInfo->ptinf_bitmap));

		if (ptNode->ptn_drawhandle)
			{
			ReleaseDrawHandle(ptNode->ptn_drawhandle);
			ptNode->ptn_drawhandle = NULL;
			}
		if (ptNode->ptn_colorhandle)
			{
			RemColorHandle(ptNode->ptn_colorhandle);
			ptNode->ptn_colorhandle = NULL;
			}
		if (ptNode->ptn_picture)
			{
			DeletePicture(ptNode->ptn_picture);
			ptNode->ptn_picture = NULL;
			}
		}

	d1(KPrintF("%s/%s/%ld: ptNode=%08lx  ptInfo=%08lx  bitmap=%08lx\n",
		__FILE__, __FUNC__, __LINE__, ptNode, ptInfo, ptInfo->ptinf_bitmap));
	d1(KPrintF("%s/%s/%ld: END Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}


// object == (struct RastPort *) result->RastPort
// message == [ (Layer *) layer, (struct Rectangle) bounds,
//              (LONG) offsetx, (LONG) offsety ]

static SAVEDS(ULONG) BackFillHookFunc(struct Hook *bfHook, struct RastPort *rp, struct BackFillMsg *msg)
{
	struct RastPort rpCopy;
	struct internalScaWindowTask *iwt = (struct internalScaWindowTask *) bfHook->h_Data;
	struct PatternInfo *ptInfo = &iwt->iwt_WindowTask.wt_PatternInfo;

	d1(KPrintF("\nSTART " "%s/%s/%ld: ptinfo=bfHook=%08lx  rp=%08lx  msg=%08lx\n", \
		__FILE__, __FUNC__, __LINE__, bfHook, rp, msg));

	d1(KPrintF("\n" "%s/%s/%ld: RastPort=%08lx\n", __FILE__, __FUNC__, __LINE__, rp));
	d1(KPrintF("%s/%s/%ld: Rect=%ld %ld %ld %ld\n", __FILE__, __FUNC__, __LINE__, \
		msg->bfm_Rect.MinX, msg->bfm_Rect.MinY, msg->bfm_Rect.MaxX, msg->bfm_Rect.MaxY));
	d1(KPrintF("%s/%s/%ld: Layer=%08lx  OffsetX=%ld  OffsetY=%ld\n", __FILE__, __FUNC__, __LINE__, \
		msg->bfm_Layer, msg->bfm_OffsetX, msg->bfm_OffsetY));

	d1(KPrintF("\n" "%s/%s/%ld: ptinf_bitmap=%08lx\n", __FILE__, __FUNC__, __LINE__, ptInfo->ptinf_bitmap));
	if (ptInfo->ptinf_bitmap)
		d1(KPrintF("\n" "%s/%s/%ld: ptinf_bitmap  w=%ld  h=%ld\n", __FILE__, __FUNC__, __LINE__, \
			GetBitMapAttr(ptInfo->ptinf_bitmap, BMA_WIDTH), GetBitMapAttr(ptInfo->ptinf_bitmap, BMA_HEIGHT)));

	d1(KPrintF("\n" "%s/%s/%ld: rp->bitmap=%08lx  w=%ld  h=%ld\n", __FILE__, __FUNC__, __LINE__, \
		rp->BitMap, GetBitMapAttr(rp->BitMap, BMA_WIDTH), GetBitMapAttr(rp->BitMap, BMA_HEIGHT)));

	rpCopy = *rp;
	rpCopy.Layer = NULL;

	WindowBackFill(&rpCopy, msg, ptInfo->ptinf_bitmap,
		ptInfo->ptinf_width, ptInfo->ptinf_height,
		iwt->iwt_WinDrawInfo->dri_Pens[BACKGROUNDPEN],
		iwt->iwt_WindowTask.wt_XOffset, iwt->iwt_WindowTask.wt_YOffset,
		NULL);

	d1(KPrintF("\n " "%s/%s/%ld: finished\n", __FILE__, __FUNC__, __LINE__));

	return 0;
}


static void PatternFreeNodeBitMap(struct PatternNode *ptNode)
{
	if (ptNode->ptn_bitmap && (ptNode->ptn_flags & PTNF_FreeBitmap))
		{
		WaitBlit();
		FreeBitMap(ptNode->ptn_bitmap);
		ptNode->ptn_bitmap = NULL;
		ptNode->ptn_flags &= ~PTNF_FreeBitmap;
		}
}


static void PatternFreeInfoBitMap(struct PatternInfo *ptInfo)
{
	if (ptInfo->ptinf_bitmap && (ptInfo->ptinf_flags & PTINFF_FreeBitMap))
		{
		WaitBlit();
		FreeBitMap(ptInfo->ptinf_bitmap);
		ptInfo->ptinf_bitmap = NULL;
		ptInfo->ptinf_flags &= ~PTINFF_FreeBitMap;
		}
	if (NO_PEN != ptInfo->ptinf_BgPen)
		{
		ReleasePen(iInfos.xii_iinfos.ii_Screen->ViewPort.ColorMap, ptInfo->ptinf_BgPen);
		ptInfo->ptinf_BgPen = NO_PEN;
		}
}


static BOOL PatternCreateCenteredBitMap(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo)
{
	ptInfo->ptinf_bitmap = CreatePictureBitMap(ptNode->ptn_drawhandle, NULL,
		GGFX_DestWidth, ptInfo->ptinf_width,
		GGFX_DestHeight, ptInfo->ptinf_height,
		TAG_END);

	d1(KPrintF("%s/%s/%ld: ptinf_bitmap=%08lx\n", __FILE__, __FUNC__, __LINE__, ptInfo->ptinf_bitmap));
	if (ptInfo->ptinf_bitmap)
		{
		LONG DestX, DestY;
		LONG Width, Height;
		struct RastPort rp;

		d1(KPrintF("%s/%s/%ld: ptn_width=%ld  ptn_height=%ld\n", __FILE__, __FUNC__, __LINE__, ptNode->ptn_width, ptNode->ptn_height));
		d1(KPrintF("%s/%s/%ld: ptinf_width=%ld  ptinf_height=%ld\n", __FILE__, __FUNC__, __LINE__, ptInfo->ptinf_width, ptInfo->ptinf_height));
		d1(KPrintF("%s/%s/%ld: R=%ld G=%ld B=%ld\n", __FILE__, __FUNC__, __LINE__, ptNode->ptn_BgColor1[0], ptNode->ptn_BgColor1[1], ptNode->ptn_BgColor1[2]));

		// setup RastPort for drawing into ptInfo->ptinf_bitmap
		InitRastPort(&rp);
		rp.BitMap = ptInfo->ptinf_bitmap;

		switch (ptNode->ptn_BgMode)
			{
		case SCP_BgType_SingleColor:
			PatternBackgroundSingleColor(ptNode, ptInfo, &rp);
			break;
		case SCP_BgType_HorizontalGradient:
			PatternBackgroundHorizontalGradient(iwt, ptNode, ptInfo, &rp);
			break;
		case SCP_BGType_VerticalGradient:
			PatternBackgroundVerticalGradient(iwt, ptNode, ptInfo, &rp);
			break;
		case SCP_BgType_Picture:
		default:
			break;
			}

		if (ptNode->ptn_width > iwt->iwt_InnerWidth)
			{
			Width = iwt->iwt_InnerWidth;
			DestX = iwt->iwt_InnerLeft;
			}
		else
			{
			Width = ptNode->ptn_width;
			DestX = iwt->iwt_InnerLeft + (iwt->iwt_InnerWidth - ptNode->ptn_width) / 2;
			}
		if (ptNode->ptn_height > iwt->iwt_InnerHeight)
			{
			Height = iwt->iwt_InnerHeight;
			DestY = iwt->iwt_InnerTop;
			}
		else
			{
			Height = ptNode->ptn_height;
			DestY = iwt->iwt_InnerTop + (iwt->iwt_InnerHeight - ptNode->ptn_height) / 2;
			}

		d1(KPrintF("%s/%s/%ld: Width=%ld  Height=%ld\n", __FILE__, __FUNC__, __LINE__, Width, Height));
		d1(KPrintF("%s/%s/%ld: DestX=%ld  DestY=%ld\n", __FILE__, __FUNC__, __LINE__, DestX, DestY));

		// blit source image centered into ptinf_bitmap
		BltBitMap(ptNode->ptn_bitmap,
			0, 0,
			ptInfo->ptinf_bitmap,
			DestX, DestY,
			Width, Height,
			ABC | ABNC, ~0, NULL);

		ptInfo->ptinf_flags |= PTINFF_FreeBitMap;
		}

	return (BOOL) (NULL != ptInfo->ptinf_bitmap);
}


static BOOL PatternCreateScaledBitMapMin(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo)
{
	LONG ScaledWidth, ScaledHeight;

	ScaledWidth = (iwt->iwt_InnerHeight * ptNode->ptn_width) / ptNode->ptn_height;

	if (ScaledWidth > iwt->iwt_InnerWidth)
		{
		ScaledWidth = iwt->iwt_InnerWidth;
		ScaledHeight = (iwt->iwt_InnerWidth * ptNode->ptn_height) / ptNode->ptn_width;
		}
	else
		{
		ScaledHeight = iwt->iwt_InnerHeight;
		}

	return PatternCreateScaledBitMapGUIGfx(iwt, ptNode, ptInfo, ScaledWidth, ScaledHeight);
}


static BOOL PatternCreateScaledBitMapMax(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo)
{
	LONG ScaledWidth, ScaledHeight;

	ScaledWidth = (iwt->iwt_InnerHeight * ptNode->ptn_width) / ptNode->ptn_height;

	if (ScaledWidth < iwt->iwt_InnerWidth)
		{
		ScaledWidth = iwt->iwt_InnerWidth;
		ScaledHeight = (iwt->iwt_InnerWidth * ptNode->ptn_height) / ptNode->ptn_width;
		}
	else
		{
		ScaledHeight = iwt->iwt_InnerHeight;
		}

	return PatternCreateScaledBitMapGUIGfx(iwt, ptNode, ptInfo, ScaledWidth, ScaledHeight);
}


static BOOL PatternCreateScaledBitMapGUIGfx(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo,
	ULONG ScaledWidth, ULONG ScaledHeight)
{
	struct BitMap *TempBitMap;

	do	{
		LONG DestX, DestY;
		LONG SrcX, SrcY;
		struct RastPort rp;
		ULONG BlitWidth, BlitHeight;

		TempBitMap = CreatePictureBitMap(ptNode->ptn_drawhandle, ptNode->ptn_picture,
			GGFX_DestWidth, ScaledWidth,
			GGFX_DestHeight, ScaledHeight,
			TAG_END);
		d1(KPrintF("%s/%s/%ld: TempBitMap=%08lx\n", __FILE__, __FUNC__, __LINE__, TempBitMap));
		if (NULL == TempBitMap)
			break;

		ptInfo->ptinf_bitmap = CreatePictureBitMap(ptNode->ptn_drawhandle, NULL,
			GGFX_DestWidth, ptInfo->ptinf_width,
			GGFX_DestHeight, ptInfo->ptinf_height,
			TAG_END);

		d1(KPrintF("%s/%s/%ld: ptinf_bitmap=%08lx\n", __FILE__, __FUNC__, __LINE__, ptInfo->ptinf_bitmap));
		if (NULL == ptInfo->ptinf_bitmap)
			break;

		d1(KPrintF("%s/%s/%ld: ptn_width=%ld  ptn_height=%ld\n", __FILE__, __FUNC__, __LINE__, ptNode->ptn_width, ptNode->ptn_height));
		d1(KPrintF("%s/%s/%ld: ptinf_width=%ld  ptinf_height=%ld\n", __FILE__, __FUNC__, __LINE__, ptInfo->ptinf_width, ptInfo->ptinf_height));
		d1(KPrintF("%s/%s/%ld: R=%ld G=%ld B=%ld\n", __FILE__, __FUNC__, __LINE__, ptNode->ptn_BgColor1[0], ptNode->ptn_BgColor1[1], ptNode->ptn_BgColor1[2]));

		// setup RastPort for drawing into ptInfo->ptinf_bitmap
		InitRastPort(&rp);
		rp.BitMap = ptInfo->ptinf_bitmap;

		switch (ptNode->ptn_BgMode)
			{
		case SCP_BgType_SingleColor:
			PatternBackgroundSingleColor(ptNode, ptInfo, &rp);
			break;
		case SCP_BgType_HorizontalGradient:
			PatternBackgroundHorizontalGradient(iwt, ptNode, ptInfo, &rp);
			break;
		case SCP_BGType_VerticalGradient:
			PatternBackgroundVerticalGradient(iwt, ptNode, ptInfo, &rp);
			break;
		case SCP_BgType_Picture:
		default:
			break;
			}

		DestX = iwt->iwt_InnerLeft;
		DestY = iwt->iwt_InnerTop;

		if (ScaledWidth < ptInfo->ptinf_width)
			{
			// Scaled image is smaller than window
			if (iwt->iwt_InnerWidth >= ScaledWidth)
				DestX += (iwt->iwt_InnerWidth - ScaledWidth) / 2;
			SrcX = 0;
			BlitWidth = ScaledWidth;
			}
		else
			{
			// Scaled image is wider than window
			SrcX =  (ScaledWidth - iwt->iwt_InnerWidth) / 2;
			BlitWidth = ptInfo->ptinf_width - DestX;
			}

		if (ScaledHeight < ptInfo->ptinf_height)
			{
			// Scaled image is smaller than window
			if (iwt->iwt_InnerHeight >= ScaledHeight)
				DestY += (iwt->iwt_InnerHeight - ScaledHeight) / 2;
			SrcY = 0;
			BlitHeight = ScaledHeight;
			}
		else
			{
			// Scaled image is taller than window
			SrcY = (ScaledHeight - iwt->iwt_InnerHeight) / 2;
			BlitHeight = ptInfo->ptinf_height;
			}

		if ((BlitWidth + DestX) > ptInfo->ptinf_width)
			BlitWidth =  ptInfo->ptinf_width - DestX;
		if ((BlitHeight + DestY) > ptInfo->ptinf_height)
			BlitHeight = ptInfo->ptinf_height - DestY;

		d1(KPrintF("%s/%s/%ld: iwt_InnerWidth=%ld  iwt_InnerHeight=%ld\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_InnerWidth, iwt->iwt_InnerHeight));
		d1(KPrintF("%s/%s/%ld: ptinf_width=%ld  ptinf_height=%ld\n", __FILE__, __FUNC__, __LINE__, ptInfo->ptinf_width, ptInfo->ptinf_height));
		d1(KPrintF("%s/%s/%ld: ScaledWidth=%ld  ScaledHeight=%ld\n", __FILE__, __FUNC__, __LINE__, ScaledWidth, ScaledHeight));
		d1(KPrintF("%s/%s/%ld: BlitWidth=%ld  BlitHeight=%ld\n", __FILE__, __FUNC__, __LINE__, BlitWidth, BlitHeight));
		d1(KPrintF("%s/%s/%ld: SrcX=%ld  SrcY=%ld\n", __FILE__, __FUNC__, __LINE__, SrcX, SrcY));
		d1(KPrintF("%s/%s/%ld: DestX=%ld  DestY=%ld\n", __FILE__, __FUNC__, __LINE__, DestX, DestY));

		// blit source image centered into ptinf_bitmap
		BltBitMap(TempBitMap,
			SrcX, SrcY,
			ptInfo->ptinf_bitmap,
			DestX, DestY,
			BlitWidth, BlitHeight,
			ABC | ABNC, ~0, NULL);

		ptInfo->ptinf_flags |= PTINFF_FreeBitMap;
		} while (0);

	if (TempBitMap)
		{
		WaitBlit();
		FreeBitMap(TempBitMap);
		}

	return (BOOL) (NULL != ptInfo->ptinf_bitmap);
}


static BOOL PatternCreateScaledBitMapNoGuiGFX(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo,
	ULONG ScaledWidth, ULONG ScaledHeight)
{
	do	{
		struct ScaleBitMapArg sbma;
		struct BitMap *ScaledBitMap;
		ULONG NumColors;
		ULONG *ColorTable;
		struct RastPort rp;

		if (2 != GetDTAttrs(ptNode->ptn_object,
			PDTA_NumColors, (ULONG) &NumColors,
			PDTA_CRegs, (ULONG) &ColorTable,
			TAG_END))
			break;

		d1(KPrintF("%s/%s/%ld: NumColors=%ld  ColorTable=%08lx\n", __FILE__, __FUNC__, __LINE__, NumColors, ColorTable));

		sbma.sbma_SourceBM = ptNode->ptn_bitmap;
		sbma.sbma_SourceWidth = ptNode->ptn_width;
		sbma.sbma_SourceHeight = ptNode->ptn_height;
		sbma.sbma_DestWidth = &ScaledWidth;
		sbma.sbma_DestHeight = &ScaledHeight;
		sbma.sbma_NumberOfColors = NumColors;
		sbma.sbma_ColorTable = ColorTable;
		sbma.sbma_Flags = SCALEFLAGF_BILINEAR | SCALEFLAGF_AVERAGE | SCALEFLAGF_DOUBLESIZE;
		sbma.sbma_ScreenBM = iInfos.xii_iinfos.ii_Screen->RastPort.BitMap;

		ScaledBitMap = ScalosGfxScaleBitMap(&sbma, NULL);
		d1(KPrintF("%s/%s/%ld: ScaledBitMap=%08lx\n", __FILE__, __FUNC__, __LINE__, ScaledBitMap));
		if (NULL == ScaledBitMap)
			break;

		d1(KPrintF("%s/%s/%ld: ScaledWidth=%ld  ScaledHeight=%ld\n", __FILE__, __FUNC__, __LINE__, ScaledWidth, ScaledHeight));

		PatternFreeNodeBitMap(ptNode);

		ptNode->ptn_flags |= PTNF_FreeBitmap;
		ptNode->ptn_bitmap = ScaledBitMap;

		ptNode->ptn_width = ScaledWidth;
		ptNode->ptn_height = ScaledHeight;

		d1(KPrintF("%s/%s/%ld: ptn_width=%ld  ptn_height=%ld\n", __FILE__, __FUNC__, __LINE__, ptNode->ptn_width, ptNode->ptn_height));
		d1(KPrintF("%s/%s/%ld: ptinf_width=%ld  ptinf_height=%ld\n", __FILE__, __FUNC__, __LINE__, ptInfo->ptinf_width, ptInfo->ptinf_height));
		d1(KPrintF("%s/%s/%ld: R=%ld G=%ld B=%ld\n", __FILE__, __FUNC__, __LINE__, ptNode->ptn_BgColor1[0], ptNode->ptn_BgColor1[1], ptNode->ptn_BgColor1[2]));

		// setup RastPort for drawing into ptInfo->ptinf_bitmap
		InitRastPort(&rp);
		rp.BitMap = ptNode->ptn_bitmap;

		switch (ptNode->ptn_BgMode)
			{
		case SCP_BgType_SingleColor:
			PatternBackgroundSingleColor(ptNode, ptInfo, &rp);
			break;
		case SCP_BgType_HorizontalGradient:
			PatternBackgroundHorizontalGradient(iwt, ptNode, ptInfo, &rp);
			break;
		case SCP_BGType_VerticalGradient:
			PatternBackgroundVerticalGradient(iwt, ptNode, ptInfo, &rp);
			break;
		case SCP_BgType_Picture:
		default:
			break;
			}
		} while (0);

	return (BOOL) (NULL != ptNode->ptn_bitmap);
}


static BOOL PatternBackgroundHorizontalGradient(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo, struct RastPort *rp)
{
	struct ARGB Left, Right;
	struct ARGB *Source;
	LONG DeltaRed, DeltaGreen, DeltaBlue;
	LONG StepRed, StepGreen, StepBlue;
	LONG RemRed, RemGreen, RemBlue;
	BOOL Success = FALSE;

	Left.Red   = ptNode->ptn_BgColor1[0];
	Left.Green = ptNode->ptn_BgColor1[1];
	Left.Blue  = ptNode->ptn_BgColor1[2];
	Left.Alpha = (UBYTE) ~0;

	Right.Red   = ptNode->ptn_BgColor2[0];
	Right.Green = ptNode->ptn_BgColor2[1];
	Right.Blue  = ptNode->ptn_BgColor2[2];

	d1(KPrintF("%s/%s/%ld: Left   Red=%3ld Green=%2ld Blue=%3ld\n", __FILE__, __FUNC__, __LINE__, Left.Red, Left.Green, Left.Blue));
	d1(KPrintF("%s/%s/%ld: Right  Red=%3ld Green=%2ld Blue=%3ld\n", __FILE__, __FUNC__, __LINE__, Right.Red, Right.Green, Right.Blue));

	DeltaRed   = Right.Red - Left.Red;
	DeltaGreen = Right.Green - Left.Green;
	DeltaBlue  = Right.Blue - Left.Blue;

	d1(KPrintF("%s/%s/%ld: DeltaRed=%ld  DeltaGreen=%ld  DeltaBlue=%ld\n", __FILE__, __FUNC__, __LINE__, DeltaRed, DeltaGreen, DeltaBlue));

	StepRed = DeltaRed / ptInfo->ptinf_width;
	RemRed  = DeltaRed % ptInfo->ptinf_width;

	d1(KPrintF("%s/%s/%ld: StepRed=%ld  RemRed=%ld\n", __FILE__, __FUNC__, __LINE__, StepRed, RemRed));

	StepGreen = DeltaGreen / ptInfo->ptinf_width;
	RemGreen  = DeltaGreen % ptInfo->ptinf_width;

	d1(KPrintF("%s/%s/%ld: StepGreen=%ld  RemGreen=%ld\n", __FILE__, __FUNC__, __LINE__, StepGreen, RemGreen));

	StepBlue = DeltaBlue / ptInfo->ptinf_width;
	RemBlue  = DeltaBlue % ptInfo->ptinf_width;

	d1(KPrintF("%s/%s/%ld: StepBlue=%ld  RemBlue=%ld\n", __FILE__, __FUNC__, __LINE__, StepBlue, RemBlue));

	Source = ScalosGfxCreateARGB(ptInfo->ptinf_width, 1, NULL);
	if (Source)
		{
		LONG x, y;
		struct ARGB *pSrc = Source;
		LONG AccumulatedDiffRed = 0;
		LONG AccumulatedDiffGreen = 0;
		LONG AccumulatedDiffBlue = 0;

		// fill Source with RGB left-to-right gradient
		for (x = 0; x < ptInfo->ptinf_width; x++)
			{
			*pSrc = Left;

			Left.Red += StepRed;
			AccumulatedDiffRed += RemRed;
			if (AccumulatedDiffRed >= ptInfo->ptinf_width)
				{
				Left.Red++;
				AccumulatedDiffRed -= ptInfo->ptinf_width;
				}
			else if (AccumulatedDiffRed <= -ptInfo->ptinf_width)
				{
				Left.Red--;
				AccumulatedDiffRed += ptInfo->ptinf_width;
				}

			Left.Green += StepGreen;
			AccumulatedDiffGreen += RemGreen;
			if (AccumulatedDiffGreen >= ptInfo->ptinf_width)
				{
				Left.Green++;
				AccumulatedDiffGreen -= ptInfo->ptinf_width;
				}
			else if (AccumulatedDiffGreen <= -ptInfo->ptinf_width)
				{
				Left.Green--;
				AccumulatedDiffGreen += ptInfo->ptinf_width;
				}

			Left.Blue += StepBlue;
			AccumulatedDiffBlue += RemBlue;
			if (AccumulatedDiffBlue >= ptInfo->ptinf_width)
				{
				Left.Blue++;
				AccumulatedDiffBlue -= ptInfo->ptinf_width;
				}
			else if (AccumulatedDiffBlue <= -ptInfo->ptinf_width)
				{
				Left.Blue--;
				AccumulatedDiffBlue += ptInfo->ptinf_width;
				}

			pSrc++;
			}
		d1(KPrintF("%s/%s/%ld: Left   Red=%3ld Green=%2ld Blue=%3ld\n", __FILE__, __FUNC__, __LINE__, Left.Red, Left.Green, Left.Blue));

		// blit pre-generated gradient line by line into dest bitmap
		for (y = 0; y < ptInfo->ptinf_height; y++)
			{
			WritePixelArray(Source,
				0, 0,
				sizeof(struct ARGB) * ptInfo->ptinf_width,
				rp,
				0, y,
				ptInfo->ptinf_width, 1,
				RECTFMT_ARGB);
			}

		ScalosGfxFreeARGB(&Source);
		Success = TRUE;
		}

	return Success;
}


static BOOL PatternBackgroundVerticalGradient(struct internalScaWindowTask *iwt,
	struct PatternNode *ptNode, struct PatternInfo *ptInfo, struct RastPort *rp)
{
	struct ARGB Top, Bottom;
	struct ARGB *Source;
	LONG DeltaRed, DeltaGreen, DeltaBlue;
	LONG StepRed, StepGreen, StepBlue;
	LONG RemRed, RemGreen, RemBlue;
	BOOL Success = FALSE;

	Top.Red   = ptNode->ptn_BgColor1[0];
	Top.Green = ptNode->ptn_BgColor1[1];
	Top.Blue  = ptNode->ptn_BgColor1[2];
	Top.Alpha = (UBYTE) ~0;

	Bottom.Red   = ptNode->ptn_BgColor2[0];
	Bottom.Green = ptNode->ptn_BgColor2[1];
	Bottom.Blue  = ptNode->ptn_BgColor2[2];

	d1(KPrintF("%s/%s/%ld: Top   Red=%3ld Green=%2ld Blue=%3ld\n", __FILE__, __FUNC__, __LINE__, Top.Red, Top.Green, Top.Blue));
	d1(KPrintF("%s/%s/%ld: Bottom  Red=%3ld Green=%2ld Blue=%3ld\n", __FILE__, __FUNC__, __LINE__, Bottom.Red, Bottom.Green, Bottom.Blue));

	DeltaRed   = Bottom.Red - Top.Red;
	DeltaGreen = Bottom.Green - Top.Green;
	DeltaBlue  = Bottom.Blue - Top.Blue;

	d1(KPrintF("%s/%s/%ld: DeltaRed=%ld  DeltaGreen=%ld  DeltaBlue=%ld\n", __FILE__, __FUNC__, __LINE__, DeltaRed, DeltaGreen, DeltaBlue));

	StepRed = DeltaRed / ptInfo->ptinf_height;
	RemRed  = DeltaRed % ptInfo->ptinf_height;

	d1(KPrintF("%s/%s/%ld: StepRed=%ld  RemRed=%ld\n", __FILE__, __FUNC__, __LINE__, StepRed, RemRed));

	StepGreen = DeltaGreen / ptInfo->ptinf_height;
	RemGreen  = DeltaGreen % ptInfo->ptinf_height;

	d1(KPrintF("%s/%s/%ld: StepGreen=%ld  RemGreen=%ld\n", __FILE__, __FUNC__, __LINE__, StepGreen, RemGreen));

	StepBlue = DeltaBlue / ptInfo->ptinf_height;
	RemBlue  = DeltaBlue % ptInfo->ptinf_height;

	d1(KPrintF("%s/%s/%ld: StepBlue=%ld  RemBlue=%ld\n", __FILE__, __FUNC__, __LINE__, StepBlue, RemBlue));

	Source = ScalosGfxCreateARGB(1, ptInfo->ptinf_height, NULL);
	if (Source)
		{
		LONG x, y;
		struct ARGB *pSrc = Source;
		LONG AccumulatedDiffRed = 0;
		LONG AccumulatedDiffGreen = 0;
		LONG AccumulatedDiffBlue = 0;

		// fill Source with RGB Top-to-Bottom gradient
		for (y = 0; y < ptInfo->ptinf_height; y++)
			{
			*pSrc = Top;

			Top.Red += StepRed;
			AccumulatedDiffRed += RemRed;
			if (AccumulatedDiffRed >= ptInfo->ptinf_height)
				{
				Top.Red++;
				AccumulatedDiffRed -= ptInfo->ptinf_height;
				}
			else if (AccumulatedDiffRed <= -ptInfo->ptinf_height)
				{
				Top.Red--;
				AccumulatedDiffRed += ptInfo->ptinf_height;
				}

			Top.Green += StepGreen;
			AccumulatedDiffGreen += RemGreen;
			if (AccumulatedDiffGreen >= ptInfo->ptinf_height)
				{
				Top.Green++;
				AccumulatedDiffGreen -= ptInfo->ptinf_height;
				}
			else if (AccumulatedDiffGreen <= -ptInfo->ptinf_height)
				{
				Top.Green--;
				AccumulatedDiffGreen += ptInfo->ptinf_height;
				}

			Top.Blue += StepBlue;
			AccumulatedDiffBlue += RemBlue;
			if (AccumulatedDiffBlue >= ptInfo->ptinf_height)
				{
				Top.Blue++;
				AccumulatedDiffBlue -= ptInfo->ptinf_height;
				}
			else if (AccumulatedDiffBlue <= -ptInfo->ptinf_height)
				{
				Top.Blue--;
				AccumulatedDiffBlue += ptInfo->ptinf_height;
				}

			pSrc++;
			}
		d1(KPrintF("%s/%s/%ld: Top   Red=%3ld Green=%2ld Blue=%3ld\n", __FILE__, __FUNC__, __LINE__, Top.Red, Top.Green, Top.Blue));

		// blit pre-generated gradient column by column into dest bitmap
		for (x = 0; x < ptInfo->ptinf_width; x++)
			{
			WritePixelArray(Source,
				0, 0,
				sizeof(struct ARGB) * 1,
				rp,
				x, 0,
				1, ptInfo->ptinf_height,
				RECTFMT_ARGB);
			}

		ScalosGfxFreeARGB(&Source);

		Success = TRUE;
		}

	return Success;
}


static BOOL PatternBackgroundSingleColor(struct PatternNode *ptNode,
	struct PatternInfo *ptInfo, struct RastPort *rp)
{
	// Try to allocate pen for background color
	ptInfo->ptinf_BgPen = ObtainBestPen(iInfos.xii_iinfos.ii_Screen->ViewPort.ColorMap,
		RGB_8_TO_32(ptNode->ptn_BgColor1[0]),
		RGB_8_TO_32(ptNode->ptn_BgColor1[1]),
		RGB_8_TO_32(ptNode->ptn_BgColor1[2]),
		OBP_Precision, PRECISION_GUI,
		OBP_FailIfBad, FALSE,
		TAG_END);

	if (NO_PEN == ptInfo->ptinf_BgPen)
		return FALSE;		// fail if no pen available

	SetRast(rp, ptInfo->ptinf_BgPen);

	return TRUE;
}
