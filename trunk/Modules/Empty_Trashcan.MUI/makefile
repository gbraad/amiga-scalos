# Makefile for Empty_Trashcan.module (MUI)
# using GNU make and SAS/C
# $Date$
# $Revision$

#####################################################################

SUBDIRMAKE 	= $(MAKE) -s -C 
CHEADERS	= 
CC		= sc
CFLAGS		= optimize nostackcheck nochkabort debug=s \
		  idlen=128 strmer nover streq data=far \
		  ignore=217 idir=sc:include/ \
		  idir=include: idir=//include
AS		= phxass
AFLAGS		= QUIET m=68020 linedebug opt=NRQB i=include:
LD		= slink
LNFLAGS		= quiet batch noicons stripdebug
LNDBFLAGS 	= quiet batch noicons addsym
LIBS		= LIB:sc.lib LIB:debug.lib LIB:amiga.lib
CATCOMP		= catcomp
CSTARTUP	= LIB:c.o
OBJ_DIR		= .sasobj

#####################################################################

.SUFFIXES: .asm

#####################################################################

NAME		= Empty_Trashcan.module
DBGNAME		= $(NAME).debug
CATCOMPH	= Empty_Trashcan_Locale.h
CAT_FILE	= Scalos/Empty_Trashcan.catalog
DESTCAT		= Locale:Catalogs
ALLCATS 	= Catalogs/deutsch/$(CAT_FILE) \
		  Catalogs/français/$(CAT_FILE)

#####################################################################

all:	$(NAME) \
	$(DBGNAME) \
	allcatalogs
# install
# clean

#####################################################################

# make all Scalos .catalogs
allcatalogs:
	@$(SUBDIRMAKE) Catalogs/deutsch/Scalos
	@$(SUBDIRMAKE) Catalogs/français/Scalos

#####################################################################

CSRCS =		Empty_Trashcan.c

#####################################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

#####################################################################

XOBJS = $(ASRCS:%.asm=$(OBJ_DIR)/%.o) $(CSRCS:%.c=$(OBJ_DIR)/%.o)
OBJS = $(XOBJS)

#####################################################################

$(NAME): $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJS) TO $@ LIB $(LIBS) $(LNFLAGS)

$(DBGNAME): $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJS) TO $@ LIB $(LIBS) $(LNDBFLAGS)

#####################################################################

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $< objectname $@

$(OBJ_DIR)/%.o : %.asm
	@printf '\033[32mAssemble: \033[31m\033[1m$<\033[0m\n'
	@$(AS) $(AFLAGS) $< to $@

#####################################################################

$(CATCOMPH) : Empty_Trashcan.cd
	@printf '\033[32mMake CatComp header: \033[31m\033[1m$(CATCOMPH) \033[32mfrom \033[31m$<\033[0m\n'
	@$(CATCOMP) $< cfile=$(CATCOMPH) XDEF

$(OBJ_DIR)/Empty_Trashcan.o : Empty_Trashcan.c Empty_Trashcan.h $(CATCOMPH)

#####################################################################

$(NAME) $(DBGNAME) : $(OBJS)

#####################################################################

install:
	@printf '\033[32mInstall: \033[31m\033[1m$(NAME) \033[0mto  \033[31m\033[1mScalos:modules/ \033[0m\n'
	@copy $(NAME) Scalos:modules/
	@avail flush
	@printf '\033[32mInstall: \033[31m\033[1m$(CAT_FILE)\033[0m\n'
	-@copy "catalogs/deutsch/$(CAT_FILE)"  	"$(DESTCAT)/Deutsch/Scalos/" clone
	-@copy "catalogs/français/$(CAT_FILE)" 	"$(DESTCAT)/français/Scalos/" clone

#####################################################################

# A little something to clean it all up
clean:
	@printf '\033[32mCleanup: \033[31m\033[1m'
	@delete $(XOBJS) $(NAME) $(DBGNAME) \
	  $(CATCOMPH)
	@printf '\033[0m'

#####################################################################
