plugindir = $(libexecdir)/xfce4/panel-plugins
plugin_PROGRAMS = xfce4-mount-plugin

LIBS = @LIBS@ @SOLLIBS@

xfce4_mount_plugin_SOURCES =                                \
        devices.c                                           \
        devices.h                                           \
        mount-plugin.c  				\
	mount-plugin.h

xfce4_mount_plugin_CFLAGS =			         \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"               \
	@LIBXFCE4PANEL_CFLAGS@				\
	@LIBXFCEGUI4_CFLAGS@

xfce4_mount_plugin_LDADD =                                \
        @SOLLIBS@                                   			\
	@LIBXFCE4PANEL_LIBS@				\
	@LIBXFCEGUI4_LIBS@

# .desktop file
#
# Some automake trickery here. Because we cannot use $(libexecdir) in the
# automake stage, we'll use sed to get the full path into the .desktop file.
# We also need to let intltool merge the translated fields, so we add an
# additional level of indirection: a <name>.desktop.in.in file.
# 
desktop_in_in_files = xfce4-mount-plugin.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in)
	
desktopdir = $(datadir)/xfce4/panel-plugins
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = 						      	                     \
	helpers.c													\
	icons.h 						                                 \
	kcmpartitions.png                                        \
	$(desktop_in_in_files)

DISTCLEANFILES =				                        \
	$(desktop_DATA) $(desktop_in_files)
	
noinst_DATA =							                           \
	icons.h 						                                 \
	kcmpartitions.png

icons.h: $(srcdir)/kcmpartitions.png
	gdk-pixbuf-csource --raw --build-list 			            \
	icon_plugin kcmpartitions.png > icons.h
	
# get full path into .desktop file
%.desktop.in: %.desktop.in.in
	sed -e "s^@PLUGIN_PATH@^$(libexecdir)/xfce4/panel-plugins^" \
		$< > $@



