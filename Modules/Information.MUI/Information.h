// Information.h
// $Date$
// $Revision$


#ifndef INFORMATION_H
#define	INFORMATION_H

#define	NO_ICON_POSITION_SHORT	((UWORD) 0x8000)

#define INFO_ICONPATH_ENV	"ENV:Scalos/InfoShowIconPath"
#define INFO_ICONPATH_ENVARC	"ENVARC:Scalos/InfoShowIconPath"

#define INFO_AUTOGETSIZE_ENV	"ENV:Scalos/InfoAutoGetSize"
#define INFO_AUTOGETSIZE_ENVARC	"ENVARC:Scalos/InfoAutoGetSize"

const char versTag[] = "\0$VER: ";	// $VER version string

enum StartFromType
	{
	STARTFROM_Workbench,
	STARTFROM_CLI,
	STARTFROM_Arexx,
	};

struct DropZoneMsg
	{
	struct Message dzm_Msg;
	struct AppWindowDropZoneMsg dzm_adzm;
	};

struct DefIconInfo
	{
	BPTR dii_DirLock;
	CONST_STRPTR dii_IconName;
	Object **dii_IconObjectFromDisk;
	Object **dii_IconObjectFromScalos;
	};

#if defined(__SASC)
int snprintf(char *, size_t, const char *, /*args*/ ...);
int vsnprintf(char *, size_t, const char *, va_list ap);
#endif /* __SASC */

#endif	/* INFORMATION_H */
