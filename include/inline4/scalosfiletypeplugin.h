/*
** This file was automatically generated by fdtrans 52.1.
** Do not edit it by hand. Instead, edit the sfd file
** that was used to generate this file
*/

#ifndef INLINE4_SCALOSFILETYPEPLUGIN_H
#define INLINE4_SCALOSFILETYPEPLUGIN_H

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

#define SCAToolTipInfoString(_ttshd,_args ) ({ \
	STRPTR _ret; \
	if (IScalosFileTypePlugin != NULL) { \
		_ret = IScalosFileTypePlugin->SCAToolTipInfoString(_ttshd,_args ); \
	} else { \
		_ret = EmulateTags(ScalosFileTypePluginBase, \
			ET_Offset, -30, \
			ET_RegisterA0, _ttshd, \
			ET_RegisterA1, _args, \
			ET_RegisterA6, ScalosFileTypePluginBase, \
			ET_SaveRegs, TRUE, \
			TAG_DONE); \
	} \
	_ret; \
})

#endif
