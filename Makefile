
PDFLATEX=pdflatex -interaction batchmode
.PHONY: all clean

all: docs/proposal.pdf

%.aux %.log %.pdf: %.tex
	cd $$(dirname $<) && $(PDFLATEX) $$(basename $<) && $(PDFLATEX) $$(basename $<)

clean:
	$(RM) docs/*.aux docs/*.pdf docs/*.log
