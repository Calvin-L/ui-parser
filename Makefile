
# input files
SOURCES=$(shell find src -iname '*.cpp')
PKG_CONFIG_PACKAGES=tesseract opencv

# tools
PDFLATEX=pdflatex -interaction batchmode

# misc
.PHONY: all clean
CXXFLAGS=-Os -Wall -pedantic -fwrapv -pipe
CXXFLAGS+=$(shell pkg-config --cflags $(PKG_CONFIG_PACKAGES))
LDFLAGS+=$(shell pkg-config --libs $(PKG_CONFIG_PACKAGES))

all: docs/proposal.pdf parse-layout

include $(SOURCES:.cpp=.d)

parse-layout: $(SOURCES:.cpp=.o)
	$(CXX) $(LDFLAGS) $^ -o $@

%.d: %.cpp
	$(CPP) -M $< -o $@

%.aux %.log %.pdf: %.tex
	cd $$(dirname $<) && $(PDFLATEX) $$(basename $<) && $(PDFLATEX) $$(basename $<)

clean:
	$(RM) docs/*.aux docs/*.pdf docs/*.log
	$(RM) parse-layout src/*.d src/*.o
