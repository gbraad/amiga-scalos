# makefile f?r Scalos (translated Texts : ?e?tina)
# 01 Jan 2004 12:43:59

.SUFFIXES: .ct .catalog

.ct.catalog :
	CatComp ///$*.cd $*.ct CATALOG $*.catalog VB=2
	avail flush

ScalosPalette.catalog : ScalosPalette.ct ../../../ScalosPalette.cd

All: ScalosPalette.catalog
