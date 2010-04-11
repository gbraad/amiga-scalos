# makefile für Scalos IconObject datatypes stuff
# $Date$
# $Revision$
# $Id$
#####################################################################

CSRCS 		= 	NewIconDt.c
ASRCS 		=	#newiconobject.asm

#####################################################################

AS 		= phxAss
LD 		= slink
CC 		= sc
LIBS 		= LIB:debug.lib LIB:sc.lib LIB:amiga.lib
PRECOMP 	= Include:all.gst
OBJ_DIR		= .sasobj

.SUFFIXES: .asm

#####################################################################

CFLAGS		= optimize nostackcheck nochkabort strcons debug=s \
		  strmer nover streq idir=sc:include/ idir=include: idir=//include
LNFLAGS 	= batch noicons stripdebug quiet
LNDBFLAGS 	= batch noicons addsym quiet

ifdef L
AFLAGS 		= QUIET DS NOEXE opt=NRQB LIST=* linedebug I=SC:Assembler_Headers
else
AFLAGS 		= QUIET DS NOEXE opt=NRQB linedebug I=SC:Assembler_Headers
endif

#####################################################################

OBJS = $(ASRCS:%.asm=$(OBJ_DIR)/%.o) $(CSRCS:%.c=$(OBJ_DIR)/%.o)

#####################################################################

DTNAME		= newiconobject.datatype
DTDBGNAME	= $(DTNAME).debug

#####################################################################

all:	$(DTNAME) \
	$(DTDBGNAME)

#####################################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

#####################################################################

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $< objectname $@

$(OBJ_DIR)/%.o : %.s
	@printf '\033[32mAssemble: \033[31m\033[1m$<\033[0m\n'
	@$(AS) $(AFLAGS) $< to $@

$(OBJ_DIR)/%.o : %.asm
	@printf '\033[32mAssemble: \033[31m\033[1m$<\033[0m\n'
	@$(AS) $(AFLAGS) $< to $@

#####################################################################

$(OBJ_DIR)/NewIconDt.o : NewIconDt.h

#####################################################################

$(DTNAME) : $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) from $(OBJS) to $(DTNAME) LIB $(LIBS) $(LNFLAGS)

$(DTDBGNAME) : $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) from $(OBJS) to $(DTDBGNAME) LIB $(LIBS) $(LNDBFLAGS)

#####################################################################

install:
	@printf '\033[32mFlushing memory...\033[0m\n'
	avail flush
	@printf '\033[32mInstall: \033[31m\033[1m$(DTNAME)\033[0m\n'
	@copy $(DTNAME) scalos:IconDatatypes/datatypes

#####################################################################

clean:
	-@delete $(DTNAME) $(DTDBGNAME) $(OBJS)

#####################################################################

