
AC_PREREQ(2.53)
AC_INIT(moblin-panel-devices, 0.1.6)
AC_CONFIG_AUX_DIR([build])
AC_CONFIG_MACRO_DIR([build])
AM_INIT_AUTOMAKE([1.10 foreign -Wno-portability no-define])
AM_CONFIG_HEADER(config.h)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])

AC_CANONICAL_HOST
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=moblin-panel-devices
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],
                   ["$GETTEXT_PACKAGE"],
                   [Gettext domain name])
AM_GLIB_GNU_GETTEXT

localedir='${prefix}/${DATADIRNAME}/locale'
AC_SUBST([localedir])

# For absolute paths in desktop files etc.
AS_AC_EXPAND(LIBEXECDIR, "$libexecdir")
AS_AC_EXPAND(PKGTHEMEDIR, "$datadir/$PACKAGE/theme")

AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_CC_C_O

PKG_PROG_PKG_CONFIG
AS_ALL_LINGUAS

#
# Gnome Disk Utility
#

gdu_reqs="glib-2.0 >= 2.22 \
 gobject-2.0 >= 2.16 \
 gio-2.0 >= 2.22 \
 gio-unix-2.0 >= 2.22 \
 dbus-glib-1 >= 0.74 \
 gnome-keyring-1 >= 2.22 \
 udisks >= 1.0.0 \
 udisks < 1.1.0"

# Try to find udisks dbus interfaces.
udisks_dbus_dir='dbus-1/interfaces'
udisks_dbus_if='org.freedesktop.UDisks.xml'
udisks_dbus_path="$udisks_dbus_dir/$udisks_dbus_if"
UDISKS_DATADIR=

# '/opt/moblin/share' path is hardcoded so the maintainer can easily distcheck.
AS_AC_EXPAND([UDISKS_DATADIR], $datadir)
udisks_search_path="$UDISKS_DATADIR
/usr/share
/opt/moblin/share"

AC_ARG_WITH([udisks],
            [AS_HELP_STRING([--with-udisks=PREFIX], [Use udisks in PREFIX])])

UDISKS_DATADIR=
if test "$with_udisks" != "no"; then

PKG_CHECK_MODULES(GDU, $gdu_reqs)
gdu_deps="$gdu_reqs"

if test "$with_udisks" != "yes" &&
   test "$with_udisks" != ""; then
udisks_search_path="$withval/share
$udisks_search_path"
fi

for dir in $udisks_search_path; do
  AC_MSG_CHECKING([for udisks dbus interface $dir/$udisks_dbus_path])
  if test -f $dir/$udisks_dbus_path; then
    AC_MSG_RESULT([yes])
    UDISKS_DATADIR=$dir
    break
  else
    AC_MSG_RESULT([no])
  fi
done

if test "$UDISKS_DATADIR" == ""; then
  AC_MSG_ERROR([Udisks dbus interfaces not found, use --with-udisks=PREFIX or --without-udisks.])
fi

AC_DEFINE([HAVE_UDISKS], [1], [Udisks is available for volume monitoring])

fi # with-udisks

AC_SUBST([UDISKS_DATADIR])
AM_CONDITIONAL([HAVE_UDISKS], [test "$with_udisks" != "no"])

#
# Gnome Volume Control
#

gvc_deps='libpulse-mainloop-glib >= 0.9.15'
PKG_CHECK_MODULES(GVC, $gvc_deps)

#
# Moblin Panel Devices
#

PKG_CHECK_MODULES(MPD,
                  $gdu_deps
                  $gvc_deps
                  clutter-1.0
                  clutter-gtk-0.10
                  devkit-power-gobject
                  moblin-panel >= 0.66.5
                  mx-1.0)

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

#
# Egg Console Kit
#

eck_deps='dbus-glib-1'
PKG_CHECK_MODULES(ECK, $eck_deps)

#
# Egg Idle Time
#

eit_deps='gdk-x11-2.0'
PKG_CHECK_MODULES(EIT, $eit_deps)

#
# Moblin Power Icon
#

PKG_CHECK_MODULES(MPI,
                  $eck_deps
                  $eit_deps
                  clutter-1.0
                  clutter-gtk-0.10
                  devkit-power-gobject
                  gconf-2.0
                  gdk-x11-2.0
                  libnotify
                  moblin-panel >= 0.66.5
                  mx-1.0)

#
# Common stuff
#

MOBLIN_PANELS_DIR=`$PKG_CONFIG --variable=moblin_panel_panels_dir moblin-panel`
AC_SUBST(MOBLIN_PANELS_DIR)

AC_OUTPUT([
  build/Makefile
  data/icons/Makefile
  data/Makefile
  data/moblin-panel-devices.desktop.in
  data/org.moblin.UX.Shell.Panels.devices.service
  data/theme/Makefile
  egg-console-kit/Makefile
  egg-idletime/Makefile
  gdu/Makefile
  gvc/Makefile
  Makefile
  po/Makefile.in
  power-icon/data/Makefile
  power-icon/data/moblin-power-icon.desktop.in
  power-icon/data/org.moblin.UX.Shell.Panels.power.service
  power-icon/data/theme/Makefile
  power-icon/data/theme/power-icon.css
  power-icon/Makefile
  power-icon/src/Makefile
  power-icon/tests/Makefile
  src/Makefile
  tests/Makefile
])

