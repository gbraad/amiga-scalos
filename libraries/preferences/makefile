# makefile für Scalos Preferences library (C-Version)
# $Date$
# $Revision$
# $Id$
#############################################################

CSRCS 	= Preferences-classic.c Preferences.c
ASRCS 	= 

SPLAT	= sc:c/splat
LINK 	= slink
CC	= sc
LIBS 	= LIB:sc.lib LIB:debug.lib LIB:amiga.lib
PRECOMP = Include:all.gst
OBJ_DIR	= .sasobj

#############################################################

.SUFFIXES: .s .asm .plugin .plugin.debug

#############################################################

CFLAGS 		= optimize nostackcheck nover dbg=s DATA=far idir=//include
LNFLAGS		= quiet batch noicons stripdebug SD
LNDBFLAGS 	= quiet batch noicons addsym SD

#############################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

#############################################################

OBJS = $(ASRCS:%.asm=$(OBJ_DIR)/%.o) $(CSRCS:%.c=$(OBJ_DIR)/%.o)

#############################################################

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $* objectname $@

$(OBJ_DIR)/%.o : %.s
	@printf '\033[32mAssemble: \033[31m\033[1m$<\033[0m\n'
	@$(AS) $(AFLAGS) $< to $@

$(OBJ_DIR)/%.o : %.asm
	@printf '\033[32mAssemble: \033[31m\033[1m$<\033[0m\n'
	@$(AS) $(AFLAGS) $< to $@

############################################################

NAME    = preferences.library
DBGNAME = $(NAME).debug

#############################################################

all:	$(NAME) $(DBGNAME)
# install
# clean

#############################################################

$(OBJ_DIR)/Preferences.o : Preferences_base.h                

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
	@avail flush

#############################################################

clean:
	@printf '\033[32mCleanup: \033[31m\033[1m'
	@delete $(NAME) $(DBGNAME) $(OBJS)
	@printf '\033[0m'

#############################################################

nodebug:
	-@$(SPLAT) -s -o "d2(" "d1(" "#?.c"

#############################################################
