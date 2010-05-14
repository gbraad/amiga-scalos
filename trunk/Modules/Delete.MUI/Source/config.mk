# $Date$
# $Revision$
##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

SCALOS_LOCALE	= $(OBJDIR)/Delete_Locale.h

CFLAGS += -D SCALOSLOCALE=$(SCALOS_LOCALE)

##############################################################################
# Check gcc

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

INCLUDES	+= 	#

LFLAGS	+=	#


else
ifeq ($(MACHINE), ppc-amigaos)

###############################################################################
# AmigaOS4

INCLUDES	+= 	#

LFLAGS	+=	#

###############################################################################
# AmigaOS

INCLUDES	+= 	#

LFLAGS	+=	-lauto

endif
endif
