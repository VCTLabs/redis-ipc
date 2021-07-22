#! /bin/sh

set -e

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

THEDIR="`pwd`"
cd "$srcdir"
DIE=0

abort () {
    echo "$1 not found or command failed. Aborting!"
    exit 1
}

#set -x
libtoolize --ltdl --force --copy || abort "libtoolize"
aclocal || abort "aclocal"
autoheader || abort "autoheader"
automake --gnu --add-missing --copy || abort "automake"
autoconf || abort "autoconf"

if test -z "$*"; then
        echo "You still need to run ./configure - if you wish to pass any arguments"
        echo "to it, please specify them on the $0 command line."
fi

#set +x

echo "After running ./configure, type:"
echo
echo "make"
echo "make install"
echo
echo "have fun."

