# makefile for GetHidden (translated Texts : fran?ais)
# $Date: 2009-02-17 21:22:13 +0200 (Mar, 17 Fév 2009) $

.SUFFIXES: .ct .catalog

.ct.catalog :
	@printf '\033[32mMake catalog: \033[31m\033[1mfran?ais\033[0m\n'
	CatComp ///$*.cd $*.ct CATALOG $*.catalog VB=2
	avail flush

GetHidden.catalog : GetHidden.ct ../../../GetHidden.cd

All: GetHidden.catalog
