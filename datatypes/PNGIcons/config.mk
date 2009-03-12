##############################################################################

include $(TOPLEVEL)/config.mk
include $(TOPLEVEL)/rules.mk

##############################################################################
# Check gcc

ifeq ($(MACHINE), ppc-morphos)

###############################################################################
# MorphOS

LFLAGS	+=      -nostartfiles \
		-lpng \
		-lz \
		-lmempools \
#		-Wl,-Map,png.map \
#		--verbose


else
ifeq ($(MACHINE), ppc-amigaos)

###############################################################################
# AmigaOS4

LFLAGS	+=      -nostartfiles \
		-lpng -lz -lm

else

###############################################################################
# AmigaOS

LFLAGS	+=	-ldebug -liconobject -lcybergraphics \
		-lmcpgfx -lz \
		-lnix -lamiga21 -lamiga -lstubs

endif
endif

