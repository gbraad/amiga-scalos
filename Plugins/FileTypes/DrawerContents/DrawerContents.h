// DrawerContents.h
// $Date$
// $Revision$

#ifndef DRAWERCONTENTS_H
#define DRAWERCONTENTS_H

int DrawerContentsInit(struct DrawerContentsBase *DrawerContentsBase);
int DrawerContentsOpen(struct DrawerContentsBase *DrawerContentsBase);
void DrawerContentsCleanup(struct DrawerContentsBase *DrawerContentsBase);
LIBFUNC_P3_PROTO(STRPTR, LIBToolTipInfoString,
	A0, struct ScaToolTipInfoHookData *, ttshd,
	A1, CONST_STRPTR, args,
	A6, struct DrawerContentsBase *, DrawerContentsBase);

//---------------------------------------------------------------

// Debugging...
#define	d(x)	;
#define	d1(x)	;
#define	d2(x)	x;

// aus debug.lib
extern int kprintf(const char *fmt, ...);
extern int KPrintF(const char *fmt, ...);

#endif /* DRAWERCONTENTS_H */

