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

#include "config.h"
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#ifdef USE_QUEUE_CONFIG
#include <gconf/gconf-client.h>
#endif

#include "br-marshal.h"
#include "br-queue.h"
#include "bognor-queue-bindings.h"

/* default settings */
#define BR_QUEUE_DBUS_SERVICE "org.moblin.BognorRegis"
#define BR_LOCAL_QUEUE_PATH "/org/moblin/BognorRegis/Queues/local_queue"

enum {
    PROP_0,
    PROP_PATH,
    PROP_SERVICE,
};

enum {
    ADDED,
    REMOVED,
    MOVED,
    INDEX_CHANGED,
    POSITION_CHANGED,
    STATE_CHANGED,
    LAST_SIGNAL
};

struct _BrQueuePrivate {
    DBusGProxy *proxy;
    char *object_path;
    char *service_name;
};

struct _AsyncClosure {
    BrQueue *queue;
    GCallback cb;
    gpointer userdata;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), BR_TYPE_QUEUE, BrQueuePrivate))
G_DEFINE_TYPE (BrQueue, br_queue, G_TYPE_OBJECT);
static guint32 signals[LAST_SIGNAL] = {0, };

static void uri_added_cb (DBusGProxy *proxy,
                          const char *uri,
                          int         index,
                          BrQueue    *queue);
static void uri_removed_cb (DBusGProxy *proxy,
                            const char *uri,
                            int         index,
                            BrQueue    *queue);
static void item_moved_cb (DBusGProxy *proxy,
                           int         old_position,
                           int         new_position,
                           int         updated_index,
                           BrQueue    *queue);
static void index_changed_cb (DBusGProxy *proxy,
                              int         index,
                              BrQueue    *queue);
static void position_changed_cb (DBusGProxy *proxy,
                                 double      position,
                                 BrQueue    *queue);
static void state_changed_cb (DBusGProxy *proxy,
                              int         state,
                              BrQueue    *queue);

static void
br_queue_finalize (GObject *object)
{
    BrQueue *self = (BrQueue *) object;
    BrQueuePrivate *priv = self->priv;

    if (priv->object_path) {
        g_free (priv->object_path);
        priv->object_path = NULL;
    }

    if (priv->service_name) {
        g_free (priv->service_name);
        priv->service_name = NULL;
    }

    G_OBJECT_CLASS (br_queue_parent_class)->finalize (object);
}

static void
br_queue_dispose (GObject *object)
{
    BrQueue *self = (BrQueue *) object;
    BrQueuePrivate *priv = self->priv;

    if (priv->proxy) {
        dbus_g_proxy_disconnect_signal (priv->proxy, "UriAdded",
                                        G_CALLBACK (uri_added_cb), self);
        dbus_g_proxy_disconnect_signal (priv->proxy, "UriRemoved",
                                        G_CALLBACK (uri_removed_cb), self);
        dbus_g_proxy_disconnect_signal (priv->proxy, "ItemMoved",
                                        G_CALLBACK (item_moved_cb), self);
        dbus_g_proxy_disconnect_signal (priv->proxy, "IndexChanged",
                                        G_CALLBACK (index_changed_cb), self);
        dbus_g_proxy_disconnect_signal (priv->proxy, "PositionChanged",
                                        G_CALLBACK (position_changed_cb), self);
        dbus_g_proxy_disconnect_signal (priv->proxy, "StateChanged",
                                        G_CALLBACK (state_changed_cb), self);

        g_object_unref (priv->proxy);
        priv->proxy = NULL;
    }

    G_OBJECT_CLASS (br_queue_parent_class)->dispose (object);
}

static void
br_queue_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
    BrQueue *self = (BrQueue *) object;
    BrQueuePrivate *priv = self->priv;

    switch (prop_id) {

    case PROP_PATH:
        priv->object_path = g_value_dup_string (value);
        break;

    case PROP_SERVICE:
        priv->service_name = g_value_dup_string (value);
        break;

    default:
        break;
    }
}

static void
br_queue_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
    BrQueue *self = (BrQueue *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
uri_added_cb (DBusGProxy *proxy,
              const char *uri,
              int         index,
              BrQueue    *queue)
{
    g_signal_emit (queue, signals[ADDED], 0, uri, index);
}

static void
uri_removed_cb (DBusGProxy *proxy,
                const char *uri,
                int         index,
                BrQueue    *queue)
{
    g_signal_emit (queue, signals[REMOVED], 0, uri, index);
}

static void
item_moved_cb (DBusGProxy *proxy,
               int         old_position,
               int         new_position,
               int         updated_index,
               BrQueue    *queue)
{
    g_signal_emit (queue, signals[MOVED], 0, old_position, new_position,
                   updated_index);
}

static void
index_changed_cb (DBusGProxy *proxy,
                  int         index,
                  BrQueue    *queue)
{
    g_signal_emit (queue, signals[INDEX_CHANGED], 0, index);
}

static void
position_changed_cb (DBusGProxy *proxy,
                     double      position,
                     BrQueue    *queue)
{
    g_signal_emit (queue, signals[POSITION_CHANGED], 0, position);
}

static void
state_changed_cb (DBusGProxy *proxy,
                  int         state,
                  BrQueue    *queue)
{
    g_signal_emit (queue, signals[STATE_CHANGED], 0, state);
}

static GObject *
br_queue_constructor (GType                  type,
                      guint                  n_construct_properties,
                      GObjectConstructParam *construct_properties)
{
    BrQueue *queue;
    BrQueuePrivate *priv;
    DBusGConnection *connection;
    DBusGProxy *proxy;
    GError *error = NULL;
    guint start_ret;

    queue = BR_QUEUE (G_OBJECT_CLASS (br_queue_parent_class)->constructor
                      (type, n_construct_properties, construct_properties));
    priv = queue->priv;

    connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
    if (connection == NULL) {
        g_warning ("Failed to open connection to bus: %s",
                   error->message);
        g_error_free (error);

        priv->proxy = NULL;
        return G_OBJECT (queue);
    }

    proxy = dbus_g_proxy_new_for_name (connection,
                                       DBUS_SERVICE_DBUS,
                                       DBUS_PATH_DBUS,
                                       DBUS_INTERFACE_DBUS);
    if (!org_freedesktop_DBus_start_service_by_name (proxy,
                                                     priv->service_name, 0,
                                                     &start_ret, &error)) {
        g_warning ("bognor-regis could not start queue service '%s': %s",
		   priv->service_name, error->message);
        g_error_free (error);

        priv->proxy = NULL;
        return G_OBJECT (queue);
    }

    priv->proxy = dbus_g_proxy_new_for_name (connection, priv->service_name,
                                             priv->object_path,
                                             BR_QUEUE_DBUS_INTERFACE);

    dbus_g_proxy_add_signal (priv->proxy, "UriAdded",
                             G_TYPE_STRING, G_TYPE_INT, G_TYPE_INVALID);
    dbus_g_proxy_add_signal (priv->proxy, "UriRemoved",
                             G_TYPE_STRING, G_TYPE_INT, G_TYPE_INVALID);
    dbus_g_proxy_add_signal (priv->proxy, "ItemMoved",
                             G_TYPE_INT, G_TYPE_INT, G_TYPE_INT,
                             G_TYPE_INVALID);
    dbus_g_proxy_add_signal (priv->proxy, "IndexChanged",
                             G_TYPE_INT, G_TYPE_INVALID);
    dbus_g_proxy_add_signal (priv->proxy, "PositionChanged",
                             G_TYPE_DOUBLE, G_TYPE_INVALID);
    dbus_g_proxy_add_signal (priv->proxy, "StateChanged",
                             G_TYPE_INT, G_TYPE_INVALID);
    dbus_g_proxy_connect_signal (priv->proxy, "UriAdded",
                                 G_CALLBACK (uri_added_cb), queue, NULL);
    dbus_g_proxy_connect_signal (priv->proxy, "UriRemoved",
                                 G_CALLBACK (uri_removed_cb), queue, NULL);
    dbus_g_proxy_connect_signal (priv->proxy, "ItemMoved",
                                 G_CALLBACK (item_moved_cb), queue, NULL);
    dbus_g_proxy_connect_signal (priv->proxy, "IndexChanged",
                                 G_CALLBACK (index_changed_cb), queue, NULL);
    dbus_g_proxy_connect_signal (priv->proxy, "PositionChanged",
                                 G_CALLBACK (position_changed_cb), queue, NULL);
    dbus_g_proxy_connect_signal (priv->proxy, "StateChanged",
                                 G_CALLBACK (state_changed_cb), queue, NULL);

    return G_OBJECT (queue);
}

static void
br_queue_class_init (BrQueueClass *klass)
{
    GObjectClass *o_class = (GObjectClass *)klass;

    o_class->dispose = br_queue_dispose;
    o_class->finalize = br_queue_finalize;
    o_class->set_property = br_queue_set_property;
    o_class->get_property = br_queue_get_property;
    o_class->constructor = br_queue_constructor;

    g_type_class_add_private (klass, sizeof (BrQueuePrivate));

    dbus_g_object_register_marshaller (br_marshal_VOID__STRING_INT,
                                       G_TYPE_NONE, G_TYPE_STRING,
                                       G_TYPE_INT, G_TYPE_INVALID);

    dbus_g_object_register_marshaller (br_marshal_VOID__INT_INT_INT,
                                       G_TYPE_NONE, G_TYPE_INT, G_TYPE_INT,
                                       G_TYPE_INT, G_TYPE_INVALID);

#if 0
    dbus_g_object_register_marshaller (br_marshal_VOID__INT_DOUBLE,
                                       G_TYPE_NONE, G_TYPE_INT,
                                       G_TYPE_DOUBLE, G_TYPE_INVALID);
#endif

    g_object_class_install_property (o_class, PROP_PATH,
                                     g_param_spec_string ("object-path",
                                                          "", "", "",
                                                          G_PARAM_WRITABLE |
                                                          G_PARAM_CONSTRUCT_ONLY |
                                                          G_PARAM_STATIC_STRINGS));
    g_object_class_install_property (o_class, PROP_SERVICE,
                                     g_param_spec_string ("service-name",
                                                          "", "", "",
                                                          G_PARAM_WRITABLE |
                                                          G_PARAM_CONSTRUCT_ONLY |
                                                          G_PARAM_STATIC_STRINGS));

    signals[ADDED] = g_signal_new ("uri-added",
                                   G_TYPE_FROM_CLASS (klass),
                                   G_SIGNAL_RUN_FIRST |
                                   G_SIGNAL_NO_RECURSE, 0, NULL, NULL,
                                   br_marshal_VOID__STRING_INT,
                                   G_TYPE_NONE, 2, G_TYPE_STRING,
                                   G_TYPE_INT);
    signals[REMOVED] = g_signal_new ("uri-removed",
                                     G_TYPE_FROM_CLASS (klass),
                                     G_SIGNAL_RUN_FIRST |
                                     G_SIGNAL_NO_RECURSE, 0, NULL, NULL,
                                     br_marshal_VOID__STRING_INT,
                                     G_TYPE_NONE, 2, G_TYPE_STRING,
                                     G_TYPE_INT);
    signals[MOVED] = g_signal_new ("item-moved",
                                   G_TYPE_FROM_CLASS (klass),
                                   G_SIGNAL_RUN_FIRST |
                                   G_SIGNAL_NO_RECURSE, 0, NULL, NULL,
                                   br_marshal_VOID__INT_INT_INT,
                                   G_TYPE_NONE, 3, G_TYPE_INT, G_TYPE_INT,
                                   G_TYPE_INT);
    signals[INDEX_CHANGED] = g_signal_new ("index-changed",
                                           G_TYPE_FROM_CLASS (klass),
                                           G_SIGNAL_RUN_FIRST |
                                           G_SIGNAL_NO_RECURSE, 0, NULL, NULL,
                                           g_cclosure_marshal_VOID__INT,
                                           G_TYPE_NONE, 1, G_TYPE_INT);
    signals[POSITION_CHANGED] = g_signal_new ("position-changed",
                                              G_TYPE_FROM_CLASS (klass),
                                              G_SIGNAL_RUN_FIRST |
                                              G_SIGNAL_NO_RECURSE,
                                              0, NULL, NULL,
                                              g_cclosure_marshal_VOID__DOUBLE,
                                              G_TYPE_NONE, 1, G_TYPE_DOUBLE);
    signals[STATE_CHANGED] = g_signal_new ("state-changed",
                                           G_TYPE_FROM_CLASS (klass),
                                           G_SIGNAL_RUN_FIRST |
                                           G_SIGNAL_NO_RECURSE,
                                           0, NULL, NULL,
                                           g_cclosure_marshal_VOID__INT,
                                           G_TYPE_NONE, 1, G_TYPE_INT);
}

static void
br_queue_init (BrQueue *self)
{
    self->priv = GET_PRIVATE (self);
}

BrQueue *
br_queue_new (const char *service_name,
              const char *object_path)
{
    BrQueue *queue;

    queue = g_object_new (BR_TYPE_QUEUE,
                          "object-path", object_path,
                          "service-name", service_name,
                          NULL);
    return queue;
}

/* allow different implementations of the queue service */

static char *
get_queue_dbus_service (void)
{
#ifdef USE_QUEUE_CONFIG
    char *queue;

    queue = gconf_client_get_string
        (gconf_client_get_default (),
	 "/desktop/moblin/media_queue/service", NULL);
    if (queue)
        return queue;
#endif

    return g_strdup (BR_QUEUE_DBUS_SERVICE);
}

static char *
get_local_dbus_path (void)
{
#ifdef USE_QUEUE_CONFIG
    char *path;

    path = gconf_client_get_string
        (gconf_client_get_default (),
	 "/desktop/moblin/media_queue/local_queue", NULL);
    if (path)
        return path;
#endif

    return g_strdup (BR_LOCAL_QUEUE_PATH);
}

BrQueue *
br_queue_new_local (void)
{
    BrQueue *queue;

#ifdef USE_QUEUE_CONFIG
    char *path, *service;

    path = get_local_dbus_path ();
    service = get_queue_dbus_service ();

    queue = br_queue_new (service, path);

    g_free (path);
    g_free (service);
#else
    queue = br_queue_new (BR_QUEUE_DBUS_SERVICE, BR_LOCAL_QUEUE_PATH);
#endif

    return queue;
}

static void
async_reply (DBusGProxy *proxy,
             GError     *error,
             gpointer    userdata)
{
    if (error != NULL) {
        g_warning ("Error talking to Bognor-Regis : %s", error->message);
        g_error_free (error);
    }
}

/**
 * br_queue_play:
 * @queue: A #BrQueue
 *
 * Requests that the queue represented by @queue starts to play
 */
void
br_queue_play (BrQueue *queue)
{
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    org_moblin_BognorRegis_Queue_play_async (priv->proxy, async_reply, queue);
}

/**
 * br_queue_stop:
 * @queue: A #BrQueue
 *
 * Requests that the queue represented by @queue stops playing
 */
void
br_queue_stop (BrQueue *queue)
{
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    org_moblin_BognorRegis_Queue_stop_async (priv->proxy, async_reply, queue);
}

/**
 * br_queue_next:
 * @queue: A #BrQueue
 *
 * Requests that the queue represented by @queue moves to the next track. This
 * does not affect the queue's state. If it was playing before the request,
 * it will still be playing after. If it was not playing before the request,
 * it will still be stopped after.
 */
void
br_queue_next (BrQueue *queue)
{
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    org_moblin_BognorRegis_Queue_next_async (priv->proxy, async_reply, queue);
}

/**
 * br_queue_previous:
 * @queue: A #BrQueue
 *
 * Requests that the queue represented by @queue moves to the previous track.
 * This does not affect the queue's state.
 */
void
br_queue_previous (BrQueue *queue)
{
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    org_moblin_BognorRegis_Queue_previous_async (priv->proxy, async_reply,
                                                 queue);
}

static void
get_state_reply (DBusGProxy *proxy,
                 int         state,
                 GError     *error,
                 gpointer    userdata)
{
    struct _AsyncClosure *data = (struct _AsyncClosure *) userdata;
    BrQueueGetStateCallback cb;

    cb = (BrQueueGetStateCallback) (data->cb);

    if (error) {
        /* Initialise the parameters correctly */
        state = BR_QUEUE_STATE_STOPPED;
    }

    cb (data->queue, state, error, data->userdata);

    if (error) {
        g_error_free (error);
    }

    g_free (data);
}

/**
 * br_queue_get_state:
 * @queue: A #BrQueue
 * @cb: The callback when the state has been returned
 * @userdata: Data to be passed to @cb
 *
 * Requests the current state of the queue represented by @queue. This is an
 * asynchronous command, and @cb will be called with @userdata whenever the
 * state has been returned
 */
void
br_queue_get_state (BrQueue                *queue,
                    BrQueueGetStateCallback cb,
                    gpointer                userdata)
{
    struct _AsyncClosure *data;
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    data = g_new (struct _AsyncClosure, 1);
    data->queue = queue;
    data->cb = G_CALLBACK (cb);
    data->userdata = userdata;

    org_moblin_BognorRegis_Queue_get_state_async (priv->proxy,
                                                  get_state_reply, data);
}

/**
 * br_queue_set_index:
 * @queue: A #BrQueue
 * @index: The index of the item
 *
 * Requests that the queue represented by @queue sets @index as the current
 * track. This does not affect the state of the queue.
 */
void
br_queue_set_index (BrQueue *queue,
                    int      index)
{
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));
    priv = queue->priv;

    org_moblin_BognorRegis_Queue_set_index_async (priv->proxy, index,
                                                  async_reply, queue);
}

static void
get_index_reply (DBusGProxy *proxy,
                 int         OUT_index,
                 GError     *error,
                 gpointer    userdata)
{
    struct _AsyncClosure *data = (struct _AsyncClosure *) userdata;
    BrQueueGetIndexCallback cb;

    cb = (BrQueueGetIndexCallback) (data->cb);

    if (error) {
        OUT_index = 0;
    }

    cb (data->queue, OUT_index, error, data->userdata);

    if (error) {
        g_error_free (error);
    }

    g_free (data);
}

/**
 * br_queue_get_index:
 * @queue: A #BrQueue
 * @cb: The callback for when the index is returned
 * @userdata: The data to be passed to @cb
 *
 * Requests the index of the current item in the queue represented by @queue.
 * This is an asynchronous call, and @cb will be called with @userdata whenever
 * the index has been returned.
 */
void
br_queue_get_index (BrQueue                *queue,
                    BrQueueGetIndexCallback cb,
                    gpointer                userdata)
{
    struct _AsyncClosure *data;
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    data = g_new (struct _AsyncClosure, 1);
    data->queue = queue;
    data->cb = G_CALLBACK (cb);
    data->userdata = userdata;

    org_moblin_BognorRegis_Queue_get_index_async (priv->proxy,
                                                  get_index_reply,
                                                  data);
}

static void
get_repeat_mode_reply (DBusGProxy *proxy,
                       int         OUT_repeat_mode,
                       GError     *error,
                       gpointer    userdata)
{
    struct _AsyncClosure *data = (struct _AsyncClosure *) userdata;
    BrQueueGetRepeatModeCallback cb;

    cb = (BrQueueGetRepeatModeCallback) (data->cb);

    if (error) {
        OUT_repeat_mode= -1;
    }

    if (cb) {
        cb (data->queue, OUT_repeat_mode, error, data->userdata);
    }

    if (error) {
        g_error_free (error);
    }

    g_free (data);
}

/**
 * br_queue_get_repeat_mode:
 * @queue: A #BrQueue
 * @cb: The callback for when the repeat_mode is returned
 * @userdata: The data to be passed to @cb
 *
 * Requests the repeat-mode of the queue represented by @queue.
 * This is an asynchronous call, and @cb will be called with @userdata whenever
 * the repeat_mode has been returned.
 */
void
br_queue_get_repeat_mode (BrQueue                *queue,
                    BrQueueGetRepeatModeCallback cb,
                    gpointer                userdata)
{
    struct _AsyncClosure *data;
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    data = g_new (struct _AsyncClosure, 1);
    data->queue = queue;
    data->cb = G_CALLBACK (cb);
    data->userdata = userdata;

    org_moblin_BognorRegis_Queue_get_repeat_mode_async (priv->proxy,
                                                        get_repeat_mode_reply,
                                                        data);
}

static void
get_duration_reply (DBusGProxy *proxy,
                    int         OUT_duration,
                    GError     *error,
                    gpointer    userdata)
{
    struct _AsyncClosure *data = (struct _AsyncClosure *) userdata;
    BrQueueGetDurationCallback cb;

    cb = (BrQueueGetDurationCallback) (data->cb);

    if (error) {
        OUT_duration= 0;
    }

    if (cb) {
        cb (data->queue, OUT_duration, error, data->userdata);
    }

    if (error) {
        g_error_free (error);
    }

    g_free (data);
}

/**
 * br_queue_get_duration:
 * @queue: A #BrQueue
 * @cb: The callback for when duration is returned
 * @userdata: The data to be passed to @cb
 *
 * Requests the repeat-mode of the queue represented by @queue.
 * This is an asynchronous call, and @cb will be called with @userdata whenever
 * the repeat_mode has been returned.
 */
void
br_queue_get_duration (BrQueue                *queue,
                          BrQueueGetDurationCallback cb,
                          gpointer                userdata)
{
    struct _AsyncClosure *data;
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    data = g_new (struct _AsyncClosure, 1);
    data->queue = queue;
    data->cb = G_CALLBACK (cb);
    data->userdata = userdata;

    org_moblin_BognorRegis_Queue_get_duration_async (priv->proxy,
                                                     get_duration_reply,
                                                     data);
}

static void
get_index_uri_reply (DBusGProxy *proxy,
                     char       *OUT_uri,
                     char       *OUT_mimetype,
                     GError     *error,
                     gpointer    userdata)
{
    struct _AsyncClosure *data = (struct _AsyncClosure *) userdata;
    BrQueueGetIndexUriCallback cb;

    cb = (BrQueueGetIndexUriCallback) (data->cb);

    if (error) {
        OUT_uri = NULL;
        OUT_mimetype = NULL;
    }

    cb (data->queue, OUT_uri, OUT_mimetype, error, data->userdata);

    if (error) {
        g_error_free (error);
    } else {
        g_free (OUT_uri);
        g_free (OUT_mimetype);
    }

    g_free (data);
}

/**
 * br_queue_get_index_uri:
 * @queue: A #BrQueue
 * @index: The index of the item being requested
 * @cb: The callback that will be used when the data is returned
 * @userdata: The data to be passed to @cb
 *
 * Requests the uri and mimetype of the item at @index position in the queue
 * represented by @queue. This is an asynchronous call and @cb will be called
 * with @userdata whenever the data is returned
 */
void
br_queue_get_index_uri (BrQueue                   *queue,
                        int                        index,
                        BrQueueGetIndexUriCallback cb,
                        gpointer                   userdata)
{
    struct _AsyncClosure *data;
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    data = g_new (struct _AsyncClosure, 1);
    data->queue = queue;
    data->cb = G_CALLBACK (cb);
    data->userdata = userdata;

    org_moblin_BognorRegis_Queue_get_index_uri_async (priv->proxy, index,
                                                      get_index_uri_reply,
                                                      data);
}

/**
 * br_queue_append_uris:
 * @queue: A #BrQueue
 * @count: The number of uris to be added
 * @uris: An array of uris, NULL-terminated
 * @mimetypes: An array of mimetypes, NULL-terminated
 *
 * Requests that @count uris be appended to the end of the queue represented by
 * @queue. The mimetypes in @mimetypes correspond to the uris in @uris. No
 * checking is done on the contents of @uris for their existence or suitability
 * when this call is made. Bognor-Regis will ignore any invalid URIs when they
 * are attempted to be played.
 */
void
br_queue_append_uris (BrQueue     *queue,
                      int          count,
                      const char **uris,
                      const char **mimetypes)
{
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    org_moblin_BognorRegis_Queue_append_uris_async (priv->proxy, count,
                                                    uris, mimetypes,
                                                    async_reply, queue);
}

/**
 * br_queue_remove_range:
 * @queue: A #BrQueue
 * @index: The first item to be removed
 * @count: The number of items to be removed
 *
 * Requests that @count items, starting from @index, be removed from the queue
 * represented by @queue
 */
void
br_queue_remove_range (BrQueue *queue,
                       int      index,
                       int      count)
{
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    org_moblin_BognorRegis_Queue_remove_range_async (priv->proxy, index,
                                                     count, async_reply, queue);
}

/**
 * br_queue_set_repeat_mode:
 * @queue: A #BrQueue
 * @mode: repeat mode
 *
 * set @queue's repeat mode
 *  BOGNOR_QUEUE_MODE_NORMAL,
 *  BOGNOR_QUEUE_MODE_REPEATING,
 *  BOGNOR_QUEUE_MODE_SINGLE_REPEATING,
 *  BOGNOR_QUEUE_MODE_SHUFFLE = 1 << 7,
 */
void
br_queue_set_repeat_mode (BrQueue *queue,
                          int      mode)
{
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;
    org_moblin_BognorRegis_Queue_set_repeat_mode_async (priv->proxy, mode, async_reply, queue);
}

/**
 * br_queue_move_item:
 * @queue: A #BrQueue
 * @old_position: The position of the item to be moved
 * @new_position: The position where the item should be moved to
 *
 * Requests that the item at position @old_position, be moved to @new_position
 * in the queue represented by @queue.
 *
 * This is d
 */
void
br_queue_move_item (BrQueue *queue,
                    int      old_position,
                    int      new_position)
{
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    org_moblin_BognorRegis_Queue_move_item_async (priv->proxy, old_position,
                                                  new_position, async_reply,
                                                  queue);
}

/**
 * br_queue_insert_uris:
 * @queue: A #BrQueue
 * @index: The position that the first item will be inserted
 * @count: The number of items to be added
 * @uris: An array of uris, NULL-terminated
 * @mimetypes: An array of mimetypes, NULL-terminated
 *
 * Requests that @count items be inserted at @index into the queue represented
 * by @queue. For example, if a queue exists containing 0 1 2 3, and 2 items are
 * added at index=2, then the queue will look like 0 a b 1 2 3 after the
 * operation.
 *
 * The mimetypes in @mimetypes correspond to the uris in @uris. No
 * checking is done on the contents of @uris for their existence or suitability
 * when this call is made. Bognor-Regis will ignore any invalid URIs when they
 * are attempted to be played.
 */
void
br_queue_insert_uris (BrQueue     *queue,
                      int          index,
                      int          count,
                      const char **uris,
                      const char **mimetypes)
{
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    org_moblin_BognorRegis_Queue_insert_uris_async (priv->proxy, index,
                                                    count, uris, mimetypes,
                                                    async_reply, queue);
}

static void
list_uris_reply (DBusGProxy *proxy,
                 char      **OUT_uris,
                 GError     *error,
                 gpointer    userdata)
{
    struct _AsyncClosure *data = (struct _AsyncClosure *) userdata;
    BrQueueListUrisCallback cb;

    cb = (BrQueueListUrisCallback) (data->cb);

    if (error) {
        /* Initialise the parameters correctly */
        OUT_uris = NULL;
    }

    cb (data->queue, (const char **) OUT_uris, error, data->userdata);

    if (error) {
        g_error_free (error);
    } else {
        g_strfreev (OUT_uris);
    }

    g_free (data);
}

/**
 * br_queue_list_uris:
 * @queue: A #BrQueue
 * @cb: The callback for when the data is returned
 * @userdata: The data passed to @cb
 *
 * Requests an array containing the uris currently in the queue represented by
 * @queue. This is an asychronous call and @cb will be called with @userdata
 * when the data is returned
 */
void
br_queue_list_uris (BrQueue                *queue,
                    BrQueueListUrisCallback cb,
                    gpointer                userdata)
{
    struct _AsyncClosure *data;
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    data = g_new (struct _AsyncClosure, 1);
    data->queue = queue;
    data->cb = G_CALLBACK (cb);
    data->userdata = userdata;

    org_moblin_BognorRegis_Queue_list_uris_async (priv->proxy,
                                                  list_uris_reply, data);
}

static void
get_name_reply (DBusGProxy *proxy,
                char       *OUT_name,
                GError     *error,
                gpointer    userdata)
{
    struct _AsyncClosure *data = (struct _AsyncClosure *) userdata;
    BrQueueGetNameCallback cb;

    cb = (BrQueueGetNameCallback) (data->cb);

    if (error) {
        OUT_name = NULL;
    }

    cb (data->queue, (const char *) OUT_name, error, data->userdata);

    if (error) {
        g_error_free (error);
    } else {
        g_free (OUT_name);
    }

    g_free (data);
}

/**
 * br_queue_get_name:
 * @queue: A #BrQueue
 * @cb: The callback for when the name is returned
 * @userdata: The data to be passed to @cb
 *
 * Requests the name of the queue represented by @queue. This is an
 * asynchronous call, and @cb will be called with @userdata when the name is
 * returned
 */
void
br_queue_get_name (BrQueue               *queue,
                   BrQueueGetNameCallback cb,
                   gpointer               userdata)
{
    struct _AsyncClosure *data;
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    data = g_new (struct _AsyncClosure, 1);
    data->queue = queue;
    data->cb = G_CALLBACK (cb);
    data->userdata = userdata;

    org_moblin_BognorRegis_Queue_get_name_async (priv->proxy,
                                                 get_name_reply, data);
}

/**
 * br_queue_set_position:
 * @queue: A #BrQueue
 * @position: The play position in the current item
 *
 * Requests that that play position in the current item in the queue represented
 * by @queue is set to @position. @position is a value in the range [0.0, 1.0]
 * with 0.0 representing the start of the track, and 1.0 representing the end.
 */
void
br_queue_set_position (BrQueue    *queue,
                       double      position)
{
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;
    org_moblin_BognorRegis_Queue_set_position_async (priv->proxy, position,
                                                     async_reply, queue);
}

static void
get_position_reply (DBusGProxy *proxy,
                    double      OUT_position,
                    GError     *error,
                    gpointer    userdata)
{
    struct _AsyncClosure *data = (struct _AsyncClosure *) userdata;
    BrQueueGetPositionCallback cb;

    cb = (BrQueueGetPositionCallback) (data->cb);

    if (error) {
        OUT_position = 0.0;
    }

    cb (data->queue, OUT_position, error, data->userdata);

    if (error) {
        g_error_free (error);
    }

    g_free (data);
}

/**
 * br_queue_get_position:
 * @queue: A #BrQueue
 * @cb: The callback for when the position is returned
 * @userdata: The data to be passed to @cb
 *
 * Requests the play position of the current item in the queue represented by
 * @queue. The position will be returned as a double in the range [0.0, 1.0]
 * with 0.0 representing the start of the track and 1.0 representing the end.
 * This is an asynchronous call and @cb will be called with @userdata when the
 * position is returned
 */
void
br_queue_get_position (BrQueue                   *queue,
                       BrQueueGetPositionCallback cb,
                       gpointer                   userdata)
{
    struct _AsyncClosure *data;
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    data = g_new (struct _AsyncClosure, 1);
    data->queue = queue;
    data->cb = G_CALLBACK (cb);
    data->userdata = userdata;

    org_moblin_BognorRegis_Queue_get_position_async (priv->proxy,
                                                     get_position_reply, data);
}

/**
 * br_queue_set_mute:
 * @queue: A #BrQueue
 * @mute: to mute the playbin or not
 *
 * Requests to set playbin to mute state
 * @mute is a boolean
 * with TRUE means 'mute', FALSE means 'not mute'
 */
void
br_queue_set_mute (BrQueue    *queue,
                   gboolean    mute)
{
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;
    org_moblin_BognorRegis_Queue_set_mute_async (priv->proxy, mute,
                                                 async_reply, queue);
}

static void
get_mute_reply (DBusGProxy *proxy,
                gboolean    OUT_mute,
                GError     *error,
                gpointer    userdata)
{
    struct _AsyncClosure *data = (struct _AsyncClosure *) userdata;
    BrQueueGetMuteCallback cb;

    cb = (BrQueueGetMuteCallback) (data->cb);

    if (error) {
        OUT_mute = 0.0;
    }

    cb (data->queue, OUT_mute, error, data->userdata);

    if (error) {
        g_error_free (error);
    }

    g_free (data);
}

/**
 * br_queue_get_mute:
 * @queue: A #BrQueue
 * @cb: The callback for when the mute status is returned
 * @userdata: The data to be passed to @cb
 *
 * Requests the mute status of @queue
 * This is an asynchronous call and @cb will be called with @userdata when the
 * mute is returned
 */
void
br_queue_get_mute (BrQueue                   *queue,
                   BrQueueGetMuteCallback     cb,
                   gpointer                   userdata)
{
    struct _AsyncClosure *data;
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    data = g_new (struct _AsyncClosure, 1);
    data->queue = queue;
    data->cb = G_CALLBACK (cb);
    data->userdata = userdata;

    org_moblin_BognorRegis_Queue_get_mute_async (priv->proxy,
                                                 get_mute_reply, data);
}

/**
 * br_queue_set_volume:
 * @queue: A #BrQueue
 * @volume: volume level
 *
 * Requests to set playbin volume level
 * audio stream volume [0.0, 10.0]
 *
 */
void
br_queue_set_volume (BrQueue    *queue,
                     double      volume)
{
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;
    org_moblin_BognorRegis_Queue_set_volume_async (priv->proxy, volume,
                                                   async_reply, queue);
    g_printf("set volume %f\n", volume);
}

static void
get_volume_reply (DBusGProxy *proxy,
                  double      OUT_volume,
                  GError     *error,
                  gpointer    userdata)
{
    struct _AsyncClosure *data = (struct _AsyncClosure *) userdata;
    BrQueueGetVolumeCallback cb;

    cb = (BrQueueGetVolumeCallback) (data->cb);

    if (error) {
        OUT_volume = 1.0;
    }

    cb (data->queue, OUT_volume, error, data->userdata);

    if (error) {
        g_error_free (error);
    }

    g_free (data);
}

/**
 * br_queue_get_volume:
 * @queue: A #BrQueue
 * @cb: The callback for when the volume level is returned
 * @userdata: The data to be passed to @cb
 *
 * Requests the volume level of @queue
 * This is an asynchronous call and @cb will be called with @userdata when the
 * volume is returned
 */
void
br_queue_get_volume (BrQueue                   *queue,
                     BrQueueGetVolumeCallback   cb,
                     gpointer                   userdata)
{
    struct _AsyncClosure *data;
    BrQueuePrivate *priv;

    g_return_if_fail (IS_BR_QUEUE (queue));

    priv = queue->priv;

    data = g_new (struct _AsyncClosure, 1);
    data->queue = queue;
    data->cb = G_CALLBACK (cb);
    data->userdata = userdata;

    org_moblin_BognorRegis_Queue_get_volume_async (priv->proxy,
                                                   get_volume_reply, data);
}

