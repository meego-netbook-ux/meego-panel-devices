
ACLOCAL_AMFLAGS = -I build

if HAVE_UDISKS
gdu_subdir = gdu
endif

SUBDIRS = \
  $(gdu_subdir) \
  build \
  egg-console-kit \
  egg-idletime \
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
