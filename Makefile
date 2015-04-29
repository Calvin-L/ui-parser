
# input files
SOURCES=$(shell find src -iname '*.cpp')
TEX=$(shell find docs -iname '*.tex')
PKG_CONFIG_PACKAGES=tesseract opencv lept

# tools
PDFLATEX=pdflatex -interaction nonstopmode

# misc
.PHONY: all clean
CXXFLAGS=-Os -Wall -pedantic -fwrapv -pipe -std=c++11 -stdlib=libc++
CXXFLAGS+=$(shell pkg-config --cflags-only-I $(PKG_CONFIG_PACKAGES))
LDFLAGS=-stdlib=libc++
LDFLAGS+=$(shell pkg-config --libs $(PKG_CONFIG_PACKAGES))

all: $(TEX:.tex=.pdf) parse-layout

include $(SOURCES:.cpp=.d)

parse-layout: $(SOURCES:.cpp=.o)
	$(CXX) $(LDFLAGS) $^ -o $@

%.d: %.cpp
	$(CPP) -M -MP -MT $(<:.cpp=.o) $< | sed -E 's_^([^:]+):_\1 $(@:.o=.d):_' >$@

%.aux %.log %.pdf: %.tex
	cd $$(dirname $<) && $(PDFLATEX) $$(basename $<) && $(PDFLATEX) $$(basename $<)

clean:
	$(RM) docs/*.aux docs/*.pdf docs/*.log
	$(RM) parse-layout src/*.d src/*.o
