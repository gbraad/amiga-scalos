# makefile f?r Scalos (translated Texts : ?e?tina)
# 01 Jan 2004 12:38:57

.SUFFIXES: .ct .catalog

.ct.catalog :
	CatComp ///$*.cd $*.ct CATALOG $*.catalog VB=2
	avail flush

ScalosMenu.catalog : ScalosMenu.ct ../../../ScalosMenu.cd

All: ScalosMenu.catalog 
