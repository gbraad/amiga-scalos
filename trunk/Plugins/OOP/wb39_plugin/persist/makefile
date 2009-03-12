# makefile for Scalos persist.plugin
# $Date$
# using GNU make and SAS/C 6.58


#############################################################

CC 		= sc
AS		= phxass
PRECOMP 	= INCLUDE:All.gst
LD      	= slink
LDFLAGS 	= quiet batch noicons sd
LDLIBS  	= LIB:sc.lib LIB:debug.lib LIB:amiga.lib
OBJ_DIR		= .sasobj
DESTPLUG	= Scalos:Plugins/OOP
COMMON_DIR 	= ../../../../common/Plugin

.SUFFIXES: .plugin .plugin.debug

#############################################################

# Optimizer Flags
# Ignore Note 306: local function inlined: "InsertMH"
OPT_FLG 	= OPTIMIZE OPTINLOCAL OPTTIME OPTSCHED IGNORE=306,308 ERROR=87
OPT_FLG2 	= NOOPTIMIZE IGNORE=306,308 ERROR=87

ifdef DEBUG
	CFLAGS		= nostkchk nochkabort dbg=ff nover gst=$(PRECOMP) idlen=64 \
			idir=////include \
                  	idir=$(subst ../,/,$(COMMON_DIR))

	CSTARTUP	= LIB:c.o
else
	CFLAGS		= nostkchk nochkabort $(OPT_FLG) DBG=f def=NDEBUG \
			  nover gst=$(PRECOMP) idlen=64 \
			idir=////include \
                  	idir=$(subst ../,/,$(COMMON_DIR))
	CSTARTUP 	= LIB:cback.o
endif

AFLAGS		= quiet I=sc:Assembler_Headers

#############################################################

PERSISTNAME  	= persist.plugin
CSRCS 		= 	$(COMMON_DIR)/plugin-classic.c \
			persist.c \

XSRCS = $(notdir $(CSRCS))
OBJS= $(XSRCS:%.c=$(OBJ_DIR)/$(notdir %).o)

#############################################################

LIBS	= LIB:mempools.lib LIB:sc.lib LIB:debug.lib LIB:amiga.lib

#############################################################

all:	$(PERSISTNAME) \
	$(PERSISTNAME).debug \
# install
# clean

#############################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

#############################################################

$(OBJ_DIR)/plugin-classic.o : $(COMMON_DIR)/plugin-classic.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $*.c objectname $@

#############################################################

$(OBJ_DIR)/plugin-classic.o : $(COMMON_DIR)/plugin-classic.c \
	$(COMMON_DIR)/plugin-common.c $(COMMON_DIR)/plugin.h plugin_data.h

$(OBJ_DIR)/persist.o : Persist.c Persist.h plugin_data.h

#############################################################

$(PERSISTNAME): $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(OBJS) TO $(PERSISTNAME) lib $(LIBS) $(LDFLAGS) STRIPDEBUG 

$(PERSISTNAME).debug: $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(OBJS) TO $(PERSISTNAME).debug lib $(LIBS) $(LDFLAGS) ADDSYM

#############################################################

install:
	@printf '\033[32mInstall: \033[31m\033[1m$(PERSISTNAME).\033[0m\n'
	@copy $(PERSISTNAME) $(DESTPLUG) clone

#############################################################

clean:
	@printf '\033[32mCleanup: \033[31m\033[1m'
	-@delete $(OBJS) \
		$(PERSISTNAME) \
		$(PERSISTNAME).debug
	@printf '\033[0m'

#############################################################
