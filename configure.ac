dnl configure.ac
dnl
dnl xfce4-mount-plugin - Mount plugin for xfce4-panel
dnl
dnl 2003,2004 Benedikt Meurer <benny@xfce.org>
dnl 2005 Fabian Nowak <timystery@arcor.de>
dnl
dnl for Xfce4-panel 4.3 and higher only!

m4_define([mount_version],[0.4.3])

AC_INIT([xfce4-mount-plugin], [mount_version()],
	[timystery@arcor.de])

MOUNT_VERSION=mount_version()
AM_INIT_AUTOMAKE([xfce4-mount-plugin], [$MOUNT_VERSION])
AM_CONFIG_HEADER([config.h])

AM_MAINTAINER_MODE

dnl Check for UNIX variants
AC_AIX
AC_ISC_POSIX
AC_MINIX

dnl Check for basic programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_INTLTOOL

dnl Check for standard header files
AC_HEADER_STDC

dnl Substitute in Makefiles
AC_SUBST(OS)

AC_CHECK_FUNCS([gethostname])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([strstr])
AC_CHECK_FUNCS([malloc])
AC_CHECK_FUNCS([bzero])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([sys/ioctl.h])
AC_CHECK_HEADERS([sys/param.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([sys/sockio.h])
AC_HEADER_SYS_WAIT
AC_PROG_GCC_TRADITIONAL
AC_TYPE_SIZE_T

AC_CHECK_LIB(kstat, kstat_open, SOLLIBS="-lkstat -lsocket", SOLLIBS="")
AC_CHECK_LIB(nsl, kstat_open, SOLLIBS="$SOLLIBS -linet_ntop", SOLLIBS="$SOLLIBS")
AC_SUBST(SOLLIBS)

dnl configure the panel plugin
XDT_CHECK_PACKAGE([LIBXFCE4PANEL], [libxfce4panel-1.0], [4.3.20])

dnl Check for i18n support
XDT_I18N([de el eu fr hu ko lt ru uk])

dnl Check for debugging support
XDT_FEATURE_DEBUG()

AC_OUTPUT([
Makefile
xfce4-mount-plugin.spec
panel-plugin/Makefile
icons/Makefile
po/Makefile.in
])

