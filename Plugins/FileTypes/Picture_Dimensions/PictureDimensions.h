// PictureDimensions.h
// $Date$
// $Revision$

#ifndef PICTUREDIMENSIONS_H
#define PICTUREDIMENSIONS_H

int PictureDimensionsInit(struct PictureDimensionsBase *PictureDimensionsBase);
int PictureDimensionsOpen(struct PictureDimensionsBase *PictureDimensionsBase);
void PictureDimensionsCleanup(struct PictureDimensionsBase *PictureDimensionsBase);
LIBFUNC_P3_PROTO(STRPTR, LIBToolTipInfoString,
	A0, struct ScaToolTipInfoHookData *, ttshd,
	A1, CONST_STRPTR, args,
	A6, struct PictureDimensionsBase *, PictureDimensionsBase);

//---------------------------------------------------------------

// Debugging...
#define	d(x)	;
#define	d1(x)	;
#define	d2(x)	x;

// aus debug.lib
extern int kprintf(const char *fmt, ...);
extern int KPrintF(const char *fmt, ...);

#endif /* PICTUREDIMENSIONS_H */

