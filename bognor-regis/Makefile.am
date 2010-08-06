lib_LTLIBRARIES = libbognor-regis-@BOGNOR_MAJORMINOR@.la

BUILT_SOURCES =				\
	br-marshal.c			\
	br-marshal.h			\
	br-iface-player-glue.h		\
	bognor-queue-bindings.h

libbognor_regis_@BOGNOR_MAJORMINOR@_la_SOURCES = 		\
	$(BUILT_SOURCES)		\
	br-iface-player.c		\
	br-queue.c

libbognor_regis_@BOGNOR_MAJORMINOR@_la_CFLAGS = 		\
	-I$(top_srcdir)			\
	-I$(top_builddir)		\
	$(BOGNOR_REGIS_CFLAGS)

libbognor_regis_@BOGNOR_MAJORMINOR@_la_LIBADD =		\
	$(BOGNOR_REGIS_LIBS)

bognor_regis_headers = 			\
	br-iface-player.h		\
	br-queue.h

libbognor_regis_includedir = $(includedir)/bognor-regis-@BOGNOR_MAJORMINOR@/bognor
libbognor_regis_include_HEADERS =	\
	$(bognor_regis_headers)

if HAVE_QUEUE_CONFIG
schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = bognor-regis.schemas
endif

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schema_DATA)
else
install-data-local:
endif

CLEANFILES = $(BUILT_SOURCES)		\
	stamp-br-iface-player-glue.h	\
	stamp-bognor-queue-bindings.h

%-bindings.h: stamp-%-bindings.h
	@true
stamp-bognor-queue-bindings.h: ../interfaces/BognorQueue.xml
	$(AM_V_GEN)$(DBUS_BINDING_TOOL) --mode=glib-client --prefix=bognor_queue $< > xgen-$(@F) \
	&& (cmp -s xgen-$(@F) $(@F:stamp-%=%) || cp xgen-$(@F) $(@F:stamp-%=%)) \
	&& rm -f xgen-$(@F) \
	&& echo timestamp > $(@F)

%-glue.h: stamp-%-glue.h
	@true
stamp-br-iface-player-glue.h: ../interfaces/BognorPlayer.xml
	$(AM_V_GEN)$(DBUS_BINDING_TOOL) --mode=glib-server --prefix=br_iface_player $< > xgen-$(@F) \
	&& (cmp -s xgen-$(@F) $(@F:stamp-%=%) || cp xgen-$(@F) $(@F:stamp-%=%)) \
	&& rm -f xgen-$(@F) \
	&& echo timestamp > $(@F)

br-marshal.h: br-marshal.list
	$(AM_V_GEN)$(GLIB_GENMARSHAL) $< --header --prefix=br_marshal > $@
br-marshal.c: br-marshal.list
	$(AM_V_GEN)echo "#include \"br-marshal.h\"" > $@ \
	&& $(GLIB_GENMARSHAL) $< --body --prefix=br_marshal >> $@

MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST=$(schema_DATA) br-marshal.list
