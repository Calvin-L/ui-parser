
# input files
SOURCES=$(shell find src -iname '*.cpp')
EXAMPLES=$(shell find examples -iname '*.png')
TEX=$(shell find docs -iname '*.tex')
PKG_CONFIG_PACKAGES=tesseract opencv lept

# tools
PDFLATEX=pdflatex -interaction nonstopmode

# misc
.PHONY: all test doc clean
CXXFLAGS=-Os -Wall -pedantic -fwrapv -pipe -std=c++11 -stdlib=libc++
CXXFLAGS+=$(shell pkg-config --cflags-only-I $(PKG_CONFIG_PACKAGES))
LDFLAGS=-stdlib=libc++
LDFLAGS+=$(shell pkg-config --libs $(PKG_CONFIG_PACKAGES))

all: parse-layout

docs: $(TEX:.tex=.pdf)

test: $(EXAMPLES:.png=.html)

include $(SOURCES:.cpp=.d)

parse-layout: $(SOURCES:.cpp=.o)
	$(CXX) $(LDFLAGS) $^ -o $@

%.d: %.cpp
	$(CPP) -M -MP -MT $(<:.cpp=.o) $< | sed -E 's_^([^:]+):_\1 $(@:.o=.d):_' >$@

%.html: %.png parse-layout
	./parse-layout --no-debug $< >$@

%.aux %.log %.pdf: %.tex
	cd $$(dirname $<) && $(PDFLATEX) $$(basename $<) && $(PDFLATEX) $$(basename $<)

docs/writeup.bbl: docs/writeup.aux docs/bibliography.bib
	cd docs && bibtex writeup

docs/nips13submit_e.sty:
	curl -fL 'http://media.nips.cc/Conferences/2013/Styles/nips13submit_e.sty' -o $@

docs/progress-2015-04-15.pdf: docs/progress-2015-04-15.tex docs/progress-2015-04-15-screenshot.png
docs/progress-2015-04-29.pdf: docs/progress-2015-04-29.tex docs/progress-2015-04-29-screenshot.png
docs/progress-2015-05-13.pdf: docs/progress-2015-05-13.tex docs/progress-2015-05-13-screenshot.png
docs/progress-2015-06-01.pdf: docs/progress-2015-06-01.tex docs/progress-2015-06-01-screenshot.png
docs/writeup.aux: docs/writeup.tex docs/nips13submit_e.sty docs/benchmark.tex
docs/writeup.pdf: docs/writeup.tex docs/nips13submit_e.sty docs/writeup.bbl docs/benchmark.tex
docs/benchmark.tex: parse-layout $(EXAMPLES) eval/benchmark.sh
	./eval/benchmark.sh './parse-layout --no-debug' $(EXAMPLES) >benchmark.out
	mv benchmark.out docs/benchmark.tex

clean:
	$(RM) *.out docs/*.{aux,log,pdf,out,bbl,blg}
	$(RM) docs/benchmark.tex
	$(RM) parse-layout src/*.d src/*.o
