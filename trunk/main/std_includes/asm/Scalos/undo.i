	IFND	SCA_UNDO_I
SCA_UNDO_I     SET     1
**
**	$VER: undo.i 41.1 (23 Oct 2009 21:31:48)
**
**	File format for scalos_pattern preferences
**
**	(C) Copyright 2009 The Scalos Team
**	All Rights Reserved
**

;---------------------------------------------------------------------------

	IFND UTILITY_TAGITEM_I
	INCLUDE "utility/tagitem.i"
	ENDC

;---------------------------------------------------------------------------

;enum ScalosUndoType

UNDO_Snapshot		equ	0
UNDO_Unsnapshot		equ	1
UNDO_Cleanup		equ	2
UNDO_Copy		equ	3
UNDO_Move		equ	4
UNDO_CreateLink		equ	5
UNDO_ChangeIconPos	equ	6

;enum ScalosUndoTags
UNDOTAG_TagBase		equ     TAG_USER+1287

UNDOTAG_UndoMultiStep	equ     UNDOTAG_TagBase+1
UNDOTAG_IconNode	equ	UNDOTAG_TagBase+1	; struct ScaIconNode *
UNDOTAG_IconList	equ	UNDOTAG_TagBase+2	; struct ScaIconNode *
UNDOTAG_IconDirLock	equ	UNDOTAG_TagBase+3	; BPTR
UNDOTAG_CopySrcDirLock	equ	UNDOTAG_TagBase+4	; BPTR
UNDOTAG_CopyDestDirLock	equ	UNDOTAG_TagBase+5	; BPTR
UNDOTAG_CopySrcName	equ	UNDOTAG_TagBase+6	; CONST_STRPTR
UNDOTAG_CopyDestName	equ	UNDOTAG_TagBase+7	; CONST_STRPTR
UNDOTAG_IconPosX	equ	UNDOTAG_TagBase+8	; LONG
UNDOTAG_IconPosY	equ	UNDOTAG_TagBase+9	; LONG
UNDOTAG_OldIconPosX	equ	UNDOTAG_TagBase+10	; LONG
UNDOTAG_OldIconPosY	equ	UNDOTAG_TagBase+11	; LONG
UNDOTag_WindowTask	equ	UNDOTAG_TagBase+12	; struct internalScaWindowTask *
UNDOTag_CleanupMode	equ	UNDOTAG_TagBase+13	; ULONG
UNDOTAG_UndoHook	equ	UNDOTAG_TagBase+14	; struct Hook *
UNDOTAG_RedoHook	equ	UNDOTAG_TagBase+15	; struct Hook *
UNDOTAG_DisposeHook	equ	UNDOTAG_TagBase+16	; struct Hook *

:enum ScalosUndoCleanupMode
CLEANUP_Default		equ	0
CLEANUP_ByName		equ	1
CLEANUP_ByDate		equ	2
CLEANUP_BySize		equ	3
CLEANUP_ByType		equ	4

;---------------------------------------------------------------------------

	ENDC	; SCA_UNDO_I
