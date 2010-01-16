// LocaleStrings.c
// 28 Apr 2002 01:34:47

#include <exec/types.h>
#include <intuition/classusr.h>
#include <dos/datetime.h>

#include "debug.h"
#include <proto/scalos.h>
#include <proto/locale.h>

#include <defs.h>
#include <scalos/scalos.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scalos_structures.h"
#include "functions.h"
#include "Variables.h"

#define	CATCOMP_CODE
#define	CATCOMP_BLOCK
#include "locale.h"

//----------------------------------------------------------------------------

// local data structures

//----------------------------------------------------------------------------

// local functions

//----------------------------------------------------------------------------

// local data items

//----------------------------------------------------------------------------

// public data items

//----------------------------------------------------------------------------

CONST_STRPTR GetLocString(LONG StringID)
{
	struct LocaleInfo li;

#ifndef __amigaos4__
	li.li_LocaleBase = LocaleBase;
#else
	li.li_ILocale = ILocale;
#endif

	li.li_Catalog = ScalosCatalog;

	return (CONST_STRPTR) GetString(&li, StringID);
}

