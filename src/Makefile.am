libexec_PROGRAMS = bognor-regis-daemon

common_sources = 			\
	bgr-item.c			\
	bgr-item.h			\
	bgr-tracker-client.c		\
	bgr-tracker-client.h		\
	bognor-av-transport.c		\
	bognor-av-transport.h		\
	bognor-connection-manager.c	\
	bognor-connection-manager.h	\
	bognor-last-change.c		\
	bognor-last-change.h		\
	bognor-local-queue.h		\
	bognor-queue.c			\
	bognor-queue.h			\
	bognor-queue-item.c		\
	bognor-queue-item.h		\
	bognor-queue-manager.c		\
	bognor-queue-manager.h		\
	bognor-render-control.c		\
	bognor-render-control.h		\
	bognor-upnp-cp.c		\
	bognor-upnp-cp.h		\
	main.c

if HELIX

bognor_regis_daemon_CFLAGS = 		\
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	-I$(top_builddir)		\
        -DPKGDATADIR=\"$(pkgdatadir)/\" \
        -DLOCALEDIR=\""$(localedir)"\"  \
	-DSCPDDIR=\""$(datadir)/bognor-regis/scpd"\" \
	$(BOGNOR_CFLAGS) $(HELIX_CFLAGS)

bognor_regis_daemon_LDADD =		\
	$(BOGNOR_LIBS) $(HELIX_LIBS)

bognor_regis_daemon_SOURCES =		\
	$(BUILT_SOURCES)		\
	bognor-local-queue-helix.c	\
	$(common_sources)

else

bognor_regis_daemon_CFLAGS = 		\
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	-I$(top_builddir)		\
        -DPKGDATADIR=\"$(pkgdatadir)/\" \
        -DLOCALEDIR=\""$(localedir)"\"  \
        -DSCPDDIR=\""$(datadir)/bognor-regis/scpd"\" \
	$(BOGNOR_CFLAGS) $(GST_CFLAGS)

bognor_regis_daemon_LDADD =		\
	$(BOGNOR_LIBS) $(GST_LIBS)

bognor_regis_daemon_SOURCES =		\
	$(BUILT_SOURCES)		\
	bognor-local-queue-gst.c	\
	$(common_sources)

endif

BUILT_SOURCES =				\
	bognor-marshal.c		\
	bognor-marshal.h		\
	bognor-player-bindings.h	\
	bognor-queue-glue.h		\
	bognor-queue-manager-glue.h

EXTRA_DIST = $(BUILT_SOURCES) bognor-marshal.list

CLEANFILES = $(BUILT_SOURCES)		\
	stamp-bognor-queue-glue.h	\
	stamp-bognor-queue-manager-glue.h \
	stamp-bognor-player-bindings.h

MAINTAINTERCLEANFILES = Makefile.in

%-glue.h: stamp-%-glue.h
	@true
stamp-bognor-queue-glue.h: ../interfaces/BognorQueue.xml
	$(AM_V_GEN)$(DBUS_BINDING_TOOL) --prefix=bognor_queue --mode=glib-server $< > xgen-$(@F) \
	&& (cmp -s xgen-$(@F) $(@F:stamp-%=%) || cp xgen-$(@F) $(@F:stamp-%=%)) \
	&& rm -f xgen-$(@F) \
	&& echo timestamp > $(@F)
stamp-bognor-queue-manager-glue.h: ../interfaces/BognorQueueManager.xml
	$(AM_V_GEN)$(DBUS_BINDING_TOOL) --prefix=bognor_queue_manager --mode=glib-server $< > xgen-$(@F) \
	&& (cmp -s xgen-$(@F) $(@F:stamp-%=%) || cp xgen-$(@F) $(@F:stamp-%=%)) \
	&& rm -f xgen-$(@F) \
	&& echo timestamp > $(@F)

%-bindings.h: stamp-%-bindings.h
	@true
stamp-bognor-player-bindings.h: ../interfaces/BognorPlayer.xml
	$(AM_V_GEN)$(DBUS_BINDING_TOOL) --prefix=bognor_player --mode=glib-client $< > xgen-$(@F) \
	&& (cmp -s xgen-$(@F) $(@F:stamp-%=%) || cp xgen-$(@F) $(@F:stamp-%=%)) \
	&& rm -f xgen-$(@F) \
	&& echo timestamp > $(@F)

bognor-marshal.h: bognor-marshal.list
	$(AM_V_GEN)$(GLIB_GENMARSHAL) $< --header --prefix=bognor_marshal > $@
bognor-marshal.c: bognor-marshal.list
	@echo "#include \"bognor-marshal.h\"" > $@ \
	&& $(GLIB_GENMARSHAL) $< --body --prefix=bognor_marshal >> $@
