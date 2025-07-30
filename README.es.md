FRACTAL (versión 3.4.24)
=======================

:gb:[english](README.md) :es:[español](README.es.md)
:fr:[français](README.fr.md)

Un programa que usa crecimiento de fractales como banco de pruebas de
bibliotecas de paralelización y dibujo.

¡OJO!
-------

Este programa es un banco de pruebas en paralelización. **¡Usa intensamente la
CPU!**

AUTORES
-------

* Javier Burguete Tolosa
  [jburguete@eead.csic.es](mailto:jburguete@eead.csic.es).

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
* draw.c: fichero fuente de dibujo.
* simulator.h: fichero cabecera de interfaz.
* simulator.c: fichero fuente de interfaz.
* main.c: fichero fuente principal.
* logo.png: fichero de logo.
* logo2.png: fichero de logo.
* fractal.ico: fichero de icono.
* Doxyfile: fichero de configuración para generar documentación doxygen.
* po/es/LC\_MESSAGES/fractal.po: fichero de lenguaje español.
* po/fr/LC\_MESSAGES/fractal.po: fichero de lenguaje francés.
* tests/\*.xml: algunos ficheros de entrada para pruebas.
* TODO: fichero de tareas de desarrollo.

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

Además, para obtener las características de la máquina:
* [libgtop](https://github.com/GNOME/libgtop)

UTILIDADES OPCIONALES PARA CONSTRUIR EL MANUAL DE REFERENCIA
------------------------------------------------------------

* [doxygen](http://www.stack.nl/~dimitri/doxygen) formato de comentarios
  estándar para generar documentación.
* [latex](https://www.latex-project.org/) para construir los manuales PDF.

INSTRUCCIONES PARA CONSTRUCCIÓN Y EJECUCIÓN
-------------------------------------------

En sistemas Microsoft Windows deben instalarse
[MSYS2](http://sourceforge.net/projects/msys2) y las bibliotecas y utilidades
requeridas. Puede seguir instrucciones detalladas en
[install-unix](https://github.com/jburguete/install-unix/blob/master/tutorial.pdf)

En NetBSD 10.1, para compilar con la última versión del compilador GCC, hay que
hacer primero en el terminal:
> $ export PATH=/usr/pkg/gcc9/bin:$PATH"
Si se quiere hacer permanente tambien se puede añadir en el fichero ".profile"
de la carpeta raíz del usuario la línea:
> PATH="/usr/pkg/gcc9/bin:$PATH"

En OpenBSD 7.7 hay que hacer primero en el terminal de construcción:
> $ export AUTOCONF\_VERSION=2.69 AUTOMAKE\_VERSION=1.16

Descárguese este repositorio y ejecútese en un terminal:
> $ cd RUTA\_DE\_FRACTAL/3.4.24
>
> $ sh build.sh

Este script (build.sh) trata de construir el ejecutable con usando PGO (Profile
Guided Optimization) que falla en algunos sistemas. En ese caso hágase:
> $ cd RUTA\_DE\_FRACTAL/3.4.24
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

Por defecto el programa usa el widget GtkGLArea de la biblioteca GTK3. Éste
widget falla en máquinas virtuales (QEMU y Virtualbox). Para usar otras
bibliotecas gráficas hágase:
* FreeGLUT
> $ ./configure --with-freeglut
>
> $ make
* GLFW
> $ ./configure --with-glfw
>
> $ make
* SDL2
> $ ./configure --with-sdl
>
> $ make

El fichero Makefile usa características propias del programa GNU make. En
sistemas BSD, sustitúyase el comando **make** por **gmake**.

Opcionalmente para hacer una versión final:
> $ make strip

Para ejecutar el programa haga en el terminal:
> $ ./fractal

o, usando un fichero de entrada:
> $ ./fractal fichero\_de\_entrada

Algunos ficheros de entrada como ejemplo están en la carpeta **tests**.

Este programa ha sido construido y probado en los siguientes sistemas
operativos:
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

Probablemente, puede construirse en otros sistemas, distribuciones o versiones
pero no ha sido probado.

PARA HACER EL MANUAL DE INSTRUCCIONES (latex/refman.pdf file)
-------------------------------------------------------------

Ejecutar en un terminal:
> $ cd RUTA\_DE\_FRACTAL/3.4.24
>
> $ doxygen
>
> $ cd latex
>
> $ make
