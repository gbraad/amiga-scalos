# makefile für Scalos popupmenu library
# Copyright Henrik Isaksson <henrik@isaksson.tk>
# $Date$
# $Revision$
# $Id$
#############################################################

CSRCS 	=  	popupmenu-classic.c	\
		pm.c			\
		pmgraph.c		\
		pminit.c		\
		window.c		\
                pmimages.c		\
		pmfind.c		\
		pmshadow.c		\
		pmtopography.c		\
		pmdlist.c		\
		pmlayout.c		\
		pmdrawshadow.c		\
		pmtags.c		\
		pmcreate.c		\
		pmmem.c			\
                pmprefs.c		\
                pminput.c		\
                pmversion.c		\
		iff.c			\

ASRCS 	= 

SPLAT	= sc:c/splat
LINK 	= slink
CC	= sc
LIBS 	= LIB:sc.lib LIB:debug.lib LIB:amiga.lib
PRECOMP = Include:all.gst
OBJ_DIR	= .sasobj

#############################################################

.SUFFIXES: plugin .plugin.debug

#############################################################

CFLAGS 		= optimize nostackcheck nover dbg=s DATA=far idlen=64 idir=//include
LNFLAGS		= quiet batch noicons stripdebug SD
LNDBFLAGS 	= quiet batch noicons addsym SD

ifdef L
AFLAGS	= QUIET DS NOEXE opt=NRQB LIST=* XREFS linedebug I=SC:Assembler_Headers
#AFLAGS = -l -t -iINCLUDE:
else
AFLAGS	= QUIET DS opt=NRQB NOEXE linedebug I=SC:Assembler_Headers
#AFLAGS = -t -iINCLUDE:
endif

#############################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

#############################################################

OBJS = $(ASRCS:%.asm=$(OBJ_DIR)/%.o) $(CSRCS:%.c=$(OBJ_DIR)/%.o)

#############################################################

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $* objectname $@

############################################################

NAME    = popupmenu.library
DBGNAME = $(NAME).debug

#############################################################

all:	$(NAME) $(DBGNAME)

#############################################################

pmpriv.h : compiler.h popupmenu.h pmtypes.h pmmem.h \
	pmshadow.h pmtopography.h pminput.h \
        pmfind.h pminternals.h pmgraph.h pmimages.h

$(OBJ_DIR)/popupmenu-classic.o 	: pmpriv.h
$(OBJ_DIR)/pm.o 		: pmpriv.h
$(OBJ_DIR)/pmgraph.o 		: pmpriv.h
$(OBJ_DIR)/pminit.o 		: pmpriv.h
$(OBJ_DIR)/window.o 		: pmpriv.h
$(OBJ_DIR)/pmfind.o 		: pmpriv.h
$(OBJ_DIR)/pmshadow.o 		: pmpriv.h
$(OBJ_DIR)/pmtopography.o 	: pmpriv.h
$(OBJ_DIR)/pmdlist.o 		: pmpriv.h
$(OBJ_DIR)/pmlayout.o 		: pmpriv.h
$(OBJ_DIR)/pmdrawshadow.o 	: pmpriv.h
$(OBJ_DIR)/pmtags.c 		: pmpriv.h
$(OBJ_DIR)/pmcreate.o 		: pmpriv.h
$(OBJ_DIR)/pmmem.o 		: pmpriv.h
$(OBJ_DIR)/pmprefs.o 		: pmpriv.h
$(OBJ_DIR)/pminput.o 		: pmpriv.h
$(OBJ_DIR)/pmverson.o 		: pmpriv.h
$(OBJ_DIR)/iff.o 		: pmpriv.h

#############################################################

$(NAME) : $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LINK) to $@ FROM $(OBJS) lib $(LIBS) $(LNFLAGS)

$(DBGNAME) : $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LINK) to $@ FROM $(OBJS) lib $(LIBS) $(LNDBFLAGS)

#############################################################

install:
	@printf '\033[32mInstall: \033[31m\033[1m$(NAME)\033[0m to \033[1mLIBS: \033[0m\n'
	@copy $(NAME) LIBS: clone

#############################################################

clean:
	@printf '\033[32mCleanup: \033[31m\033[1m'
	@delete $(NAME) $(DBGNAME) $(OBJS)
	@printf '\033[0m'

#############################################################

nodebug:
	-@$(SPLAT) -s -o "d2(" "d1(" "#?.c"

#############################################################
