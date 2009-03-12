/*
** This file was automatically generated by fdtrans 52.1.
** Do not edit it by hand. Instead, edit the sfd file
** that was used to generate this file
*/

#ifdef __USE_INLINE__
#undef __USE_INLINE__
#endif
#ifndef __NOGLOBALIFACE__
#define __NOGLOBALIFACE__
#endif

#include <exec/interfaces.h>
#include <exec/libraries.h>
#include <exec/emulation.h>
#include <interfaces/exec.h>
#include <interfaces/pm.h>
#include <proto/pm.h>


static inline int8  convert_int8 (uint32 x) { return x; }
static inline int16 convert_int16(uint32 x) { return x; }


STATIC struct Library * stub_OpenPPC(ULONG *regarray)
{
    struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
    struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((ULONG)Base + Base->lib_PosSize);
    struct LibraryManagerInterface *Self = (struct LibraryManagerInterface *) ExtLib->ILibrary;

    return Self->Open(0);
}
STATIC CONST struct EmuTrap stub_Open = { TRAPINST, TRAPTYPE, (ULONG (*)(ULONG *))stub_OpenPPC };

STATIC APTR stub_ClosePPC(ULONG *regarray)
{
    struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
    struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((ULONG)Base + Base->lib_PosSize);
    struct LibraryManagerInterface *Self = (struct LibraryManagerInterface *) ExtLib->ILibrary;

    return Self->Close();
}
STATIC CONST struct EmuTrap stub_Close = { TRAPINST, TRAPTYPE, (ULONG (*)(ULONG *))stub_ClosePPC };

STATIC APTR stub_ExpungePPC(ULONG *regarray __attribute__((unused)))
{
    return NULL;
}
STATIC CONST struct EmuTrap stub_Expunge = { TRAPINST, TRAPTYPE, (ULONG (*)(ULONG *))stub_ExpungePPC };

STATIC ULONG stub_ReservedPPC(ULONG *regarray __attribute__((unused)))
{
    return 0UL;
}
STATIC CONST struct EmuTrap stub_Reserved = { TRAPINST, TRAPTYPE, stub_ReservedPPC };

static struct PopupMenu * stub_PM_MakeMenuAPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_MakeMenuA(
		(struct TagItem *)regarray[9]
	);
}
STATIC CONST struct EmuTrap stub_PM_MakeMenuA = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_MakeMenuAPPC };

static struct PopupMenu * stub_PM_MakeItemAPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_MakeItemA(
		(struct TagItem *)regarray[9]
	);
}
STATIC CONST struct EmuTrap stub_PM_MakeItemA = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_MakeItemAPPC };

static VOID stub_PM_FreePopupMenuPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	Self->PM_FreePopupMenu(
		(struct PopupMenu *)regarray[9]
	);
}
STATIC CONST struct EmuTrap stub_PM_FreePopupMenu = { TRAPINST, TRAPTYPENR, (uint32 (*)(uint32 *))stub_PM_FreePopupMenuPPC };

static ULONG stub_PM_OpenPopupMenuAPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_OpenPopupMenuA(
		(struct Window *)regarray[9],
		(struct TagItem *)regarray[10]
	);
}
STATIC CONST struct EmuTrap stub_PM_OpenPopupMenuA = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_OpenPopupMenuAPPC };

static struct PM_IDLst * stub_PM_MakeIDListAPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_MakeIDListA(
		(struct TagItem *)regarray[9]
	);
}
STATIC CONST struct EmuTrap stub_PM_MakeIDListA = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_MakeIDListAPPC };

static BOOL stub_PM_ItemCheckedPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_ItemChecked(
		(struct PopupMenu *)regarray[9],
		(ULONG)regarray[1]
	);
}
STATIC CONST struct EmuTrap stub_PM_ItemChecked = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_ItemCheckedPPC };

static LONG stub_PM_GetItemAttrsAPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_GetItemAttrsA(
		(struct PopupMenu *)regarray[10],
		(struct TagItem *)regarray[9]
	);
}
STATIC CONST struct EmuTrap stub_PM_GetItemAttrsA = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_GetItemAttrsAPPC };

static LONG stub_PM_SetItemAttrsAPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_SetItemAttrsA(
		(struct PopupMenu *)regarray[10],
		(struct TagItem *)regarray[9]
	);
}
STATIC CONST struct EmuTrap stub_PM_SetItemAttrsA = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_SetItemAttrsAPPC };

static struct PopupMenu * stub_PM_FindItemPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_FindItem(
		(struct PopupMenu *)regarray[9],
		(ULONG)regarray[1]
	);
}
STATIC CONST struct EmuTrap stub_PM_FindItem = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_FindItemPPC };

static VOID stub_PM_AlterStatePPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	Self->PM_AlterState(
		(struct PopupMenu *)regarray[9],
		(struct PM_IDLst *)regarray[10],
		(UWORD)(regarray[1] & 0xffff)
	);
}
STATIC CONST struct EmuTrap stub_PM_AlterState = { TRAPINST, TRAPTYPENR, (uint32 (*)(uint32 *))stub_PM_AlterStatePPC };

static struct PM_IDLst * stub_PM_ExLstAPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_ExLstA(
		(ULONG *)regarray[9]
	);
}
STATIC CONST struct EmuTrap stub_PM_ExLstA = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_ExLstAPPC };

static APTR stub_PM_FilterIMsgAPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_FilterIMsgA(
		(struct Window *)regarray[8],
		(struct PopupMenu *)regarray[9],
		(struct IntuiMessage *)regarray[10],
		(struct TagItem *)regarray[11]
	);
}
STATIC CONST struct EmuTrap stub_PM_FilterIMsgA = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_FilterIMsgAPPC };

static LONG stub_PM_InsertMenuItemAPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_InsertMenuItemA(
		(struct PopupMenu *)regarray[8],
		(struct TagItem *)regarray[9]
	);
}
STATIC CONST struct EmuTrap stub_PM_InsertMenuItemA = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_InsertMenuItemAPPC };

static struct PopupMenu * stub_PM_RemoveMenuItemPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_RemoveMenuItem(
		(struct PopupMenu *)regarray[8],
		(struct PopupMenu *)regarray[9]
	);
}
STATIC CONST struct EmuTrap stub_PM_RemoveMenuItem = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_RemoveMenuItemPPC };

static BOOL stub_PM_AbortHookPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_AbortHook(
		(APTR)regarray[8]
	);
}
STATIC CONST struct EmuTrap stub_PM_AbortHook = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_AbortHookPPC };

static STRPTR stub_PM_GetVersionPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_GetVersion(
	);
}
STATIC CONST struct EmuTrap stub_PM_GetVersion = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_GetVersionPPC };

static VOID stub_PM_ReloadPrefsPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	Self->PM_ReloadPrefs(
	);
}
STATIC CONST struct EmuTrap stub_PM_ReloadPrefs = { TRAPINST, TRAPTYPENR, (uint32 (*)(uint32 *))stub_PM_ReloadPrefsPPC };

static LONG stub_PM_LayoutMenuAPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	return Self->PM_LayoutMenuA(
		(struct Window *)regarray[8],
		(struct PopupMenu *)regarray[9],
		(struct TagItem *)regarray[10]
	);
}
STATIC CONST struct EmuTrap stub_PM_LayoutMenuA = { TRAPINST, TRAPTYPE, (uint32 (*)(uint32 *))stub_PM_LayoutMenuAPPC };

static VOID stub_PM_FreeIDListPPC(uint32 *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((uint32)Base + Base->lib_PosSize);
	struct PopupMenuIFace *Self = (struct PopupMenuIFace *) ExtLib->MainIFace;

	Self->PM_FreeIDList(
		(struct PM_IDLst *)regarray[8]
	);
}
STATIC CONST struct EmuTrap stub_PM_FreeIDList = { TRAPINST, TRAPTYPENR, (uint32 (*)(uint32 *))stub_PM_FreeIDListPPC };

CONST CONST_APTR VecTable68K[] =
{
	&stub_Open,
	&stub_Close,
	&stub_Expunge,
	&stub_Reserved,
	&stub_PM_MakeMenuA,
	&stub_PM_MakeItemA,
	&stub_PM_FreePopupMenu,
	&stub_PM_OpenPopupMenuA,
	&stub_PM_MakeIDListA,
	&stub_PM_ItemChecked,
	&stub_PM_GetItemAttrsA,
	&stub_PM_SetItemAttrsA,
	&stub_PM_FindItem,
	&stub_PM_AlterState,
	&stub_Reserved,
	&stub_PM_ExLstA,
	&stub_PM_FilterIMsgA,
	&stub_PM_InsertMenuItemA,
	&stub_PM_RemoveMenuItem,
	&stub_PM_AbortHook,
	&stub_PM_GetVersion,
	&stub_PM_ReloadPrefs,
	&stub_PM_LayoutMenuA,
	&stub_Reserved,
	&stub_PM_FreeIDList,
	(CONST_APTR)-1
};
