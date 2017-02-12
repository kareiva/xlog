DESKTOP_FILES = xlog.desktop

desktopdir = $(datadir)/applications
desktop_DATA = $(DESKTOP_FILES)

EXTRA_DIST = $(DESKTOP_FILES)

install-data-hook:
if HAVE_DESKTOP
	$(UPDATE_DESKTOP_DATABASE) "$(DESTDIR)$(datadir)/applications"
endif

uninstall-hook:
if HAVE_DESKTOP
	$(UPDATE_DESKTOP_DATABASE) "$(DESTDIR)$(datadir)/applications"
endif
