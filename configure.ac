dnl configure.ac
dnl
dnl xfce4-mount-plugin - Mount plugin for xfce4-panel
dnl
dnl 2003-2004 Benedikt Meurer <benny@xfce.org>
dnl 2005-2020 Fabian Nowak <timystery@arcor.de>
dnl 2020 Olaf Hering <olaf@aepfle.de>
dnl 2012 Landry Breuil <landry@xfce.org>

m4_define([copyright_year], [2025])
XDT_VERSION_INIT([1.2.0])

AC_COPYRIGHT([Copyright (c) 2005-copyright_year()
        The Xfce development team. All rights reserved.])

AC_INIT([xfce4-mount-plugin], [xdt_version],
  [https://gitlab.xfce.org/panel-plugins/xfce4-mount-plugin/], [xfce4-mount-plugin])
AC_PREREQ([2.69])
AC_REVISION([xdt_version_build])
AM_INIT_AUTOMAKE([1.8 dist-bzip2 tar-ustar no-dist-gzip])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])
AC_DEFINE([VERSION_FULL], [PACKAGE_VERSION], [Alias for VERSION and PACKAGE_VERSION for meson compatibility])
COPYRIGHT_YEAR=copyright_year()
AC_DEFINE_UNQUOTED([COPYRIGHT_YEAR], ["$COPYRIGHT_YEAR"], [Copyright year])
AC_SUBST([COPYRIGHT_YEAR])

AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Check for UNIX variants
AC_USE_SYSTEM_EXTENSIONS

dnl Check for basic programs
AC_PROG_CC
AC_PROG_INSTALL

dnl Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

dnl No more try substituting "OS" in Makefiles, but might be required in future again
dnl AC_SUBST(OS)

dnl getmentent() requires stdio.h/mntent.h 
AC_CHECK_FUNCS([getmntent])
dnl On BSD, getmntinfo is used instead
AC_CHECK_FUNCS([getmntinfo])

dnl configure the panel plugin
XDT_CHECK_PACKAGE([GLIB], [glib-2.0], [2.50.0])
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.22.0])
XDT_CHECK_PACKAGE([LIBXFCE4UI], [libxfce4ui-2], [4.16.0])
XDT_CHECK_PACKAGE([LIBXFCE4UTIL], [libxfce4util-1.0], [4.16.0])
XDT_CHECK_PACKAGE([LIBXFCE4PANEL], [libxfce4panel-2.0], [4.16.0])

dnl Check for i18n support
GETTEXT_PACKAGE="$PACKAGE"
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Name of default gettext domain])
AC_SUBST([GETTEXT_PACKAGE])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])

dnl Check for debugging support
XDT_FEATURE_DEBUG([xdt_debug_default])

dnl Set PACKAGE PREFIX
if test "x${prefix}" = "xNONE"; then
  packageprefix=${ac_default_prefix}
else
  packageprefix=${prefix}
fi

dnl Set PACKAGE DATA & DOC DIR
packagedatadir=share

dnl Subst PACKAGE_DATA_DIR.
NO_PREFIX_PACKAGE_DATA_DIR="${packagedatadir}"
AC_SUBST(NO_PREFIX_PACKAGE_DATA_DIR)
PACKAGE_DATA_DIR="${packageprefix}/${packagedatadir}"
AC_SUBST(PACKAGE_DATA_DIR)

AC_DEFINE_UNQUOTED([PACKAGE_DATA_DIR], "${packageprefix}/${packagedatadir}", ["Data dir"])

AC_CONFIG_FILES([
Makefile
panel-plugin/Makefile
icons/Makefile
po/Makefile.in
])
AC_OUTPUT
