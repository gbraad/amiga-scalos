#ifndef SCALOSPREVIEWPLUGIN_INTERFACE_DEF_H
#define SCALOSPREVIEWPLUGIN_INTERFACE_DEF_H

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

#ifndef SCALOS_SCALOS_H
#include <scalos/scalos.h>
#endif
#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

struct ScalosPreviewPluginIFace
{
	struct InterfaceData Data;

	uint32 APICALL (*Obtain)(struct ScalosPreviewPluginIFace *Self);
	uint32 APICALL (*Release)(struct ScalosPreviewPluginIFace *Self);
	void APICALL (*Expunge)(struct ScalosPreviewPluginIFace *Self);
	struct Interface * APICALL (*Clone)(struct ScalosPreviewPluginIFace *Self);
	LONG APICALL (*SCAPreviewGenerate)(struct ScalosPreviewPluginIFace *Self, struct ScaWindowTask * wt, BPTR dirLock, CONST_STRPTR iconName, struct TagItem * tagList);
	LONG APICALL (*SCAPreviewGenerateTags)(struct ScalosPreviewPluginIFace *Self, struct ScaWindowTask * wt, BPTR dirLock, CONST_STRPTR iconName, ...);
};

#endif /* SCALOSPREVIEWPLUGIN_INTERFACE_DEF_H */
