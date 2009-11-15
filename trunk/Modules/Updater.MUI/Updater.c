// Updater.c
// $Date$
// $Revision$

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <dos/dos.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/io.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <libraries/dos.h>
#include <libraries/asl.h>
#include <libraries/mui.h>
#include <libraries/gadtools.h>
#include <mui/NListview_mcc.h>
#include <devices/clipboard.h>
#include <devices/timer.h>
#include <workbench/workbench.h>
#include <intuition/intuition.h>
#include <intuition/classusr.h>
#include <libraries/amisslmaster.h>
#include <libraries/amissl.h>
#include <openssl/pem.h>
#include <scalos/scalos.h>
#include <curl/curl.h>

#include <clib/alib_protos.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/asl.h>
#include <proto/muimaster.h>
#include <proto/locale.h>
#include <proto/scalos.h>
#include <proto/timer.h>
#include <proto/socket.h>
#include <proto/amissl.h>
#include <proto/amisslmaster.h>

#include <DefIcons.h>
#include <defs.h>
#include <Year.h> // +jmc+

#include "Updater.h"
#include "debug.h"

#define	CATCOMP_NUMBERS
#define	CATCOMP_BLOCK
#define	CATCOMP_CODE
#include "Updater_Locale.h"

//----------------------------------------------------------------------------

#if !defined(__amigaos4__) && !defined(__AROS__)
#include <dos.h>

long _stack = 16384;		// minimum stack size, used by SAS/C startup code
#endif

//----------------------------------------------------------------------------

// local data structures

#define KeyButtonHelp(name,key,HelpText)\
	TextObject,\
		ButtonFrame,\
		MUIA_CycleChain, TRUE, \
		MUIA_Font, MUIV_Font_Button,\
		MUIA_Text_Contents, name,\
		MUIA_Text_PreParse, "\33c",\
		MUIA_Text_HiChar  , key,\
		MUIA_ControlChar  , key,\
		MUIA_InputMode    , MUIV_InputMode_RelVerify,\
		MUIA_Background   , MUII_ButtonBack,\
		MUIA_ShortHelp, HelpText,\
		End

struct VersionFileHandle
	{
	STRPTR vfh_AllocatedBuffer;
	STRPTR vfh_BuffPtr;
	size_t vfh_AllocatedLength;
	size_t vfh_TotalLength;
	size_t vfh_CurrentLength;
	};

struct ComponentListEntry
	{
	STRPTR cle_Dir;
	STRPTR cle_File;
	STRPTR cle_Package;
	STRPTR cle_Hash;

	ULONG cle_RemoteVersion;
	ULONG cle_RemoteRevision;

	Object *cle_ImageObject;
	ULONG cle_ImageNr;
	ULONG cle_Selected;

	ULONG cle_LocalVersion;
	ULONG cle_LocalRevision;

	char cle_LocalVersionString[10];
	char cle_RemoteVersionString[10];
	char cle_ImageNrString[20];
	};

struct ProgressData
	{
	ULONG pgd_Min;
	ULONG pgd_Max;
	};

//----------------------------------------------------------------------------

// local functions

static BOOL init(void);
static void fail(APTR APP_Main, CONST_STRPTR str);
static BOOL OpenLibraries(void);
static void CloseLibraries(void);
static SAVEDS(APTR) INTERRUPT OpenAboutMUIFunc(struct Hook *hook, Object *o, Msg msg);
static SAVEDS(void) INTERRUPT OpenAboutFunc(struct Hook *hook, Object *o, Msg msg);

static void TranslateStringArray(CONST_STRPTR *stringArray);
static void TranslateNewMenu(struct NewMenu *nm);
static STRPTR GetLocString(ULONG MsgId);
BOOL CheckMCCVersion(CONST_STRPTR name, ULONG minver, ULONG minrev);
static void ProcessVersionFile(CONST_STRPTR text, size_t len);
static size_t VersionFileWrite( void *ptr, size_t size, size_t nmemb, void *stream);
static APTR ComponentsConstructHookFunc(struct Hook *hook, Object *obj, struct NList_ConstructMessage *msg);
static void ComponentsDestructHookFunc(struct Hook *hook, Object *obj, struct NList_DestructMessage *msg);
static ULONG ComponentsDisplayHookFunc(struct Hook *hook, Object *obj, struct NList_DisplayMessage *msg);
static LONG ComponentsCompareHookFunc(struct Hook *hook, Object *obj, struct NList_CompareMessage *msg);
static void ComponentToggleUpdateHookFunc(struct Hook *hook, Object *obj, Msg *msg);
static STRPTR GetAssignListVersionString(CONST_STRPTR Dir, CONST_STRPTR Filename, STRPTR VersionString, size_t MaxLen);
static STRPTR GetFileVersionString(CONST_STRPTR Dir, CONST_STRPTR Filename, STRPTR VersionString, size_t MaxLen);
static void ExtractVersionNumberFromVersionString(CONST_STRPTR VersionString, ULONG *Version, ULONG *Revision);
static void CheckForUpdatesHookFunc(struct Hook *hook, Object *obj, Msg *msg);
static void StartUpdateUpdateHookFunc(struct Hook *hook, Object *obj, Msg *msg);
static void SelectAllComponentsHookFunc(struct Hook *hook, Object *obj, Msg *msg);
static void DeselectAllComponentsHookFunc(struct Hook *hook, Object *obj, Msg *msg);
static void UpdateSelectedCount(void);
static int CheckForUpdatesProgressFunc(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
static int PerformUpdateProgressFunc(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
static BOOL CalculateFileHash(CONST_STRPTR Filename, unsigned char *digest);
static RSA* GetScalosPublicKey(void);
static BOOL VerifyFileSignature(CONST_STRPTR FileName, CONST_STRPTR SignatureString);
static BOOL HexStringDecode(CONST_STRPTR HexString, unsigned char *outbuf, size_t BuffLength);
static UBYTE HexDigit(char ch);
//static void ByteDump(unsigned char *Data, size_t Length);
static STRPTR EscapeHttpName(CONST_STRPTR name);
static BOOL SetProxyOptions(CURL *curl);
static void OpenSSLError(CONST_STRPTR Function, CONST_STRPTR Operation);

//----------------------------------------------------------------------------

static struct Hook AboutHook = {{ NULL, NULL }, HOOKFUNC_DEF(OpenAboutFunc), NULL };
static struct Hook AboutMUIHook = {{ NULL, NULL }, HOOKFUNC_DEF(OpenAboutMUIFunc), NULL };

//----------------------------------------------------------------------------
 
// local data items

struct DosLibrary *DOSBase;
struct IntuitionBase *IntuitionBase = NULL;
struct Library *MUIMasterBase = NULL;
struct ScalosBase *ScalosBase = NULL;
extern struct Library *SocketBase;
T_LOCALEBASE LocaleBase = NULL;
T_TIMERBASE TimerBase;
struct Library *AmiSSLMasterBase;
struct Library *AmiSSLBase;

#ifdef __amigaos4__
struct DOSIFace *IDOS;
struct IntuitionIFace *IIntuition = NULL;
struct MUIMasterIFace *IMUIMaster = NULL;
struct ScalosIFace *IScalos = NULL;
struct SocketIFace *ISocket = NULL;
struct LocaleIFace *ILocale = NULL;
struct TimerIFace *ITimer;
struct AmiSSLMasterIFace *IAmiSSLMaster;
struct AmiSSLIFace *IAmiSSL;
#endif

static struct Catalog *FindCatalog;

static Object *APP_Main;
static Object *WIN_Main;
static Object *WIN_AboutMUI;
static Object *MenuAbout, *MenuAboutMUI, *MenuQuit;
static Object *GaugeProgress;
static Object *NListComponents;
static Object *SelectedCountMsg;
static Object *ButtonCheckForUpdates, *ButtonStartUpdate;
static Object *ButtonSelectAll, *ButtonDeselectAll;
static Object *ContextMenuComponents;
static Object *CheckUseProxy, *CheckUseProxyAuth;
static Object *StringProxyAddr, *StringProxyPort;
static Object *StringProxyUser, *StringProxyPwd;

struct WBStartup *WBenchMsg;

static char TextProgressBuffer[256];

static T_TIMEREQUEST *TimerIO;
static struct MsgPort *TimerPort;

static ULONG globalImageNr = 0;

static ULONG SelectedCount = 0;

#if defined(__amigaos4__)
static CONST_STRPTR OsName = "AmigaOS4";
#elif defined(__MORPHOS__)
static CONST_STRPTR OsName = "MorphOS";
#else
static CONST_STRPTR OsName = "AmigaOS3";
#endif

static struct Hook ComponentsConstructHook = {{ NULL, NULL }, HOOKFUNC_DEF(ComponentsConstructHookFunc), NULL };
static struct Hook ComponentsDestructHook = {{ NULL, NULL }, HOOKFUNC_DEF(ComponentsDestructHookFunc), NULL };
static struct Hook ComponentsDisplayHook = {{ NULL, NULL }, HOOKFUNC_DEF(ComponentsDisplayHookFunc), NULL };
static struct Hook ComponentsCompareHook = {{ NULL, NULL }, HOOKFUNC_DEF(ComponentsCompareHookFunc), NULL };
static struct Hook ComponentToggleUpdateHook = {{ NULL, NULL }, HOOKFUNC_DEF(ComponentToggleUpdateHookFunc), NULL };
static struct Hook CheckForUpdatesHook = {{ NULL, NULL }, HOOKFUNC_DEF(CheckForUpdatesHookFunc), NULL };
static struct Hook StartUpdateUpdateHook = {{ NULL, NULL }, HOOKFUNC_DEF(StartUpdateUpdateHookFunc), NULL };
static struct Hook SelectAllComponentsHook = {{ NULL, NULL }, HOOKFUNC_DEF(SelectAllComponentsHookFunc), NULL };
static struct Hook DeselectAllComponentsHook = {{ NULL, NULL }, HOOKFUNC_DEF(DeselectAllComponentsHookFunc), NULL };

static CONST_STRPTR TempFilePath = "ram:";
static CONST_STRPTR ScalosHttpAddr = "scalos.noname.fr";

static ULONG fUseProxy = FALSE;   	// Flag: use HTTP Proxy
static ULONG fUseProxyAuth = FALSE;
static STRPTR ProxyAddr = "proxy-host.com";
static USHORT ProxyPort = 8080;
static STRPTR ProxyUser = "user";
static STRPTR ProxyPasswd = "password";

static CONST_STRPTR VersTag = "\0$VER: Scalos Updater.module V" VERS_MAJOR "." VERS_MINOR " (" __DATE__ ") " COMPILER_STRING;

DISPATCHER_PROTO(myComponentsNList);

static struct MUI_CustomClass *myComponentsNListClass;

static RSA *ScalosPubKeyRSA;		// Scalos Public key for signature verification

//----------------------------------------------------------------------------

static CONST_STRPTR RegisterTitles[] =
	{
	(CONST_STRPTR) MSGID_REGISTER_MAIN,
	(CONST_STRPTR) MSGID_REGISTER_CONFIGURATION,
	NULL
	};

//----------------------------------------------------------------------------

// Context menu in components list
static struct NewMenu NewContextMenuComponents[] =
	{
	{ NM_TITLE, (STRPTR) MSGID_MENU_COMPONENTS_TITLE,	NULL, 	0,	0, NULL },
	{  NM_ITEM, (STRPTR) MSGID_MENU_COMPONENTS_SELECT_ALL,	NULL, 	0,	0, (APTR) &SelectAllComponentsHook },
	{  NM_ITEM, (STRPTR) MSGID_MENU_COMPONENTS_SELECT_NONE,	NULL, 	0,	0, (APTR) &DeselectAllComponentsHook },
	{  NM_ITEM, NM_BARLABEL,				NULL, 	0,	0, NULL },
	{  NM_ITEM, (STRPTR) MSGID_MENU_PROJECT_ABOUT,		NULL, 	0,	0, (APTR) &AboutHook },

	{   NM_END, NULL, NULL,		0, 0, 0,},
	};

//----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	LONG win_opened;
	
	WBenchMsg = (argc == 0) ? (struct WBStartup *)argv : NULL;

	if (!init())
		{
		return 20;
		}

	if (!CheckMCCVersion(MUIC_NListview, 19, 66))
		{
		d1(KPrintF(__FILE__ "/%s/%ld: CheckMCCVersion failed\n", __FUNC__, __LINE__));
		return 10;
		}

	TranslateStringArray(RegisterTitles);

	ContextMenuComponents = MUI_MakeObject(MUIO_MenustripNM, NewContextMenuComponents, 0);
	if (NULL == ContextMenuComponents)
		{
		fail(APP_Main, "Failed to create Collections Context Menu.");
		}

	APP_Main = ApplicationObject,
		MUIA_Application_Title,		GetLocString(MSGID_TITLENAME),
		MUIA_Application_Version,	1 + VersTag,
		MUIA_Application_Copyright,	"© The Scalos Team, 2009" CURRENTYEAR,
		MUIA_Application_Author,	"The Scalos Team",
		MUIA_Application_Description,	"Scalos Updater module",
		MUIA_Application_Base,		"SCALOS_UPDATER_MODULE",

		SubWindow, WIN_Main = WindowObject,
			MUIA_Window_Title, GetLocString(MSGID_TITLENAME),
			MUIA_Window_ID,	MAKE_ID('M','A','I','N'),
			MUIA_Window_AppWindow, TRUE,

			WindowContents, VGroup,

				Child, RegisterGroup(RegisterTitles),

	//------- Main page ----------------
					Child, VGroup,
						Child, NListviewObject,
							MUIA_NListview_NList, NListComponents = NewObject(myComponentsNListClass->mcc_Class, 0,
								MUIA_NList_Format, "BAR,BAR,BAR,BAR,BAR",
								MUIA_Background, MUII_ListBack,
								MUIA_NList_ConstructHook2, &ComponentsConstructHook,
								MUIA_NList_DestructHook2, &ComponentsDestructHook,
								MUIA_NList_DisplayHook2, &ComponentsDisplayHook,
								MUIA_NList_CompareHook2, &ComponentsCompareHook,
								MUIA_NList_AdjustWidth, TRUE,
								MUIA_NList_Title, TRUE,
								MUIA_NList_TitleSeparator, TRUE,
								MUIA_NList_SortType, 1,
								MUIA_NList_TitleMark, MUIV_NList_TitleMark_Down | 1,
								MUIA_ContextMenu, ContextMenuComponents,
								End, //NListObject
							End, //NListviewObject

						Child, HGroup,
							Child, ColGroup(2),
								Child, ButtonSelectAll = KeyButtonHelp(GetLocString(MSGID_BUTTON_SELECT_ALL),
									GetLocString(MSGID_BUTTON_SELECT_ALL_SHORT),
									GetLocString(MSGID_BUTTON_SELECT_ALL_HELP)),
								Child, ButtonDeselectAll = KeyButtonHelp(GetLocString(MSGID_BUTTON_DESELECT_ALL),
									GetLocString(MSGID_BUTTON_DESELECT_ALL_SHORT),
									GetLocString(MSGID_BUTTON_DESELECT_ALL_HELP)),
								End, //ColGroup

							Child, SelectedCountMsg = TextObject,
								MUIA_Text_Contents, "",
								MUIA_Text_PreParse, MUIX_C,
								End, //TextObject
							End, //HGroup

						Child, HGroup,
							GroupFrame,
							MUIA_Background, MUII_GroupBack,

							Child, GaugeProgress = GaugeObject,
								GaugeFrame,
								MUIA_InputMode, MUIV_InputMode_None,
								MUIA_ShowSelState, FALSE,
								MUIA_Gauge_InfoText, "",
								MUIA_Gauge_Horiz, TRUE,
								MUIA_Gauge_Max, 100,
								End, //GaugeObject
							End, //HGroup
						

						Child, ColGroup(2),
							Child, ButtonCheckForUpdates = KeyButtonHelp(GetLocString(MSGID_BUTTON_CHECKFORUPDATES),
								GetLocString(MSGID_BUTTON_CHECKFORUPDATES_SHORT),
								GetLocString(MSGID_BUTTON_CHECKFORUPDATES_HELP)),
							Child, ButtonStartUpdate = KeyButtonHelp(GetLocString(MSGID_BUTTON_STARTUPDATE),
								GetLocString(MSGID_BUTTON_STARTUPDATE_SHORT),
								GetLocString(MSGID_BUTTON_STARTUPDATE_HELP)),
							End, //HGroup

						End, //VGroup

	//------- Configuration page ----------------
					Child, VGroup,
						Child, VGroup,
							GroupFrame,
							MUIA_Background, MUII_GroupBack,
							MUIA_FrameTitle, GetLocString(MSGID_GROUP_PROXY),
							Child, HGroup,
								Child, Label1((ULONG) GetLocString(MSGID_CHECK_USE_PROXY)),
								Child, HGroup,
									Child, CheckUseProxy = CheckMark(fUseProxy),
									Child, HVSpace,
									MUIA_ShortHelp, (ULONG) GetLocString(MSGID_CHECK_USE_PROXY_SHORTHELP),
									End, //HGroup
								End, //HGroup,
							Child, HGroup,
								Child, Label2((ULONG) GetLocString(MSGID_STRING_PROXY_ADDR)),
								Child, StringProxyAddr = StringObject,
									StringFrame,
									MUIA_Disabled, !fUseProxy,
									MUIA_CycleChain, TRUE,
									MUIA_String_Contents, ProxyAddr,
									MUIA_ShortHelp, (ULONG) GetLocString(MSGID_STRING_PROXY_ADDR_SHORTHELP),
									End, //StringObject
								Child, Label2((ULONG) GetLocString(MSGID_STRING_PROXY_PORT)),
								Child, StringProxyPort = StringObject,
									MUIA_Weight, 25,
									StringFrame,
									MUIA_Disabled, !fUseProxy,
									MUIA_CycleChain, TRUE,
									MUIA_String_Integer, ProxyPort,
									MUIA_ShortHelp, (ULONG) GetLocString(MSGID_STRING_PROXY_PORT_SHORTHELP),
									End, //StringObject
								End, //HGroup

							Child, HGroup,
								Child, Label1((ULONG) GetLocString(MSGID_CHECK_USE_PROXYAUTH)),
								Child, HGroup,
									Child, CheckUseProxyAuth = ImageObject,
										ImageButtonFrame,
										MUIA_InputMode, MUIV_InputMode_Toggle,
										MUIA_Image_Spec, MUII_CheckMark,
										MUIA_Image_FreeVert, TRUE,
										MUIA_Selected, fUseProxyAuth,
										MUIA_Background, MUII_ButtonBack,
										MUIA_ShowSelState, FALSE,
										MUIA_Disabled, !fUseProxy,
										MUIA_ShortHelp, (ULONG) GetLocString(MSGID_CHECK_USE_PROXYAUTH_SHORTHELP),
										End, //Checkmark()
									
									Child, HVSpace,
									End, //HGroup
								End, //HGroup,
							Child, HGroup,
								Child, Label2((ULONG) GetLocString(MSGID_STRING_PROXY_USERNAME)),
								Child, StringProxyUser = StringObject,
									StringFrame,
									MUIA_Disabled, !(fUseProxy && fUseProxyAuth),
									MUIA_CycleChain, TRUE,
									MUIA_String_Contents, ProxyUser,
									MUIA_ShortHelp, (ULONG) GetLocString(MSGID_STRING_PROXY_USERNAME_SHORTHELP),
									End, //StringObject
								Child, Label2((ULONG) GetLocString(MSGID_STRING_PROXY_PASSWD)),
								Child, StringProxyPwd = StringObject,
									StringFrame,
									MUIA_Disabled, !(fUseProxy && fUseProxyAuth),
									MUIA_CycleChain, TRUE,
									MUIA_String_Contents, ProxyPasswd,
									MUIA_ShortHelp, (ULONG) GetLocString(MSGID_STRING_PROXY_PASSWD_SHORTHELP),
									End, //StringObject
								End, //HGroup
							
							End, //VGroup
						End, //VGroup

					End, //RegisterGroup

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

	if (NULL == APP_Main)
		{
		fail(APP_Main, "Failed to create Application.");
		}

	DoMethod(APP_Main, MUIM_Application_Load, MUIV_Application_Load_ENV);

	//--------------------------------------------------------------------------//

	DoMethod(WIN_Main, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, 
		WIN_Main, 3, MUIM_Set, MUIA_Window_Open, FALSE);
	DoMethod(WIN_Main, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		APP_Main, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

	//--------------------------------------------------------------------------//

	DoMethod(MenuAbout, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
		APP_Main, 2, MUIM_CallHook, &AboutHook);
	DoMethod(MenuAboutMUI, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
		APP_Main, 2, MUIM_CallHook, &AboutMUIHook);
	DoMethod(MenuQuit, MUIM_Notify,MUIA_Menuitem_Trigger,MUIV_EveryTime,
		APP_Main, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

	//--------------------------------------------------------------------------//

	// setup sorting hooks for NListComponents list
	DoMethod(NListComponents, MUIM_Notify, MUIA_NList_TitleClick, MUIV_EveryTime,
		NListComponents, 4, MUIM_NList_Sort3, MUIV_TriggerValue, MUIV_NList_SortTypeAdd_2Values, MUIV_NList_Sort3_SortType_Both);
	DoMethod(NListComponents, MUIM_Notify, MUIA_NList_TitleClick2, MUIV_EveryTime,
		NListComponents, 4, MUIM_NList_Sort3, MUIV_TriggerValue, MUIV_NList_SortTypeAdd_2Values, MUIV_NList_Sort3_SortType_2);
	DoMethod(NListComponents, MUIM_Notify, MUIA_NList_SortType, MUIV_EveryTime,
		NListComponents, 3, MUIM_Set, MUIA_NList_TitleMark, MUIV_TriggerValue);
	DoMethod(NListComponents, MUIM_Notify, MUIA_NList_SortType2, MUIV_EveryTime,
		NListComponents, 3, MUIM_Set, MUIA_NList_TitleMark2, MUIV_TriggerValue);

	//--------------------------------------------------------------------------//

	// toggle update selection buttons on click
	DoMethod(NListComponents, MUIM_Notify, MUIA_NList_ButtonClick, MUIV_EveryTime,
		NListComponents, 2, MUIM_CallHook, &ComponentToggleUpdateHook);

	// Enable proxy configuration if "Use Proxy" is selected
	DoMethod(CheckUseProxy, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
		StringProxyAddr, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
	DoMethod(CheckUseProxy, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
		StringProxyPort, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
	DoMethod(CheckUseProxy, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
		CheckUseProxyAuth, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);

	DoMethod(CheckUseProxyAuth, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
		StringProxyUser, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
	DoMethod(CheckUseProxyAuth, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
		StringProxyPwd, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);

	UpdateSelectedCount();

	DoMethod(ButtonStartUpdate, MUIM_Notify, MUIA_Pressed, FALSE,
		ButtonStartUpdate, 2, MUIM_CallHook, &StartUpdateUpdateHook);
	DoMethod(ButtonCheckForUpdates, MUIM_Notify, MUIA_Pressed, FALSE,
		ButtonCheckForUpdates, 2, MUIM_CallHook, &CheckForUpdatesHook);

	// Setup callback hooks for "Select All" and "Deselect All" buttons
	DoMethod(ButtonSelectAll, MUIM_Notify, MUIA_Pressed, FALSE,
		ButtonSelectAll, 2, MUIM_CallHook, &SelectAllComponentsHook);
	DoMethod(ButtonDeselectAll, MUIM_Notify, MUIA_Pressed, FALSE,
		ButtonDeselectAll, 2, MUIM_CallHook, &DeselectAllComponentsHook);

	set(WIN_Main, MUIA_Window_Open, TRUE);
	get(WIN_Main, MUIA_Window_Open, &win_opened);

	if (win_opened)
		{
		ULONG sigs = 0;
		BOOL Run = TRUE;

		while (Run)
			{
			ULONG Action = DoMethod(APP_Main, MUIM_Application_NewInput, &sigs);

			switch (Action)
				{
			case MUIV_Application_ReturnID_Quit:
				Run = FALSE;
				break;
			default:
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
	else
		{
		printf("failed to open main window !\n");
		}

	set(WIN_Main, MUIA_Window_Open, FALSE);

	DoMethod(APP_Main, MUIM_Application_Save, MUIV_Application_Save_ENV);
	DoMethod(APP_Main, MUIM_Application_Save, MUIV_Application_Save_ENVARC);

	fail(APP_Main, NULL);

	return 0;
}

//----------------------------------------------------------------------------

static VOID fail(APTR APP_Main, CONST_STRPTR str)
{
	int rc = 0;

	if (ScalosPubKeyRSA)
		{
		RSA_free(ScalosPubKeyRSA);
		ScalosPubKeyRSA = NULL;
		}
	if (ContextMenuComponents)
		{
		MUI_DisposeObject(ContextMenuComponents);
		ContextMenuComponents = NULL;
		}
	if (APP_Main)
		{
		MUI_DisposeObject(APP_Main);
		}
	if (FindCatalog)
		{
		CloseCatalog(FindCatalog);
		FindCatalog = NULL;
		}
	if (myComponentsNListClass)
		{
		MUI_DeleteCustomClass(myComponentsNListClass);
		myComponentsNListClass = NULL;
		}

	if (str)
		{
		puts(str);
		rc = 20;
		}

	CloseLibraries();

	exit(rc);
}

//----------------------------------------------------------------------------

DISPATCHER(myComponentsNList)
{
	ULONG Result;

	d1(KPrintF(__FILE__ "/%s/%ld: MethodID=%08lx\n", __FUNC__, __LINE__, msg->MethodID));

	switch(msg->MethodID)
		{
	case MUIM_ContextMenuChoice:
		{
		struct MUIP_ContextMenuChoice *cmc = (struct MUIP_ContextMenuChoice *) msg;
		Object *MenuObj;
		struct Hook *MenuHook = NULL;

		d1(kprintf("%s/%ld:  MUIM_ContextMenuChoice  item=%08lx\n", __FUNC__, __LINE__, cmc->item));

		MenuObj = cmc->item;

		d1(kprintf("%s/%ld: MenuObj=%08lx\n", __FUNC__, __LINE__, MenuObj));
		d1(kprintf("%s/%ld: msg=%08lx *msg=%08lx\n", __FUNC__, __LINE__, msg, *((ULONG *) msg)));

		get(MenuObj, MUIA_UserData, &MenuHook);

		d1(kprintf("%s/%ld: MenuHook=%08lx\n", __FUNC__, __LINE__, MenuHook));
		if (MenuHook)
			DoMethod(obj, MUIM_CallHook, MenuHook, 0);

		Result = 0;
		}
		break;

	default:
		Result = DoSuperMethodA(cl, obj, msg);
		}

	d1(KPrintF(__FILE__ "/%s/%ld: MethodID=%08lx  Result=%ld\n", __FUNC__, __LINE__, msg->MethodID, Result));

	return Result;
}
DISPATCHER_END

//----------------------------------------------------------------------------

static BOOL init(void)
{
	if (!OpenLibraries())
		fail(NULL, "Failed to open "MUIMASTER_NAME".");

	if (LocaleBase)
		FindCatalog = OpenCatalogA(NULL, "Scalos/Updater.catalog", NULL);

	myComponentsNListClass = MUI_CreateCustomClass(NULL, MUIC_NList,
		NULL, 0, DISPATCHER_REF(myComponentsNList));

	TranslateNewMenu(NewContextMenuComponents);

	if (!InitAmiSSLMaster(AMISSL_CURRENT_VERSION, TRUE))
		fail(NULL, "AmiSSL version is too old!\n");

	AmiSSLBase = OpenAmiSSL();
	if (NULL == AmiSSLBase)
		fail(NULL, "Couldn't open AmiSSL!\n");

#ifdef __amigaos4__
	IAmiSSL = (struct AmiSSLIFace *)GetInterface(AmiSSLBase, "main", 1, NULL);
	if (NULL == IAmiSSL)
		fail(NULL, "Couldn't get AmiSSL interface!\n");
#endif //__amigaos4__

	if (0 != InitAmiSSL(AmiSSL_ErrNoPtr, (ULONG) &errno,
			    TAG_END))
		{
		fail(NULL, "Couldn't initialize AmiSSL!\n");
		}

	ScalosPubKeyRSA	= GetScalosPublicKey();
	if (NULL == ScalosPubKeyRSA)
		{
		fail(NULL, "Couldn't get Scalos Public Key!\n");
		}

	return TRUE;
}

//----------------------------------------------------------------------------

static BOOL OpenLibraries(void)
{
	DOSBase = (struct DosLibrary *) OpenLibrary(DOSNAME, 39);
	if (NULL == DOSBase)
		return FALSE;
#ifdef __amigaos4__
	IDOS = (struct DOSIFace *)GetInterface((struct Library *)DOSBase, "main", 1, NULL);
	if (NULL == IDOS)
		return FALSE;
#endif //__amigaos4__

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
#endif //__amigaos4__

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
#endif //__amigaos4__

	SocketBase = OpenLibrary("bsdsocket.library", 0);
	if (NULL == SocketBase)
		return FALSE;
#ifdef __amigaos4__
	else
		{
		ISocket = (struct SocketIFace *)GetInterface(SocketBase, "main", 1, NULL);
		if (NULL == ISocket)
			return FALSE;
		}
#endif //__amigaos4__


	ScalosBase = (struct ScalosBase *) OpenLibrary("scalos.library", 40);
	if (NULL == ScalosBase)
		return FALSE;
#ifdef __amigaos4__
	else
		{
		IScalos = (struct ScalosIFace *)GetInterface((struct Library *)ScalosBase, "main", 1, NULL);
		if (NULL == IScalos)
			return FALSE;
		}
#endif //__amigaos4__

	LocaleBase = (T_LOCALEBASE) OpenLibrary("locale.library", 39);
#ifdef __amigaos4__
	if (NULL != LocaleBase)
		{
		ILocale = (struct LocaleIFace *)GetInterface((struct Library *)LocaleBase, "main", 1, NULL);
		if (NULL == ILocale)
			return FALSE;
		}
#endif //__amigaos4__

	TimerPort = CreateMsgPort();
	TimerIO = (T_TIMEREQUEST *)CreateIORequest(TimerPort, sizeof(T_TIMEREQUEST));
	if (NULL == TimerIO)
		return FALSE;

	OpenDevice("timer.device", UNIT_VBLANK, &TimerIO->tr_node, 0);
	TimerBase = (T_TIMERBASE) TimerIO->tr_node.io_Device;
	if (NULL == TimerBase)
		return FALSE;
#ifdef __amigaos4__
	ITimer = (struct TimerIFace *)GetInterface((struct Library *)TimerBase, "main", 1, NULL);
	if (NULL == ITimer)
		return FALSE;
#endif //__amigaos4__

	AmiSSLMasterBase = OpenLibrary("amisslmaster.library", AMISSLMASTER_MIN_VERSION);
	if (NULL == AmiSSLMasterBase)
		return FALSE;
#ifdef __amigaos4__
	IAmiSSLMaster = (struct AmiSSLMasterIFace *)GetInterface(AmiSSLMasterBase, "main", 1, NULL);
	if (NULL == IAmiSSLMaster)
		return FALSE;
#endif //__amigaos4__

	return TRUE;
}

//----------------------------------------------------------------------------

static void CloseLibraries(void)
{
	if (AmiSSLBase)
		{
#ifdef __amigaos4__
		if (IAmiSSL)
			{
			CleanupAmiSSL(TAG_DONE);
			DropInterface((struct Interface *)IAmiSSL);
			IAmiSSL = NULL;
			}
#else //__amigaos4__
		CleanupAmiSSL(TAG_END);
#endif //__amigaos4__

		CloseAmiSSL();
		AmiSSLBase = NULL;
		}

#ifdef __amigaos4__
	DropInterface((struct Interface *)IAmiSSLMaster);
	IAmiSSLMaster = NULL;
#endif //__amigaos4__

	CloseLibrary(AmiSSLMasterBase);
	AmiSSLMasterBase = NULL;
	if (TimerIO)
		{
#ifdef __amigaos4__
		DropInterface((struct Interface *)ITimer);
#endif
		CloseDevice(&TimerIO->tr_node);
		DeleteIORequest(&TimerIO->tr_node);

		TimerIO	= NULL;
		TimerBase = NULL;
		}
	if (TimerPort)
		{
		DeleteMsgPort(TimerPort);
		TimerPort = NULL;
		}
#ifdef __amigaos4__
	if (IDOS)
		{
		DropInterface((struct Interface *)IDOS);
		IDOS = NULL;
		}
#endif //__amigaos4__
	if (DOSBase)
		{
		CloseLibrary((struct Library *) DOSBase);
		DOSBase = NULL;
		}
	if (LocaleBase)
		{
		if (FindCatalog)
			{
			CloseCatalog(FindCatalog);
			FindCatalog = NULL;
			}
#ifdef __amigaos4__
		if (ILocale)
			{
			DropInterface((struct Interface *)ILocale);
			ILocale = NULL;
			}
#endif
		CloseLibrary((struct Library *) LocaleBase);
		LocaleBase = NULL;
		}

#ifdef __amigaos4__
	if (ISocket)
		{
		DropInterface((struct Interface *)ISocket);
		ISocket	= NULL;
		}
#endif
	if (SocketBase)
		{
		CloseLibrary((struct Library *) SocketBase);
		SocketBase = NULL;
		}

#ifdef __amigaos4__
	if (IScalos)
		{
		DropInterface((struct Interface *)IScalos);
		IScalos = NULL;
		}
#endif
	if (ScalosBase)
		{
		CloseLibrary((struct Library *) ScalosBase);
		ScalosBase = NULL;
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
#ifdef __amigaos4__
	if (IIntuition)
		{
		DropInterface((struct Interface *)IIntuition);
		IIntuition = NULL;
		}
#endif
	if (IntuitionBase)
		{
		CloseLibrary((struct Library *) IntuitionBase);
		IntuitionBase = NULL;
		}
}

//----------------------------------------------------------------------------

static STRPTR GetLocString(ULONG MsgId)
{
	struct LocaleInfo li;

	li.li_Catalog = FindCatalog;  
#ifndef __amigaos4__
	li.li_LocaleBase = LocaleBase;
#else
	li.li_ILocale = ILocale;
#endif

	return (STRPTR)GetString(&li, MsgId);
}

//----------------------------------------------------------------------------

static void TranslateStringArray(CONST_STRPTR *stringArray)
{
	while (*stringArray)
		{
		*stringArray = GetLocString((ULONG) *stringArray);
		stringArray++;
		}
}

//----------------------------------------------------------------------------

static void TranslateNewMenu(struct NewMenu *nm)
{
	while (nm && NM_END != nm->nm_Type)
		{
		if (NM_BARLABEL != nm->nm_Label)
			nm->nm_Label = GetLocString((ULONG) nm->nm_Label);

		if (nm->nm_CommKey)
			nm->nm_CommKey = GetLocString((ULONG) nm->nm_CommKey);

		nm++;
		}
}

//----------------------------------------------------------------------------

static SAVEDS(APTR) INTERRUPT OpenAboutMUIFunc(struct Hook *hook, Object *o, Msg msg)
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

static SAVEDS(void) INTERRUPT OpenAboutFunc(struct Hook *hook, Object *o, Msg msg)
{
	MUI_Request(APP_Main, WIN_Main, 0, NULL, 
		GetLocString(MSGID_ABOUTREQOK), 
		GetLocString(MSGID_ABOUTREQFORMAT), 
		VERSION_MAJOR, VERSION_MINOR, COMPILER_STRING, CURRENTYEAR);
}

//----------------------------------------------------------------------------

//  Checks if a certain version of a MCC is available
BOOL CheckMCCVersion(CONST_STRPTR name, ULONG minver, ULONG minrev)
{
	BOOL flush = TRUE;

	d1(KPrintF(__FILE__ "/" __FUNC__ "/%ld: %s ", __LINE__, name);)

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
				if (MUI_Request(NULL, NULL, 0L,
					(STRPTR) GetLocString(MSGID_STARTUP_FAILURE),
					(STRPTR) GetLocString(MSGID_STARTUP_RETRY_QUIT_GAD),
                                        (STRPTR) GetLocString(MSGID_STARTUP_MCC_IN_USE),
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
				extern struct ExecBase *SysBase;

				Forbid();
				if ((result = (struct Library *) FindName(&SysBase->LibList, name)))
					RemLibrary(result);
				Permit();
				flush = FALSE;
				}
			else
				{
				d1(kprintf(__FILE__ "/" __FUNC__ "/%ld: couldn`t find minimum required version.\n", __LINE__);)

				// We're out of luck - open count is 0, we've tried to flush
				// and still haven't got the version we want
				if (MUI_Request(NULL, NULL, 0L,
					(STRPTR) GetLocString(MSGID_STARTUP_FAILURE),
					(STRPTR) GetLocString(MSGID_STARTUP_RETRY_QUIT_GAD),
                                        (STRPTR) GetLocString(MSGID_STARTUP_OLD_MCC),
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
			if (!MUI_Request(NULL, NULL, 0L,
				(STRPTR) GetLocString(MSGID_STARTUP_FAILURE),
				(STRPTR) GetLocString(MSGID_STARTUP_RETRY_QUIT_GAD),
                                (STRPTR) GetLocString(MSGID_STARTUP_MCC_NOT_FOUND),
				name, minver, minrev))
				{
				break;
				}
			}
		}

	return FALSE;
}

//----------------------------------------------------------------------------

static void ProcessVersionFile(CONST_STRPTR text, size_t len)
{
	size_t TotalLen = len;

	d1(KPrintF("%s/%s/%ld: Received data\n", __FILE__, __FUNC__, __LINE__));
	d1(KPrintF("%s/%s/%ld: =======================\n", __FILE__, __FUNC__, __LINE__));

	while (len)
		{
		size_t LineLen = 0;

		while (len && '\n' != text[LineLen])
			{
			LineLen++;
			len--;
			}

		set(GaugeProgress, MUIA_Gauge_Current, 100 - (50 * len) / TotalLen);

		// Skip empty lines, and comment lines.
		if (LineLen && ';' != *text && '#' != *text)
			{
			enum { ARG_DIR, ARG_FILE, ARG_VERSION, ARG_PKG, ARG_OS, ARG_HASH };
			static CONST_STRPTR Template = "DIR/A/K,FILE/A/K,VERSION/A/K,PKG/K,OS/A/K,HASH/K";
			struct RDArgs ReadArgs;
			LONG Args[6];

			memset(Args, 0, sizeof(Args));
			memset(&ReadArgs, 0, sizeof(ReadArgs));

			ReadArgs.RDA_Source.CS_Buffer = (STRPTR) text;
			ReadArgs.RDA_Source.CS_Length = LineLen;
			ReadArgs.RDA_Source.CS_CurChr = 0;
			ReadArgs.RDA_Flags |= RDAF_NOPROMPT;

			if (ReadArgs(Template, Args, &ReadArgs))
				{
				struct ComponentListEntry cle;

				cle.cle_Dir = (STRPTR) Args[ARG_DIR];
				cle.cle_File = (STRPTR) Args[ARG_FILE];
				cle.cle_Package = (STRPTR) Args[ARG_PKG];
				cle.cle_Hash = (STRPTR) Args[ARG_HASH];
				cle.cle_Selected = FALSE;

				d1(KPrintF("%s/%s/%ld: DIR=<%s> FILE=<%s> VERSION=<%s> PKG=<%s> OS=<%s>\n", __FILE__, __FUNC__, __LINE__, \
					Args[ARG_DIR], Args[ARG_FILE], Args[ARG_VERSION], Args[ARG_PKG], Args[ARG_OS]));

				if (0 == strcmp(OsName, (STRPTR) Args[ARG_OS]))
					{
					if (2 == sscanf((STRPTR) Args[ARG_VERSION], "%lu.%lu", &cle.cle_RemoteVersion, &cle.cle_RemoteRevision))
						{
						char VersionString[80];

						if (':' == cle.cle_Dir[strlen(cle.cle_Dir) - 1])
							GetAssignListVersionString(cle.cle_Dir, cle.cle_File, VersionString, sizeof(VersionString));
						else
							GetFileVersionString(cle.cle_Dir, cle.cle_File, VersionString, sizeof(VersionString));

						d1(KPrintF("%s/%s/%ld: VersionString=<%s>\n", __FILE__, __FUNC__, __LINE__, VersionString));
						
						ExtractVersionNumberFromVersionString(VersionString,
							&cle.cle_LocalVersion, &cle.cle_LocalRevision);

						d1(KPrintF("%s/%s/%ld: cle_LocalVersion=%lu  cle_LocalRevision=%lu\n", __FILE__, __FUNC__, __LINE__, cle.cle_LocalVersion, cle.cle_LocalRevision));

						cle.cle_Selected = (cle.cle_LocalVersion < cle.cle_RemoteVersion)
							|| ((cle.cle_LocalVersion == cle.cle_RemoteVersion) && (cle.cle_LocalRevision < cle.cle_RemoteRevision));

						if (cle.cle_Selected)
							SelectedCount++;

						DoMethod(NListComponents, MUIM_NList_InsertSingle,
							&cle, MUIV_NList_Insert_Sorted);
						}
					}

				FreeArgs(&ReadArgs);
				}
			else
				{
				d2(KPrintF("%s/%s/%ld: ReadArgs failed IoErr=%ld\n", __FILE__, __FUNC__, __LINE__, IoErr()));
				}
			}

		text += LineLen;

		// Skip '\n'
		len--;	
		text++;
		}

	d1(KPrintF("%s/%s/%ld: =======================\n", __FILE__, __FUNC__, __LINE__));
}

//----------------------------------------------------------------------------

static size_t VersionFileWrite( void *ptr, size_t size, size_t nmemb, void *stream)
{
	struct VersionFileHandle *vfh = (struct VersionFileHandle *) stream;
	size_t BytesWritten = 0;

	if (vfh)
		{
		size_t DataLength = size * nmemb;

		if ((vfh->vfh_CurrentLength + DataLength) > vfh->vfh_AllocatedLength)
			{
			size_t NewSize = vfh->vfh_AllocatedLength + DataLength + 32768;

			vfh->vfh_AllocatedBuffer = realloc(vfh->vfh_AllocatedBuffer, NewSize);
			vfh->vfh_AllocatedLength = NewSize;
			vfh->vfh_BuffPtr = vfh->vfh_AllocatedBuffer + vfh->vfh_CurrentLength;
			}
		if (vfh->vfh_AllocatedBuffer)
			{
			memcpy(vfh->vfh_BuffPtr, ptr, DataLength);
			vfh->vfh_BuffPtr += DataLength;
			vfh->vfh_CurrentLength += DataLength;
			vfh->vfh_TotalLength = vfh->vfh_CurrentLength;
			BytesWritten = DataLength;
			}
		}

	return BytesWritten;
}

//----------------------------------------------------------------------------

static APTR ComponentsConstructHookFunc(struct Hook *hook, Object *obj, struct NList_ConstructMessage *msg)
{
	struct ComponentListEntry *cle = AllocPooled(msg->pool, sizeof(struct ComponentListEntry));

	if (cle)
		{
		const struct ComponentListEntry *cleIn = (struct ComponentListEntry *) msg->entry;

		cle->cle_Dir = strdup(cleIn->cle_Dir);
		cle->cle_File = strdup(cleIn->cle_File);
		cle->cle_Package = strdup(cleIn->cle_Package);
		cle->cle_Hash = strdup(cleIn->cle_Hash);

		cle->cle_RemoteVersion = cleIn->cle_RemoteVersion;
		cle->cle_RemoteRevision = cleIn->cle_RemoteRevision;
		cle->cle_LocalVersion = cleIn->cle_LocalVersion;
		cle->cle_LocalRevision = cleIn->cle_LocalRevision;
		cle->cle_Selected = cleIn->cle_Selected;
		cle->cle_ImageNr = ++globalImageNr;

		cle->cle_ImageObject = ImageObject,
			MUIA_Image_Spec, MUII_CheckMark,
			ImageButtonFrame,
			MUIA_CycleChain, TRUE,
			MUIA_InputMode, MUIV_InputMode_Toggle,
			MUIA_Selected, cle->cle_Selected,
			MUIA_Background, MUII_ButtonBack,
			MUIA_ShowSelState, FALSE,
			End;
		DoMethod(obj, MUIM_NList_UseImage, cle->cle_ImageObject, cle->cle_ImageNr, 0);

		snprintf(cle->cle_LocalVersionString, sizeof(cle->cle_LocalVersionString), "%lu.%lu", cle->cle_LocalVersion, cle->cle_LocalRevision);
		snprintf(cle->cle_RemoteVersionString, sizeof(cle->cle_RemoteVersionString), "%lu.%lu", cle->cle_RemoteVersion, cle->cle_RemoteRevision);
		}

	return cle;
}

static void ComponentsDestructHookFunc(struct Hook *hook, Object *obj, struct NList_DestructMessage *msg)
{
	if (msg->entry)
		{
		struct ComponentListEntry *cle = (struct ComponentListEntry *) msg->entry;

		if (cle->cle_Hash)
			free(cle->cle_Hash);
		if (cle->cle_Package)
			free(cle->cle_Package);
		if (cle->cle_Dir)
			free(cle->cle_Dir);
		if (cle->cle_File)
			free(cle->cle_File);

		if (cle->cle_ImageObject)
			{
			DoMethod(obj, MUIM_NList_UseImage, NULL, cle->cle_ImageNr, 0);
			MUI_DisposeObject(cle->cle_ImageObject);
			cle->cle_ImageObject = NULL;
			}

		FreePooled(msg->pool, msg->entry, sizeof(struct ComponentListEntry));
		}
}

static ULONG ComponentsDisplayHookFunc(struct Hook *hook, Object *obj, struct NList_DisplayMessage *msg)
{
	if (msg->entry)
		{
		struct ComponentListEntry *cle = (struct ComponentListEntry *) msg->entry;

		snprintf(cle->cle_ImageNrString, sizeof(cle->cle_ImageNrString), "\33o[%ld@%ld|0]", cle->cle_ImageNr, cle->cle_ImageNr);
		msg->strings[0] = cle->cle_ImageNrString;
		msg->strings[1] = cle->cle_Dir;
		msg->strings[2] = cle->cle_File;
		msg->strings[3] = cle->cle_RemoteVersionString;
		msg->strings[4] = cle->cle_LocalVersionString;

		msg->preparses[0] = MUIX_C;

		if (0 == cle->cle_LocalVersion)
			{
			// local version not available
			msg->preparses[1] = msg->preparses[2] =
				msg->preparses[3] = msg->preparses[4] = MUIX_I;
			}
		else if ( (cle->cle_LocalVersion < cle->cle_RemoteVersion) ||
			(cle->cle_LocalVersion == cle->cle_RemoteVersion && cle->cle_LocalRevision < cle->cle_RemoteRevision) )
			{
			// needs to be updated
			msg->preparses[1] = msg->preparses[2] =
				msg->preparses[3] = msg->preparses[4] = MUIX_B;
			}
		else if ( (cle->cle_LocalVersion == cle->cle_RemoteVersion) && (cle->cle_LocalRevision == cle->cle_RemoteRevision) )
			{
			// versions match
			msg->preparses[1] = msg->preparses[2] =
				msg->preparses[3] = msg->preparses[4] = MUIX_N;
			}
		else
			{
			// Huh?
			// Local version newer than remote version
			msg->preparses[1] = msg->preparses[2] =
				msg->preparses[3] = msg->preparses[4] = MUIX_PH;
			}
		}
	else
		{
		// display titles
		msg->strings[0] = GetLocString(MSGID_COMPONENTLIST_UPDATE); ;
		msg->strings[1] = GetLocString(MSGID_COMPONENTLIST_DIRECTORY);
		msg->strings[2] = GetLocString(MSGID_COMPONENTLIST_FILE);
		msg->strings[3] = GetLocString(MSGID_COMPONENTLIST_LATEST_VERSION);
		msg->strings[4] = GetLocString(MSGID_COMPONENTLIST_INSTALLED_VERSION);
		}

	return 0;
}

static LONG ComponentsCompareHookFunc(struct Hook *hook, Object *obj, struct NList_CompareMessage *ncm)
{
	const struct ComponentListEntry *cle1 = (const struct ComponentListEntry *) ncm->entry1;
	const struct ComponentListEntry *cle2 = (const struct ComponentListEntry *) ncm->entry2;
	LONG col1 = ncm->sort_type & MUIV_NList_TitleMark_ColMask;
	LONG col2 = ncm->sort_type2 & MUIV_NList_TitleMark2_ColMask;
	LONG Result = 0;

	if (ncm->sort_type != MUIV_NList_SortType_None)
		{
		// primary sorting
		switch (col1)
			{
		case 1:		// sort by directory
			if (ncm->sort_type & MUIV_NList_TitleMark_TypeMask)
				Result = Stricmp(cle2->cle_Dir, cle1->cle_Dir);
			else
				Result = Stricmp(cle1->cle_Dir, cle2->cle_Dir);
			break;
		case 2:		// sort by filename
			if (ncm->sort_type & MUIV_NList_TitleMark_TypeMask)
				Result = Stricmp(cle2->cle_File, cle1->cle_File);
			else
				Result = Stricmp(cle1->cle_File, cle2->cle_File);
			break;
		case 3:		// sort by version
			if (ncm->sort_type & MUIV_NList_TitleMark_TypeMask)
				Result = cle2->cle_RemoteVersion - cle1->cle_RemoteVersion;
			else
				Result = cle1->cle_RemoteVersion - cle2->cle_RemoteVersion;
			break;
		case 4:		// sort by revision
			if (ncm->sort_type & MUIV_NList_TitleMark_TypeMask)
				Result = cle2->cle_RemoteRevision - cle1->cle_RemoteRevision;
			else
				Result = cle1->cle_RemoteRevision - cle2->cle_RemoteRevision;
			break;
		default:
			break;
			}

		if (0 == Result && col1 != col2)
			{
			// Secondary sorting
			switch (col2)
				{
			case 1:		// sort by directory
				if (ncm->sort_type2 & MUIV_NList_TitleMark2_TypeMask)
					Result = Stricmp(cle2->cle_Dir, cle1->cle_Dir);
				else
					Result = Stricmp(cle1->cle_Dir, cle2->cle_Dir);
				break;
			case 2:		// sort by filename
				if (ncm->sort_type2 & MUIV_NList_TitleMark2_TypeMask)
					Result = Stricmp(cle2->cle_File, cle1->cle_File);
				else
					Result = Stricmp(cle1->cle_File, cle2->cle_File);
				break;
			case 3:		// sort by version
				if (ncm->sort_type2 & MUIV_NList_TitleMark2_TypeMask)
					Result = cle2->cle_RemoteVersion - cle1->cle_RemoteVersion;
				else
					Result = cle1->cle_RemoteVersion - cle2->cle_RemoteVersion;
				break;
			case 4:		// sort by revision
				if (ncm->sort_type2 & MUIV_NList_TitleMark2_TypeMask)
					Result = cle2->cle_RemoteRevision - cle1->cle_RemoteRevision;
				else
					Result = cle1->cle_RemoteRevision - cle2->cle_RemoteRevision;
				break;
			default:
				break;
				}
			}
		}

	return Result;
}

//----------------------------------------------------------------------------

static void ComponentToggleUpdateHookFunc(struct Hook *hook, Object *obj, Msg *msg)
{
	ULONG ButtonNr;
	ULONG Entries;
	ULONG n;

	get(NListComponents, MUIA_NList_ButtonClick, &ButtonNr);

	d1(KPrintF("%s/%s/%ld: Button clicked, n=%ld\n", __FILE__, __FUNC__, __LINE__, ButtonNr));

	get(NListComponents, MUIA_NList_Entries, &Entries);

	for (n = 0; n < Entries; n++)
		{
		struct ComponentListEntry *cle = NULL;

		DoMethod(NListComponents, MUIM_NList_GetEntry,
			n, &cle);
		if (cle && cle->cle_ImageNr == ButtonNr)
			{
			cle->cle_Selected = !cle->cle_Selected;
			set(cle->cle_ImageObject, MUIA_Selected, cle->cle_Selected);

			if (cle->cle_Selected)
				SelectedCount++;
			else
				SelectedCount--;

			d1(KPrintF("%s/%s/%ld: Selected=%ld\n", __FILE__, __FUNC__, __LINE__, cle->cle_Selected));
			DoMethod(NListComponents, MUIM_NList_RedrawEntry, cle);

			UpdateSelectedCount();
			}
		}
}

//----------------------------------------------------------------------------

static STRPTR GetAssignListVersionString(CONST_STRPTR Dir, CONST_STRPTR Filename, STRPTR VersionString, size_t MaxLen)
{
	STRPTR NameCopy = NULL;

	d1(KPrintF("%s/%s/%ld: Dir=<%s>  Filename=<%s>\n", __FILE__, __FUNC__, __LINE__, Dir, Filename));

	*VersionString = '\0';

	do	{
		struct DosList *dl;

		GetFileVersionString(Dir, Filename, VersionString, MaxLen);
		if (strlen(VersionString) > 0)
			break;		// found!

		NameCopy = strdup(Dir);
		if (NULL == NameCopy)
			break;

		NameCopy[strlen(NameCopy) - 1] = '\0';	// strip trailing ":"

		dl = LockDosList(LDF_ASSIGNS | LDF_READ);

		if (dl = FindDosEntry(dl, NameCopy, LDF_ASSIGNS))
			{
			struct AssignList *asl;

			d2(KPrintF("%s/%s/%ld: dl=%08lx\n", __FILE__, __FUNC__, __LINE__, dl, dl->dol_misc.dol_assign.dol_AssignName));
			asl = dl->dol_misc.dol_assign.dol_List;

			while (asl)
				{
				char AssignDirName[300];

				debugLock_d1(asl->al_Lock);

				if (NameFromLock(asl->al_Lock, AssignDirName, sizeof(AssignDirName)))
					{
					GetFileVersionString(AssignDirName, Filename, VersionString, MaxLen);
					if (strlen(VersionString) > 0)
						break;		// found!
					}

				asl = asl->al_Next;
				}
			}

		UnLockDosList(LDF_ASSIGNS | LDF_READ);
		} while (0);

	if (NameCopy)
		free(NameCopy);

	return VersionString;
}

//----------------------------------------------------------------------------

static STRPTR GetFileVersionString(CONST_STRPTR Dir, CONST_STRPTR Filename, STRPTR VersionString, size_t MaxLen)
{
	char VersMask[10];
	BPTR fh = (BPTR)NULL;
	UBYTE *Buffer;
	const size_t BuffLength = 1024;
	T_TIMEVAL StartTime, currentTime;
	STRPTR OrigVersionString = VersionString;
	BPTR DirLock = (BPTR)NULL;

	// do not use statically initalized "$VER" variable here
	// since it might confuse the "version" command.
	// skip the leading "\0" in versTag
	stccpy(VersMask, VersTag + 1, 6 + 1);

	d1(KPrintF("%s/%s/%ld: Dir=<%s>  Filename=<%s>\n", __FILE__, __FUNC__, __LINE__, Dir, Filename));

	do	{
		ULONG TotalLen = 0;
		LONG ActLen;
		ULONG MaskNdx = 0;
		BOOL Finished = FALSE;
		BOOL Found = FALSE;
		BPTR OldDir;

		Buffer = malloc(BuffLength);
		if (NULL == Buffer)
			break;

		GetSysTime(&StartTime);

		DirLock = Lock(Dir, ACCESS_READ);
		if ((BPTR)NULL == DirLock)
			{
			d2(KPrintF("%s/%s/%ld: failed to lock dir=<%s>\n", __FILE__, __FUNC__, __LINE__, Dir));
			break;
			}

		OldDir = CurrentDir(DirLock);
		fh = Open(Filename, MODE_OLDFILE);
		CurrentDir(OldDir);

		if ((BPTR)NULL == fh)
			{
			d2(KPrintF("%s/%s/%ld: failed to open Filename=<%s>\n", __FILE__, __FUNC__, __LINE__, Filename));
			break;
			}

		do	{
			ActLen = Read(fh, Buffer, BuffLength);

			if (ActLen > 0)
				{
				ULONG n;
				UBYTE *BufPtr = Buffer;

				TotalLen += ActLen;

				for (n=0; !Finished && n<ActLen; n++)
					{
					if (Found)
						{
						if (MaxLen > 1 && *BufPtr && *BufPtr >= ' ' && *BufPtr <= 0x7f)
							{
							*VersionString++ = *BufPtr++;
							MaxLen--;
							}
						else
							Finished = TRUE;
						}
					else
						{
						if (*BufPtr++ == VersMask[MaskNdx])
							{
							MaskNdx++;

							if (MaskNdx >= strlen(VersMask))
								{
								Found = TRUE;
								}
							}
						else
							{
							MaskNdx = 0;
							}
						}
					}
				}

			GetSysTime(&currentTime);
			if ((currentTime.tv_secs - StartTime.tv_secs) > 2)
				{
				d2(KPrintF("%s/%s/%ld: timeout\n", __FILE__, __FUNC__, __LINE__));
				break;
				}
			} while (ActLen > 0 && !Finished);
		} while (0);

	*VersionString = '\0';

	if (Buffer)
		free(Buffer);
	if (fh)
		Close(fh);

	if (DirLock)
		UnLock(DirLock);

	return OrigVersionString;
}

//----------------------------------------------------------------------------

static void ExtractVersionNumberFromVersionString(CONST_STRPTR VersionString, ULONG *Version, ULONG *Revision)
{
	*Version = *Revision = 0;

	do	{
		while (*VersionString && !isspace(*VersionString))
			VersionString++;

		while (*VersionString && isspace(*VersionString))
			VersionString++;
		} while (*VersionString && !isdigit(*VersionString) && 'v' != ToLower(*VersionString));

	if ('v' == ToLower(*VersionString))
		VersionString++;

	sscanf(VersionString, "%ld.%ld", Version, Revision);

	d1(KPrintF("%s/%s/%ld: Version=%lu  Revision=%lu\n", __FILE__, __FUNC__, __LINE__, *Version, *Revision));
}

//----------------------------------------------------------------------------

static void CheckForUpdatesHookFunc(struct Hook *hook, Object *obj, Msg *msg)
{
	BOOL Success = FALSE;
	CURL *curl;

	set(GaugeProgress, MUIA_Gauge_Current, 0);

	curl = curl_easy_init();
	if (curl)
		{
		char addr[255];
		CURLcode res;
		struct ProgressData pgd;
		struct VersionFileHandle vfh;

		memset(&vfh, 0, sizeof(vfh));

		pgd.pgd_Min = 0;
		pgd.pgd_Max = 50;

		snprintf(addr, sizeof(addr), "%s%s", ScalosHttpAddr, "/versions.txt");

		snprintf(TextProgressBuffer, sizeof(TextProgressBuffer),
			GetLocString(MSG_PROGRESS_CONNECTING), ScalosHttpAddr);
		set(GaugeProgress, MUIA_Gauge_InfoText, TextProgressBuffer);

		if (SetProxyOptions(curl))
			{
			curl_easy_setopt(curl, CURLOPT_URL, addr);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, VersionFileWrite);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &vfh);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, CheckForUpdatesProgressFunc);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &pgd);

			res = curl_easy_perform(curl);

			/* always cleanup */
			curl_easy_cleanup(curl);

			if (0 == res)
				{
				set(GaugeProgress, MUIA_Gauge_InfoText, GetLocString(MSG_PROGRESS_PROCESS_HTTP_RESPONSE));
				ProcessVersionFile(vfh.vfh_AllocatedBuffer, vfh.vfh_TotalLength);
				Success = TRUE;
				}
			else
				{
				snprintf(TextProgressBuffer, sizeof(TextProgressBuffer),
					GetLocString(MSG_PROGRESS_FAILURE), ScalosHttpAddr, curl_easy_strerror(res));
				set(GaugeProgress, MUIA_Gauge_InfoText, TextProgressBuffer);
				}
			}

		free(vfh.vfh_AllocatedBuffer);
		}

	set(GaugeProgress, MUIA_Gauge_Current, 100);
	set(GaugeProgress, MUIA_Gauge_InfoText, GetLocString(Success ? MSG_PROGRESS_DONE : MSG_PROGRESS_FAILED));

	UpdateSelectedCount();
}

//----------------------------------------------------------------------------

static void StartUpdateUpdateHookFunc(struct Hook *hook, Object *obj, Msg *msg)
{
	CURL *curl;

	set(GaugeProgress, MUIA_Gauge_InfoText, GetLocString(MSG_PROGRESS_START_UPDATE));
	set(GaugeProgress, MUIA_Gauge_Current, 0);

	curl = curl_easy_init();
	if (curl)
		{
		ULONG Count = 0;
		ULONG Entries;
		ULONG n;

		get(NListComponents, MUIA_NList_Entries, &Entries);

		for (n = 0; n < Entries; n++)
			{
			FILE *fh = NULL;
			struct ComponentListEntry *cle = NULL;
			STRPTR Buffer = NULL;
			STRPTR HttpAddr = NULL;
			STRPTR EscapedPackageName = NULL;

			DoMethod(NListComponents, MUIM_NList_GetEntry,
				n, &cle);

			do	{
				struct ProgressData pgd;
				size_t len;
				CURLcode res;

				if (!cle->cle_Selected)
					break;

				pgd.pgd_Min = (100 * Count) / SelectedCount;
				pgd.pgd_Max = (100 * (1 + Count)) / SelectedCount;

				set(GaugeProgress, MUIA_Gauge_Current, pgd.pgd_Min);

				len = 1 + strlen(TempFilePath) + strlen(FilePart(cle->cle_Package));

				Buffer = malloc(len);
				if (NULL == Buffer)
					break;

				strcpy(Buffer, TempFilePath);
				AddPart(Buffer, FilePart(cle->cle_Package), len);

				snprintf(TextProgressBuffer, sizeof(TextProgressBuffer),
					GetLocString(MSG_PROGRESS_DOWNLOAD_UPDATE), cle->cle_Package);
				set(GaugeProgress, MUIA_Gauge_InfoText, TextProgressBuffer);

				EscapedPackageName = EscapeHttpName(cle->cle_Package);
				d2(KPrintF("%s/%s/%ld: EscapedPackageName=%08lx\n", __FILE__, __FUNC__, __LINE__, EscapedPackageName));
				if (NULL == EscapedPackageName)
					break;

				len = 1 + strlen(ScalosHttpAddr) + strlen(EscapedPackageName);
				HttpAddr = malloc(len);
				if (NULL == HttpAddr)
					break;

				snprintf(HttpAddr, len, "%s%s", ScalosHttpAddr, EscapedPackageName);

				d2(KPrintF("%s/%s/%ld: HttpAddr=<%s>\n", __FILE__, __FUNC__, __LINE__, HttpAddr));

				fh = fopen(Buffer, "wb");
				if (NULL == fh)
					break;

				if (!SetProxyOptions(curl))
					break;

				curl_easy_setopt(curl, CURLOPT_URL, HttpAddr);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, fh);
				curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, PerformUpdateProgressFunc);
				curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
				curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &pgd);

				d2(KPrintF("%s/%s/%ld: cle_Dir=<%s>  cle_File=<%s>  cle_Package=<%s>\n", \
					__FILE__, __FUNC__, __LINE__, cle->cle_Dir, cle->cle_File, cle->cle_Package));

				res = curl_easy_perform(curl);
				if (0 != res)
					break;

				fclose(fh);
				fh = NULL;

				set(GaugeProgress, MUIA_Gauge_InfoText, GetLocString(MSG_PROGRESS_VERIFY_SIGNATURE));

				if (!VerifyFileSignature(Buffer, cle->cle_Hash))
					{
					d2(KPrintF("%s/%s/%ld: File signature mismatch\n", __FILE__, __FUNC__, __LINE__));
					DeleteFile(Buffer);
					break;
					}

//      lha x -v Delete.module.lha #?.catalog locale:

				set(GaugeProgress, MUIA_Gauge_Current, pgd.pgd_Max);

				Count++;
				} while (0);

			if (fh)
				fclose(fh);
			if (Buffer)
				free(Buffer);
			if (HttpAddr)
				free(HttpAddr);
			if (EscapedPackageName)
				free(EscapedPackageName);

			}
		curl_easy_cleanup(curl);
		}

	set(GaugeProgress, MUIA_Gauge_Current, 100);
	set(GaugeProgress, MUIA_Gauge_InfoText, GetLocString(MSG_PROGRESS_DONE));
}

//----------------------------------------------------------------------------

static void SelectAllComponentsHookFunc(struct Hook *hook, Object *obj, Msg *msg)
{
	ULONG Entries;
	ULONG n;

	get(NListComponents, MUIA_NList_Entries, &Entries);

	set(NListComponents, MUIA_NList_Quiet, TRUE);

	for (n = 0; n < Entries; n++)
		{
		struct ComponentListEntry *cle = NULL;

		DoMethod(NListComponents, MUIM_NList_GetEntry,
			n, &cle);

		if (!cle->cle_Selected)
			{
			cle->cle_Selected = TRUE;
			set(cle->cle_ImageObject, MUIA_Selected, cle->cle_Selected);

			if (cle->cle_Selected)
				SelectedCount++;
			else
				SelectedCount--;

			DoMethod(NListComponents, MUIM_NList_RedrawEntry, cle);
			}
		}
	set(NListComponents, MUIA_NList_Quiet, FALSE);
	UpdateSelectedCount();
}

//----------------------------------------------------------------------------

static void DeselectAllComponentsHookFunc(struct Hook *hook, Object *obj, Msg *msg)
{
	ULONG Entries;
	ULONG n;

	get(NListComponents, MUIA_NList_Entries, &Entries);
	set(NListComponents, MUIA_NList_Quiet, TRUE);

	for (n = 0; n < Entries; n++)
		{
		struct ComponentListEntry *cle = NULL;

		DoMethod(NListComponents, MUIM_NList_GetEntry,
			n, &cle);

		if (cle->cle_Selected)
			{
			cle->cle_Selected = FALSE;
			set(cle->cle_ImageObject, MUIA_Selected, cle->cle_Selected);

			if (cle->cle_Selected)
				SelectedCount++;
			else
				SelectedCount--;

			DoMethod(NListComponents, MUIM_NList_RedrawEntry, cle);
			}
		}

	set(NListComponents, MUIA_NList_Quiet, FALSE);
	UpdateSelectedCount();
}

//----------------------------------------------------------------------------

static void UpdateSelectedCount(void)
{
	static char SelectedCountBuffer[60];

	snprintf(SelectedCountBuffer, sizeof(SelectedCountBuffer),
		GetLocString(MSGID_TEXT_SELECTED_COUNT), SelectedCount);
	set(SelectedCountMsg, MUIA_Text_Contents, SelectedCountBuffer);

	set(ButtonStartUpdate, MUIA_Disabled, 0 == SelectedCount);
	set(ButtonDeselectAll, MUIA_Disabled, 0 == SelectedCount);
}

//----------------------------------------------------------------------------

#if !defined(__amigaos4__)
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	struct timeval *timeout)
{
	return (int) WaitSelect(nfds, readfds, writefds, exceptfds, timeout, NULL);
}
#endif

//----------------------------------------------------------------------------

#if !defined(__amigaos4__)
int ioctl(int s, int cmd, char *arg)
{
	return IoctlSocket(s, cmd, arg);
}
#endif

//----------------------------------------------------------------------------

static int CheckForUpdatesProgressFunc(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	const struct ProgressData *pgd = (const struct ProgressData *) clientp;
	LONG dlLevel;

	dlLevel = pgd->pgd_Min + (LONG) (((pgd->pgd_Max - pgd->pgd_Min) * dlnow) / dltotal);

	d1(kprintf("%s/%ld: dlLevel=%ld\n", __FUNC__, __LINE__, dlLevel));
	set(GaugeProgress, MUIA_Gauge_Current, dlLevel);

	return 0;
}

//----------------------------------------------------------------------------

static int PerformUpdateProgressFunc(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	const struct ProgressData *pgd = (const struct ProgressData *) clientp;
	LONG dlLevel;

	dlLevel = pgd->pgd_Min + (LONG) (((pgd->pgd_Max - pgd->pgd_Min) * dlnow) / dltotal);

	d1(kprintf("%s/%ld: dlLevel=%ld\n", __FUNC__, __LINE__, dlLevel));
	set(GaugeProgress, MUIA_Gauge_Current, dlLevel);

	return 0;
}

//----------------------------------------------------------------------------

static BOOL CalculateFileHash(CONST_STRPTR Filename, unsigned char *digest)
{
	FILE *fh = NULL;
	UBYTE *Buffer = NULL;
	BOOL Success = FALSE;

	d2(KPrintF("%s/%s/%ld: START  Filename=<%s>\n", __FILE__, __FUNC__, __LINE__, Filename));

	do	{
		size_t nBytes;
		size_t BuffSize = 4096;
		SHA_CTX	ctx;

		memset(&ctx, 0, sizeof(ctx));
		SHA1_Init(&ctx);

		Buffer = malloc(BuffSize);
		if (NULL == Buffer)
			break;

		fh = fopen(Filename, "rb");
		if (NULL == fh)
			break;

		while ((nBytes = fread(Buffer, 1, BuffSize, fh)) > 0)
			{
			d1(KPrintF("%s/%s/%ld: nBytes=%ld\n", __FILE__, __FUNC__, __LINE__, nBytes));
			SHA1_Update(&ctx, Buffer, nBytes);
			}

		SHA1_Final(digest, &ctx);

		Success = TRUE;
		} while (0);

	if (fh)
		fclose(fh);
	if (Buffer)
		free(Buffer);

	d2(KPrintF("%s/%s/%ld: END  Success=%ld\n", __FILE__, __FUNC__, __LINE__, Success));

	return Success;
}

//----------------------------------------------------------------------------

static RSA *GetScalosPublicKey(void)
{
	static CONST_STRPTR ScalosPubKey =
		"-----BEGIN PUBLIC KEY-----\n"
		"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDER/rCy5ESeTLgUXDl7xbTmXR7\n"
		"OySYhzMk3pD3E9hS293vkU3rUCxbGVdI/ywg8Hw3UgrrYwg+7vxeShV4A1zzl8rm\n"
		"mWE5ZmV1mYH4UxU95zZfCgLDAiQDVp/dMHR5YBcb9MUMWYZGewiQo29yNUbSd+hQ\n"
		"W595xYqTCIN3lAfezwIDAQAB\n"
		"-----END PUBLIC KEY-----";
	RSA *rsa = NULL;
	BIO *bp;

	d1(KPrintF("%s/%s/%ld: START \n", __FILE__, __FUNC__, __LINE__));

	bp = BIO_new_mem_buf((void *) ScalosPubKey, strlen(ScalosPubKey));
	d2(KPrintF("%s/%s/%ld:  bp=%08lx\n", __FILE__, __FUNC__, __LINE__, bp));
	if (bp)
		{
		rsa = PEM_read_bio_RSA_PUBKEY(bp, NULL, NULL, NULL);

		if (NULL == rsa)
			{
			OpenSSLError(__FUNCTION__, "PEM_read_bio_RSA_PUBKEY");
			}
		BIO_free(bp);
		}
	else
		{
		OpenSSLError(__FUNCTION__, "BIO_new_mem_buf");
		}

	d1(KPrintF("%s/%s/%ld: END  rsa=%08lx\n", __FILE__, __FUNC__, __LINE__, rsa));

	return rsa;
}

//----------------------------------------------------------------------------

static BOOL VerifyFileSignature(CONST_STRPTR FileName, CONST_STRPTR SignatureString)
{
	BOOL Success = FALSE;

	d2(KPrintF("%s/%s/%ld: START  Filename=<%s>\n", __FILE__, __FUNC__, __LINE__, FileName));
	d1(KPrintF("%s/%s/%ld: SignatureString=<%s>\n", __FILE__, __FUNC__, __LINE__, SignatureString));

	do	{
		unsigned char signature[128];
		unsigned char digest[SHA_DIGEST_LENGTH];
		int rc;

		if (RSA_size(ScalosPubKeyRSA) != sizeof(signature))
			break;

		if (!HexStringDecode(SignatureString, signature, sizeof(signature)))
			{
			d2(KPrintF("%s/%s/%ld: HexStringDecode(%s) failed.\n", __FILE__, __FUNC__, __LINE__, SignatureString));
			break;
			}

		d1(KPrintF("%s/%s/%ld: signature=%08lx\n", __FILE__, __FUNC__, __LINE__, signature));
		//ByteDump(signature, sizeof(signature));
		d1(KPrintF("%s/%s/%ld: digest=%08lx\n", __FILE__, __FUNC__, __LINE__, digest));
		//ByteDump(digest, sizeof(digest));

		if (!CalculateFileHash(FileName, digest))
			{
			d2(KPrintF("%s/%s/%ld:  CalculateFileHash(%s) failed.\n", __FILE__, __FUNC__, __LINE__, FileName));
			break;
			}

		d1(KPrintF("%s/%s/%ld:  RSA_size=%ld.\n", __FILE__, __FUNC__, __LINE__, RSA_size(ScalosPubKeyRSA)));

		rc = RSA_verify(NID_sha1, digest, sizeof(digest),
			signature, sizeof(signature), ScalosPubKeyRSA);

		d2(KPrintF("%s/%s/%ld:  RSA_verify returned %ld.\n", __FILE__, __FUNC__, __LINE__, rc));

		if (1 == rc)
			{
			Success = TRUE;
			}
		else
			{
			OpenSSLError(__FUNCTION__, "RSA_verify");
			}
		} while (0);

	return Success;
}

//----------------------------------------------------------------------------

static BOOL HexStringDecode(CONST_STRPTR HexString, unsigned char *outbuf, size_t BuffLength)
{
	while (strlen(HexString) >= 2 && BuffLength)
		{
		if (!isxdigit(HexString[0]) || !isxdigit(HexString[1]))
			break;

		*outbuf++ = (HexDigit(HexString[0]) << 4) + HexDigit(HexString[1]);

		BuffLength--;
		HexString += 2;
		}

	return (0 == BuffLength) && (0 == strlen(HexString));
}

//----------------------------------------------------------------------------

static UBYTE HexDigit(char ch)
{
	UBYTE val = 0;

	if (isxdigit(ch))
		{
		if (ch >= 'a')
			val = ch - 'a' + 10;
		else if (ch >= 'A')
			val = ch - 'A' + 10;
		else
			val = ch - '0';
		}

	return val;
}

//----------------------------------------------------------------------------

#if 0
static void ByteDump(unsigned char *Data, size_t Length)
{
	unsigned long count;
	unsigned char Line[80], *lp;

	lp = Line;
	for (count=0; count<Length; count++)
		{
		*lp++ = isprint(*Data) ? *Data : '.';
		KPrintF("%02x ", *Data++);
		if ((count+1) % 16 == 0)
			{
			*lp = '\0';
			lp = Line;
			KPrintF("\t%s\n", Line);
			}
		}

	*lp = '\0';
	while (count % 16 != 0)
		{
		KPrintF("   ");
		count++;
		}
	KPrintF("\t%s\n", Line);
}
#endif

//----------------------------------------------------------------------------

static STRPTR EscapeHttpName(CONST_STRPTR name)
{
	size_t len = strlen(name);
	STRPTR EscapedName;

	EscapedName = malloc(1 + len);
	if (EscapedName)
		{
		STRPTR dp = EscapedName;
		ULONG n = 0;

		while (*name)
			{
			if (' ' == *name)
				{
				len += 2;
				EscapedName = realloc(EscapedName, len + 1);
				if (NULL == EscapedName)
					break;

				dp = EscapedName + n;

				*dp++ = '%';
				*dp++ = '2';
				*dp++ = '0';
				name++;
				n += 3;
				}
			else
				{
				*dp++ = *name++;
				n++;
				}
			}
		*dp = '\0';
		}

	return EscapedName;
}

//----------------------------------------------------------------------------

static BOOL SetProxyOptions(CURL *curl)
{
	BOOL Success = FALSE;
	STRPTR ProxyUserPwd = NULL;
	STRPTR ProxyName = NULL;

	do	{
		STRPTR ProxyAddr;
		ULONG ProxyPort;
		STRPTR ProxyUsername;
		STRPTR ProxyPasswd;
		size_t ProxyNameLen;
		size_t ProxyUserPwdLen;
		ULONG useProxy;
		ULONG useProxyAuth;

		get(CheckUseProxy, MUIA_Selected, &useProxy);
		if (!fUseProxy)
			{
			Success = TRUE;
			break;
			}

		get(StringProxyAddr, MUIA_String_Contents, &ProxyAddr);
		get(StringProxyPort, MUIA_String_Integer, &ProxyPort);

		get(StringProxyUser, MUIA_String_Contents, &ProxyUsername);
		get(StringProxyPwd, MUIA_String_Contents, &ProxyPasswd);

		get(CheckUseProxyAuth, MUIA_Selected, &useProxyAuth);

		ProxyUserPwdLen = 2 + strlen(ProxyUsername) + strlen(ProxyPasswd);
		ProxyNameLen = strlen(ProxyAddr) + 10;
		ProxyName = malloc(ProxyNameLen);
		if (NULL == ProxyName)
			break;

		snprintf(ProxyName, ProxyNameLen, "%s:%ld", ProxyName, ProxyPort);

		curl_easy_setopt(curl, CURLOPT_PROXY, ProxyName);

		if (useProxyAuth)
			{
			ProxyUserPwd = malloc(ProxyUserPwdLen);
			if (NULL == ProxyUserPwd)
				break;

			snprintf(ProxyUserPwd, ProxyUserPwdLen, "%s:%s", ProxyUsername, ProxyPasswd);

			curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, ProxyUserPwd);
			}

		Success = TRUE;
		} while (0);

	if (ProxyName)
		free(ProxyName);
	if (ProxyUserPwd)
		free(ProxyUserPwd);

	return Success;
}

//----------------------------------------------------------------------------

static void OpenSSLError(CONST_STRPTR Function, CONST_STRPTR Operation)
{
	STRPTR ErrorBuffer;

	ERR_load_crypto_strings();

	ErrorBuffer = ERR_error_string(ERR_get_error(), NULL);
	d2(KPrintF("%s/%s/%ld:  <%s>\n", __FILE__, __FUNC__, __LINE__, ErrorBuffer));

	MUI_Request(APP_Main, WIN_Main, 0, NULL,
		GetLocString(MSGID_ABOUTREQOK),
		GetLocString(MSGID_REQFORMAT_OPENSSL_ERROR),
		Function, Operation, ErrorBuffer);

	ERR_free_strings();
}

//----------------------------------------------------------------------------


