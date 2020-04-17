aclocal
autoconf
automake --add-missing
./configure --with-gtkglarea --with-pgo
if test -x "`command -v gmake`"; then 
	gmake 
else
	make
fi
