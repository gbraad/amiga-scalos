# $Date$
# $Revision$
##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

MKDIR   	= mkdir -p #makedir force
DT_DIR		= scalos:IconDatatypes/datatypes
MCPGFX_DIR	= $(TOPLEVEL)/common/McpGfx

##############################################################################
# Check gcc

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

INCLUDES	+=	-I$(MCPGFX_DIR)/

LFLAGS	+=	-nostartfiles \
		-lmempools \
#		--verbose


else
ifeq ($(MACHINE), ppc-amigaos)

###############################################################################
# AmigaOS4

INCLUDES	+=	-I$(MCPGFX_DIR)/

LFLAGS	+=	-nostartfiles \

else
ifeq ($(MACHINE), i386-aros)

###############################################################################
# i386-aros

INCLUDES	+=	-I$(MCPGFX_DIR)/

LFLAGS	+=	-nostartfiles -lrom \

else
###############################################################################
# AmigaOS

INCLUDES	+=	-I$(MCPGFX_DIR)/

LFLAGS	+=	-ldebug -lmcpgfx -lnix -lamiga -lstubs

endif
endif
endif
