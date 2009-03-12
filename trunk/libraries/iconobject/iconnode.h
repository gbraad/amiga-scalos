/*
 * Scalos/iconnode.h
 * iconnode.h
 *
 * $VER: iconnode.h 1.0 (31/01/2000)
 * Richard Drummond
 *
 * Module to handle nodes in library's internal
 * list of iconobject datatypes
 *
 */

#ifndef ICONNODE_H
#define ICONNODE_H

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef  EXEC_NODES_H
#include <exec/nodes.h>
#endif

#ifndef  INTUITION_CLASSES_H
#include <intuition/classes.h>
#endif

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif

/*
 * iconobject.library maintains a list of all iconobject datatypes.
 * Each member has the following structure.
 */

struct IconNode
{
    struct MinNode Node;
    UBYTE          in_pad;
    BYTE           in_Priority;     /* Priority, for sorting */
    STRPTR      in_Name;         /* Full path name of this datatype */
    Class 	*in_Class;        /* Pointer to the datatype's BOOPSI class */
    struct Library *in_LibBase;      /* Pointer to the datatype's library */
#ifdef __amigaos4__
	struct DTClassIFace *in_IFace;     /* Pointer to the datatype's interface */
#endif
    ULONG          in_ID;           /* This datatype's ID */
    STRPTR         in_Suffix;       /* Filename suffix for icon file's of this type */
    WORD           in_SuffixLen;    /* Length of the above */
};

#endif

