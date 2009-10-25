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
UNDO_Rename		equ	7
UNDO_Relabel		equ	8

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
UNDOTAG_SaveIcon,	equ	UNDOTAG_TagBase+17	; ULONG
UNDOTAG_CustomAddHook	equ	UNDOTAG_TagBase+18	; struct Hook *

;enum ScalosUndoCleanupMode
CLEANUP_Default		equ	0
CLEANUP_ByName		equ	1
CLEANUP_ByDate		equ	2
CLEANUP_BySize		equ	3
CLEANUP_ByType		equ	4

  STRUCTURE	UndoCleanupIconEntry,0
	APTR	ucin_IconNode		;const struct ScaIconNode *ucin_IconNode;
	LONG 	ucin_Left		;original x position of icon
	LONG 	ucin_Top		;original y position of icon
	LABEL	ucin_SIZEOF

  STRUCTURE	UndoCopyMoveEventData,0
	APTR 	ucmed_srcDirName	;STRPTR
	APTR 	ucmed_destDirName	;STRPTR
	APTR 	ucmed_srcName		;STRPTR
	APTR 	ucmed_destName		;STRPTR
	LABEL	ucmed_SIZEOF

  STRUCTURE	UndoIconEventData,0
	APTR 	uid_DirName		;STRPTR
	APTR 	uid_IconName		;STRPTR
	LABEL	uid_SIZEOF

  STRUCTURE	UndoCleanupData,0
	ULONG	ucd_CleanupMode		;enum ScalosUndoCleanupMode
	APTR	ucd_Icons		;struct UndoCleanupIconEntry *
	ULONG 	ucd_IconCount		;number of entries in ucd_Icons
	APTR	ucd_WindowTask		;struct internalScaWindowTask *
	LABEL	ucd_SIZEOF

  STRUCTURE 	UndoSnaphotIconData,0
	APTR 	usid_DirName		;STRPTR
	APTR	usid_IconName		;STRPTR
	APTR	usid_IconObj		;Object * - Iconobject BEFORE snapshot/unsnaphot
	ULONG 	usid_SaveIcon
	LABEL	usid_SIZEOF

; uev_Data_SIZEOF is the largest of all sizes of the sub-structures
; UndoCopyMoveEventData, UndoIconEventData, UndoCleanupData, and UndoSnaphotIconData

uev_Data_SIZEOF	set     ucmed_SIZEOF
	IFGT    usid_SIZEOF-ucmed_SIZEOF
uev_Data_SIZEOF	set     usid_SIZEOF
	ENDIF
	IFGT    ucd_SIZEOF-ucmed_SIZEOF
uev_Data_SIZEOF	set     ucd_SIZEOF
	ENDIF
	IFGT    ucin_SIZEOF-ucmed_SIZEOF
uev_Data_SIZEOF	set     ucin_SIZEOF
	ENDIF
	IFGT    uid_SIZEOF-ucmed_SIZEOF
uev_Data_SIZEOF	set     uid_SIZEOF
	ENDIF


  STRUCTURE	UndoEvent,0
	STRUCT	uev_Node,LN_SIZE
	ALIGNLONG	
	ULONG	uev_Type		;enum ScalosUndoType

	APTR	uev_UndoHook		;struct Hook *
	APTR	uev_RedoHook		;struct Hook *
	APTR	uev_DisposeHook		;struct Hook *
	APTR	uev_CustomAddHook	;struct Hook *

	LONG uev_OldPosX
	LONG uev_OldPosY
	LONG uev_NewPosX
	LONG uev_NewPosY

	STRUCT  uev_Data,uev_Data_SIZEOF

	LABEL	uev_SIZEOF

;---------------------------------------------------------------------------

	ENDC	; SCA_UNDO_I
