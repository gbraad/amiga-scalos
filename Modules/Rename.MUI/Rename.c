// Rename.c
// $Date$
// $Revision$


#include <exec/types.h>
#include <exec/alerts.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/classusr.h>
#include <libraries/mui.h>
#include <mui/BetterString_mcc.h>
#include <workbench/startup.h>

#include <clib/alib_protos.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <proto/utility.h>
#include <proto/muimaster.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <defs.h>
#include <Year.h> // +jmc+

#include "Rename.h"

#define	CATCOMP_NUMBERS
#define	CATCOMP_BLOCK
#define	CATCOMP_CODE
#include "Rename_Locale.h"

//----------------------------------------------------------------------------

#define	VERSION_MAJOR	40
#define	VERSION_MINOR	3

//----------------------------------------------------------------------------

#define	Application_Return_Ok	1001
#define	Application_Return_Skip	1002

//----------------------------------------------------------------------------

#define KeyButtonHelp(name,key,HelpText)\
	TextObject,\
		ButtonFrame,\
		MUIA_Font, MUIV_Font_Button,\
		MUIA_Text_Contents, name,\
		MUIA_Text_PreParse, "\33c",\
		MUIA_Text_HiChar  , key,\
		MUIA_ControlChar  , key,\
		MUIA_InputMode    , MUIV_InputMode_RelVerify,\
		MUIA_Background   , MUII_ButtonBack,\
		MUIA_ShortHelp    , HelpText,\
		MUIA_CycleChain   , TRUE,\
		End

//----------------------------------------------------------------------------

static BOOL OpenLibraries(void);
static void CloseLibraries(void);
static void UpdateGadgets(struct WBArg *arg);
static void ReportError(LONG Result, CONST_STRPTR OldObjName, CONST_STRPTR NewObjName);
static LONG RenameObject(struct WBArg *arg);
void StripTrailingColon(STRPTR Line);
static SAVEDS(APTR) INTERRUPT OpenAboutMUIHookFunc(struct Hook *hook, Object *o, Msg msg);
static SAVEDS(void) INTERRUPT OpenAboutHookFunc(struct Hook *hook, Object *o, Msg msg);
static STRPTR GetLocString(ULONG MsgId);
static BOOL CheckInfoData(const struct InfoData *info);
static BOOL isDiskWritable(BPTR dLock);
static BOOL CheckMCCVersion(CONST_STRPTR name, ULONG minver, ULONG minrev);
static BOOL IfLocateName(STRPTR FileName);

//----------------------------------------------------------------------------

#if !defined(__amigaos4__) && !defined(__AROS__)
#include <dos.h>

long _stack = 16384;		// minimum stack size, used by SAS/C startup code
#endif

//----------------------------------------------------------------------------

extern struct ExecBase *SysBase;
struct IntuitionBase *IntuitionBase = NULL;
T_LOCALEBASE LocaleBase = NULL;
#ifndef __amigaos4__
T_UTILITYBASE UtilityBase = NULL;
#endif
struct Library *MUIMasterBase = NULL;

#ifdef __amigaos4__
struct IntuitionIFace *IIntuition = NULL;
struct LocaleIFace *ILocale = NULL;
struct MUIMasterIFace *IMUIMaster = NULL;
#endif

static struct Catalog       *gb_Catalog;


static BOOL RenameVolume = FALSE;

static struct Hook AboutHook = { { NULL, NULL }, HOOKFUNC_DEF(OpenAboutHookFunc), NULL };
static struct Hook AboutMUIHook = { { NULL, NULL }, HOOKFUNC_DEF(OpenAboutMUIHookFunc), NULL };

//----------------------------------------------------------------------------

static Object *Group_Buttons2;
static Object *APP_Main;
static Object *WIN_Main;
static Object *WIN_AboutMUI;
static Object *OkButton, *SkipButton, *CancelButton;
static Object *MenuAbout, *MenuAboutMUI, *MenuQuit;
static Object *TextEnterName;
static Object *TextPath;
static Object *StringNewName;

//----------------------------------------------------------------------------

const int Max_PathLen = 1000;

//----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	LONG win_opened;
	ULONG n;
	struct WBStartup *WBenchMsg =
		(argc == 0) ? (struct WBStartup *)argv : NULL;

#if 0
	if (WBenchMsg)
		{
		LONG n;

		kprintf("NumArgs=%ld\n", WBenchMsg->sm_NumArgs);

		for (n=0; n<WBenchMsg->sm_NumArgs; n++)
			{
			char xName[512];

			NameFromLock(WBenchMsg->sm_ArgList[n].wa_Lock, xName, sizeof(xName));

			kprintf("%ld. Lock=<%s>  Name=<%s>\n", n, xName, WBenchMsg->sm_ArgList[n].wa_Name);
			}
		}
#endif

	if (NULL == WBenchMsg)
		{
		return 5;
		}
	if (WBenchMsg->sm_NumArgs < 2)
		{
		return 5;
		}

	do	{
		if (!OpenLibraries())
			break;

		if (!CheckMCCVersion(MUIC_BetterString, 11, 0))
			break;

		gb_Catalog = OpenCatalogA(NULL, "Scalos/Rename.catalog",NULL);

		APP_Main = ApplicationObject,
			MUIA_Application_Title,		GetLocString(MSGID_TITLENAME),
			MUIA_Application_Version,	"$VER: Scalos Rename.module V"
				STR(VERSION_MAJOR) "." STR(VERSION_MINOR) " (" __DATE__ ") " COMPILER_STRING,
			MUIA_Application_Copyright,	"� The Scalos Team, 2004" CURRENTYEAR,
			MUIA_Application_Author,	"The Scalos Team",
			MUIA_Application_Description,	"Scalos Rename module",
			MUIA_Application_Base,		"SCALOS_RENAME",

			SubWindow, WIN_Main = WindowObject,
				MUIA_Window_Title, GetLocString(MSGID_TITLENAME),
//				MUIA_Window_ID,	MAKE_ID('M','A','I','N'),
				MUIA_Window_AppWindow, TRUE,
				MUIA_Window_UseBottomBorderScroller, TRUE, // +jmc+

				MUIA_Window_TopEdge, MUIV_Window_TopEdge_Moused,
				MUIA_Window_LeftEdge, MUIV_Window_LeftEdge_Moused,

				MUIA_Window_Width, MUIV_Window_Width_Screen(30), // +jmc+

				WindowContents, VGroup,
					Child, VGroup,
						Child, TextEnterName = TextObject,
							MUIA_Text_PreParse, MUIX_C,
							End, //TextObject

						Child, 	GroupObject, // +jmc+
							Child, ScrollgroupObject,
								MUIA_Scrollgroup_VertBar, NULL,
								MUIA_Scrollgroup_HorizBar, NULL,
								MUIA_Scrollgroup_FreeHoriz, TRUE,
								MUIA_Scrollgroup_FreeVert, FALSE,
								MUIA_Scrollgroup_UseWinBorder, TRUE,
								MUIA_Scrollgroup_Contents,
								VirtgroupObject,
									Child, TextPath = TextObject,
										MUIA_Text_PreParse, MUIX_C,
										MUIA_ShortHelp, GetLocString(MSGID_PATHOF_NAME),
									End, //TextObject
								End, //VirtgroupObject
							End, //ScrollgroupObject
						End, //GroupObject

						Child, VSpace(1),

						Child, HGroup,
							Child, Label1(GetLocString(MSGID_LABEL_NEWNAME)),
							Child, StringNewName = BetterStringObject,
								StringFrame,
								MUIA_String_Reject, "/:",
								MUIA_Background, MUII_TextBack,
								MUIA_String_MaxLen, 108,
								MUIA_CycleChain, TRUE,
								End, //StringObject
						End, //HGroup
					End, //VGroup

					Child, Group_Buttons2 = HGroup,
						MUIA_Group_SameWidth, TRUE,
						Child, OkButton = KeyButtonHelp(GetLocString(MSGID_OKBUTTON),
									*GetLocString(MSGID_OKBUTTON_SHORT), GetLocString(MSGID_SHORTHELP_OKBUTTON)),
						Child, SkipButton = KeyButtonHelp(GetLocString(MSGID_SKIPBUTTON),
									*GetLocString(MSGID_SKIPBUTTON_SHORT), GetLocString(MSGID_SHORTHELP_SKIPBUTTON)),
						Child, CancelButton = KeyButtonHelp(GetLocString(MSGID_CANCELBUTTON),
									*GetLocString(MSGID_CANCELBUTTON_SHORT), GetLocString(MSGID_SHORTHELP_CANCELBUTTON)),
						End, //HGroup
				End, //VGroup
			End, //WindowObject

			MUIA_Application_Menustrip, MenustripObject,
				Child, MenuObjectT(GetLocString(MSGID_MENU_PROJECT)),

					Child, MenuAbout = MenuitemObject,
						MUIA_Menuitem_Title, GetLocString(MSGID_MENU_PROJECT_ABOUT),
					End,
					Child, MenuAboutMUI = MenuitemObject,
						MUIA_Menuitem_Title, GetLocString(MSGID_MENU_PROJECT_ABOUTMUI),
					End,
					Child, MenuitemObject,
						MUIA_Menuitem_Title, -1,
					End,
					Child, MenuQuit = MenuitemObject,
						MUIA_Menuitem_Title, GetLocString(MSGID_MENU_PROJECT_QUIT),
						MUIA_Menuitem_Shortcut, GetLocString(MSGID_MENU_PROJECT_QUIT_SHORT),
					End,

				End, //MenuObjectT
			End, //MenuStripObject

		End; //ApplicationObject

		d1(KPrintF(__FILE__ "/" __FUNC__ "/%ld: \n", __LINE__));

		if (NULL == APP_Main)
			{
			printf(GetLocString(MSGID_CREATE_APPLICATION_FAILED));
			break;
			}

		DoMethod(WIN_Main, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
			WIN_Main, 3, MUIM_Set, MUIA_Window_Open, FALSE);
		DoMethod(WIN_Main, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
			APP_Main, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

		DoMethod(CancelButton, MUIM_Notify, MUIA_Pressed, FALSE,
			APP_Main, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
		DoMethod(OkButton, MUIM_Notify, MUIA_Pressed, FALSE,
			APP_Main, 2, MUIM_Application_ReturnID, Application_Return_Ok);
		DoMethod(SkipButton, MUIM_Notify, MUIA_Pressed, FALSE,
			APP_Main, 2, MUIM_Application_ReturnID, Application_Return_Skip);

		DoMethod(StringNewName, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
			APP_Main, 2, MUIM_Application_ReturnID, Application_Return_Ok);

		DoMethod(MenuAbout, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			APP_Main, 2, MUIM_CallHook, &AboutHook);
		DoMethod(MenuAboutMUI, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			APP_Main, 2, MUIM_CallHook, &AboutMUIHook);
		DoMethod(MenuQuit, MUIM_Notify,MUIA_Menuitem_Trigger,MUIV_EveryTime,
			APP_Main, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

		for (n = 1; n < WBenchMsg->sm_NumArgs; n++)
			{
			struct WBArg *arg = &WBenchMsg->sm_ArgList[n];
			ULONG sigs = 0;
			BOOL Run = TRUE;
			BOOL IsWriteable = isDiskWritable(arg->wa_Lock);

			UpdateGadgets(arg);

			set(OkButton, MUIA_Disabled, !IsWriteable);
			set(StringNewName, MUIA_Disabled, !IsWriteable);

			set(WIN_Main, MUIA_Window_Open, TRUE);
			get(WIN_Main, MUIA_Window_Open, &win_opened);

			if (!win_opened)
				{
				printf(GetLocString(MSGID_CREATE_MAINWINDOW_FAILED));
				break;
				}

			d1(KPrintF(__FILE__ "/" __FUNC__ "/%ld: \n", __LINE__));

			set(WIN_Main, MUIA_Window_ActiveObject, StringNewName);

			while (Run)
				{
				ULONG Action = DoMethod(APP_Main, MUIM_Application_NewInput, &sigs);

				switch (Action)
					{
				case Application_Return_Ok:
					RenameObject(arg);
					Run = FALSE;
					break;
				case Application_Return_Skip:
					Run = FALSE;
					break;
				case MUIV_Application_ReturnID_Quit:
					 n = WBenchMsg->sm_NumArgs;
					Run = FALSE;
					break;
					}

				if (Run && sigs)
					{
					sigs = Wait(sigs | SIGBREAKF_CTRL_C);

					if (sigs & SIGBREAKF_CTRL_C)
						{
						Run = FALSE;
						}
					}
				}
			}

		set(WIN_Main, MUIA_Window_Open, FALSE);
		} while (0);

	if (APP_Main)
		MUI_DisposeObject(APP_Main);
	if(gb_Catalog)
		CloseCatalog(gb_Catalog);
		
	CloseLibraries();

	return 0;
}

//----------------------------------------------------------------------------

static BOOL OpenLibraries(void)
{
	MUIMasterBase = OpenLibrary(MUIMASTER_NAME, MUIMASTER_VMIN-1);
	if (NULL == MUIMasterBase)
		return FALSE;
#ifdef __amigaos4__
	else
		{
		IMUIMaster = (struct MUIMasterIFace *)GetInterface((struct Library *)MUIMasterBase, "main", 1, NULL);
		if (NULL == IMUIMaster)
			return FALSE;
		}
#endif

	IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 39);
	if (NULL == IntuitionBase)
		return FALSE;
#ifdef __amigaos4__
	else
		{
		IIntuition = (struct IntuitionIFace *)GetInterface((struct Library *)IntuitionBase, "main", 1, NULL);
		if (NULL == IIntuition)
			return FALSE;
		}
#endif

	LocaleBase = (T_LOCALEBASE) OpenLibrary("locale.library", 38);
	if (NULL == LocaleBase)
		return FALSE;
#ifdef __amigaos4__
	else
		{
		ILocale = (struct LocaleIFace *)GetInterface((struct Library *)LocaleBase, "main", 1, NULL);
		if (NULL == ILocale)
			return FALSE;
		}
#endif

#ifndef __amigaos4__
	UtilityBase = (T_UTILITYBASE) OpenLibrary("utility.library", 39);
	if (NULL == UtilityBase)
		return FALSE;
#endif

	return TRUE;
}

//----------------------------------------------------------------------------

static void CloseLibraries(void)
{
#ifndef __amigaos4__
	if (UtilityBase)
		{
		CloseLibrary((struct Library *) UtilityBase);
		UtilityBase = NULL;
		}
#endif
#ifdef __amigaos4__
	if (ILocale)
		{
		DropInterface((struct Interface *)ILocale);
		IIntuition = NULL;
		}
#endif
	if (LocaleBase)
		{
		CloseLibrary(LocaleBase);
		LocaleBase = NULL;
		}
#ifdef __amigaos4__
	if (IIntuition)
		{
		DropInterface((struct Interface *)IIntuition);
		IIntuition = NULL;
		}
#endif
	if (IntuitionBase)
		{
		CloseLibrary((struct Library *)IntuitionBase);
		IntuitionBase = NULL;
		}
#ifdef __amigaos4__
	if (IMUIMaster)
		{
		DropInterface((struct Interface *)IMUIMaster);
		IMUIMaster = NULL;
		}
#endif
	if (MUIMasterBase)
		{
		CloseLibrary(MUIMasterBase);
		MUIMasterBase = NULL;
		}
}

//----------------------------------------------------------------------------

static void UpdateGadgets(struct WBArg *arg)
{
	static char LabelText[1024];
	static char PathText[1024];
	STRPTR FileName;
	char xName[512];
	BPTR NLock;
	BPTR oldDir = (BPTR)NULL;

	NameFromLock(arg->wa_Lock, xName, sizeof(xName));

	if (arg->wa_Name && strlen(arg->wa_Name) > 0)
		FileName = arg->wa_Name;
	else
		FileName = FilePart(xName);

	d1(KPrintF(__FILE__ "/" __FUNC__ "/%ld: FileName =<%s> xName=<%s>  wa_Name=%08lx <%s>\n",   __LINE__, FileName, xName, arg->wa_Name, arg->wa_Name ? arg->wa_Name : ""));

	if (':' == xName[strlen(xName)-1] && strlen(arg->wa_Name) < 1)
		{
		sprintf(PathText, GetLocString(MSGID_TEXTLINE_PATH), xName);

		StripTrailingColon(xName);

		FileName = xName;

		RenameVolume = TRUE;
		}
	else
		{
		NLock = DupLock(arg->wa_Lock);
		if (NLock)
			{
			BOOL IfName;
			char xIconOnly[512];

			oldDir = CurrentDir(NLock);
			stccpy(xIconOnly, (STRPTR) FileName, sizeof(xIconOnly));

			d1(KPrintF(__FILE__ "/" __FUNC__ "/%ld: AddPart(): FileName=<%s> xIconOnly =<%s>\n",  __LINE__, FileName, xIconOnly));

			IfName = IfLocateName(xIconOnly);

			d1(KPrintF(__FILE__ "/" __FUNC__ "/%ld: IfLocateName(%s) FileName = <%s> IfName = %ld\n",  __LINE__, xIconOnly, FileName, IfName));

			if (!IfName)
				{
				BOOL IfNewName;

				strcat(xIconOnly,".info");
				IfNewName = IfLocateName(xIconOnly);

				if (IfNewName)
					FileName = xIconOnly;

				d1(KPrintF(__FILE__ "/" __FUNC__ "/%ld: FileName = <%s> IfNewName = %ld xIconOnly =<%s>\n",  __LINE__, FileName, IfNewName, xIconOnly));
				}
			}

		if (NLock)
			{
			CurrentDir(oldDir);
			UnLock(NLock);
			}

		d1(KPrintF(__FILE__ "/" __FUNC__ "/%ld: xname=<%s> FileName=<%s>\n",  __LINE__, xName, FileName));
		
		sprintf(PathText, GetLocString(MSGID_TEXTLINE_PATH), xName);
		d1(KPrintF("FileName =<%s> xName=<%s>  wa_Name=%08lx <%s>\n", FileName, xName, arg->wa_Name, arg->wa_Name ? arg->wa_Name : ""));
		}


	sprintf(LabelText, GetLocString(MSGID_TEXTLINE_ENTERNEWNAME), FileName);

	SetAttrs(StringNewName,
		MUIA_String_Contents, FileName,
		MUIA_BetterString_SelectSize, -strlen(FileName),
		TAG_END);

	SetAttrs(TextEnterName,
		MUIA_Text_Contents, LabelText,
		TAG_END);
	SetAttrs(TextPath,
		MUIA_Text_Contents, PathText,
		TAG_END);
}

//----------------------------------------------------------------------------

static BOOL IfLocateName(STRPTR FileName)
{
	BPTR FLock;

	FLock = Lock(FileName, ACCESS_READ);
	if ((BPTR)NULL == FLock)
		return FALSE;

	if (FLock)
		UnLock(FLock);

	return TRUE;
}

//----------------------------------------------------------------------------

static void ReportError(LONG Result, CONST_STRPTR OldObjName, CONST_STRPTR NewObjName)
{
	char FaultText[256];

	Fault(Result, "", FaultText, sizeof(FaultText));

	MUI_Request(APP_Main, WIN_Main, 0, NULL,
		GetLocString(MSGID_REQ_OK),
		GetLocString(MSGID_ERRORREQ_BODYTEXT),
		OldObjName, NewObjName, FaultText);
}

//----------------------------------------------------------------------------

static LONG RenameObject(struct WBArg *arg)
{
	BPTR dirLock;
	LONG Result = RETURN_OK;
	STRPTR NewObjName = NULL;
	STRPTR OldObjName;
	char ObjName[512];
	BPTR oldDir = (BPTR)NULL;
	BOOL IsWriteable = isDiskWritable(arg->wa_Lock);

	if (!IsWriteable)
		return ERROR_WRITE_PROTECTED;

	NameFromLock(arg->wa_Lock, ObjName, sizeof(ObjName));

	GetAttr(MUIA_String_Contents, StringNewName, (APTR) &NewObjName);

	if (arg->wa_Name && strlen(arg->wa_Name) > 0)
		{
		OldObjName = arg->wa_Name;
		dirLock = DupLock(arg->wa_Lock);
		}
	else
		{
		OldObjName = FilePart(ObjName);
		dirLock = ParentDir(arg->wa_Lock);
		}

	if (dirLock)
		oldDir = CurrentDir(dirLock);

	d1(KPrintF("dirLock=%08lx\n", dirLock));

	if (NewObjName && strlen(NewObjName) > 0)
		{
		BOOL IfName;
		char xIconOnly[512];

		if (RenameVolume)
			{
			StripTrailingColon(NewObjName);

			d1(KPrintF("OldName=<%s>  NewName=<%s>\n", ObjName, NewObjName));

			if (!Relabel(ObjName, NewObjName))
				{
				Result = IoErr();
			        OldObjName = ObjName;
			        }
			}
		else
			{

			d1(KPrintF(__FILE__ "/" __FUNC__ "/%ld: OldName=<%s>  NewName=<%s>\n",   __LINE__, OldObjName, NewObjName));

			stccpy(xIconOnly, OldObjName, sizeof(xIconOnly));

			d1(KPrintF(__FILE__ "/" __FUNC__ "/%ld: stccpy(): OldObjName=<%s> xIconOnly =<%s>\n",  __LINE__, OldObjName, xIconOnly));

			IfName = IfLocateName(xIconOnly);

			d1(KPrintF(__FILE__ "/" __FUNC__ "/%ld: IfLocateName(%s) OldObjName = <%s> IfName = %ld\n",  __LINE__, xIconOnly, OldObjName, IfName));

			if (!IfName)
				{
				BOOL IfNewName;

				strcat(xIconOnly,".info");
				IfNewName = IfLocateName(xIconOnly);

				if (IfNewName)
					{
					d1(KPrintF(__FILE__ "/" __FUNC__ "/%ld: IfLocateName(%s) OldObjName = <%s> IfNewName = %ld\n",  __LINE__, xIconOnly, OldObjName, IfNewName));
					OldObjName = xIconOnly;
					}
				}

			if (!Rename(OldObjName, NewObjName))
				Result = IoErr();

			// handle orphaned icons here (icons w/o object)
			if (ERROR_OBJECT_NOT_FOUND == Result)
				Result = RETURN_OK;
			}

		if (RETURN_OK != Result)
			ReportError(Result, OldObjName, NewObjName);
		else
			{
			// try to rename associated icon
			char OldIconName[512];
			char NewIconName[512];

			stccpy(OldIconName, OldObjName, sizeof(OldIconName) - 5);
			strcat(OldIconName, ".info");

			stccpy(NewIconName, NewObjName, sizeof(NewIconName) - 5);
			strcat(NewIconName, ".info");

			if (!Rename(OldIconName, NewIconName))
				Result = IoErr();

			if (ERROR_OBJECT_NOT_FOUND == Result)
				Result = RETURN_OK;

			if (RETURN_OK != Result)
				ReportError(Result, OldIconName, NewIconName);
			}
		}


	if (dirLock)
		{
		CurrentDir(oldDir);
		UnLock(dirLock);
		}

	return Result;
}

//----------------------------------------------------------------------------

void StripTrailingColon(STRPTR Line)
{
	size_t Len = strlen(Line);

	Line += Len - 1;

	if (':' == *Line)
		*Line = '\0';
}

//----------------------------------------------------------------------------

static STRPTR GetLocString(ULONG MsgId)
{
	struct LocaleInfo li;

	li.li_Catalog = gb_Catalog;
#ifndef __amigaos4__
	li.li_LocaleBase = LocaleBase;
#else
	li.li_ILocale = ILocale;
#endif

	return (STRPTR)GetString(&li, MsgId);
}

/*
static void TranslateStringArray(STRPTR *stringArray)
{
	while (*stringArray)
		{
		*stringArray = GetLocString((ULONG) *stringArray);
		stringArray++;
		}
}
*/

//----------------------------------------------------------------------------

static SAVEDS(APTR) INTERRUPT OpenAboutMUIHookFunc(struct Hook *hook, Object *o, Msg msg)
{
	if (NULL == WIN_AboutMUI)
		{
		WIN_AboutMUI = MUI_NewObject(MUIC_Aboutmui,
			MUIA_Window_RefWindow, WIN_Main,
			MUIA_Aboutmui_Application, APP_Main,
			End;
		}

	if (WIN_AboutMUI)
		set(WIN_AboutMUI, MUIA_Window_Open, TRUE);

	return 0;
}

//----------------------------------------------------------------------------

static SAVEDS(void) INTERRUPT OpenAboutHookFunc(struct Hook *hook, Object *o, Msg msg)
{
	MUI_Request(APP_Main, WIN_Main, 0, NULL,
		GetLocString(MSGID_ABOUTREQOK),
		GetLocString(MSGID_ABOUTREQFORMAT),
		VERSION_MAJOR, VERSION_MINOR, COMPILER_STRING, CURRENTYEAR);
}

//----------------------------------------------------------------------------

// return TRUE if Disk is writable
static BOOL CheckInfoData(const struct InfoData *info)
{
	BOOL Result = TRUE;

	d1(kprintf(__FILE__ "/" __FUNC__ "/%ld: DiskType=%ld  DiskState=%ld\n", __LINE__, info->id_DiskType, info->id_DiskState));

	switch (info->id_DiskType)
		{
	case ID_NO_DISK_PRESENT:
	case ID_UNREADABLE_DISK:
		Result = FALSE;
		break;
		}

	if (ID_WRITE_PROTECTED == info->id_DiskState)
		Result = FALSE;

	return Result;
}


static BOOL isDiskWritable(BPTR dLock)
{
	struct InfoData *info = malloc(sizeof(struct InfoData));
	BOOL Result = TRUE;

	if (info)
		{
		Info(dLock, info);

		if (!CheckInfoData(info))
			Result = FALSE;

		free(info);
		}

	return Result;
}

//----------------------------------------------------------------------------

//  Checks if a certain version of a MCC is available
static BOOL CheckMCCVersion(CONST_STRPTR name, ULONG minver, ULONG minrev)
{
	BOOL flush = TRUE;

	d1(kprintf(__FILE__ "/" __FUNC__ "/%ld: %s ", name, __LINE__);)

	while (1)
		{
		ULONG ver;
		ULONG rev;
		struct Library *base;
		char libname[256];

		// First we attempt to acquire the version and revision through MUI
		Object *obj = MUI_NewObject((STRPTR) name, TAG_DONE);
		if (obj)
			{
			get(obj, MUIA_Version, &ver);
			get(obj, MUIA_Revision, &rev);

			MUI_DisposeObject(obj);

			if(ver > minver || (ver == minver && rev >= minrev))
				{
				d1(kprintf(__FILE__ "/" __FUNC__ "/%ld: v%ld.%ld found through MUIA_Version/Revision\n", __LINE__, ver, rev);)
				return TRUE;
				}
			}

		// If we did't get the version we wanted, let's try to open the
		// libraries ourselves and see what happens...
		stccpy(libname, "PROGDIR:mui", sizeof(libname));
		AddPart(libname, name, sizeof(libname));

		if ((base = OpenLibrary(&libname[8], 0)) || (base = OpenLibrary(&libname[0], 0)))
			{
			UWORD OpenCnt = base->lib_OpenCnt;

			ver = base->lib_Version;
			rev = base->lib_Revision;

			CloseLibrary(base);

			// we add some additional check here so that eventual broken .mcc also have
			// a chance to pass this test (i.e. Toolbar.mcc is broken)
			if (ver > minver || (ver == minver && rev >= minrev))
				{
				d1(kprintf(__FILE__ "/" __FUNC__ "/%ld: v%ld.%ld found through OpenLibrary()\n", __LINE__, ver, rev);)
				return TRUE;
				}

			if (OpenCnt > 1)
				{
				if (MUI_Request(NULL, NULL, 0L, GetLocString(MSGID_STARTUP_FAILURE),
					GetLocString(MSGID_STARTUP_RETRY_QUIT_GAD), GetLocString(MSGID_STARTUP_MCC_IN_USE),
					name, minver, minrev, ver, rev))
					{
					flush = TRUE;
					}
				else
					break;
				}

			// Attempt to flush the library if open count is 0 or because the
			// user wants to retry (meaning there's a chance that it's 0 now)

			if (flush)
				{
				struct Library *result;

				Forbid();
				if ((result = (struct Library *) FindName(&((struct ExecBase *)SysBase)->LibList, name)))
					RemLibrary(result);
				Permit();
				flush = FALSE;
				}
			else
				{
				d1(kprintf(__FILE__ "/" __FUNC__ "/%ld: couldn`t find minimum required version.\n", __LINE__);)

				// We're out of luck - open count is 0, we've tried to flush
				// and still haven't got the version we want
				if (MUI_Request(NULL, NULL, 0L, GetLocString(MSGID_STARTUP_FAILURE),
					GetLocString(MSGID_STARTUP_RETRY_QUIT_GAD), GetLocString(MSGID_STARTUP_OLD_MCC),
					name, minver, minrev, ver, rev))
					{
					flush = TRUE;
					}
				else
					break;
				}
			}
		else
			{
			// No MCC at all - no need to attempt flush
			flush = FALSE;
			if (!MUI_Request(NULL, NULL, 0L, GetLocString(MSGID_STARTUP_FAILURE),
				GetLocString(MSGID_STARTUP_RETRY_QUIT_GAD), GetLocString(MSGID_STARTUP_MCC_NOT_FOUND),
				name, minver, minrev))
				{
				break;
				}
			}
		}

	return FALSE;
}

//----------------------------------------------------------------------------