#ifndef _PPCINLINE_GUIGFX_H
#define _PPCINLINE_GUIGFX_H

#ifndef CLIB_GUIGFX_PROTOS_H
#define CLIB_GUIGFX_PROTOS_H
#endif

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef GUIGFX_BASE_NAME
#define GUIGFX_BASE_NAME GuiGFXBase
#endif

#define MakePictureA(array, width, height, tags) \
	LP4(0x1e, APTR, MakePictureA, APTR, array, a0, ULONG, width, d0, ULONG, height, d1, struct TagItem *, tags, a1, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define MakePicture(array, width, height, tags...) \
	({ULONG _tags[] = {tags}; MakePictureA((array), (width), (height), (struct TagItem *) _tags);})
#endif

#define LoadPictureA(filename, tags) \
	LP2(0x24, APTR, LoadPictureA, STRPTR, filename, a0, struct TagItem *, tags, a1, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define LoadPicture(filename, tags...) \
	({ULONG _tags[] = {tags}; LoadPictureA((filename), (struct TagItem *) _tags);})
#endif

#define ReadPictureA(rp, colormap, x, y, width, height, tags) \
	LP7(0x2a, APTR, ReadPictureA, struct RastPort *, rp, a0, struct ColorMap *, colormap, a1, ULONG, x, d0, ULONG, y, d1, ULONG, width, d2, ULONG, height, d3, struct TagItem *, tags, a2, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define ReadPicture(rp, colormap, x, y, width, height, tags...) \
	({ULONG _tags[] = {tags}; ReadPictureA((rp), (colormap), (x), (y), (width), (height), (struct TagItem *) _tags);})
#endif

#define ClonePictureA(pic, tags) \
	LP2(0x30, APTR, ClonePictureA, APTR, pic, a0, struct TagItem *, tags, a1, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define ClonePicture(pic, tags...) \
	({ULONG _tags[] = {tags}; ClonePictureA((pic), (struct TagItem *) _tags);})
#endif

#define DeletePicture(pic) \
	LP1NR(0x36, DeletePicture, APTR, pic, a0, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define AddPictureA(psm, pic, tags) \
	LP3(0x42, APTR, AddPictureA, APTR, psm, a0, APTR, pic, a1, struct TagItem *, tags, a2, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define AddPicture(psm, pic, tags...) \
	({ULONG _tags[] = {tags}; AddPictureA((psm), (pic), (struct TagItem *) _tags);})
#endif

#define AddPaletteA(psm, palette, tags) \
	LP3(0x48, APTR, AddPaletteA, APTR, psm, a0, APTR, palette, a1, struct TagItem *, tags, a2, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define AddPalette(psm, palette, tags...) \
	({ULONG _tags[] = {tags}; AddPaletteA((psm), (palette), (struct TagItem *) _tags);})
#endif

#define AddPixelArrayA(psm, array, width, height, tags) \
	LP5(0x4e, APTR, AddPixelArrayA, APTR, psm, a0, APTR, array, a1, ULONG, width, d0, ULONG, height, d1, struct TagItem *, tags, a2, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define AddPixelArray(psm, array, width, height, tags...) \
	({ULONG _tags[] = {tags}; AddPixelArrayA((psm), (array), (width), (height), (struct TagItem *) _tags);})
#endif

#define RemColorHandle(colorhandle) \
	LP1NR(0x54, RemColorHandle, APTR, colorhandle, a0, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CreatePenShareMapA(tags) \
	LP1(0x5a, APTR, CreatePenShareMapA, struct TagItem *, tags, a0, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define CreatePenShareMap(tags...) \
	({ULONG _tags[] = {tags}; CreatePenShareMapA((struct TagItem *) _tags);})
#endif

#define DeletePenShareMap(psm) \
	LP1NR(0x60, DeletePenShareMap, APTR, psm, a0, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ObtainDrawHandleA(psm, rp, cm, tags) \
	LP4(0x66, APTR, ObtainDrawHandleA, APTR, psm, a0, struct RastPort *, rp, a1, struct ColorMap *, cm, a2, struct TagItem *, tags, a3, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define ObtainDrawHandle(psm, rp, cm, tags...) \
	({ULONG _tags[] = {tags}; ObtainDrawHandleA((psm), (rp), (cm), (struct TagItem *) _tags);})
#endif

#define ReleaseDrawHandle(drawhandle) \
	LP1NR(0x6c, ReleaseDrawHandle, APTR, drawhandle, a0, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DrawPictureA(drawhandle, pic, x, y, tags) \
	LP5(0x72, BOOL, DrawPictureA, APTR, drawhandle, a0, APTR, pic, a1, ULONG, x, d0, ULONG, y, d1, struct TagItem *, tags, a2, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define DrawPicture(drawhandle, pic, x, y, tags...) \
	({ULONG _tags[] = {tags}; DrawPictureA((drawhandle), (pic), (x), (y), (struct TagItem *) _tags);})
#endif

#define MapPaletteA(drawhandle, palette, pentab, tags) \
	LP4(0x78, BOOL, MapPaletteA, APTR, drawhandle, a0, APTR, palette, a1, UBYTE *, pentab, a2, struct TagItem *, tags, a3, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define MapPalette(drawhandle, palette, pentab, tags...) \
	({ULONG _tags[] = {tags}; MapPaletteA((drawhandle), (palette), (pentab), (struct TagItem *) _tags);})
#endif

#define MapPenA(drawhandle, rgb, tags) \
	LP3(0x7e, LONG, MapPenA, APTR, drawhandle, a0, ULONG, rgb, a1, struct TagItem *, tags, a2, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define MapPen(drawhandle, rgb, tags...) \
	({ULONG _tags[] = {tags}; MapPenA((drawhandle), (rgb), (struct TagItem *) _tags);})
#endif

#define CreatePictureBitMapA(drawhandle, pic, tags) \
	LP3(0x84, struct BitMap *, CreatePictureBitMapA, APTR, drawhandle, a0, APTR, pic, a1, struct TagItem *, tags, a2, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define CreatePictureBitMap(drawhandle, pic, tags...) \
	({ULONG _tags[] = {tags}; CreatePictureBitMapA((drawhandle), (pic), (struct TagItem *) _tags);})
#endif

#define DoPictureMethodA(pic, method, arguments) \
	LP3(0x8a, ULONG, DoPictureMethodA, APTR, pic, a0, ULONG, method, d0, ULONG *, arguments, a1, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define DoPictureMethod(pic, method, tags...) \
	({ULONG _tags[] = {tags}; DoPictureMethodA((pic), (method), (ULONG *) _tags);})
#endif

#define GetPictureAttrsA(pic, tags) \
	LP2(0x90, ULONG, GetPictureAttrsA, APTR, pic, a0, struct TagItem *, tags, a1, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define GetPictureAttrs(pic, tags...) \
	({ULONG _tags[] = {tags}; GetPictureAttrsA((pic), (struct TagItem *) _tags);})
#endif

#define LockPictureA(pic, mode, args) \
	LP3(0x96, ULONG, LockPictureA, APTR, pic, a0, ULONG, mode, d0, ULONG *, args, a1, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define LockPicture(pic, mode, tags...) \
	({ULONG _tags[] = {tags}; LockPictureA((pic), (mode), (ULONG *) _tags);})
#endif

#define UnLockPicture(pic, mode) \
	LP2NR(0x9c, UnLockPicture, APTR, pic, a0, ULONG, mode, d0, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define IsPictureA(filename, tags) \
	LP2(0xa2, BOOL, IsPictureA, STRPTR, filename, a0, struct TagItem *, tags, a1, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define IsPicture(filename, tags...) \
	({ULONG _tags[] = {tags}; IsPictureA((filename), (struct TagItem *) _tags);})
#endif

#define CreateDirectDrawHandleA(drawhandle, sw, sh, dw, dh, tags) \
	LP6(0xa8, APTR, CreateDirectDrawHandleA, APTR, drawhandle, a0, ULONG, sw, d0, ULONG, sh, d1, ULONG, dw, d2, ULONG, dh, d3, struct TagItem *, tags, a1, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define CreateDirectDrawHandle(drawhandle, sw, sh, dw, dh, tags...) \
	({ULONG _tags[] = {tags}; CreateDirectDrawHandleA((drawhandle), (sw), (sh), (dw), (dh), (struct TagItem *) _tags);})
#endif

#define DeleteDirectDrawHandle(ddh) \
	LP1NR(0xae, DeleteDirectDrawHandle, APTR, ddh, a0, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DirectDrawTrueColorA(ddh, array, x, y, tags) \
	LP5(0xb4, BOOL, DirectDrawTrueColorA, APTR, ddh, a0, ULONG *, array, a1, ULONG, x, d0, ULONG, y, d1, struct TagItem *, tags, a2, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define DirectDrawTrueColor(ddh, array, x, y, tags...) \
	({ULONG _tags[] = {tags}; DirectDrawTrueColorA((ddh), (array), (x), (y), (struct TagItem *) _tags);})
#endif

#define CreatePictureMaskA(pic, mask, maskwidth, tags) \
	LP4(0xba, BOOL, CreatePictureMaskA, APTR, pic, a0, UBYTE *, mask, a1, ULONG, maskwidth, d0, struct TagItem *, tags, a2, \
	, GUIGFX_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define CreatePictureMask(pic, mask, maskwidth, tags...) \
	({ULONG _tags[] = {tags}; CreatePictureMaskA((pic), (mask), (maskwidth), (struct TagItem *) _tags);})
#endif

#endif /*  _PPCINLINE_GUIGFX_H  */
