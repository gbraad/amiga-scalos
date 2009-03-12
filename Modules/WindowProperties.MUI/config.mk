# $Date$
# $Revision$
#############################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

TOOLTYPE_DIR	=  ../IconProperties.MUI
ICONOBJMCC_DIR	= $(TOPLEVEL)/common/IconobjectMCC
BITMAPMCC_DIR	= $(TOPLEVEL)/common/BitMapMCC
BACKFILLMCC_DIR	= $(TOPLEVEL)/common/BackfillMCC

vpath	%.c	$(TOOLTYPE_DIR)	$(ICONOBJMCC_DIR) $(BITMAPMCC_DIR) $(BACKFILLMCC_DIR)

INCLUDES	+= 	-I$(TOOLTYPE_DIR) \
			-I$(BITMAPMCC_DIR) \
			-I$(BACKFILLMCC_DIR) \
			-I$(ICONOBJMCC_DIR)

##############################################################################
# Check gcc

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

LFLAGS	+=	#


else
ifeq ($(MACHINE), ppc-amigaos)

###############################################################################
# AmigaOS4

LFLAGS	+=	#


else

###############################################################################
# AmigaOS

LFLAGS	+=	#

endif
endif

