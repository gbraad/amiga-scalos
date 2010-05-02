# $Date$
# $Revision$
##############################################################################

ifndef $(TOPLEVEL)
	TOPLEVEL=$(shell pwd)/..
endif

###############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################

MCPGFX_DIR	= $(TOPLEVEL)/common/McpGfx
INT64_DIR	= $(TOPLEVEL)/common/Int64
FS_DIR		= $(TOPLEVEL)/common/Fs
MALLOC_DIR	= $(TOPLEVEL)/common/malloc

vpath	%.c	$(MCPGFX_DIR) $(INT64_DIR) $(FS_DIR) $(MALLOC_DIR)

SCALOS_LOCALE	= $(OBJDIR)/Scalos_Locale.h

CFLAGS += -D SCALOSLOCALE=$(SCALOS_LOCALE)

###############################################################################

INCLUDES	+=	-I$(MCPGFX_DIR)/ \
			-I$(INT64_DIR)/ \
			-I$(FS_DIR)/ \
			-I$(MALLOC_DIR)/ \

###############################################################################
# Check compiler

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

GENMSGIDNAMES 	= Tools/$(BINDIR)/GenMsgIdNames
INCLUDES	+=

LFLAGS	+=	-lpng \
		-lz \
		#


else
ifeq ($(MACHINE), ppc-amigaos)

###############################################################################
# AmigOS4

GENMSGIDNAMES 	= ./GenMsgIdNames
INCLUDES	+=

LFLAGS	+=      -lpng \
		-lz \
		-lm \
		#

else

###############################################################################
# AmigaOS

GENMSGIDNAMES 	= GenMsgIdNames
INCLUDES	+=

LFLAGS	+=	-lpng \
		-lgcc \
		-lnix \
		-lnixmain \
		-lamiga21 \
		-lamiga \
		-lstubs

endif
endif
