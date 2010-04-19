
ACLOCAL_AMFLAGS = -I build

SUBDIRS = \
  build \
  egg-console-kit \
  gpm \
  gvc \
  data \
  po \
  power-icon \
  src \
  tests \
  $(NULL)

EXTRA_DIST = \
  AUTHORS \
  autogen.sh \
  COPYING \
  NEWS \
  README \
  $(NULL)

GITIGNOREFILES = \
  aclocal.m4 \
  config.h.in \
  $(NULL)

extract-news:
	$(top_srcdir)/build/git-log-extract-news.sh $(VERSION) 2>/dev/null

-include $(top_srcdir)/git.mk
