# Header
AC_PREREQ([2.69])
AC_INIT([fractal],[3.4.23])

# Checks for operative systems
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AC_MSG_NOTICE([Build: $build])
AC_MSG_NOTICE([Host: $host])
win=0
win32=0
haiku=0
if test $host = "x86_64-w64-mingw32"; then
	AC_MSG_NOTICE([Microsoft Windows 64 bits])
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
	win32=1
elif test $host = "i686-pc-mingw32"; then
	AC_MSG_NOTICE([Microsoft Windows 32 bits])
	LDFLAGS="$LDFLAGS -lfreeglut -lopengl32"
	AC_SUBST(ccflags, "-march=pentium4 -mfpmath=sse -msse2 -mwindows")
	AC_SUBST(libgcc, "libgcc_s_dw2-1")
	AC_SUBST(icon, [logo.o])
	AC_SUBST(sysdir, [/mingw32/])
	AC_SUBST(basedir, [win32/])
	win=1
	win32=1
elif test $host = "x86_64-pc-cygwin"; then
	AC_MSG_NOTICE([Microsoft Windows 64 bits])
	AC_SUBST(ccflags, "-march=corei7 -mfpmath=sse -msse3 -mwindows")
	AC_SUBST(libgcc, "libgcc_s_seh-1")
	AC_SUBST(icon, [logo.o])
	AC_SUBST(sysdir, [/mingw64/])
	AC_SUBST(basedir, [win64/])
	win=1
elif test $host = "i686-pc-cygwin"; then
	AC_MSG_NOTICE([Microsoft Windows 32 bits])
	LDFLAGS="$LDFLAGS -lfreeglut -lopengl32"
	AC_SUBST(ccflags, "-march=pentium4 -mfpmath=sse -msse2 -mwindows")
	AC_SUBST(libgcc, "libgcc_s_dw2-1")
	AC_SUBST(icon, [logo.o])
	AC_SUBST(sysdir, [/mingw32/])
	AC_SUBST(basedir, [win32/])
	win=1
	win32=1
elif test `uname -s` = "Darwin"; then
	AC_MSG_NOTICE([MacOS])
	LDFLAGS="$LDFLAGS -L/usr/local/opt/llvm@13/lib -L/System/Library/Frameworks -framework GLUT -framework OpenGL"
	CPPFLAGS+="$CPPFLAGS -I/usr/local/include -I/usr/local/opt/llvm@13/include"
elif test `uname -s` = "NetBSD"; then
	AC_MSG_NOTICE([NetBSD])
	LDFLAGS="$LDFLAGS -L/usr/pkg/lib -L/usr/X11R7/lib"
	CPPFLAGS="$CPPFLAGS -I/usr/pkg/include -I/usr/X11R7/include"
elif test `uname -s` = "OpenBSD"; then
	AC_MSG_NOTICE([OpenBSD])
	LDFLAGS="$LDFLAGS -L/usr/X11R6/lib"
	CPPFLAGS="$CPPFLAGS -I/usr/X11R6/include -I/usr/local/include"
elif test `uname -s` = "DragonFly"; then
	AC_MSG_NOTICE([DragonFly BSD])
	LDFLAGS="$LDFLAGS -L/usr/local/lib"
	CPPFLAGS="$CPPFLAGS -I/usr/local/include"
elif test `uname -s` = "SunOS"; then
	AC_MSG_NOTICE([SunOS])
	AC_SUBST(gnu, [/usr/gnu/bin/])
elif test `uname -s` = "Haiku"; then
	AC_MSG_NOTICE([Haiku])
	haiku=1
else
	AC_MSG_NOTICE([Unix type system])
fi
if test $win = 1; then
	AC_SUBST(EXE, [.exe])
fi

# Checks for programs
AC_PROG_CC([gcc-15 gcc15 gcc-14 gcc14 gcc-13 gcc13 gcc-12 gcc12 egcc gcc clang])
AC_MSG_NOTICE([Compiler: $CC])
$CC -v
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LANG([C])
PKG_PROG_PKG_CONFIG
if test "`command -v gmake`"; then
	AC_SUBST(MAKE, [gmake])
else
	AC_SUBST(MAKE, [make])
fi
if test "$win" = "1"; then
	AC_CHECK_TOOL(WINDRES, windres)
fi
if test $CC = "clang"; then
	AC_CHECK_PROGS([LLVM_PROFDATA], [llvm-profdata llvm-profdata-15], 0)
	if test $LLVM_PROFDATA = 0; then
	      	AC_MSG_ERROR([No llvm-profdata])
	fi
	AC_MSG_NOTICE([llvm-profdata: $LLVM_PROFDATA])
fi

# Checks for compiler flags
compiler=$CC

# Checking -march=native compiler flag
AC_ARG_WITH([native], AS_HELP_STRING([--with-native],
	[Build using -march=native compiler flag]))
if test "x$with_native" = "xyes"; then
	CC="$CC -march=native"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])], [arch=1], [arch=0])
	CC=$compiler
	if test $arch = 0; then
		AC_MSG_NOTICE([checking for -march=native... no])
	else
		AC_MSG_NOTICE([checking for -march=native... yes])
		AC_SUBST(ARCH, "-march=native")
	fi
fi

# Checking -flto compiler flag
CC="$CC -flto"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])], [lto=1], [lto=0])
CC=$compiler
if test $lto = 0; then
	AC_MSG_NOTICE("checking for -flto... no")
else
	AC_MSG_NOTICE("checking for -flto... yes")
	AC_SUBST(LTO, "-flto")
fi

# Checking PGO optimiztion
AC_ARG_WITH([pgo], AS_HELP_STRING([--with-pgo], [Build using PGO]))
pgo=0
if test "x$with_pgo" = "xyes"; then
	CC="$CC -fprofile-generate"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])], [pgo=1])
	CC=$compiler
	if test $pgo = 0; then
		CC="$CC -fprofile-instr-generate"
		AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])], [pgo=1])
		CC=$compiler
	fi
fi
if test $pgo = 0; then
		AC_MSG_NOTICE("checking for PGO... no")
else
		AC_MSG_NOTICE("checking for PGO... yes")
fi
AC_SUBST(PGO, [$pgo])

# Checks for libraries.
AC_SEARCH_LIBS([sqrt], [m])
AC_SEARCH_LIBS([textdomain], [intl])
PKG_CHECK_MODULES([GSL], [gsl])
PKG_CHECK_MODULES([XML], [libxml-2.0])
PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([GTHREAD], [gthread-2.0])
PKG_CHECK_MODULES([GTOP], [libgtop-2.0], AC_DEFINE([HAVE_GTOP], [1]),
	AC_MSG_NOTICE("No libgtop"))
PKG_CHECK_MODULES([PNG], [libpng])
PKG_CHECK_MODULES([FREETYPE], [freetype2])
PKG_CHECK_MODULES([EPOXY], [epoxy])
AC_SEARCH_LIBS([glViewport], [GL opengl32])

# Checks for GTK interface
AC_ARG_WITH([gtk], AS_HELP_STRING([--with-gtk],
	[Build using a version of GTK (3 (--with-gtk=3 or 4 (--with-gtk=4))]))
if test "x$with_gtk" = "x4"; then
	PKG_CHECK_MODULES([GTK], [gtk4], [gtk4=1], [gtk4=0])
fi
if test "x$gtk4" = "x1"; then
	AC_SUBST(GTK4, "-DGTK4=1")
else
	AC_SUBST(GTK4, "-DGTK4=0")
	PKG_CHECK_MODULES([GTK], [gtk+-3.0])
fi

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
	PKG_CHECK_MODULES([GLUT], [glut], ,
		[PKG_CHECK_MODULES([GLUT], [freeglut])])
else
	if test "x$with_glfw" = "xyes"; then
		AC_SUBST(GRAPHIC, "-DHAVE_GLFW=1")
		PKG_CHECK_MODULES([GLFW], [glfw3])
	else
		if test "x$with_sdl" = "xyes"; then
			AC_SUBST(GRAPHIC, "-DHAVE_SDL=1")
			PKG_CHECK_MODULES([SDL], [sdl2])
		else
			AC_SUBST(GRAPHIC, "-DHAVE_GTKGLAREA=1 -DWINDOW_GLAREA=1")
		fi
	fi
fi

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h unistd.h], ,
	AC_MSG_ERROR([No standard headers]))
AC_CHECK_HEADER([sys/sysinfo.h], AC_SUBST(SYSINFO, "-DHAVE_SYSINFO=1"))

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([round sqrt cos sin sinf cosf], ,
	AC_MSG_ERROR([No standard mathematical functions]))
AC_CHECK_FUNC([sincos], [sincos=1], [sincos=0]) 
AC_CHECK_FUNC([sincosf], [sincosf=1], [sincosf=0])
AC_SUBST(SINCOS, [-DHAVE_SINCOS=$sincos])
AC_SUBST(SINCOSF, [-DHAVE_SINCOSF=$sincosf])

# Check for font
AC_MSG_NOTICE([Searching font...])
if test $win = 1; then
	if test $win32 = 1; then
		dir="/mingw32"
	else
		dir="/ucrt64"
	fi
elif test $haiku = 1; then
	dir="/"
else
	dir="/usr"
fi
font=`find $dir -type f -iname freemono.otf`
if test "x$font" = "x"; then
	font=`find $dir -type f -iname freemono.ttf`
	if test "x$font" = "x"; then
		font=`find $dir -type f -iname NotoSansMono-Regular.ttf`
		if test "x$font" = "x"; then
			AC_MSG_ERROR([FreeMono font not found])
		fi
	fi
fi
AC_MSG_NOTICE([Font: $font])
AC_SUBST(FONT, "-DFONT=\\\"$font\\\"")

# Output
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
