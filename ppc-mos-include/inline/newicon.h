#ifndef _PPCINLINE_NEWICON_H
#define _PPCINLINE_NEWICON_H

#ifndef CLIB_NEWICON_PROTOS_H
#define CLIB_NEWICON_PROTOS_H
#endif

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  LIBRARIES_NEWICON_H
#include <libraries/newicon.h>
#endif

#ifndef NEWICON_BASE_NAME
#define NEWICON_BASE_NAME NewIconBase
#endif

#define GetNewDiskObject(name) \
	LP1(0x1e, struct NewDiskObject *, GetNewDiskObject, UBYTE *, name, a0, \
	, NEWICON_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PutNewDiskObject(name, newdiskobj) \
	LP2(0x24, BOOL, PutNewDiskObject, UBYTE *, name, a0, struct NewDiskObject *, newdiskobj, a1, \
	, NEWICON_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define FreeNewDiskObject(newdiskobj) \
	LP1NR(0x2a, FreeNewDiskObject, struct NewDiskObject *, newdiskobj, a0, \
	, NEWICON_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define newiconPrivate1(diskobj) \
	LP1(0x30, BOOL, newiconPrivate1, struct NewDiskObject *, diskobj, a0, \
	, NEWICON_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define newiconPrivate2(diskobj) \
	LP1(0x36, UBYTE **, newiconPrivate2, struct NewDiskObject *, diskobj, a0, \
	, NEWICON_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define newiconPrivate3(diskobj) \
	LP1NR(0x3c, newiconPrivate3, struct NewDiskObject *, diskobj, a0, \
	, NEWICON_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RemapChunkyImage(chunkyimage, screen) \
	LP2(0x42, struct Image *, RemapChunkyImage, struct ChunkyImage *, chunkyimage, a0, struct Screen *, screen, a1, \
	, NEWICON_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define FreeRemappedImage(image, screen) \
	LP2NR(0x48, FreeRemappedImage, struct Image *, image, a0, struct Screen *, screen, a1, \
	, NEWICON_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define GetDefNewDiskObject(def_type) \
	LP1(0x4e, struct NewDiskObject *, GetDefNewDiskObject, LONG, def_type, d0, \
	, NEWICON_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /*  _PPCINLINE_NEWICON_H  */
