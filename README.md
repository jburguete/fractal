FRACTAL (3.4.24 version)
=======================

:gb:[english](README.md) :es:[español](README.es.md)
:fr:[français](README.fr.md)

A program using growing fractals to benchmark parallelization and drawing
libraries.

WARNING!
--------

This program is a benchmark on parallelization. **It uses intensively the CPU!**

AUTHORS
-------

* Javier Burguete Tolosa
  [jburguete@eead.csic.es](mailto:jburguete@eead.csic.es).

FILES
-----

* build.sh: simplified building script.
* configure.ac: configure generator.
* Makefile.in: Makefile generator.
* config.h: config header file.
* fractal.h: fractal calculations header file.
* fractal.c: fractal calculations source file.
* image.h: image drawing header file.
* image.c: image drawing source file.
* text.h: text drawing header file.
* text.c: text drawing source file.
* graphic.h: graphic drawing header file.
* graphic.c: graphic drawing source file.
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
* tests/\*.xml: several test input files.
* TODO: development tasks file.

TOOLS AND LIBRARIES REQUIRED TO BUILD THE EXECUTABLE
----------------------------------------------------

* [gcc](https://gcc.gnu.org) or [clang](http://clang.llvm.org) to compile the
  source code
* [make](http://www.gnu.org/software/make) to build the executable file
* [autoconf](http://www.gnu.org/software/autoconf) to generate the Makefile in
  different operative systems
* [automake](http://www.gnu.org/software/automake) to check the operative
  system
* [pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config) to find the
  libraries to compile
* [gsl](http://www.gnu.org/software/gsl) to generate pseudo-random numbers
* [gettext](http://www.gnu.org/software/gettext) to work with different
  international codes and languages
* [libxml](http://xmlsoft.org) to read input files
* [glib](https://developer.gnome.org/glib) extended utilities of C to work with
  data, lists, mapped files, regular expressions, using multicores in shared
  memory machines, ...
* [png](http://www.libpng.org) to work with PNG files
* [glew](http://glew.sourceforge.net) to use advanced OpenGL functions to draw
  graphics
* [gtk3](http://www.gtk.org) to create the interactive GUI tool

OPTIONAL LIBRARIES TO BUILD THE EXECUTABLE
------------------------------------------

The following libraries can be used as alternative to the GtkGLArea widget of
the GTK library to interact with OpenGL to draw graphics:
* [freeglut](http://freeglut.sourceforge.net)
* [sdl2](https://www.libsdl.org)
* [glfw](http://www.glfw.org)

Moreover,  to get the processors characteristics:
* [libgtop](https://github.com/GNOME/libgtop)

OPTIONAL TOOLS TO BUILD THE REFERENCE MANUAL
--------------------------------------------

* [doxygen](http://www.stack.nl/~dimitri/doxygen) (standard comments format to
generate documentation)
* [latex](https://www.latex-project.org/) (to build the PDF manuals)

BUILDING AND EXECUTING INSTRUCTIONS
-----------------------------------

On Microsoft Windows systems you have to install
[MSYS2](http://sourceforge.net/projects/msys2) and the required
libraries and utilities. You can follow detailed instructions in
[install-unix](https://github.com/jburguete/install-unix/blob/master/tutorial.pdf)

On NetBSD 10.1, to compile with last GCC version, you have to do first on the
building terminal:
> $ export PATH=/usr/pkg/gcc9/bin:$PATH"
To do permanent this change the following line can be added to the ".profile"
file in the user root directory:
> PATH="/usr/pkg/gcc9/bin:$PATH"

On OpenBSD 7.7 you have to do first on the building terminal:
> $ export AUTOCONF\_VERSION=2.69 AUTOMAKE\_VERSION=1.16

Download this repository and execute on a terminal:
> $ cd PATH\_TO\_FRACTAL/3.4.24
>
> $ sh build.sh

Optionally to make a final version:
> $ make strip

To execute the program do on a terminal:
> $ ./fractal

or, if used an input file:
> $ ./fractal input\_file

Several input file examples are in the **tests** directory.

This software has been built and tested in the following operative systems:
* Alpine Linux
* Arch Linux
* Debian 12 (Linux)
* Devuan Linux 5
* Dragonfly BSD 6.4.1
* Fedora Linux 42
* FreeBSD 14.2
* Gentoo Linux
* Linux Mint DE 6
* MacOS Ventura + Homebrew
* Manjaro Linux
* Microsoft Windows 11 + MSYS2
* NetBSD 10.1
* OpenBSD 7.7
* OpenInidiana Hipster
* OpenSUSE Linux Leap 15.5
* Ubuntu Linux 25.04

Probably, it can be built in other systems, distributions or versions but it has
not been tested.

MAKING MANUAL INSTRUCTIONS (latex/refman.pdf file)
--------------------------------------------------

Execute on a terminal:
> $ cd PATH\_TO\_FRACTAL/3.4.24
>
> $ doxygen
>
> $ cd latex
>
> $ make
