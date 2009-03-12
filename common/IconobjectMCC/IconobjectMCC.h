// IconobjectMCC.h
// $Date$
// $Revision$


#ifndef ICONOBJECT_MCC_H
#define	ICONOBJECT_MCC_H

/* ------------------------------------------------------------------------- */

struct MUI_CustomClass *InitIconobjectClass(void);
void CleanupIconobjectClass(struct MUI_CustomClass *mcc);

/* ------------------------------------------------------------------------- */

#define IconobjectMCCObject NewObject(IconobjectClass->mcc_Class, 0

/* ------------------------------------------------------------------------- */

#define MUIA_Iconobj_Object			0x80429876 	/* I.. Object * */

/* ------------------------------------------------------------------------- */

#endif /* ICONOBJECT_MCC_H */
