plugindir = $(libdir)/xfce4/panel/plugins
plugin_LTLIBRARIES = libmount.la

LIBS = @LIBS@ @SOLLIBS@

libmount_la_SOURCES =                                \
	devices.c                                           \
	devices.h                                           \
	mount-plugin.c  				\
	mount-plugin.h					\
	helpers.c						\
	helpers.h

libmount_la_CFLAGS =			         \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"               \
	@LIBXFCE4PANEL_CFLAGS@				\
	@LIBXFCE4UI_CFLAGS@

libmount_la_LIBADD =                                \
	@SOLLIBS@                                   			\
	@LIBXFCE4PANEL_LIBS@				\
	@LIBXFCE4UI_LIBS@

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
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = $(desktop_in_files)

DISTCLEANFILES = $(desktop_DATA)
