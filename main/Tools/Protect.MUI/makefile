# ECX makefile for AOS 68k Protect module
# $Date: $
# $Revision: $
# $Id: $
#############################################################

CC      	= ECX:bin/ECX

CFLAGS  	= AMIGAOS VAR LINE SYM ICA

OPTFLAGS  	= EXENAME

#############################################################

NAME		= Protect.module.68k
DESTTOOL	= Scalos:Modules/Protect.module

#############################################################

SRC  		= Protect.module.e

MSR1		= GetIconObject.e
MOD1		= GetIconObject.m

MSR2		= Scalos_Protect_Locale.e
MOD2		= Scalos_Protect_Locale.m

MSR3		= VERSTAG.e
MOD3		= VERSTAG.m

MSR4		= getpath.e
MOD4		= getpath.m

MSR5		= protmodule.e
MOD5		= protmodule.m

#############################################################

All:	$(MOD1) \
 	$(MOD2) \
 	$(MOD3) \
 	$(MOD4) \
 	$(MOD5) \
	$(NAME)

#############################################################

$(MOD1):
	@printf '\033[32mCompile: \033[31m\033[1m$@\033[0m\n'
	@$(CC) $(MSR1) $(CFLAGS)

$(MOD2):
	@printf '\033[32mCompile: \033[31m\033[1m$@\033[0m\n'
	@$(CC) $(MSR2) $(CFLAGS)

$(MOD3):
	@printf '\033[32mCompile: \033[31m\033[1m$@\033[0m\n'
	@$(CC) $(MSR3) $(CFLAGS)

$(MOD4):
	@printf '\033[32mCompile: \033[31m\033[1m$@\033[0m\n'
	@$(CC) $(MSR4) $(CFLAGS)

$(MOD5):
	@printf '\033[32mCompile: \033[31m\033[1m$@\033[0m\n'
	@$(CC) $(MSR5) $(CFLAGS)

$(NAME):
	@printf '\033[32mCompile: \033[31m\033[1m$@\033[0m\n'
	@$(CC) $(SRC) $(CFLAGS) $(OPTFLAGS) $@

#############################################################

# A little something to clean it all up
clean:
	@printf '\033[32mDelete: \033[31m\033[1m'
	@delete $(MOD1) $(MOD2) $(MOD3) $(MOD4) $(MOD5) $(NAME)
	@printf '\033[0m'

#############################################################

# copy all generated file to their final destinations
install:
	@printf '\033[32mInstall: \033[31m\033[1m$(DESTTOOL)\033[0m\n'
	@copy $(NAME) $(DESTTOOL) clone

