filterdir=$(libdir)/cups/filter

filter_PROGRAMS= cmdtocanonij2

cmdtocanonij2_SOURCES= \
	cmdtocanonij.c cnijutil.c

cmdtocanonij2_LDADD= -lcnbpcnclapicom2
AM_CFLAGS= -O2 -Wall -ldl
