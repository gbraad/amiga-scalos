# makefile für Scalos PopupMenu.prefsplugin
# $Date$
# §Revision:                 $

#############################################################

SUBDIRMAKE 	= $(MAKE) -s -C 
CATCOMP 	= CatComp
AS 		= phxAss
LD		= slink
CC 		= sc
ECHO		= echo
LIBS 		= LIB:sc.lib LIB:debug.lib LIB:mempools.lib LIB:amiga.lib
LDFLAGS		= quiet batch noicons 
PRECOMP 	= Include:all.gst
OBJ_DIR		= .sasobj
DATATYPESMCC_DIR = ../../../common/DataTypesMCC
COMMON_DIR	= ../../../common/Plugin

.SUFFIXES: .asm

#############################################################

CSRCS = 	$(COMMON_DIR)/plugin-classic.c \
		PopupMenuPrefs.c \
                FrameButtonMCC.c \
		$(DATATYPESMCC_DIR)/DataTypesMCC.c \

#############################################################

PLUGIN 		= PopupMenu.prefsplugin
PLUGINDBG	= $(PLUGIN).debug
CAT_FILE	= Scalos/ScalosPopupMenu.catalog
DESTTOOL	= Scalos:Prefs/
DESTCAT		= Locale:Catalogs
CATCOMPHEADER	= PopupMenu_locale.h

CATS		= deutsch \
		  français \


ALLCATS		= $(foreach cat,$(CATS),Catalogs/$(cat)/$(CAT_FILE))

#############################################################

CFLAGS 		= optimize nostackcheck nover dbg=s DATA=far \
		  idlen=64 \
		  idir=///include \
                  idir=$(subst ../,/,$(DATATYPESMCC_DIR)) \
		  idir=///common/Plugin

#############################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

XSRCS = $(notdir $(CSRCS))
OBJS = $(ASRCS:%.asm=$(OBJ_DIR)/%.o) $(XSRCS:%.c=$(OBJ_DIR)/$(notdir %).o)

#############################################################

All:	$(PLUGIN) $(PLUGINDBG) \
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

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $*.c objectname $@

#############################################################

$(OBJ_DIR)/plugin-classic.o : $(COMMON_DIR)/plugin-common.c \
	plugin_data.h $(COMMON_DIR)/plugin.h

$(OBJ_DIR)/FrameButtonMCC.o : FrameButtonMCC.c FrameButtonMCC.h \
	PopupMenuPrefs.h pmprefs.h

$(OBJ_DIR)/DataTypesMCC.o: $(DATATYPESMCC_DIR)/DataTypesMCC.c \
	$(DATATYPESMCC_DIR)/DataTypesMCC.h

$(OBJ_DIR)/PopupMenuPrefs.o : PopupMenuPrefs.h PopupMenuPrefsImage.h \
	plugin_data.h $(COMMON_DIR)/plugin.h PopupMenu_locale.h \
        pmprefs.h FrameButtonMCC.h

#############################################################

$(CATCOMPHEADER) : ScalosPopupMenu.cd
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
	@copy $(PLUGIN) $(DESTTOOL)
	@printf '\033[32mInstall: \033[31m\033[1m$(CAT_FILE)\033[0m\n'
	-@$(foreach cat,$(CATS),copy "Catalogs/$(cat)/$(CAT_FILE)" "$(DESTCAT)/$(cat)/Scalos" clone;)

#############################################################

clean:
	@printf '\033[32mCleanup: \033[31m\033[1m'
	-@delete $(PLUGIN) $(PLUGINDBG) $(OBJS) $(subst ../,/,$(CATCOMPHEADER)) $(ALLCATS)
	@printf '\033[0m'

#############################################################

# make all Scalos preferences .catalogs
allcatalogs:
	-@$(foreach cat,$(CATS),$(SUBDIRMAKE) Catalogs/$(cat)/Scalos;)

#############################################################

