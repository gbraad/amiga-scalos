// ScalosPrefs.h
// $Date$
// $Revision$


#ifndef SCALOSPREFS_H
#define SCALOSPREFS_H

#include <stdarg.h>
#include <intuition/classusr.h> 
#include <scalos/preferences.h>
#include <scalos/scalosprefs.h>

#define	MAX_FILENAME	256
#define	MAX_FONTNAME	128
#define	MAX_TTFONTDESC	256

// Limits for plugin strings
#define	MAX_PLUGIN_CLASSNAME		50
#define	MAX_PLUGIN_SUPERCLASSNAME	50
#define	MAX_PLUGIN_REALNAME		100
#define	MAX_PLUGIN_DESCRIPTION		200
#define	MAX_PLUGIN_CREATORNAME		50


#define	SYS_FONTPREFS_NAME              "SYS:Prefs/Font"
#define	SYS_WORKBENCHPREFS_NAME         "SYS:Prefs/Workbench"


enum ObjIndex
{
	APPLICATION,
	WINDOW_MAIN,
	WINDOW_ICONFRAMES,
	WINDOW_SPLASH,
	GROUP_MAIN,
	TEXT_SPLASHWINDOW,
	LISTVIEW,
	LIST,
	SAVE,
	USE,
	CANCEL,
	GROUP,
	CYCLE_SCREENTITLEMODE,
	STORAGE_TIPS,
	USED_TIPS,
	CHECK_TOOLTIPS,
	SLIDER_TITLEREFRESH,
	CHECK_TITLEMEM,
	SLIDER_WINTITLEREFRESH,
	CHECK_WINTITLEMEM,
	SLIDER_TIPDELAY,
	CHECK_MULTILINES,
	CHECK_HILITEUNDERMOUSE,
	SLIDER_LABELSPACE,
	CYCLE_LABELSTYLE,
	NLIST_STORAGE_FILEDISPLAY,
	NLISTVIEW_STORAGE_FILEDISPLAY,
	STRING_CONSOLENAME,
	CLOSEWB,
	DROPSTART,
	CHECK_AUTOLEAVEOUT,
	POP_TEXTMODEFONT,
	GROUP_TEXTWINDOW_FONT_SELECT,
	STRING_TEXTMODEFONT_SAMPLE,
	GROUP_TEXTWINDOW_TTTEXTWINDOWFONT,
	MCC_TEXTWINDOW_TTTEXTWINDOWFONT_SAMPLE,
	CHECK_TEXTWINDOW_TTTEXTWINDOWFONT_ENABLE,
	POPSTRING_TEXTWINDOW_TTTEXTWINDOWFONT,
	SOFTTEXTSLINK,
	SOFTICONSLINK,
	TOOLTIP_SETTINGSGROUP,
	CHECK_SHOWSPLASH,
	SLIDER_SPLASHCLOSE,
	GROUP_SCRTITLE,

//	MOD_DELETE,
//	MOD_TRASH,
//	MOD_EXECOM,
//	MOD_INFO,
//	MOD_NEWDRAWER,
//	MOD_REBOOT,
//	MOD_RENAME,
//	MOD_SYSINFO,
//	MOD_DUMMY,
//	MOD_DUMMY2,

	CYCLE_ROUTINE,
	CYCLE_STYLE,
	POP_FONTPREFS,
	MCC_ICONFONT_SAMPLE,
	POP_ICONFONT,
	GROUP_ICONSPAGE_TTICONFONT,
	CHECK_ICONSPAGE_TTICONFONT_ENABLE,
	POPSTRING_ICONSPAGE_TTICONFONT,
	MCC_ICONSPAGE_TTICONFONT_SAMPLE,
	POP_WORKBENCHPREFS,

	GROUP_ICONFONT_NOTICE,
	GROUP_ICONFONT_SELECT,
	GROUP_ICONSPAGE_ICONFONT,

	ADD_PLUGIN,
	REMOVE_PLUGIN,
	PLUGIN_FILENAME,
	PLUGIN_NAME,
	PLUGIN_VERSION,
	PLUGIN_LIST,
	PLUGIN_LISTVIEW,
	PLUGIN_DESCRIPTION,
	PLUGIN_CREATOR,

	CYCLE_SHOWTHUMBNAILS,
	CHECK_SHOWTHUMBNAILS_AS_DEFAULT,
	CYCLE_THUMBNAILSIZE,
	SLIDER_THUMBNAILS_MAXAGE,
	SLIDER_THUMBNAILS_MINSIZE_LIMIT,
	SLIDER_THUMBNAILS_QUALITY,
	CHECK_THUMBNAILS_SQUARE,
	CHECK_THUMBNAILS_BACKFILL,
	SLIDER_ICONTRANSPARENCY_THUMBNAILBACK,
	GROUP_THUMBNAIL_BACKFILL_TRANSPARENCY,
	CYCLE_ICONMINSIZE,
	CYCLE_ICONMAXSIZE,
	CHECK_AUTOREMOVE,
	CHECK_EASY_MULTISELECT,
	CHECK_EASY_MULTIDRAG,
	CHECK_GROUPDRAG,
	CHECK_ENABLE_DROPMENU,
	SLIDER_POPOPENWINDOW_DELAY,
	COPY_HOTKEY,
	COPY_HOTKEY_SCAN,
	MAKELINK_HOTKEY,
	MAKELINK_HOTKEY_SCAN,
	MOVE_HOTKEY,
	MOVE_HOTKEY_SCAN,
	SINGLE_WINDOW_LASSO_HOTKEY,
	SLIDER_WBSDELAY,
	SLIDER_DISKREFRESH,
	CHECK_ICONMASK,
	CHECK_MULTISELECT,
	SLIDER_ICONRMAP_PRECISION,
	CHECK_NIMASK,
	CHECK_DEFFIRST,
	SLIDER_ICONTRANSPARENCY_DRAG,
	SLIDER_ICONTRANSPARENCY_SHADOW,
	SLIDER_ICONTRANSPARENCY_DEFICONS,
	CHECK_MISCPAGE_POPUP_SELECTED_ALWAYS,

	ICONLEFT,
	ICONTOP,
	ICONRIGHT,
	ICONBOTTOM,
	CHECK_ICONSSAVE,
	RADIO_BOBLOOK,
	CYCLE_TRANSPMODE,

	CHECK_SEL_ICONTEXT_RECTANGLE,
	SLIDER_SEL_ICONTEXT_RECT_BORDERX,
	SLIDER_SEL_ICONTEXT_RECT_BORDERY,
	SLIDER_SEL_ICONTEXT_RECT_RADIUS,

	NLISTVIEW_CONTROLBARGADGETS_NORMAL_AVAILABLE,
	NLIST_CONTROLBARGADGETS_NORMAL_AVAILABLE,
	NLISTVIEW_CONTROLBARGADGETS_NORMAL_ACTIVE,
	NLIST_CONTROLBARGADGETS_NORMAL_ACTIVE,
	STRING_CONTROLBARGADGETS_NORMAL_NORMALIMAGE,
	STRING_CONTROLBARGADGETS_NORMAL_SELECTEDIMAGE,
	STRING_CONTROLBARGADGETS_NORMAL_DISABLEDIMAGE,
	POP_CONTROLBARGADGETS_NORMAL_ACTION,
	NLISTVIEW_CONTROLBARGADGETS_NORMAL_ACTION,
	STRING_CONTROLBARGADGETS_NORMAL_ACTION,
	TEXTEDITOR_CONTROLBARGADGETS_NORMAL_HELPTEXT,
	DTIMG_CONTROLBARGADGETS_NORMAL_NORMALIMAGE,
	DTIMG_CONTROLBARGADGETS_NORMAL_SELECTEDIMAGE,
	DTIMG_CONTROLBARGADGETS_NORMAL_DISABLEDIMAGE,

	NLISTVIEW_CONTROLBARGADGETS_BROWSER_AVAILABLE,
	NLIST_CONTROLBARGADGETS_BROWSER_AVAILABLE,
	NLISTVIEW_CONTROLBARGADGETS_BROWSER_ACTIVE,
	NLIST_CONTROLBARGADGETS_BROWSER_ACTIVE,
	STRING_CONTROLBARGADGETS_BROWSER_NORMALIMAGE,
	STRING_CONTROLBARGADGETS_BROWSER_SELECTEDIMAGE,
	STRING_CONTROLBARGADGETS_BROWSER_DISABLEDIMAGE,
	POP_CONTROLBARGADGETS_BROWSER_ACTION,
	NLISTVIEW_CONTROLBARGADGETS_BROWSER_ACTION,
	STRING_CONTROLBARGADGETS_BROWSER_ACTION,
	TEXTEDITOR_CONTROLBARGADGETS_BROWSER_HELPTEXT,
	DTIMG_CONTROLBARGADGETS_BROWSER_NORMALIMAGE,
	DTIMG_CONTROLBARGADGETS_BROWSER_SELECTEDIMAGE,
	DTIMG_CONTROLBARGADGETS_BROWSER_DISABLEDIMAGE,

	THUMBNAILS_LEFTBORDER,
	THUMBNAILS_TOPBORDER,
	THUMBNAILS_RIGHTBORDER,
	THUMBNAILS_BOTTOMBORDER,

	CHECK_TRIG_DISK,
	CHECK_TRIG_DRAWER,
	CHECK_TRIG_TOOL,
	CHECK_TRIG_PROJECT,
	CHECK_TRIG_TRASHCAN,
	CHECK_TRIG_KICK,
	CHECK_TRIG_APPICON,
	CHECK_TRIG_APPWIN,
	CHECK_TRIG_ICONIFIEDWIN,
	CHECK_TRIG_SCALOSWIN,
	CHECK_TRIG_DUMMY,

	CYCLE_DESKTOP_LAYOUT_WBDISK,
	CYCLE_DESKTOP_LAYOUT_WBDRAWER,
	CYCLE_DESKTOP_LAYOUT_WBTOOL,
	CYCLE_DESKTOP_LAYOUT_WBPROJECT,
	CYCLE_DESKTOP_LAYOUT_WBGARBAGE,
	CYCLE_DESKTOP_LAYOUT_WBDEVICE,
	CYCLE_DESKTOP_LAYOUT_WBKICK,
	CYCLE_DESKTOP_LAYOUT_WBAPPICON,

	CYCLE_ICONS_LAYOUT_WBDRAWER,
	CYCLE_ICONS_LAYOUT_WBTOOL,
	CYCLE_ICONS_LAYOUT_WBPROJECT,

	CYCLE_WINDOWTYPE,
	CHECK_POPTITLEONLY,
	CHECK_MMBMOVE,
	WINLEFT,
	WINTOP,
	WINWIDTH,
	WINHEIGHT,
	SLIDER_CLEANLEFT,
	SLIDER_CLEANTOP,
	SLIDER_CLEANXSKIP,
	SLIDER_CLEANYSKIP,
	NLIST_USE_FILEDISPLAY,
	NLISTVIEW_USE_FILEDISPLAY,
	HIDEHIDDENFILES,
	HIDEPROTECTHIDDENFILES,
	CYCLE_SHOWALLDEFAULT,
	CYCLE_VIEWBYICONSDEFAULT,

	SLIDER_TRANSPARENCY_ACTIVEWINDOW,
	SLIDER_TRANSPARENCY_INACTIVEWINDOW,

	CHECK_STRIPED_WINDOW,
	CHECK_SELECTTEXTICONNAME,

	GROUP_TTSCREENFONT,
	GROUP_TTGLOBALS,
	CYCLE_TTANTIALIAS,
	SLIDER_TTGAMMA,
	CHECK_TTSCREENFONT_ENABLE,
	POPSTRING_TTSCREENFONT,
	MCC_TTSCREENFONT_SAMPLE,
	GROUP_TTICONFONT,
	CHECK_TTICONFONT_ENABLE,
	POPSTRING_TTICONFONT,
	MCC_TTICONFONT_SAMPLE,
	GROUP_TTTEXTWINDOWFONT,
	CHECK_TTTEXTWINDOWFONT_ENABLE,
	POPSTRING_TTTEXTWINDOWFONT,
	MCC_TTTEXTWINDOWFONT_SAMPLE,

	CYCLE_CREATELINKS,
	CHECK_USEEXALL,
	CHECK_HARDEMULATION,
	CHECK_MENUCURRENTDIR,
	POPUP_SELECTED_HOTKEY,
	GROUP_DEFAULTSTACKSIZE,
	GROUP2_DEFAULTSTACKSIZE,
	SLIDER_DEFAULTSTACKSIZE,

	POPPH_ROOTWINDOWTITLE,
	POPPH_DIRWINDOWTITLE,
	POPPH_SCREENTITLE,
	STRING_TOOLTIPFONT,
	SLIDER_ICONTOOLTIPS_TRANSPARENCY,

	SLIDER_TEXTWINDOWS_SELECTBORDERTRANSPARENCY,
	SLIDER_TEXTWINDOWS_SELECTFILLTRANSPARENCY,
	COLORADJUST_TEXTWINDOWS_SELECTIONMARK,
	MCC_TEXTWINDOWS_SELECTMARKER_SAMPLE,
	GROUP_TEXTWINDOWS_SELECTIONMARK,

	STRING_WBSTARTPATH,
	STRING_SCALOSHOME,
	STRING_THEMES,
	STRING_IMAGECACHE,
	STRING_THUMBNAILDB,
	STRING_DISKCOPY,
	STRING_FORMAT,
	STRING_DEFICONPATH,
	STRING_SQLITE3TEMPPATH,

	CHECK_SHOWDRAG,
	CHECK_STATUSBAR,
	CHECK_CHECKOVERLAP,
	CYCLE_DROPMARK,
	CHECK_CLEANUP_ONRESIZE,

	FRAME_ICONNORMAL,
	FRAME_ICONSELECTED,
	FRAME_ICON_THUMBNAIL_NORMAL,
	FRAME_ICON_THUMBNAIL_SELECTED,

	NLISTVIEW_HIDDENDEVICES,
	NLIST_HIDDENDEVICES,

	GROUP_COPYBUFFERSIZE,
	SLIDER_COPYBUFFERSIZE,

	SLIDER_UNDOSTEPS,

	MENU_OPEN,
	MENU_SAVE_AS,
	MENU_ABOUT,
	MENU_ABOUT_MUI,
	MENU_QUIT,
	MENU_RESET_TO_DEFAULTS,
	MENU_LAST_SAVED,
	MENU_RESTORE,
	MENU_CREATE_ICONS,

	// ------------------------------

	NLISTVIEW_MODULES,
	NLIST_MODULES,

	TOTAL
};

enum ImgIndex
{
	// prefs page images
	IMAGE_ABOUT,
	IMAGE_PATHS,
	IMAGE_STARTUP,
	IMAGE_DESKTOP,
	IMAGE_ICONS,
	IMAGE_DRAGNDROP,
	IMAGE_WINDOWS,
	IMAGE_TEXTWINDOWS,
	IMAGE_TTFONTS,
	IMAGE_MISC,
	IMAGE_PLUGINS,
	IMAGE_MODULES,

	IMAGE_FILEDISPLAY,
	// ------------------------------

	// modules images
	IMAGE_DELETE,
	IMAGE_EMPTY_TRASHCAN,
	IMAGE_EXECUTE_COMMAND,
	IMAGE_FORMAT_DISK,
	IMAGE_INFORMATION,
	IMAGE_ICONPROPERTIES,
	IMAGE_NEWDRAWER,
	IMAGE_REBOOT,
	IMAGE_RENAME,
	IMAGE_SYSTEMINFO,
	IMAGE_WINDOWPROPERTIES,
};

//--------------------------------------------------------------------

#define	MUIV_Application_ReturnID_EDIT	1000
#define	MUIV_Application_ReturnID_USE	1001
#define	MUIV_Application_ReturnID_SAVE	1002

//--------------------------------------------------------------------

// Global Variables

struct SCAModule
{
    Object *Obj[TOTAL]; // pointers to various objects in GUI
};

//--------------------------------------------------------------------

struct PluginDef
{
	struct	MinNode plug_Node;
	UWORD   plug_libVersion;	    /* major */
	UWORD   plug_libRevision;	    /* minor */
	WORD	plug_priority;
	UWORD	plug_instsize;
	UWORD	plug_NeededVersion;
	BOOL	plug_SuccessfullyLoaded;
	char	plug_classname[MAX_PLUGIN_CLASSNAME];
	char	plug_superclassname[MAX_PLUGIN_SUPERCLASSNAME];
	char	plug_RealName[MAX_PLUGIN_REALNAME];
	char	plug_Description[MAX_PLUGIN_DESCRIPTION];
	char	plug_Creator[MAX_PLUGIN_CREATORNAME];
	char	plug_filename[MAX_FILENAME];	//DiskPlugin filename

	char	plug_VersionString[30];
};

//--------------------------------------------------------------------

struct ControlBarGadgetEntry
	{
	struct	Node cgy_Node;

	enum SCPGadgetTypes cgy_GadgetType;
	ULONG 	cgy_ImageObjectIndex;   // internal, only for list display
	Object 	*cgy_Image;		// Image object for normal image or NULL
	char    cgy_ImageString[20];	// internal, only for list display
	char 	cgy_Action[40];		// Name of Menu Command
	STRPTR 	cgy_NormalImage;	// file name of normal state image
	STRPTR	cgy_SelectedImage;	// file name of selected state image
	STRPTR	cgy_DisabledImage;	// file name of disabled state image
	STRPTR	cgy_HelpText;		// Bubble help text
	};

//--------------------------------------------------------------------

struct NewPageListEntry
{
	CONST_STRPTR nple_TitleString;
	Object *nple_TitleImage;
	ULONG nple_ImageIndex;
	Object *(*nple_CreateTitleImage)(void *UserData);
	void *nple_UserData;
};

//--------------------------------------------------------------------

struct CommandTableEntry
	{
	CONST_STRPTR cte_Command;
	ULONG cte_NameMsgId;
	};

//--------------------------------------------------------------------

struct FileDisplayListEntry
{
	ULONG	fdle_Index;	// Index into cFileDisplayColumns[]
};

//--------------------------------------------------------------------

struct ModuleListEntry
{
	Object *mdle_Image;
	Object *mdle_Popup;

	ULONG mdle_EntryIndex;			// continuous index starting from 0, returned by MUIA_NList_ButtonClick

	ULONG mdle_ImageObjectIndex;		// MUIM_NList_UseImage index for module logo
	ULONG mdle_PopupObjectIndex;		// MUIM_NList_UseImage index for popup button

	char mdle_Modulename[108];		// module name
	char mdle_Filename[512];		// File name for module prefs executable

	char mdle_ImageString[20];
	char mdle_PopupObjectString[20];
};

//--------------------------------------------------------------------

struct NewHiddenDevice
	{
	ULONG nhd_Hidden;
	CONST_STRPTR nhd_DeviceName;
	CONST_STRPTR nhd_VolumeName;
	};

struct HiddenDeviceListEntry
	{
	Object *hde_CheckboxImage;

	ULONG hde_EntryIndex;			// continuous index starting from 0, returned by MUIA_NList_ButtonClick

	ULONG hde_CheckboxObjectIndex;		// MUIM_NList_UseImage index for checkbox

	ULONG hde_Hidden;

	char hde_CheckboxString[20];
	char hde_DeviceName[128];		// device name
	char hde_VolumeName[128];		// volume name
	};

//--------------------------------------------------------------------

// Values for Screen title bar cycle
enum ScreenTitleModes
{
	SCREENTITLE_Visible = 0,	// always visible
	SCREENTITLE_Popup,		// popup under mouse
	SCREENTITLE_Hidden,		// always hidden
};

//--------------------------------------------------------------------

// Values for Icon min/max size limits
enum IconSizes
{
	ICONSIZE_16 = 0,
	ICONSIZE_24,
	ICONSIZE_32,
	ICONSIZE_48,
	ICONSIZE_64,
	ICONSIZE_96,
	ICONSIZE_128,
	ICONSIZE_Unlimited,
};

//--------------------------------------------------------------------

// Values for Thumbnail sizes
enum ThumbnailSizes
{
	THUMBNAILSIZE_16 = 0,
	THUMBNAILSIZE_24,
	THUMBNAILSIZE_32,
	THUMBNAILSIZE_48,
	THUMBNAILSIZE_64,
	THUMBNAILSIZE_96,
	THUMBNAILSIZE_128,
};

//--------------------------------------------------------------------

VOID MsgBox(char *message, char *, struct SCAModule *);
CONST_STRPTR GetLocString(ULONG MsgId);
LONG SaveIcon(CONST_STRPTR IconName);
BOOL CheckMCCVersion(CONST_STRPTR name, ULONG minver, ULONG minrev);
void SetThumbNailSize(struct SCAModule *app, UWORD ThumbnailSize);
UWORD GetThumbNailSize(struct SCAModule *app);
void SetIconSizeConstraints(struct SCAModule *app, const struct Rectangle *SizeConstraints);
void GetIconSizeConstraints(struct SCAModule *app, struct Rectangle *SizeConstraints);
Object *CreatePrefsPage(struct SCAModule *app, Object *Page, struct NewPageListEntry *nple);
BOOL ExistsObject(CONST_STRPTR Name);

//--------------------------------------------------------------------

#ifndef __SASC
// Replacement for SAS/C library functions
size_t stccpy(char *dest, const char *src, size_t MaxLen);
#endif /* __SASC */

#if defined(__SASC)
int snprintf(char *, size_t, const char *, /*args*/ ...);
int vsnprintf(char *, size_t, const char *, va_list ap);
#endif /* __SASC */

//--------------------------------------------------------------------

// from debug.lib
extern int kprintf(const char *fmt, ...);
extern int KPrintF(const char *fmt, ...);

//--------------------------------------------------------------------

#define	d1(x)		;
#define	d2(x)		x;

//--------------------------------------------------------------------

#define	max(x, y)	((x) > (y) ? (x) : (y))
#define	min(x, y)	((x) > (y) ? (y) : (x))

#define	Sizeof(array)	(sizeof(array) / sizeof(array[0]))

//--------------------------------------------------------------------

extern ULONG fCreateIcons;
extern ULONG ModuleImageIndex;
extern STRPTR ProgramName;
extern 	struct Hook AslIntuiMsgHook;
extern struct Hook CalculateMaxRadiusHook;

#endif /* SCALOSPREFS_H */
