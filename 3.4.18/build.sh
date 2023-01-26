#!/usr/bin/env bash
if [[ $# != 4 ]]; then
	echo "The syntax is: ./build.sh A B C D"
	echo "A: 1 on native"
	echo "B: 1 on PGO"
	echo "C: 3 on GTK3, 4 on GTK4"
	echo "D: 1 on FreeGLUT, 2 on GLFW, 3 on SDL, 4 on GtkGLArea"
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
aclocal
autoconf
automake --add-missing
./configure $a1 $a2 $a3 $a4
if test -x "`command -v gmake`"; then 
	gmake 
else
	make
fi
