// Delete.module.h
// $Date$
// $Revision$

//     ___       ___
//   _/  /_______\  \_     ___ ___ __ _                       _ __ ___ ___
//__//  / _______ \  \\___/                                               \___
//_/ | '  \__ __/  ` | \_/        � Copyright 1999, Christopher Page       \__
// \ | |    | |__  | | / \   Released as Free Software under the GNU GPL   /.
//  >| .    |  _/  . |<   >--- --- -- -                       - -- --- ---<.
// / \  \   | |   /  / \ /   This file is part of the ScalosDelete code    \.
// \  \  \_/   \_/  /  / \  and it is released under the GNU GPL. Please   /.
//  \  \           /  /   \   read the "COPYING" file which should have   /.
// //\  \_________/  /\\ //\    been included in the distribution arc.   /.
//- --\   _______   /-- - --\      for full details of the license      /-----
//-----\_/       \_/---------\   ___________________________________   /------
//                            \_/                                   \_/.
//
// Description:
//
// Includes and shared stuff for the various source files for ScalosDelete.
//
//  Yeah, I know I overdo the #include stuff, so kill me...


// MUI
#include <libraries/mui.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/Lamp_mcc.h>

// System
#include <exec/types.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/locale.h>
#include <utility/utility.h>
#include <utility/tagitem.h>
#include <utility/hooks.h>
#include <scalos/scalos.h>
#include <scalos/undo.h>

// Prototypes
#include <clib/alib_protos.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/icon.h>
#include <proto/asl.h>
#include <proto/locale.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/muimaster.h>
#include <proto/scalos.h>

// ANSI C
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <defs.h>

// Revision stuff
#include "Delete.module_rev.h"


struct Delete_LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};

#define	Delete_NUMBERS
#include STR(SCALOSLOCALE)


// Couple of macros to make the code more readable.

// Need MAKE_ID for the windows, so best to be sure...
#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d)     ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif

#ifndef __AROS__
#define	BNULL		((BPTR) NULL)
#endif

// Some macros to make the MUI code a bit easier to read.
#define KeyCheckMarkHelpID(selected, control, cyclechain, ident, help)\
    ImageObject,\
        ImageButtonFrame,\
        MUIA_InputMode        , MUIV_InputMode_Toggle,\
        MUIA_Image_Spec       , MUII_CheckMark,\
        MUIA_Image_FreeVert   , TRUE,\
        MUIA_Selected         , selected,\
        MUIA_Background       , MUII_ButtonBack,\
        MUIA_ShowSelState     , FALSE,\
        MUIA_ControlChar      , control,\
        MUIA_CycleChain       , cyclechain,\
        MUIA_ShortHelp        , help,\
        MUIA_ExportID         , ident,\
    End

#define KeyButtonChain(name, key, cyclechain)\
    TextObject,\
        ButtonFrame,\
        MUIA_Text_Contents, name,\
        MUIA_Text_PreParse, "\33c",\
        MUIA_Text_HiChar  , key,\
        MUIA_ControlChar  , key,\
        MUIA_InputMode    , MUIV_InputMode_RelVerify,\
        MUIA_Background   , MUII_ButtonBack,\
        MUIA_CycleChain   , cyclechain,\
    End


// Events generated by the interface.
#define DELETE_IT 1
#define DETAIL_IT 2
#define STOP_IT   3

#define PREFSSAVE 10
#define PREFSUSE  11
#define PREFSLOAD 12

// Results generated by the delete functions.
#define RESULT_OK     0
#define RESULT_FAIL   1
#define RESULT_CANCEL 2
#define RESULT_HALT   3

#define	MAX_FILENAME_LEN	512

// ���������������������������� ---------------- ���������������������������
// ��������������������������� In Delete.module.c ��������������������������
// ���������������������������� ---------------- ���������������������������

// Globals...
extern struct Library *AslBase;
extern T_UTILITYBASE UtilityBase  ;
extern struct ScalosBase *ScalosBase   ;
extern T_LOCALEBASE LocaleBase   ;
extern struct Library *MUIMasterBase;
extern struct Catalog *WordsCat     ;

extern        Object  *MUI_App      ;
extern        Object  *WI_Delete    ;
extern        Object  *LV_Files     ;
extern        Object  *BT_Details   ;
extern        Object  *TX_ReadOut   ;
extern        Object  *BT_Delete    ;
extern        Object  *BT_Cancel    ;
extern        Object  *GP_Progress  ;
extern        Object  *TX_Progress  ;
extern        Object  *GA_Progress  ;
extern        Object  *BT_STOP      ;
extern        Object  *ST_TrashDir  ;
extern        Object  *CM_TrashCan  ;
extern        Object  *CM_DirConf   ;
extern        Object  *CM_FileConf  ;

CONST_STRPTR GetLocString(ULONG MsgId);
STRPTR safe_strcat(STRPTR dest, CONST_STRPTR src, size_t DestLen);

#if !defined(__SASC) && !defined(__MORPHOS__) && !defined(__amigaos4__)
size_t stccpy(char *dest, const char *src, size_t MaxLen);
#endif /* __SASC */

#if defined(__SASC)
int snprintf(char *, size_t, const char *, /*args*/ ...);
#endif /* __SASC */


// ������������������������� ---------------------- ������������������������
// ������������������������ In Delete.module_hooks.c �����������������������
// ������������������������� ---------------------- ������������������������

extern STRPTR FileBuffer;
extern STRPTR PathBuffer;
extern STRPTR DirBuffer ;
extern char TextBuffer[80 + 1];
extern struct Hook   ConList   ;
extern struct Hook   DesList   ;
extern struct Hook   DisList   ;
extern struct Hook   CmpList   ;

extern BOOL ListFiles (struct WBStartup *);
extern BOOL GetDetails(struct WBStartup *);
extern void KillFiles (struct WBStartup *);


// Make debugging a bit easier to activate/ deactivate....
extern void kprintf(UBYTE *fmt, ...);
extern void KPrintF(UBYTE *fmt, ...);

#ifdef __AROS__
#define KPrintF kprintf
#endif

#define d1(x)	;
#define d2(x)	x;
