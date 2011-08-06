#ifndef INLINE4_TTENGINE_H
#define INLINE4_TTENGINE_H

/*
** This file was auto generated by idltool 51.3.
**
** It provides compatibility to OS3 style library
** calls by substituting functions.
**
** Do not edit manually.
*/ 

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef EXEC_EXEC_H
#include <exec/exec.h>
#endif
#ifndef EXEC_INTERFACES_H
#include <exec/interfaces.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef LIBRARIES_TTENGINE_H
#include <libraries/ttengine.h>
#endif
#ifndef GRAPHICS_TEXT_H
#include <graphics/text.h>
#endif

/* Inline macros for Interface "main" */
#define TT_OpenFontA(taglist) ITTEngine->TT_OpenFontA(taglist) 
#if !defined(__cplusplus) && (__GNUC__ >= 3    \
    || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95) \
    || (__STDC_VERSION__ && __STDC_VERSION__ >= 199901L))
#define TT_OpenFont(...) ITTEngine->TT_OpenFont(__VA_ARGS__) 
#endif
#define TT_SetFont(rp, font) ITTEngine->TT_SetFont(rp, font) 
#define TT_CloseFont(font) ITTEngine->TT_CloseFont(font) 
#define TT_Text(rp, string, count) ITTEngine->TT_Text(rp, string, count) 
#define TT_SetAttrsA(rp, taglist) ITTEngine->TT_SetAttrsA(rp, taglist) 
#if !defined(__cplusplus) && (__GNUC__ >= 3    \
    || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95) \
    || (__STDC_VERSION__ && __STDC_VERSION__ >= 199901L))
#define TT_SetAttrs(...) ITTEngine->TT_SetAttrs(__VA_ARGS__) 
#endif
#define TT_GetAttrsA(rp, taglist) ITTEngine->TT_GetAttrsA(rp, taglist) 
#if !defined(__cplusplus) && (__GNUC__ >= 3    \
    || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95) \
    || (__STDC_VERSION__ && __STDC_VERSION__ >= 199901L))
#define TT_GetAttrs(...) ITTEngine->TT_GetAttrs(__VA_ARGS__) 
#endif
#define TT_TextLength(rp, string, count) ITTEngine->TT_TextLength(rp, string, count) 
#define TT_TextExtent(rp, string, count, te) ITTEngine->TT_TextExtent(rp, string, count, te) 
#define TT_TextFit(rp, string, count, te, tec, dir, cwidth, cheight) ITTEngine->TT_TextFit(rp, string, count, te, tec, dir, cwidth, cheight) 
#define TT_GetPixmapA(font, string, count, taglist) ITTEngine->TT_GetPixmapA(font, string, count, taglist) 
#if !defined(__cplusplus) && (__GNUC__ >= 3    \
    || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95) \
    || (__STDC_VERSION__ && __STDC_VERSION__ >= 199901L))
#define TT_GetPixmap(font, string, ...) ITTEngine->TT_GetPixmap(font, string, __VA_ARGS__) 
#endif
#define TT_FreePixmap(pixmap) ITTEngine->TT_FreePixmap(pixmap) 
#define TT_DoneRastPort(rp) ITTEngine->TT_DoneRastPort(rp) 
#define TT_AllocRequest() ITTEngine->TT_AllocRequest() 
#define TT_RequestA(request, taglist) ITTEngine->TT_RequestA(request, taglist) 
#if !defined(__cplusplus) && (__GNUC__ >= 3    \
    || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95) \
    || (__STDC_VERSION__ && __STDC_VERSION__ >= 199901L))
#define TT_Request(...) ITTEngine->TT_Request(__VA_ARGS__) 
#endif
#define TT_FreeRequest(request) ITTEngine->TT_FreeRequest(request) 
#define TT_ObtainFamilyListA(taglist) ITTEngine->TT_ObtainFamilyListA(taglist) 
#if !defined(__cplusplus) && (__GNUC__ >= 3    \
    || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95) \
    || (__STDC_VERSION__ && __STDC_VERSION__ >= 199901L))
#define TT_ObtainFamilyList(...) ITTEngine->TT_ObtainFamilyList(__VA_ARGS__) 
#endif
#define TT_FreeFamilyList(list) ITTEngine->TT_FreeFamilyList(list) 

#endif /* INLINE4_TTENGINE_H */