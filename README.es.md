FRACTAL (versión 3.4.10)
=======================

:gb:[english](README.md) :es:[español](README.es.md)

Un programa que usa crecimiento de fractales como banco de pruebas de
bibliotecas de paralelización y dibujo.

¡OJO!
-------

Este programa es un banco de pruebas en paralelización. ¡Usa intensamente la
CPU!

AUTORES
-------

* Javier Burguete Tolosa
  [jburguete@eead.csic.es](mailto:jburguete@eead.csic.es).

UTILIDADES Y BIBLIOTECAS REQUERIDAS PARA CONSTRUIR EL EJECUTABLE
----------------------------------------------------------------

* [gcc](https://gcc.gnu.org) o [clang](http://clang.llvm.org) para compilar el
  código fuente.
* [make](http://www.gnu.org/software/make) para construir el fichero ejecutable.
* [autoconf](http://www.gnu.org/software/autoconf) para generar el Makefile en
  diferentes sistemas operativos.
* [automake](http://www.gnu.org/software/automake) para detectar el sistema
  operativo.
* [pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config) para
  encontrar las bibliotecas para compilar.
* [gsl](http://www.gnu.org/software/gsl) para generar números pseudo-aleatorios.
* [gettext](http://www.gnu.org/software/gettext) para trabajar con diferentes
  códigos y lenguajes internacionales.
* [libxml](http://xmlsoft.org) para leer los ficheros de entrada
* [libgtop](https://github.com/GNOME/libgtop) para obtener las características
  de la máquina
* [glib](https://developer.gnome.org/glib) utilidades extendidas de C para
  trabajar con datos, listas, ficheros mapeados, expresiones regulares, usar
  multiprocesador en máquinas de memoria compartida, ...
* [png](http://libpng.sourceforge.net) para trabajar con ficheros PNG.
* [gtk3](http://www.gtk.org) para trabajar con ventanas interactivas.
* [glew](http://glew.sourceforge.net) funciones OpenGL de alto nivel.

BIBLIOTECAS OPCIONALES PARA CONSTRUIR EL EJECUTABLE
---------------------------------------------------

Pueden usarse como alternaviva al widget GtkGLArea de la biblioteca GTK para
interactuar con OpenGL para dibujar las gráficas las bibliotecas siguientes:
* [freeglut](http://freeglut.sourceforge.net)
* [sdl2](https://www.libsdl.org)
* [glfw](http://www.glfw.org)

UTILIDADES OPCIONALES PARA CONSTRUIR EL MANUAL DE REFERENCIA
------------------------------------------------------------

* [doxygen](http://www.stack.nl/~dimitri/doxygen) formato de comentarios
  estándar para generar documentación.
* [latex](https://www.latex-project.org/) para construir los manuales PDF.

FICHEROS
--------

* build.sh: script de construcción simplificado.
* configure.ac: generador de configuración.
* Makefile.in: generador del Makefile.
* config.h: fichero cabecera de configuración.
* fractal.h: fichero cabecera de cálculos fractales.
* fractal.c: fichero fuente de cálculos fractales.
* image.h: fichero cabecera de dibujo de imágenes.
* image.c: image drawing source file.
* text.h: fichero cabecera de dibujo de texto.
* text.c: text drawing source file.
* graphic.h: fichero cabecera de dibujo de gráficos.
* graphic.c: graphic drawing source file.
* draw.h: fichero cabecera de dibujo.
* draw.c: drawing source file.
* simulator.h: fichero cabecera de interfaz.
* simulator.c: interface source file.
* main.c: fichero fuente principal.
* logo.png: fichero de logo.
* logo2.png: fichero de logo.
* fractal.ico: fichero de icono.
* Doxyfile: fichero de configuración para generar documentación doxygen.
* po/es/LC\_MESSAGES/fractal.po: fihcero de lenguaje español.
* po/fr/LC\_MESSAGES/fractal.po: fichero de lenguaje francés.
* tests/\*.xml: algunos ficheros de entrada para pruebas.
* TODO: fichero de tareas de desarrollo.

INSTRUCCIONES PARA CONSTRUCCIÓN Y EJECUCIÓN
-------------------------------------------

On Microsoft Windows systems you have to install
[MSYS2](http://sourceforge.net/projects/msys2) and the required
libraries and utilities. You can follow detailed instructions in
[install-unix](https://github.com/jburguete/install-unix/blob/master/tutorial.pdf)

On NetBSD 9.0, to compile with last GCC version you have to do first on the
building terminal:
> $ export PATH=/usr/pkg/gcc9/bin:$PATH"

On OpenBSD 6.7 you have to do first on the building terminal:
> $ export AUTOCONF\_VERSION=2.69 AUTOMAKE\_VERSION=1.16

This software has been built and tested in the following operative systems:
* Debian 9 (Linux, kFreeBSD and Hurd)
* DragonFly BSD 5.2
* Dyson Illumos
* Fedora Linux 29
* FreeBSD 12.1
* Linux Mint DE 3
* Manjaro Linux
* Microsoft Windows 10
* NetBSD 9.0
* OpenBSD 6.7
* OpenInidiana Hipster
* OpenSUSE Linux Leap 15
* Ubuntu Mate Linux 18.04
* Xubuntu Linux 18.10

Descárguese este repositorio y ejecutar en un terminal:
> $ cd RUTA\_DE\_FRACTAL/3.4.10
>
> $ sh build.sh

Opcionalmente para hacer una versión final:
> $ make strip

Probablemente, puede construirse en otros sistemas, distribuciones o versiones
pero no ha sido probado.

Para ejecutar el programa haga en el terminal:
> $ ./fractal

o, usando un fichero de entrada:
> $ ./fractal fichero\_de\_entrada

Algunos ficheros de entrada como ejemplo están en la carpeta **tests**.

PARA HACER EL MANUAL DE INSTRUCCIONES (latex/refman.pdf file)
-------------------------------------------------------------

Ejecutar en un terminal:
> $ cd RUTA\_DE\_FRACTAL/3.4.10
>
> $ doxygen
>
> $ cd latex
>
> $ make
