FRACTAL
=======

A program using drawing fractals to benchmark parallelization and drawing
libraries.

WARNING
-------

This program is a benchmark on parallelization. It uses intensively the CPU.

REQUIRED
--------

* autoconf (to generate the Makefile in different systems)
* gettext (to work with different locales)
* gthreads (to use multicores in shared memory machines)
* glib (extended utilities of C to work with data, lists, random numbers, ...)
* png (to work with PNG files)
* gtk+ (to make the interface windows)
* freeglut (interaction with OpenGL to draw graphics)
* doxygen (standard comments format to generate documentation)

FILES
-----

* configure.ac: configure generator.
* Makefile.in: Makefile generator.
* config.h: config header file.
* fractal.h: fractal calculations header file.
* fractal.c: fractal calculations source file.
* draw.h: drawing header file.
* draw.c: drawing source file.
* simulator.h: interface header file.
* simulator.c: interface source file.
* main.c: main source file.
* Doxyfile: configuration file to generate doxygen documentation.
* es/LC_MESSAGES/fractal.po: spanish language file.
* fr/LC_MESSAGES/fractal.po: french language file.

BUILDING INSTRUCTIONS
---------------------

aclocal
autoconf
./configure
make

MAKING MANUAL INSTRUCTIONS
--------------------------

doxygen
cd latex
make 
