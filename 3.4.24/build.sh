#!/usr/bin/env bash
if [[ `uname -s` = "OpenBSD" ]]; then
	export AUTOCONF_VERSION=2.71
	export AUTOMAKE_VERSION=1.16
elif [[ `uname -s` = "SunOS" ]]; then
	export PATH=/usr/gcc/11/bin:$PATH
fi
if [[ $# != 5 ]]; then
	echo "The syntax is: ./build.sh A B C D E"
	echo "A: 1 on native"
	echo "B: 1 on PGO"
	echo "C: 3 on GTK3, 4 on GTK4"
	echo "D: 1 on FreeGLUT, 2 on GLFW, 3 on SDL, 4 on GtkGLArea"
	echo "E: 1 on strip"
	exit 1
fi
if [[ $1 = 1 ]]; then
	a1="--with-native"
fi
if [[ $2 = 1 ]]; then
	a2="--with-pgo"
fi
if [[ $3 = 3 || $3 = 4 ]]; then
	a3="--with-gtk=$3"
fi
if [[ $4 = 1 ]]; then
	a4="--with-freeglut"
elif [[ $4 = 2 ]]; then
	a4="--with-glfw"
elif [[ $4 = 3 ]]; then
	a4="--with-sdl"
elif [[ $4 = 4 ]]; then
	a4="--with-gtkglarea"
fi
if [[ $5 = 1 ]]; then
	a5="strip"
fi
aclocal
autoconf
automake --add-missing
./configure $a1 $a2 $a3 $a4
if test -x "`command -v gmake`"; then
	gmake $a5
else
	make $a5
fi
