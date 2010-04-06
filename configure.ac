
AC_PREREQ(2.53)
AC_INIT([moblin-panel-devices], [0.1.17], [http://bugzilla.moblin.org])
AC_CONFIG_AUX_DIR([build])
AC_CONFIG_MACRO_DIR([build])
AM_INIT_AUTOMAKE([1.10 foreign -Wno-portability no-define])
AM_CONFIG_HEADER(config.h)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])

AC_CANONICAL_HOST
IT_PROG_INTLTOOL([0.35.0])

AC_SUBST([GETTEXT_PACKAGE], AC_PACKAGE_NAME)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], "AC_PACKAGE_NAME", [Gettext domain name])
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
AC_TYPE_INT64_T
AM_PROG_CC_C_O

PKG_PROG_PKG_CONFIG
AS_ALL_LINGUAS

headers="
errno.h \
execinfo.h \
fcntl.h \
signal.h \
stdarg.h \
stdbool.h \
stdio.h \
sys/ioctl.h \
sys/statvfs.h \
time.h"
AC_CHECK_HEADERS($headers, [],
[
  AC_MSG_ERROR([missing required header])
])

#
# Gnome Disk Utility
#

gdu_reqs="glib-2.0 >= 2.22 \
 gobject-2.0 >= 2.16 \
 gio-2.0 >= 2.22 \
 gio-unix-2.0 >= 2.22 \
 dbus-glib-1 >= 0.74"

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
                  gio-2.0
                  libcanberra-gtk
                  libnotify
                  moblin-panel >= 0.66.5
                  mx-1.0 >= 0.99.0)

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

