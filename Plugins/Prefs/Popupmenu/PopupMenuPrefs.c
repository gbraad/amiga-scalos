// PopupMenuPrefs.c
// $Date$
// $Revision$


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
#include <dos/dos.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/io.h>
#include <exec/resident.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <libraries/mui.h>
#include <libraries/iffparse.h>
#include <libraries/pm.h>
#include <devices/clipboard.h>
#include <utility/utility.h>
#include <workbench/workbench.h>
#include <workbench/icon.h>
#include <intuition/intuition.h>
#include <intuition/classusr.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <prefs/prefhdr.h>

#include <clib/alib_protos.h>

#define	__USE_SYSBASE

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/icon.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/utility.h>
#include <proto/asl.h>
#include <proto/pm.h>
#include <proto/locale.h>
#include <proto/iffparse.h>
#define	NO_INLINE_STDARG
#include <proto/muimaster.h>

#include <MUI/NListview_mcc.h>
#include <MUI/NListtree_mcc.h>
#include <MUI/Lamp_mcc.h>
#include <Scalos/scalosprefsplugin.h>
#include <defs.h>
#include <Year.h>

#define	CATCOMP_NUMBERS
#define	CATCOMP_BLOCK
#define	CATCOMP_CODE
#include "PopupMenu_locale.h"

#include "PopupMenuPrefs.h"
#include "DataTypesMCC.h"
#include "FrameButtonMCC.h"
#include "pmprefs.h"

#include "plugin.h"

//----------------------------------------------------------------------------

// local data structures

#define CheckMarkHelp(selected, HelpTextID)\
	ImageObject,\
		ImageButtonFrame,\
		MUIA_InputMode        , MUIV_InputMode_Toggle,\
		MUIA_Image_Spec       , MUII_CheckMark,\
		MUIA_Image_FreeVert   , TRUE,\
		MUIA_Selected         , selected,\
		MUIA_Background       , MUII_ButtonBack,\
		MUIA_ShowSelState     , FALSE,\
		MUIA_CycleChain       , TRUE,\
		MUIA_ShortHelp	      , GetLocString(HelpTextID), \
		End

#define IMG(prefix1,PREFIX2) \
  BodychunkObject,\
    MUIA_FixWidth             , PREFIX2##_WIDTH ,\
    MUIA_FixHeight            , PREFIX2##_HEIGHT,\
    MUIA_Bitmap_Width         , PREFIX2##_WIDTH ,\
    MUIA_Bitmap_Height        , PREFIX2##_HEIGHT,\
    MUIA_Bodychunk_Depth      , PREFIX2##_DEPTH ,\
    MUIA_Bodychunk_Body       , (UBYTE *) prefix1##_body,\
    MUIA_Bodychunk_Compression, PREFIX2##_COMPRESSION,\
    MUIA_Bodychunk_Masking    , PREFIX2##_MASKING,\
    MUIA_Bitmap_SourceColors  , (ULONG *) prefix1##_colors,\
    MUIA_Bitmap_Transparent   , 0,\
  End

#define KeyButtonHelp(name,key,HelpText)\
	TextObject,\
		ButtonFrame,\
		MUIA_CycleChain, TRUE, \
		MUIA_Font, MUIV_Font_Button,\
		MUIA_Text_Contents, (name),\
		MUIA_Text_PreParse, "\33c",\
		MUIA_Text_HiChar  , (key),\
		MUIA_ControlChar  , (key),\
		MUIA_InputMode    , MUIV_InputMode_RelVerify,\
		MUIA_Background   , MUII_ButtonBack,\
		MUIA_ShortHelp, HelpText,\
		End
#if 0
#define FrameButton(frame, text, selected, raised)\
	NewObject(FrameButtonClass->mcc_Class, 0, \
		MUIA_Frame, MUIV_Frame_None, \
		MUIA_Text_Contents, (text),\
		MUIA_Text_PreParse, "\33c", \
		MUIA_Font, MUIV_Font_Button,\
		MUIA_CycleChain, TRUE, \
		MUIA_InputMode, MUIV_InputMode_Immediate,\
		MUIA_Background, MUII_ButtonBack,\
		MUIA_ScaFrameButton_FrameType, (frame),\
		MUIA_ScaFrameButton_Selected, (selected), \
		MUIA_ScaFrameButton_Raised, (raised), \
		End
#endif

#define FrameButton(objindex, frame, text, selected, raised)\
	VGroup, \
		MUIA_ShowSelState, FALSE, \
		Child, NewObject(FrameButtonClass->mcc_Class, 0, \
			MUIA_Frame, MUIV_Frame_None, \
			MUIA_Text_Contents, (text),\
			MUIA_Text_PreParse, "\33c", \
			MUIA_Font, MUIV_Font_Button,\
			MUIA_CycleChain, TRUE, \
			MUIA_InputMode, MUIV_InputMode_Immediate,\
			MUIA_Background, MUII_ButtonBack,\
			MUIA_ScaFrameButton_FrameType, (frame),\
			MUIA_ScaFrameButton_Selected, (selected), \
			MUIA_ScaFrameButton_Raised, (raised), \
			End, \
		Child, HGroup, \
			Child, HVSpace, \
			Child, inst->mpb_Objects[objindex] = ImageObject,\
				ImageButtonFrame,\
				MUIA_InputMode        , MUIV_InputMode_Immediate,\
				MUIA_Image_Spec       , MUII_CheckMark,\
				MUIA_Image_FreeVert   , TRUE,\
				MUIA_Selected         , FALSE,\
				MUIA_Background       , MUII_ButtonBack,\
				MUIA_ShowSelState     , FALSE,\
				MUIA_CycleChain       , TRUE,\
				End, /* ImageObject */ \
			Child, HVSpace, \
			End, /* HGroup */ \
		End /* VGroup */

#define	Application_Return_EDIT	0
#define	Application_Return_USE	1001
#define	Application_Return_SAVE	1002

//----------------------------------------------------------------------------

// imported from mempools.lib

extern int _STI_240_InitMemFunctions(void);
extern void _STD_240_TerminateMemFunctions(void);

//----------------------------------------------------------------------------

// local functions

static BOOL OpenLibraries(void);
static void CloseLibraries(void);

DISPATCHER_PROTO(PopupMenuPrefs);
static Object *CreatePrefsGroup(struct PopupMenuPrefsInst *inst);

static SAVEDS(ULONG) INTERRUPT ImagePopAslFileStartHookFunc(struct Hook *hook, Object *o, Msg msg);

static STRPTR GetLocString(ULONG MsgId);
static void TranslateStringArray(STRPTR *stringArray);

static SAVEDS(APTR) INTERRUPT ResetToDefaultsHookFunc(struct Hook *hook, Object *o, Msg msg);
static SAVEDS(APTR) INTERRUPT OpenHookFunc(struct Hook *hook, Object *o, Msg msg);
static SAVEDS(APTR) INTERRUPT SaveAsHookFunc(struct Hook *hook, Object *o, Msg msg);
static SAVEDS(APTR) INTERRUPT AboutHookFunc(struct Hook *hook, Object *o, Msg msg);
static SAVEDS(APTR) INTERRUPT LastSavedHookFunc(struct Hook *hook, Object *o, Msg msg);
static SAVEDS(APTR) INTERRUPT RestoreHookFunc(struct Hook *hook, Object *o, Msg msg);

static SAVEDS(APTR) INTERRUPT AppMessageHookFunc(struct Hook *hook, Object *o, Msg msg);
static SAVEDS(APTR) INTERRUPT SettingsChangedHookFunc(struct Hook *hook, Object *o, Msg msg);
static SAVEDS(void) INTERRUPT AslIntuiMsgHookFunc(struct Hook *hook, Object *o, Msg msg);
static SAVEDS(void) INTERRUPT FrameTypeSelectHookFunc(struct Hook *hook, Object *o, Msg msg);
static SAVEDS(void) INTERRUPT SelFrameTypeSelectHookFunc(struct Hook *hook, Object *o, Msg msg);

static LONG ReadPrefsFile(struct PopupMenuPrefsInst *inst, CONST_STRPTR Filename, BOOL Quiet);
static LONG WritePrefsFile(struct PopupMenuPrefsInst *inst, CONST_STRPTR Filename);
static LONG SaveIcon(struct PopupMenuPrefsInst *inst, CONST_STRPTR IconName);
static Object *CreatePrefsImage(void);
static void InitHooks(struct PopupMenuPrefsInst *inst);
static void SetChangedFlag(struct PopupMenuPrefsInst *inst, BOOL changed);
static void ParseToolTypes(struct PopupMenuPrefsInst *inst, struct MUIP_ScalosPrefs_ParseToolTypes *ptt);
static void PrefsToGUI(struct PopupMenuPrefsInst *inst, const struct PopupMenuPrefs *pmPrefs);
static void GUItoPrefs(struct PopupMenuPrefsInst *inst, struct PopupMenuPrefs *pmPrefs);
static ULONG getv(APTR obj, ULONG attr);
#if !defined(__SASC) && !defined(__MORPHOS__) && !defined(__amigaos4__)
static size_t stccpy(char *dest, const char *src, size_t MaxLen);
#endif /* !defined(__SASC) && !defined(__MORPHOS__) && !defined(__amigaos4__) */

//----------------------------------------------------------------------------

// local data items

struct Library *MUIMasterBase;
T_LOCALEBASE LocaleBase;
struct GfxBase *GfxBase;
struct Library *IconBase;
struct Library *IFFParseBase;
T_UTILITYBASE UtilityBase;
struct IntuitionBase *IntuitionBase;
struct PopupMenuBase *PopupMenuBase;
struct Library *DataTypesBase;

#ifdef __amigaos4__
struct Library *DOSBase;
struct DOSIFace *IDOS;
struct MUIMasterIFace *IMUIMaster;
struct LocaleIFace *ILocale;
struct GraphicsIFace *IGraphics;
struct IconIFace *IIcon;
struct IFFParseIFace *IIFFParse;
struct UtilityIFace *IUtility;
struct IntuitionIFace *IIntuition;
struct PopupMenuIFace *IPopupMenu;
struct DataTypesIFace *IDataTypes;
struct Library *NewlibBase;
struct Interface *INewlib;
#endif

#include "PopupMenuPrefsImage.h"

static ULONG SIgnature = 0x4711;

static ULONG MajorVersion, MinorVersion;

static const struct MUIP_ScalosPrefs_MCCList RequiredMccList[] =
	{
	{ MUIC_Lamp, 11, 1 },
	{ MUIC_NListtree, 18, 18 },
	{ MUIC_NListview, 18, 0 },
	{ NULL, 0, 0 }
	};

static const struct Hook PopupMenuPrefsHooks[] =
{
	{ { NULL, NULL }, HOOKFUNC_DEF(ImagePopAslFileStartHookFunc), NULL },

	{ { NULL, NULL }, HOOKFUNC_DEF(ResetToDefaultsHookFunc), NULL },
	{ { NULL, NULL }, HOOKFUNC_DEF(OpenHookFunc), NULL },
	{ { NULL, NULL }, HOOKFUNC_DEF(LastSavedHookFunc), NULL },
	{ { NULL, NULL }, HOOKFUNC_DEF(RestoreHookFunc), NULL },
	{ { NULL, NULL }, HOOKFUNC_DEF(SaveAsHookFunc), NULL },

	{ { NULL, NULL }, HOOKFUNC_DEF(AboutHookFunc), NULL },
	{ { NULL, NULL }, HOOKFUNC_DEF(AppMessageHookFunc), NULL },
	{ { NULL, NULL }, HOOKFUNC_DEF(SettingsChangedHookFunc), NULL },
	{ { NULL, NULL }, HOOKFUNC_DEF(AslIntuiMsgHookFunc), NULL },

	{ { NULL, NULL }, HOOKFUNC_DEF(FrameTypeSelectHookFunc), NULL },
	{ { NULL, NULL }, HOOKFUNC_DEF(SelFrameTypeSelectHookFunc), NULL },
};

static struct Locale *PopupMenuPrefsLocale;
static struct Catalog *PopupMenuPrefsCatalog;

struct MUI_CustomClass *PopupMenuPrefsClass;
struct MUI_CustomClass *FrameButtonClass;
struct MUI_CustomClass *DataTypesImageClass;

static const LONG StopChunkList[] =
	{
	ID_PREF, ID_PMNU,
	};

static STRPTR ColorNames[] =
	{
	(STRPTR) MSGID_COLORNAME_TEXT,
	(STRPTR) MSGID_COLORNAME_BRIGHT_EDGES,
	(STRPTR) MSGID_COLORNAME_DARK_EDGES,
	(STRPTR) MSGID_COLORNAME_ACTIVE_ITEM_BG,
	(STRPTR) MSGID_COLORNAME_ACTIVE_ITEM_TEXT,
	(STRPTR) MSGID_COLORNAME_BACKGROUND,
	(STRPTR) MSGID_COLORNAME_MENUTITLES,
	(STRPTR) MSGID_COLORNAME_TEXT_SHADOW,
	NULL
	};

static STRPTR ColourMatchingPrecisionNames[] =
	{
	(STRPTR) MSGID_COLORMATCHING_EXACT,
	(STRPTR) MSGID_COLORMATCHING_IMAGE,
	(STRPTR) MSGID_COLORMATCHING_ICON,
	(STRPTR) MSGID_COLORMATCHING_GUI,
	NULL
	};

static STRPTR PrefsPageNames[] =
	{
//	  (STRPTR) MSGID_PREFSPAGES_COLOURS,
	(STRPTR) MSGID_PREFSPAGES_MISC,
//	  (STRPTR) MSGID_PREFSPAGES_IMAGES,
	(STRPTR) MSGID_PREFSPAGES_BORDERS,
	(STRPTR) MSGID_PREFSPAGES_SPACING,
	(STRPTR) MSGID_PREFSPAGES_TEXT,
	(STRPTR) MSGID_PREFSPAGES_PULLDOWN,
	(STRPTR) MSGID_PREFSPAGES_ADVANCED,
	(STRPTR) MSGID_PREFSPAGES_TRANSPARENCY,
	NULL
	};

static STRPTR PulldownModeNames[] =
	{
	(STRPTR) MSGID_PULLDOWN_ALWAYS,
	(STRPTR) MSGID_PULLDOWN_POPUP,
	(STRPTR) MSGID_PULLDOWN_POSITION_DEPENDENT,
	NULL
	};

static STRPTR AppearAtNames[] =
	{
	(STRPTR) MSGID_APPEAR_AT_SCREENBAR,
	(STRPTR) MSGID_APPEAR_AT_WINDOWBAR,
	NULL
	};

static STRPTR ImageSetNames[] =
	{
	(STRPTR) MSGID_IMAGESET_SYSTEM,
	(STRPTR) MSGID_IMAGESET_MAGICMENU_8,
	(STRPTR) MSGID_IMAGESET_MAGICMENU_11,
	(STRPTR) MSGID_IMAGESET_USER,
	(STRPTR) MSGID_IMAGESET_NONE,
	NULL
	};

static STRPTR AnimationNames[] =
	{
	(STRPTR) MSGID_ANIMATION_NONE,
	(STRPTR) MSGID_ANIMATION_ZOOM,
	(STRPTR) MSGID_ANIMATION_FADE,
	(STRPTR) MSGID_ANIMATION_EXPLODE,
	NULL
	};

static const struct PopupMenuPrefs DefaultPMPrefs = {
	1,			/* pmp_Flags		*/
	0,			/* pmp_SubMenuDelay	*/
	PMP_ANIM_NONE,		/* pmp_Animation	*/
	PMP_PD_SCREENBAR,	/* pmp_PulldownPos	*/
	FALSE,			/* pmp_Sticky		*/
	FALSE,			/* pmp_SameHeight	*/
	0,			/* pmp_MenuBorder	*/
	0,			/* pmp_SelItemBorder	*/
	0,			/* pmp_SeparatorBar	*/
	0,			/* pmp_MenuTitles	*/
	0,			/* pmp_MenuItems	*/
	2,			/* pmp_XOffset		*/
	2,			/* pmp_YOffset		*/
	2,			/* pmp_XSpace		*/
	2,			/* pmp_YSpace		*/
	2,			/* pmp_Intermediate	*/
	0,			/* pmp_TextDisplace	*/
	-30,			/* pmp_ShadowR		*/
	-30,			/* pmp_ShadowG		*/
	-30,			/* pmp_ShadowB		*/
	0,			/* pmp_TransparencyR	*/
	0,			/* pmp_TransparencyG	*/
	0,			/* pmp_TransparencyB	*/
	0,			/* pmp_TransparencyBlur	*/
	0,			/* pmp_AnimationSpeed	*/

	{0},			/* pmp_Reserved		*/
};

//----------------------------------------------------------------------------

VOID closePlugin(struct PluginBase *PluginBase)
{
	d1(kprintf("%s/%s/%ld: start\n", __FILE__, __FUNC__, __LINE__));

	if (DataTypesImageClass)
		{
		CleanupDtpicClass(DataTypesImageClass);
		DataTypesImageClass = NULL;
		}
	if (FrameButtonClass)
		{
		CleanupFrameButtonClass(FrameButtonClass);
		FrameButtonClass = NULL;
		}
	if (PopupMenuPrefsCatalog)
		{
		CloseCatalog(PopupMenuPrefsCatalog);
		PopupMenuPrefsCatalog = NULL;
		}
	if (PopupMenuPrefsLocale)
		{
		CloseLocale(PopupMenuPrefsLocale);
		PopupMenuPrefsLocale = NULL;
		}

	if (PopupMenuPrefsClass)
		{
		MUI_DeleteCustomClass(PopupMenuPrefsClass);
		PopupMenuPrefsClass = NULL;
		}

	CloseLibraries();

	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

#ifndef __amigaos4__
	_STD_240_TerminateMemFunctions();
#endif

	d1(kprintf("%s/%s/%ld: endt\n", __FILE__, __FUNC__, __LINE__));
}


LIBFUNC_P2(ULONG, LIBSCAGetPrefsInfo,
	D0, ULONG, which,
	A6, struct PluginBase *, PluginBase);
{
	ULONG result;

	d1(kprintf("%s/%s/%ld: which=%ld\n", __FILE__, __FUNC__, __LINE__, which));

	(void) PluginBase;

	switch(which)
		{
	case SCAPREFSINFO_GetClass:
		result = (ULONG) PopupMenuPrefsClass;
		break;

	case SCAPREFSINFO_GetTitle:
		result = (ULONG) GetLocString(MSGID_PLUGIN_LIST_TITLE);
		break;

	case SCAPREFSINFO_GetTitleImage:
		result = (ULONG) CreatePrefsImage();
		break;

	default:
		result = (ULONG) NULL;
		break;
		}

	d1(kprintf("%s/%s/%ld: result=%lu\n", __FILE__, __FUNC__, __LINE__, result));

	return result;
}
LIBFUNC_END

//----------------------------------------------------------------------------

DISPATCHER(PopupMenuPrefs)
{
	struct PopupMenuPrefsInst *inst;
	ULONG result = 0;

	switch(msg->MethodID)
		{
	case OM_NEW:
		obj = (Object *) DoSuperMethodA(cl, obj, msg);
		d1(kprintf("%s/%s/%ld: OM_NEW obj=%08lx\n", __FILE__, __FUNC__, __LINE__, obj));
		if (obj)
			{
			Object *prefsobject;
			struct opSet *ops = (struct opSet *) msg;

			inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);

			memset(inst, 0, sizeof(struct PopupMenuPrefsInst));

			inst->mpb_Changed = FALSE;
			inst->mpb_PMPrefs = DefaultPMPrefs;
			inst->mpb_WBScreen = LockPubScreen("Workbench");

			InitHooks(inst);

			inst->mpb_fCreateIcons = GetTagData(MUIA_ScalosPrefs_CreateIcons, TRUE, ops->ops_AttrList);
			inst->mpb_ProgramName = (CONST_STRPTR) GetTagData(MUIA_ScalosPrefs_ProgramName, (ULONG) "", ops->ops_AttrList);
			inst->mpb_Objects[OBJNDX_WIN_Main] = (APTR) GetTagData(MUIA_ScalosPrefs_MainWindow, (ULONG) NULL, ops->ops_AttrList);
			inst->mpb_Objects[OBJNDX_APP_Main] = (APTR) GetTagData(MUIA_ScalosPrefs_Application, (ULONG) NULL, ops->ops_AttrList);

			prefsobject = CreatePrefsGroup(inst);
			d1(kprintf("%s/%s/%ld: prefsobject=%08lx\n", __FILE__, __FUNC__, __LINE__, prefsobject));
			if (prefsobject)
				{
				DoMethod(obj, OM_ADDMEMBER, prefsobject);

				result = (ULONG) obj;
				}
			else
				{
				CoerceMethod(cl, obj, OM_DISPOSE);
				}
			}
		break;

	case OM_DISPOSE:
		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		d1(kprintf("%s/%s/%ld: OM_DISPOSE obj=%08lx\n", __FILE__, __FUNC__, __LINE__, obj));
		if (inst->mpb_WBScreen)
			{
			UnlockPubScreen(NULL, inst->mpb_WBScreen);
			inst->mpb_WBScreen = NULL;
			}
		if (inst->mpb_SaveReq)
			{
			MUI_FreeAslRequest(inst->mpb_SaveReq);
			inst->mpb_SaveReq = NULL;
			}
		if (inst->mpb_LoadReq)
			{
			MUI_FreeAslRequest(inst->mpb_LoadReq);
			inst->mpb_LoadReq = NULL;
			}
		return DoSuperMethodA(cl, obj, msg);
		break;

	case OM_SET:
		{
		struct opSet *ops = (struct opSet *) msg;

		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);

		inst->mpb_fCreateIcons = GetTagData(MUIA_ScalosPrefs_CreateIcons, inst->mpb_fCreateIcons, ops->ops_AttrList);
		inst->mpb_ProgramName = (CONST_STRPTR) GetTagData(MUIA_ScalosPrefs_ProgramName, (ULONG) inst->mpb_ProgramName, ops->ops_AttrList);
		inst->mpb_Objects[OBJNDX_WIN_Main] = (APTR) GetTagData(MUIA_ScalosPrefs_MainWindow, 
			(ULONG) inst->mpb_Objects[OBJNDX_WIN_Main], ops->ops_AttrList);
		inst->mpb_Objects[OBJNDX_APP_Main] = (APTR) GetTagData(MUIA_ScalosPrefs_Application, 
			(ULONG) inst->mpb_Objects[OBJNDX_APP_Main], ops->ops_AttrList);

		return DoSuperMethodA(cl, obj, msg);
		}
		break;

	case OM_GET:
		{
		struct opGet *opg = (struct opGet *) msg;

		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		switch (opg->opg_AttrID)
			{
		case MUIA_ScalosPrefs_CreateIcons:
			*opg->opg_Storage = inst->mpb_fCreateIcons;
			result = 0;
			break;
		default:
			result = DoSuperMethodA(cl, obj, msg);
			}
		}
		break;

	case MUIM_ScalosPrefs_ParseToolTypes:
		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		d1(kprintf(__FILE__ "/%s/%ld: before ParseToolTypes\n", __FUNC__, __LINE__));
		ParseToolTypes(inst, (struct MUIP_ScalosPrefs_ParseToolTypes *) msg);
		d1(kprintf(__FILE__ "/%s/%ld: after ParseToolTypes\n", __FUNC__, __LINE__));
		 break;

	case MUIM_ScalosPrefs_LoadConfig:
		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		ReadPrefsFile(inst, PMP_PATH, TRUE);
		PrefsToGUI(inst, &inst->mpb_PMPrefs);
		SetChangedFlag(inst, FALSE);
		break;

	case MUIM_ScalosPrefs_UseConfig:
		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		WritePrefsFile(inst, PMP_PATH);
		PM_ReloadPrefs();
		SetChangedFlag(inst, FALSE);
		break;

	case MUIM_ScalosPrefs_UseConfigIfChanged:
		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		if (inst->mpb_Changed)
			{
			WritePrefsFile(inst, PMP_PATH);
			PM_ReloadPrefs();
			SetChangedFlag(inst, FALSE);
			}
		break;

	case MUIM_ScalosPrefs_SaveConfig:
		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		WritePrefsFile(inst, PMP_S_PATH);
		WritePrefsFile(inst, PMP_PATH);
		PM_ReloadPrefs();
		SetChangedFlag(inst, FALSE);
		break;

	case MUIM_ScalosPrefs_SaveConfigIfChanged:
		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		if (inst->mpb_Changed)
			{
			WritePrefsFile(inst, PMP_S_PATH);
			WritePrefsFile(inst, PMP_PATH);
			PM_ReloadPrefs();
			SetChangedFlag(inst, FALSE);
			}
		break;

	case MUIM_ScalosPrefs_RestoreConfig:
		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		DoMethod(obj, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_Restore], 0);
		break;

	case MUIM_ScalosPrefs_ResetToDefaults:
		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		DoMethod(obj, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_ResetToDefaults], 0);
		break;

	case MUIM_ScalosPrefs_LastSavedConfig:
		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		DoMethod(obj, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_LastSaved], 0);
		break;

	case MUIM_ScalosPrefs_PageActive:
		{
		struct MUIP_ScalosPrefs_PageActive *spa = (struct MUIP_ScalosPrefs_PageActive *) msg;

		d1(kprintf("%s/%s/%ld: MUIM_ScalosPrefs_PageActive  isActive=%08lx\n", __FILE__, __FUNC__, __LINE__, spa->isActive));
		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		inst->mpb_PageIsActive = spa->isActive;
		}
		break;

	case MUIM_ScalosPrefs_OpenConfig:
		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		DoMethod(obj, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_Open], 0);
		break;

	case MUIM_ScalosPrefs_SaveConfigAs:
		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		DoMethod(obj, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_SaveAs], 0);
		break;

	case MUIM_ScalosPrefs_About:
		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		DoMethod(obj, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_About], 0);
		break;

	case MUIM_ScalosPrefs_LoadNamedConfig:
		{
		struct MUIP_ScalosPrefs_LoadNamedConfig *lnc = (struct MUIP_ScalosPrefs_LoadNamedConfig *) msg;

		inst = (struct PopupMenuPrefsInst *) INST_DATA(cl, obj);
		ReadPrefsFile(inst, lnc->ConfigFileName, FALSE);
		PrefsToGUI(inst, &inst->mpb_PMPrefs);
		}
		break;

	case MUIM_ScalosPrefs_CreateSubWindows:
		result = (ULONG) NULL;
		break;

	case MUIM_ScalosPrefs_GetListOfMCCs:
		result = (ULONG) RequiredMccList;
		break;

	default:
		d1(kprintf("%s/%s/%ld: MethodID=%08lx\n", __FILE__, __FUNC__, __LINE__, msg->MethodID));
		result = DoSuperMethodA(cl, obj, msg);
		break;
		}

	return result;
}
DISPATCHER_END

//----------------------------------------------------------------------------

static Object *CreatePrefsGroup(struct PopupMenuPrefsInst *inst)
{
	d1(kprintf("%s/%s/%ld:  inst=%08lx\n", __FILE__, __FUNC__, __LINE__, inst));

	inst->mpb_Objects[OBJNDX_Group_Main] = VGroup,
		MUIA_Background, MUII_PageBack,

		Child, RegisterObject,
			MUIA_Register_Titles, PrefsPageNames,
/*
			//------ Colours ----------------------
			Child, VGroup,
				MUIA_Background, MUII_PageBack,
				MUIA_FrameTitle, (ULONG) GetLocString(MSGID_COLOURSPAGE_TITLE),

				Child, HGroup,
					Child, inst->mpb_Objects[OBJNDX_Listview_ColorNames] = ListviewObject,
						InputListFrame,
						MUIA_CycleChain, TRUE,
						MUIA_Listview_List, ListObject,
							MUIA_List_SourceArray, ColorNames,
							End, //ListObject
						End, //ListviewObject

					Child, ColoradjustObject,
						End, //ColoradjustObject
					End, //HGroup

				Child, HGroup,
					Child, HVSpace,
					Child, inst->mpb_Objects[OBJNDX_CheckMark_UseDrawInfoPens] = CheckMarkHelp(TRUE, MSGID_COLOURSPAGE_USE_DRAWINFO_PENS_SHORTHELP),
					Child, LLabel1((ULONG) GetLocString(MSGID_COLOURSPAGE_USE_DRAWINFO_PENS)),
					Child, HVSpace,
					End, //HGroup
				
				End, //VGroup
*/
			//------ Misc. ----------------------
			Child, VGroup,
				MUIA_Background, MUII_PageBack,
				MUIA_FrameTitle, (ULONG) GetLocString(MSGID_MISCPAGE_TITLE),

				Child, HVSpace,

				Child, HGroup,
					Child, Label1((ULONG) GetLocString(MSGID_MISCPAGE_DELAY_SUBMENUS)),
					Child, inst->mpb_Objects[OBJNDX_Slider_DelaySubMenus] = SliderObject,
						MUIA_CycleChain, TRUE,
						MUIA_Numeric_Min, 0,
						MUIA_Numeric_Max, 10,
						MUIA_Slider_Horiz, TRUE,
						MUIA_Numeric_Value, 0,
						End, //SliderObject
					MUIA_ShortHelp, (ULONG) GetLocString(MSGID_MISCPAGE_DELAY_SUBMENUS_SHORTHELP),
					End, //HGroup

				Child, HVSpace,

				Child, VGroup,
					GroupFrame,
					MUIA_FrameTitle, (ULONG) GetLocString(MSGID_MISCPAGE_ANIMATION),

					Child, ColGroup(2),
						Child, Label1((ULONG) GetLocString(MSGID_MISCPAGE_ANIMATION_TYPE)),
						Child, inst->mpb_Objects[OBJNDX_Cycle_AnimationType] = CycleObject,
							MUIA_CycleChain, TRUE,
							MUIA_Cycle_Entries, AnimationNames,
							MUIA_ShortHelp, (ULONG) GetLocString(MSGID_MISCPAGE_ANIMATION_TYPE_SHORTHELP),
							End, //Cycle

						Child, Label1((ULONG) GetLocString(MSGID_MISCPAGE_ANIMATION_SPEED)),
						Child, inst->mpb_Objects[OBJNDX_Slider_AnimationSpeed] = SliderObject,
							MUIA_CycleChain, TRUE,
							MUIA_Numeric_Min, 0,
							MUIA_Numeric_Max, 100,
							MUIA_Slider_Horiz, TRUE,
							MUIA_Numeric_Value, 2,
							MUIA_ShortHelp, (ULONG) GetLocString(MSGID_MISCPAGE_ANIMATION_SPEED_SHORTHELP),
							End, //SliderObject
						End, //ColGroup
					End, //VGroup

				Child, HVSpace,


				Child, HVSpace,

				Child, ColGroup(4),

					Child, HVSpace,

					Child, inst->mpb_Objects[OBJNDX_CheckMark_SameHeight] = CheckMarkHelp(TRUE, MSGID_MISCPAGE_SAME_HEIGHT_SHORTHELP),
					Child, LLabel1((ULONG) GetLocString(MSGID_MISCPAGE_SAME_HEIGHT)),

					Child, HVSpace,
					Child, HVSpace,

					Child, inst->mpb_Objects[OBJNDX_CheckMark_MenuShadows] = CheckMarkHelp(TRUE, MSGID_MISCPAGE_MENUSHADOWS_SHORTHELP),
					Child, LLabel1((ULONG) GetLocString(MSGID_MISCPAGE_MENUSHADOWS)),

					Child, HVSpace,
					Child, HVSpace,

					Child, inst->mpb_Objects[OBJNDX_CheckMark_Sticky] = CheckMarkHelp(TRUE, MSGID_MISCPAGE_STICKY_SHORTHELP),
					Child, LLabel1((ULONG) GetLocString(MSGID_MISCPAGE_STICKY)),

					Child, HVSpace,

					End, //ColGroup

				Child, HVSpace,
				End, //VGroup
/*
			//------ Images ----------------------
			Child, VGroup,
				MUIA_Background, MUII_PageBack,
				MUIA_FrameTitle, (ULONG) GetLocString(MSGID_IMAGESPAGE_TITLE),

				Child, HVSpace,

				Child, inst->mpb_Objects[OBJNDX_Cycle_ImageSet] = CycleObject,
					MUIA_CycleChain, TRUE,
					MUIA_Cycle_Entries, ImageSetNames,
					MUIA_ShortHelp, (ULONG) GetLocString(MSGID_IMAGEPAGE_IMAGESET_SHORTHELP),
					End, //Cycle

				Child, HVSpace,

				Child, inst->mpb_Objects[OBJNDX_PopAsl_LoadImageSet] = PopaslObject,
					MUIA_CycleChain, TRUE,
					MUIA_ShortHelp, GetLocString(MSGID_POPASL_LOADIMAGESET_SHORTHELP),
					MUIA_Popasl_Type, ASL_FileRequest,
//					  MUIA_Popasl_StartHook, &inst->mpb_Hooks[HOOKNDX_ImagePopAslFileStart],
					MUIA_Popstring_String, MUI_MakeObject(MUIO_String, NULL, 256),
					MUIA_Popstring_Button, PopButton(MUII_PopFile),
					End, //PopaslObject

				Child, HVSpace,
				End, //VGroup
*/
			//------ Borders ----------------------
			Child, VGroup,
				MUIA_Background, MUII_PageBack,
				MUIA_FrameTitle, (ULONG) GetLocString(MSGID_BORDERSPAGE_TITLE),

				Child, HVSpace,

				Child, HGroup,
					GroupFrame,
					MUIA_FrameTitle, (ULONG) GetLocString(MSGID_BORDERSPAGE_MENUBORDER),

					Child, FrameButton(OBJNDX_Button_FrameThin, PMP_MENUBORDER_THIN, "", FALSE, FALSE),
					Child, FrameButton(OBJNDX_Button_FrameMM, PMP_MENUBORDER_MM, "", FALSE, FALSE),
					Child, FrameButton(OBJNDX_Button_FrameThick, PMP_MENUBORDER_THICK, "", FALSE, FALSE),
					Child, FrameButton(OBJNDX_Button_FrameRidge, PMP_MENUBORDER_RIDGE, "", FALSE, FALSE),
					Child, FrameButton(OBJNDX_Button_FrameDropBox, PMP_MENUBORDER_DROPBOX, "", FALSE, FALSE),
					Child, FrameButton(OBJNDX_Button_FrameOldStyle, PMP_MENUBORDER_OLDSTYLE, "", FALSE, FALSE),
					End, //HGroup

				Child, HVSpace,

				Child, HGroup,
					GroupFrame,
					MUIA_FrameTitle, (ULONG) GetLocString(MSGID_BORDERSPAGE_SELECTEDITEM),

					Child, FrameButton(OBJNDX_Button_Normal, PMP_MENUBORDER_NONE, GetLocString(MSGID_BORDERSPAGE_NORMAL), TRUE, FALSE),
					Child, FrameButton(OBJNDX_Button_Raised, PMP_MENUBORDER_THIN, GetLocString(MSGID_BORDERSPAGE_RAISED), TRUE, TRUE),
					Child, FrameButton(OBJNDX_Button_Recessed, PMP_MENUBORDER_THIN, GetLocString(MSGID_BORDERSPAGE_RECESSED), TRUE, FALSE),
					End, //HGroup

				Child, HVSpace,

				Child, HGroup,
					GroupFrame,
					MUIA_FrameTitle, (ULONG) GetLocString(MSGID_BORDERSPAGE_SEPARATORS),

					Child, HVSpace,

					Child, inst->mpb_Objects[OBJNDX_Button_NewLook] = TextObject,
						ButtonFrame,
						MUIA_Text_Contents, GetLocString(MSGID_BORDERSPAGE_NEW_LOOK),\
						MUIA_Text_PreParse, "\33c",\
						MUIA_Font, MUIV_Font_Button,
						MUIA_CycleChain, TRUE,
						MUIA_InputMode, MUIV_InputMode_Immediate,
						MUIA_Background, MUII_ButtonBack,
						End,
					Child, inst->mpb_Objects[OBJNDX_Button_OldLook] = TextObject,
						ButtonFrame,
						MUIA_Text_Contents, GetLocString(MSGID_BORDERSPAGE_OLD_LOOK),\
						MUIA_Text_PreParse, "\33c",\
						MUIA_Font, MUIV_Font_Button,
						MUIA_CycleChain, TRUE,
						MUIA_InputMode, MUIV_InputMode_Immediate,
						MUIA_Background, MUII_ButtonBack,
						End,

					Child, HVSpace,
					End, //HGroup

				Child, HVSpace,
				End, //VGroup

			//------ Spacing ----------------------
			Child, VGroup,
				MUIA_Background, MUII_PageBack,
				MUIA_FrameTitle, (ULONG) GetLocString(MSGID_SPACINGPAGE_TITLE),

				Child, HVSpace,
				
				Child, ColGroup(5),
					Child, HVSpace,

					Child, VGroup,
						Child, HVSpace,
						Child, IMG(Horizontal_Space, HORIZONTAL_SPACE),
						Child, inst->mpb_Objects[OBJNDX_Slider_HorizontalSpacing] = SliderObject,
							MUIA_CycleChain, TRUE,
							MUIA_Numeric_Min, 0,
							MUIA_Numeric_Max, 10,
							MUIA_Slider_Horiz, TRUE,
							MUIA_Numeric_Value, 2,
							End, //SliderObject
						Child, HVSpace,
						End, //VGroup

					Child, HVSpace,

					Child, VGroup,
						Child, HVSpace,
						Child, IMG(Vertical_Space, VERTICAL_SPACE),
						Child, inst->mpb_Objects[OBJNDX_Slider_VerticalSpacing] = SliderObject,
							MUIA_CycleChain, TRUE,
							MUIA_Numeric_Min, 0,
							MUIA_Numeric_Max, 10,
							MUIA_Slider_Horiz, TRUE,
							MUIA_Numeric_Value, 1,
							End, //SliderObject
						Child, HVSpace,
						End, //VGroup

					Child, HVSpace,
					Child, HVSpace,

					Child, VGroup,
						Child, HVSpace,
						Child, IMG(Horizontal, HORIZONTAL),
						Child, inst->mpb_Objects[OBJNDX_Slider_Horizontal] = SliderObject,
							MUIA_CycleChain, TRUE,
							MUIA_Numeric_Min, 0,
							MUIA_Numeric_Max, 10,
							MUIA_Slider_Horiz, TRUE,
							MUIA_Numeric_Value, 0,
							End, //SliderObject
						Child, HVSpace,
						End, //VGroup

					Child, HVSpace,

					Child, VGroup,
						Child, HVSpace,
						Child, IMG(Vertical_Offset, VERTICAL_OFFSET),
						Child, inst->mpb_Objects[OBJNDX_Slider_VerticalOffset] = SliderObject,
							MUIA_CycleChain, TRUE,
							MUIA_Numeric_Min, 0,
							MUIA_Numeric_Max, 10,
							MUIA_Slider_Horiz, TRUE,
							MUIA_Numeric_Value, 0,
							End, //SliderObject
						Child, HVSpace,
						End, //VGroup

					Child, HVSpace,
					Child, HVSpace,

					Child, VGroup,
						Child, HVSpace,
						Child, IMG(Intermediate_Spacing, INTERMEDIATE_SPACING),
						Child, inst->mpb_Objects[OBJNDX_Slider_IntermediateSpacing] = SliderObject,
							MUIA_CycleChain, TRUE,
							MUIA_Numeric_Min, 0,
							MUIA_Numeric_Max, 10,
							MUIA_Slider_Horiz, TRUE,
							MUIA_Numeric_Value, 0,
							End, //SliderObject
						Child, HVSpace,
						End, //VGroup

					Child, HVSpace,

					Child, VGroup,
						Child, HVSpace,
						Child, IMG(Text_Displacement, TEXT_DISPLACEMENT),
						Child, inst->mpb_Objects[OBJNDX_Slider_TextDisplacement] = SliderObject,
							MUIA_CycleChain, TRUE,
							MUIA_Numeric_Min, -5,
							MUIA_Numeric_Max, 5,
							MUIA_Slider_Horiz, TRUE,
							MUIA_Numeric_Value, 0,
							End, //SliderObject
						Child, HVSpace,
						End, //VGroup

					Child, HVSpace,
					End, //ColGroup

				Child, HVSpace,
                                End, //VGroup

			//------ Text ----------------------
			Child, VGroup,
				MUIA_Background, MUII_PageBack,
				MUIA_FrameTitle, (ULONG) GetLocString(MSGID_TEXTPAGE_TITLE),

				Child, HVSpace,

				Child, ColGroup(4),
					GroupFrame,
					MUIA_FrameTitle, (ULONG) GetLocString(MSGID_TEXTPAGE_MENU_TITLES),

					Child, ColGroup(2),
						Child, inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Italic] = CheckMarkHelp(TRUE, MSGID_TEXTPAGE_MENUTITLES_ITALIC_SHORTHELP),
						Child, LLabel1((ULONG) GetLocString(MSGID_TEXTPAGE_MENUTITLES_ITALIC)),

						Child, inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Underlined] = CheckMarkHelp(TRUE, MSGID_TEXTPAGE_MENUTITLES_UNDERLINED_SHORTHELP),
						Child, LLabel1((ULONG) GetLocString(MSGID_TEXTPAGE_MENUTITLES_UNDERLINED)),

						Child, inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Bold] = CheckMarkHelp(TRUE, MSGID_TEXTPAGE_MENUTITLES_BOLD_SHORTHELP),
						Child, LLabel1((ULONG) GetLocString(MSGID_TEXTPAGE_MENUTITLES_BOLD)),

						End, //ColGroup

					Child, HVSpace,

					Child, ColGroup(2),
						Child, inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Shadowed] = CheckMarkHelp(TRUE, MSGID_TEXTPAGE_MENUTITLES_SHADOWED_SHORTHELP),
						Child, LLabel1((ULONG) GetLocString(MSGID_TEXTPAGE_MENUTITLES_SHADOWED)),

						Child, inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Outlined] = CheckMarkHelp(TRUE, MSGID_TEXTPAGE_MENUTITLES_OUTLINED_SHORTHELP),
						Child, LLabel1((ULONG) GetLocString(MSGID_TEXTPAGE_MENUTITLES_OUTLINED)),

						Child, inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Embossed] = CheckMarkHelp(TRUE, MSGID_TEXTPAGE_MENUTITLES_EMBOSSED_SHORTHELP),
						Child, LLabel1((ULONG) GetLocString(MSGID_TEXTPAGE_MENUTITLES_EMBOSSED)),

						End, //ColGroup

					Child, HVSpace,

					End, //ColGroup

				Child, ColGroup(4),
					GroupFrame,
					MUIA_FrameTitle, (ULONG) GetLocString(MSGID_TEXTPAGE_MENU_ITEMS),

					Child, ColGroup(2),
						Child, inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Italic] = CheckMarkHelp(TRUE, MSGID_TEXTPAGE_MENUITEMS_ITALIC_SHORTHELP),
						Child, LLabel1((ULONG) GetLocString(MSGID_TEXTPAGE_MENUITEMS_ITALIC)),

						Child, inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Underlined] = CheckMarkHelp(TRUE, MSGID_TEXTPAGE_MENUITEMS_UNDERLINED_SHORTHELP),
						Child, LLabel1((ULONG) GetLocString(MSGID_TEXTPAGE_MENUITEMS_UNDERLINED)),

						Child, inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Bold] = CheckMarkHelp(TRUE, MSGID_TEXTPAGE_MENUITEMS_BOLD_SHORTHELP),
						Child, LLabel1((ULONG) GetLocString(MSGID_TEXTPAGE_MENUITEMS_BOLD)),

						End, //ColGroup

					Child, HVSpace,

					Child, ColGroup(2),
						Child, inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Shadowed] = CheckMarkHelp(TRUE, MSGID_TEXTPAGE_MENUITEMS_SHADOWED_SHORTHELP),
						Child, LLabel1((ULONG) GetLocString(MSGID_TEXTPAGE_MENUITEMS_SHADOWED)),

						Child, inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Outlined] = CheckMarkHelp(TRUE, MSGID_TEXTPAGE_MENUITEMS_OUTLINED_SHORTHELP),
						Child, LLabel1((ULONG) GetLocString(MSGID_TEXTPAGE_MENUITEMS_OUTLINED)),

						Child, inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Embossed] = CheckMarkHelp(TRUE, MSGID_TEXTPAGE_MENUITEMS_EMBOSSED_SHORTHELP),
						Child, LLabel1((ULONG) GetLocString(MSGID_TEXTPAGE_MENUITEMS_EMBOSSED)),

						End, //ColGroup

					Child, HVSpace,

					End, //ColGroup

				Child, HVSpace,

				End, //VGroup

			//------ Pulldown ----------------------
			Child, VGroup,
				MUIA_FrameTitle, (ULONG) GetLocString(MSGID_PULLDOWNPAGE_TITLE),

				Child, HVSpace,

				Child, ColGroup(2),
					Child, Label1((ULONG) GetLocString(MSGID_PULLDOWNPAGE_MODE)),
					Child, inst->mpb_Objects[OBJNDX_Cycle_PulldownMode] = CycleObject,
						MUIA_CycleChain, TRUE,
						MUIA_Cycle_Entries, PulldownModeNames,
						MUIA_ShortHelp, (ULONG) GetLocString(MSGID_PULLDOWNPAGE_MODE_SHORTHELP),
						End, //Cycle

					Child, Label1((ULONG) GetLocString(MSGID_PULLDOWNPAGE_APPEAR_AT)),
					Child, inst->mpb_Objects[OBJNDX_Cycle_PulldownLocation] = CycleObject,
						MUIA_CycleChain, TRUE,
						MUIA_Cycle_Entries, AppearAtNames,
						MUIA_ShortHelp, (ULONG) GetLocString(MSGID_PULLDOWNPAGE_APPEAR_AT_SHORTHELP),
						End, //Cycle
					End, //ColGroup

				Child, HVSpace,
				
                                End, //VGroup

			//------ Advanced ----------------------
			Child, VGroup,
				MUIA_FrameTitle, (ULONG) GetLocString(MSGID_ADVANCEDPAGE_TITLE),

				Child, HVSpace,

				Child, HGroup,
					Child, Label1((ULONG) GetLocString(MSGID_ADVANCEDPAGE_COLOUR_MATCHING_PRECISION)),
					Child, inst->mpb_Objects[OBJNDX_Cycle_ColourPrecision] = CycleObject,
						MUIA_CycleChain, TRUE,
						MUIA_Cycle_Entries, ColourMatchingPrecisionNames,
						MUIA_ShortHelp, (ULONG) GetLocString(MSGID_PULLDOWNPAGE_MODE_SHORTHELP),
						End, //CycleObject
					MUIA_ShortHelp, (ULONG) GetLocString(MSGID_ADVANCEDPAGE_COLOUR_MATCHING_PRECISION_SHORTHELP),
					End, //HGroup

				Child, HVSpace,

				Child, ColGroup(3),

					Child, inst->mpb_Objects[OBJNDX_CheckMark_UseWindows] = CheckMarkHelp(TRUE, MSGID_ADVANCEDPAGE_USE_WINDOWS_SHORTHELP),
					Child, LLabel1((ULONG) GetLocString(MSGID_ADVANCEDPAGE_USE_WINDOWS)),

					Child, HVSpace,

					Child, inst->mpb_Objects[OBJNDX_CheckMark_RealShadows] = CheckMarkHelp(TRUE, MSGID_ADVANCEDPAGE_REALSHADOWS_SHORTHELP),
					Child, LLabel1((ULONG) GetLocString(MSGID_ADVANCEDPAGE_REALSHADOWS)),

					Child, HVSpace,

					End, //ColGroup

				Child, HVSpace,

				Child, ColGroup(2),
					MUIA_FrameTitle, (ULONG) GetLocString(MSGID_ADVANCEDPAGE_SHADOW_INTENSITY),
					GroupFrame,

					Child, Label1((ULONG) GetLocString(MSGID_ADVANCEDPAGE_SHADOW_R)),
					Child, inst->mpb_Objects[OBJNDX_Slider_ShadowR] = SliderObject,
						MUIA_CycleChain, TRUE,
						MUIA_Numeric_Min, -100,
						MUIA_Numeric_Max, 100,
						MUIA_Slider_Horiz, TRUE,
						MUIA_Numeric_Value, -16,
						MUIA_Numeric_Format, (ULONG) GetLocString(MSGID_ADVANCEDPAGE_SHADOW_NUMERIC_FORMAT),
						MUIA_ShortHelp, (ULONG) GetLocString(MSGID_ADVANCEDPAGE_SHADOW_R_SHORTHELP),
						End, //SliderObject
					Child, Label1((ULONG) GetLocString(MSGID_ADVANCEDPAGE_SHADOW_G)),
					Child, inst->mpb_Objects[OBJNDX_Slider_ShadowG] = SliderObject,
						MUIA_CycleChain, TRUE,
						MUIA_Numeric_Min, -100,
						MUIA_Numeric_Max, 100,
						MUIA_Slider_Horiz, TRUE,
						MUIA_Numeric_Value, -16,
						MUIA_Numeric_Format, (ULONG) GetLocString(MSGID_ADVANCEDPAGE_SHADOW_NUMERIC_FORMAT),
						MUIA_ShortHelp, (ULONG) GetLocString(MSGID_ADVANCEDPAGE_SHADOW_G_SHORTHELP),
						End, //SliderObject
					Child, Label1((ULONG) GetLocString(MSGID_ADVANCEDPAGE_SHADOW_B)),
					Child, inst->mpb_Objects[OBJNDX_Slider_ShadowB] = SliderObject,
						MUIA_CycleChain, TRUE,
						MUIA_Numeric_Min, -100,
						MUIA_Numeric_Max, 100,
						MUIA_Slider_Horiz, TRUE,
						MUIA_Numeric_Value, -16,
						MUIA_Numeric_Format, (ULONG) GetLocString(MSGID_ADVANCEDPAGE_SHADOW_NUMERIC_FORMAT),
						MUIA_ShortHelp, (ULONG) GetLocString(MSGID_ADVANCEDPAGE_SHADOW_B_SHORTHELP),
						End, //SliderObject
					End, //ColGroup

				Child, HVSpace,

				End, //VGroup

			//------ Transparency ----------------------
			Child, VGroup,
				MUIA_Background, MUII_PageBack,
				MUIA_FrameTitle, (ULONG) GetLocString(MSGID_TRANSPARENCYPAGE_TITLE),

				Child, HVSpace,

				Child, ColGroup(2),
					GroupFrame,
					MUIA_FrameTitle, (ULONG) GetLocString(MSGID_TRANSPARENCYPAGE_TRANSPARENCY),

					Child, Label1((ULONG) GetLocString(MSGID_TRANSPARENCYPAGE_TRANSP_R)),
					Child, inst->mpb_Objects[OBJNDX_Slider_TranspR] = SliderObject,
						MUIA_CycleChain, TRUE,
						MUIA_Numeric_Min, -100,
						MUIA_Numeric_Max, 100,
						MUIA_Slider_Horiz, TRUE,
						MUIA_Numeric_Value, -16,
						MUIA_Numeric_Format, (ULONG) GetLocString(MSGID_TRANSPARENCYPAGE_TRANSP_NUMERIC_FORMAT),
						MUIA_ShortHelp, (ULONG) GetLocString(MSGID_TRANSPARENCYPAGE_TRANSP_R_SHORTHELP),
						End, //SliderObject

					Child, Label1((ULONG) GetLocString(MSGID_TRANSPARENCYPAGE_TRANSP_G)),
					Child, inst->mpb_Objects[OBJNDX_Slider_TranspG] = SliderObject,
						MUIA_CycleChain, TRUE,
						MUIA_Numeric_Min, -100,
						MUIA_Numeric_Max, 100,
						MUIA_Slider_Horiz, TRUE,
						MUIA_Numeric_Value, -16,
						MUIA_Numeric_Format, (ULONG) GetLocString(MSGID_TRANSPARENCYPAGE_TRANSP_NUMERIC_FORMAT),
						MUIA_ShortHelp, (ULONG) GetLocString(MSGID_TRANSPARENCYPAGE_TRANSP_G_SHORTHELP),
						End, //SliderObject

					Child, Label1((ULONG) GetLocString(MSGID_TRANSPARENCYPAGE_TRANSP_B)),
					Child, inst->mpb_Objects[OBJNDX_Slider_TranspB] = SliderObject,
						MUIA_CycleChain, TRUE,
						MUIA_Numeric_Min, -100,
						MUIA_Numeric_Max, 100,
						MUIA_Slider_Horiz, TRUE,
						MUIA_Numeric_Value, -16,
						MUIA_Numeric_Format, (ULONG) GetLocString(MSGID_TRANSPARENCYPAGE_TRANSP_NUMERIC_FORMAT),
						MUIA_ShortHelp, (ULONG) GetLocString(MSGID_TRANSPARENCYPAGE_TRANSP_B_SHORTHELP),
						End, //SliderObject
					End, //ColGroup

				Child, HVSpace,

				Child, ColGroup(2),
					Child, Label1((ULONG) GetLocString(MSGID_TRANSPARENCYPAGE_BLUR)),
					Child, inst->mpb_Objects[OBJNDX_Slider_Transp_Blur] = SliderObject,
						MUIA_CycleChain, TRUE,
						MUIA_Numeric_Min, 0,
						MUIA_Numeric_Max, 100,
						MUIA_Slider_Horiz, TRUE,
						MUIA_Numeric_Value, -16,
						MUIA_Numeric_Format, (ULONG) GetLocString(MSGID_TRANSPARENCYPAGE_BLUR_NUMERIC_FORMAT),
						MUIA_ShortHelp, (ULONG) GetLocString(MSGID_TRANSPARENCYPAGE_BLUR_SHORTHELP),
						End, //SliderObject
					End, //ColGroup

				Child, HVSpace,

				End, //VGroup

                        End, //RegisterObject

		End; //VGroup

	if (NULL == inst->mpb_Objects[OBJNDX_Group_Main])
		return NULL;

	PrefsToGUI(inst, &inst->mpb_PMPrefs);

	DoMethod(inst->mpb_Objects[OBJNDX_Group_Main], MUIM_Notify, MUIA_AppMessage, MUIV_EveryTime,
		inst->mpb_Objects[OBJNDX_Group_Main], 3, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_AppMessage], MUIV_TriggerValue);

	DoMethod(inst->mpb_Objects[OBJNDX_Button_FrameThin], MUIM_Notify, MUIA_Selected, TRUE,
		inst->mpb_Objects[OBJNDX_Button_FrameThin], 3, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_FrameSelected], MUIV_TriggerValue);

	// Mutual exclusion for frame type buttons
	DoMethod(inst->mpb_Objects[OBJNDX_Button_FrameMM], MUIM_Notify, MUIA_Selected, TRUE,
		inst->mpb_Objects[OBJNDX_Button_FrameMM], 3, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_FrameSelected], MUIV_TriggerValue);
	DoMethod(inst->mpb_Objects[OBJNDX_Button_FrameThick], MUIM_Notify, MUIA_Selected, TRUE,
		inst->mpb_Objects[OBJNDX_Button_FrameThick], 3, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_FrameSelected], MUIV_TriggerValue);
	DoMethod(inst->mpb_Objects[OBJNDX_Button_FrameRidge], MUIM_Notify, MUIA_Selected, TRUE,
		inst->mpb_Objects[OBJNDX_Button_FrameRidge], 3, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_FrameSelected], MUIV_TriggerValue);
	DoMethod(inst->mpb_Objects[OBJNDX_Button_FrameDropBox], MUIM_Notify, MUIA_Selected, TRUE,
		inst->mpb_Objects[OBJNDX_Button_FrameDropBox], 3, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_FrameSelected], MUIV_TriggerValue);
	DoMethod(inst->mpb_Objects[OBJNDX_Button_FrameOldStyle], MUIM_Notify, MUIA_Selected, TRUE,
		inst->mpb_Objects[OBJNDX_Button_FrameOldStyle], 3, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_FrameSelected], MUIV_TriggerValue);

	// Mutual exclusion for selected frame type buttons
	DoMethod(inst->mpb_Objects[OBJNDX_Button_Normal], MUIM_Notify, MUIA_Selected, TRUE,
		inst->mpb_Objects[OBJNDX_Button_Normal], 3, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_SelFrameSelected], MUIV_TriggerValue);
	DoMethod(inst->mpb_Objects[OBJNDX_Button_Raised], MUIM_Notify, MUIA_Selected, TRUE,
		inst->mpb_Objects[OBJNDX_Button_Raised], 3, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_SelFrameSelected], MUIV_TriggerValue);
	DoMethod(inst->mpb_Objects[OBJNDX_Button_Recessed], MUIM_Notify, MUIA_Selected, TRUE,
		inst->mpb_Objects[OBJNDX_Button_Recessed], 3, MUIM_CallHook, &inst->mpb_Hooks[HOOKNDX_SelFrameSelected], MUIV_TriggerValue);

	// Mutual exclusion for separator style buttons
	DoMethod(inst->mpb_Objects[OBJNDX_Button_NewLook], MUIM_Notify, MUIA_Selected, TRUE,
		inst->mpb_Objects[OBJNDX_Button_OldLook], 3, MUIM_Set, MUIA_Selected, FALSE);
	DoMethod(inst->mpb_Objects[OBJNDX_Button_OldLook], MUIM_Notify, MUIA_Selected, TRUE,
		inst->mpb_Objects[OBJNDX_Button_NewLook], 3, MUIM_Set, MUIA_Selected, FALSE);

	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));
	d1(kprintf("%s/%s/%ld:  LocaleBase=%08lx  IFFParseBase=%08lx\n", __FILE__, __FUNC__, __LINE__, LocaleBase, IFFParseBase));

	return inst->mpb_Objects[OBJNDX_Group_Main];
}


static BOOL OpenLibraries(void)
{
	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	DOSBase = (APTR) OpenLibrary( "dos.library", 39 );
	if (NULL == DOSBase)
		return FALSE;
#ifdef __amigaos4__
	IDOS = (struct DOSIFace *)GetInterface((struct Library *)DOSBase, "main", 1, NULL);
	if (NULL == IDOS)
		return FALSE;
#endif

	IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 39);
	if (NULL == IntuitionBase)
		return FALSE;
#ifdef __amigaos4__
	IIntuition = (struct IntuitionIFace *)GetInterface((struct Library *)IntuitionBase, "main", 1, NULL);
	if (NULL == IIntuition)
		return FALSE;
#endif

	MUIMasterBase = OpenLibrary("zune.library", 0);
	if (NULL == MUIMasterBase)
		MUIMasterBase = OpenLibrary(MUIMASTER_NAME, MUIMASTER_VMIN-1);
	if (NULL == MUIMasterBase)
		return FALSE;
#ifdef __amigaos4__
	IMUIMaster = (struct MUIMasterIFace *)GetInterface((struct Library *)MUIMasterBase, "main", 1, NULL);
	if (NULL == IMUIMaster)
		return FALSE;
#endif

	IFFParseBase = OpenLibrary("iffparse.library", 36);
	if (NULL == IFFParseBase)
		return FALSE;
#ifdef __amigaos4__
	IIFFParse = (struct IFFParseIFace *)GetInterface((struct Library *)IFFParseBase, "main", 1, NULL);
	if (NULL == IIFFParse)
		return FALSE;
#endif

	IconBase = OpenLibrary("icon.library", 0);
	if (NULL == IconBase)
		return FALSE;
#ifdef __amigaos4__
	IIcon = (struct IconIFace *)GetInterface((struct Library *)IconBase, "main", 1, NULL);
	if (NULL == IIcon)
		return FALSE;
#endif

	GfxBase = (struct GfxBase *) OpenLibrary(GRAPHICSNAME, 39);
	if (NULL == GfxBase)
		return FALSE;
#ifdef __amigaos4__
	IGraphics = (struct GraphicsIFace *)GetInterface((struct Library *)GfxBase, "main", 1, NULL);
	if (NULL == IGraphics)
		return FALSE;
#endif

	UtilityBase = (T_UTILITYBASE) OpenLibrary(UTILITYNAME, 39);
	if (NULL == UtilityBase)
		return FALSE;
#ifdef __amigaos4__
	IUtility = (struct UtilityIFace *)GetInterface((struct Library *)UtilityBase, "main", 1, NULL);
	if (NULL == IUtility)
		return FALSE;
#endif

	PopupMenuBase = (struct PopupMenuBase *) OpenLibrary(POPUPMENU_NAME, POPUPMENU_VERSION);
	if (NULL == PopupMenuBase)
		return FALSE;

	DataTypesBase = OpenLibrary("datatypes.library", 39);
	if (NULL == DataTypesBase)
		return FALSE;
#ifdef __amigaos4__
	IDataTypes = (struct DataTypesIFace *)GetInterface((struct Library *)DataTypesBase, "main", 1, NULL);
	if (NULL == IDataTypes)
		return FALSE;
#endif

#ifdef __amigaos4__
	NewlibBase = OpenLibrary("newlib.library", 0);
	if (NULL == NewlibBase)
		return FALSE;
	INewlib = GetInterface(NewlibBase, "main", 1, NULL);
	if (NULL == INewlib)
		return FALSE;
#endif

	// LocaleBase is optional
	LocaleBase = (T_LOCALEBASE) OpenLibrary("locale.library", 39);
#ifdef __amigaos4__
	if (LocaleBase)
		ILocale = (struct LocaleIFace *)GetInterface((struct Library *)LocaleBase, "main", 1, NULL);
#endif

#ifdef __amigaos4__
	if (PopupMenuBase)
		IPopupMenu = (struct PopupMenuIFace *)GetInterface((struct Library *)PopupMenuBase, "main", 1, NULL);
#endif

	d1(kprintf("%s/%s/%ld:  LocaleBase=%08lx  IFFParseBase=%08lx\n", __FILE__, __FUNC__, __LINE__, LocaleBase, IFFParseBase));

	return TRUE;
}


static void CloseLibraries(void)
{
	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

#ifdef __amigaos4__
	if (INewlib)
		{
		DropInterface(INewlib);
		INewlib = NULL;
		}
	if (NewlibBase)
		{
		CloseLibrary(NewlibBase);
		NewlibBase = NULL;
		}
	if (IDOS)
		{
		DropInterface((struct Interface *)IDOS);
		IDOS = NULL;
		}
#endif
	if (DOSBase)
		{
		CloseLibrary((struct Library *) DOSBase);
		DOSBase = NULL;
		}
#ifdef __amigaos4__
	if (IDataTypes)
		{
		DropInterface((struct Interface *)IDataTypes);
		IDataTypes = NULL;
		}
#endif
	if (DataTypesBase)
		{
		CloseLibrary(DataTypesBase);
		DataTypesBase = NULL;
		}
#ifdef __amigaos4__
	if (ILocale)
		{
		DropInterface((struct Interface *)ILocale);
		ILocale = NULL;
		}
#endif
	if (LocaleBase)
		{
		CloseLibrary((struct Library *) LocaleBase);
		LocaleBase = NULL;
		}
#ifdef __amigaos4__
	if (IUtility)
		{
		DropInterface((struct Interface *)IUtility);
		IUtility = NULL;
		}
#endif
	if (UtilityBase)
		{
		CloseLibrary((struct Library *) UtilityBase);
		UtilityBase = NULL;
		}
#ifdef __amigaos4__
	if (IGraphics)
		{
		DropInterface((struct Interface *)IGraphics);
		IGraphics = NULL;
		}
#endif
	if (GfxBase)
		{
		CloseLibrary((struct Library *) GfxBase);
		GfxBase = NULL;
		}
#ifdef __amigaos4__
	if (IIcon)
		{
		DropInterface((struct Interface *)IIcon);
		IIcon = NULL;
		}
#endif
	if (IconBase)
		{
		CloseLibrary(IconBase);
		IconBase = NULL;
		}
#ifdef __amigaos4__
	if (IIFFParse)
		{
		DropInterface((struct Interface *)IIFFParse);
		IIFFParse = NULL;
		}
#endif
	if (IFFParseBase)
		{
		CloseLibrary(IFFParseBase);
		IFFParseBase = NULL;
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
#ifdef __amigaos4__
	if (IPopupMenu)
		{
		DropInterface((struct Interface *)IPopupMenu);
		IPopupMenu = NULL;
		}
#endif
	if (PopupMenuBase)
		{
		CloseLibrary((struct Library *) PopupMenuBase);
		PopupMenuBase = NULL;
		}
}

//----------------------------------------------------------------------------

static SAVEDS(ULONG) INTERRUPT ImagePopAslFileStartHookFunc(struct Hook *hook, Object *o, Msg msg)
{
	struct TagItem *TagList = (struct TagItem *) msg;
//	  struct PopupMenuPrefsInst *inst = (struct FileTypesPrefsInst *) hook->h_Data;

	d1(KPrintF("%s/%ld: inst=%08lx\n", __FUNC__, __LINE__, inst));

	while (TAG_END != TagList->ti_Tag)
		TagList++;

	TagList->ti_Tag = ASLFR_DrawersOnly;
	TagList->ti_Data = FALSE;
	TagList++;

//	  TagList->ti_Tag = ASLFR_InitialFile;
//	  TagList->ti_Data = (ULONG) inst->fpb_FileName;
//	  TagList++;

	TagList->ti_Tag = ASLFR_InitialDrawer;
	TagList->ti_Data = (ULONG) "THEME:FileTypes/";
	TagList++;

	TagList->ti_Tag = TAG_END;

	return TRUE;
}

//----------------------------------------------------------------------------

static STRPTR GetLocString(ULONG MsgId)
{
	struct LocaleInfo li;

	li.li_Catalog = PopupMenuPrefsCatalog;	     
#ifndef __amigaos4__
	li.li_LocaleBase = LocaleBase;
#else
	li.li_ILocale = ILocale;
#endif

	return (STRPTR) GetString(&li, MsgId);
}

static void TranslateStringArray(STRPTR *stringArray)
{
	while (*stringArray)
		{
		*stringArray = GetLocString((ULONG) *stringArray);
		stringArray++;
		}
}

//----------------------------------------------------------------------------

static SAVEDS(APTR) INTERRUPT ResetToDefaultsHookFunc(struct Hook *hook, Object *o, Msg msg)
{
	struct PopupMenuPrefsInst *inst = (struct PopupMenuPrefsInst *) hook->h_Data;

	d1(KPrintF("%s/%s/%ld: START\n", __FILE__, __FUNC__, __LINE__));

	inst->mpb_PMPrefs = DefaultPMPrefs;
	PrefsToGUI(inst, &inst->mpb_PMPrefs);

	d1(KPrintF("%s/%s/%ld: END\n", __FILE__, __FUNC__, __LINE__));

	return 0;
}

//----------------------------------------------------------------------------

static SAVEDS(APTR) INTERRUPT OpenHookFunc(struct Hook *hook, Object *o, Msg msg)
{
	struct PopupMenuPrefsInst *inst = (struct PopupMenuPrefsInst *) hook->h_Data;

	if (NULL == inst->mpb_LoadReq)
		{
		inst->mpb_LoadReq = MUI_AllocAslRequestTags(ASL_FileRequest,
			ASLFR_InitialFile, "PopupMenu.pre",
			ASLFR_Flags1, FRF_DOPATTERNS,
			ASLFR_InitialDrawer, "SYS:Prefs/presets",
			ASLFR_InitialPattern, "#?.(pre|prefs)",
			ASLFR_UserData, inst,
			ASLFR_IntuiMsgFunc, &inst->mpb_Hooks[HOOKNDX_AslIntuiMsg],
			TAG_END);
		}
	if (inst->mpb_LoadReq)
		{
		BOOL Result;
		struct Window *win = NULL;

		get(inst->mpb_Objects[OBJNDX_WIN_Main], MUIA_Window_Window, (APTR) &win);

		//AslRequest(
		Result = MUI_AslRequestTags(inst->mpb_LoadReq,
			ASLFR_Window, win,
			ASLFR_SleepWindow, TRUE,
			ASLFR_TitleText, GetLocString(MSGID_MENU_PROJECT_OPEN),
			TAG_END);

		if (Result)
			{
			BPTR dirLock = Lock(inst->mpb_LoadReq->fr_Drawer, ACCESS_READ);

			if (dirLock)
				{
				BPTR oldDir = CurrentDir(dirLock);

				ReadPrefsFile(inst, inst->mpb_LoadReq->fr_File, FALSE);
				PrefsToGUI(inst, &inst->mpb_PMPrefs);

				CurrentDir(oldDir);
				UnLock(dirLock);
				}
			}
		}

	return 0;
}

//----------------------------------------------------------------------------

static SAVEDS(APTR) INTERRUPT LastSavedHookFunc(struct Hook *hook, Object *o, Msg msg)
{
	struct PopupMenuPrefsInst *inst = (struct PopupMenuPrefsInst *) hook->h_Data;

	ReadPrefsFile(inst, PMP_S_PATH, FALSE);
	PrefsToGUI(inst, &inst->mpb_PMPrefs);
	SetChangedFlag(inst, TRUE);

	return 0;
}

//----------------------------------------------------------------------------

static SAVEDS(APTR) INTERRUPT RestoreHookFunc(struct Hook *hook, Object *o, Msg msg)
{
	struct PopupMenuPrefsInst *inst = (struct PopupMenuPrefsInst *) hook->h_Data;

	ReadPrefsFile(inst, PMP_PATH, FALSE);
	PrefsToGUI(inst, &inst->mpb_PMPrefs);
	SetChangedFlag(inst, TRUE);

	return 0;
}

//----------------------------------------------------------------------------

static SAVEDS(APTR) INTERRUPT SaveAsHookFunc(struct Hook *hook, Object *o, Msg msg)
{
	struct PopupMenuPrefsInst *inst = (struct PopupMenuPrefsInst *) hook->h_Data;

	if (NULL == inst->mpb_SaveReq)
		{
		inst->mpb_SaveReq = MUI_AllocAslRequestTags(ASL_FileRequest,
			ASLFR_InitialFile, "PopupMenu.pre",
			ASLFR_DoSaveMode, TRUE,
			ASLFR_InitialDrawer, "SYS:Prefs/presets",
			ASLFR_UserData, inst,
			ASLFR_IntuiMsgFunc, &inst->mpb_Hooks[HOOKNDX_AslIntuiMsg],
			TAG_END);
		}
	if (inst->mpb_SaveReq)
		{
		BOOL Result;
		struct Window *win = NULL;

		get(inst->mpb_Objects[OBJNDX_WIN_Main], MUIA_Window_Window, (APTR) &win);

		//AslRequest(
		Result = MUI_AslRequestTags(inst->mpb_SaveReq,
			ASLFR_TitleText, GetLocString(MSGID_MENU_PROJECT_SAVEAS),
			ASLFR_Window, win,
			ASLFR_SleepWindow, TRUE,
			TAG_END);

		if (Result)
			{
			BPTR dirLock = Lock(inst->mpb_SaveReq->fr_Drawer, ACCESS_READ);

			if (dirLock)
				{
				BPTR oldDir = CurrentDir(dirLock);

				WritePrefsFile(inst, inst->mpb_SaveReq->fr_File);

				CurrentDir(oldDir);
				UnLock(dirLock);
				}
			}
		}

	return 0;
}

//----------------------------------------------------------------------------

static SAVEDS(APTR) INTERRUPT AboutHookFunc(struct Hook *hook, Object *o, Msg msg)
{
	struct PopupMenuPrefsInst *inst = (struct PopupMenuPrefsInst *) hook->h_Data;

	MUI_Request(inst->mpb_Objects[OBJNDX_APP_Main], inst->mpb_Objects[OBJNDX_WIN_Main], 0, NULL, 
		GetLocString(MSGID_ABOUTREQOK), 
		GetLocString(MSGID_ABOUTREQFORMAT), 
		MajorVersion, MinorVersion, COMPILER_STRING, CURRENTYEAR);

	return 0;
}

//----------------------------------------------------------------------------

static LONG ReadPrefsFile(struct PopupMenuPrefsInst *inst, CONST_STRPTR Filename, BOOL Quiet)
{
	LONG Result;
	struct IFFHandle *iff;
	BOOL iffOpened = FALSE;

	d1(KPrintF("%s/%s/%ld:  Filename=<%s>  LocaleBase=%08lx  IFFParseBase=%08lx\n", \
		__FILE__, __FUNC__, __LINE__, Filename, LocaleBase, IFFParseBase));

	do	{
		iff = AllocIFF();
		if (NULL == iff)
			{
			Result = IoErr();
			break;
			}

		InitIFFasDOS(iff);

		iff->iff_Stream = Open(Filename, MODE_OLDFILE);
		if ((BPTR)NULL == iff->iff_Stream)
			{
			Result = IoErr();
			break;
			}

		Result = OpenIFF(iff, IFFF_READ);
		if (RETURN_OK != Result)
			break;

		iffOpened = TRUE;

		Result = StopChunks(iff, StopChunkList, 2);
		if (RETURN_OK != Result)
			break;

		while (1)
			{
			struct ContextNode *cn;

			Result = ParseIFF(iff, IFFPARSE_SCAN);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			if (RETURN_OK != Result)
				break;

			cn = CurrentChunk(iff);

			d1(KPrintF("%s/%s/%ld:  cn=%08lx\n", __FILE__, __FUNC__, __LINE__, cn));
			if (NULL == cn)
				break;

			d1(KPrintF("%s/%s/%ld:  cn=%08lx  id=%08lx\n", __FILE__, __FUNC__, __LINE__, cn, cn->cn_ID));
			if (ID_PMNU == cn->cn_ID)
				{
				LONG Actual;

				d1(KPrintF("%s/%s/%ld:  cn_Size=%lu\n", __FILE__, __FUNC__, __LINE__, cn->cn_Size));

				if (cn->cn_Size != sizeof(struct PopupMenuPrefs))
					break;

				Actual = ReadChunkBytes(iff, &inst->mpb_PMPrefs, cn->cn_Size);
				d1(KPrintF("%s/%s/%ld:  Actual=%lu\n", __FILE__, __FUNC__, __LINE__, Actual));
				if (Actual != cn->cn_Size)
					break;
				}
			}

		d1(KPrintF(__FILE__ "%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
		if (IFFERR_EOF == Result)
			Result = RETURN_OK;
		} while (0);

	if (iff)
		{
		if (iffOpened)
			CloseIFF(iff);

		if (iff->iff_Stream)
			Close(iff->iff_Stream);

		FreeIFF(iff);
		}

	if (RETURN_OK != Result && !Quiet)
		{
		char Buffer[120];

		Fault(Result, "", Buffer, sizeof(Buffer) - 1);

		// MUI_RequestA()
		MUI_Request(inst->mpb_Objects[OBJNDX_APP_Main], inst->mpb_Objects[OBJNDX_WIN_Main], 0, NULL, 
			GetLocString(MSGID_ABOUTREQOK), 
			GetLocString(MSGID_REQTITLE_READERROR), 
			Filename,
			Buffer);
		}

	d1(KPrintF("%s/%s/%ld: pmp_MenuTitles=%08lx\n", __FILE__, __FUNC__, __LINE__, inst->mpb_PMPrefs.pmp_MenuTitles));
	d1(KPrintF("%s/%s/%ld: pmp_MenuItems=%08lx\n", __FILE__, __FUNC__, __LINE__, inst->mpb_PMPrefs.pmp_MenuItems));
	d1(KPrintF("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	return Result;
}


static LONG WritePrefsFile(struct PopupMenuPrefsInst *inst, CONST_STRPTR Filename)
{
	LONG Result;
	struct IFFHandle *iff;
	BOOL IffOpen = FALSE;

	do	{
		static const struct PrefHeader prefHeader = { 1, 0, 0L };

		GUItoPrefs(inst, &inst->mpb_PMPrefs);

		iff = AllocIFF();
		if (NULL == iff)
			{
			Result = IoErr();
			break;
			}

		InitIFFasDOS(iff);

		iff->iff_Stream = Open(Filename, MODE_NEWFILE);
		if ((BPTR)NULL == iff->iff_Stream)
			{
			// ... try to create missing directories here
			STRPTR FilenameCopy;

			Result = IoErr();
			d1(KPrintF("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

			FilenameCopy = AllocVec(1 + strlen(Filename), MEMF_PUBLIC);

			if (FilenameCopy)
				{
				STRPTR lp;

				strcpy(FilenameCopy, Filename);

				lp = PathPart(FilenameCopy);
				if (lp)
					{
					BPTR dirLock;

					*lp = '\0';
					dirLock = CreateDir(FilenameCopy);

					if (dirLock)
						UnLock(dirLock);

					iff->iff_Stream = Open(Filename, MODE_NEWFILE);
					if ((BPTR)NULL == iff->iff_Stream)
						Result = IoErr();
					else
						Result = RETURN_OK;
					}

				FreeVec(FilenameCopy);
				}

			d1(KPrintF("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			if (RETURN_OK != Result)
				break;
			}

		Result = OpenIFF(iff, IFFF_WRITE);
		if (RETURN_OK != Result)
			break;

		IffOpen = TRUE;

		Result = PushChunk(iff, ID_PREF, ID_FORM, IFFSIZE_UNKNOWN);
		if (RETURN_OK != Result)
			break;

		Result = PushChunk(iff, 0, ID_PRHD, IFFSIZE_UNKNOWN);
		if (RETURN_OK != Result)
			break;

		if (WriteChunkBytes(iff, (APTR) &prefHeader, sizeof(prefHeader)) < 0)
			{
			Result = IoErr();
			break;
			}

		Result = PopChunk(iff);
		if (RETURN_OK != Result)
			break;

		Result = PushChunk(iff, 0, ID_PMNU, IFFSIZE_UNKNOWN);
		if (RETURN_OK != Result)
			break;

		if (WriteChunkBytes(iff, (APTR) &inst->mpb_PMPrefs, sizeof(struct PopupMenuPrefs)) < 0)
			{
			Result = IoErr();
			break;
			}

		Result = PopChunk(iff);
		if (RETURN_OK != Result)
			break;

		} while (0);

	if (iff)
		{
		if (IffOpen)
			CloseIFF(iff);

		if (iff->iff_Stream)
			{
			Close(iff->iff_Stream);
			iff->iff_Stream = (BPTR)NULL;
			}

		FreeIFF(iff);
		}

	if (RETURN_OK == Result)
		{
		if (inst->mpb_fCreateIcons)
			SaveIcon(inst, Filename);
		}
	else
		{
		char Buffer[120];

		Fault(Result, "", Buffer, sizeof(Buffer) - 1);

		// MUI_RequestA()
		MUI_Request(inst->mpb_Objects[OBJNDX_APP_Main], inst->mpb_Objects[OBJNDX_WIN_Main], 0, NULL, 
			GetLocString(MSGID_ABOUTREQOK), 
			GetLocString(MSGID_REQTITLE_SAVEERROR), 
			Filename,
			Buffer);
		}

	return Result;
}


static LONG SaveIcon(struct PopupMenuPrefsInst *inst, CONST_STRPTR IconName)
{
	struct DiskObject *icon, *allocIcon;

	static UWORD ImageData[] =
		{
		0x0000, 0x0000, 0x0004, 0x0000, 0x0000, 0x0000, 0x0001, 0x0000,
		0x0000, 0x07FF, 0x8000, 0x4000, 0x0000, 0x1800, 0x6000, 0x1000,
		0x0000, 0x20FC, 0x1000, 0x0800, 0x0000, 0x4102, 0x0800, 0x0C00, 
		0x0000, 0x4082, 0x0800, 0x0C00, 0x0000, 0x4082, 0x0800, 0x0C00, 
		0x0000, 0x2104, 0x0800, 0x0C00, 0x0000, 0x1E18, 0x1000, 0x0C00,
		0x0000, 0x0060, 0x2000, 0x0C00, 0x0000, 0x0080, 0xC000, 0x0C00, 
		0x0000, 0x0103, 0x0000, 0x0C00, 0x0000, 0x021C, 0x0000, 0x0C00, 
		0x0000, 0x0108, 0x0000, 0x0C00, 0x0000, 0x00F0, 0x0000, 0x0C00, 
		0x0000, 0x0108, 0x0000, 0x0C00, 0x0000, 0x0108, 0x0000, 0x0C00, 
		0x4000, 0x00F0, 0x0000, 0x0C00, 0x1000, 0x0000, 0x0000, 0x0C00,
		0x0400, 0x0000, 0x0000, 0x0C00, 0x01FF, 0xFFFF, 0xFFFF, 0xFC00, 
		0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFF8, 0x0000, 
		0xD555, 0x5555, 0x5556, 0x0000, 0xD555, 0x5000, 0x5555, 0x8000, 
		0xD555, 0x47FF, 0x9555, 0x6000, 0xD555, 0x5F03, 0xE555, 0x5000, 
		0xD555, 0x3E55, 0xF555, 0x5000, 0xD555, 0x3F55, 0xF555, 0x5000,
		0xD555, 0x3F55, 0xF555, 0x5000, 0xD555, 0x5E53, 0xF555, 0x5000, 
		0xD555, 0x4147, 0xE555, 0x5000, 0xD555, 0x551F, 0xD555, 0x5000, 
		0xD555, 0x557F, 0x1555, 0x5000, 0xD555, 0x54FC, 0x5555, 0x5000, 
		0xD555, 0x55E1, 0x5555, 0x5000, 0xD555, 0x54F5, 0x5555, 0x5000, 
		0xD555, 0x5505, 0x5555, 0x5000, 0xD555, 0x54F5, 0x5555, 0x5000,
		0xD555, 0x54F5, 0x5555, 0x5000, 0x3555, 0x5505, 0x5555, 0x5000, 
		0x0D55, 0x5555, 0x5555, 0x5000, 0x0355, 0x5555, 0x5555, 0x5000, 
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		};

	static struct Image NormalImage =
		{
		0, 0, 54, 23,
		2,
		ImageData,
		3, 0,
		NULL
		};

	static STRPTR defToolTypes[] =
		{
		"ACTION=USE",
		NULL
		};

	static struct DiskObject DefaultIcon =
		{
		WB_DISKMAGIC, WB_DISKVERSION,

		{
		NULL, 
		0, 0, 54, 24,
		GFLG_GADGIMAGE | GFLG_GADGHBOX,
		GACT_RELVERIFY | GACT_IMMEDIATE,
		GTYP_BOOLGADGET,
		&NormalImage, NULL,
		NULL,
		0,
		NULL,
		0,
		NULL
		},

		WBPROJECT,
		NULL,
		defToolTypes,
		NO_ICON_POSITION, NO_ICON_POSITION,
		NULL,
		NULL,
		8192
		};

	icon = allocIcon = GetDiskObject("ENV:sys/def_ScaPopupMenuPrefs");
	if (NULL == icon)
		icon = allocIcon = GetDiskObject("ENV:sys/def_pref");
	if (NULL == icon)
		icon = &DefaultIcon;

	if (icon)
		{
		STRPTR oldDefaultTool = icon->do_DefaultTool;

		icon->do_DefaultTool = (STRPTR) inst->mpb_ProgramName;

		PutDiskObject((STRPTR) IconName, icon);

		icon->do_DefaultTool = oldDefaultTool;

		if (allocIcon)
			FreeDiskObject(allocIcon);
		}

	return 0;
}


static SAVEDS(APTR) INTERRUPT AppMessageHookFunc(struct Hook *hook, Object *o, Msg msg)
{
#if 0
	struct PopupMenuPrefsInst *inst = (struct PopupMenuPrefsInst *) hook->h_Data;
	struct AppMessage *AppMsg = *(struct AppMessage **) msg;
	struct MUI_NListtree_TreeNode *TreeNode = NULL;

	set(inst->mpb_Objects[OBJNDX_MainListTree], MUIA_NListtree_Quiet, TRUE);

	TreeNode = (struct MUI_NListtree_TreeNode *) DoMethod(inst->mpb_Objects[OBJNDX_MainListTree],
		MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Active,
		MUIV_NListtree_GetEntry_Position_Active, 0);

	if (TreeNode)
		{
		struct MUI_NListtree_TreeNode *ParentNode = (struct MUI_NListtree_TreeNode *) MUIV_NListtree_Insert_PrevNode_Tail;
		LONG n;

		for (n=0; n<AppMsg->am_NumArgs; )
			{
			struct PopupMenuListEntry *mle = (struct PopupMenuListEntry *) TreeNode->tn_User;
			char Buffer[1000];
			STRPTR FileName;

			if (!NameFromLock(AppMsg->am_ArgList[n].wa_Lock, Buffer, sizeof(Buffer)))
				break;
			if (AppMsg->am_ArgList[n].wa_Name && strlen(AppMsg->am_ArgList[n].wa_Name) > 0)
				AddPart(Buffer, AppMsg->am_ArgList[n].wa_Name, sizeof(Buffer));

			FileName = FilePart(Buffer);

			switch (mle->llist_EntryType)
				{
			case SCAPopupMenuTYPE_MainPopupMenu:
				d1(kprintf("%s/%s/%ld: insert at SCAPopupMenuTYPE_MainPopupMenu\n", __FILE__, __FUNC__, __LINE__));
				if (mle->llist_Flags & LLISTFLGF_MayNotHaveChildren)
					{
					AppMessage_PopupMenu(inst, &TreeNode, &ParentNode, &AppMsg->am_ArgList[n], Buffer, FileName);
					n++;
					}
				else
					{
					TreeNode = (struct MUI_NListtree_TreeNode *) DoMethod(inst->mpb_Objects[OBJNDX_MainListTree],
						MUIM_NListtree_Insert, "DragIn PopupMenu",
						CombineEntryTypeAndFlags(SCAPopupMenuTYPE_PopupMenu, 0),
						TreeNode, ParentNode, TNF_OPEN | TNF_LIST);
					}
				break;

			case SCAPopupMenuTYPE_PopupMenu:
			case SCAPopupMenuTYPE_ToolsPopupMenu:
				d1(kprintf("%s/%s/%ld: insert at SCAPopupMenuTYPE_PopupMenu\n", __FILE__, __FUNC__, __LINE__));
				AppMessage_PopupMenu(inst, &TreeNode, &ParentNode, &AppMsg->am_ArgList[n], Buffer, FileName);
				n++;
				break;

			case SCAPopupMenuTYPE_PopupMenuItem:
				d1(kprintf("%s/%s/%ld: insert at SCAPopupMenuTYPE_PopupMenuItem\n", __FILE__, __FUNC__, __LINE__));
				ParentNode = TreeNode;
				TreeNode = (struct MUI_NListtree_TreeNode *) DoMethod(inst->mpb_Objects[OBJNDX_MainListTree],
					MUIM_NListtree_GetEntry, TreeNode, MUIV_NListtree_GetEntry_Position_Parent, 0);

				AppMessage_PopupMenu(inst, &TreeNode, &ParentNode, &AppMsg->am_ArgList[n], Buffer, FileName);
				n++;
				break;

			case SCAPopupMenuTYPE_Command:
				d1(kprintf("%s/%s/%ld: insert at SCAPopupMenuTYPE_Command\n", __FILE__, __FUNC__, __LINE__));
				TreeNode = (struct MUI_NListtree_TreeNode *) DoMethod(inst->mpb_Objects[OBJNDX_MainListTree],
					MUIM_NListtree_GetEntry, TreeNode, MUIV_NListtree_GetEntry_Position_Parent, 0);
				break;

			default:
				break;
				}
			}
		}

	set(inst->mpb_Objects[OBJNDX_MainListTree], MUIA_NListtree_Quiet, FALSE);
#endif
	return 0;
}

//----------------------------------------------------------------------------

static SAVEDS(APTR) INTERRUPT SettingsChangedHookFunc(struct Hook *hook, Object *o, Msg msg)
{
	struct PopupMenuPrefsInst *inst = (struct PopupMenuPrefsInst *) hook->h_Data;

	d1(kprintf("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
	SetChangedFlag(inst, TRUE);

	return NULL;
}

//----------------------------------------------------------------------------

static SAVEDS(void) INTERRUPT AslIntuiMsgHookFunc(struct Hook *hook, Object *o, Msg msg)
{
	struct PopupMenuPrefsInst *inst = (struct PopupMenuPrefsInst *) hook->h_Data;
	struct IntuiMessage *iMsg = (struct IntuiMessage *) msg;

	if (IDCMP_REFRESHWINDOW == iMsg->Class)
		{
		DoMethod(inst->mpb_Objects[OBJNDX_APP_Main], MUIM_Application_CheckRefresh);
		}
}

//----------------------------------------------------------------------------

static SAVEDS(void) INTERRUPT FrameTypeSelectHookFunc(struct Hook *hook, Object *o, Msg msg)
{
	struct PopupMenuPrefsInst *inst = (struct PopupMenuPrefsInst *) hook->h_Data;

	set(o, MUIA_Selected, TRUE);

	// turn off selected state for all frame type buttons except <o>
	if (o != inst->mpb_Objects[OBJNDX_Button_FrameThin])
		set(inst->mpb_Objects[OBJNDX_Button_FrameThin], MUIA_Selected, FALSE);
	if (o != inst->mpb_Objects[OBJNDX_Button_FrameMM])
		set(inst->mpb_Objects[OBJNDX_Button_FrameMM], MUIA_Selected, FALSE);
	if (o != inst->mpb_Objects[OBJNDX_Button_FrameThick])
		set(inst->mpb_Objects[OBJNDX_Button_FrameThick], MUIA_Selected, FALSE);
	if (o != inst->mpb_Objects[OBJNDX_Button_FrameRidge])
		set(inst->mpb_Objects[OBJNDX_Button_FrameRidge], MUIA_Selected, FALSE);
	if (o != inst->mpb_Objects[OBJNDX_Button_FrameDropBox])
		set(inst->mpb_Objects[OBJNDX_Button_FrameDropBox], MUIA_Selected, FALSE);
	if (o != inst->mpb_Objects[OBJNDX_Button_FrameOldStyle])
		set(inst->mpb_Objects[OBJNDX_Button_FrameOldStyle], MUIA_Selected, FALSE);
}

//----------------------------------------------------------------------------

static SAVEDS(void) INTERRUPT SelFrameTypeSelectHookFunc(struct Hook *hook, Object *o, Msg msg)
{
	struct PopupMenuPrefsInst *inst = (struct PopupMenuPrefsInst *) hook->h_Data;

	set(o, MUIA_Selected, TRUE);

	// turn off selected state for all selected frame type buttons except <o>
	if (o != inst->mpb_Objects[OBJNDX_Button_Raised])
		set(inst->mpb_Objects[OBJNDX_Button_Raised], MUIA_Selected, FALSE);
	if (o != inst->mpb_Objects[OBJNDX_Button_Normal])
		set(inst->mpb_Objects[OBJNDX_Button_Normal], MUIA_Selected, FALSE);
	if (o != inst->mpb_Objects[OBJNDX_Button_Recessed])
		set(inst->mpb_Objects[OBJNDX_Button_Recessed], MUIA_Selected, FALSE);
}

//----------------------------------------------------------------------------

static void PrefsToGUI(struct PopupMenuPrefsInst *inst, const struct PopupMenuPrefs *pmPrefs)
{
	switch (pmPrefs->pmp_MenuBorder)
		{
	case PMP_MENUBORDER_THIN:
	default:
		set(inst->mpb_Objects[OBJNDX_Button_FrameThin], MUIA_Selected, TRUE);
		break;
	case PMP_MENUBORDER_MM:
		set(inst->mpb_Objects[OBJNDX_Button_FrameMM], MUIA_Selected, TRUE);
		break;
	case PMP_MENUBORDER_THICK:
		set(inst->mpb_Objects[OBJNDX_Button_FrameThick], MUIA_Selected, TRUE);
		break;
	case PMP_MENUBORDER_RIDGE:
		set(inst->mpb_Objects[OBJNDX_Button_FrameRidge], MUIA_Selected, TRUE);
		break;
	case PMP_MENUBORDER_DROPBOX:
		set(inst->mpb_Objects[OBJNDX_Button_FrameDropBox], MUIA_Selected, TRUE);
		break;
	case PMP_MENUBORDER_OLDSTYLE:
		set(inst->mpb_Objects[OBJNDX_Button_FrameOldStyle], MUIA_Selected, TRUE);
		break;
		}

	switch (pmPrefs->pmp_SelItemBorder)
		{
	case PMP_SELITEM_NO_BORDER:
	default:
		set(inst->mpb_Objects[OBJNDX_Button_Normal], MUIA_Selected, TRUE);
		break;
	case PMP_SELITEM_RAISE:
		set(inst->mpb_Objects[OBJNDX_Button_Raised], MUIA_Selected, TRUE);
		break;
	case PMP_SELITEM_RECESS:
		set(inst->mpb_Objects[OBJNDX_Button_Recessed], MUIA_Selected, TRUE);
		break;
		}

	if (pmPrefs->pmp_SeparatorBar)
		set(inst->mpb_Objects[OBJNDX_Button_OldLook], MUIA_Selected, TRUE);
	else
		set(inst->mpb_Objects[OBJNDX_Button_NewLook], MUIA_Selected, TRUE);

	setslider(inst->mpb_Objects[OBJNDX_Slider_DelaySubMenus], pmPrefs->pmp_SubMenuDelay);
	setslider(inst->mpb_Objects[OBJNDX_Slider_AnimationSpeed], pmPrefs->pmp_AnimationSpeed);

	setslider(inst->mpb_Objects[OBJNDX_Slider_Horizontal], pmPrefs->pmp_XOffset);
	setslider(inst->mpb_Objects[OBJNDX_Slider_VerticalOffset], pmPrefs->pmp_YOffset);
	setslider(inst->mpb_Objects[OBJNDX_Slider_HorizontalSpacing], pmPrefs->pmp_XSpace);
	setslider(inst->mpb_Objects[OBJNDX_Slider_VerticalSpacing], pmPrefs->pmp_YSpace);
	setslider(inst->mpb_Objects[OBJNDX_Slider_IntermediateSpacing], pmPrefs->pmp_Intermediate);
	setslider(inst->mpb_Objects[OBJNDX_Slider_TextDisplacement], pmPrefs->pmp_TextDisplace);
	setslider(inst->mpb_Objects[OBJNDX_Slider_ShadowR], pmPrefs->pmp_ShadowR);
	setslider(inst->mpb_Objects[OBJNDX_Slider_ShadowG], pmPrefs->pmp_ShadowG);
	setslider(inst->mpb_Objects[OBJNDX_Slider_ShadowB], pmPrefs->pmp_ShadowB);

	setcycle(inst->mpb_Objects[OBJNDX_Cycle_AnimationType], pmPrefs->pmp_Animation);

	setcheckmark(inst->mpb_Objects[OBJNDX_CheckMark_SameHeight], pmPrefs->pmp_SameHeight);
	setcheckmark(inst->mpb_Objects[OBJNDX_CheckMark_Sticky], pmPrefs->pmp_Sticky);

	d1(KPrintF("%s/%s/%ld: pmp_MenuTitles=%08lx\n", __FILE__, __FUNC__, __LINE__, pmPrefs->pmp_MenuTitles));
	d1(KPrintF("%s/%s/%ld: pmp_MenuItems=%08lx\n", __FILE__, __FUNC__, __LINE__, pmPrefs->pmp_MenuItems));

	setcheckmark(inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Embossed], pmPrefs->pmp_MenuTitles & PMP_TITLE_EMBOSS);
	setcheckmark(inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Outlined], pmPrefs->pmp_MenuTitles & PMP_TEXT_OUTLINE);
	setcheckmark(inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Shadowed], pmPrefs->pmp_MenuTitles & PMP_TEXT_SHADOW);
	setcheckmark(inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Underlined], pmPrefs->pmp_MenuTitles & PMP_TEXT_UNDERLINE);
	setcheckmark(inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Bold], pmPrefs->pmp_MenuTitles & PMP_TEXT_BOLD);
	setcheckmark(inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Italic], pmPrefs->pmp_MenuTitles & PMP_TEXT_ITALIC);

	setcheckmark(inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Embossed], pmPrefs->pmp_MenuItems & PMP_TEXT_EMBOSS);
	setcheckmark(inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Outlined], pmPrefs->pmp_MenuItems & PMP_TEXT_OUTLINE);
	setcheckmark(inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Shadowed], pmPrefs->pmp_MenuItems & PMP_TEXT_SHADOW);
	setcheckmark(inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Underlined], pmPrefs->pmp_MenuItems & PMP_TEXT_UNDERLINE);
	setcheckmark(inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Bold], pmPrefs->pmp_MenuItems & PMP_TEXT_BOLD);
	setcheckmark(inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Italic], pmPrefs->pmp_MenuItems & PMP_TEXT_ITALIC);

	setslider(inst->mpb_Objects[OBJNDX_Slider_TranspR], pmPrefs->pmp_TransparencyR);
	setslider(inst->mpb_Objects[OBJNDX_Slider_TranspG], pmPrefs->pmp_TransparencyG);
	setslider(inst->mpb_Objects[OBJNDX_Slider_TranspB], pmPrefs->pmp_TransparencyB);
	setslider(inst->mpb_Objects[OBJNDX_Slider_Transp_Blur], pmPrefs->pmp_TransparencyBlur);
}

//----------------------------------------------------------------------------

static void GUItoPrefs(struct PopupMenuPrefsInst *inst, struct PopupMenuPrefs *pmPrefs)
{
	if (getv(inst->mpb_Objects[OBJNDX_Button_FrameThin], MUIA_Selected))
		pmPrefs->pmp_MenuBorder = PMP_MENUBORDER_THIN;
	else if (getv(inst->mpb_Objects[OBJNDX_Button_FrameMM], MUIA_Selected))
		pmPrefs->pmp_MenuBorder = PMP_MENUBORDER_MM;
	else if (getv(inst->mpb_Objects[OBJNDX_Button_FrameThick], MUIA_Selected))
		pmPrefs->pmp_MenuBorder = PMP_MENUBORDER_THICK;
	else if (getv(inst->mpb_Objects[OBJNDX_Button_FrameRidge], MUIA_Selected))
		pmPrefs->pmp_MenuBorder = PMP_MENUBORDER_RIDGE;
	else if (getv(inst->mpb_Objects[OBJNDX_Button_FrameDropBox], MUIA_Selected))
		pmPrefs->pmp_MenuBorder = PMP_MENUBORDER_DROPBOX;
	else if (getv(inst->mpb_Objects[OBJNDX_Button_FrameOldStyle], MUIA_Selected))
		pmPrefs->pmp_MenuBorder = PMP_MENUBORDER_OLDSTYLE;
	else
		pmPrefs->pmp_MenuBorder = PMP_MENUBORDER_THIN;

	if (getv(inst->mpb_Objects[OBJNDX_Button_Normal], MUIA_Selected))
		pmPrefs->pmp_SelItemBorder = PMP_SELITEM_NO_BORDER;
	else if (getv(inst->mpb_Objects[OBJNDX_Button_Raised], MUIA_Selected))
		pmPrefs->pmp_SelItemBorder = PMP_SELITEM_RAISE;
	else if (getv(inst->mpb_Objects[OBJNDX_Button_Recessed], MUIA_Selected))
		pmPrefs->pmp_SelItemBorder = PMP_SELITEM_RECESS;
	else
		pmPrefs->pmp_SelItemBorder = PMP_SELITEM_NO_BORDER;

	if (getv(inst->mpb_Objects[OBJNDX_Button_OldLook], MUIA_Selected))
		pmPrefs->pmp_SeparatorBar = TRUE;
	else
		pmPrefs->pmp_SeparatorBar = FALSE;

	pmPrefs->pmp_SubMenuDelay 	= getv(inst->mpb_Objects[OBJNDX_Slider_DelaySubMenus], MUIA_Numeric_Value);
	pmPrefs->pmp_AnimationSpeed 	= getv(inst->mpb_Objects[OBJNDX_Slider_AnimationSpeed], MUIA_Numeric_Value);

	pmPrefs->pmp_XOffset		= getv(inst->mpb_Objects[OBJNDX_Slider_Horizontal], MUIA_Numeric_Value);
	pmPrefs->pmp_YOffset		= getv(inst->mpb_Objects[OBJNDX_Slider_VerticalOffset], MUIA_Numeric_Value);
	pmPrefs->pmp_XSpace		= getv(inst->mpb_Objects[OBJNDX_Slider_HorizontalSpacing], MUIA_Numeric_Value);
	pmPrefs->pmp_YSpace		= getv(inst->mpb_Objects[OBJNDX_Slider_VerticalSpacing], MUIA_Numeric_Value);
	pmPrefs->pmp_Intermediate	= getv(inst->mpb_Objects[OBJNDX_Slider_IntermediateSpacing], MUIA_Numeric_Value);
	pmPrefs->pmp_TextDisplace	= getv(inst->mpb_Objects[OBJNDX_Slider_TextDisplacement], MUIA_Numeric_Value);
	pmPrefs->pmp_ShadowR		= getv(inst->mpb_Objects[OBJNDX_Slider_ShadowR], MUIA_Numeric_Value);
	pmPrefs->pmp_ShadowG		= getv(inst->mpb_Objects[OBJNDX_Slider_ShadowG], MUIA_Numeric_Value);
	pmPrefs->pmp_ShadowB		= getv(inst->mpb_Objects[OBJNDX_Slider_ShadowB], MUIA_Numeric_Value);

	pmPrefs->pmp_Animation		= getv(inst->mpb_Objects[OBJNDX_Cycle_AnimationType], MUIA_Cycle_Active);

	pmPrefs->pmp_SameHeight		= getv(inst->mpb_Objects[OBJNDX_CheckMark_SameHeight], MUIA_Selected);
	pmPrefs->pmp_Sticky		= getv(inst->mpb_Objects[OBJNDX_CheckMark_Sticky], MUIA_Selected);

	if (getv(inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Embossed], MUIA_Selected))
		pmPrefs->pmp_MenuTitles |= PMP_TITLE_EMBOSS;
	else
		pmPrefs->pmp_MenuTitles &= ~PMP_TITLE_EMBOSS;
	if (getv(inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Outlined], MUIA_Selected))
		pmPrefs->pmp_MenuTitles |= PMP_TEXT_OUTLINE;
	else
		pmPrefs->pmp_MenuTitles &= ~PMP_TEXT_OUTLINE;
	if (getv(inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Shadowed], MUIA_Selected))
		pmPrefs->pmp_MenuTitles |= PMP_TEXT_SHADOW;
	else
		pmPrefs->pmp_MenuTitles &= ~PMP_TEXT_SHADOW;
	if (getv(inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Underlined], MUIA_Selected))
		pmPrefs->pmp_MenuTitles |= PMP_TEXT_UNDERLINE;
	else
		pmPrefs->pmp_MenuTitles &= ~PMP_TEXT_UNDERLINE;
	if (getv(inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Bold], MUIA_Selected))
		pmPrefs->pmp_MenuTitles |= PMP_TEXT_BOLD;
	else
		pmPrefs->pmp_MenuTitles &= ~PMP_TEXT_BOLD;
	if (getv(inst->mpb_Objects[OBJNDX_CheckMark_MenuTitles_Italic], MUIA_Selected))
		pmPrefs->pmp_MenuTitles |= PMP_TEXT_ITALIC;
	else
		pmPrefs->pmp_MenuTitles &= ~PMP_TEXT_ITALIC;
	d1(KPrintF("%s/%s/%ld: pmp_MenuTitles=%08lx\n", __FILE__, __FUNC__, __LINE__, pmPrefs->pmp_MenuTitles));

	if (getv(inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Embossed], MUIA_Selected))
		pmPrefs->pmp_MenuItems |= PMP_TEXT_EMBOSS;
	else
		pmPrefs->pmp_MenuItems &= ~PMP_TEXT_EMBOSS;
	if (getv(inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Outlined], MUIA_Selected))
		pmPrefs->pmp_MenuItems |= PMP_TEXT_OUTLINE;
	else
		pmPrefs->pmp_MenuItems &= ~PMP_TEXT_OUTLINE;
	if (getv(inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Shadowed], MUIA_Selected))
		pmPrefs->pmp_MenuItems |= PMP_TEXT_SHADOW;
	else
		pmPrefs->pmp_MenuItems &= ~PMP_TEXT_SHADOW;
	if (getv(inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Underlined], MUIA_Selected))
		pmPrefs->pmp_MenuItems |= PMP_TEXT_UNDERLINE;
	else
		pmPrefs->pmp_MenuItems &= ~PMP_TEXT_UNDERLINE;
	if (getv(inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Bold], MUIA_Selected))
		pmPrefs->pmp_MenuItems |= PMP_TEXT_BOLD;
	else
		pmPrefs->pmp_MenuItems &= ~PMP_TEXT_BOLD;
	if (getv(inst->mpb_Objects[OBJNDX_CheckMark_MenuItems_Italic], MUIA_Selected))
		pmPrefs->pmp_MenuItems |= PMP_TEXT_ITALIC;
	else
		pmPrefs->pmp_MenuItems &= ~PMP_TEXT_ITALIC;
	d1(KPrintF("%s/%s/%ld: pmp_MenuItems=%08lx\n", __FILE__, __FUNC__, __LINE__, pmPrefs->pmp_MenuItems));

	pmPrefs->pmp_TransparencyR	= getv(inst->mpb_Objects[OBJNDX_Slider_TranspR], MUIA_Numeric_Value);
	pmPrefs->pmp_TransparencyG	= getv(inst->mpb_Objects[OBJNDX_Slider_TranspG], MUIA_Numeric_Value);
	pmPrefs->pmp_TransparencyB	= getv(inst->mpb_Objects[OBJNDX_Slider_TranspB], MUIA_Numeric_Value);
	pmPrefs->pmp_TransparencyBlur	= getv(inst->mpb_Objects[OBJNDX_Slider_Transp_Blur], MUIA_Numeric_Value);
}

//----------------------------------------------------------------------------

void _XCEXIT(long x)
{
}


static Object *CreatePrefsImage(void)
{
	Object	*img;

	// First try to load datatypes image from THEME: tree
	img = NewObject(DataTypesImageClass->mcc_Class, 0,
		MUIA_ScaDtpic_Name, (ULONG) "THEME:prefs/plugins/popupmenu",
		MUIA_ScaDtpic_FailIfUnavailable, TRUE,
		End; //DataTypesMCCObject

	if (NULL == img)
		img = IMG(PopupMenuPrefs, POPUPMENUPREFS);	// use built-in fallback image

	return img;
}


static void InitHooks(struct PopupMenuPrefsInst *inst)
{
	ULONG n;

	for (n=0; n<HOOKNDX_MAX; n++)
		{
		inst->mpb_Hooks[n] = PopupMenuPrefsHooks[n];
		inst->mpb_Hooks[n].h_Data = inst;
		}
}


static void SetChangedFlag(struct PopupMenuPrefsInst *inst, BOOL changed)
{
	if (changed != inst->mpb_Changed)
		{
		set(inst->mpb_Objects[OBJNDX_Lamp_Changed], 
			MUIA_Lamp_Color, changed ? MUIV_Lamp_Color_Ok : MUIV_Lamp_Color_Off);
		inst->mpb_Changed = changed;
		}
}

//-----------------------------------------------------------------

static void ParseToolTypes(struct PopupMenuPrefsInst *inst, struct MUIP_ScalosPrefs_ParseToolTypes *ptt)
{
#if 0
	STRPTR tt;

	d1(kprintf("%s/%ld: start  ptt=%08lx  tooltypes=%08lx\n", __FUNC__, __LINE__, ptt, ptt->ToolTypes));
	d1(kprintf("%s/%ld: start  tooltypes=%08lx %08lx %08lx\n", __FUNC__, __LINE__, ptt, ptt->ToolTypes[0], ptt->ToolTypes[1], ptt->ToolTypes[2]));

#endif
	d1(kprintf("%s/%ld: end\n", __FUNC__, __LINE__));
}

//----------------------------------------------------------------------------

BOOL initPlugin(struct PluginBase *PluginBase)
{
	MajorVersion = PluginBase->pl_LibNode.lib_Version;
	MinorVersion = PluginBase->pl_LibNode.lib_Revision;

	d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

	if (0x4711 == SIgnature++)
		{
		d1(kprintf("%s/%s/%ld:   PluginBase=%08lx  procName=<%s>\n", __FILE__, __FUNC__, __LINE__, \
			PluginBase, FindTask(NULL)->tc_Node.ln_Name));

		d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

		d1(kprintf("%s/%s/%ld:   PopupMenuPrefsLibBase=%08lx  procName=<%s>\n", __FILE__, __FUNC__, __LINE__, \
			PopupMenuPrefsLibBase, FindTask(NULL)->tc_Node.ln_Name));

#ifndef __amigaos4__
		DOSBase = (struct DosLibrary *) OpenLibrary(DOSNAME, 39 );
#else
		DOSBase = OpenLibrary(DOSNAME, 39);
#endif

		d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

		if (!OpenLibraries())
			return FALSE;

		d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

#ifndef __amigaos4__
		if (_STI_240_InitMemFunctions())
			return FALSE;
#endif

		d1(kprintf("%s/%s/%ld:\n", __FILE__, __FUNC__, __LINE__));

		PopupMenuPrefsClass = MUI_CreateCustomClass(&PluginBase->pl_LibNode, MUIC_Group,
				NULL, sizeof(struct PopupMenuPrefsInst), DISPATCHER_REF(PopupMenuPrefs));

		d1(kprintf("%s/%s/%ld:  PopupMenuPrefsClass=%08lx\n", __FILE__, __FUNC__, __LINE__, PopupMenuPrefsClass));
		if (NULL == PopupMenuPrefsClass)
			return FALSE;

		FrameButtonClass = InitFrameButtonClass();
		d1(KPrintF("%s/%s/%ld:  FrameButtonClass=%08lx\n", __FILE__, __FUNC__, __LINE__, FrameButtonClass));
		if (NULL == FrameButtonClass)
			return FALSE;

		d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));

		DataTypesImageClass = InitDtpicClass();
		d1(KPrintF(__FUNC__ "/%ld: DataTypesImageClass=%08lx\n", __LINE__, DataTypesImageClass));
		if (NULL == DataTypesImageClass)
			return FALSE;	// Failure

		if (LocaleBase)
			{
			if (NULL == PopupMenuPrefsLocale)
				PopupMenuPrefsLocale = OpenLocale(NULL);

			if (PopupMenuPrefsLocale)
				{
				if (NULL == PopupMenuPrefsCatalog)
					{
					PopupMenuPrefsCatalog = OpenCatalog(PopupMenuPrefsLocale,
						(STRPTR) "Scalos/ScalosPopupMenu.catalog", NULL);
					}
				}
			}

		TranslateStringArray(PrefsPageNames);
		TranslateStringArray(AppearAtNames);
		TranslateStringArray(PulldownModeNames);
		TranslateStringArray(ColorNames);
		TranslateStringArray(ColourMatchingPrecisionNames);
		TranslateStringArray(ImageSetNames);
		TranslateStringArray(AnimationNames);
		}

	d1(KPrintF("%s/%s/%ld: success\n", __FILE__, __FUNC__, __LINE__));

	return TRUE;	// Success
}

//----------------------------------------------------------------------------

static ULONG getv(APTR obj, ULONG attr)
{
    ULONG v;
    GetAttr(attr, obj, &v);
    return (v);
}

//----------------------------------------------------------------------------

#if !defined(__SASC) && !defined(__amigaos4__)
// Replacement for SAS/C library functions

#if !defined(__MORPHOS__)

static char *stpblk(const char *q)
{
	while (q && *q && isspace((int) *q))
		q++;

	return (char *) q;
}

static size_t stccpy(char *dest, const char *src, size_t MaxLen)
{
	size_t Count = 0;

	while (*src && MaxLen > 1)
		{
		*dest++ = *src++;
		MaxLen--;
		Count++;
		}
	*dest = '\0';
	Count++;

	return Count;
}
#endif /*__MORPHOS__*/

void exit(int x)
{
   (void) x;
   while (1)
      ;
}

APTR _WBenchMsg;

#endif /* !defined(__SASC) && !defined(__amigaos4__) */

