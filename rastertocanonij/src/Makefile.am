filterdir=$(libdir)/cups/filter

filter_PROGRAMS= rastertocanonij

rastertocanonij_SOURCES= \
	main.c getsettings.c paramlist.c canonopt.c common.c

rastertocanonij_LDADD= -lcups

AM_CFLAGS= -O2 -Wall

