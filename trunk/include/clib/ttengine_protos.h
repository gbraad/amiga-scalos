/* Automatically generated header! Do not edit! */

#ifndef CLIB_TTENGINE_PROTOS_H
#define CLIB_TTENGINE_PROTOS_H

/*
**	$VER: ttengine_protos.h 7.0 (13.01.2005)
**
**	C prototypes. For use with 32 bit integers only.
**
**	Copyright © 2001 Amiga, Inc.
**	    All Rights Reserved
*/

#include <exec/types.h>
#include <utility/tagitem.h>
#include <libraries/ttengine.h>
#include <graphics/text.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*--- functions in V4 or higher ---*/
APTR TT_OpenFontA(struct TagItem *taglist);
APTR TT_OpenFont(Tag tag1, ...);
BOOL TT_SetFont(struct RastPort *rp, APTR font);
VOID TT_CloseFont(APTR font);
VOID TT_Text(struct RastPort *rp, APTR string, ULONG count);
ULONG TT_SetAttrsA(struct RastPort *rp, struct TagItem *taglist);
ULONG TT_SetAttrs(struct RastPort *rp, Tag tag1, ...);
ULONG TT_GetAttrsA(struct RastPort *rp, struct TagItem *taglist);
ULONG TT_GetAttrs(struct RastPort *rp, Tag tag1, ...);
ULONG TT_TextLength(struct RastPort *rp, APTR string, ULONG count);
VOID TT_TextExtent(struct RastPort *rp, APTR string, WORD count, struct TextExtent *te);
ULONG TT_TextFit(struct RastPort *rp, APTR string, UWORD count, struct TextExtent *te, struct TextExtent *tec, WORD dir, UWORD cwidth, UWORD cheight);

/*--- functions in V5 or higher ---*/
struct TT_Pixmap * TT_GetPixmapA(APTR font, APTR string, ULONG count, struct TagItem *taglist);
struct TT_Pixmap * TT_GetPixmap(APTR font, APTR string, ULONG count, Tag tag1, ...);
VOID TT_FreePixmap(struct TT_Pixmap *pixmap);
VOID TT_DoneRastPort(struct RastPort *rp);

/*--- functions in V6 or higher ---*/
APTR TT_AllocRequest(void);
struct TagItem* TT_RequestA(APTR request, struct TagItem *taglist);
struct TagItem* TT_Request(APTR request, Tag tag1, ...);
VOID TT_FreeRequest(APTR request);

/*--- functions in V7 or higher ---*/
STRPTR * TT_ObtainFamilyListA(struct TagItem *taglist);
STRPTR * TT_ObtainFamilyList(Tag tag1, ...);
VOID TT_FreeFamilyList(STRPTR *list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CLIB_TTENGINE_PROTOS_H */
