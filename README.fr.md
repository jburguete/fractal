FRACTAL (versión 3.4.20)
=======================

:gb:[english](README.md) :es:[español](README.es.md)
:fr:[français](README.fr.md)

Un programme qu'utilise croissance de fractals comme banc d'essais de
bibliothèques de parallèlization et dessin.

¡ATTENTION!
-------

Ce programme est un banc d'essais en parallèlization. **L'usage de la CPU est
intense!**

AUTEURS
-------

* Javier Burguete Tolosa
  [jburguete@eead.csic.es](mailto:jburguete@eead.csic.es).

FICHIERS
--------

* build.sh: script de construction par défaut.
* configure.ac: pour configurer la construction du Makefile.
* Makefile.in: modèle pour générer le Makefile.
* config.h: fichier d'en tête de configuration.
* fractal.h: fichier d'en tête de calculs fractals.
* fractal.c: fichier source de calculs fractals.
* image.h: fichier d'en tête de dessin d'images.
* image.c: fichier source de dessin d'images.
* text.h: fichier d'en tête de dessin de texte.
* text.c: fichier source de dessin de texte.
* graphic.h: fichier d'en tête de dessin de graphiques.
* graphic.c: fichier source de dessin de graphiques.
* draw.h: fichier d'en tête de dessin.
* draw.c: fichier source de dessin.
* simulator.h: fichier d'en tête d'interface.
* simulator.c: fichier source d'interface.
* main.c: fichier source principal.
* logo.png: fichier logo.
* logo2.png: fichier logo.
* fractal.ico: fichier d'icône.
* Doxyfile: fichier de configuration pour générer documentation doxygen.
* po/es/LC\_MESSAGES/fractal.po: fichier de langue espagnole.
* po/fr/LC\_MESSAGES/fractal.po: fichier de langue française.
* tests/\*.xml: quelques fichiers d'entrée pour tests.
* TODO: fichier de tâches pour developpement.

OUTILS ET BIBLIOTHÈQUES REQUIS POUR CONSTRUIRE L'EXÉCUTABLE
-----------------------------------------------------------

* [gcc](https://gcc.gnu.org) ou [clang](http://clang.llvm.org) pour compiler le
  code source.
* [make](http://www.gnu.org/software/make) pour construire le fichier
  executable.
* [autoconf](http://www.gnu.org/software/autoconf) pour générer le Makefile en
  differents systèmes d'exploitation.
* [automake](http://www.gnu.org/software/automake) pour détecter le système
  d'exploitation.
* [pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config) pour trouver
  les bibliothèques pour compiler.
* [glib](https://developer.gnome.org/glib) outils étendues de C pour travailler
  avec données, listes, fichiers mappés, expressions régulières, utiliser
* [gsl](http://www.gnu.org/software/gsl) pour générer nombres pseudo-aléatoires.
* [gettext](http://www.gnu.org/software/gettext) pour travailler avec differents
  codes et langues internationaux.
* [libxml](http://xmlsoft.org) pour travailler avec fichiers XML.
* [png](http://libpng.sourceforge.net) pour travailler avec fichiers PNG.
* [gtk3](http://www.gtk.org) pour travailler avec fenêtres interactives.
* [glew](http://glew.sourceforge.net) functions OpenGL de haut niveau.

BIBLIOTHÈQUES EN OPTION POUR CONSTRUIRE L'EXÉCUTABLE
----------------------------------------------------

Les bibliothèques suivantes peuvent être utilisées alternativement au widget
GtkGLArea de la bibliothèque GTK pour interagir avec OpenGL pour dessiner
les graphiques:
* [freeglut](http://freeglut.sourceforge.net)
* [sdl2](https://www.libsdl.org)
* [glfw](http://www.glfw.org)

En plus, pour obtenir les characteristiques des processeurs:
* [libgtop](https://github.com/GNOME/libgtop)

OUTILS EN OPTION POUR CONSTRUIR LE MANUEL DE RÉFÉRENCE
------------------------------------------------------

* [doxygen](http://www.stack.nl/~dimitri/doxygen) format de commentaires
  standard pour générer documentation.
* [latex](https://www.latex-project.org/) pour construire les manuels PDF.

INSTRUCTIONS POUR CONSTRUCTION ET EXÉCUTION
-------------------------------------------

Avec systèmes Microsoft Windows on doit installer
[MSYS2](http://sourceforge.net/projects/msys2) et en plus les bibliothèques et
les outils requis. On peut suivre les instructions détaillées dans le
didacticiel
[install-unix](https://github.com/jburguete/install-unix/blob/master/tutorial.pdf).

Avec NetBSD 9.3, pour utiliser le derniére version du compilateur GCC, il faut
faire avant dans le terminal:
> $ export PATH="/usr/pkg/gcc9/bin:$PATH"
Pour faire le changement permanent on peut ajouter à la dernière ligne di
fichier ".profile" dans le répertoire racine du utilisateur:
> PATH="/usr/pkg/gcc9/bin:$PATH"

Avec OpenBSD 7.3 on doit faire avant dans le terminal:
> $ export AUTOCONF\_VERSION=2.69 AUTOMAKE\_VERSION=1.16

Téléchargez ce dépôt et exécutez dans un terminal:
> $ cd RUTA\_DE\_FRACTAL/3.4.20
>
> $ sh build.sh

Ce script (build.sh) essai de construire l'exécutable an utilisant PGO (Profile
Guided Optimization) mais c'échoue dans some systèmes. Dans ce cas il faut
fair:
> $ cd RUTA\_DE\_FRACTAL/3.4.20
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

Par défaut, le programme utilise le widget GtkGLArea de la bibliothèque GTK3.
Ce widget échoue dans machines virtuals (QEMU et Virtualbox). Pour utiliser
d'autres bibliothèques graphiques on peut faire:
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

Le fichier Makefile utilise characteristiques uniques du programme GNU make.
En systèmes BSD il faut changer le command **make** pour **gmake**.

Éventuellement pour faire une version finale:
> $ make strip

Pour exécuter le programme faites dans le terminal:
> $ ./fractal

ou, en utilisant un fichier d'entrée:
> $ ./fractal fichero\_de\_entrada

Quelques examples de fichiers d'entrée sont au répertoire **tests**.

Ce programme a été construit et prouvé avec les suivants systèmes
d'exploitation:
* Arch Linux
* Debian 11 (Linux)
* Devuan Linux 4
* Dragonfly BSD 6.4.0
* Fedora Linux 38
* Gentoo Linux
* FreeBSD 13.2
* Linux Mint DE 5
* MacOS Monterey + Homebrew
* Manjaro Linux
* Microsoft Windows 10 + MSYS2
* NetBSD 9.3
* OpenBSD 7.3
* OpenInidiana Hipster
* OpenSUSE Linux Leap 15.2
* Ubuntu Linux 21.04

Peut être qu'on puisse construire le programme en autres systèmes, distributions
ou versions, mais ça n'a pas été prouvé.

POUR FAIRE LE MANUEL D'INSTRUCTIONS (latex/refman.pdf file)
-----------------------------------------------------------

Exécutez dans un terminal:
> $ cd RUTA\_DE\_FRACTAL/3.4.20
>
> $ doxygen
>
> $ cd latex
>
> $ make
