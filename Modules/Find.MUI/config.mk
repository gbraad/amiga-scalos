# $Date$
# §Revision:                 $
##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

SCALOS_LOCALE	= $(OBJDIR)/Find_Locale.h

CFLAGS += -D SCALOSLOCALE=$(SCALOS_LOCALE)

ICONOBJMCC_DIR	= $(TOPLEVEL)/common/IconobjectMCC

##############################################################################
# Check gcc

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

INCLUDES	+=      -I$(ICONOBJMCC_DIR) \

LFLAGS	+=	#


else
ifeq ($(MACHINE), ppc-amigaos)

###############################################################################
# AmigaOS4

INCLUDES	+=      -I$(ICONOBJMCC_DIR) \

LFLAGS	+=	-lauto


else


###############################################################################
# AmigaOS

LFLAGS	+=	#

endif
endif

