// Splash.c
// $Date$
// $Revision$


#include <exec/types.h>
#include <devices/timer.h>
#include <graphics/gels.h>
#include <graphics/rastport.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <utility/hooks.h>
#include <graphics/gfxmacros.h>
#include <intuition/gadgetclass.h>
#include <hardware/blit.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <datatypes/pictureclass.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/exall.h>

#define	__USE_SYSBASE

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/layers.h>
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
#include <scalos/scalos.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "scalos_structures.h"
#include "functions.h"
#include "locale.h"
#include "Variables.h"

//----------------------------------------------------------------------------

// select relatively high priority for splash process to ensure
// fast reply to messages
#define SPLASH_PROCESS_TASK_PRI	3

#define	SPLASH_MSG_HOLD_TICKS	3		// minimum time for a message to stay visible (in ticks, 50 per seconds)

struct SM_UpdateSplash
	{
	struct ScalosMessage    ScalosMessage;
	ULONG	smu_Command;
	char	smu_String[1];		// String to display - variable length !!!
	};

struct SplashTimer
	{
	struct MsgPort *spt_ioPort;
	T_TIMEREQUEST *spt_TimeReq;
	BOOL spt_timerOpen;
	BOOL spt_timerPending;		// TRUE when spt_TimeReq is in use
	};

enum SplashCommands { SPLCMD_Update, SPLCMD_Close, SPLCMD_AddUser, SPLCMD_RemUser };

//----------------------------------------------------------------------------

// local functions

static BOOL SendSplashMsg(ULONG Command, CONST_STRPTR Text);
static ULONG SplashProcess(APTR Dummy, struct SM_RunProcess *msg);
static void UpdateSplash(CONST_STRPTR text, BOOL ForceRedraw);
static BOOL OpenSplash(WORD iWidth, WORD iHeight);
static void CloseSplash(void);
static SAVEDS(ULONG) BackFillFunc(struct Hook *bfHook, struct RastPort *rp, struct BackFillMsg *msg);
static BOOL InitSplashTimer(struct SplashTimer *Timer);
static void CleanupSplashTimer(struct SplashTimer *Timer);
static BOOL ReadSplashLogo(struct DatatypesImage **Logo);
static void RenderSplashLogo(void);

//----------------------------------------------------------------------------

// local Data

static BOOL fInitialized = FALSE;
static SCALOSSEMAPHORE SplashSema;
struct MsgPort *SplashPort;
static struct Window *SplashWindow = NULL;
static ULONG SplashWindowMask = 0l;
static STRPTR LastSplashText = NULL;
static struct DatatypesImage *SplashLogo;
static struct DatatypesImage *SplashBackground;
static struct Hook BackFillHook =
	{
	{ NULL },
	HOOKFUNC_DEF(BackFillFunc),
	&SplashWindow
	};

//----------------------------------------------------------------------------


// returns : 
//	RETURN_OK if no error or dos error code

ULONG InitSplash(struct MsgPort *ReplyPort)
{
///
	ScalosInitSemaphore(&SplashSema);

	d1(kprintf("%s/%s/%ld: ReplyPort=%08lx\n", __FILE__, __FUNC__, __LINE__, ReplyPort));

	if (CurrentPrefs.pref_EnableSplash)
		{
		d1(KPrintF("%s/%s/%ld: RunProcess(SplashProcess)\n", __FILE__, __FUNC__, __LINE__));
		if (!RunProcess(&MainWindowTask->mwt.iwt_WindowTask, SplashProcess, 0, NULL, ReplyPort))
			return ERROR_NO_FREE_STORE;

		fInitialized = TRUE;
		}

	return RETURN_OK;
///
}


void SplashAddUser(void)
{
	d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
	SendSplashMsg(SPLCMD_AddUser, NULL);
}


void SplashRemoveUser(void)
{
	d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
	SendSplashMsg(SPLCMD_RemUser, NULL);
}


void SplashDisplayProgress(const char *fmt, ULONG NumArgs, ...)
{
///
	char text[512];
	va_list args;
	ULONG *ArgList = NULL;
	BOOL SemaLocked = FALSE;

	d1(kprintf("%s/%s/%ld: fmt=<%s>\n", __FILE__, __FUNC__, __LINE__, fmt));

	do	{
		ULONG *pArg;

		if (!fInitialized)
			break;

		if (!CurrentPrefs.pref_EnableSplash)
			break;

		ScalosObtainSemaphore(&SplashSema);
		SemaLocked = TRUE;

		if (NULL == SplashPort || NULL == iInfos.xii_iinfos.ii_Screen)
			break;

		va_start(args, NumArgs);

		ArgList = ScalosAllocVecPooled(1 + NumArgs * sizeof(ULONG));
		if (NULL == ArgList)
			break;

		pArg = ArgList;
		while (NumArgs--)
			*pArg++ = va_arg(args, ULONG);

		if (strlen(fmt) > 0)
			ScaFormatStringArgs(text, sizeof(text), fmt, ArgList);
		else
			strcpy(text, "");

		va_end(args);

		d1(KPrintF("%s/%s/%ld: text=<%s>\n", __FILE__, __FUNC__, __LINE__, text));

		if (strlen(text) > 0)
			SendSplashMsg(SPLCMD_Update, text);
		else
			SendSplashMsg(SPLCMD_Close, NULL);
		} while (0);

	if (ArgList)
		ScalosFreeVecPooled(ArgList);
	if (SemaLocked)
		ScalosReleaseSemaphore(&SplashSema);
///
}


static BOOL SendSplashMsg(ULONG Command, CONST_STRPTR Text)
{
///
	struct SM_UpdateSplash *msg;
	BOOL Result = FALSE;
	size_t Len = sizeof(struct SM_UpdateSplash) - sizeof(struct SM_CloseWindow);

	d1(KPrintF("%s/%s/%ld: Task=%08lx  SplashPort=%08lx  Cmd=%ld\n", __FILE__, __FUNC__, __LINE__, FindTask(NULL), SplashPort, Command));

	if (!fInitialized || NULL == SplashPort)
		return FALSE;

	if (Text)
		Len += strlen(Text);

	msg = (struct SM_UpdateSplash *) SCA_AllocMessage(MTYP_CloseWindow, Len);

	if (msg)
		{
		struct MsgPort *SplashReplyPort;
		struct SM_UpdateSplash *rMsg;

		SplashReplyPort = CreateMsgPort();
		d1(KPrintF("%s/%s/%ld: SplashReplyPort=%08lx  msg=%08lx\n", __FILE__, __FUNC__, __LINE__, SplashReplyPort, msg));
		if (SplashReplyPort)
			{
			ULONG WaitMask = 1 << SplashReplyPort->mp_SigBit;
			ULONG SigsReceived;

			msg->smu_Command = Command;

			if (Text)
				strcpy(msg->smu_String, Text);

			msg->ScalosMessage.sm_Message.mn_ReplyPort = SplashReplyPort;
			PutMsg(SplashPort, (struct Message *) msg);

			d1(KPrintF("%s/%s/%ld: Task=%08lx  SplashReplyPort=%08lx  msg=%08lx\n", __FILE__, __FUNC__, __LINE__, FindTask(NULL), SplashReplyPort, msg));

			// Wait for reply to make sure update is complete
			SigsReceived = Wait(WaitMask);

			if (SigsReceived & WaitMask)
				{
				while ((rMsg = (struct SM_UpdateSplash *) GetMsg(SplashReplyPort)))
					{
					d1(KPrintF("%s/%s/%ld: rMsg=%08lx\n", __FILE__, __FUNC__, __LINE__, rMsg));
					SCA_FreeMessage((struct ScalosMessage *) rMsg);
					}
				}

			d1(KPrintF("%s/%s/%ld: Task=%08lx\n", __FILE__, __FUNC__, __LINE__, FindTask(NULL)));

			DeleteMsgPort(SplashReplyPort);
			}
		else
			{
			SCA_FreeMessage((struct ScalosMessage *) msg);
			}

		Result = TRUE;
		}

	d1(KPrintF("%s/%s/%ld: Task=%08lx\n", __FILE__, __FUNC__, __LINE__, FindTask(NULL)));

	return Result;
///
}


static ULONG SplashProcess(APTR Dummy, struct SM_RunProcess *msg)
{
///
	struct SplashTimer CloseTimer;

	memset(&CloseTimer, 0, sizeof(CloseTimer));

	do	{
		BOOL Running = TRUE;
		BOOL Closed = FALSE;
		LONG UserCount = 0;
		ULONG SplashMask, ioMask;

		SetTaskPri(FindTask(NULL), SPLASH_PROCESS_TASK_PRI);

		SplashPort = CreateMsgPort();
		if (NULL == SplashPort)
			break;

		d1(kprintf("%s/%s/%ld: SplashPort=%08lx\n", __FILE__, __FUNC__, __LINE__, SplashPort));

		if (!InitSplashTimer(&CloseTimer))
			break;

		d1(kprintf("%s/%s/%ld: SplashPort=%08lx\n", __FILE__, __FUNC__, __LINE__, SplashPort));

		SplashMask = 1 << SplashPort->mp_SigBit;
		ioMask = 1 << CloseTimer.spt_ioPort->mp_SigBit;

		d1(kprintf("%s/%s/%ld: SplashMask=%08lx  ioMask=%08lx\n", __FILE__, __FUNC__, __LINE__, SplashMask, ioMask));

		do	{
			struct SM_UpdateSplash *msg;
			ULONG RcvdMask;

			d1(kprintf("%s/%s/%ld: Mask=%08lx\n", __FILE__, __FUNC__, __LINE__, SplashMask | ioMask));

			RcvdMask = Wait(SplashMask | ioMask | SplashWindowMask);

			d1(kprintf("%s/%s/%ld: RcvdMask=%08lx\n", __FILE__, __FUNC__, __LINE__, RcvdMask));

			if (RcvdMask & SplashMask)
				{
				while ((msg = (struct SM_UpdateSplash *) GetMsg(SplashPort)))
					{
					d1(kprintf("%s/%s/%ld: msg=%08lx  Cmd=%ld  timerPending=%ld\n", __FILE__, __FUNC__, __LINE__, msg, msg->smu_Command, CloseTimer.spt_timerPending));

					if (CloseTimer.spt_timerPending)
						{
						d1(kprintf("%s/%s/%ld: RcvdMask=%08lx\n", __FILE__, __FUNC__, __LINE__, RcvdMask));

						AbortIO((struct IORequest *) CloseTimer.spt_TimeReq);
						WaitIO((struct IORequest *) CloseTimer.spt_TimeReq);
						CloseTimer.spt_timerPending = FALSE;
						RcvdMask &= ~ioMask;
						}

					switch (msg->smu_Command)
						{
					case SPLCMD_AddUser:
						d1(kprintf("%s/%s/%ld: SPLCMD_AddUser UserCount=%ld  timerPending=%ld\n", __FILE__, __FUNC__, __LINE__, UserCount, CloseTimer.spt_timerPending));
						UserCount++;
						break;

					case SPLCMD_RemUser:
						d1(kprintf("%s/%s/%ld: SPLCMD_RemUser UserCount=%ld  timerPending=%ld\n", __FILE__, __FUNC__, __LINE__, UserCount, CloseTimer.spt_timerPending));
						UserCount--;
						if (UserCount <= 0)
							{
							CloseTimer.spt_TimeReq->tr_node.io_Command = TR_ADDREQUEST;
							CloseTimer.spt_TimeReq->tr_time.tv_micro = 0;
							CloseTimer.spt_TimeReq->tr_time.tv_secs = CurrentPrefs.pref_SplashHoldTime;

							BeginIO((struct IORequest *) CloseTimer.spt_TimeReq);
							CloseTimer.spt_timerPending = TRUE;
							}
						break;

					case SPLCMD_Update:
						d1(kprintf("%s/%s/%ld: SPLCMD_Update UserCount=%ld  timerPending=%ld\n", __FILE__, __FUNC__, __LINE__, UserCount, CloseTimer.spt_timerPending));
						if (UserCount <= 0)
							{
							CloseTimer.spt_TimeReq->tr_time.tv_micro = 0;
							CloseTimer.spt_TimeReq->tr_time.tv_secs = CurrentPrefs.pref_SplashHoldTime;

							BeginIO((struct IORequest *) CloseTimer.spt_TimeReq);
							CloseTimer.spt_timerPending = TRUE;
							}

						if (!Closed)
							UpdateSplash(msg->smu_String, FALSE);

						Delay(SPLASH_MSG_HOLD_TICKS);
						break;

					case SPLCMD_Close:
						d1(kprintf("%s/%s/%ld: SPLCMD_Close UserCount=%ld  timerPending=%ld\n", __FILE__, __FUNC__, __LINE__, UserCount, CloseTimer.spt_timerPending));
						Running = FALSE;
						break;
						}

					d1(kprintf("%s/%s/%ld: replying msg=%08lx  ReplyPort=%08lx\n", __FILE__, __FUNC__, __LINE__, \
						msg, msg->ScalosMessage.sm_Message.mn_ReplyPort));

					ReplyMsg((struct Message *) msg);
					}

				d1(kprintf("%s/%s/%ld: Running=%ld\n", __FILE__, __FUNC__, __LINE__, Running));
				}
			if (RcvdMask & ioMask)
				{
				d1(kprintf("%s/%s/%ld: UserCount=%ld  timerPending=%ld\n", __FILE__, __FUNC__, __LINE__, UserCount, CloseTimer.spt_timerPending));
				if (CloseTimer.spt_timerPending)
					{
					WaitIO((struct IORequest *) CloseTimer.spt_TimeReq);
					CloseTimer.spt_timerPending = FALSE;

					Running = FALSE;
					}
				}
			if (RcvdMask & SplashWindowMask)
				{
				struct IntuiMessage *iMsg;

				d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

				while (SplashWindow && (iMsg = (struct IntuiMessage *) GetMsg(SplashWindow->UserPort)))
					{
					ULONG mClass = iMsg->Class;
//					UWORD mCode = iMsg->Code;

					ReplyMsg((struct Message *) iMsg);

					d1(KPrintF("%s/%s/%ld: mClass=%08lx\n", __FILE__, __FUNC__, __LINE__, mClass));

					switch (mClass)
						{
					case IDCMP_MOUSEBUTTONS:
						Closed = TRUE;
						CloseSplash();
						break;

					case IDCMP_CHANGEWINDOW:
						d1(KPrintF("%s/%s/%ld: IDCMP_CHANGEWINDOW\n", __FILE__, __FUNC__, __LINE__));
						EraseRect(SplashWindow->RPort,
							SplashWindow->BorderLeft,
							SplashWindow->BorderTop,
							SplashWindow->Width - SplashWindow->BorderRight - 1,
							SplashWindow->Height - SplashWindow->BorderBottom - 1
							);

						RenderSplashLogo();
						if (LastSplashText)
							UpdateSplash(LastSplashText, TRUE);
						else
							UpdateSplash("", TRUE);
						break;
					default:
						break;
						}

					d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
					}
				}

			} while (Running);

		d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

		ScalosObtainSemaphore(&SplashSema);

		d1(KPrintF("%s/%s/%ld: after ScalosObtainSemaphore\n", __FILE__, __FUNC__, __LINE__));

		CloseSplash();

		d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

		DeleteMsgPort(SplashPort);
		SplashPort = NULL;

		ScalosReleaseSemaphore(&SplashSema);
		} while (0);

	d1(kprintf("%s/%s/%ld: SplashPort=%08lx\n", __FILE__, __FUNC__, __LINE__, SplashPort));

	CleanupSplashTimer(&CloseTimer);

	if (SplashPort)
		{
		DeleteMsgPort(SplashPort);
		SplashPort = NULL;
		}
		
	return 0;
///
}


static void UpdateSplash(CONST_STRPTR text, BOOL ForceRedraw)
{
///
	d1(kprintf("%s/%s/%ld: text=<%s>\n", __FILE__, __FUNC__, __LINE__, text));

	if (iInfos.xii_iinfos.ii_Screen)
		{
		struct RastPort rp;
		char VersionText[256];
		struct TextExtent tExtT, tExtV, tExtS;
		short y, iTop, iWidth, iHeight;
		BOOL NeedDrawText = ForceRedraw;

		d1(kprintf("%s/%s/%ld: text=<%s>\n", __FILE__, __FUNC__, __LINE__, text));

		ScaFormatStringMaxLength(VersionText, sizeof(VersionText),
			GetLocString(MSGID_PROGRESS_SCALOSVERSION), 
			ScalosBase->scb_LibNode.lib_Version,
			ScalosBase->scb_LibNode.lib_Revision,
			(ULONG) ScalosBuildNr);

		rp = iInfos.xii_iinfos.ii_Screen->RastPort;

		Scalos_SetFont(&rp, iInfos.xii_iinfos.ii_Screen->RastPort.Font, &ScreenTTFont);

		Scalos_SetSoftStyle(&rp, FSF_BOLD, FSF_BOLD, &ScreenTTFont);
		Scalos_TextExtent(&rp, VersionText, strlen(VersionText), &tExtV);

		Scalos_SetSoftStyle(&rp, FS_NORMAL, FSF_BOLD, &ScreenTTFont);
		Scalos_TextExtent(&rp, COMPILER_STRING, strlen(COMPILER_STRING), &tExtS);

		Scalos_SetSoftStyle(&rp, FS_NORMAL, FSF_BOLD, &ScreenTTFont);
		Scalos_TextExtent(&rp, text, strlen(text), &tExtT);

		iWidth = max(tExtT.te_Width, max(tExtS.te_Width, tExtV.te_Width)) + 2 * tExtT.te_Height;
		if (iWidth < iInfos.xii_iinfos.ii_Screen->Width/10)
			iWidth = iInfos.xii_iinfos.ii_Screen->Width/10;
		iHeight = tExtT.te_Height * 5;

		d1(kprintf("%s/%s/%ld: iWidth=%ld  iHeight=%ld\n", __FILE__, __FUNC__, __LINE__, iWidth, iHeight));

		if (SplashLogo)
			{
			if (iWidth < SplashLogo->dti_BitMapHeader->bmh_Width + 2*5)
				iWidth = SplashLogo->dti_BitMapHeader->bmh_Width + 2*5;
			iHeight += SplashLogo->dti_BitMapHeader->bmh_Height + 2*5;

			d1(kprintf("%s/%s/%ld: iWidth=%ld  iHeight=%ld\n", __FILE__, __FUNC__, __LINE__, iWidth, iHeight));
			}

		if (SplashWindow)
			{
			WORD jWidth = SplashWindow->Width - SplashWindow->BorderLeft - SplashWindow->BorderRight;
			WORD jHeight = SplashWindow->Height - SplashWindow->BorderTop - SplashWindow->BorderBottom;

			if (jWidth < iWidth || jHeight < iHeight)
				{
				WORD NewWidth = SplashWindow->BorderLeft + SplashWindow->BorderRight + iWidth;
				WORD NewHeight = SplashWindow->BorderTop + SplashWindow->BorderBottom + iHeight;

				WindowToFront(SplashWindow);

				if (NewWidth > SplashWindow->Width || NewHeight > SplashWindow->Height)
					{
					WORD NewLeftEdge = (iInfos.xii_iinfos.ii_Screen->Width - NewWidth) / 2;
					WORD NewTopEdge = (iInfos.xii_iinfos.ii_Screen->Height - NewHeight) / 2;

					ChangeWindowBox(SplashWindow,
						NewLeftEdge, NewTopEdge,
						NewWidth, NewHeight
						);

					NeedDrawText = TRUE;
					}
				}
			else
				{
				iWidth = jWidth;
				iHeight = jHeight;
				}

			d1(kprintf("%s/%s/%ld: iWidth=%ld  iHeight=%ld\n", __FILE__, __FUNC__, __LINE__, iWidth, iHeight));
			}
		else
			{
			if (!OpenSplash(iWidth, iHeight))
				return;

			d1(kprintf("%s/%s/%ld: iWidth=%ld  iHeight=%ld\n", __FILE__, __FUNC__, __LINE__, iWidth, iHeight));

			if (SplashLogo)
				{
				if (iWidth < SplashLogo->dti_BitMapHeader->bmh_Width + 2*5)
					iWidth = SplashLogo->dti_BitMapHeader->bmh_Width + 2*5;
				iHeight += SplashLogo->dti_BitMapHeader->bmh_Height + 2*5;

				d1(kprintf("%s/%s/%ld: iWidth=%ld  iHeight=%ld\n", __FILE__, __FUNC__, __LINE__, iWidth, iHeight));
				}

			NeedDrawText = TRUE;
			}

		iTop = SplashWindow->BorderTop;

		if (SplashLogo)
			{
			iTop += SplashLogo->dti_BitMapHeader->bmh_Height + 2*5;
			}

		d1(kprintf("%s/%s/%ld: MinY=%ld  MaxY=%ld\n", __FILE__, __FUNC__, __LINE__, tExtT.te_Extent.MinY, tExtT.te_Extent.MaxY));

		y = iTop + ((iHeight - iTop) - tExtV.te_Height) / 2
			- tExtV.te_Extent.MinY + tExtV.te_Extent.MaxY - 2 * tExtV.te_Height;

		if (NeedDrawText)
			{
			EraseRect(SplashWindow->RPort,
				SplashWindow->BorderLeft,
				iTop,
				SplashWindow->Width - SplashWindow->BorderRight - 1,
				SplashWindow->Height - SplashWindow->BorderBottom - 1
				);

			Move(SplashWindow->RPort, (iWidth - tExtV.te_Width) / 2, y);
			Scalos_SetSoftStyle(SplashWindow->RPort, FSF_BOLD, FSF_BOLD, &ScreenTTFont);
			Scalos_Text(SplashWindow->RPort, VersionText, strlen(VersionText));

			y += (3 * tExtV.te_Height) / 2;

			Move(SplashWindow->RPort, (iWidth - tExtS.te_Width) / 2, y);
			Scalos_SetSoftStyle(SplashWindow->RPort, FS_NORMAL, FSF_BOLD, &ScreenTTFont);
			Scalos_Text(SplashWindow->RPort, COMPILER_STRING, strlen(COMPILER_STRING));
			}
		else
			{
			y += (3 * tExtV.te_Height) / 2;
			}


		y += (3 * tExtS.te_Height) / 2;

#if 0
		{
		SHORT y1 = y + tExtT.te_Extent.MinY;

		Move(SplashWindow->RPort, SplashWindow->BorderLeft, y1);
		Draw(SplashWindow->RPort, SplashWindow->Width - SplashWindow->BorderRight - 1, y1);
		Draw(SplashWindow->RPort, SplashWindow->Width - SplashWindow->BorderRight - 1, y1 + tExtT.te_Height);
		Draw(SplashWindow->RPort, SplashWindow->BorderLeft, y1 + tExtT.te_Height);
		Draw(SplashWindow->RPort, SplashWindow->BorderLeft, y1);
		}
#endif

		EraseRect(SplashWindow->RPort, 
			SplashWindow->BorderLeft, 
			y + tExtT.te_Extent.MinY,
			SplashWindow->Width - SplashWindow->BorderRight - 1, 
			y + tExtT.te_Extent.MinY + tExtT.te_Height);

		Move(SplashWindow->RPort, (iWidth - tExtT.te_Width) / 2, y);
		Scalos_SetSoftStyle(SplashWindow->RPort, FS_NORMAL, FSF_BOLD, &ScreenTTFont);
		Scalos_Text(SplashWindow->RPort, text, strlen(text));

		if (LastSplashText)
			ScalosFreeVecPooled(LastSplashText);

		LastSplashText = ScalosAllocVecPooled(strlen(text) + 1);
		if (LastSplashText)
			strcpy(LastSplashText, text);
		}
///
}


static BOOL OpenSplash(WORD iWidth, WORD iHeight)
{
///
	d1(KPrintF("%s/%s/%ld: Width=%ld  Height=%ld\n", __FILE__, __FUNC__, __LINE__, iWidth, iHeight));
	d1(KPrintF("%s/%s/%ld: PubScreen=%08lx\n", __FILE__, __FUNC__, __LINE__, iInfos.xii_iinfos.ii_Screen));

	if (ReadSplashLogo(&SplashLogo))
		{
		if (iWidth < SplashLogo->dti_BitMapHeader->bmh_Width + 2*5)
			iWidth = SplashLogo->dti_BitMapHeader->bmh_Width + 2*5;
		iHeight += SplashLogo->dti_BitMapHeader->bmh_Height + 2*5;
		}
	SplashBackground = CreateDatatypesImage("THEME:SplashBackground", 0);

	SplashWindow = LockedOpenWindowTags(NULL,
		WA_Left, (iInfos.xii_iinfos.ii_Screen->Width - iWidth) / 2,
		WA_Top, (iInfos.xii_iinfos.ii_Screen->Height - iHeight) / 2,
		WA_Activate, FALSE,
		WA_InnerWidth, iWidth,
		WA_InnerHeight, iHeight,
		WA_SizeGadget, FALSE,
		WA_DragBar, FALSE,
		WA_DepthGadget, FALSE,
		WA_Borderless, TRUE,
		WA_SmartRefresh, TRUE,
		WA_NewLookMenus, TRUE,
		WA_BackFill, &BackFillHook,
#if defined(WA_FrontWindow)
		WA_FrontWindow, TRUE,
#elif defined(WA_StayTop)
		WA_StayTop, TRUE,
#endif /* WA_FrontWindow */
#if defined(__MORPHOS__) && defined(WA_Opacity)
		WA_Opacity, 0x0,
#elif defined(__amigaos4__) && defined(WA_Opaqueness)
		WA_Opaqueness, 0,
#endif //defined(__amigaos4__) && defined(WA_Opaqueness)
		WA_IDCMP, IDCMP_CHANGEWINDOW | IDCMP_MOUSEBUTTONS,
		WA_PubScreen, iInfos.xii_iinfos.ii_Screen,
		TAG_END);

	d1(kprintf("%s/%s/%ld: SplashWindow=%08lx\n", __FILE__, __FUNC__, __LINE__, SplashWindow));

	if (NULL == SplashWindow)
		{
		return FALSE;
		}

	SplashWindowMask = 1 << SplashWindow->UserPort->mp_SigBit;

	Scalos_SetFont(SplashWindow->RPort, iInfos.xii_iinfos.ii_Screen->RastPort.Font, &ScreenTTFont);
	SetAPen(SplashWindow->RPort, 1);
	SetDrMd(SplashWindow->RPort, JAM1);

	RenderSplashLogo();

	WindowFadeIn(SplashWindow);

	return TRUE;
///
}


static void CloseSplash(void)
{
///
	d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
	if (SplashWindow)
		{
		WindowFadeOut(SplashWindow);
		SplashWindowMask = 0L;
		LockedCloseWindow(SplashWindow);
		SplashWindow = NULL;
		}

	d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
	DisposeDatatypesImage(&SplashLogo);

	d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
	DisposeDatatypesImage(&SplashBackground);

	d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

	if (LastSplashText)
		{
		ScalosFreeVecPooled(LastSplashText);
		LastSplashText = NULL;
		}
///
}

// object == (struct RastPort *) result->RastPort
// message == [ (Layer *) layer, (struct Rectangle) bounds,
//              (LONG) offsetx, (LONG) offsety ]

static SAVEDS(ULONG) BackFillFunc(struct Hook *bfHook, struct RastPort *rp, struct BackFillMsg *msg)
{
///
	struct RastPort rpCopy;

	d1(kprintf("%s/%s/%ld: RastPort=%08lx\n", __FILE__, __FUNC__, __LINE__, rp));
	d1(kprintf("%s/%s/%ld: Rect=%ld %ld %ld %ld\n", __FILE__, __FUNC__, __LINE__, \
		msg->bfm_Rect.MinX, msg->bfm_Rect.MinY, msg->bfm_Rect.MaxX, msg->bfm_Rect.MaxY));
	d1(kprintf("%s/%s/%ld: Layer=%08lx  OffsetX=%ld  OffsetY=%ld\n", __FILE__, __FUNC__, __LINE__, \
		msg->bfm_Layer, msg->bfm_OffsetX, msg->bfm_OffsetY));

	rpCopy = *rp;
	rpCopy.Layer = NULL;

	if (SplashBackground)
		{
		WindowBackFill(&rpCopy, msg, SplashBackground->dti_BitMap,
			SplashBackground->dti_BitMapHeader->bmh_Width,
			SplashBackground->dti_BitMapHeader->bmh_Height,
			iInfos.xii_iinfos.ii_DrawInfo->dri_Pens[BACKGROUNDPEN],
			0, 0,
			NULL);
		}
	else
		{
		WindowBackFill(&rpCopy, msg, NULL,
			0, 0,
			iInfos.xii_iinfos.ii_DrawInfo->dri_Pens[BACKGROUNDPEN],
			0, 0,
			NULL);
		}

	return 0;
///
}


static BOOL InitSplashTimer(struct SplashTimer *Timer)
{
///
	BYTE odError;

	memset(Timer, 0, sizeof(struct SplashTimer));

	Timer->spt_ioPort = CreateMsgPort();
	if (NULL == Timer->spt_ioPort)
		return FALSE;

	d1(kprintf("%s/%s/%ld: ioPort=%08lx\n", __FILE__, __FUNC__, __LINE__, Timer->spt_ioPort));

	Timer->spt_TimeReq = (T_TIMEREQUEST *)CreateIORequest(Timer->spt_ioPort, sizeof(T_TIMEREQUEST));
	if (NULL == Timer->spt_TimeReq)
		return FALSE;

	d1(kprintf("%s/%s/%ld: TimeReq=%08lx\n", __FILE__, __FUNC__, __LINE__, Timer->spt_TimeReq));

	odError = OpenDevice(TIMERNAME, UNIT_VBLANK, &Timer->spt_TimeReq->tr_node, 0);
	d1(kprintf("%s/%s/%ld: OpenDevice returned %ld\n", __FILE__, __FUNC__, __LINE__, odError));
	if (0 != odError)
		return FALSE;

	Timer->spt_timerOpen = TRUE;

	return TRUE;
///
}


static void CleanupSplashTimer(struct SplashTimer *Timer)
{
///
	d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
	if (Timer->spt_TimeReq)
		{
		if (Timer->spt_timerPending)
			{
			AbortIO((struct IORequest *) Timer->spt_TimeReq);
			WaitIO((struct IORequest *) Timer->spt_TimeReq);

			Timer->spt_timerPending = FALSE;
			}
		d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		if (Timer->spt_timerOpen)
			{
			CloseDevice(&Timer->spt_TimeReq->tr_node);
			Timer->spt_timerOpen = FALSE;
			}

		d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		DeleteIORequest((struct IORequest *) Timer->spt_TimeReq);
		Timer->spt_TimeReq = NULL;
		}
	d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
	if (Timer->spt_ioPort)
		{
		DeleteMsgPort(Timer->spt_ioPort);
		Timer->spt_ioPort = NULL;
		}
	d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
///
}


static BOOL ReadSplashLogo(struct DatatypesImage **Logo)
{
///
	*Logo  = CreateDatatypesImage("THEME:ScalosSplashLogo", 0);
	if (NULL == *Logo)
		*Logo = CreateDatatypesImage("THEME:ScalosLogo", 0);

	return (BOOL) (NULL != *Logo);
///
}



static void RenderSplashLogo(void)
{
///
	if (SplashLogo)
		{
		WORD x0, y0;
		struct RastPort rp;

		Scalos_InitRastPort(&rp);

		rp.BitMap = SplashLogo->dti_BitMap;

		y0 = SplashWindow->BorderTop + 5;
		x0 = (SplashWindow->Width - SplashLogo->dti_BitMapHeader->bmh_Width) / 2;

		DtImageDraw(SplashLogo,
			SplashWindow->RPort,
			x0,
			y0,
			SplashLogo->dti_BitMapHeader->bmh_Width,
			SplashLogo->dti_BitMapHeader->bmh_Height
			);

		Scalos_DoneRastPort(&rp);
		}
///
}


