# User interface parser

## Compile from source

You will need the following:

 - opencv (>= 2.4.11)
 - tesseract (>= 3.03rc1_3)
 - leptonica (>= 1.71)
 - pkg-config (with package files for the aforementioned libraries)
 - a C++11 capable compiler

If you are building on a non-OSX machine, you may need to adjust the `CXXFLAGS`
and `LDFLAGS` macros in the Makefile.

To build:

    $ make

## Running

Examples live in the "examples" folder. You can build HTML files for them using

    $ make test

or manually using

    $ ./parse-layout <input-image>
