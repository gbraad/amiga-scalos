# makefile for Scalos (translated Texts espa?ol)
# $Date$
# $Revision$

.SUFFIXES: .ct .catalog

all: scalos.catalog

scalos.catalog : scalos.ct ../../../scalos.cd
	@printf '\033[32mMake catalog: \033[31m\033[1mespa?ol \033[0m\n'
	CatComp ///scalos.cd scalos.ct CATALOG $@ VB=2
	@avail flush
