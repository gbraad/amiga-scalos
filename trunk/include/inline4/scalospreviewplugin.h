/*
** This file was automatically generated by fdtrans 52.1.
** Do not edit it by hand. Instead, edit the sfd file
** that was used to generate this file
*/

#ifndef INLINE4_SCALOSPREVIEWPLUGIN_H
#define INLINE4_SCALOSPREVIEWPLUGIN_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef EXEC_EXEC_H
#include <exec/exec.h>
#endif
#ifndef EXEC_INTERFACES_H
#include <exec/interfaces.h>
#endif

#ifndef EXEC_EMULATION_H
#include <exec/emulation.h>
#endif

#ifndef SCALOS_SCALOS_H
#include <scalos/scalos.h>
#endif
#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

#define SCAPreviewGenerate(_wt,_dirLock,_iconName,_tagList ) ({ \
	LONG _ret; \
	if (IScalosPreviewPlugin != NULL) { \
		_ret = IScalosPreviewPlugin->SCAPreviewGenerate(_wt,_dirLock,_iconName,_tagList ); \
	} else { \
		_ret = EmulateTags(ScalosPreviewPluginBase, \
			ET_Offset, -30, \
			ET_RegisterA0, _wt, \
			ET_RegisterA1, _dirLock, \
			ET_RegisterA2, _iconName, \
			ET_RegisterA3, _tagList, \
			ET_RegisterA6, ScalosPreviewPluginBase, \
			ET_SaveRegs, TRUE, \
			TAG_DONE); \
	} \
	_ret; \
})

#ifndef NO_INLINE_STDARG
#define SCAPreviewGenerateTags(_wt,_dirLock,_iconName,_firstTag...) ({ \
	uint32 _func_taglist[] = { _firstTag }; \
	SCAPreviewGenerate(_wt,_dirLock,_iconName,_func_taglist); \
})
#endif

#endif

