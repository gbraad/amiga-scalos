# makefile for Delete.module (translated Texts : fran?ais)
# $Date$
# $Revision$

.SUFFIXES: .ct .catalog

.ct.catalog :
	@printf '\033[32mMake catalog: \033[31m\033[1mfran?ais\033[0m\n'
	CatComp ///$*.cd $*.ct CATALOG $*.catalog VB=2
	avail flush

Delete.catalog : Delete.ct ../../../Delete.cd

All: Delete.catalog
