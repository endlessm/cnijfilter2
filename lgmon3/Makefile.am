## Process this file with automake to produce Makefile.in

SUBDIRS = po src

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir); \
	if test -d $(srcdir)/keytext; then \
	  for keytext in $(srcdir)/keytext/*; do \
	    if test -f $$keytext; then \
	      $(INSTALL_DATA) $$keytext $(DESTDIR)$(pkgdatadir); \
	    fi \
	  done \
	fi

dist-hook:
	mkdir $(distdir); \
	if test -d keytext; then \
	  mkdir $(distdir)/keytext; \
	  for keytext in keytext/*; do \
	    if test -f $$keytext; then \
	      cp -p $$keytext $(distdir)/keytext; \
	    fi \
	  done \
	fi

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = mkinstalldirs  config.rpath
