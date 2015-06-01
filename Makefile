
# input files
SOURCES=$(shell find src -iname '*.cpp')
EXAMPLES=$(shell find examples -iname '*.png')
TEX=$(shell find docs -iname '*.tex')
PKG_CONFIG_PACKAGES=tesseract opencv lept

# tools
PDFLATEX=pdflatex -interaction nonstopmode

# misc
.PHONY: all test clean
CXXFLAGS=-Os -Wall -pedantic -fwrapv -pipe -std=c++11 -stdlib=libc++
CXXFLAGS+=$(shell pkg-config --cflags-only-I $(PKG_CONFIG_PACKAGES))
LDFLAGS=-stdlib=libc++
LDFLAGS+=$(shell pkg-config --libs $(PKG_CONFIG_PACKAGES))

all: $(TEX:.tex=.pdf) parse-layout

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

docs/progress-2015-04-15.pdf: docs/progress-2015-04-15.tex docs/progress-2015-04-15-screenshot.png
docs/progress-2015-04-29.pdf: docs/progress-2015-04-29.tex docs/progress-2015-04-29-screenshot.png

clean:
	$(RM) docs/*.aux docs/*.pdf docs/*.log
	$(RM) parse-layout src/*.d src/*.o
