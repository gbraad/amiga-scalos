# makefile f?r Scalos IconObject datatypes stuff
# $Date$
# $Revision$
#####################################################################

ASRCS 		=	
CSRCS 		= 				\
		AmigaIconObject-classic.c  	\
		AmigaIconObject.c		\


#####################################################################

AS		= phxAss
LD 		= slink
CC 		= sc
MKDIR   	= mkdir -p #makedir force
LIBS 		= LIB:debug.lib LIB:sc.lib LIB:amiga.lib
PRECOMP 	= Include:all.gst
DT_DIR		= scalos:IconDatatypes/datatypes
OBJDIR		= .sasobj

.SUFFIXES: .asm

#####################################################################

CFLAGS 		= optimize nostackcheck nover math=standard data=far \
		  dbg=f idir=//include
LNFLAGS 	= batch noicons stripdebug quiet
LNDBFLAGS 	= batch noicons addsym quiet

ifdef L
AFLAGS 		= QUIET DS NOEXE opt=NRQB LIST=* linedebug I=SC:Assembler_Headers
else
AFLAGS 		= QUIET DS NOEXE opt=NRQB linedebug I=SC:Assembler_Headers
endif

#####################################################################

OBJS = $(ASRCS:%.asm=$(OBJDIR)/%.o) $(CSRCS:%.c=$(OBJDIR)/%.o)

#####################################################################

DTNAME		= .bin_os3/amigaiconobject.datatype
DTDBGNAME	= $(DTNAME).debug

#####################################################################

all:	$(DTNAME) \
	$(DTDBGNAME)

#####################################################################

$(OBJDIR)::
	@[ -d $(OBJDIR) ] || mkdir $(OBJDIR) > /dev/null 2>&1

#####################################################################

$(OBJDIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $* objectname $@

$(OBJDIR)/%.o : %.s
	@printf '\033[32mAssemble: \033[31m\033[1m$<\033[0m\n'
	@$(AS) $(AFLAGS) $< to $@

$(OBJDIR)/%.o : %.asm
	@printf '\033[32mAssemble: \033[31m\033[1m$<\033[0m\n'
	@$(AS) $(AFLAGS) $< to $@

#####################################################################

$(DTNAME): $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) from $(OBJS) to $(DTNAME) LIB $(LIBS) $(LNFLAGS)

$(DTDBGNAME): $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) from $(OBJS) to $(DTDBGNAME) LIB $(LIBS) $(LNDBFLAGS)

#####################################################################

$(OBJDIR)/AmigaIconObject.o $(OBJDIR)/AmigaIconObject-classic.o : AmigaIconObject.h

#####################################################################

install:
	@printf '\033[32mFlushing memory...\033[0m\n'
	avail flush
	@printf '\033[32mInstall: \033[31m\033[1m$(DTNAME)\033[0m\n'
	-@$(MKDIR) $(DT_DIR)
	copy $(DTNAME) $(DT_DIR)

#####################################################################

clean:
	-@delete $(DTNAME) $(DTDBGNAME) $(OBJS)

#####################################################################

