##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

FONTSAMPLE_DIR	= $(TOPLEVEL)/common/FontSampleMCC
DATATYPESMCC_DIR  = $(TOPLEVEL)/common/DataTypesMCC
MCPGFX_DIR	= $(TOPLEVEL)/common/McpGfx

INCLUDES	+=	-I$(FONTSAMPLE_DIR)/	\
			-I$(MCPGFX_DIR)/	\
			-I$(DATATYPESMCC_DIR)/	\

SCALOS_LOCALE	= $(OBJDIR)/ScalosPrefs_locale.h

CFLAGS += -D SCALOSLOCALE=$(SCALOS_LOCALE)

##############################################################################
# Check gcc

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS


LFLAGS		+=	#


else
ifeq ($(MACHINE), ppc-amigaos)

###############################################################################
# AmigaOS4

LFLAGS		+=	#


else

###############################################################################
# AmigaOS

# remove "-msmall-code"
OPTIMIZE	=	-O3 -fomit-frame-pointer

LFLAGS	+=	-lstack \
		-lnix \
		-lnixmain \
		-lamiga21 \
		-lamiga \
		-lstubs

endif
endif

