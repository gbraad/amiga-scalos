# makefile für Scalos scalosgfx.library
# $Date$
# $Revision$
# $Id$
############################################################

CSRCS 	=	scalosgfx-classic.c \
		scalosgfx.c \
                argb.c \
                blit.c \
                Dither.c \
		Render.c \
                BitMapScale.c \


ASRCS 	= 

SPLAT	= sc:c/splat
LINK 	= slink
CC	= sc
LIBS 	= LIB:scmnb.lib LIB:scnb.lib LIB:debug.lib LIB:amiga.lib
PRECOMP = Include:all.gst
OBJ_DIR	= .sasobj

#############################################################

.SUFFIXES: .s .asm .plugin .plugin.debug

#############################################################

CFLAGS 		= optimize nochkabort nostackcheck nover \
		  dbg=f DATA=far code=far \
		  ignore=306 idlen=64 idir=//include
LNFLAGS		= quiet batch noicons stripdebug 
LNDBFLAGS 	= quiet batch noicons addsym

#############################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

#############################################################

OBJS = $(CSRCS:%.c=$(OBJ_DIR)/%.o)

#############################################################

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $* objectname $@

############################################################

NAME    = scalosgfx.library
DBGNAME = $(NAME).debug

#############################################################

all:	$(NAME) $(DBGNAME)
# install
# clean

#############################################################

$(OBJ_DIR)/scalosgfx-classic.o 	: scalosgfx_base.h scalosgfx.h
$(OBJ_DIR)/scalosgfx.o 		: scalosgfx_base.h scalosgfx.h
$(OBJ_DIR)/argb.o 		: scalosgfx_base.h scalosgfx.h
$(OBJ_DIR)/blit.o 		: scalosgfx_base.h scalosgfx.h
$(OBJ_DIR)/Dither.o 		: scalosgfx_base.h scalosgfx.h
$(OBJ_DIR)/Render.o 		: scalosgfx_base.h scalosgfx.h
$(OBJ_DIR)/BitMapScale.o	: scalosgfx_base.h scalosgfx.h

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
