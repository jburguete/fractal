aclocal
autoconf
automake --add-missing
./configure --with-freeglut --with-pgo
if test -x "`command -v gmake`"; then 
	gmake 
else
	make
fi
