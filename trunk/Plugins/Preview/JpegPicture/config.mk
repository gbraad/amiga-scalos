##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

COMMON_DIR	= $(TOPLEVEL)/common/Plugin

INCLUDES	+= -I$(COMMON_DIR)

vpath 	%.c	$(COMMON_DIR)

##############################################################################

include $(COMMON_DIR)/config.mk

##############################################################################

# Check gcc

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

INCLUDES	+=	#

LFLAGS	+=	-nostartfiles \
		-ljpeg \
		-lmempools \
#		 -Wl,-Map,jpegpicture.map \
#		--verbose


else
ifeq ($(MACHINE), ppc-amigaos)

###############################################################################
# AmigaOS4

LFLAGS  +=      -nostartfiles \
		-ljpeg \
#


else

###############################################################################
# AmigaOS

INCLUDES	+=	#

LFLAGS	+=	-lmempools \
		-ljpeg \
		-ldebug \
		-lamiga21 \
		-lamiga \
		-lstubs

endif
endif

