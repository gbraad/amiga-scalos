#ifndef  CLIB_DTLIB_PROTOS_H
#define  CLIB_DTLIB_PROTOS_H

/*
**	$Id: dtlib_protos.h 1702 2006-05-04 19:11:43Z juergen $
**
**	C prototypes. For use with 32 bit integers only.
**
**	�1999-2005 The Scalos Team
**	    All Rights Reserved
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  INTUITION_CLASSES_H
#include <intuition/classes.h>
#endif
Class *ObtainInfoEngine( struct Library *libBase );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* CLIB_DTLIB_PROTOS_H */
