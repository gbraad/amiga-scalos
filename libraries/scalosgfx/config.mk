# $Date$
# $Revision$

ifndef TOPLEVEL
	TOPLEVEL=$(shell pwd)/../..
endif

###############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

###############################################################################
# Check compiler

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

LFLAGS	+=	-nostartfiles \
#		--verbose


else
ifeq ($(MACHINE), ppc-amigaos)

###############################################################################
# AmigaOS4

LFLAGS	+=	-nostartfiles \
#		--verbose

else
ifeq ($(MACHINE), i386-aros)

###############################################################################
# i386-aros

LFLAGS	+=	-nostartfiles -lrom -larossupport

else

###############################################################################
# AmigaOS

LFLAGS	+=	-lstack -lnix -lnixmain -lamiga -lstubs

endif
endif
endif
