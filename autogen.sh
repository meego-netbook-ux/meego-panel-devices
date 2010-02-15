#!/bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

olddir=`pwd`
cd $srcdir

intltoolize
autoreconf -v -i && ./configure --enable-silent-rules $@

cd $olddir
