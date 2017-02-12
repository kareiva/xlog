dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)
AC_INIT(xlog, 2.0.14, kb1oiq@arrl.net)
AC_CONFIG_SRCDIR([src/logfile/flog.c])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE
dnl m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AC_PROG_RANLIB

dnl check for headers
AC_HEADER_STDC
AC_CHECK_HEADERS(sys/ipc.h sys/shm.h)

dnl check for win32 functions
AC_FUNC_STRPTIME
AC_CHECK_FUNCS(strchr index)

if test "${ac_cv_c_compiler_gnu}" = "yes"; then
  CFLAGS="${CFLAGS} -Wall -Wno-unused-but-set-variable -Werror"
fi

PKG_CHECK_MODULES(GLIB, glib-2.0)
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.18.0)
PKG_CHECK_MODULES(HAMLIB, hamlib)

dnl ---------------------------------------------------------------------------
dnl - Is freedesktop mime dbase available?
dnl ---------------------------------------------------------------------------
AC_PATH_PROG(UPDATE_MIME_DATABASE, update-mime-database, no)
AM_CONDITIONAL(HAVE_MIME, test x$UPDATE_MIME_DATABASE != xno)

AC_PATH_PROG(UPDATE_DESKTOP_DATABASE, update-desktop-database, no)
AM_CONDITIONAL(HAVE_DESKTOP, test x$UPDATE_DESKTOP_DATABASE != xno)

GETTEXT_PACKAGE=xlog
AC_SUBST(GETTEXT_PACKAGE)
dnl Add the languages which your application supports here.
ALL_LINGUAS="cs de es fr it nl pl pt sk sv"
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.18.3)

AC_ARG_ENABLE(mime-update,
[  --enable-mime-update    update the mime database],
[case "${enableval}" in
  yes) mime_update=true ;;
  no)  mime_update=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-mime-update) ;;
esac],[mime_update=false])
AM_CONDITIONAL(MIMEUPDATE, test x$mime_update = xtrue)

if test "x${mime_update}" != "xfalse"; then
	echo "enabled updating of the mime database"
else
	echo "disabled updating of the mime database"
fi


AC_OUTPUT([po/Makefile.in
m4/Makefile
Makefile
data/man/xlog.1
src/Makefile
src/logfile/Makefile
data/Makefile
data/desktop/Makefile
data/pixmaps/Makefile
data/mime/Makefile
data/man/Makefile
data/maps/Makefile
data/maps/3D2/Makefile
data/maps/3Y/Makefile
data/maps/E5/Makefile
data/maps/FK/Makefile
data/maps/FO/Makefile
data/maps/FR/Makefile
data/maps/HK0/Makefile
data/maps/JD/Makefile
data/maps/KH8/Makefile
data/maps/SV/Makefile
data/maps/VP6/Makefile
data/maps/VP8/Makefile
data/doc/Makefile
data/doc/manual/Makefile
data/doc/manual/output/Makefile
data/doc/manual/output/html/Makefile
data/doc/manual/output/html/search/Makefile
data/dxcc/Makefile
data/remote/Makefile
data/glabels/Makefile
data/utils/Makefile
data/images/Makefile
])
