# To work in:
#	- Debian 8.2 (Linux, kFreeBSD or Hurd)
#	- FreeBSD 10.2
#	- Microsoft Windows 7
#	- Microsoft Windows 8.1
#	- NetBSD 6.1.5 (with modular xorg)
#	- DragonFly BSD 4.2.4
#
# ./build

# To work in OpenBSD 5.7:
#
# export AUTOCONF_VERSION=2.69 AUTOMAKE_VERSION=1.15
# ./build

AC_PREREQ([2.67])
AC_INIT(fractal, [2.4.3])

# Checks for operative systems
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE
AC_MSG_NOTICE([Build: $build])
AC_MSG_NOTICE([Host: $host])
if test $host = "x86_64-w64-mingw32"; then
	AC_MSG_NOTICE([Microsoft Windows 64 bits])
	CPPFLAGS="$CPPFLAGS -I/mingw/include"
	AC_SUBST(ccflags, "-march=corei7 -mfpmath=sse -msse3 -mwindows")
	AC_SUBST(libgcc, "libgcc_s_seh-1")
	AC_SUBST(icon, [logo.o])
	win=1
elif test $host = "x86_64-pc-mingw64"; then
	AC_MSG_NOTICE([Microsoft Windows 64 bits])
	AC_SUBST(ccflags, "-march=corei7 -mfpmath=sse -msse3 -mwindows")
	AC_SUBST(libgcc, "libgcc_s_seh-1")
	AC_SUBST(icon, [logo.o])
	AC_SUBST(sysdir, [/mingw64/])
	AC_SUBST(basedir, [win64/])
	win=1
elif test $host = "x86_64-pc-mingw32"; then
	AC_MSG_NOTICE([Microsoft Windows 32 bits])
	LDFLAGS="$LDFLAGS -lfreeglut -lopengl32"
	AC_SUBST(ccflags, "-march=pentium4 -mfpmath=sse -msse2 -mwindows")
	AC_SUBST(libgcc, "libgcc_s_dw2-1")
	AC_SUBST(icon, [logo.o])
	AC_SUBST(sysdir, [/mingw32/])
	AC_SUBST(basedir, [win32/])
	win=1
elif test $host = "i686-pc-mingw32"; then
	AC_MSG_NOTICE([Microsoft Windows 32 bits])
	LDFLAGS="$LDFLAGS -lfreeglut -lopengl32"
	AC_SUBST(ccflags, "-march=pentium4 -mfpmath=sse -msse2 -mwindows")
	AC_SUBST(libgcc, "libgcc_s_dw2-1")
	AC_SUBST(icon, [logo.o])
	AC_SUBST(sysdir, [/mingw32/])
	AC_SUBST(basedir, [win32/])
	win=1
elif test `uname -s` = "NetBSD"; then
	AC_MSG_NOTICE([NetBSD])
	LDFLAGS="$LDFLAGS -L/usr/pkg/lib"
	CPPFLAGS="$CPPFLAGS -I/usr/pkg/include"
	PATH="/usr/pkg/gcc48/bin:$PATH"
	win=0
elif test `uname -s` = "OpenBSD"; then
	AC_MSG_NOTICE([OpenBSD])
	LDFLAGS="$LDFLAGS -L/usr/X11R6/lib"
	CPPFLAGS="$CPPFLAGS -I/usr/X11R6/include"
	win=0
elif test `uname -s` = "DragonFly"; then
	AC_MSG_NOTICE([DragonFly BSD])
	LDFLAGS="$LDFLAGS -L/usr/local/lib"
	CPPFLAGS="$CPPFLAGS -I/usr/local/include"
	win=0
else
	AC_MSG_NOTICE([Unix type system])
	AC_SUBST(ccflags, "-march=native")
	win=0
fi

# Checks for programs
AC_PROG_CC([gcc5 gcc49 gcc48 egcc gcc clang])
AC_PROG_MAKE_SET
AC_LANG([C])
PKG_PROG_PKG_CONFIG
AC_MSG_NOTICE([Compiler: $CC])
$CC -v
if test $win = 1; then
	AC_CHECK_TOOL(WINDRES, windres)
fi

# Checks for compiler flags
compiler=$CC
CC="$CC -flto"
AC_TRY_LINK([], [int main(int argc, char **argv){return 0;}], [lto=1], [lto=0])
CC=$compiler
if test $lto = 0; then
	AC_MSG_NOTICE("checking for -flto... no")
	AC_SUBST(LTO, "")
else
	AC_MSG_NOTICE("checking for -flto... yes")
	AC_SUBST(LTO, "-flto")
fi

# Checks for libraries.
AC_SEARCH_LIBS([sqrt], [m])
AC_SEARCH_LIBS([textdomain], [intl])
PKG_CHECK_MODULES([GSL], [gsl])
PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([GTHREAD], [gthread-2.0])
PKG_CHECK_MODULES([GTK], [gtk+-3.0])
PKG_CHECK_MODULES([PNG], [libpng])
AC_SEARCH_LIBS([glutInit], [freeglut glut])
AC_SEARCH_LIBS([glEnd], [GL opengl32])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h unistd.h libintl.h GL/freeglut.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([sqrt cos sin])

# Output
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
