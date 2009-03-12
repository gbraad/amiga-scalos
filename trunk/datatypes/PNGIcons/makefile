# makefile for PNG icon datatype
# using GNU make and SAS/C
# $Date$
# $Revision$
# $Id$
#####################################################################

ASRCS =
CSRCS =		PNGIconDt-classic.c \
		PNGIconDt.c \
		LoadPngIcon.c \
		GetPNGPicture.c \
		SavePngIcon.c \

#####################################################################

AS		= phxass
SPLAT		= sc:c/splat
LD		= slink
CC		= sc
LIBS		= 	//SAS-lib/png.lib \
			//SAS-lib/z.lib \
			LIB:mempools.lib \
			LIB:scm.lib \
			LIB:sc.lib \
			LIB:debug.lib \
			LIB:amiga.lib
PRECOMP 	= Include:all.gst
OBJ_DIR		= .sasobj

.SUFFIXES: .asm

#############################################################

.PHONY: clean install nodebug

#####################################################################

CFLAGS		= optimize nostackcheck nochkabort strcons debug=s \
		  strmer nover streq idir=sc:include/ idir=include: idir=//include
AFLAGS		= QUIET m=68020 linedebug opt=NRQB i=include:
LNFLAGS		= quiet batch noicons stripdebug
LNDBFLAGS 	= quiet batch noicons addsym
CSTARTUP	=

#####################################################################

OBJS = $(ASRCS:%.asm=$(OBJ_DIR)/%.o) $(CSRCS:%.c=$(OBJ_DIR)/%.o)

#####################################################################

DTNAME		= pngiconobject.datatype
DTDBGNAME	= $(DTNAME).debug

#####################################################################

all:	$(DTNAME) \
	$(DTDBGNAME)

#####################################################################

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

#####################################################################

$(OBJ_DIR)/%.o : %.c
	@printf '\033[32mCompile: \033[31m\033[1m$<\033[0m\n'
	@$(CC) $(CFLAGS) $< objectname $@

$(OBJ_DIR)/%.o : %.s
	@printf '\033[32mAssemble: \033[31m\033[1m$<\033[0m\n'
	@$(AS) $(AFLAGS) $< to $@

$(OBJ_DIR)/%.o : %.asm
	@printf '\033[32mAssemble: \033[31m\033[1m$<\033[0m\n'
	@$(AS) $(AFLAGS) $< to $@

#####################################################################

$(DTNAME): $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(OBJS) TO $(DTNAME) LIB $(LIBS) $(LNFLAGS)

$(DTDBGNAME): $(OBJS)
	@printf '\033[32mLink: \033[31m\033[1m$@\033[0m\n'
	@$(LD) FROM $(OBJS) TO $(DTDBGNAME) LIB $(LIBS) $(LNDBFLAGS)

#####################################################################

$(OBJ_DIR)/PNGIconDt-classic.o : PNGIconDt.c PngIconDt.h PNGIconProto.h
$(OBJ_DIR)/PNGIconDt.o : PNGIconDt.c PngIconDt.h PNGIconProto.h
$(OBJ_DIR)/LoadPngIcon.o : LoadPngIcon.c PngIconDt.h PNGIconProto.h
$(OBJ_DIR)/SavePngIcon.o : SavePngIcon.c PngIconDt.h PNGIconProto.h

#####################################################################

install:
	@printf '\033[32mFlushing memory...\033[0m\n'
	avail flush
	@printf '\033[32mInstall: \033[31m\033[1m$(DTNAME)\033[0m\n'
	copy $(DTNAME) sys:classes/datatypes

#####################################################################

clean:
	@printf '\033[32mCleanup...\033[0m\n'
	-@delete $(OBJS) $(DTNAME) $(DTNAME).debug

#############################################################

nodebug:
	-@$(SPLAT) -s -o "d2(" "d1(" "#?.c"

#############################################################

