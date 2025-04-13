plugindir = $(libdir)/xfce4/panel/plugins
plugin_LTLIBRARIES = libmount.la

libmount_la_SOURCES =                                \
	devices.c                                           \
	devices.h                                           \
	mount-plugin.c  				\
	mount-plugin.h					\
	helpers.c						\
	helpers.h

libmount_la_CFLAGS =			         \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"               \
	-DDATADIR=\"$(datadir)\"			\
	@GLIB_CFLAGS@					\
	@GTK_CFLAGS@					\
	@LIBXFCE4PANEL_CFLAGS@				\
	@LIBXFCE4UI_CFLAGS@				\
	@LIBXFCE4UTIL_CFLAGS@

libmount_la_LIBADD =                                \
	@GLIB_LIBS@					\
	@GTK_LIBS@					\
	@LIBXFCE4PANEL_LIBS@				\
	@LIBXFCE4UI_LIBS@				\
	@LIBXFCE4UTIL_LIBS@

libmount_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

# .desktop file
#
desktop_in_files = xfce4-mount-plugin.desktop.in
	
desktopdir = $(datadir)/xfce4/panel/plugins
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

%.desktop: %.desktop.in
	$(AM_V_GEN) $(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

EXTRA_DIST = \
	$(desktop_in_files) \
	meson.build \
	$(NULL)

DISTCLEANFILES = $(desktop_DATA)
