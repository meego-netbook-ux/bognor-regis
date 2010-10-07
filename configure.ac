# bognor package version number, (as distinct from shared library version)
# An odd micro number indicates in-progress developmen
# An even micro number indicates a released version.
m4_define(bognor_major_version, 0)
m4_define(bognor_minor_version, 6)
m4_define(bognor_micro_version, 12)

m4_define([bognor_version],
          [bognor_major_version.bognor_minor_version.bognor_micro_version])

m4_define([bognor_api_version],
          [bognor_major_version.bognor_minor_version])

# increase the interface age for each release; if the API changes, set to 0
m4_define([bognor_interface_age], [0])
m4_define([bognor_binary_age],
          [m4_eval(100 * bognor_minor_version + bognor_micro_version)])

AC_PREREQ(2.53)

AC_INIT([bognor-regis], [bognor_version], [http://bugzilla.moblin.org])

AC_CONFIG_SRCDIR(src/bognor-queue.c)
AC_CONFIG_MACRO_DIR(m4)

AM_INIT_AUTOMAKE([1.9])
AM_CONFIG_HEADER([config.h])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

GETTEXT_PACKAGE=bognor-regis
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [The gettext package name])

IT_PROG_INTLTOOL([0.34.90])

BOGNOR_MAJOR_VERSION=bognor_major_version
BOGNOR_MINOR_VERSION=bognor_minor_version
BOGNOR_MICRO_VERSION=bognor_micro_version
BOGNOR_VERSION=bognor_version
BOGNOR_API_VERSION=bognor_api_version
BOGNOR_MAJORMINOR=bognor_api_version
AC_SUBST(BOGNOR_MAJOR_VERSION)
AC_SUBST(BOGNOR_MINOR_VERSION)
AC_SUBST(BOGNOR_MICRO_VERSION)
AC_SUBST(BOGNOR_VERSION)
AC_SUBST(BOGNOR_API_VERSION)
AC_SUBST(BOGNOR_MAJORMINOR)

m4_define([lt_current],
          [m4_eval(100 * bognor_minor_version + bognor_micro_version - bognor_interface_age)])
m4_define([lt_revision], [bognor_interface_age])
m4_define([lt_age], [m4_eval(bognor_binary_age - bognor_interface_age)])

BOGNOR_LT_CURRENT=lt_current
BOGNOR_LT_REV=lt_revision
BOGNOR_LT_AGE=lt_age
BOGNOR_LT_VERSION="$BOGNOR_LT_CURRENT:$BOGNOR_LT_REV:$BOGNOR_LT_AGE"
BOGNOR_LT_LDFLAGS="-version-info $BOGNOR_LT_VERSION"

AC_SUBST(BOGNOR_LT_VERSION)
AC_SUBST(BOGNOR_LT_LDFLAGS)

AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_LIBTOOL

AC_ARG_ENABLE(queue-config,
	AC_HELP_STRING([--enable-queue-config],[Build with configurable queue service]),
    enable_queue_config="$enableval",
    enable_queue_config=no)

gconf_pkg=
if test "x$enable_queue_config" = "xyes" ; then
    AC_DEFINE(USE_QUEUE_CONFIG, 1, [Build with configurable queue service])

    AC_PATH_PROG(GCONFTOOL, gconftool-2, no)
    if test x"$GCONFTOOL" = xno; then
        AC_MSG_ERROR([gconftool-2 executable not found in your path - should be installed with GConf])
    fi

    AM_GCONF_SOURCE_2
    gconf_pkg=gconf-2.0
else
    GCONF_SCHEMAS_INSTALL_TRUE='#'
    GCONF_SCHEMAS_INSTALL_FALSE=
fi
AM_CONDITIONAL(HAVE_QUEUE_CONFIG, test "x$enable_queue_config" = "xyes")

PKG_CHECK_MODULES(BOGNOR, gtk+-2.0 glib-2.0 dbus-glib-1 gio-2.0 libnotify tracker-client-0.9 gupnp-1.0 >= 0.13.0 gupnp-av-1.0 >= 0.5)
PKG_CHECK_MODULES(BOGNOR_REGIS, glib-2.0 dbus-glib-1 $gconf_pkg)
PKG_CHECK_MODULES(CLIENT, gio-2.0)

AC_ARG_ENABLE(helix, AC_HELP_STRING([--enable-helix], [enable helix support]),
              [enable_helix=${enableval}], [enable_helix="no"])
AS_IF([test "${enable_helix}" = "yes"], 
    [
        PKG_CHECK_MODULES(HELIX, hxclient)
        AC_SUBST(HELIX_CFLAGS)
        AC_SUBST(HELIX_LIBS)
        AC_DEFINE(ENABLE_HELIX, [1], [Use Helix Client])
    ],
    [
        PKG_CHECK_MODULES(GST, gstreamer-0.10)
        AC_SUBST(GST_CFLAGS)
        AC_SUBST(GST_LIBS)
        AC_DEFINE(ENABLE_GST, [1], [Use Gstreamer Media Framework])
    ])
AM_CONDITIONAL(HELIX, test "${enable_helix}" = "yes")
AM_CONDITIONAL(GST, test "${enable_helix}" = "no")

AC_PATH_PROG(DBUS_BINDING_TOOL, dbus-binding-tool)
DBUS_SERVICES_DIR="${datadir}/dbus-1/services"
AC_SUBST(DBUS_SERVICES_DIR)

AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

AS_ALL_LINGUAS

AC_OUTPUT([
Makefile
src/Makefile
bognor-regis/Makefile
examples/Makefile
po/Makefile.in
data/Makefile
data/bognor-regis.pc
interfaces/Makefile
scpd/Makefile
])
