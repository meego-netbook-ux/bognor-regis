servicedir = $(DBUS_SERVICES_DIR)
service_in_files = org.moblin.BognorRegis.service.in
service_DATA = $(service_in_files:.service.in=.service)

org.moblin.BognorRegis.service: org.moblin.BognorRegis.service.in
	@sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

bognor-regis-@BOGNOR_MAJORMINOR@.pc: bognor-regis.pc
	@cp -f bognor-regis.pc bognor-regis-@BOGNOR_MAJORMINOR@.pc

pkgconfig_DATA = bognor-regis-@BOGNOR_MAJORMINOR@.pc
pkgconfigdir = $(libdir)/pkgconfig

EXTRA_DIST = org.moblin.BognorRegis.service.in	\
	bognor-regis.pc.in

CLEANFILES = org.moblin.BognorRegis.service	\
	bognor-regis-@BOGNOR_MAJORMINOR@.pc

MAINTAINERCLEANFILES = Makefile.in

