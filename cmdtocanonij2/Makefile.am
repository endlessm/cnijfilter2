SUBDIRS = filter 

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir); \
	if test -d $(srcdir)/utilfiles; then \
	  for utilfiles in $(srcdir)/utilfiles/*; do \
	    if test -f $$utilfiles; then \
	      $(INSTALL_DATA) $$utilfiles $(DESTDIR)$(pkgdatadir); \
	    fi \
	  done \
	fi

dist-hook:
	mkdir $(distdir); \
	if test -d utilfiles; then \
	  mkdir $(distdir)/utilfiles; \
	  for utilfiles in utilfiles/*; do \
	    if test -f $$utilfiles; then \
	      cp -p $$utilfiles $(distdir)/utilfiles; \
	    fi \
	  done \
	fi

