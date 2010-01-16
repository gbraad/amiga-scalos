# $Date$
# $Revision$
##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

INT64_DIR	= $(TOPLEVEL)/common/Int64
FS_DIR		= $(TOPLEVEL)/common/Fs

vpath	%.c	$(INT64_DIR) $(FS_DIR)

##############################################################################

INCLUDES	+=      -I$(INT64_DIR) \
			-I$(FS_DIR) \

##############################################################################
# Check gcc

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

INCLUDES	+=

LFLAGS	+=	#


else
ifeq ($(MACHINE), ppc-amigaos)

###############################################################################
# AmigaOS4

INCLUDES	+=

LFLAGS	+=	#


else

###############################################################################
# AmigaOS

INCLUDES	+=

LFLAGS	+=	-lstack \
		-lnix \
		-lnixmain \
		-lamiga21 \
		-lamiga \
		-lstubs

endif
endif

