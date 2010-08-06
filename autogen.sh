#! /bin/sh
#gtkdocize || exit 1

intltoolize --force || die "intltoolize failed"

autoreconf -v --install || exit 1
./configure "$@"
