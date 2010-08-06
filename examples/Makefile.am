noinst_PROGRAMS = bognor-regis
AM_CFLAGS = $(BOGNOR_REGIS_CFLAGS) \
	 $(CLIENT_CFLAGS)	\
	 -I$(top_srcdir)/bognor-regis	\
	 -I$(top_srcdir)
AM_LDFLAGS = $(BOGNOR_REGIS_LIBS) $(CLIENT_LIBS) $(top_builddir)/bognor-regis/libbognor-regis-@BOGNOR_MAJORMINOR@.la

bognor_regis_SOURCES = bognor-regis.c
bognor_regis_DEPENDENCIES = $(top_builddir)/bognor-regis/libbognor-regis-@BOGNOR_MAJORMINOR@.la

MAINTAINERCLEANFILES = Makefile.in
