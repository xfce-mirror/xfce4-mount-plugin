dnl configure.ac
dnl
dnl xfce4-mount-plugin - Mount plugin for xfce4-panel
dnl
dnl 2003,2004 Benedikt Meurer <benny@xfce.org>
dnl 2005 Fabian Nowak <timystery@arcor.de>
dnl

AC_INIT([panel-plugin/mount-plugin.c])

AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([xfce4-mount-plugin], [0.3.3])

AM_MAINTAINER_MODE

dnl Check for UNIX variants
AC_AIX
AC_ISC_POSIX
AC_MINIX

dnl Check for basic programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl Check for standard header files
AC_HEADER_STDC

dnl configure the panel plugin
XFCE_PANEL_PLUGIN([XFCE4_PANEL], [4.0.0])

dnl I18n support
BM_I18N([xfce4-mount-plugin], [de el fr hu ko lt ru uk])

dnl Check for debugging support
BM_DEBUG_SUPPORT

AC_OUTPUT([
Makefile
xfce4-mount-plugin.spec
panel-plugin/Makefile
po/Makefile.in
])
