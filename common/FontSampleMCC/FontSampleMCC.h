// FontSampleMCC.h
// $Date$
// $Revision$


#ifndef FONTSAMPLE_MCC_H
#define	FONTSAMPLE_MCC_H

/* ------------------------------------------------------------------------- */

#include <defs.h>

/* ------------------------------------------------------------------------- */

extern struct MUI_CustomClass *FontSampleClass;

struct MUI_CustomClass *InitFontSampleClass(void);
void CleanupFontSampleClass(struct MUI_CustomClass *mcc);
Object *FontSampleMCCObject(Tag tags, ...);

/* ------------------------------------------------------------------------- */

#define	MAX_TTFONTDESC	256

/* ------------------------------------------------------------------------- */

#define	MUIA_FontSample_TTFontDesc		0x82457651	/* CONST_STRPTR */
#define	MUIA_FontSample_DemoString		0x82457652	/* CONST_STRPTR */
#define	MUIA_FontSample_StdFontDesc		0x82457653	/* CONST_STRPTR */
#define	MUIA_FontSample_Antialias		0x82457654	/* ULONG */
#define	MUIA_FontSample_Gamma			0x82457655	/* ULONG */

//--------------------------------------------------------------------

// from debug.lib
extern int kprintf(const char *fmt, ...);

//--------------------------------------------------------------------

#define	d1(x)		;
#define	d2(x)		x;

/* ------------------------------------------------------------------------- */

#endif /* FONTSAMPLE_MCC_H */
