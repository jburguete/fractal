if test -x `command -v gmake`; then
	MAKE="gmake"
else
	MAKE="make"
fi
export MAKE
aclocal
autoconf
automake --add-missing
./configure --with-freeglut --with-pgo
$MAKE
