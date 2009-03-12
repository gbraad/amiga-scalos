#ifndef PRAGMAS_DTLIB_PRAGMAS_H
#define PRAGMAS_DTLIB_PRAGMAS_H

/*
**	$Id: dtlib_pragmas.h 1.2 2006/04/26 19:03:48 juergen Exp $
**
**	Direct ROM interface (pragma) definitions.
**
**	Copyright � 2001 Amiga, Inc.
**	    All Rights Reserved
*/

#if defined(LATTICE) || defined(__SASC) || defined(_DCC)
#ifndef __CLIB_PRAGMA_LIBCALL
#define __CLIB_PRAGMA_LIBCALL
#endif /* __CLIB_PRAGMA_LIBCALL */
#else /* __MAXON__, __STORM__ or AZTEC_C */
#ifndef __CLIB_PRAGMA_AMICALL
#define __CLIB_PRAGMA_AMICALL
#endif /* __CLIB_PRAGMA_AMICALL */
#endif /* */

#if defined(__SASC) || defined(__STORM__)
#ifndef __CLIB_PRAGMA_TAGCALL
#define __CLIB_PRAGMA_TAGCALL
#endif /* __CLIB_PRAGMA_TAGCALL */
#endif /* __MAXON__, __STORM__ or AZTEC_C */

#ifndef CLIB_DTLIB_PROTOS_H
#include <clib/dtlib_protos.h>
#endif /* CLIB_DTLIB_PROTOS_H */

#ifdef __CLIB_PRAGMA_LIBCALL
 #pragma libcall DtLibBase ObtainInfoEngine 1e 801
#endif /* __CLIB_PRAGMA_LIBCALL */

#endif /* PRAGMAS_DTLIB_PRAGMAS_H */
