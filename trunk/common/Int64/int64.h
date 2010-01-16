// int64.h
// $Date$
// $Revision$

#ifndef	INT64_H
#define	INT64_H

#include <exec/types.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/exall.h>
#include <libraries/locale.h>
#include <scalos/int64types.h>

// ------------------------------------------------------------------------

// defined in int64.c
///
ULONG64 MakeU64( ULONG n );
SLONG64 MakeS64( LONG n );
ULONG64 Incr64(ULONG64 x, ULONG n );
ULONG64 Decr64(ULONG64 x, ULONG n );
ULONG64 Add64(ULONG64 x, ULONG64 y );
ULONG64 Sub64(ULONG64 x, ULONG64 y );
void ShiftR64(ULONG64 *x, UBYTE b);
void ShiftL64(ULONG64 *x, UBYTE b);
ULONG64 Mul64(ULONG64 x, ULONG64 y, BOOL *overflow);
ULONG64 Div64(ULONG64 x, ULONG64 y, ULONG64 *rmdrP);
int Cmp64(ULONG64 x, ULONG64 y);
void Convert64(const struct Locale *locale, ULONG64 Number, STRPTR Buffer, size_t MaxLen);
///
/* ------------------------------------------------- */

// defined in Dos64.c
///
LONG ScalosExamine64(BPTR lock, struct FileInfoBlock *fib);
LONG ScalosExNext64(BPTR lock, struct FileInfoBlock *fib);
ULONG64 ScalosSeek64(BPTR file, ULONG64 position, LONG mode);
ULONG64 ScalosFibSize64(const struct FileInfoBlock *fib);
ULONG64 ScalosFibNumBlocks64(const struct FileInfoBlock *fib);
void ScalosSetFibSize64(struct FileInfoBlock *fib, ULONG64 Size);
ULONG64 ScalosExAllSize64(const struct ExAllData *ead, ULONG edType);
void ScalosSetExAllSize64(struct ExAllData *ead, ULONG edType, ULONG64 Size);
void ScalosDosPacketExNext64(struct DosPacket *dp, BPTR lock, struct FileInfoBlock *fib);
BOOL ScalosSupports64(struct MsgPort *fileSysPort, BPTR dirLock, struct FileInfoBlock *fib);
///
/* ------------------------------------------------- */

#endif	// INT64_H
