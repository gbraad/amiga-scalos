# makefile for Scalos wbrexx.plugin
# $Date$
# using GNU make and SAS/C 6.58


#############################################################

CC 		= sc
AS		= phxass
PRECOMP 	= INCLUDE:All.gst
LD      	= slink
LDFLAGS 	= quiet batch noicons sd
LDLIBS  	= LIB:sc.lib \
			LIB:debug.lib \
			LIB:amiga.lib \
			LIB:mempools.lib \
			////SAS-lib/snprintf.lib \
			LIB:sc.lib 
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
			idir=/ \
			idir=////include \
                  	idir=$(subst ../,/,$(COMMON_DIR))
	CSTARTUP	= LIB:c.o
else
	CFLAGS		= nostkchk nochkabort $(OPT_FLG) dbg=f def=NDEBUG \
			  def=NODEBUG nover gst=$(PRECOMP) idlen=64 \
			idir=/ \
			idir=////include \
                  	idir=$(subst ../,/,$(COMMON_DIR))
	CSTARTUP 	= LIB:cback.o
endif

AFLAGS		= quiet I=sc:Assembler_Headers

#############################################################

# Files for wbrexx.plugin
WBREXXNAME = wbrexx.plugin

WBREXXCSRCS = 	$(COMMON_DIR)/plugin-classic.c \
		WBRexxMain.c \
		WBRexx.c \
		RexxGetAttrs.c \
		RexxIcon.c \
		RexxCmd.c \
		../Scalos_Helper.c \

WBREXXOBJS = $(WBREXXCSRCS:%.c=$(OBJ_DIR)/%.o)

#############################################################

all:	$(WBREXXNAME) \
	$(WBREXXNAME).debug \
# install
# clean

#############################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

XSRCS = $(notdir $(WBREXXCSRCS))
WBREXXOBJS = $(XSRCS:%.c=$(OBJ_DIR)/$(notdir %).o)

#############################################################

$(OBJ_DIR)/plugin-classic.o : $(COMMON_DIR)/plugin-classic.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/Scalos_Helper.o : ../Scalos_Helper.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $*.c objectname $@

#############################################################

$(OBJ_DIR)/plugin-classic.o : $(COMMON_DIR)/plugin-classic.c \
	$(COMMON_DIR)/plugin-common.c $(COMMON_DIR)/plugin.h plugin_data.h

$(OBJ_DIR)/RexxGetAttrs.o $(OBJ_DIR)/RexxIcon.o $(OBJ_DIR)/Scalos_Helper.o \
	$(OBJ_DIR)/WBRexxMain.o : ../wb39.h

$(OBJ_DIR)/WBRexx.o $(OBJ_DIR)/RexxGetAttrs.o $(OBJ_DIR)/RexxIcon.o \
	$(OBJ_DIR)/RexxCmd.o : wbrexx.h ../Scalos_Helper.h

#############################################################

$(WBREXXNAME): $(WBREXXOBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(WBREXXOBJS) TO $@ LIB $(LDLIBS) $(LDFLAGS) STRIPDEBUG 

$(WBREXXNAME).debug: $(WBREXXOBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(WBREXXOBJS) TO $@ lib $(LDLIBS) $(LDFLAGS) ADDSYM

#############################################################

install:
	@printf '\033[32mInstall: \033[31m\033[1m$(WBREXXNAME).\033[0m\n'
	@copy $(WBREXXNAME) $(DESTPLUG) clone

#############################################################

clean:
	@printf '\033[32mCleanup: \033[31m\033[1m'
	-@delete $(OBJS) \
		$(WBREXXNAME) \
		$(WBREXXNAME).debug
	@printf '\033[0m'

#############################################################
