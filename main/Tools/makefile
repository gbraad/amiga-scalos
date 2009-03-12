# makefile for Scalos Tools
# $Date$
# $Revision$
# $Id$
# using GNU make and SAS/C


CHEADERS 	= scalos_structures.h variables.h functions.h debug.h SemaphoreDebug.h wrappers.h \
		  ../../include/scalos/scalos.h ../../include/scalosdebug.h
CC      	= sc
CFLAGS  	= nostkchk nochkabort strcons strmer opt dbg=f nover streq data=far \
		  idir=//include \
		  idir=$(subst ../,/,$(INT64_DIR)) \
                  idlen=64 \
                  ERROR=87,88 \
                  IGNORE=306,316
#define DEBUG_SEMAPHORES=1
LD      	= slink
LNFLAGS 	= quiet noicons batch stripdebug
LNDBFLAGS 	= quiet noicons batch addsym
LDLIBS  	= LIB:scnb.lib LIB:debug.lib LIB:amiga.lib
CSTARTUP 	= LIB:c.o
ECHO		= echo
OBJ_DIR		= .sasobj
INT64_DIR	= ../../common/Int64

#############################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

#############################################################

CTRL	 = ScalosCtrl
CTRLDBG	 = $(CTRL).debug
SEMA	 = ScalosSema
SEMADBG  = $(SEMA).debug
LWB	 = LoadWB.Scalos
LWBDBG	 = $(LWB).debug
GENMSGID = GenMsgIdNames

#############################################################

# The default target (all) and what to really build
all: 	$(CTRL) \
	$(CTRLDBG) \
	$(SEMA) \
	$(SEMADBG) \
	$(LWB) \
	$(LWBDBG) \
	$(GENMSGID)
# install
# clean

#############################################################

# List of all the object files that go together to make up the final executable
# Make sure that startup.o is first in the list as this contains our own startup
# code (which must be used, as we link without startup code for shared objects)
OBJS		= $(OBJ_DIR)/ScalosCtrl.o
OBJSSEMA	= $(OBJ_DIR)/ScalosSema.o
OBJSLWB		= $(OBJ_DIR)/LoadWB.o
OBJSGENMSGID	= $(OBJ_DIR)/GenMsgIdNames.o

#############################################################

# CLI command used when linking GenMsgIdNames
$(GENMSGID) : $(OBJSGENMSGID)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJSGENMSGID) TO $@ LIB $(LDLIBS) $(LNFLAGS)

# CLI command used when linking ScalosCtrl
$(CTRL) : $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJS) TO $@ LIB $(LDLIBS) $(LNFLAGS)

# CLI command used when linking ScalosCtrl.debug
$(CTRLDBG) : $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJS) TO $@ LIB $(LDLIBS) $(LNDBFLAGS)

# CLI command used when linking ScalosSema
$(SEMA) : $(OBJSSEMA)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJSSEMA) TO $@ LIB $(LDLIBS) $(LNFLAGS)

# CLI command used when linking ScalosSema.debug
$(SEMADBG) : $(OBJSSEMA)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJSSEMA) TO $@ LIB $(LDLIBS) $(LNDBFLAGS)

# CLI command used when linking LoadWB
$(LWB) : $(OBJSLWB)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJSLWB) TO $@ LIB $(LDLIBS) $(LNFLAGS)

$(LWBDBG) : $(OBJSLWB)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(CSTARTUP) $(OBJSLWB) TO $@ LIB $(LDLIBS) $(LNDBFLAGS)

#############################################################

# CLI command used when compiling. This is generic and will be called for all the
# dependancies (C source files) that need compiling.
$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $*.c objectname $@

#############################################################

ScalosSema.o : ScalosSema.c ../debug.h ../SemaphoreDebug.h ../scalos_structures.h

#############################################################
#############################################################

install:
	@printf '\033[32mInstall: \033[31m\033[1m$(CTRL) \033[0mto \033[1mScalos:Tools/ \033[0m\n'
	@copy $(CTRL) Scalos:Tools/
	@printf '\033[32mInstall: \033[31m\033[1m$(LWB) \033[0mas \033[1mc:LoadWB \033[0m\n'
	@copy $(LWB) c:LoadWB

#############################################################

# A little something to clean it all up
clean:
	@printf '\033[32mCleanup: \033[31m\033[1m'
	@delete $(OBJS) $(OBJSSEMA) $(OBJSLWB) $(OBJSGENMSGID) \
		 $(CTRL) $(CTRLDBG) $(LWB) $(LWBDBG) $(SEMA) $(SEMADBG) $(GENMSGID)
	@printf '\033[0m'

#############################################################
#############################################################
