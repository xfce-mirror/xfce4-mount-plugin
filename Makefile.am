@SET_MAKE@

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = panel-plugin \
					po					 \
					icons

distclean-local:
	rm -rf *.cache *~

ChangeLog: Makefile
	(GIT_DIR=$(top_srcdir)/.git git log > .changelog.tmp \
		&& mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) \
		|| (touch ChangeLog; echo 'Git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog

EXTRA_DIST =							\
	AUTHORS									\
	COPYING									\
	NEWS										\
	README									\
	meson.build								\
	po/meson.build								\
	xfce-revision.h.in							\
	$(NULL)
