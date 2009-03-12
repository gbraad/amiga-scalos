// NewDrawer.h
// $Date$
// $Revision$


#ifndef NEWDRAWER_H
#define	NEWDRAWER_H

#define NDP_IconCreation	0x80000001

#define d1(x)		;
#define d2(x)		x;


extern int kprintf(CONST_STRPTR, ...);
extern int KPrintF(CONST_STRPTR, ...);

ULONG mui_getv(APTR, ULONG );

#endif	/* NEWDRAWER_H */
