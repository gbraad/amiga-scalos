# makefile f�r Scalos FileTypesPrefs.prefsplugin
# $Date$

#############################################################

SUBDIRMAKE 	= $(MAKE) -s -C 
CATCOMP 	= CatComp
AS		= phxAss
LD 		= slink
CC 		= sc
ECHO		= echo
LIBS 		= LIB:scm.lib \
		LIB:sc.lib \
		LIB:mempools.lib \
		///SAS-lib/snprintf.lib \
		LIB:debug.lib \
		LIB:amiga.lib
LDFLAGS		= quiet batch noicons 
PRECOMP 	= Include:all.gst
OBJ_DIR		= .sasobj
PREFSPATH	= ../../../Prefs/FileTypes
FONTSAMPLE_DIR	= ../../../common/FontSampleMCC
ICONOBJMCC_DIR	= ../../../common/IconobjectMCC
DATATYPESMCC_DIR = ../../../common/DataTypesMCC
COMMON_DIR	= ../../../common/Plugin

.SUFFIXES: .s .asm .plugin .plugin.debug

#############################################################

CSRCS 	= 	$(COMMON_DIR)/plugin-classic.c \
		FileTypesPrefs.c \
		ReadFtPrefs.c \
		WriteFtPrefs.c \
		DefIcons.c \
		DefIconsPrefs.c \
                DefaultDefIconsPrefs.c \
		$(FONTSAMPLE_DIR)/FontSampleMCC.c \
		$(ICONOBJMCC_DIR)/IconobjectMCC.c \
		$(DATATYPESMCC_DIR)/DataTypesMCC.c \

#############################################################

CFLAGS 		= optimize nostackcheck nover dbg=s \
		  data=FAR code=far \
		  idlen=64 idir=///include idir=///common/FontSampleMCC \
		  idir=$(subst ../,/,$(PREFSPATH)) \
		  idir=$(subst ../,/,$(ICONOBJMCC_DIR)) \
                  idir=$(subst ../,/,$(DATATYPESMCC_DIR)) \
		  idir=///common/Plugin
AFLAGS 		= QUIET DS opt=NRQB NOEXE linedebug I=SC:Assembler_Headers
#AFLAGS 	= -t -iINCLUDE:

#############################################################

PLUGIN		= FileTypes.prefsplugin
PLUGINDBG	= $(PLUGIN).debug
CAT_FILE	= Scalos/ScalosFileTypes.catalog
DESTTOOL	= Scalos:Prefs/
DESTCAT		= Locale:Catalogs
SRCCAT		= $(subst ../,/,$(PREFSPATH))
CATCOMPHEADER	= $(PREFSPATH)/ScalosFileTypes_locale.h
CATS		= deutsch \
		  fran�ais

ALLCATS		= $(foreach cat,$(CATS),$(SRCCAT)/catalogs/$(cat)/$(CAT_FILE))

#############################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

XSRCS = $(notdir $(CSRCS))
OBJS = $(ASRCS:%.asm=$(OBJ_DIR)/%.o) $(XSRCS:%.c=$(OBJ_DIR)/$(notdir %).o)

#############################################################

All:	$(PLUGIN) \
	$(PLUGINDBG) \
	allcatalogs
# install
# clean

#############################################################

$(OBJ_DIR)/plugin-classic.o : $(COMMON_DIR)/plugin-classic.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/DataTypesMCC.o : $(DATATYPESMCC_DIR)/DataTypesMCC.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/FontSampleMCC.o : $(FONTSAMPLE_DIR)/FontSampleMCC.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/IconobjectMCC.o : $(ICONOBJMCC_DIR)/IconobjectMCC.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $< objectname $@

#############################################################

$(OBJ_DIR)/plugin-classic.o : $(COMMON_DIR)/plugin-common.c \
	plugin_data.h $(COMMON_DIR)/plugin.h

$(OBJ_DIR)/FileTypesPrefs.o : FileTypesPrefs.h FileTypesPrefs_proto.h \
	$(ICONOBJMCC_DIR)/IconobjectMCC.h \
	FileTypesPrefsImage.h plugin_data.h $(COMMON_DIR)/plugin.h \
      	$(PREFSPATH)/ScalosFileTypes_locale.h debug.h

$(OBJ_DIR)/ReadFtPrefs.o : ReadFtPrefs.c FileTypesPrefs.h \
	FileTypesPrefs_proto.h debug.h

$(OBJ_DIR)/WriteFtPrefs.o : WriteFtPrefs.c FileTypesPrefs.h \
	FileTypesPrefs_proto.h debug.h

$(OBJ_DIR)/DefIcons.o : DefIcons.c FileTypesPrefs.h \
	FileTypesPrefs_proto.h debug.h

$(OBJ_DIR)/DefIconsPrefs.o : DefIconsPrefs.c FileTypesPrefs.h \
	FileTypesPrefs_proto.h debug.h \
        $(PREFSPATH)/ScalosFileTypes_locale.h

$(OBJ_DIR)/DefaultDefIconsPrefs.o : DefaultDefIconsPrefs.c

$(OBJ_DIR)/DataTypesMCC.o: $(DATATYPESMCC_DIR)/DataTypesMCC.c \
	$(DATATYPESMCC_DIR)/DataTypesMCC.h

$(OBJ_DIR)/FontSampleMCC.o : $(FONTSAMPLE_DIR)/FontSampleMCC.c \
	$(FONTSAMPLE_DIR)/FontSampleMCC.h

$(OBJ_DIR)/IconobjectMCC.o  : $(ICONOBJMCC_DIR)/IconobjectMCC.c \
	$(ICONOBJMCC_DIR)/IconobjectMCC.h debug.h

#############################################################

$(CATCOMPHEADER) : $(PREFSPATH)/ScalosFileTypes.cd
	@printf '\033[32mMake CatComp header: \033[31m\033[1m$@ \033[32mfrom \033[31m$<\033[0m\n'
	@$(CATCOMP) $(subst ../,/,$<) cfile=$(subst ../,/,$@)

$(PLUGIN) : $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) to $@ FROM $(OBJS) lib $(LIBS) $(LDFLAGS) STRIPDEBUG

$(PLUGINDBG) : $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) to $@ FROM $(OBJS) lib $(LIBS) $(LDFLAGS) ADDSYM

#############################################################

install:
	@avail flush
	@printf '\033[32mInstall: \033[31m\033[1m$(PLUGIN)\033[0m\n'
	@copy $(PLUGIN) "$(DESTTOOL)"
	@printf '\033[32mInstall: \033[31m\033[1m$(CAT_FILE)\033[0m\n'
	-@$(foreach cat,$(CATS),copy "$(SRCCAT)/catalogs/$(cat)/$(CAT_FILE)" "$(DESTCAT)/$(cat)/Scalos" clone;)

#############################################################

clean:
	@printf '\033[32mCleanup: \033[31m\033[1m'
	@delete $(PLUGIN) $(PLUGINDBG) $(OBJS) $(subst ../,/,$(CATCOMPHEADER)) $(ALLCATS)
	@printf '\033[0m'

#############################################################

# make all Scalos preferences .catalogs
allcatalogs:
	-@$(foreach cat,$(CATS),$(SUBDIRMAKE) $(PREFSPATH)/catalogs/$(cat)/Scalos;)


#############################################################
