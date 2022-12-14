#ifndef SCALOSPREFSPLUGIN_INTERFACE_DEF_H
#define SCALOSPREFSPLUGIN_INTERFACE_DEF_H

/*
** This file was machine generated by idltool 52.1.
** Do not edit
*/ 

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef EXEC_EXEC_H
#include <exec/exec.h>
#endif
#ifndef EXEC_INTERFACES_H
#include <exec/interfaces.h>
#endif


struct ScalosPrefsPluginIFace
{
	struct InterfaceData Data;

	uint32 APICALL (*Obtain)(struct ScalosPrefsPluginIFace *Self);
	uint32 APICALL (*Release)(struct ScalosPrefsPluginIFace *Self);
	void APICALL (*Expunge)(struct ScalosPrefsPluginIFace *Self);
	struct Interface * APICALL (*Clone)(struct ScalosPrefsPluginIFace *Self);
	ULONG APICALL (*SCAGetPrefsInfo)(struct ScalosPrefsPluginIFace *Self, LONG which);
};

#endif /* SCALOSPREFSPLUGIN_INTERFACE_DEF_H */
