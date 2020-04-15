FRACTAL (3.4.3 version)
=======================

A program growing fractals to benchmark parallelization and drawing libraries.

WARNING
-------

This program is a benchmark on parallelization. It uses intensively the CPU.

TOOLS AND LIBRARIES REQUIRED TO BUILD THE EXECUTABLE
----------------------------------------------------

* [gcc](https://gcc.gnu.org) or [clang](http://clang.llvm.org) (to compile the
source code)
* [make](http://www.gnu.org/software/make) (to build the executable file)
* [autoconf](http://www.gnu.org/software/autoconf) (to generate the Makefile in
different operative systems)
* [automake](http://www.gnu.org/software/automake) (to check the operative
system)
* [pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config) (to find the
libraries to compile)
* [gsl](http://www.gnu.org/software/gsl) (to generate pseudo-random numbers)
* [gettext](http://www.gnu.org/software/gettext) (to work with different
locales)
* [glib](https://developer.gnome.org/glib) (extended utilities of C to work with
data, lists, mapped files, regular expressions, using multicores in shared
memory machines, ...)
* [gtk+3](http://www.gtk.org) (to create the interactive GUI tool)
* [glew](http://glew.sourceforge.net) (to use advanced OpenGL functions to draw
graphics)
* [freeglut](http://freeglut.sourceforge.net), [sdl2](https://www.libsdl.org) or
[glfw](http://www.glfw.org) (interaction with OpenGL to draw graphics)
* [png](http://www.libpng.org) (to work with PNG files)

OPTIONAL TOOLS TO BUILD THE REFERENCE MANUAL
--------------------------------------------

* [doxygen](http://www.stack.nl/~dimitri/doxygen) (standard comments format to
generate documentation)
* [latex](https://www.latex-project.org/) (to build the PDF manuals)

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
* fractal.ico: icon file.
* Doxyfile: configuration file to generate doxygen documentation.
* po/es/LC\_MESSAGES/fractal.po: spanish language file.
* po/fr/LC\_MESSAGES/fractal.po: french language file.

BUILDING INSTRUCTIONS
---------------------

On Microsoft Windows systems you have to install
[MSYS2](http://sourceforge.net/projects/msys2) and the required
libraries and utilities. You can follow detailed instructions in
[install-unix](https://github.com/jburguete/install-unix/blob/master/tutorial.pdf)

On NetBSD 8.0, to compile with last GCC version you have to do first on the
building terminal:
> $ export PATH=/usr/pkg/gcc8/bin:$PATH"

On OpenBSD 6.4 you have to do first on the building terminal:
> $ export AUTOCONF\_VERSION=2.69 AUTOMAKE\_VERSION=1.16

This software has been built and tested in the following operative systems:
* Debian 9 (Linux, kFreeBSD and Hurd)
* DragonFly BSD 5.2
* Dyson Illumos
* Fedora Linux 29
* FreeBSD 11.2
* Linux Mint DE 3
* Manjaro Linux
* Microsoft Windows 7
* Microsoft Windows 10
* NetBSD 8.0 (from source with modular xorg)
* OpenBSD 6.4
* OpenInidiana Hipster
* OpenSUSE Linux Leap 15
* Ubuntu Mate Linux 18.04
* Xubuntu Linux 18.10

Download this repository and execute on a terminal:
> $ cd PATH\_TO\_FRACTAL/3.4.3
>
> $ sh build.sh
>
> $ strip fractal (optional: to make a final version)

Probably, it can be built in other systems, distributions, or versions but it
has not been tested.

MAKING MANUAL INSTRUCTIONS (latex/refman.pdf file)
--------------------------------------------------

Execute on a terminal:
> $ cd PATH\_TO\_FRACTAL/3.4.3
>
> $ doxygen
>
> $ cd latex
>
> $ make
