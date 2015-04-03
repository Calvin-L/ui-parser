
PDFLATEX=pdflatex -interaction batchmode
.PHONY: all clean

all: proposal.pdf

%.aux %.log %.pdf: %.tex
	$(PDFLATEX) proposal
	$(PDFLATEX) proposal

clean:
	rm *.aux *.pdf *.log
