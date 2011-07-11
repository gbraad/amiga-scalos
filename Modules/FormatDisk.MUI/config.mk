# $Date$
# $Revision$
##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

SCALOS_LOCALE	= $(OBJDIR)/FormatDisk_Locale.h

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


else
ifeq ($(MACHINE), i386-aros)

###############################################################################
# i386-aros

INCLUDES	+= 	#

LFLAGS	+=	-lmui -larossupport


else

###############################################################################
# AmigaOS

INCLUDES	+= 	#

LFLAGS	+=	#

endif
endif
endif
