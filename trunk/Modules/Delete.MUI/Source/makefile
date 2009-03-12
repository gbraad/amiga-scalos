# makefile for Scalos Delete.MUI module
# $Date$
# $Revision$
# $Id$

#############################################################

CSRCS 		= Delete.module.c \
		  Delete.module_files.c

#############################################################

CC 		= sc
LINK 		= slink
LIBS		= \
			///SAS-lib/snprintf.lib \
			LIB:sc.lib \
			LIB:debug.lib \
			LIB:amiga.lib

# Optimizer Flags
# Ignore Note 306: local function inlined: "InsertMH"
# Ignore Note 308: inline function does not use formal parameter ...
OPT_FLG 	= OPTIMIZE OPTINLOCAL OPTTIME OPTSCHED IGNORE=306 IGNORE=308 IGNORE=304
COPTS 		= NOWVRET NOVERSION nostackcheck idlen=128 idir=///include
OPTIONS 	=  $(OPT_FLG) DATA=FAR DEBUG=L $(COPTS) def=NDEBUG
DBFLAG 		= nodebug
CSTARTUP 	= LIB:c.o
CATCOMP		= catcomp
OBJ_DIR		= .sasobj
SUBDIRMAKE 	= $(MAKE) -s -C 

#############################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

OBJS = $(CSRCS:%.c=$(OBJ_DIR)/%.o)

#############################################################

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(OPTIONS) $*.c objectname $@

#############################################################

NAME 		= Delete.module
NAMEDBG		= $(NAME).debug
CAT_FILE	= Scalos/Delete.catalog
DESTCAT		= Locale:Catalogs
CATS		= deutsch \
		  français

ALLCATS		= $(foreach cat,$(CATS),Catalogs/$(cat)/$(CAT_FILE))
CATCOMPH	= Delete_Locale.h

#############################################################

All:	$(NAME) \
	allcatalogs

#############################################################

$(NAME): $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LINK) FROM $(CSTARTUP) $(OBJS) TO $@ lib $(LIBS) noicons batch quiet

$(OBJ_DIR)/Delete.module.o: Delete.module.c Delete.module.h $(CATCOMPH)

$(OBJ_DIR)/Delete.module_files.o: Delete.module_files.c Delete.module.h $(CATCOMPH)

#####################################################################

$(CATCOMPH) : Delete.cd
	@printf '\033[32mMake CatComp header: \033[31m\033[1m$(CATCOMPH) \033[32mfrom \033[31m$<\033[0m\n'
	@$(CATCOMP) $< cfile=$(CATCOMPH) XDEF

#############################################################

install:
	@printf '\033[32mInstall: \033[31m\033[1m$(NAME)\033[0m\n'
	@copy $(NAME) Scalos:Modules/
	@$(foreach cat,$(CATS),printf "\033[32mInstall catalog: \033[31m\033[1mcatalogs/$(cat)/$(CAT_FILE)\033[0m to \033[31m\033[1m$(DESTCAT)/$(cat)/Scalos\033[0m\n";)
	-@$(foreach cat,$(CATS),copy "catalogs/$(cat)/$(CAT_FILE)" "$(DESTCAT)/$(cat)/Scalos" clone;)

#############################################################

# make all Scalos .catalogs
allcatalogs:
	-@$(foreach cat,$(CATS),$(SUBDIRMAKE) Catalogs/$(cat)/Scalos;)

#############################################################

clean:
	@printf '\033[32mCleanup: \033[31m\033[1m'
	@delete $(NAME) $(OBJS)
	-@delete $(OBJS) $(NAME) $(NAMEDBG) $(CATCOMPH) $(ALLCATS) 
	@printf '\033[0m'

#############################################################

