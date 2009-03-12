#ifndef NEWDRAWERSTRINGS_H
#define NEWDRAWERSTRINGS_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifdef CATCOMP_ARRAY
#undef CATCOMP_NUMBERS
#undef CATCOMP_STRINGS
#define CATCOMP_NUMBERS
#define CATCOMP_STRINGS
#endif

#ifdef CATCOMP_BLOCK
#undef CATCOMP_STRINGS
#define CATCOMP_STRINGS
#endif


/****************************************************************************/


#ifdef CATCOMP_NUMBERS

#define MSGID_WINTITLE_NEWDRAWER 256
#define MSGID_LABEL_ENTERNAME 257
#define MSGID_NEWNAME_DEFAULT 258
#define MSGID_LABEL_NEWDRAWER 259
#define MSGID_GADGETTEXT_CREATEICON 260
#define MSGID_BUTTON_OK 261
#define MSGID_BUTTON_CANCEL 262
#define MSGID_TITLE_ERRORREQ 263
#define MSGID_ERRORREQ_BODYTEXT 264
#define MSGID_REQ_OK 265

#endif /* CATCOMP_NUMBERS */


/****************************************************************************/


#ifdef CATCOMP_STRINGS

#define MSGID_WINTITLE_NEWDRAWER_STR "Add a new drawer"
#define MSGID_LABEL_ENTERNAME_STR "Enter Name for a new Drawer in '%s'"
#define MSGID_NEWNAME_DEFAULT_STR "New_Drawer_Name"
#define MSGID_LABEL_NEWDRAWER_STR "New Drawer"
#define MSGID_GADGETTEXT_CREATEICON_STR "Create _Icon"
#define MSGID_BUTTON_OK_STR "_Ok"
#define MSGID_BUTTON_CANCEL_STR "_Cancel"
#define MSGID_TITLE_ERRORREQ_STR "NewDrawer Error"
#define MSGID_ERRORREQ_BODYTEXT_STR "Could not create new drawer\n'%s'\n%s"
#define MSGID_REQ_OK_STR " _Ok"

#endif /* CATCOMP_STRINGS */


/****************************************************************************/


#ifdef CATCOMP_ARRAY

struct CatCompArrayType
{
    LONG   cca_ID;
    STRPTR cca_Str;
};

static const struct CatCompArrayType CatCompArray[] =
{
    {MSGID_WINTITLE_NEWDRAWER,(STRPTR)MSGID_WINTITLE_NEWDRAWER_STR},
    {MSGID_LABEL_ENTERNAME,(STRPTR)MSGID_LABEL_ENTERNAME_STR},
    {MSGID_NEWNAME_DEFAULT,(STRPTR)MSGID_NEWNAME_DEFAULT_STR},
    {MSGID_LABEL_NEWDRAWER,(STRPTR)MSGID_LABEL_NEWDRAWER_STR},
    {MSGID_GADGETTEXT_CREATEICON,(STRPTR)MSGID_GADGETTEXT_CREATEICON_STR},
    {MSGID_BUTTON_OK,(STRPTR)MSGID_BUTTON_OK_STR},
    {MSGID_BUTTON_CANCEL,(STRPTR)MSGID_BUTTON_CANCEL_STR},
    {MSGID_TITLE_ERRORREQ,(STRPTR)MSGID_TITLE_ERRORREQ_STR},
    {MSGID_ERRORREQ_BODYTEXT,(STRPTR)MSGID_ERRORREQ_BODYTEXT_STR},
    {MSGID_REQ_OK,(STRPTR)MSGID_REQ_OK_STR},
};

#endif /* CATCOMP_ARRAY */


/****************************************************************************/


#ifdef CATCOMP_BLOCK

static const char CatCompBlock[] =
{
    "\x00\x00\x01\x00\x00\x12"
    MSGID_WINTITLE_NEWDRAWER_STR "\x00\x00"
    "\x00\x00\x01\x01\x00\x24"
    MSGID_LABEL_ENTERNAME_STR "\x00"
    "\x00\x00\x01\x02\x00\x10"
    MSGID_NEWNAME_DEFAULT_STR "\x00"
    "\x00\x00\x01\x03\x00\x0C"
    MSGID_LABEL_NEWDRAWER_STR "\x00\x00"
    "\x00\x00\x01\x04\x00\x0E"
    MSGID_GADGETTEXT_CREATEICON_STR "\x00\x00"
    "\x00\x00\x01\x05\x00\x04"
    MSGID_BUTTON_OK_STR "\x00"
    "\x00\x00\x01\x06\x00\x08"
    MSGID_BUTTON_CANCEL_STR "\x00"
    "\x00\x00\x01\x07\x00\x10"
    MSGID_TITLE_ERRORREQ_STR "\x00"
    "\x00\x00\x01\x08\x00\x24"
    MSGID_ERRORREQ_BODYTEXT_STR "\x00"
    "\x00\x00\x01\x09\x00\x06"
    MSGID_REQ_OK_STR "\x00\x00"
};

#endif /* CATCOMP_BLOCK */


/****************************************************************************/


struct LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};



#endif /* NEWDRAWERSTRINGS_H */
