# $Date$
# $Revision$
##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

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

###############################################################################
# AmigaOS

INCLUDES	+=	-I$(MCPGFX_DIR)/

LFLAGS	+=	-ldebug -lmcpgfx -lnix -lamiga -lstubs

endif
endif

