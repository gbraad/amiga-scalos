# makefile for Scalos Menu preferences
# $Date$
# $Revision$
# using SAS/C

#############################################################

SUBDIRMAKE 	= $(MAKE) -s -C 

#############################################################

.PHONY: clean install nodebug

#############################################################

CC      	= sc
CFLAGS  	= nostkchk nochkabort strmer dbg=f nover streq data=far \
		  idlen=64 idir=//include
AS      	= phxass
AFLAGS  	= noexe m=68020 opt=NRQB i=sc:Assembler_Headers/
LD      	= slink
LDFLAGS 	= quiet batch noicons sd
LDLIBS  	= LIB:mempools.lib LIB:scm.lib LIB:sc.lib LIB:debug.lib LIB:amiga.lib
ASTARTUP 	=
CSTARTUP 	= LIB:c.o
OBJ_DIR		= .sasobj
CATCOMP 	= CatComp

#############################################################

CSRCS =	Menu.c

#############################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

OBJS = $(ASRCS:%.asm=$(OBJ_DIR)/%.o) $(CSRCS:%.c=$(OBJ_DIR)/%.o)

#############################################################

NAME		= Scalos_Menu
NAMEDBG		= $(NAME).debug
DESTTOOL	= Scalos:Prefs/Scalos Menu
CAT_FILE	= Scalos/ScalosMenu.catalog
CATCOMPHEADER	= ScalosMenu_locale.h
DESTCAT		= Locale:Catalogs
CATS		= dansk \
		  deutsch \
		  español \
		  français \
		  italiano \
		  svenska \
		  ÃeÓtina

ALLCATS		= $(foreach cat,$(CATS),catalogs/$(cat)/$(CAT_FILE))

#############################################################

# The default target (all) and what to really build
all:	$(NAME) $(NAMEDBG) \
	allcatalogs
# install
# clean
# launch

#############################################################

$(OBJ_DIR)/Menu.o : Menu.c ScalosMenu_locale.h

$(CATCOMPHEADER) : ScalosMenu.cd
	@printf '\033[32mMake CatComp header: \033[31m\033[1m$@ \033[32mfrom \033[31m$<\033[0m\n'
	@$(CATCOMP) $< cfile=$@

#############################################################

# CLI command used when linking the final executables
$(NAME) : $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJS) TO $@ LIB $(LDLIBS) $(LDFLAGS) STRIPDEBUG 

$(NAMEDBG) : $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJS) TO $@ LIB $(LDLIBS) $(LDFLAGS) ADDSYM

#############################################################

# commands generated a catalog (.catalog) from a catalog translation (.ct) file.
.ct.catalog:
	$(CATCOMP) $*.cd $< CATALOG $*.catalog VB=1

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $*.c objectname $@

$(OBJ_DIR)/%.o : %.s
	@printf '\033[32mAssemble: \033[31m\033[1m$<\033[0m\n'
	@$(AS) $(AFLAGS) $< to $@

$(OBJ_DIR)/%.o : %.asm
	@printf '\033[32mAssemble: \033[31m\033[1m$<\033[0m\n'
	@$(AS) $(AFLAGS) $< to $@

#############################################################

# copy all generated file to their final destinations
install:
	@printf '\033[32mInstall: \033[31m\033[1m$(NAME)\033[0m\n'
	@copy $(NAME) "$(DESTTOOL)"
	@printf '\033[32mInstall: \033[31m\033[1m$(CAT_FILE)\033[0m\n'
	-@$(foreach cat,$(CATS),copy "catalogs/$(cat)/$(CAT_FILE)" "$(DESTCAT)/$(cat)/Scalos" clone;)

#############################################################

# make all Scalos preferences .catalogs
allcatalogs:
	-@$(foreach cat,$(CATS),$(SUBDIRMAKE) catalogs/$(cat)/Scalos;)

#############################################################

# A little something to clean it all up
clean:
	@printf '\033[32mCleanup: \033[31m\033[1m'
	-@delete $(NAME) $(NAMEDBG) $(OBJS) $(CATCOMPHEADER) $(ALLCATS)
	@printf '\033[0m'

#############################################################

nodebug:
	-@$(SPLAT) -s -o "d2(" "d1(" "#?.c"

#############################################################
