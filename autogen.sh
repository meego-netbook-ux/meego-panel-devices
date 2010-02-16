#!/bin/sh
intltoolize
autoreconf -v -i && ./configure --enable-silent-rules $@

