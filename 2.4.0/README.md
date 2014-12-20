FRACTAL (2.4.0 version)
=======================

A program growing fractals to benchmark parallelization and drawing libraries.

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
* doxygen (optional: standard comments format to generate documentation)
* latex (optional: to build the PDF reference manual)

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
* logo.png: logo file.
* logo2.png: logo file.
* Doxyfile: configuration file to generate doxygen documentation.
* po/es/LC_MESSAGES/fractal.po: spanish language file.
* po/fr/LC_MESSAGES/fractal.po: french language file.

BUILDING INSTRUCTIONS
---------------------

On Microsoft Windows systems you have to install
[MSYS/MinGW](http://www.mingw.org) and the required libraries and utilities. You
can follow detailed instructions in
[MinGW-64-Make](https://github.com/jburguete/MinGW-64-Make)

On OpenBSD 5.6 you have to do first on the building terminal:
> $ export AUTOCONF_VERSION=2.69 AUTOMAKE_VERSION=1.14

This software has been built and tested in the following operative systems:

Debian Linux 7.7
________________
Debian kFreeBSD 7.7
___________________
Debian Hurd 7.7
_______________
DragonFly BSD 4.0.1
___________________
FreeBSD 10.0
____________
Microsoft Windows 7 32 bits
___________________________
Microsoft Windows XP 32 bits
____________________________
NetBSD 6.1.5 (with modular xorg)
________________________________
OpenBSD 5.6
___________

Download this repository and execute on a terminal:
> $ cd PATH_TO_FRACTAL/2.4.0
>
> $ aclocal
>
> $ autoconf
>
> $ automake --add-missing
>
> $ ./configure
>
> $ make
>
> $ strip fractal (optional: to make a final version)

Microsoft Windows 7 64 bits
___________________________
Microsoft Windows XP 64 bits
____________________________

Download this repository and execute on a MSYS terminal:
> $ cd PATH_TO_FRACTAL/2.4.0
>
> $ cd 2.4.0
>
> $ aclocal
>
> $ autoconf
>
> $ automake --add-missing
>
> $ configure --host=x86_64-w64-mingw32
>
> $ make
>
> $ strip fractal.exe (optional: to make a final version)

MAKING MANUAL INSTRUCTIONS (latex/refman.pdf file)
--------------------------------------------------

Execute on a terminal:
> $ cd PATH_TO_FRACTAL/2.4.0
>
> $ doxygen
>
> $ cd latex
>
> $ make
