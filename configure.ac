AC_PREREQ(2.53)
AC_INIT(moblin-panel-devices, 0.1.0)
AC_CONFIG_AUX_DIR([build])
AC_CONFIG_MACRO_DIR([build])
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)

AC_CANONICAL_HOST
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=moblin-panel-devices
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],
                   ["$GETTEXT_PACKAGE"],
                   [Gettext domain name])
AM_GLIB_GNU_GETTEXT

AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_CC_C_O

AS_ALL_LINGUAS

CFLAGS="$CFLAGS -Wall"

PKG_CHECK_MODULES(MPL, moblin-panel >= 0.50.0)

MOBLIN_PANELS_DIR=`$PKG_CONFIG --variable=moblin_panel_panels_dir moblin-panel`
AC_SUBST(MOBLIN_PANELS_DIR)

egg_console_kit_deps='dbus-glib-1'
PKG_CHECK_MODULES(EGG_CONSOLE_KIT, $egg_console_kit_deps)

PKG_CHECK_MODULES(DEVICES,
                  $egg_console_kit_deps
                  clutter-1.0
                  devkit-power-gobject
                  gdk-x11-2.0
                  gio-2.0
                  gtk+-2.0
                  libnotify
                  moblin-panel
                  mx-1.0)

#PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.14 gobject-2.0)
#PKG_CHECK_MODULES(DBUS_GLIB, dbus-glib-1)
#PKG_CHECK_MODULES(GTK, gtk+-2.0)
#PKG_CHECK_MODULES(PULSE, libpulse-mainloop-glib >= 0.9.10)
#PKG_CHECK_MODULES(CANBERRA, libcanberra-gtk)
#PKG_CHECK_MODULES(NBTK_GTK, nbtk-gtk-1.2)
#PKG_CHECK_MODULES(LIBNOTIFY, libnotify)
#PKG_CHECK_MODULES(GCONF, gconf-2.0)

AC_ARG_ENABLE([cache],
              [AC_HELP_STRING([--enable-cache],
                              [Enable Mx image cache generation])],
              [],
              [enable_cache=no])

AS_IF([test "x$enable_cache" = "xyes"],
      [
      AC_CHECK_PROG([MX_CREATE_IMAGE_CACHE],
                [mx-create-image-cache],
                [$(which mx-create-image-cache)])
])

AM_CONDITIONAL([ENABLE_CACHE],   [test "x$enable_cache" = "xyes"])

AC_ARG_ENABLE([moblin-desktop-file],
              [AC_HELP_STRING([--disable-moblin-desktop-file],
                              [Disable installation of moblin desktop file])],
              [enable_mdesktop=$enableval],
              [enable_mdesktop=yes])

AM_CONDITIONAL([ENABLE_MOBLIN_DESKTOP_FILE], [test "x$enable_mdesktop" = "xyes"])

AC_OUTPUT([
  Makefile
  build/Makefile
  data/Makefile
  data/theme/Makefile
  egg-console-kit/Makefile
  po/Makefile.in
  src/Makefile
  tests/Makefile
])

#AC_OUTPUT([
#  dalston/Makefile
#  libegg-idletime/Makefile
#  libgvc/Makefile
#  libhal-glib/Makefile
#  libhal-panel-glib/Makefile
#  libhal-power-glib/Makefile
#])

