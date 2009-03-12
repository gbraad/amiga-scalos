# $Date$
# §Revision:                 $
##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

FONTSAMPLE_DIR	= $(TOPLEVEL)/common/FontSampleMCC
ICONOBJMCC_DIR	= $(TOPLEVEL)/common/IconobjectMCC
DATATYPESMCC_DIR  = $(TOPLEVEL)/common/DataTypesMCC
COMMON_DIR	= $(TOPLEVEL)/common/Plugin

INCLUDES	+=	-I$(FONTSAMPLE_DIR) \
			-I$(ICONOBJMCC_DIR) \
			-I$(DATATYPESMCC_DIR) \
			-I$(COMMON_DIR)

vpath 	%.c	$(COMMON_DIR)

##############################################################################

include $(COMMON_DIR)/config.mk


##############################################################################
# Check gcc

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

LFLAGS	+=	-nostartfiles \
		-lmempools
#		--verbose


OPTIONS	+=	-Wno-format-y2k


else
ifeq ($(MACHINE), ppc-amigaos)

###############################################################################
# AmigaOS4

LFLAGS  +=      -nostartfiles \
#


else

###############################################################################
# AmigaOS

LFLAGS	+=	-lscalos \
		-lpreferences \
		-liconobject \
		-lmempools \
		-ldebug \
		-lmui \
		-lstack \
		-lnix \
		-lnixmain \
		-lamiga21 \
		-lamiga \
		-lstubs \

endif
endif

