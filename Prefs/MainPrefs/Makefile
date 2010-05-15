# makefile for Scalos main preferences
# $Date$
# $Revision$
# using SAS/C

#############################################################

SUBDIRMAKE 	= $(MAKE) -s -C 

#############################################################

.PHONY: clean install nodebug

#############################################################

SVNVERSION	= $(shell svnversion -n)
CC      	= sc
CFLAGS  	= nostkchk nochkabort dbg=f nover streq \
		  data=far code=far strmer stringsection=far \
		  memorysize=huge \
                  define SVN_VERSION=$(SVNVERSION) \
                  define SCALOSLOCALE=$(SCALOS_LOCALE) \
		  idlen=68 idir=//include \
                  idir=$(subst ../,/,$(FONTSAMPLEMCC_DIR)) \
                  idir=$(subst ../,/,$(DATATYPESMCC_DIR)) \
		  idir=$(subst ../,/,$(MCPGFX_DIR))

SPLAT		= sc:c/splat
AS      	= phxass
AFLAGS  	= quiet noexe m=68020 opt=NRQB i=sc:Assembler_Headers/
LD      	= slink
LDFLAGS 	= quiet batch noicons 
LDLIBS  	= LIB:mempools.lib \
		//SAS-lib/snprintf.lib \
		LIB:scm.lib \
		LIB:sc.lib \
		LIB:debug.lib \
		LIB:amiga.lib
CSTARTUP 	= LIB:c.o
OBJ_DIR		= .sasobj
CATCOMP 	= CatComp
FONTSAMPLEMCC_DIR  = ../../common/FontSampleMCC
DATATYPESMCC_DIR = ../../common/DataTypesMCC
MCPGFX_DIR	= ../../common/McpGfx
SCALOSINCDIR = ../../include/scalos

SCALOS_LOCALE	= $(OBJ_DIR)/ScalosPrefs_locale.h

#############################################################

CSRCS =						\
	Hooks.c 				\
	PrefsPlugins.c 				\
	ReadWritePrefs.c 			\
        HiddenDevices.c				\
	$(DATATYPESMCC_DIR)/DataTypesMCC.c 	\
	$(FONTSAMPLEMCC_DIR)/FontSampleMCC.c	\
	Images.c 				\
	McpFrameMCC.c				\
	SelectMarkSampleClass.c			\
	Scalos.c 				\
	PrefsVersion.c 				\
	$(MCPGFX_DIR)/ScalosMcpGfx.c		\

#############################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

XSRCS = $(notdir $(CSRCS))
OBJS = $(ASRCS:%.asm=$(OBJ_DIR)/%.o) $(XSRCS:%.c=$(OBJ_DIR)/$(notdir %).o)

#############################################################

NAME		= Scalos_Prefs
NAMEDBG		= $(NAME).debug
CAT_FILE	= Scalos/Scalos_Prefs.catalog
CATCOMPHEADER	= $(SCALOS_LOCALE)
DESTTOOL	= Scalos:Prefs/Scalos Prefs
DESTCAT		= Locale:Catalogs
CATS		= deutsch \
		  français

ALLCATS		= $(foreach cat,$(CATS),catalogs/$(cat)/$(CAT_FILE))

#############################################################

all:	$(NAME) $(NAMEDBG) allcatalogs
# install
# clean
# launch

#############################################################

# commands generated a catalog (.catalog) from a catalog translation (.ct) file.
.ct.catalog:
	$(CATCOMP) $*.cd $< CATALOG $*.catalog VB=1

$(OBJ_DIR)/DataTypesMCC.o : $(DATATYPESMCC_DIR)/DataTypesMCC.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/FontSampleMCC.o : $(FONTSAMPLEMCC_DIR)/FontSampleMCC.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/ScalosMcpGfx.o : $(MCPGFX_DIR)/ScalosMcpGfx.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $< objectname $@

#############################################################

$(NAME) : $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJS) TO $@ LIB $(LDLIBS) $(LDFLAGS) STRIPDEBUG 

$(NAMEDBG) : $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJS) TO $@ LIB $(LDLIBS) $(LDFLAGS) ADDSYM

#############################################################

$(OBJ_DIR)/DataTypesMCC.o: $(DATATYPESMCC_DIR)/DataTypesMCC.c \
	$(DATATYPESMCC_DIR)/DataTypesMCC.h $(SCALOSINCDIR)/scalos.h

$(OBJ_DIR)/McpFrameMCC.o : McpFrameMCC.c McpFrameMCC.h MCPFrame_mcc.h

$(OBJ_DIR)/SelectMarkSampleClass.o : SelectMarkSampleClass.c SelectMarkSampleClass.h

$(OBJ_DIR)/FontSampleMCC.o : $(FONTSAMPLEMCC_DIR)/FontSampleMCC.c \
	$(FONTSAMPLEMCC_DIR)/FontSampleMCC.h $(SCALOSINCDIR)/scalos.h

$(OBJ_DIR)/Hooks.o: Hooks.c Hooks.h General.h ScalosPrefs.h $(SCALOS_LOCALE) \
	PrefsPlugins.h HiddenDevices.h $(DATATYPESMCC_DIR)/DataTypesMCC.h \
        $(FONTSAMPLEMCC_DIR)/FontSampleMCC.h $(SCALOSINCDIR)/scalos.h

$(OBJ_DIR)/Images.o : Images.c Images.h General.h \
	$(DATATYPESMCC_DIR)/DataTypesMCC.h  ScalosPrefs.h $(SCALOSINCDIR)/scalos.h

$(OBJ_DIR)/PrefsPlugins.o : PrefsPlugins.c PrefsPlugins.h \
	ScalosPrefs.h  $(SCALOSINCDIR)/scalos.h

$(OBJ_DIR)/HiddenDevices.o : HiddenDevices.c HiddenDevices.h \
	ScalosPrefs.h  $(SCALOSINCDIR)/scalos.h

$(OBJ_DIR)/ReadWritePrefs.o: ReadWritePrefs.c ReadWritePrefs.h \
	ScalosPrefs.h HiddenDevices.h \
        $(FONTSAMPLEMCC_DIR)/FontSampleMCC.h  $(SCALOSINCDIR)/scalos.h

$(OBJ_DIR)/Scalos.o: Scalos.c General.h ScalosPrefs.h \
	ReadWritePrefs.h $(DATATYPESMCC_DIR)/DataTypesMCC.h \
	McpFrameMCC.h $(FONTSAMPLEMCC_DIR)/FontSampleMCC.h \
         $(SCALOSINCDIR)/scalos.h \
	HiddenDevices.h Hooks.h $(SCALOS_LOCALE) \
	SelectMarkSampleClass.h


$(OBJ_DIR)/PrefsVersion.o: PrefsVersion.c FORCE

FORCE:

#############################################################

$(CATCOMPHEADER) : Scalos_Prefs.cd
	@printf '\033[32mCatcomp header: \033[31m\033[1m$@ \033[32mfrom \033[31m$<\033[0m\n'
	@$(CATCOMP) $< cfile=$@ NOBLOCK NOCODE

#############################################################

# copy all generated file to their final destinations
install:
	@printf '\033[32mInstall: \033[31m\033[1m$(NAME)\033[0m\n'
	@copy $(NAME) '$(DESTTOOL)'
	@printf '\033[32mInstall: \033[31m\033[1m$(CAT_FILE)\033[0m\n'
	-@$(foreach cat,$(CATS),copy "catalogs/$(cat)/$(CAT_FILE)" "$(DESTCAT)/$(cat)/Scalos" clone;)

#############################################################

# make all Scalos .catalogs
allcatalogs:
	-@$(foreach cat,$(CATS),$(SUBDIRMAKE) catalogs/$(cat)/Scalos;)

#############################################################

# A little something to clean it all up
clean:
	@printf '\033[32mCleanup: \033[31m\033[1m'
	-@delete $(OBJS) $(NAME) $(NAMEDBG) $(CATCOMPHEADER) $(ALLCATS) 
	@printf '\033[0m'

#############################################################

nodebug:
	-@$(SPLAT) -s -o "d2(" "d1(" "#?.c"

#############################################################

