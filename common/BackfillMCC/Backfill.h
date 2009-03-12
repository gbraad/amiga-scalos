// Backfill.h
// $Date$
// $Revision$


#ifndef SCALOS_BACKFILL_H
#define SCALOS_BACKFILL_H

//----------------------------------------------------------------------------

#define BACKFILLBASE		0x80430100


#define BFA_BitmapObject	(BACKFILLBASE+1)	// [IS.] Object *

//----------------------------------------------------------------------------

struct SetBackFillMsg
	{
	ULONG sbf_MethodID;
	CONST_STRPTR sbf_FileName;
	ULONG sbf_PattFlags;
	ULONG sbf_DefsFlags;
	};

//----------------------------------------------------------------------------

extern struct MUI_CustomClass *BackfillClass;
extern struct MUI_CustomClass *BitMapPicClass;

//----------------------------------------------------------------------------

struct MUI_CustomClass *InitBackfillClass(void);
void CleanupBackfillClass(void);

//----------------------------------------------------------------------------

#endif /* SCALOS_BACKFILL_H */

