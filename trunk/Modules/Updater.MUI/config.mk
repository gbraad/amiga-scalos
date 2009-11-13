# $Date$
# §Revision:                 $
##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

ICONOBJMCC_DIR	= $(TOPLEVEL)/common/IconobjectMCC

##############################################################################
# Check gcc

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

INCLUDES	+=      -I$(ICONOBJMCC_DIR) \

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

LFLAGS	+=	#

endif
endif

