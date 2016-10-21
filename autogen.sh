export AUTOCONF_VERSION=2.59
export AUTOMAKE_VERSION=1.4

make -i clean

rm -rf Makefile config.h autom4te.cache config.log config.status aclocal.m4
aclocal
autoconf
