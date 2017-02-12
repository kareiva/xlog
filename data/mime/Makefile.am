xml_files = xlog.xml

newmimedir = $(datadir)/mime/packages
newmime_DATA = $(xml_files)

EXTRA_DIST = $(newmime_DATA)

if MIMEUPDATE
install-data-hook:
if HAVE_MIME
	$(UPDATE_MIME_DATABASE) "$(DESTDIR)$(datadir)/mime"
endif
endif

if MIMEUPDATE
uninstall-hook:
if HAVE_MIME
	$(UPDATE_MIME_DATABASE) "$(DESTDIR)$(datadir)/mime"
endif
endif
