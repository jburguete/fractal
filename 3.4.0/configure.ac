AC_PREREQ([2.67])
AC_INIT(fractal, [3.4.0])

# Checks for operative systems
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE
AC_MSG_NOTICE([Build: $build])
AC_MSG_NOTICE([Host: $host])
win=0
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
elif test `uname -s` = "OpenBSD"; then
	AC_MSG_NOTICE([OpenBSD])
	LDFLAGS="$LDFLAGS -L/usr/X11R6/lib"
	CPPFLAGS="$CPPFLAGS -I/usr/X11R6/include"
elif test `uname -s` = "DragonFly"; then
	AC_MSG_NOTICE([DragonFly BSD])
	LDFLAGS="$LDFLAGS -L/usr/local/lib"
	CPPFLAGS="$CPPFLAGS -I/usr/local/include"
elif test `uname -s` = "SunOS"; then
	AC_MSG_NOTICE([SunOS])
	AC_SUBST(gnu,[/usr/gnu/bin/])
else
	AC_MSG_NOTICE([Unix type system])
fi

# Checks for programs
AC_PROG_CC([gcc-9 gcc-8 gcc8 gcc7 gcc6 gcc5 gcc49 gcc48 egcc gcc clang])
AC_PROG_MAKE_SET
AC_LANG([C])
PKG_PROG_PKG_CONFIG
AC_MSG_NOTICE([Compiler: $CC])
$CC -v
if test "x$win" = "x1"; then
	AC_CHECK_TOOL(WINDRES, windres)
fi

# Checks for compiler flags
compiler=$CC

CC="$CC -flto"
AC_TRY_LINK([], [int main(int argc, char **argv){return 0;}], [lto=1], [lto=0])
CC=$compiler
if test "x$lto" = "x0"; then
	AC_MSG_NOTICE("checking for -flto... no")
	AC_SUBST(LTO, "")
else
	AC_MSG_NOTICE("checking for -flto... yes")
	AC_SUBST(LTO, "-flto")
fi

CC="$CC -std=gnu11"
AC_TRY_LINK([], [int main(int argc, char **argv){return 0;}], [std=1], [std=0])
CC=$compiler
if test "x$std" = "x0"; then
	AC_MSG_NOTICE("checking for -std=gnu11... no")
	CC="$CC -std=gnu99"
	AC_TRY_LINK([], [int main(int argc, char **argv){return 0;}], [std=1],
		[std=0])
	CC=$compiler
	if test "x$std" = "x0"; then
		AC_MSG_NOTICE("checking for -std=gnu99... no")
		AC_SUBST(STD, "")
	else
		AC_MSG_NOTICE("checking for -std=gnu99... yes")
		AC_SUBST(STD, "-std=gnu99")
	fi
else
	AC_MSG_NOTICE("checking for -std=gnu11... yes")
	AC_SUBST(STD, "-std=gnu11")
fi

AC_ARG_WITH([pgo],
	AS_HELP_STRING([--with-pgo], [Build using PGO]))
if test "x$with_pgo" = "xyes"; then
	CC="$CC -fprofile-generate"
	AC_TRY_LINK([], [int main(int argc, char **argv){return 0;}], [pgo=1],
		[pgo=0])
	CC=$compiler
	if test "x$pgo" = "x0"; then
		AC_MSG_NOTICE("checking for PGO... no")
		AC_SUBST(PGO, "0")
	else
		AC_MSG_NOTICE("checking for PGO... yes")
		AC_SUBST(PGO, "1")
	fi
fi

if test "x$win" = "x0"; then
	CC="$CC -march=native"
	AC_TRY_LINK([], [int main(int argc, char **argv){return 0;}], [native=1],
		[native=0])
	CC=$compiler
	if test "x$native" = "x0"; then
		AC_MSG_NOTICE("checking for -march=native... no")
		AC_SUBST(NATIVE, "")
	else
		AC_MSG_NOTICE("checking for -march=native... yes")
		AC_SUBST(NATIVE, "-march=native")
	fi
fi

# Checks for libraries.
AC_SEARCH_LIBS([sqrt], [m])
AC_SEARCH_LIBS([textdomain], [intl])
PKG_CHECK_MODULES([GSL], [gsl])
PKG_CHECK_MODULES([XML], [libxml-2.0])
PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([GTHREAD], [gthread-2.0])
PKG_CHECK_MODULES([GTOP], [libgtop-2.0])
PKG_CHECK_MODULES([PNG], [libpng])
PKG_CHECK_MODULES([FREETYPE], [freetype2])
PKG_CHECK_MODULES([GLEW], [glew])
PKG_CHECK_MODULES([GTK], [gtk+-3.0])
AC_SEARCH_LIBS([glViewport], [GL opengl32])

# Checks for GtkGLArea, FreeGLUT, GLFW or SDL to draw the graphic window
AC_ARG_WITH([gtkglarea],
	AS_HELP_STRING([--with-gtkglarea], [Build graphic window with GtkGLArea]))
AC_ARG_WITH([freeglut],
	AS_HELP_STRING([--with-freeglut], [Build graphic window with FreeGLUT]))
AC_ARG_WITH([glfw],
	AS_HELP_STRING([--with-glfw], [Build graphic window with GLFW]))
AC_ARG_WITH([sdl],
	AS_HELP_STRING([--with-sdl], [Build graphic window with SDL]))
if test "x$with_freeglut" = "xyes"; then
	AC_SUBST(GRAPHIC, "-DHAVE_FREEGLUT=1")
	AC_SEARCH_LIBS([glutInit], [freeglut glut], glut="yes", glut="no")
else
	if test "x$with_glfw" = "xyes"; then
		AC_SUBST(GRAPHIC, "-DHAVE_GLFW=1")
		PKG_CHECK_MODULES([GLFW], [glfw3])
	else
		if test "x$with_sdl" = "xyes"; then
			AC_SUBST(GRAPHIC, "-DHAVE_SDL=1")
			PKG_CHECK_MODULES([SDL], [sdl2])
		else
			AC_SUBST(GRAPHIC, "-DHAVE_GTKGLAREA=1")
		fi
	fi
fi

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h unistd.h libintl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([sqrt cos sin])

# Output
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
