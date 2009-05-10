// MenuPrefs.h
// $Date$
// $Revision$


#ifndef SCALOS_MENUPREFS_H
#define	SCALOS_MENUPREFS_H

#include <Scalos/menu.h>


#define	DEFAULT_STACKSIZE	16384	// default stack size for new menu command entries

enum HookIndex
{
	HOOKNDX_TreeConstruct,
	HOOKNDX_TreeDestruct,
	HOOKNDX_TreeDisplay,

	HOOKNDX_CmdListConstruct,
	HOOKNDX_CmdListDestruct,
	HOOKNDX_CmdListDisplay,
	HOOKNDX_CmdListCompare,

	HOOKNDX_ImagePopAslFileStart,

	HOOKNDX_ResetToDefaults,
	HOOKNDX_Open,
	HOOKNDX_LastSaved,
	HOOKNDX_Restore,
	HOOKNDX_SaveAs,
	HOOKNDX_Merge,
	HOOKNDX_ImportTD,
	HOOKNDX_ImportP,
	HOOKNDX_RenameEntry,
	HOOKNDX_ChangeHotkey,
	HOOKNDX_ChangeEntry3,
	HOOKNDX_ChangeUnselectedImage,
	HOOKNDX_ChangeSelectedImage,

	HOOKNDX_SelectEntry,
	HOOKNDX_PopButton,
	HOOKNDX_AddMenu,
	HOOKNDX_AddCommand,
	HOOKNDX_AddMenuItem,
	HOOKNDX_RemoveEntry,
	HOOKNDX_About,
	HOOKNDX_CollapseAll,
	HOOKNDX_ExpandAll,
	HOOKNDX_CmdSelected,
	HOOKNDX_ContextMenuTrigger,
	HOOKNDX_AppMessage,
	HOOKNDX_SettingsChanged,
	HOOKNDX_AslIntuiMsg,
	HOOKNDX_HideObsolete,

	HOOKNDX_MAX
};

enum ObjectIndex
{
	OBJNDX_APP_Main,
	OBJNDX_WIN_Main,

	OBJNDX_Group_Main,
	OBJNDX_Group_List,
	OBJNDX_Group_Buttons1,
	OBJNDX_Group_Name,
	OBJNDX_Group_Hotkey,
	OBJNDX_Group_String, 
	OBJNDX_Group_CmdMisc,

	OBJNDX_StringHotkey,
	OBJNDX_CycleGad,
	OBJNDX_NewCommandButton, 
	OBJNDX_MainListView,
	OBJNDX_MainListTree,
	OBJNDX_HiddenListTree,
	OBJNDX_NameString,
	OBJNDX_NewMenuButton, 
	OBJNDX_NewItemButton, 
	OBJNDX_DelButton,
	OBJNDX_StringStack, 
	OBJNDX_CheckArgs,
	OBJNDX_SliderPriority,
	OBJNDX_StringCmd,
	OBJNDX_PopObject,
	OBJNDX_Popbutton,
	OBJNDX_CmdListView,
	OBJNDX_Lamp_Changed,
	OBJNDX_PopAsl_UnselectedImage,
	OBJNDX_PopAsl_SelectedImage,
	OBJNDX_DtImage_UnselectedImage,
	OBJNDX_DtImage_SelectedImage,

	OBJNDX_ContextMenu,
	OBJNDX_Menu_HideObsolete,

	OBJNDX_MAX
};

enum PopupMenuNodeIndex
{
	POPMENUINDEX_Disk,
	POPMENUINDEX_Drawer,
	POPMENUINDEX_ToolProject,
	POPMENUINDEX_Trashcan,
	POPMENUINDEX_Window,
	POPMENUINDEX_AppIcon,
	POPMENUINDEX_MAX	// MUST be last entry!!
};

enum DefaultMenuParentNode
	{
	DEFAULTPARENTNODE_MainMenu,
	DEFAULTPARENTNODE_Popup_Disk,
	DEFAULTPARENTNODE_Popup_Drawer,
	DEFAULTPARENTNODE_Popup_ToolProject,
	DEFAULTPARENTNODE_Popup_Trashcan,
	DEFAULTPARENTNODE_Popup_Window,
	DEFAULTPARENTNODE_Popup_AppIcon,
	DEFAULTPARENTNODE_LastInsertedMenu,
	DEFAULTPARENTNODE_LastInsertedMenuItem,
	DEFAULTPARENTNODE_LastInserted,
	};

struct DefaultMenuEntry
	{
	ULONG dme_EntryType;
	ULONG dme_Level;
	ULONG dme_NameId;
	ULONG dme_MenuShortId;
	enum DefaultMenuParentNode dme_ParentNode;
	enum ScalosMenuCommandType dme_CmdType;
	BYTE dme_CmdPri;
	UBYTE dme_CmdFlags;
	ULONG dme_CmdStackSize;
	};

struct MenuPrefsPopMenuNode
	{
	Object *mpn_Listtree;
	struct MUI_NListtree_TreeNode *mpn_ListNode;
	};

struct MenuPrefsInst
{
	ULONG	mpb_fCreateIcons;
	CONST_STRPTR	mpb_ProgramName;

	struct	Hook mpb_Hooks[HOOKNDX_MAX];
	APTR	mpb_Objects[OBJNDX_MAX];

	ULONG	mpb_PageIsActive;
	BOOL	mpb_Changed;
	BOOL	mpb_QuietFlag;
	BOOL	mpb_HideObsoletePopupMenus;

	struct Screen *mpb_WBScreen;

	struct FileRequester *mpb_LoadReq;
	struct FileRequester *mpb_SaveReq;

	struct MenuPrefsPopMenuNode mpb_MainMenuNode;
	struct MenuPrefsPopMenuNode mpb_PopMenuNode[POPMENUINDEX_MAX];
};

//----------------------------------------------------------------------------

#ifndef	Sizeof
#define	Sizeof(array)	(sizeof(array)/sizeof(array[0]))
#endif

//----------------------------------------------------------------------------

#define	d1(x)	;
#define	d2(x)	x;

// from debug.lib
extern int kprintf(const char *fmt, ...);
extern int KPrintF(const char *fmt, ...);

//----------------------------------------------------------------------------

#endif /* SCALOS_MENUPREFS_H */