# $Date$
# $Revision$
##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

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

DEFINES	+= -DINTUITION_PRE_V36_NAMES

else

###############################################################################
# AmigaOS

INCLUDES	+= 	#

LFLAGS	+=	#

endif
endif
