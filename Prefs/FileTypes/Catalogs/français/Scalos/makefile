# makefile for Scalos (translated Texts : fran?ais)
# $Date$

.SUFFIXES: .ct .catalog

.ct.catalog :
	@printf '\033[32mMake catalog: \033[31m\033[1mfran?ais\033[0m\n'
	CatComp ///$*.cd $*.ct CATALOG $*.catalog VB=2
	avail flush

ScalosFileTypes.catalog : ScalosFileTypes.ct ../../../ScalosFileTypes.cd

All: ScalosFileTypes.catalog 
