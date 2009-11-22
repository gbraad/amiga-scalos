# $Date$
# §Revision:                 $
##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

ICONOBJMCC_DIR	= $(TOPLEVEL)/common/IconobjectMCC
AMISSL_INC_DIR	= /gg/amissl-v3-sdk/include/include_h

##############################################################################
# Check gcc

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

INCLUDES	+=      -I$(ICONOBJMCC_DIR) \
			-I$(AMISSL_INC_DIR)\

LFLAGS	+=	\
		-lcurl \
		-lz \


else
ifeq ($(MACHINE), ppc-amigaos)

###############################################################################
# AmigaOS4

INCLUDES	+=      -I$(ICONOBJMCC_DIR) \

LFLAGS	+=	-lauto \
		-lcurl \
		-lamisslauto \
		-lz \

else


###############################################################################
# AmigaOS

INCLUDES	+=      -I$(AMISSL_INC_DIR)\

LFLAGS	+=	-nodefaultlibs \
		-lcurl \
		-lamissl \
		-lz \
		-lm2 \
		-lgcc \
		-lc \
		-ldebug \
		-lamiga \
		-lstubs \

CFLAGS		+=	-Dfd_set=APTR


endif
endif

