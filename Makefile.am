## Process this file with automake to produce Makefile.in

SUBDIRS = src po data m4

EXTRA_DIST = config.rpath mkinstalldirs

CLEANFILES = *~

ACLOCAL_AMFLAGS = -I m4

dist-hook:
	rm -f AUTHORS ChangeLog
	ln -s data/doc/AUTHORS
	ln -s data/doc/ChangeLog
