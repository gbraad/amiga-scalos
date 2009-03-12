##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

DATATYPESMCC_DIR  = $(TOPLEVEL)/common/DataTypesMCC
COMMON_DIR	= $(TOPLEVEL)/common/Plugin

INCLUDES	+= -I$(DATATYPESMCC_DIR) -I$(COMMON_DIR)

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
