
AM_CFLAGS = \
  $(MPI_CFLAGS) \
  -I$(top_srcdir) \
  -I$(top_srcdir)/src \
  -DLOCALEDIR=\"$(localedir)\" \
  -DPKGTHEMEDIR=\"$(PKGTHEMEDIR)/power-icon\" \
  -DI_KNOW_THE_DEVICEKIT_POWER_API_IS_SUBJECT_TO_CHANGE \
  $(NULL)

libexec_PROGRAMS = \
  meego-power-icon \
  $(NULL)

meego_power_icon_LDADD = \
  $(top_builddir)/egg-console-kit/libegg-console-kit.la \
  $(top_builddir)/gpm/libgpm.la \
  $(MPI_LIBS) \
  $(NULL)

meego_power_icon_SOURCES = \
  $(top_srcdir)/src/mpd-conf.c \
  $(top_srcdir)/src/mpd-battery-device.c \
  $(top_srcdir)/src/mpd-display-device.c \
  $(top_srcdir)/src/mpd-gobject.c \
  meego-power-icon.c \
  mpd-global-key.c \
  mpd-global-key.h \
  mpd-idle-manager.c \
  mpd-idle-manager.h \
  mpd-lid-device.c \
  mpd-lid-device.h \
  mpd-power-icon.c \
  mpd-power-icon.h \
  mpd-shutdown-notification.c \
  mpd-shutdown-notification.h \
  $(NULL)


-include $(top_srcdir)/git.mk
