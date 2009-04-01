AC_PREREQ(2.53)
AC_INIT(dalston, 0.0.4)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_CC_C_O

CFLAGS="$CFLAGS -Wall"

PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.14 gobject-2.0)
PKG_CHECK_MODULES(DBUS_GLIB, dbus-glib-1)
PKG_CHECK_MODULES(GTK, gtk+-2.0)
PKG_CHECK_MODULES(PULSE, libpulse-mainloop-glib >= 0.9.10)
PKG_CHECK_MODULES(CANBERRA, libcanberra-gtk)

AC_OUTPUT([
        Makefile
        libhal-glib/Makefile
        libhal-panel-glib/Makefile
        libgvc/Makefile
        dalston/Makefile
        tests/Makefile
        data/Makefile
        data/icons/Makefile
        src/Makefile
])
