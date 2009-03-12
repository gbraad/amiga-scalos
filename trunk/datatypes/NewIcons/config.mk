##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################
# Check gcc

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

LFLAGS	+=	-nostartfiles \
		-lmempools \
#		--verbose


else

###############################################################################
# AmigaOS

LFLAGS	+=	-ldebug -lamiga -lpreferences -lcybergraphics -lnix 

endif
