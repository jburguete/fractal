# To work in Debian (Linux, kFreeBSD or Hurd) or in FreeBSD:
# aclocal
# autoconf
# ./configure
# make

# To work in Microsoft Windows (64 bits) with MinGW/MSYS:
# aclocal -I /mingw/share/aclocal
# autoconf
# CPPFLAGS='-I/mingw/include -I/mingw/x86_64-w64-mingw32/include' configure --host=x86_64-w64-mingw32
# make

# To work in Microsoft Windows (32 bits) with MinGW/MSYS:
# aclocal
# autoconf
# LDFLAGS='-lglut -lopengl32' configure --host=mingw32
# make

# To work in NetBSD:
# ln -s /usr/X11R7/include/GL/glu.h /usr/pkg/include/GL
# ln -s /usr/X11R7/lib/libGL.so.2 /usr/pkg/lib
# aclocal
# autoconf
# CPPFLAGS=-I/usr/pkg/include LDFLAGS=-L/usr/pkg/lib ./configure --host=x86_64--netbsd
# make

# To work in OpenBSD:
# AUTOCONF_VERSION=2.69 aclocal-1.11
# autoconf-2.69
# CPPFLAGS='-I/usr/local/include -I/usr/X11R6/include' LDFLAGS='-L/usr/local/lib -L/usr/X11R6/lib' ./configure
# make

# To work in DragonFly BSD:
# setenv PATH ${PATH}:/usr/pkg/gcc47/bin
# setenv CPPFLAGS '-I/usr/pkg/include -I/usr/pkg/gcc47/include -I/usr/pkg/include/gettext'
# setenv CFLAGS '-I/usr/pkg/include -I/usr/pkg/gcc47/include -I/usr/pkg/include/gettext'
# setenv LDFLAGS '-L/usr/pkg/lib -L/usr/pkg/gcc47/lib'
# aclocal
# autoconf
# ./configure --host=x86_64-pc-dragonfly
# make

AC_PREREQ([2.67])
AC_INIT(fractal, [2.2])

# Checks for operative systems
#AC_CANONICAL_HOST: Requires automake
AC_MSG_NOTICE([System: $host])
if test $host == "x86_64-w64-mingw32"; then
	AC_MSG_NOTICE([Microsoft Windows 64 bits])
	AC_SUBST(ccflags, "-march=corei7 -mfpmath=sse -msse3")
	win=1
elif test $host == "mingw32"; then
	AC_MSG_NOTICE([Microsoft Windows 32 bits])
	AC_SUBST(ccflags, "-march=pentium4 -mfpmath=sse -msse2")
	win=1
else
	AC_MSG_NOTICE([Unix type system])
	AC_SUBST(ccflags, "-march=native")
	win=0
fi

# Checks for programs
AC_PROG_CC([gcc47 gcc46 egcc gcc clang])
AC_PROG_MAKE_SET
AC_LANG([C])
PKG_PROG_PKG_CONFIG
AC_MSG_NOTICE([Compiler: $CC])
if test $win == 1; then
	AC_CHECK_TOOL(WINDRES, windres)
fi

# Checks for compiler flags
compiler=$CC
CC="$CC -flto"
AC_TRY_LINK([], [int main(int argc, char **argv){return 0;}], [lto=1], [lto=0])
CC=$compiler
if test $lto == 0; then
	AC_MSG_NOTICE("checking for -flto... no")
	AC_SUBST(LTO, "")
else
	AC_MSG_NOTICE("checking for -flto... yes")
	AC_SUBST(LTO, "-flto")
fi

# Checks for libraries.
AC_SEARCH_LIBS([glutInit], [freeglut glut])
AC_SEARCH_LIBS([glEnd], [GL opengl32])
PKG_CHECK_MODULES([GTK], [gtk+-3.0], [gtk3=1], [gtk3=0])
if test $gtk3 == 0; then
	PKG_CHECK_MODULES([GTK], [gtk+-2.0])
fi
PKG_CHECK_MODULES([PNG], [libpng])
PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([GTHREAD], [gthread-2.0])
AC_SEARCH_LIBS([textdomain], [intl])
AC_SEARCH_LIBS([sqrt], [m])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h unistd.h libintl.h GL/freeglut.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([sqrt cos sin])

# Output
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
