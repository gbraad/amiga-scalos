
// FileTransClass.c
// $Date$
// $Revision$


#include <exec/types.h>
#include <graphics/rastport.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <datatypes/pictureclass.h>
#include <utility/hooks.h>
#include <devices/timer.h>
#include <intuition/gadgetclass.h>
#include <intuition/newmouse.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <dos/dostags.h>
#include <dos/datetime.h>

#define	__USE_SYSBASE

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/layers.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/locale.h>
#include <proto/gadtools.h>
#include <proto/timer.h>
#include <proto/iconobject.h>
#include "debug.h"
#include <proto/scalos.h>

#include <clib/alib_protos.h>

#include <defs.h>
#include <datatypes/iconobject.h>
#include <scalos/scalos.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scalos_structures.h"
#include "functions.h"
#include "locale.h"
#include "Variables.h"

#include "int64.h"
#include "FileTransClass.h"
#include "GaugeGadgetClass.h"

//----------------------------------------------------------------------------

// local data definitions

//#define	SHOW_EVERY_OBJECT
#define	SHOW_REMAINING_TIME

struct FileTransOp
	{
	struct Node fto_Node;
	ULONG fto_OpCode;	// enum ftOpCodes

	BPTR fto_SrcDirLock;
	CONST_STRPTR fto_SrcName;

	BPTR fto_DestDirLock;
	CONST_STRPTR fto_DestName;

	LONG fto_MouseX;		// position for destination icon
	LONG fto_MouseY;
	};

//----------------------------------------------------------------------------

// local functions

static SAVEDS(ULONG) FileTransClass_Dispatcher(Class *cl, Object *o, Msg msg);
static ULONG FileTransClass_New(Class *cl, Object *o, Msg msg);
static ULONG FileTransClass_Dispose(Class *cl, Object *o, Msg msg);
static ULONG FileTransClass_Copy(Class *cl, Object *o, Msg msg);
static ULONG FileTransClass_Move(Class *cl, Object *o, Msg msg);
static ULONG FileTransClass_CreateLink(Class *cl, Object *o, Msg msg);
static ULONG FileTransClass_CheckAbort(Class *cl, Object *o, Msg msg);
static ULONG FileTransClass_OpenWindow(Class *cl, Object *o, Msg msg);
static ULONG FileTransClass_DelayedOpenWindow(Class *cl, Object *o, Msg msg);
static ULONG FileTransClass_UpdateWindow(Class *cl, Object *o, Msg msg);

static void AddFTOp(struct FileTransClassInstance *inst,
	enum ftOpCodes opType,
	BPTR srcDir, CONST_STRPTR srcName,
	BPTR destDir, CONST_STRPTR destName,
	LONG x, LONG y);
static void ProcessFTOps(Class *cl, Object *o);
static LONG DoFileTransCopy(Class *cl, Object *o, struct FileTransOp *fto);
static LONG DoFileTransMove(Class *cl, Object *o, struct FileTransOp *fto);
static LONG DoFileTransCreateLink(Class *cl, Object *o, struct FileTransOp *fto);
static void PutIcon2(CONST_STRPTR SrcName, BPTR srcLock, 
	CONST_STRPTR DestName, BPTR destLock, LONG x, LONG y);
static void FreeFTO(struct FileTransOp *fto);
static LONG CountFTOps(Class *cl, Object *o, struct FileTransOp *fto);
static SAVEDS(ULONG) FileTrans_BackFillFunc(struct Hook *bfHook, struct RastPort *rp, struct BackFillMsg *msg);
static LONG GetElapsedTime(T_TIMEVAL *tv);
static ULONG FileTransClass_CheckCountTimeout(Class *cl, Object *o, Msg msg);
static ULONG FileTransClass_OverwriteRequest(Class *cl, Object *o, Msg msg);
static ULONG FileTransClass_WriteProtectedRequest(Class *cl, Object *o, Msg msg);

//----------------------------------------------------------------------------

struct ScalosClass *initFileTransClass(const struct PluginClass *plug)
{
	struct ScalosClass *FileTransClass;

	FileTransClass = SCA_MakeScalosClass(plug->plug_classname, 
			plug->plug_superclassname,
			sizeof(struct FileTransClassInstance),
			NULL);

	if (FileTransClass)
		{
		// initialize the cl_Dispatcher Hook
		SETHOOKFUNC(FileTransClass->sccl_class->cl_Dispatcher, FileTransClass_Dispatcher);
		}

	return FileTransClass;
}


static SAVEDS(ULONG) FileTransClass_Dispatcher(Class *cl, Object *o, Msg msg)
{
	ULONG Result;

	switch (msg->MethodID)
		{
	case OM_NEW:
		Result = FileTransClass_New(cl, o, msg);
		break;

	case OM_DISPOSE:
		Result = FileTransClass_Dispose(cl, o, msg);
		break;

	case SCCM_FileTrans_Copy:
		Result = FileTransClass_Copy(cl, o, msg);
		break;

	case SCCM_FileTrans_Move:
		Result = FileTransClass_Move(cl, o, msg);
		break;

	case SCCM_FileTrans_CreateLink:
		Result = FileTransClass_CreateLink(cl, o, msg);
		break;

	case SCCM_FileTrans_CheckAbort:
		Result = FileTransClass_CheckAbort(cl, o, msg);
		break;

	case SCCM_FileTrans_OpenWindow:
		Result = FileTransClass_OpenWindow(cl, o, msg);
		break;

	case SCCM_FileTrans_OpenWindowDelayed:
		Result = FileTransClass_DelayedOpenWindow(cl, o, msg);
		break;

	case SCCM_FileTrans_UpdateWindow:
		Result = FileTransClass_UpdateWindow(cl, o, msg);
		break;

	case SCCM_FileTrans_CountTimeout:
		Result = FileTransClass_CheckCountTimeout(cl, o, msg);
		break;

	case SCCM_FileTrans_OverwriteRequest:
		Result = FileTransClass_OverwriteRequest(cl, o, msg);
		break;

	case SCCM_FileTrans_WriteProtectedRequest:
		Result = FileTransClass_WriteProtectedRequest(cl, o, msg);
		break;

	default:
		Result = DoSuperMethodA(cl, o, msg);
		break;
		}

	return Result;
}


static ULONG FileTransClass_New(Class *cl, Object *o, Msg msg)
{
	d1(kprintf("%s/%s/%ld: iwt=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, iwt, iwt->iwt_WinTitle));

	o = (Object *) DoSuperMethodA(cl, o, msg);
	if (o)	
		{
		struct opSet *ops = (struct opSet *) msg;
		struct FileTransClassInstance *inst = INST_DATA(cl, o);

		ScalosInitSemaphore(&inst->ftci_OpListSemaphore);
		NewList(&inst->ftci_OpList);

		inst->ftci_TotalBytes = Make64(0);
		inst->ftci_TotalItems = 0;
		inst->ftci_TotalFiles = 0;
		inst->ftci_TotalDirs = 0;
		inst->ftci_TotalLinks = 0;

		SETHOOKFUNC(inst->ftci_BackFillHook, FileTrans_BackFillFunc);
		inst->ftci_BackFillHook.h_Data = inst;

		inst->ftci_Background = CreateDatatypesImage("THEME:FileTransBackground", 0);

		ScalosObtainSemaphoreShared(&CopyHookSemaphore);

		inst->ftci_Number = GetTagData(SCCA_FileTrans_Number, 0, ops->ops_AttrList);
		inst->ftci_Screen = (struct Screen *) GetTagData(SCCA_FileTrans_Screen, (ULONG)NULL, ops->ops_AttrList);
		inst->ftci_ReplaceMode = GetTagData(SCCA_FileTrans_ReplaceMode, SCCV_ReplaceMode_Ask, ops->ops_AttrList);
		inst->ftci_OverwriteMode = GetTagData(SCCA_FileTrans_WriteProtectedMode, SCCV_WriteProtectedMode_Ask, ops->ops_AttrList);

		inst->ftci_Line1Buffer = AllocPathBuffer();
		inst->ftci_Line2Buffer = AllocPathBuffer();

		if (NULL == inst->ftci_Line1Buffer || NULL == inst->ftci_Line2Buffer)
			{
			DoMethod(o, OM_DISPOSE);
			o = NULL;
			}

		if (globalCopyHook)
			{
			struct CopyBeginMsg cbm;

			cbm.cbm_Length = sizeof(cbm);
			cbm.cbm_Action = CPACTION_Begin;
			cbm.cbm_SourceDrawer = (BPTR)NULL;
			cbm.cbm_DestinationDrawer = (BPTR)NULL;

			inst->ftci_HookResult = CallHookPkt(globalCopyHook, NULL, &cbm);
			}
		}

	return (ULONG) o;
}


static ULONG FileTransClass_Dispose(Class *cl, Object *o, Msg msg)
{
	struct FileTransClassInstance *inst = INST_DATA(cl, o);

	if (globalCopyHook && RETURN_OK == inst->ftci_HookResult)
		{
		struct CopyEndMsg cem;

		cem.cem_Length = sizeof(cem);
		cem.cem_Action = CPACTION_End;

		inst->ftci_HookResult = CallHookPkt(globalCopyHook, NULL, &cem);
		}

	ProcessFTOps(cl, o);

	ScalosReleaseSemaphore(&CopyHookSemaphore);

	if (inst->ftci_Window)
		LockedCloseWindow(inst->ftci_Window);

	FreeGadgets(inst->ftci_GadgetList);

	if (inst->ftci_GaugeGadget)
		SCA_DisposeScalosObject((Object *) inst->ftci_GaugeGadget);

	if (inst->ftci_Line1Buffer)
		FreePathBuffer(inst->ftci_Line1Buffer);
	if (inst->ftci_Line2Buffer)
		FreePathBuffer(inst->ftci_Line2Buffer);

	if (inst->ftci_LastErrorFileName)
		FreeCopyString(inst->ftci_LastErrorFileName);

	DisposeDatatypesImage(&inst->ftci_Background);

	d1(kprintf("%s/%s/%ld: iwt=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, iwt, iwt->iwt_WinTitle));

	return DoSuperMethodA(cl, o, msg);
}


static ULONG FileTransClass_Copy(Class *cl, Object *o, Msg msg)
{
	struct FileTransClassInstance *inst = INST_DATA(cl, o);
	struct msg_Copy *mcp = (struct msg_Copy *) msg;
	ULONG Result;

	d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

	if (globalCopyHook && RETURN_OK == inst->ftci_HookResult)
		{
		struct CopyDataMsg cdm;

		cdm.cdm_Length = sizeof(cdm);
		cdm.cdm_Action = CPACTION_Copy;
		cdm.cdm_SourceLock = mcp->mcp_SrcDirLock;
		cdm.cdm_SourceName = (STRPTR) mcp->mcp_SrcName;
		cdm.cdm_DestinationLock = mcp->mcp_DestDirLock;
		cdm.cdm_DestinationName = (STRPTR) mcp->mcp_DestName;
		cdm.cdm_DestinationX = mcp->mcp_MouseX;
		cdm.cdm_DestinationY = mcp->mcp_MouseY;

		if (NULL == cdm.cdm_SourceName)
			{
			// SrcFileName may be NULL, 
			// then SourceDirLock is an absolute filename, i.e. "Volume:"
			cdm.cdm_SourceName = (STRPTR) "";
			}

		if (NULL == cdm.cdm_DestinationName)
			cdm.cdm_DestinationName = cdm.cdm_SourceName;

		d1(kprintf("%s/%s/%ld: Src name=<%s>\n", __FILE__, __FUNC__, __LINE__, cdm.cdm_SourceName));
		debugLock_d1(cdm.cdm_SourceLock);
		d1(kprintf("%s/%s/%ld: Dest name=<%s>\n", __FILE__, __FUNC__, __LINE__, cdm.cdm_DestinationName));
		debugLock_d1(cdm.cdm_DestinationLock);

		Result = CallHookPkt(globalCopyHook, NULL, &cdm);

		return Result;
		}
	else
		{
		AddFTOp(inst, FTOPCODE_Copy,
			mcp->mcp_SrcDirLock, mcp->mcp_SrcName,
			mcp->mcp_DestDirLock, mcp->mcp_DestName,
			mcp->mcp_MouseX, mcp->mcp_MouseY);
		}

	return RETURN_OK;
}


static ULONG FileTransClass_Move(Class *cl, Object *o, Msg msg)
{
	struct FileTransClassInstance *inst = INST_DATA(cl, o);
	struct msg_Move *mmv = (struct msg_Move *) msg;

	d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
	d1(kprintf("%s/%s/%ld: Name=<%s>  x=%ld  y=%ld\n", \
		__LINE__, mmv->mmv_Name, mmv->mmv_MouseX, mmv->mmv_MouseY));

	AddFTOp(inst, FTOPCODE_Move,
		mmv->mmv_SrcDirLock, mmv->mmv_Name,
		mmv->mmv_DestDirLock, mmv->mmv_Name,
		mmv->mmv_MouseX, mmv->mmv_MouseY);

	return RETURN_OK;
}


static ULONG FileTransClass_CreateLink(Class *cl, Object *o, Msg msg)
{
	struct FileTransClassInstance *inst = INST_DATA(cl, o);
	struct msg_Move *mmv = (struct msg_Move *) msg;

	d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
	d1(kprintf("%s/%s/%ld: Name=<%s>  x=%ld  y=%ld\n", \
		__LINE__, mmv->mmv_Name, mmv->mmv_MouseX, mmv->mmv_MouseY));

	AddFTOp(inst, FTOPCODE_CreateLink,
		mmv->mmv_SrcDirLock, mmv->mmv_Name,
		mmv->mmv_DestDirLock, mmv->mmv_Name,
		mmv->mmv_MouseX, mmv->mmv_MouseY);

	return RETURN_OK;
}


static ULONG FileTransClass_OpenWindow(Class *cl, Object *o, Msg msg)
{
	struct FileTransClassInstance *inst = INST_DATA(cl, o);
	struct Gadget *gad;
	struct NewGadget ng;

	AskFont(&inst->ftci_Screen->RastPort, &inst->ftci_TextAttr);

	do	{
		LONG WinWidth, WinHeight;
		CONST_STRPTR WinTitle;

		WinWidth = 300;

		gad = CreateContext(&inst->ftci_GadgetList);
		if (NULL == gad)
			break;

		ng.ng_LeftEdge = 10;
		ng.ng_TopEdge = 10 + inst->ftci_TextAttr.ta_YSize;
		ng.ng_Width = WinWidth - 10 - 10;
		ng.ng_Height = inst->ftci_TextAttr.ta_YSize + 4;
		ng.ng_GadgetText = NULL;
		ng.ng_TextAttr = &inst->ftci_TextAttr;
		ng.ng_GadgetID = 0;
		ng.ng_Flags = PLACETEXT_LEFT;
		ng.ng_VisualInfo = iInfos.ii_visualinfo;
		ng.ng_UserData = NULL;

#ifdef SHOW_EVERY_OBJECT
		d1(kprintf("%s/%s/%ld: Line1Buffer=<%s>\n", __FILE__, __FUNC__, __LINE__, inst->ftci_Line1Buffer));

		gad = inst->ftci_TextLine1Gadget = CreateGadget(TEXT_KIND, gad, &ng,
			GTTX_Text, inst->ftci_Line1Buffer,
			GTTX_Border, FALSE,
			GTTX_Clipped, TRUE,
			GTTX_Justification, GTJ_CENTER,
			TAG_END);

		if (NULL == gad)
			break;

		ng.ng_TopEdge += ng.ng_Height + 5;
#endif /* SHOW_EVERY_OBJECT */

		d1(kprintf("%s/%s/%ld: Line2Buffer=<%s>\n", __FILE__, __FUNC__, __LINE__, inst->ftci_Line2Buffer));

		gad = inst->ftci_TextLine2Gadget = CreateGadget(TEXT_KIND, gad, &ng,
			GTTX_Text, (ULONG) inst->ftci_Line2Buffer,
			GTTX_Border, FALSE,
			GTTX_Clipped, TRUE,
			GTTX_Justification, GTJ_LEFT,
			TAG_END);

		if (NULL == gad)
			break;

		ng.ng_TopEdge += ng.ng_Height + 5;

		gad = inst->ftci_GaugeGadget = (struct Gadget *) SCA_NewScalosObjectTags("Gauge.sca",
			GA_Left, ng.ng_LeftEdge,
			GA_Top, ng.ng_TopEdge,
			GA_Width, ng.ng_Width,
			GA_Height, ng.ng_Height,
			GA_Previous, (ULONG) gad,
			SCAGAUGE_MinValue, 0,
			SCAGAUGE_MaxValue, 100,
			SCAGAUGE_Level, 0,
			SCAGAUGE_Vertical, FALSE,
			SCAGAUGE_WantNumber, FALSE,
			SCAGAUGE_BarPen, iInfos.ii_DrawInfo->dri_Pens[FILLPEN],
			TAG_END);

		if (NULL == gad)
			break;
#ifdef SHOW_REMAINING_TIME
		ng.ng_TopEdge += ng.ng_Height + 5;

		gad = inst->ftci_TextLine3Gadget = CreateGadget(TEXT_KIND, gad, &ng,
			GTTX_Text, (ULONG) inst->ftci_Line3Buffer,
			GTTX_Border, FALSE,
			GTTX_Clipped, TRUE,
			GTTX_Justification, GTJ_CENTER,
			TAG_END);

		if (NULL == gad)
			break;
#endif
		ng.ng_TopEdge += ng.ng_Height + 5;

		ng.ng_GadgetText = (STRPTR) GetLocString(MSGID_CANCELBUTTON);
		ng.ng_Height = inst->ftci_TextAttr.ta_YSize + 4 + 2;
		ng.ng_Width = 5 + 5 + Scalos_TextLength(&inst->ftci_Screen->RastPort, ng.ng_GadgetText, strlen(ng.ng_GadgetText));
		ng.ng_LeftEdge = (WinWidth - ng.ng_Width) / 2;
		ng.ng_Flags = PLACETEXT_IN;
		ng.ng_GadgetID = GID_CancelButton;

		gad = inst->ftci_CancelButtonGadget = CreateGadget(BUTTON_KIND, gad, &ng,
			GT_Underscore, '_',
			TAG_END);

		if (NULL == gad)
			break;

		WinHeight = ng.ng_TopEdge + ng.ng_Height + 10;

		switch (inst->ftci_CurrentOperation)
			{
		case FTOPCODE_Copy:
			WinTitle = GetLocString(MSGID_FILETRANSTITLE_COPY);
			break;
		case FTOPCODE_Move:
			WinTitle = GetLocString(MSGID_FILETRANSTITLE_MOVE);
			break;
		default:
			WinTitle = "";
			break;
			}

		inst->ftci_Window = LockedOpenWindowTags(NULL,
			WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_GADGETUP,
			WA_Gadgets, inst->ftci_GadgetList,
			WA_CustomScreen, inst->ftci_Screen,
			WA_Left, (inst->ftci_Screen->Width - WinWidth) / 2,
			WA_Top, (inst->ftci_Screen->Height - WinHeight) / 2,
			WA_Height, WinHeight,
			WA_Width, WinWidth,
			WA_BackFill, &inst->ftci_BackFillHook,
			WA_ScreenTitle, ScreenTitleBuffer,
			WA_Flags, WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_SMART_REFRESH | WFLG_RMBTRAP,
			WA_Title, WinTitle,
			TAG_END);
		if (NULL == inst->ftci_Window)
			break;

		// ...seems like GTTX_Text isn't honored during CreateGadget(),
		// so I set the Gadget texts here again
#ifdef SHOW_EVERY_OBJECT
		GT_SetGadgetAttrs(inst->ftci_TextLine1Gadget, inst->ftci_Window, NULL,
			GTTX_Text, inst->ftci_Line1Buffer,
			TAG_END);
#endif /* SHOW_EVERY_OBJECT */

		GT_SetGadgetAttrs(inst->ftci_TextLine2Gadget, inst->ftci_Window, NULL,
			GTTX_Text, (ULONG) inst->ftci_Line2Buffer,
			TAG_END);

		} while (0);

	d1(Delay(5 * 50));

	return (ULONG) inst->ftci_Window;
}


static ULONG FileTransClass_CheckAbort(Class *cl, Object *o, Msg msg)
{
	ULONG Result = RETURN_OK;
	struct FileTransClassInstance *inst = INST_DATA(cl, o);

	if (inst->ftci_Window)
		{
		struct IntuiMessage *iMsg;

		d1(kprintf("%s/%s/%ld: Handling intuition messages for copy window\n", __FILE__, __FUNC__, __LINE__));

		while ((iMsg = GT_GetIMsg(inst->ftci_Window->UserPort)))
			{
			ULONG msgClass = iMsg->Class;
			struct Gadget *g = (struct Gadget *) iMsg->IAddress;

			GT_ReplyIMsg(iMsg);

			d1(kprintf("%s/%s/%ld: Class=%08lx\n", __FILE__, __FUNC__, __LINE__, msgClass));

			switch (msgClass)
				{
			case IDCMP_GADGETUP:
				switch (g->GadgetID)
					{
				case GID_CancelButton:
					Result = RESULT_UserAborted;
					break;
					}
				break;

			case IDCMP_CLOSEWINDOW:
				Result = RESULT_UserAborted;
				break;
				}
			}
		}

	return Result;
}


static void AddFTOp(struct FileTransClassInstance *inst,
	enum ftOpCodes opType,
	BPTR srcDir, CONST_STRPTR srcName,
	BPTR destDir, CONST_STRPTR destName,
	LONG x, LONG y)
{
	struct FileTransOp *fto;

	inst->ftci_CurrentOperation = opType;

	fto = ScalosAllocVecPooled(sizeof(struct FileTransOp));
	d1(kprintf("%s/%s/%ld: fto=%08lx\n", __FILE__, __FUNC__, __LINE__, fto));
	if (fto)
		{
		debugLock_d1(srcDir);
		debugLock_d1(destDir);
		d1(kprintf("%s/%s/%ld: Op=%ld  Src=<%s>  Dest=<%s>\n", __FILE__, __FUNC__, __LINE__, opType, srcName, destName));
		d1(kprintf("%s/%s/%ld: x=%ld  y=%ld\n", __FILE__, __FUNC__, __LINE__, x, y));

		fto->fto_OpCode = opType;
		fto->fto_SrcDirLock = DupLock(srcDir);
		fto->fto_DestDirLock = DupLock(destDir);

		if (NULL == srcName)
			{
			// SrcFileName may be NULL, 
			// then SourceDirLock is an absolute filename, i.e. "Volume:"
			fto->fto_SrcName = AllocCopyString("");
			}
		else
			fto->fto_SrcName = AllocCopyString(srcName);

		if (NULL == destName)
			fto->fto_DestName = AllocCopyString(srcName);
		else
			fto->fto_DestName = AllocCopyString(destName);

		fto->fto_MouseX = x;
		fto->fto_MouseY = y;

		ScalosObtainSemaphore(&inst->ftci_OpListSemaphore);
		AddTail(&inst->ftci_OpList, &fto->fto_Node);
		ScalosReleaseSemaphore(&inst->ftci_OpListSemaphore);
		}
}


static void ProcessFTOps(Class *cl, Object *o)
{
	struct FileTransClassInstance *inst = INST_DATA(cl, o);
	struct internalScaWindowTask *iwtMain = (struct internalScaWindowTask *) iInfos.ii_MainWindowStruct->ws_WindowTask;

	inst->ftci_CurrentBytes = Make64(0);
	inst->ftci_CurrentItems = 0;

	GetSysTime(&inst->ftci_ProcessStartTime);

	inst->ftci_CopyStartTime = inst->ftci_ProcessStartTime;
	inst->ftci_LastRemainTimeDisplay = inst->ftci_ProcessStartTime;

	ScalosObtainSemaphore(&inst->ftci_OpListSemaphore);

	if (!IsListEmpty(&inst->ftci_OpList))
		{
		struct FileTransOp *fto;
		LONG Result = RETURN_OK;

		DoMethod(o, SCCM_FileTrans_OpenWindowDelayed);

		fto = (struct FileTransOp *) inst->ftci_OpList.lh_Head;

#ifdef SHOW_EVERY_OBJECT
		stccpy(inst->ftci_Line1Buffer, 
			GetLocString((FTOPCODE_Copy == fto->fto_OpCode) ? MSGID_PREPARING_COPY : MSGID_PREPARING_MOVE), 
			Max_PathLen);
		strcpy(inst->ftci_Line2Buffer, "");
#else
		stccpy(inst->ftci_Line2Buffer, 
			GetLocString((FTOPCODE_Copy == fto->fto_OpCode) ? MSGID_PREPARING_COPY : MSGID_PREPARING_MOVE), 
			Max_PathLen);
#endif /* SHOW_EVERY_OBJECT */

		DoMethod(o, SCCM_FileTrans_UpdateWindow,
			FTUPDATE_Initial,
			NULL, NULL, "");

		for (fto = (struct FileTransOp *) inst->ftci_OpList.lh_Head;
			(RETURN_OK == Result) && (fto != (struct FileTransOp *) &inst->ftci_OpList.lh_Tail);
			fto = (struct FileTransOp *) fto->fto_Node.ln_Succ)
			{
			d1(KPrintF("%s/%s/%ld: fto=%08lx\n", __FILE__, __FUNC__, __LINE__, fto));
			Result = CountFTOps(cl, o, fto);
			}

		inst->ftci_CountValid = RETURN_OK == Result;
		d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

		GetSysTime(&inst->ftci_CopyStartTime);

		if (inst->ftci_LastErrorFileName)
			{
			FreeCopyString(inst->ftci_LastErrorFileName);
			inst->ftci_LastErrorFileName = NULL;
			}

		Result = inst->ftci_LastErrorCode = RETURN_OK;

		while ((fto = (struct FileTransOp *) RemHead(&inst->ftci_OpList)))
			{
			d1(kprintf("%s/%s/%ld: fto=%08lx\n", __FILE__, __FUNC__, __LINE__, fto));
			debugLock_d1(fto->fto_SrcDirLock);
			debugLock_d1(fto->fto_DestDirLock);
			d1(kprintf("%s/%s/%ld: Op=%ld  Src=<%s>  Dest=<%s>\n", \
				__LINE__, fto->fto_OpCode, fto->fto_SrcName, fto->fto_DestName));

			if (RETURN_OK == Result)
				{
				switch (fto->fto_OpCode)
					{
				case FTOPCODE_Copy:
					Result = DoFileTransCopy(cl, o, fto);
					break;

				case FTOPCODE_Move:
					Result = DoFileTransMove(cl, o, fto);
					break;

				case FTOPCODE_CreateLink:
					Result = DoFileTransCreateLink(cl, o, fto);
					break;
					}

				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				}

			FreeFTO(fto);
			}
		d1(kprintf("%s/%s/%ld: fto=%08lx\n", __FILE__, __FUNC__, __LINE__, fto));
		d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
		}

	ScalosReleaseSemaphore(&inst->ftci_OpListSemaphore);

	DoMethod(iwtMain->iwt_WindowTask.mt_MainObject, SCCM_ClearClipboard);
}


static LONG DoFileTransCopy(Class *cl, Object *o, struct FileTransOp *fto)
{
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG Result;
	BOOL VolumeHasIcon = FALSE;
	BPTR VoldDir;
	Object* OldIconObj = NULL;


	// Check if "fto->fto_SrcName" lenght = 0 from current dir "fto->fto_SrcDirLock", 
	// then try to allocate a icon Object for disk icon.
	// If ok and if icon type = "WBDISK", disk icon image will be used instead to use
	// the default def_drawer icon image.
	//
	VoldDir = CurrentDir(fto->fto_SrcDirLock);
	if (0 == strlen(fto->fto_SrcName))
		{
		OldIconObj = NewIconObject("disk", NULL);
		if (OldIconObj)
			{
			ULONG IconType;

			GetAttr(IDTA_Type, OldIconObj, &IconType);

			if (WBDISK == IconType)
				VolumeHasIcon = TRUE;
			}
		}
	CurrentDir(VoldDir);

	Result = CopyCommand(cl, o, fto->fto_SrcDirLock, fto->fto_DestDirLock,
		fto->fto_SrcName, fto->fto_DestName);

	d1(kprintf("%s/%s/%ld: Result=%08lx  ftci_MostCurrentReplaceMode=%ld\n", __FILE__, __FUNC__, __LINE__, Result, ftci->ftci_MostCurrentReplaceMode));

	if (RETURN_OK == Result)
		{
		// Copying of object finished successfully
		// Now try to copy the acompanying icon

		BPTR oldDir = CurrentDir(fto->fto_DestDirLock);
		Object *IconObj;
		LONG existsResult = EXISTREQ_Replace;

		d1(kprintf("%s/%s/%ld: SrcName=<%s>\n", __FILE__, __FUNC__, __LINE__, fto->fto_SrcName));

		IconObj = NewIconObjectTags(fto->fto_DestName, TAG_END);
		if (IconObj)
			{
			STRPTR IconName;

			switch (ftci->ftci_MostCurrentReplaceMode)
				{
			case SCCV_ReplaceMode_Ask:
				IconName = AllocPathBuffer();
				if (IconName)
					{
					CONST_STRPTR Extension = "";

					GetAttr(IDTA_Extention, IconObj, (APTR) &Extension);

					stccpy(IconName, fto->fto_DestName, Max_PathLen);
					SafeStrCat(IconName, Extension, Max_PathLen);

					existsResult = ftci->ftci_MostCurrentReplaceMode = DoMethod(o, SCCM_FileTrans_OverwriteRequest,
						OVERWRITEREQ_CopyIcon,
						fto->fto_SrcDirLock, IconName,
						fto->fto_DestDirLock, IconName,
						ftci->ftci_Window, 
						MSGID_EXISTSICON_COPY, MSGID_EXISTS_GNAME_NEW);

					d1(kprintf("%s/%s/%ld: Result=%08lx  ftci_MostCurrentReplaceMode=%ld\n", \
						__LINE__, Result, ftci->ftci_MostCurrentReplaceMode));

					FreePathBuffer(IconName);
					}
				break;
			case SCCV_ReplaceMode_Never:
				existsResult = EXISTREQ_Skip;
				break;
			case SCCV_ReplaceMode_Always:
				existsResult = EXISTREQ_Replace;
				break;
				}

			d1(kprintf("%s/%s/%ld: existsResult=%ld\n", __FILE__, __FUNC__, __LINE__, existsResult));

			switch (existsResult)
				{
			case EXISTREQ_ReplaceAll:
				existsResult = EXISTREQ_Replace;
				break;
			case EXISTREQ_SkipAll:
				existsResult = EXISTREQ_Skip;
				break;
			default:
				break;
				}

			DisposeIconObject(IconObj);
			}

		if (EXISTREQ_Replace == existsResult)
			{
			if (0 == strlen(fto->fto_SrcName))
				{
				// Source is an absolute filename, i.e. "Volume:"
				// Source is to be copied into a directory

				if (VolumeHasIcon)
					{
					// Disk icon found from source.
					// Set correct "DTA_Name" and "IDTA_Type" for OldIconObj.
					//
					SetAttrs(OldIconObj,
						DTA_Name, fto->fto_DestName,
						IDTA_Type, (ULONG) WBDRAWER,
						TAG_END);

					IconObj = OldIconObj;
					}
				else
					{
					IconObj = GetDefIconObject(WBDRAWER, NULL);
					}

				d1(kprintf("%s/%s/%ld: IconObj=%08lx\n", __FILE__, __FUNC__, __LINE__, IconObj));

				if (IconObj)
					{
					struct ExtGadget *gg = (struct ExtGadget *) IconObj;

					d1(kprintf("%s/%s/%ld: x=%ld  y=%ld\n", \
							__LINE__, fto->fto_MouseX, fto->fto_MouseY));

					gg->LeftEdge = fto->fto_MouseX;
					gg->TopEdge = fto->fto_MouseY;

					PutIconObjectTags(IconObj, fto->fto_DestName,
						ICONA_NoNewImage, TRUE,
						TAG_END);

					DisposeIconObject(IconObj);

					}
				}
			else
				{
				PutIcon2(fto->fto_SrcName, fto->fto_SrcDirLock, 
					fto->fto_DestName, fto->fto_DestDirLock,
					fto->fto_MouseX, fto->fto_MouseY);
				}

			if (!VolumeHasIcon)
				{
				if (OldIconObj)
					DisposeIconObject(OldIconObj);
				}

			// update destination window
			ScalosDropAddIcon(fto->fto_DestDirLock, fto->fto_DestName,
				fto->fto_MouseX, fto->fto_MouseY);
			}

		CurrentDir(oldDir);
		}

	return Result;
}


static LONG DoFileTransMove(Class *cl, Object *o, struct FileTransOp *fto)
{
	LONG Result;
	BPTR IconLock;

	debugLock_d1(fto->fto_SrcDirLock);
	debugLock_d1(fto->fto_DestDirLock);
	d1(kprintf("%s/%s/%ld: fto_SrcName=<%s>\n", __FILE__, __FUNC__, __LINE__, fto->fto_SrcName));

	Result = MoveCommand(cl, o, fto->fto_SrcDirLock, fto->fto_DestDirLock, fto->fto_SrcName);

	d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, Result));

	if (RETURN_OK == Result)
		{
		CONST_STRPTR ext = ".info";
		char Name[MAX_FileName];

		d1(kprintf("%s/%s/%ld: fto->fto_SrcName without 'ext' = <%s>\n", __FILE__, __FUNC__, __LINE__, fto->fto_SrcName));

		stccpy(Name, fto->fto_SrcName, sizeof(Name) - strlen(ext) - 1 );
		strcat(Name, ext);

		if (IconLock = Lock(Name, SHARED_LOCK))
			{
			UnLock(IconLock);

			d1(kprintf("%s/%s/%ld: Found ICON: <%s>\n", __FILE__, __FUNC__, __LINE__, Name));

			// move associated icon
			Result = MoveCommand(cl, o, fto->fto_SrcDirLock, fto->fto_DestDirLock, Name);

			d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, Result));
			if (RETURN_OK == Result)
				{
				PutIcon2(fto->fto_SrcName, fto->fto_DestDirLock, 
					fto->fto_DestName, fto->fto_DestDirLock, 
					fto->fto_MouseX, fto->fto_MouseY);
				}
			}
		}

	// always try to handle icons apprioately if move was successfull
	// even if no .info icon could be moved since windows might be "show all files"
	if (RETURN_OK == Result)
		{
		// update source window
		ScalosDropRemoveIcon(fto->fto_SrcDirLock, fto->fto_SrcName);

		// update destination window
		ScalosDropAddIcon(fto->fto_DestDirLock, fto->fto_DestName,
			fto->fto_MouseX, fto->fto_MouseY);
		}

	return Result;
}


static LONG DoFileTransCreateLink(Class *cl, Object *o, struct FileTransOp *fto)
{
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG Result;

	Result = CreateLinkCommand(cl, o, fto->fto_SrcDirLock, fto->fto_DestDirLock, fto->fto_SrcName);

	d1(KPrintF("%s/%s/%ld: Result=%ld  ftci_MostCurrentReplaceMode=%ld\n", __FILE__, __FUNC__, __LINE__, Result, ftci->ftci_MostCurrentReplaceMode));

	if (RETURN_OK == Result)
		{
		// Link to object created successfully
		// Now try to copy the object's icon

		BPTR oldDir = CurrentDir(fto->fto_DestDirLock);
		Object *IconObj;
		LONG existsResult = EXISTREQ_Replace;

		d1(KPrintF("%s/%s/%ld: SrcName=<%s>\n", __FILE__, __FUNC__, __LINE__, fto->fto_SrcName));

		IconObj = NewIconObjectTags(fto->fto_DestName, TAG_END);
		if (IconObj)
			{
			STRPTR IconName;

			switch (ftci->ftci_MostCurrentReplaceMode)
				{
			case SCCV_ReplaceMode_Ask:
				IconName = AllocPathBuffer();
				if (IconName)
					{
					CONST_STRPTR Extension = "";

					GetAttr(IDTA_Extention, IconObj, (APTR) &Extension);

					stccpy(IconName, fto->fto_DestName, Max_PathLen);
					SafeStrCat(IconName, Extension, Max_PathLen);

					existsResult = ftci->ftci_MostCurrentReplaceMode = DoMethod(o, SCCM_FileTrans_OverwriteRequest,
						OVERWRITEREQ_CopyIcon,
						fto->fto_SrcDirLock, IconName,
						fto->fto_DestDirLock, IconName,
						ftci->ftci_Window, 
						MSGID_EXISTSICON_COPY, MSGID_EXISTS_GNAME_NEW);

					d1(KPrintF("%s/%s/%ld: Result=%08lx  ftci_MostCurrentReplaceMode=%ld\n", \
						__LINE__, Result, ftci->ftci_MostCurrentReplaceMode));

					FreePathBuffer(IconName);
					}
				break;
			case SCCV_ReplaceMode_Never:
				existsResult = EXISTREQ_Skip;
				break;
			case SCCV_ReplaceMode_Always:
				existsResult = EXISTREQ_Replace;
				break;
				}

			d1(KPrintF("%s/%s/%ld: existsResult=%ld\n", __FILE__, __FUNC__, __LINE__, existsResult));

			switch (existsResult)
				{
			case EXISTREQ_ReplaceAll:
				existsResult = EXISTREQ_Replace;
				break;
			case EXISTREQ_SkipAll:
				existsResult = EXISTREQ_Skip;
				break;
			default:
				break;
				}

			DisposeIconObject(IconObj);
			}

		if (EXISTREQ_Replace == existsResult)
			{
			if (0 == strlen(fto->fto_SrcName))
				{
				// Source is an absolute filename, i.e. "Volume:"
				// Source is to be copied into a directory

				IconObj = GetDefIconObject(WBDRAWER, NULL);

				d1(KPrintF("%s/%s/%ld: IconObj=%08lx\n", __FILE__, __FUNC__, __LINE__, IconObj));
				if (IconObj)
					{
					struct ExtGadget *gg = (struct ExtGadget *) IconObj;

					d1(KPrintF("%s/%s/%ld: x=%ld  y=%ld\n", \
						__LINE__, fto->fto_MouseX, fto->fto_MouseY));

					gg->LeftEdge = fto->fto_MouseX;
					gg->TopEdge = fto->fto_MouseY;

					PutIconObjectTags(IconObj, fto->fto_DestName,
						ICONA_NoNewImage, TRUE,
						TAG_END);

					DisposeIconObject(IconObj);
					}
				}
			else
				{
				PutIcon2(fto->fto_SrcName, fto->fto_SrcDirLock, 
					fto->fto_DestName, fto->fto_DestDirLock,
					fto->fto_MouseX, fto->fto_MouseY);
				}

			// update destination window
			ScalosDropAddIcon(fto->fto_DestDirLock, fto->fto_DestName,
				fto->fto_MouseX, fto->fto_MouseY);
			}

		CurrentDir(oldDir);
		}

	d1(KPrintF("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	return Result;
}


static void PutIcon2(CONST_STRPTR SrcName, BPTR srcLock, 
	CONST_STRPTR DestName, BPTR destLock, LONG x, LONG y)
{
	BPTR oldDir;
	Object *IconObj;

	oldDir = CurrentDir(srcLock);

	IconObj = NewIconObject(SrcName, NULL);
	if (IconObj)
		{
		struct ExtGadget *gg = (struct ExtGadget *) IconObj;

		d1(kprintf("%s/%s/%ld: x=%ld  y=%ld\n", __FILE__, __FUNC__, __LINE__, x, y));

		gg->LeftEdge = x;
		gg->TopEdge = y;

		CurrentDir(destLock);

		// PutIconObject()
		PutIconObjectTags(IconObj, DestName, 
			ICONA_NoNewImage, TRUE,
			TAG_END);

		DisposeIconObject(IconObj);
		}

	CurrentDir(oldDir);
}


static void FreeFTO(struct FileTransOp *fto)
{
	if (fto)
		{
		if (fto->fto_SrcDirLock)
			{
			UnLock(fto->fto_SrcDirLock);
			fto->fto_SrcDirLock = (BPTR)NULL;
			}
		if (fto->fto_DestDirLock)
			{
			UnLock(fto->fto_DestDirLock);
			fto->fto_DestDirLock = (BPTR)NULL;
			}
		if (fto->fto_SrcName)
			{
			FreeCopyString((STRPTR) fto->fto_SrcName);
			fto->fto_SrcName = NULL;
			}
		if (fto->fto_DestName)
			{
			FreeCopyString((STRPTR) fto->fto_DestName);
			fto->fto_DestName = NULL;
			}

		ScalosFreeVecPooled(fto);
		}
}


// Count number of bytes and items to process
static LONG CountFTOps(Class *cl, Object *o, struct FileTransOp *fto)
{
//	struct FileTransClassInstance *inst = INST_DATA(cl, o);
	LONG Result = RETURN_OK;

	d1(KPrintF("%s/%s/%ld: START  TotalBytes=%lu  TotalItems=%ld\n", \
		__LINE__, ULONG64_LOW(inst->ftci_TotalBytes), inst->ftci_TotalItems));

	switch (fto->fto_OpCode)
		{
	case FTOPCODE_CreateLink:
		d1(KPrintF("%s/%s/%ld: FTOPCODE_CreateLink\n", __FILE__, __FUNC__, __LINE__));
		Result = CountCommand(cl, o, fto->fto_SrcDirLock, fto->fto_SrcName, FALSE);
		break;

	case FTOPCODE_Copy:
		d1(KPrintF("%s/%s/%ld: FTOPCODE_Copy\n", __FILE__, __FUNC__, __LINE__));
		CountCommand(cl, o, fto->fto_SrcDirLock, fto->fto_SrcName, TRUE);
		break;

	case FTOPCODE_Move:
		d1(KPrintF("%s/%s/%ld: FTOPCODE_Move\n", __FILE__, __FUNC__, __LINE__));
		switch (ScaSameLock(fto->fto_SrcDirLock, fto->fto_DestDirLock))
			{
		case LOCK_SAME:
			break;
		case LOCK_SAME_VOLUME:
			Result = CountCommand(cl, o, fto->fto_SrcDirLock, fto->fto_SrcName, FALSE);
			break;
		case LOCK_DIFFERENT:
			Result = CountCommand(cl, o, fto->fto_SrcDirLock, fto->fto_SrcName, TRUE);
			break;
			}
		break;
		}

	d1(KPrintF("%s/%s/%ld: END Result=%ld  TotalBytes=%lu  TotalItems=%ld\n", \
		__LINE__, Result, ULONG64_LOW(inst->ftci_TotalBytes), inst->ftci_TotalItems));

	return Result;
}


static ULONG FileTransClass_DelayedOpenWindow(Class *cl, Object *o, Msg msg)
{
	struct FileTransClassInstance *inst = INST_DATA(cl, o);

	if (NULL == inst->ftci_Window)
		{
		LONG Diff = GetElapsedTime(&inst->ftci_ProcessStartTime);

		if (Diff >= CurrentPrefs.pref_FileTransWindowDelay)
			DoMethod(o, SCCM_FileTrans_OpenWindow);
		}

	return (ULONG) inst->ftci_Window;
}


// object == (struct RastPort *) result->RastPort
// message == [ (Layer *) layer, (struct Rectangle) bounds,
//              (LONG) offsetx, (LONG) offsety ]

static SAVEDS(ULONG) FileTrans_BackFillFunc(struct Hook *bfHook, struct RastPort *rp, struct BackFillMsg *msg)
{
	struct FileTransClassInstance *inst = (struct FileTransClassInstance *) bfHook->h_Data;
	struct RastPort rpCopy;

	d1(kprintf("%s/%s/%ld: RastPort=%08lx\n", __FILE__, __FUNC__, __LINE__, rp));
	d1(kprintf("%s/%s/%ld: Rect=%ld %ld %ld %ld\n", __FILE__, __FUNC__, __LINE__, \
		msg->bfm_Rect.MinX, msg->bfm_Rect.MinY, msg->bfm_Rect.MaxX, msg->bfm_Rect.MaxY));
	d1(kprintf("%s/%s/%ld: Layer=%08lx  OffsetX=%ld  OffsetY=%ld\n", __FILE__, __FUNC__, __LINE__, \
		msg->bfm_Layer, msg->bfm_OffsetX, msg->bfm_OffsetY));

	rpCopy = *rp;
	rpCopy.Layer = NULL;

	if (inst->ftci_Background)
		{
		WindowBackFill(&rpCopy, msg, inst->ftci_Background->dti_BitMap,
			inst->ftci_Background->dti_BitMapHeader->bmh_Width,
			inst->ftci_Background->dti_BitMapHeader->bmh_Height,
			iInfos.ii_DrawInfo->dri_Pens[BACKGROUNDPEN],
			0, 0,
			NULL);
		}
	else
		{
		WindowBackFill(&rpCopy, msg, NULL,
			0, 0,
			iInfos.ii_DrawInfo->dri_Pens[BACKGROUNDPEN],
			0, 0,
			NULL);
		}

	return 0;
}


static ULONG FileTransClass_UpdateWindow(Class *cl, Object *o, Msg msg)
{
	struct FileTransClassInstance *inst = INST_DATA(cl, o);
	struct msg_UpdateWindow *muw = (struct msg_UpdateWindow *) msg;
	STRPTR DestDirName;

	switch (muw->muw_UpdateMode)
		{
	case FTUPDATE_Initial:
		DoMethod(o, SCCM_FileTrans_OpenWindowDelayed);
		break;

	case FTUPDATE_EnterNewDir:
		DoMethod(o, SCCM_FileTrans_OpenWindowDelayed);

		DestDirName = AllocPathBuffer();

		if (DestDirName)
			{
			NameFromLock(muw->muw_DestDirLock, DestDirName, Max_PathLen);

			// show <from "fromdir" to "todir"> in lower text gadget
			ScaFormatStringMaxLength(inst->ftci_Line2Buffer, Max_PathLen,
				 "%s \"%s\" %s \"%s\"", 
				(ULONG) GetLocString(MSGID_FROMNAME),
				(ULONG) muw->muw_SrcName,
				(ULONG) GetLocString(MSGID_TONAME),
				(ULONG) DestDirName);

			d1(kprintf("%s/%s/%ld: Line2Buffer=<%s>\n", __FILE__, __FUNC__, __LINE__, inst->ftci_Line2Buffer));

			if (inst->ftci_Window)
				{
				GT_SetGadgetAttrs(inst->ftci_TextLine2Gadget, inst->ftci_Window, NULL,
					GTTX_Text, (ULONG) inst->ftci_Line2Buffer,
					TAG_END);

				FreePathBuffer(DestDirName);
				}
			}
		break;

	case FTUPDATE_Entry:
		DoMethod(o, SCCM_FileTrans_OpenWindowDelayed);
#ifdef SHOW_EVERY_OBJECT
		// show <Copying "filename"> in upper text gadget
		ScaFormatStringMaxLength(inst->ftci_Line1Buffer, Max_PathLen,
			 "%s \"%s\"", 
			GetLocString(FTOPCODE_Copy == inst->ftci_CurrentOperation ? MSGID_COPYINGNAME : MSGID_MOVINGNAME), 
			muw->muw_SrcName);

		if (inst->ftci_Window)
			{
			GT_SetGadgetAttrs(inst->ftci_TextLine1Gadget, inst->ftci_Window, NULL,
				GTTX_Text, inst->ftci_Line1Buffer,
				TAG_END);
			}
#endif /* SHOW_EVERY_OBJECT */
		break;

	case FTUPDATE_SameFile:
		DoMethod(o, SCCM_FileTrans_OpenWindowDelayed);
		break;

	case FTUPDATE_Final:
		// if no more operations are outstanding, force current to total bytecount.
		if (IsListEmpty(&inst->ftci_OpList))
			inst->ftci_CurrentBytes = inst->ftci_TotalBytes;
		break;
		}

	if (inst->ftci_Window)
		{
		ULONG64 PercentFinishedBytes;
		ULONG	PercentFinishedItems;
		ULONG	PercentFinished;

		if (inst->ftci_CountValid)
			{
			PercentFinishedBytes = Div64(Mul64(inst->ftci_CurrentBytes, Make64(100), NULL), inst->ftci_TotalBytes, NULL);
			PercentFinishedItems = (inst->ftci_CurrentItems * 100) / inst->ftci_TotalItems;
			}
		else
			{
			PercentFinishedBytes = Make64(100);
			PercentFinishedItems = 100;
			}

		PercentFinished = ((90 * ULONG64_LOW(PercentFinishedBytes)) + (10 * PercentFinishedItems)) / 100;

		if (PercentFinished > 100)
			PercentFinished = 100;

#ifdef SHOW_REMAINING_TIME
		d1(KPrintF("%s/%s/%ld: cItem=%lu  tItem=%lu  Current=%lu  Total=%lu  Percent=%ld\n", \
			__LINE__, inst->ftci_CurrentItems, inst->ftci_TotalItems, \
			ULONG64_LOW(inst->ftci_CurrentBytes), ULONG64_LOW(inst->ftci_TotalBytes), PercentFinished));

		if (GetElapsedTime(&inst->ftci_LastRemainTimeDisplay) >= 1000)
			{
			LONG RemainingTime;
			ULONG elapsedTime = GetElapsedTime(&inst->ftci_CopyStartTime);

			if (PercentFinished && elapsedTime)
				{
				ULONG TotalTime = (100 * elapsedTime) / PercentFinished;

				RemainingTime = TotalTime - elapsedTime;
				}
			else
				RemainingTime = 0;

			d1(kprintf("%s/%s/%ld: ElapsedTime=%lu cItem=%lu  tItem=%lu  Current=%lu  Total=%lu  Percent=%ld  remTime=%lu\n", \
				__LINE__, GetElapsedTime(&inst->ftci_CopyStartTime), \
				inst->ftci_CurrentItems, inst->ftci_TotalItems, \
				ULONG64_LOW(inst->ftci_CurrentBytes), ULONG64_LOW(inst->ftci_TotalBytes), \
				PercentFinished, RemainingTime/1000));

			d1(kprintf("%s/%s/%ld: Percent=%ld  RemainingTime=%ld s\n", \
				__LINE__, PercentFinished, RemainingTime/1000));

			if (PercentFinished > 3)
				{
				LONG RemainingSeconds = RemainingTime / 1000;
				LONG RemainingMinutes = RemainingSeconds / 60;

				if (RemainingMinutes > 1)
					{
					ScaFormatStringMaxLength(inst->ftci_Line3Buffer, sizeof(inst->ftci_Line3Buffer),
						"%s %ld %s", 
						(ULONG) GetLocString(MSGID_FILETRANSFER_REMAININGTIME),
						RemainingMinutes,
						(ULONG) GetLocString(MSGID_FILETRANSFER_MINUTES));
					}
				else
					{
					RemainingSeconds++;

					ScaFormatStringMaxLength(inst->ftci_Line3Buffer, sizeof(inst->ftci_Line3Buffer),
						"%s %ld %s", 
						(ULONG) GetLocString(MSGID_FILETRANSFER_REMAININGTIME),
						RemainingSeconds,
						(ULONG) GetLocString(RemainingSeconds > 1 ?
							MSGID_FILETRANSFER_SECONDS : MSGID_FILETRANSFER_SECOND));
					}
				}
			else
				{
				strcpy(inst->ftci_Line3Buffer, "");
				}

			GT_SetGadgetAttrs(inst->ftci_TextLine3Gadget, inst->ftci_Window, NULL,
				GTTX_Text, (ULONG) inst->ftci_Line3Buffer,
				TAG_END);

			GetSysTime(&inst->ftci_LastRemainTimeDisplay);
			}
#endif

		SetGadgetAttrs(inst->ftci_GaugeGadget, inst->ftci_Window, NULL,
			SCAGAUGE_Level, PercentFinished,
			TAG_END);
		}

	return 0;
}


// Get elapsed time since time <tv> in milliseconds.
static LONG GetElapsedTime(T_TIMEVAL *tv)
{
	T_TIMEVAL Now;
	LONG Diff;

	GetSysTime(&Now);
	SubTime(&Now, tv);

	Diff = 1000 * Now.tv_secs + Now.tv_micro / 1000;

	d1(kprintf("%s/%s/%ld: Now s=%ld  ms=%ld   Start s=%ld  ms=%ld  Diff=%ld\n", \
		__LINE__, Now.tv_secs, Now.tv_micro, \
			tv->tv_secs, tv->tv_micro, Diff));

	return Diff;
}


static ULONG FileTransClass_CheckCountTimeout(Class *cl, Object *o, Msg msg)
{
	struct FileTransClassInstance *inst = INST_DATA(cl, o);
	LONG Diff = GetElapsedTime(&inst->ftci_ProcessStartTime);

	return (ULONG) (Diff > CurrentPrefs.pref_FileCountTimeout);
}


static ULONG FileTransClass_OverwriteRequest(Class *cl, Object *o, Msg msg)
{
	struct msg_OverwriteRequest *mov = (struct msg_OverwriteRequest *) msg;
	enum ExistsReqResult result;

	result = UseRequestArgs(mov->mov_ParentWindow,
		mov->mov_SuggestedBodyTextId, 
		mov->mov_SuggestedGadgetTextId, 
		1,
		mov->mov_DestName);

	return (ULONG) result;
}


static ULONG FileTransClass_WriteProtectedRequest(Class *cl, Object *o, Msg msg)
{
	struct msg_WriteProtectedRequest *mwp = (struct msg_WriteProtectedRequest *) msg;
	enum ExistsReqResult result;
	char ErrorText[100];

	Fault(mwp->mwp_ErrorCode, (STRPTR) "", ErrorText, sizeof(ErrorText));

	result = UseRequestArgs(mwp->mwp_ParentWindow,
		mwp->mwp_SuggestedBodyTextId,
		mwp->mwp_SuggestedGadgetTextId,
		2,
		mwp->mwp_DestName,
                ErrorText
                );

	return (ULONG) result;
}

