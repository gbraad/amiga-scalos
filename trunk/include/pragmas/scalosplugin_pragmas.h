#ifndef PRAGMAS_SCALOSPLUGIN_PRAGMAS_H
#define PRAGMAS_SCALOSPLUGIN_PRAGMAS_H

/*
**	$Id: scalosplugin_pragmas.h 1702 2006-05-04 19:11:43Z juergen $
**
**	Direct ROM interface (pragma) definitions.
**
**	�1999-2002 The Scalos Team
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

#ifndef CLIB_SCALOSPLUGIN_PROTOS_H
#include <clib/scalosplugin_protos.h>
#endif /* CLIB_SCALOSPLUGIN_PROTOS_H */

#ifdef __CLIB_PRAGMA_LIBCALL
 #pragma libcall ScalosPluginBase SCAGetClassInfo 1e 00
#endif /* __CLIB_PRAGMA_LIBCALL */

#endif /* PRAGMAS_SCALOSPLUGIN_PRAGMAS_H */
