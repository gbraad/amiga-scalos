# MakeFile für Information MUI module
# $Date$
# $Revision$
# $Id$
#############################################################

SUBDIRMAKE 	= $(MAKE) -s -C 
CATCOMP 	= CatComp
CHEADERS	= 
CC		= sc
CFLAGS		= nostackcheck nochkabort debug=s data=far code=far \
		  strmer nover streq stringsection=far idlen=64 idir=sc:include/ \
		  idir=include: \
		  idir=//include \
		  idir=$(subst ../,/,$(TOOLTYPE_DIR)) \
		  idir=$(subst ../,/,$(ICONOBJMCC_DIR)) \
		  idir=$(subst ../,/,$(INT64_DIR))
SPLAT		= sc:c/splat
AS		= phxass
AFLAGS		= QUIET m=68020 linedebug opt=NRQB i=include:
LD		= slink
LNFLAGS		= quiet batch noicons stripdebug
LNDBFLAGS 	= quiet batch noicons addsym
LIBS		= LIB:sc.lib \
		//SAS-lib/snprintf.lib \
		LIB:mempools.lib \
		LIB:debug.lib \
		LIB:amiga.lib \

CSTARTUP	= LIB:c.o
OBJ_DIR		= .sasobj
TOOLTYPE_DIR	=  ../IconProperties.MUI
ICONOBJMCC_DIR	=  ../../common/IconobjectMCC
INT64_DIR	=  ../../common/Int64

.SUFFIXES: .asm .cd

#############################################################

NAME		= Information.module
DBGNAME		= $(NAME).debug
CATCOMPHEADER	= Information_Locale.h
DESTTOOL	= Scalos:modules/
CAT_FILE	= Scalos/Information.catalog
DESTCAT		= Locale:Catalogs
ALLCATS 	= Catalogs/deutsch/$(CAT_FILE) \
		  Catalogs/français/$(CAT_FILE)

#############################################################

all:	$(NAME) \
	$(DBGNAME) \
	allcatalogs
# install
# clean

#############################################################

# make all Scalos .catalogs
allcatalogs:
	@$(SUBDIRMAKE) Catalogs/deutsch/Scalos
	@$(SUBDIRMAKE) Catalogs/français/Scalos

#############################################################

CSRCS =		Information.c \
		$(ICONOBJMCC_DIR)/IconobjectMCC.c \
		$(TOOLTYPE_DIR)/ToolTypes.c \
		$(INT64_DIR)/Dos64.c \
		$(INT64_DIR)/int64.c	\

#############################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

#############################################################

XSRCS = $(notdir $(CSRCS))
OBJS = $(ASRCS:%.asm=$(OBJ_DIR)/%.o) $(XSRCS:%.c=$(OBJ_DIR)/$(notdir %).o)

#############################################################

$(CATCOMPHEADER) : Information.cd
	@printf '\033[32mMake Catcomp header: \033[31m\033[1m$@ \033[32mfrom \033[31m$<\033[0m\n'
	@$(CATCOMP) $< cfile=$@

#############################################################

$(OBJ_DIR)/Information.o : Information.c Information.h \
	Information_Locale.h $(ICONOBJMCC_DIR)/IconobjectMCC.h \
	$(TOOLTYPE_DIR)/ToolTypes.h $(INT64_DIR)/int64.h debug.h

$(OBJ_DIR)/IconobjectMCC.o  : $(ICONOBJMCC_DIR)/IconobjectMCC.c \
	$(ICONOBJMCC_DIR)/IconobjectMCC.h debug.h

$(OBJ_DIR)/ToolTypes.o      : $(TOOLTYPE_DIR)/ToolTypes.c \
	$(TOOLTYPE_DIR)/ToolTypes.h debug.h

#############################################################

$(NAME): $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJS) TO $@ LIB $(LIBS) $(LNFLAGS)

$(DBGNAME): $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJS) TO $@ LIB $(LIBS) $(LNDBFLAGS)

#############################################################

# commands generated a catalog (.catalog) from a catalog translation (.ct) file.
.ct.catalog:
	$(CATCOMP) $*.cd $< CATALOG $*.catalog VB=1

$(OBJ_DIR)/IconobjectMCC.o  : $(ICONOBJMCC_DIR)/IconobjectMCC.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/ToolTypes.o      : $(TOOLTYPE_DIR)/ToolTypes.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/Dos64.o      : $(INT64_DIR)/Dos64.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/int64.o      : $(INT64_DIR)/int64.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $(subst ../,/,$<) objectname $@

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $< objectname $@

$(OBJ_DIR)/%.o : %.s
	@printf '\033[32mAssemble: \033[31m\033[1m$<\033[0m\n'
	@$(AS) $(AFLAGS) $< to $@

$(OBJ_DIR)/%.o : %.asm
	@printf '\033[32mAssemble: \033[31m\033[1m$<\033[0m\n'
	@$(AS) $(AFLAGS) $< to $@

#############################################################

install:
	@printf '\033[32mInstall: \033[31m\033[1m$(NAME) \033[0mto \033[1m$(DESTTOOL) \033[0m\n'
	@copy $(NAME) $(DESTTOOL) clone
	@printf '\033[32mFlushing memory\033[0m\n'
	@avail flush
	@printf '\033[32mInstall: \033[31m\033[1m$(CAT_FILE)\033[0m\n'
	-@copy 'catalogs/deutsch/$(CAT_FILE)'  	'$(DESTCAT)/Deutsch/Scalos/' clone
	-@copy 'catalogs/français/$(CAT_FILE)' 	'$(DESTCAT)/français/Scalos/' clone

#############################################################

clean:
	@printf '\033[32mCleanup: \033[31m\033[1m'
	@delete $(OBJS) $(NAME) $(DBGNAME) $(ALLCATS) $(CATCOMPHEADER)
	@printf '\033[0m'

#############################################################

nodebug:
	-@$(SPLAT) -s -o "d2(" "d1(" "#?.c"

#############################################################
