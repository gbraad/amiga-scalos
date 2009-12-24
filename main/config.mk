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

vpath	%.c	$(MCPGFX_DIR) $(INT64_DIR)

###############################################################################

INCLUDES	+=	-I$(MCPGFX_DIR)/ \
			-I$(INT64_DIR)/ \

###############################################################################
# Check compiler

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

GENMSGIDNAMES 	= GenMsgIdNames
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
