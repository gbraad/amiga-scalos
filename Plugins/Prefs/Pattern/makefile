# makefile f�r Scalos Pattern.prefsplugin
# $Date$
# $Revision$
# $Id$
#
#############################################################

SUBDIRMAKE 	= $(MAKE) -s -C 
CATCOMP 	= CatComp
LD 		= slink
CC 		= sc
SPLAT		= sc:c/splat

LIBS 		= LIB:scm.lib LIB:sc.lib LIB:mempools.lib \
		  LIB:debug.lib LIB:amiga.lib
LDFLAGS		= quiet batch noicons 
PRECOMP 	= Include:all.gst
ECHO		= echo
OBJ_DIR		= .sasobj

COMMON_DIR		= ../../../common/Plugin
BITMAPMCC_DIR		= ../../../common/BitMapMCC
BACKFILLMCC_DIR		= ../../../common/BackfillMCC
DATATYPESMCC_DIR	= ../../../common/DataTypesMCC

# search ScalosPattern.cd for Prefs/Pattern/catalogs/ 
PREFSPATH	= ../../../Prefs/Pattern

.SUFFIXES: .s .asm .plugin .plugin.debug

#############################################################

CSRCS = 	$(COMMON_DIR)/plugin-classic.c \
		PatternPrefs.c \
                $(BACKFILLMCC_DIR)/Backfill.c \
		$(BITMAPMCC_DIR)/BitMapMCC.c \
                $(DATATYPESMCC_DIR)/DataTypesMCC.c \
		DoLoadDT.c

#############################################################

.PHONY: clean install nodebug

#############################################################

CFLAGS 		= optimize nostackcheck nover dbg=s DATA=far \
		  idlen=64 ignore=306 \
		  idir=///include \
		  idir=$(subst ../,/,$(PREFSPATH)) \
		  idir=$(subst ../,/,$(BACKFILLMCC_DIR)) \
                  idir=$(subst ../,/,$(BITMAPMCC_DIR)) \
                  idir=$(subst ../,/,$(DATATYPESMCC_DIR)) \
		  idir=///common/Plugin
LNFLAGS 	= quiet batch noicons stripdebug SD
LNDBFLAGS 	= quiet batch noicons addsym SD

#############################################################

PLUGIN		= Pattern.prefsplugin
PLUGINDBG	= $(PLUGIN).debug
CAT_FILE	= Scalos/ScalosPattern.catalog
DESTTOOL	= Scalos:Prefs/
DESTCAT		= Locale:Catalogs
SRCCAT		= $(subst ../,/,$(PREFSPATH))
CATCOMPHEADER	= ScalosPattern_locale.h

CATS		= dansk \
		  deutsch \
		  espa�ol \
		  fran�ais \
		  italiano \
		  svenska \
		  �e�tina

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

$(OBJ_DIR)/BitMapMCC.o  : $(BITMAPMCC_DIR)/BitMapMCC.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/Backfill.o  : $(BACKFILLMCC_DIR)/Backfill.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/DataTypesMCC.o  : $(DATATYPESMCC_DIR)/DataTypesMCC.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $*.c objectname $@

#############################################################

$(OBJ_DIR)/plugin-classic.o : $(COMMON_DIR)/plugin-common.c \
	plugin_data.h $(COMMON_DIR)/plugin.h

$(OBJ_DIR)/BitMapMCC.o  : $(BITMAPMCC_DIR)/BitMapMCC.c \
	$(BITMAPMCC_DIR)/BitMapMCC.h debug.h

$(OBJ_DIR)/Backfill.o : $(BACKFILLMCC_DIR)/Backfill.c \
	$(BACKFILLMCC_DIR)/Backfill.h $(BITMAPMCC_DIR)/BitMapMCC.h

$(OBJ_DIR)/DataTypesMCC.o : $(DATATYPESMCC_DIR)/DataTypesMCC.h

$(OBJ_DIR)/PatternPrefs.o : PatternPrefs.h \
		PatternPrefsImage.h \
		plugin_data.h \
                $(COMMON_DIR)/plugin.h \
                $(BACKFILLMCC_DIR)/Backfill.h \
        	$(BITMAPMCC_DIR)/BitMapMCC.h \
                $(DATATYPESMCC_DIR)/DataTypesMCC.h \
                $(CATCOMPHEADER)

#############################################################

PatternPrefs.c : ScalosPattern_locale.h

$(CATCOMPHEADER) : $(PREFSPATH)/ScalosPattern.cd
	@printf '\033[32mMake CatComp header: \033[31m\033[1m$@ \033[32mfrom \033[31m$<\033[0m\n'
	@catcomp $(subst ../,/,$<) cfile=$(subst ../,/,$@)

##############################################################################

$(PLUGIN) : $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) to $@ FROM $(OBJS) lib $(LIBS) $(LDFLAGS) STRIPDEBUG

$(PLUGINDBG) : $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) to $@ FROM $(OBJS) lib $(LIBS) $(LDFLAGS) ADDSYM

#############################################################

# copy all generated files to their final destinations
install:
	@avail flush
	@printf '\033[32mInstall: \033[31m\033[1m$(PLUGIN)\033[0m\n'
	@copy $(PLUGIN) "$(DESTTOOL)"
	@printf '\033[32mInstall: \033[31m\033[1m$(CAT_FILE)\033[0m\n'
	-@$(foreach cat,$(CATS),copy "$(SRCCAT)/catalogs/$(cat)/$(CAT_FILE)" "$(DESTCAT)/$(cat)/Scalos" clone;)

#############################################################

clean:
	@printf '\033[32mCleanup: \033[31m\033[1m'
	-@delete $(PLUGIN) $(PLUGINDBG) $(OBJS) $(subst ../,/,$(CATCOMPHEADER)) $(ALLCATS)
	@printf '\033[0m'

#############################################################

# make all Scalos preferences .catalogs
allcatalogs:
	-@$(foreach cat,$(CATS),$(SUBDIRMAKE) $(PREFSPATH)/catalogs/$(cat)/Scalos;)

#############################################################

nodebug:
	-@$(SPLAT) -s -o "d2(" "d1(" "#?.c"

#############################################################
