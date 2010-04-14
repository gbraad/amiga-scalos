# $Date$
# $Revision$
##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

MKDIR   	= mkdir -p #makedir force
DT_DIR		= scalos:IconDatatypes/datatypes

##############################################################################
# Check gcc

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

LFLAGS	+=	-nostartfiles \
		-lz \
		-lmempools \
#		--verbose


else
ifeq ($(MACHINE), ppc-amigaos)

###############################################################################
# AmigaOS4

LFLAGS	+=	-nostartfiles \
		-lz \
#

else

###############################################################################
# AmigaOS

LFLAGS	+=	-ldebug -lmcpgfx -lnix -lamiga -lstubs

endif
endif

