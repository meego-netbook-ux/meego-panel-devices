AC_PREREQ(2.53)
AC_INIT(dalston, 0.1.3)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_MACRO_DIR([m4])
AM_CONFIG_HEADER(config.h)

AC_CANONICAL_HOST

IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=dalston
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [The gettext package name])

AM_GLIB_GNU_GETTEXT

localedir='${prefix}/${DATADIRNAME}/locale'
AC_SUBST(localedir)


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
PKG_CHECK_MODULES(NBTK_GTK, nbtk-gtk-1.0)
PKG_CHECK_MODULES(LIBNOTIFY, libnotify)
PKG_CHECK_MODULES(GCONF, gconf-2.0)
PKG_CHECK_MODULES(MPL, moblin-panel)

SHAVE_INIT([.], [enable])

AC_OUTPUT([
        Makefile
        shave
        shave-libtool
        m4/Makefile
        libhal-glib/Makefile
        libhal-panel-glib/Makefile
        libhal-power-glib/Makefile
        libgvc/Makefile
        dalston/Makefile
        tests/Makefile
        data/Makefile
        data/icons/Makefile
        data/theme/Makefile
        src/Makefile
        po/Makefile.in
])
