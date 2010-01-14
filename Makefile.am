
ACLOCAL_AMFLAGS = -I build

SUBDIRS = build libhal-glib libhal-panel-glib libhal-power-glib libgvc libegg-idletime dalston src data po

DISTCHECK_CONFIGURE_FLAGS = --disable-moblin-desktop-file

EXTRA_DIST = \
	AUTHORS \
	autogen.sh \
	ChangeLog \
	COPYING \
	NEWS \
	README

# Line-by-line description of the dist-local target:
# + Extract tagged commits between $(VERSION) and the previous one ($v).
# + Write out previous version.
# + Extract NEWS from $v and older to NEWS.old
# + Write a new NEWS file with the new info.
# + Append old NEWS.
# + Delete tmp files.
dist-local:
	v=`$(top_srcdir)/build/git-log-extract-news.sh $(VERSION) 2>&1 > NEWS.new` && \
	echo "Previous version is $$v" && \
	test -s NEWS && grep -A 1000000 '^$$v' NEWS > NEWS.old; \
	test -f NEWS.new && cat NEWS.new > NEWS; \
	test -f NEWS.old && cat NEWS.old >> NEWS; \
	rm -f NEWS.old NEWS.new

# Only need to re-create changelog after version bump.
ChangeLog: configure.ac
	@echo Creating ChangeLog ...
	$(top_srcdir)/build/gen-changelog.pl > $@

