@SET_MAKE@

INCLUDES =								\
	-DG_LOG_DOMAIN=\"xfce4-mount-plugin\"  \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"

plugindir = @XFCE4_PANEL_PLUGINSDIR@

plugin_LTLIBRARIES = libmount.la 

libmount_la_LDFLAGS =                                        \
        -avoid-version                                          \
        -export-dynamic                                         \
        -module

libmount_la_SOURCES =                                        \
        devices.c                                                    \
        devices.h                                                     \
        mount-plugin.c                                                   

libmount_la_CFLAGS = @XFCE4_PANEL_CFLAGS@

libmount_la_LIBADD = @XFCE4_PANEL_LIBS@

EXTRA_DIST = 							\
	icons.h 						\
	kcmpartitions.png

noinst_DATA =							\
	icons.h 						\
	kcmpartitions.png

icons.h: $(srcdir)/kcmpartitions.png
	gdk-pixbuf-csource --raw --build-list 			\
	icon_plugin kcmpartitions.png > icons.h



