# makefile f�r Scalos IconObject.datatype
# $Date$
# $Revision$
# $Id$
#####################################################################

CSRCS 		= 				\
		iconobj-classic.c		\
		iconobj.c			\
		argb.c				\
		$(MCPGFX_DIR)/ScalosMcpGfx.c	\

ASRCS 		= 

#####################################################################

SPLAT		= sc:c/splat
AS 		= phxAss
LD 		= slink
CC 		= sc
ECHO		= echo
LIBS 		= LIB:debug.lib LIB:amiga.lib
PRECOMP 	= Include:all.gst
OBJ_DIR		= .sasobj
MCPGFX_DIR	= ../../common/McpGfx

.SUFFIXES: .asm

#####################################################################

CFLAGS 		= optimize nostackcheck nover math=standard data=far \
		  dbg=f idlen=64 ignore=306 \
		  idir=//include idir=//common/McpGfx 
LNFLAGS 	= quiet batch noicons stripdebug
LNDBFLAGS 	= quiet batch noicons addsym
LIBS 		= LIB:sc.lib LIB:debug.lib LIB:amiga.lib

ifdef L
AFLAGS 		= QUIET DS NOEXE opt=NRQB LIST=* linedebug I=SC:Assembler_Headers
else
AFLAGS 		= QUIET DS NOEXE opt=NRQB linedebug I=SC:Assembler_Headers
endif

#####################################################################

XSRCS = $(notdir $(CSRCS))
OBJS = $(ASRCS:%.asm=$(OBJ_DIR)/%.o) $(XSRCS:%.c=$(OBJ_DIR)/$(notdir %).o)

#####################################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

#####################################################################

DTNAME		= iconobject.datatype
DTDBGNAME	= $(DTNAME).debug

#####################################################################

all:	$(DTNAME) \
	$(DTDBGNAME)

#####################################################################

$(OBJ_DIR)/ScalosMcpGfx.o: $(MCPGFX_DIR)/ScalosMcpGfx.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $* objectname $@

#####################################################################

$(DTNAME): $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) from $(OBJS) to $(DTNAME) LIB $(LIBS) $(LNFLAGS)

$(DTDBGNAME): $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) from $(OBJS) to $(DTDBGNAME) LIB $(LIBS) $(LNDBFLAGS)

#####################################################################

$(OBJ_DIR)/iconobj-classic.o	: iconobj.h
$(OBJ_DIR)/iconobj.o	: iconobj.h
$(OBJ_DIR)/argb.o	: iconobj.h
$(OBJDIR)/ScalosMcpGfx.o : $(MCPGFX_DIR)/ScalosMcpGfx.c

#####################################################################

install:
	@printf '\033[32mFlushing memory...\033[0m\n'
	avail flush
	@printf '\033[32mInstall: \033[31m\033[1m$(DTNAME)\033[0m\n'
	copy $(DTNAME) sys:classes/datatypes

#####################################################################

clean:
	-@delete $(DTNAME) $(DTDBGNAME) $(OBJS)

#############################################################

nodebug:
	-@$(SPLAT) -s -o "d2(" "d1(" "#?.c"

#####################################################################
