##############################################################################
# Check gcc

ifeq ($(MACHINE), ppc-amigaos)

###############################################################################
# AmigaOS4

BEGIN_OBJS	:=	/SDK/clib2/lib.threadsafe/crtbegin.o \
			$(OBJDIR)/plugin-aos4.o \
			$(OBJDIR)/plugin-aos4-68kstubs.o
END_OBJS	:=	/SDK/clib2/lib.threadsafe/crtend.o

else

###############################################################################
# AmigaOS, MorphOS

BEGIN_OBJS	:= $(OBJDIR)/plugin-classic.o
END_OBJS	:=

endif

