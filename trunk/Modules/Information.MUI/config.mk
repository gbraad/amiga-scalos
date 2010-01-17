# $Date$
# $Revision$
##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

TOOLTYPE_DIR	=  ../IconProperties.MUI
ICONOBJMCC_DIR	= $(TOPLEVEL)/common/IconobjectMCC
INT64_DIR	= $(TOPLEVEL)/common/Int64
FS_DIR	     	= $(TOPLEVEL)/common/Fs

vpath	%.c	$(TOOLTYPE_DIR) $(ICONOBJMCC_DIR) $(INT64_DIR) $(FS_DIR)

##############################################################################

INCLUDES	+= 	-I$(TOOLTYPE_DIR) \
			-I$(ICONOBJMCC_DIR) \
			-I$(INT64_DIR) \
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

LFLAGS	+=	#

endif
endif

