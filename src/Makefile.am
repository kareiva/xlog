## Process this file with automake to produce Makefile.in

SUBDIRS = logfile

AM_CPPFLAGS = \
	@GTK_CFLAGS@ @HAMLIB_CFLAGS@ \
	-DDATADIR=\""$(datadir)"\" \
	-DXLOG_LOCALEDIR=\""$(datadir)/locale"\" \
	-DXLOG_DATADIR=\""$(datadir)/xlog"\" \
	-DXLOG_DOCDIR=\""$(docdir)"\"

CLEANFILES = *~

bin_PROGRAMS = xlog

xlog_SOURCES = \
	main.c main.h \
	support.c support.h \
	gui_aboutdialog.c gui_aboutdialog.h \
	gui_b4window.c gui_b4window.h \
	gui_closedialog.c gui_closedialog.h\
	gui_countrymap.c gui_countrymap.h\
	gui_defaultsdialog.c gui_defaultsdialog.h\
	gui_dialogsdialog.c gui_dialogsdialog.h\
	gui_dupecheckdialog.c gui_dupecheckdialog.h\
	gui_dxcccheck.c gui_dxcccheck.h\
	gui_dxcclist.c gui_dxcclist.h\
	gui_openlogdialog.c gui_openlogdialog.h\
	gui_fontselectiondialog.c gui_fontselectiondialog.h\
	gui_helpdialog.c gui_helpdialog.h\
	gui_keys.c gui_keys.h\
	gui_logeditordialog.c gui_logeditordialog.h\
	gui_mainwindow.c gui_mainwindow.h \
	gui_newlogdialog.c gui_newlogdialog.h\
	gui_pathselectiondialog.c gui_pathselectiondialog.h\
	gui_preferencesdialog.c gui_preferencesdialog.h \
	gui_gtkprintdialog.c gui_gtkprintdialog.h \
	gui_saveasdialog.c gui_saveasdialog.h\
	gui_savedialog.c gui_savedialog.h \
	gui_scorewindow.c gui_scorewindow.h \
	gui_searchdialog.c gui_searchdialog.h \
	gui_setupdialog.c gui_setupdialog.h \
	gui_tracedialog.c gui_tracedialog.h\
	gui_utils.c gui_utils.h\
	gui_warningdialog.c gui_warningdialog.h\
	callbacks_preferencesdialog.c callbacks_preferencesdialog.h \
	callbacks_mainwindow_menu.c callbacks_mainwindow_menu.h \
	callbacks_mainwindow_toolbar.c callbacks_mainwindow_toolbar.h \
	callbacks_mainwindow_qsoframe.c callbacks_mainwindow_qsoframe.h \
	callbacks_mainwindow_list.c callbacks_mainwindow_list.h \
	callbacks_mainwindow.c callbacks_mainwindow.h \
	cfg.c cfg.h \
	hamlib-utils.c hamlib-utils.h \
	utils.c utils.h \
	dxcc.c dxcc.h \
	gc.c gc.h \
	remote.c remote.h \
	history.c history.h \
	log.c log.h \
	wwl.c wwl.h \
	strptime.c strptime.h \
	netkeyer.c netkeyer.h gui_netkeyer.c gui_netkeyer.h \
	gui_mergedialog.c gui_mergedialog.h \
	gui_importdialog.c gui_importdialog.h \
	gui_exportdialog.c gui_exportdialog.h \
	gui_awards_dxcc.c gui_awards_dxcc.h \
	gui_awards_wac.c gui_awards_wac.h \
	gui_awards_was.c gui_awards_was.h \
	gui_awards_waz.c gui_awards_waz.h \
	gui_awards_iota.c gui_awards_iota.h \
	gui_awards_locator.c gui_awards_locator.h \
	xlog_enum.c xlog_enum.h \
	awards_enum.c awards_enum.h

xlog_LDADD = logfile/liblogfile.a \
	@GTK_LIBS@ @HAMLIB_LIBS@ -lm

