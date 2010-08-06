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

#include <stdio.h>

#include <glib.h>

#include <dbus/dbus-protocol.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "bgr-tracker-client.h"
#include "bognor-local-queue.h"
#include "bognor-queue-manager.h"

enum {
    PROP_0,
    PROP_TRACKER,
};

enum {
    QUEUE_CREATED,
    QUEUE_DESTROYED,
    LAST_SIGNAL
};

struct _BognorQueueManagerPrivate {
    DBusGConnection *connection;
    GList *remote_queues;
    BognorQueue *local_queue;
    BgrTrackerClient *tracker;
};

#define BOGNOR_QUEUE_PATH "/org/moblin/BognorRegis/Queues/"

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), BOGNOR_TYPE_QUEUE_MANAGER, BognorQueueManagerPrivate))
G_DEFINE_TYPE (BognorQueueManager, bognor_queue_manager, G_TYPE_OBJECT);

static guint32 signals[LAST_SIGNAL] = {0, };

static gboolean bognor_queue_manager_list_queues (BognorQueueManager *manager,
                                                  char             ***queue_paths,
                                                  GError            **error);

#include "bognor-queue-manager-glue.h"

static void
bognor_queue_manager_finalize (GObject *object)
{
    BognorQueueManager *self = (BognorQueueManager *) object;

    g_signal_handlers_destroy (object);
    G_OBJECT_CLASS (bognor_queue_manager_parent_class)->finalize (object);
}

static void
bognor_queue_manager_dispose (GObject *object)
{
    BognorQueueManager *self = (BognorQueueManager *) object;

    G_OBJECT_CLASS (bognor_queue_manager_parent_class)->dispose (object);
}

static void
bognor_queue_manager_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
    BognorQueueManager *self = (BognorQueueManager *) object;
    BognorQueueManagerPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_TRACKER:
        priv->tracker = g_value_get_object (value);
        break;

    default:
        break;
    }
}

static void
bognor_queue_manager_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
    BognorQueueManager *self = (BognorQueueManager *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static gboolean
bognor_queue_manager_list_queues (BognorQueueManager *manager,
                                  char             ***queue_paths,
                                  GError            **error)
{
    BognorQueueManagerPrivate *priv = manager->priv;
    char **paths;
    int num_queues, i;
    GList *q;

    num_queues = g_list_length (priv->remote_queues);
    paths = g_new (char *, num_queues + 1);

    for (q = priv->remote_queues, i = 0; q; q = q->next, i++) {
        /* FIXME: Get some paths here */
    }

    /* NULL terminate array */
    paths[num_queues - 1] = NULL;

    *queue_paths = paths;
    return TRUE;
}

static void
bognor_queue_manager_class_init (BognorQueueManagerClass *klass)
{
    GObjectClass *o_class = (GObjectClass *)klass;

    o_class->dispose = bognor_queue_manager_dispose;
    o_class->finalize = bognor_queue_manager_finalize;
    o_class->set_property = bognor_queue_manager_set_property;
    o_class->get_property = bognor_queue_manager_get_property;

    g_type_class_add_private (klass, sizeof (BognorQueueManagerPrivate));
    dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass),
                                     &dbus_glib_bognor_queue_manager_object_info);

    g_object_class_install_property (o_class, PROP_TRACKER,
                                     g_param_spec_object ("tracker",
                                                          "", "",
                                                          BGR_TYPE_TRACKER_CLIENT,
                                                          G_PARAM_CONSTRUCT_ONLY |
                                                          G_PARAM_WRITABLE |
                                                          G_PARAM_STATIC_STRINGS));

    signals[QUEUE_CREATED] = g_signal_new ("queue-created",
                                           G_TYPE_FROM_CLASS (klass),
                                           G_SIGNAL_RUN_FIRST |
                                           G_SIGNAL_NO_RECURSE, 0, NULL, NULL,
                                           g_cclosure_marshal_VOID__STRING,
                                           G_TYPE_NONE, 1,
                                           G_TYPE_STRING);
    signals[QUEUE_DESTROYED] = g_signal_new ("queue-destroyed",
                                             G_TYPE_FROM_CLASS (klass),
                                             G_SIGNAL_RUN_FIRST |
                                             G_SIGNAL_NO_RECURSE, 0, NULL, NULL,
                                             g_cclosure_marshal_VOID__STRING,
                                             G_TYPE_NONE, 1,
                                             G_TYPE_STRING);
}

static void
bognor_queue_manager_init (BognorQueueManager *self)
{
    BognorQueueManagerPrivate *priv;

    self->priv = GET_PRIVATE (self);
    priv = self->priv;
}

static void
load_queues (BognorQueueManager *manager)
{
    BognorQueueManagerPrivate *priv;
    char *bognor_queue_dir;
    GDir *dir;

    priv = manager->priv;

    /* Ensure the correct dir is in place */
    bognor_queue_dir = g_build_filename (g_get_home_dir (), ".bognor-regis",
                                         NULL);
    g_mkdir_with_parents (bognor_queue_dir, 0777);

#if 0
    dir = g_dir_open (bognor_queue_dir, 0, &error);
    if (dir == NULL) {
        g_warning ("Error opening %s: %s", bognor_queue_dir, error->message);
        g_error_free (error);
        g_free (bognor_queue_dir);
        return;
    }

    while (name = g_dir_read_name (dir)) {
        if (g_str_has_suffix (name, ".m3u")) {
            char *path = g_build_filename (bognor_queue_dir, name, NULL);
            char *object_path;
            BognorQueue *q;

            if (g_str_equal (name, "local-queue.m3u")) {
                q = bognor_local_queue_new ();
                br->local_queue = q;
            } else {
                q = NULL;
                /* br->remote_queues = g_list_prepend (br->remote_queues, q); */
            }

            /* Register Queue with DBus */
            /* FIXME: Need to strip the .m3u */
            object_path = g_strdup_printf ("%s%s", BOGNOR_QUEUE_PATH,
                                           g_strdelimit (name, ".-", "_"));
            dbus_g_connection_register_g_object (br->connection, object_path,
                                                 G_OBJECT (q));

            /* FIXME: Emit a new queue signal */
            g_free (object_path);
            g_free (path);
        }
    }
#endif

    /* We always want to have a local queue */
    if (priv->local_queue == NULL) {
        char *object_path;

        priv->local_queue = (BognorQueue *) bognor_local_queue_new (priv->tracker);
        object_path = g_strdup_printf ("%slocal_queue", BOGNOR_QUEUE_PATH);
        dbus_g_connection_register_g_object (priv->connection, object_path,
                                             G_OBJECT (priv->local_queue));

        /* FIXME: Should we emit a signal for the local queue...
           it always exists and is at a well known location */
        g_free (object_path);
    }

#if 0
    g_dir_close (dir);
#endif
    g_free (bognor_queue_dir);
}

BognorQueueManager *
bognor_queue_manager_new (DBusGConnection  *connection,
                          BgrTrackerClient *tracker)
{
    BognorQueueManager *manager;
    BognorQueueManagerPrivate *priv;

    manager = g_object_new (BOGNOR_TYPE_QUEUE_MANAGER,
                            "tracker", tracker,
                            NULL);
    priv = manager->priv;
    priv->connection = connection;

    /* Load our queues */
    load_queues (manager);

    return manager;
}

void
bognor_queue_manager_action (BognorQueueManager      *manager,
                             BognorQueueManagerAction action)
{
    BognorQueueManagerPrivate *priv = manager->priv;

    switch (action) {
    case BOGNOR_QUEUE_MANAGER_ACTION_PLAY:
        bognor_queue_play  (priv->local_queue, NULL);
        break;

    case BOGNOR_QUEUE_MANAGER_ACTION_PAUSE:
        bognor_queue_stop (priv->local_queue, NULL);
        break;

    case BOGNOR_QUEUE_MANAGER_ACTION_NEXT:
        bognor_queue_next (priv->local_queue, NULL);
        break;

    case BOGNOR_QUEUE_MANAGER_ACTION_PREVIOUS:
        break;

    default:
        break;
    }
}

BognorQueue *
bognor_queue_manager_get_local_queue (BognorQueueManager *manager)
{
    BognorQueueManagerPrivate *priv = manager->priv;

    return priv->local_queue;
}
