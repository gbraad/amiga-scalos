# makefile für Scalos xtwindows.plugin
# $Date$

#############################################################

CSRCS = 	$(COMMON_DIR)/plugin-classic.c \
		XTWindows.c \

#############################################################

SUBDIRMAKE 	= $(MAKE) -s -C 
CATCOMP 	= CatComp
AS 		= phxAss
LD 		= slink
CC 		= sc
LIBS 		= LIB:scm.lib LIB:sc.lib LIB:mempools.lib \
		  LIB:debug.lib LIB:amiga.lib
PRECOMP 	= Include:all.gst
ECHO		= echo
OBJ_DIR		= .sasobj
LNFLAGS 	= quiet batch noicons stripdebug SD
LNDBFLAGS 	= quiet batch noicons addsym SD
OPENPATH	= ../wb39_Plugin
COMMON_DIR 	= ../../../common/Plugin

.SUFFIXES: .s .asm .plugin .plugin.debug
#############################################################

CFLAGS 		= optimize nostackcheck nover dbg=s DATA=far \
		  idlen=64 ignore=306 \
		  idir=///include \
		  idir=///common/Plugin
AFLAGS 		= QUIET DS opt=NRQB NOEXE linedebug I=SC:Assembler_Headers
#AFLAGS 	= -t -iINCLUDE:

#############################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

XSRCS = $(notdir $(CSRCS))
OBJS = $(XSRCS:%.c=$(OBJ_DIR)/$(notdir %).o)

#############################################################

PLUGIN   = xtwindows.plugin
DESTTOOL = Scalos:plugins/OOP/

#############################################################

All:	$(PLUGIN) \
	$(PLUGIN).debug
# install
# clean

#############################################################

$(OBJ_DIR)/plugin-classic.o  : $(COMMON_DIR)/plugin-classic.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[1m\033[31m$<\033[0m\n'
	@$(CC) $(CFLAGS) $*.c objectname $@

#############################################################

$(OBJ_DIR)/XTWindows.o : XTWindows.h AutoMsg.h $(COMMON_DIR)/plugin.h

$(OBJ_DIR)/plugin-classic.o : plugin_data.h \
	$(COMMON_DIR)/plugin.h $(COMMON_DIR)/plugin-common.c

#############################################################

$(PLUGIN) : $(OBJS)
	@printf '\033[32mLink: \033[1m\033[31m$@\033[0m\n'
	@$(LD) to $@ FROM $(OBJS) lib $(LIBS) $(LNFLAGS)

$(PLUGIN).debug : $(OBJS)
	@printf '\033[32mLink: \033[1m\033[31m$@\033[0m\n'
	@$(LD) to $@ FROM $(OBJS) lib $(LIBS) $(LNDBFLAGS)

#############################################################

# copy all generated files to their final destinations
install:
	@printf '\033[32mInstall: \033[31m\033[1m$(PLUGIN)\033[0m\n'
	@copy $(PLUGIN) $(DESTTOOL) clone

#############################################################

# A little something to clean it all up
clean:
	@printf '\033[32mDelete: \033[31m\033[1m'
	@delete $(OBJS) $(PLUGIN) $(PLUGIN).debug
	@printf '\033[0m'

#############################################################
