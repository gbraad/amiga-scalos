# MakeFile f?r Exchange MUI module
# $Date$

#####################################################################

TOPLEVEL	= /
SUBDIRMAKE 	= $(MAKE) -s -C 
CATCOMP 	= CatComp
FLEXCAT		= FlexCat
CHEADERS	= 
CC		= sc
CFLAGS		= optimize nostackcheck nochkabort debug=s NOWVRET \
		  strmer nover streq idlen=128 IGNORE=217 \
		  idir=sc:include/ \
                  define SCALOSLOCALE=$(SCALOS_LOCALE) \
		  idir=include: idir=//include
AFLAGS		= QUIET m=68020 linedebug opt=NRQB i=include:
LD		= slink
LNFLAGS		= quiet batch noicons stripdebug
LNDBFLAGS 	= quiet batch noicons addsym
LIBS		= LIB:sc.lib \
		//SAS-lib/snprintf.lib \
		LIB:mempools.lib \
                LIB:debug.lib \
                LIB:amiga.lib
CSTARTUP	= LIB:c.o
OBJDIR		= .sasobj

SCALOS_LOCALE	= $(OBJDIR)/Exchange_Locale.h

.SUFFIXES: .cd

#############################################################

NAME		= .bin_os3/Exchange.module
DBGNAME		= $(NAME).debug
CATCOMPHEADER	= $(SCALOS_LOCALE)
DESTTOOL	= Scalos:modules/
CAT_FILE	= Scalos/Exchange.catalog
DESTCAT		= Locale:Catalogs
ALLCATS 	= Catalogs/deutsch/$(CAT_FILE) \
		  Catalogs/fran?ais/$(CAT_FILE)

#############################################################

all:	$(NAME) \
	$(DBGNAME) \
	allcatalogs
# install
# clean

#####################################################################

# make all Scalos .catalogs
allcatalogs:
	@$(SUBDIRMAKE) Catalogs/deutsch/Scalos
	@$(SUBDIRMAKE) Catalogs/fran?ais/Scalos

#####################################################################

CSRCS =		Exchange.c \

#####################################################################

$(OBJDIR)::
	@[ -d $(OBJDIR) ] || mkdir $(OBJDIR) > /dev/null 2>&1

#############################################################

XSRCS = $(notdir $(CSRCS))
OBJS = $(ASRCS:%.asm=$(OBJDIR)/%.o) $(XSRCS:%.c=$(OBJDIR)/$(notdir %).o)

#####################################################################

$(CATCOMPHEADER) : Exchange.cd
	@printf '\033[32mMake Catcomp header: \033[31m\033[1m$@ \033[32mfrom \033[31m$<\033[0m\n'
	@$(FLEXCAT) $< $@=$(TOPLEVEL)/CatComp_h.sd

#####################################################################

$(OBJDIR)/Exchange.o : Exchange.c Exchange.h \
	$(SCALOS_LOCALE) \
        cx_private.h \
        cx_private_protos.h \
        cx_private_pragmas.h \
        debug.h

#####################################################################

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

$(OBJDIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $< objectname $@

$(OBJDIR)/%.o : %.s
	@printf '\033[32mAssemble: \033[31m\033[1m$<\033[0m\n'
	@$(AS) $(AFLAGS) $< to $@

$(OBJDIR)/%.o : %.asm
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
	-@copy 'catalogs/fran?ais/$(CAT_FILE)' 	'$(DESTCAT)/fran?ais/Scalos/' clone

#############################################################

clean:
	@printf '\033[32mCleanup: \033[31m\033[1m'
	-@delete $(OBJS) $(NAME) $(DBGNAME) $(ALLCATS) $(CATCOMPHEADER)
	@printf '\033[0m'

#############################################################
