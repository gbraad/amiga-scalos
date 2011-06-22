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

#define	CATCOMP_ARRAY
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
	if (LocaleBase != NULL && ScalosCatalog != NULL)
	{
		return GetCatalogStr(ScalosCatalog, StringID, CatCompArray[StringID].cca_Str);
	} 
	else 
	{
		return CatCompArray[StringID].cca_Str;
	}
}
