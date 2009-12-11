#ifndef SCALOS_UNDO_H
#define SCALOS_UNDO_H
/*
**  $VER: undo.h 41.1 (23 Mar 2009 21:05:36)
**
**	scalosgfx.library include
**
**  (C) Copyright 2009 The Scalos Team
**  All Rights Reserved
*/

#ifndef	 EXEC_TYPES_H
#include <exec/types.h>
#endif	//EXEC_TYPES_H

#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif	//UTILITY_TAGITEM_H

//----------------------------------------------------------------------------

enum ScalosUndoType
	{
	UNDO_Snapshot,
	UNDO_Unsnapshot,
	UNDO_Cleanup,
	UNDO_Copy,
	UNDO_Move,
	UNDO_CreateLink,
	UNDO_ChangeIconPos,
	UNDO_Rename,
	UNDO_Relabel,
	UNDO_Delete,
	UNDO_Leaveout,
	UNDO_PutAway,
	UNDO_NewDrawer,
	UNDO_SizeToFit,
	};

enum ScalosUndoTags
	{
	UNDOTAG_UndoMultiStep = (int) (TAG_USER + 1287),
	UNDOTAG_IconNode,		// struct ScaIconNode *
	UNDOTAG_IconList,		// struct ScaIconNode *
	UNDOTAG_IconDirLock,		// BPTR
	UNDOTAG_CopySrcDirLock,		// BPTR
	UNDOTAG_CopyDestDirLock,	// BPTR
	UNDOTAG_CopySrcName,		// CONST_STRPTR
	UNDOTAG_CopyDestName,		// CONST_STRPTR
	UNDOTAG_IconPosX,		// LONG
	UNDOTAG_IconPosY,		// LONG
	UNDOTAG_OldIconPosX,		// LONG
	UNDOTAG_OldIconPosY,		// LONG
	UNDOTag_WindowTask,		// struct internalScaWindowTask *
	UNDOTag_CleanupMode,		// ULONG
	UNDOTAG_UndoHook,		// struct Hook *
	UNDOTAG_RedoHook,		// struct Hook *
	UNDOTAG_DisposeHook,		// struct Hook *
	UNDOTAG_SaveIcon,		// ULONG
	UNDOTAG_CustomAddHook,		// struct Hook *
	UNDOTAG_WbArg,			// struct WBArg *
	UNDOTAG_OldWindowLeft,		// LONG
	UNDOTAG_OldWindowTop,           // LONG
	UNDOTAG_OldWindowWidth,		// ULONG
	UNDOTAG_OldWindowHeight,	// ULONG
	UNDOTAG_NewWindowLeft,		// LONG
	UNDOTAG_NewWindowTop,           // LONG
	UNDOTAG_NewWindowWidth,		// ULONG
	UNDOTAG_NewWindowHeight,	// ULONG
	UNDOTAG_OldWindowVirtX,		// LONG
	UNDOTAG_OldWindowVirtY,		// LONG
	UNDOTAG_NewWindowVirtX,		// LONG
	UNDOTAG_NewWindowVirtY,		// LONG
	UNDOTAG_CreateIcon,		// LONG
	UNDOTAG_IconName,		// CONST_STRPTR
	};

enum ScalosUndoCleanupMode
	{
	CLEANUP_Default,
	CLEANUP_ByName,
	CLEANUP_ByDate,
	CLEANUP_BySize,
	CLEANUP_ByType,
	};

struct UndoCleanupIconEntry
	{
	const struct ScaIconNode *ucin_IconNode;
	LONG ucin_Left;			// original x position of icon
	LONG ucin_Top;			// original y position of icon
	};

struct UndoEvent
	{
	struct Node uev_Node;
	enum ScalosUndoType uev_Type;

	struct Hook *uev_UndoHook;
	struct Hook *uev_RedoHook;
	struct Hook *uev_DisposeHook;
	struct Hook *uev_CustomAddHook;

	LONG uev_OldPosX;
	LONG uev_OldPosY;
	LONG uev_NewPosX;
	LONG uev_NewPosY;
	
	union UndoEventData
		{
		APTR uev_PrivateData;
		struct UndoCopyMoveEventData
			{
			// we do not keep Locks to src/dest directories here since it wouldn't be nice to block rename/delete of referenced objects!
			STRPTR ucmed_srcDirName;
			STRPTR ucmed_destDirName;
			STRPTR ucmed_srcName;
			STRPTR ucmed_destName;
			} uev_CopyMoveData;
		struct UndoNewDrawerData
			{
			// we do not keep Locks to src directory here since it wouldn't be nice to block rename/delete of referenced objects!
			STRPTR und_DirName;
			STRPTR und_srcName;
			BOOL und_CreateIcon;
			} uev_NewDrawerData;
		struct UndoIconEventData
			{
			// we do not keep Locks to icon directory here since it wouldn't be nice to block rename/delete of referenced objects!
			STRPTR uid_DirName;
			STRPTR uid_IconName;
			} uev_IconData;
		struct UndoCleanupData
			{
			enum ScalosUndoCleanupMode ucd_CleanupMode;
			struct UndoCleanupIconEntry *ucd_Icons;
			ULONG ucd_IconCount;		// number of entries in ucd_Icons
			struct internalScaWindowTask *ucd_WindowTask;
			} uev_CleanupData;
		struct UndoSnaphotIconData
			{
			STRPTR usid_DirName;
			STRPTR usid_IconName;
			Object *usid_IconObj;		// Iconobject BEFORE snapshot/unsnaphot
			ULONG usid_SaveIcon;
			} uev_SnapshotData;
		struct UndoSizeWindowData
			{
			struct internalScaWindowTask *uswd_WindowTask;
			LONG uswd_OldLeft;
			LONG uswd_OldTop;
			ULONG uswd_OldWidth;
			ULONG uswd_OldHeight;
			LONG uswd_NewLeft;
			LONG uswd_NewTop;
			ULONG uswd_NewWidth;
			ULONG uswd_NewHeight;
			LONG uswd_OldVirtX;
			LONG uswd_OldVirtY;
			LONG uswd_NewVirtX;
			LONG uswd_NewVirtY;
			} uev_SizeWindowData;
		} uev_Data;

	struct UndoStep *uev_UndoStep;
	};

/*----------------------------------------------------------------------------------*/

#endif /* SCALOS_UNDO_H */
