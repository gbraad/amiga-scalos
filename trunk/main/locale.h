// locale.h
// $Date$
// $Revision$

#ifndef LOCALE_H
#define LOCALE_H

struct Scalos_LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};

CONST_STRPTR      GetLocString(LONG lStrnum);

#define	Scalos_NUMBERS
#include STR(SCALOSLOCALE)


#endif

