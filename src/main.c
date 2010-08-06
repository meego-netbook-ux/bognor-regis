/*
 * Bognor-Regis - a media player/queue daemon.
 * Copyright © 2009, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>
#include <stdio.h>

#include <glib.h>
#include <glib/gi18n.h>

#ifdef ENABLE_GST
#include <gst/gst.h>
#endif
#include <gtk/gtk.h>

#include <dbus/dbus-protocol.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libnotify/notify.h>

#include "bgr-tracker-client.h"
#include "bognor-marshal.h"
#include "bognor-queue-manager.h"
#include "bognor-upnp-cp.h"

typedef struct BognorRegis {
    DBusGConnection *connection;
    DBusGProxy *proxy; /* Do we need to keep this around? */
    DBusGProxy *mmkeys;

    BgrTrackerClient *tracker;
    BognorQueueManager *manager;
    BognorUpnpCp *control_point;
    GMainLoop *mainloop;
} BognorRegis;

BognorRegis *br;

#define BOGNOR_NAME "org.moblin.BognorRegis"
#define BOGNOR_MANAGER_PATH "/org/moblin/BognorRegis/QueueManager"

static void
grab_keys (void)
{
    if (br->mmkeys) {
        dbus_g_proxy_call (br->mmkeys, "GrabMediaPlayerKeys", NULL,
                           G_TYPE_STRING, "Bognor-Regis",
                           G_TYPE_UINT, 0,
                           G_TYPE_INVALID, G_TYPE_INVALID);
    }
}

static void
mmkeys_press_cb (DBusGProxy *proxy,
                 const char *application,
                 const char *key,
                 gpointer    userdata)
{
    if (g_str_equal (application, "Bognor-Regis")) {
        BognorQueueManagerAction action;

        if (g_str_equal (key, "Play")) {
            action = BOGNOR_QUEUE_MANAGER_ACTION_PLAY;
        } else if (g_str_equal (key, "Stop")) {
            action = BOGNOR_QUEUE_MANAGER_ACTION_PAUSE;
        } else if (g_str_equal (key, "Next")) {
            action = BOGNOR_QUEUE_MANAGER_ACTION_NEXT;
        } else if (g_str_equal (key, "Previous")) {
            action = BOGNOR_QUEUE_MANAGER_ACTION_PREVIOUS;
        } else {
            return;
        }

        bognor_queue_manager_action (br->manager, action);
    }
}

static void
mmkeys_destroyed_cb (DBusGProxy *proxy,
                     gpointer    userdata)
{
    br->mmkeys = NULL;
}

static gboolean
init_dbus (void)
{
    guint32 request_name_ret;
    GError *error = NULL;

    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    br->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
    if (br->connection == NULL) {
        g_warning ("Error getting bus: %s", error->message);
        g_error_free (error);

        return FALSE;
    }

    br->proxy = dbus_g_proxy_new_for_name (br->connection,
                                           DBUS_SERVICE_DBUS,
                                           DBUS_PATH_DBUS,
                                           DBUS_INTERFACE_DBUS);
    if (!org_freedesktop_DBus_request_name (br->proxy, BOGNOR_NAME,
                                            0, &request_name_ret, &error)) {
        g_warning ("Error registering on DBus: %s", error->message);
        g_error_free (error);

        return FALSE;
    }

    if (request_name_ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        return FALSE;
    }

    br->tracker = g_object_new (BGR_TYPE_TRACKER_CLIENT, NULL);

    br->manager = bognor_queue_manager_new (br->connection, br->tracker);
    dbus_g_connection_register_g_object (br->connection, BOGNOR_MANAGER_PATH,
                                         G_OBJECT (br->manager));


    br->mmkeys = dbus_g_proxy_new_for_name_owner (br->connection,
                                                  "org.gnome.SettingsDaemon",
                                                  "/org/gnome/SettingsDaemon/MediaKeys",
                                                  "org.gnome.SettingsDaemon.MediaKeys",
                                                  &error);
    if (error != NULL) {
        g_warning ("Could not setup multimedia-keys: %s", error->message);
        g_error_free (error);
        return TRUE;
    }
    g_signal_connect (br->mmkeys, "destroy",
                      G_CALLBACK (mmkeys_destroyed_cb), br);

    dbus_g_object_register_marshaller (bognor_marshal_VOID__STRING_STRING,
                                       G_TYPE_NONE, G_TYPE_STRING,
                                       G_TYPE_STRING, G_TYPE_INVALID);
    dbus_g_proxy_add_signal (br->mmkeys, "MediaPlayerKeyPressed",
                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
    dbus_g_proxy_connect_signal (br->mmkeys, "MediaPlayerKeyPressed",
                                 G_CALLBACK (mmkeys_press_cb), br, NULL);

    grab_keys ();

    return TRUE;
}

static void
init_upnp (void)
{
    BognorQueue *local;

    local = bognor_queue_manager_get_local_queue (br->manager);
    br->control_point = bognor_upnp_cp_new (local);
}

int
main (int    argc,
      char **argv)
{
    g_thread_init (NULL);
    g_type_init ();
#ifdef ENABLE_GST
    gst_init (&argc, &argv);
#endif
    gtk_init (&argc, &argv);

    br = g_new0 (BognorRegis, 1);

    if (init_dbus () == FALSE) {
        return 0;
    }

    notify_init ("Bognor-Regis Media Daemon");

    init_upnp ();

    br->mainloop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (br->mainloop);

    return 0;
}
