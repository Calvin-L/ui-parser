
# input files
SOURCES=$(shell find src -iname '*.cpp')
PACKAGES=tesseract opencv

# tools
PDFLATEX=pdflatex -interaction batchmode

# misc
.PHONY: all clean
CXXFLAGS=$(shell pkg-config --cflags $(PACKAGES)) -Os -Wall -pedantic -fwrapv
LDFLAGS=$(shell pkg-config --libs $(PACKAGES))

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
