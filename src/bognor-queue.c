#include <stdio.h>

#include <glib.h>
#include <glib/gi18n.h>

#include <libnotify/notify.h>

#include "bgr-item.h"
#include "bgr-tracker-client.h"
#include "bognor-marshal.h"
#include "bognor-queue.h"
#include "bognor-queue-glue.h"

enum {
    PROP_0,
    PROP_NAME,
    PROP_TRACKER,
};

enum {
    ADDED,
    REMOVED,
    MOVED,
    INDEX_CHANGED,
    ITEM_CHANGED,
    POSITION_CHANGED,
    STATE_CHANGED,
    LAST_SIGNAL,
};

struct _BognorQueuePrivate {
    char *name; /* Name of the queue */
    GVolumeMonitor *volume_monitor; /* This probably doesn't need to be here
                                       and should be in a subclass or in the
                                       QueueMonitor */

    GQueue *play_queue; /* contains BognorQueueItem */
    GList *current_position; /* Owned by @play_queue */
    int current_index;

    int queue_duration; /* In seconds */

    BgrTrackerClient *tracker;

    BognorQueueState state;
    BognorQueueStatus status;
    BognorQueueMode mode;
    int *shuffle_array;
    int shuffle_array_size;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), BOGNOR_TYPE_QUEUE, BognorQueuePrivate))
G_DEFINE_TYPE (BognorQueue, bognor_queue, G_TYPE_OBJECT);
static guint32 signals[LAST_SIGNAL] = {0,};

static void
bq_sync_current_index(BognorQueue *queue)
{
    BognorQueuePrivate *priv = queue->priv;

    int idx;
    idx = g_queue_link_index(priv->play_queue, priv->current_position);
    if (idx == -1) {  // not found
        bognor_queue_set_index (queue, 0, NULL);
    } else {
        priv->current_index = idx;
    }
}

void
bq_make_shuffle_list(BognorQueue *queue)
{
    BognorQueuePrivate *priv = queue->priv;
    int playlist_len;
    int i, r, temp;

    playlist_len = g_queue_get_length(priv->play_queue);
    if(playlist_len > 0) {
        if (playlist_len != priv->shuffle_array_size) {
            // re-initialize shuffle list
            if (priv->shuffle_array == NULL) {
                priv->shuffle_array = g_new (int, playlist_len);
            } else {
                priv->shuffle_array = g_renew (int, priv->shuffle_array, playlist_len);
            }
            for ( i=0; i<playlist_len; i++) {
                priv->shuffle_array[i] = i;
            }
            for (i=playlist_len; i > 0; i--) {
                r = g_random_int_range (0, i);
                temp = priv->shuffle_array[i-1];
                priv->shuffle_array[i-1] = priv->shuffle_array[r];
                priv->shuffle_array[r] = temp;
            }
            for ( i=0; i<playlist_len; i++) {
                g_print("%d\n", priv->shuffle_array[i]);
            }
            priv->shuffle_array_size = playlist_len;
        }
    } else {
        priv->shuffle_array_size = 0;
        g_free(priv->shuffle_array);
        priv->shuffle_array = NULL;
    }
}

GQuark
bognor_queue_error_quark (void)
{
    static GQuark quark = 0;

    if (G_UNLIKELY (quark == 0)) {
        quark = g_quark_from_static_string ("bognor-queue-error-quark");
    }

    return quark;
}

static void
bognor_queue_finalize (GObject *object)
{
    BognorQueue *self = (BognorQueue *) object;

    G_OBJECT_CLASS (bognor_queue_parent_class)->finalize (object);
}

static void
bognor_queue_dispose (GObject *object)
{
    BognorQueue *self = (BognorQueue *) object;
    BognorQueuePrivate *priv = self->priv;

    if (priv->tracker) {
        g_object_unref (priv->tracker);
        priv->tracker = NULL;
    }

    G_OBJECT_CLASS (bognor_queue_parent_class)->dispose (object);
}

static void
bognor_queue_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    BognorQueue *self = (BognorQueue *) object;
    BognorQueuePrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_NAME:
        priv->name = g_value_dup_string (value);
        break;

    case PROP_TRACKER:
        priv->tracker = g_value_get_object (value);
        break;

    default:
        break;
    }
}

static void
bognor_queue_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    BognorQueue *self = (BognorQueue *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bognor_queue_class_init (BognorQueueClass *klass)
{
    GObjectClass *o_class = (GObjectClass *) klass;

    o_class->dispose = bognor_queue_dispose;
    o_class->finalize = bognor_queue_finalize;
    o_class->set_property = bognor_queue_set_property;
    o_class->get_property = bognor_queue_get_property;

    g_type_class_add_private (klass, sizeof (BognorQueuePrivate));
    dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass),
                                     &dbus_glib_bognor_queue_object_info);

    g_object_class_install_property (o_class, PROP_NAME,
                                     g_param_spec_string ("name", "", "", "",
                                                          G_PARAM_READWRITE |
                                                          G_PARAM_CONSTRUCT_ONLY |
                                                          G_PARAM_STATIC_STRINGS));
    g_object_class_install_property (o_class, PROP_TRACKER,
                                     g_param_spec_object ("tracker",
                                                          "", "",
                                                          BGR_TYPE_TRACKER_CLIENT,
                                                          G_PARAM_CONSTRUCT_ONLY |
                                                          G_PARAM_STATIC_STRINGS |
                                                          G_PARAM_WRITABLE));

    signals[ADDED] = g_signal_new ("uri-added",
                                   G_TYPE_FROM_CLASS (klass),
                                   G_SIGNAL_RUN_FIRST |
                                   G_SIGNAL_NO_RECURSE, 0, NULL, NULL,
                                   bognor_marshal_VOID__STRING_INT,
                                   G_TYPE_NONE, 2, G_TYPE_STRING,
                                   G_TYPE_INT);
    signals[REMOVED] = g_signal_new ("uri-removed",
                                     G_TYPE_FROM_CLASS (klass),
                                     G_SIGNAL_RUN_FIRST |
                                     G_SIGNAL_NO_RECURSE, 0, NULL, NULL,
                                     bognor_marshal_VOID__STRING_INT,
                                     G_TYPE_NONE, 2, G_TYPE_STRING,
                                     G_TYPE_INT);
    signals[MOVED] = g_signal_new ("item-moved",
                                   G_TYPE_FROM_CLASS (klass),
                                   G_SIGNAL_RUN_FIRST |
                                   G_SIGNAL_NO_RECURSE, 0, NULL, NULL,
                                   bognor_marshal_VOID__INT_INT_INT,
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
    signals[ITEM_CHANGED] = g_signal_new ("item-changed",
                                          G_TYPE_FROM_CLASS (klass),
                                          G_SIGNAL_RUN_FIRST |
                                          G_SIGNAL_NO_RECURSE, 0, NULL, NULL,
                                          g_cclosure_marshal_VOID__OBJECT,
                                          G_TYPE_NONE, 1,
                                          BOGNOR_TYPE_QUEUE_ITEM);
}

static void
bognor_queue_init (BognorQueue *self)
{
    BognorQueuePrivate *priv = GET_PRIVATE (self);

    self->priv = priv;

    priv->play_queue = g_queue_new ();
    priv->current_position = NULL;
    priv->current_index = BOGNOR_QUEUE_INDEX_END;
    priv->mode = BOGNOR_QUEUE_MODE_NORMAL;
    priv->shuffle_array = NULL;
    priv->shuffle_array_size = 0;
}

void
bognor_queue_notify_unknown_format (BognorQueue     *queue,
                                    BognorQueueItem *item)
{
    NotifyNotification *notification;
    const char *uri;
    char *basename, *unesc, *message;

    uri = bognor_queue_item_get_uri (item);
    basename = g_path_get_basename (uri);
    unesc = g_uri_unescape_string (basename, NULL);
    g_free (basename);

#ifdef ENABLE_HELIX
    message = g_strdup_printf (_("Sorry, we can't play %s, as we don't have the correct plugin."), unesc);
#else
    message = g_strdup_printf (_("Sorry, we can't play %s, as we don't have the correct plugin. You could try searching for ❝gstreamer codecs❞ on the web."), unesc);
#endif
    g_free (unesc);

    notification = notify_notification_new (_("We can't play this"),
                                            message, NULL, NULL);
    notify_notification_show (notification, NULL);
    g_object_unref (notification);

    g_free (message);
}

void
bognor_queue_emit_position_changed (BognorQueue *queue,
                                    double       position)
{
    g_signal_emit (queue, signals[POSITION_CHANGED], 0, position);
}


gboolean
bognor_queue_stop (BognorQueue *queue,
                   GError     **error)
{
    BognorQueueClass *klass = BOGNOR_QUEUE_GET_CLASS (queue);
    BognorQueuePrivate *priv = queue->priv;

    klass->set_playing (queue, FALSE);

    priv->state = BOGNOR_QUEUE_STATE_STOPPED;
    g_signal_emit (queue, signals[STATE_CHANGED], 0, priv->state);

    return TRUE;
}

gboolean
bognor_queue_play (BognorQueue *queue,
                   GError     **error)
{
    BognorQueueClass *klass = BOGNOR_QUEUE_GET_CLASS (queue);
    BognorQueuePrivate *priv = queue->priv;

    if (priv->current_position == NULL) {
        if (error != NULL) {
            *error = g_error_new (BOGNOR_QUEUE_ERROR,
                                  BOGNOR_QUEUE_ERROR_EMPTY,
                                  "The queue is empty");
        }
        return FALSE;
    }

    klass->set_playing (queue, TRUE);

    priv->state = BOGNOR_QUEUE_STATE_PLAYING;
    g_signal_emit (queue, signals[STATE_CHANGED], 0, priv->state);

    return TRUE;
}

gboolean
bognor_queue_next (BognorQueue *queue,
                   GError     **error)
{
    BognorQueueClass *klass = BOGNOR_QUEUE_GET_CLASS (queue);
    BognorQueuePrivate *priv = queue->priv;
    BgrItem *item;

    if (priv->state != BOGNOR_QUEUE_STATE_PLAYING) {
        return TRUE;
    }

    if (priv->current_position == NULL) {
        if (error != NULL) {
            *error = g_error_new (BOGNOR_QUEUE_ERROR,
                                  BOGNOR_QUEUE_ERROR_EMPTY,
                                  "The queue is empty");
        }
        return FALSE;
    }

    if ((priv->mode & 0x7f) == BOGNOR_QUEUE_MODE_SINGLE_REPEATING) {
        klass->set_item (queue, (BognorQueueItem *) priv->current_position->data);
        return TRUE;
    }

    if( (priv->mode & 0x80) == BOGNOR_QUEUE_MODE_SHUFFLE) {
        int shuffle_idx = ++priv->current_index;
        if (shuffle_idx >= priv->shuffle_array_size) {
            /* We've now finished the queue */
            if ((priv->mode & 0x7f) == BOGNOR_QUEUE_MODE_REPEATING) {
                // repeat
                bognor_queue_set_index (queue, priv->shuffle_array[0], error);
                // current_index is changed in bognor_queue_set_index
                priv->current_index = 0;
            } else {
                // no repeat
                bognor_queue_stop (queue, error);
                bognor_queue_set_position (queue, 0.0, error);
                bognor_queue_set_index (queue, priv->shuffle_array[0], error);
                priv->current_index = 0;
            }
            return TRUE;
        }
        bognor_queue_set_index (queue, priv->shuffle_array[shuffle_idx], error);
        priv->current_index = shuffle_idx;
        return TRUE;
    }

    priv->current_position = priv->current_position->next;
    priv->current_index++;

    if (priv->current_position == NULL) {
        /* We've now finished the queue */
        if ((priv->mode & 0x7f) == BOGNOR_QUEUE_MODE_REPEATING) {
            bognor_queue_set_index (queue, 0, error);
            return TRUE;
        } else {
            bognor_queue_stop (queue, error);
            bognor_queue_set_position (queue, 0.0, error);
            bognor_queue_set_index (queue, 0, error);
            return TRUE;
        }
    }

    klass->set_item (queue, (BognorQueueItem *) priv->current_position->data);
    klass->add_item_to_recent (queue, (BognorQueueItem *) priv->current_position->data);

    item = bognor_queue_item_get_item (priv->current_position->data);
    bgr_tracker_client_update_item_use_count (priv->tracker, item);

    g_signal_emit (queue, signals[INDEX_CHANGED], 0, priv->current_index);
    g_signal_emit (queue, signals[ITEM_CHANGED], 0,
                   priv->current_position->data);

    return TRUE;
}

gboolean
bognor_queue_previous (BognorQueue *queue,
                       GError     **error)
{
    BognorQueueClass *klass = BOGNOR_QUEUE_GET_CLASS (queue);
    BognorQueuePrivate *priv = queue->priv;
    BgrItem *item;

    if (priv->state != BOGNOR_QUEUE_STATE_PLAYING) {
        return TRUE;
    }

    if (priv->current_position == NULL) {
        if (error != NULL) {
            *error = g_error_new (BOGNOR_QUEUE_ERROR,
                                  BOGNOR_QUEUE_ERROR_EMPTY,
                                  "The queue is empty");
        }
        return FALSE;
    }

    if ((priv->mode & 0x7f) == BOGNOR_QUEUE_MODE_SINGLE_REPEATING) {
        klass->set_item (queue, (BognorQueueItem *) priv->current_position->data);
        return TRUE;
    }

    if( (priv->mode & 0x80) == BOGNOR_QUEUE_MODE_SHUFFLE) {
        int shuffle_idx = --priv->current_index;
        if (shuffle_idx < 0) {
            /* at the head of the queue */
            if ((priv->mode & 0x7f) == BOGNOR_QUEUE_MODE_REPEATING) {
                // repeat
                bognor_queue_set_index (queue, priv->shuffle_array[priv->shuffle_array_size-1], error);
                // current_index is changed in bognor_queue_set_index
                priv->current_index = priv->shuffle_array_size-1;
            } else {
                // no repeat
                bognor_queue_stop (queue, error);
                bognor_queue_set_position (queue, 0.0, error);
                bognor_queue_set_index (queue, priv->shuffle_array[0], error);
                priv->current_index = 0;
            }
            return TRUE;
        }
        bognor_queue_set_index (queue, priv->shuffle_array[shuffle_idx], error);
        priv->current_index = shuffle_idx;
        return TRUE;
    }

    priv->current_position = priv->current_position->prev;
    priv->current_index--;

    if (priv->current_index < 0) {
        if ((priv->mode & 0x7f) == BOGNOR_QUEUE_MODE_REPEATING) {
            /* at the head of the queue */
            bognor_queue_set_index (queue, g_queue_get_length(priv->play_queue) -1, error);
            return TRUE;
        } else {
            bognor_queue_stop (queue, error);
            bognor_queue_set_position (queue, 0.0, error);
            bognor_queue_set_index (queue, 0, error);
            return TRUE;
        }
    }

    klass->set_item (queue, (BognorQueueItem *) priv->current_position->data);
    klass->add_item_to_recent (queue, (BognorQueueItem *) priv->current_position->data);
    item = bognor_queue_item_get_item (priv->current_position->data);
    bgr_tracker_client_update_item_use_count (priv->tracker, item);

    g_signal_emit (queue, signals[INDEX_CHANGED], 0, priv->current_index);
    g_signal_emit (queue, signals[ITEM_CHANGED], 0,
                   priv->current_position->data);

    return TRUE;
}

gboolean bognor_queue_set_mute (BognorQueue   *queue,
                                gboolean       mute,
                                GError       **error)
{
    BognorQueueClass *klass = BOGNOR_QUEUE_GET_CLASS (queue);
    BognorQueuePrivate *priv = queue->priv;
    if (mute)
        klass->set_mute(queue, TRUE);
    else
        klass->set_mute(queue, FALSE);
    return TRUE;
}

gboolean bognor_queue_get_mute (BognorQueue   *queue,
                                gboolean      *mute,
                                GError       **error)
{
    BognorQueueClass *klass = BOGNOR_QUEUE_GET_CLASS (queue);
    BognorQueuePrivate *priv = queue->priv;
    *mute = klass->get_mute (queue);
    return TRUE;
}

gboolean bognor_queue_set_volume (BognorQueue   *queue,
                                  double         volume,
                                  GError       **error)
{
    BognorQueueClass *klass = BOGNOR_QUEUE_GET_CLASS (queue);
    BognorQueuePrivate *priv = queue->priv;
    klass->set_volume (queue, volume);
    return TRUE;
}

gboolean bognor_queue_get_volume (BognorQueue   *queue,
                                  double        *volume,
                                  GError       **error)
{
    BognorQueueClass *klass = BOGNOR_QUEUE_GET_CLASS (queue);
    BognorQueuePrivate *priv = queue->priv;
    *volume = klass->get_volume(queue);
    return TRUE;
}

gboolean
bognor_queue_set_position (BognorQueue *queue,
                           double       position,
                           GError     **error)
{
    BognorQueueClass *klass = BOGNOR_QUEUE_GET_CLASS (queue);
    BognorQueuePrivate *priv = queue->priv;

    if (priv->current_position == NULL) {
        if (error != NULL) {
            *error = g_error_new (BOGNOR_QUEUE_ERROR,
                                  BOGNOR_QUEUE_ERROR_EMPTY,
                                  "The queue is empty");
        }
        return FALSE;
    }

    klass->set_position (queue, position);
    return TRUE;
}

gboolean
bognor_queue_get_position (BognorQueue *queue,
                           double      *position,
                           GError     **error)
{
    BognorQueueClass *klass = BOGNOR_QUEUE_GET_CLASS (queue);
    BognorQueuePrivate *priv = queue->priv;

    if (priv->current_position == NULL) {
        if (error != NULL) {
            *error = g_error_new (BOGNOR_QUEUE_ERROR,
                                  BOGNOR_QUEUE_ERROR_EMPTY,
                                  "The queue is empty");
        }
        *position = 0.0;
        return FALSE;
    }

    *position = klass->get_position (queue);

    return TRUE;
}

gboolean
bognor_queue_append_uris (BognorQueue *queue,
                          int          count,
                          const char **uris,
                          const char **mimetypes,
                          GError     **error)
{
    BognorQueuePrivate *priv = queue->priv;
    int i;

    for (i = 0; i < count; i++) {
        BgrItem *bgr;
        BognorQueueItem *item;

        g_print ("Appending %s\n", uris[i]);
        bgr = bgr_tracker_client_get_item (priv->tracker, uris[i]);
        if (bgr == NULL) {
            item = bognor_queue_item_new (uris[i],
                                          mimetypes[i], NULL);
        } else {
            item = bognor_queue_item_new_from_item (bgr);
        }
        g_queue_push_tail (priv->play_queue, item);

        /* FIXME: Should we have an "add-start" and "add-finish" set
           of signals around this? */
        g_signal_emit (queue, signals[ADDED], 0, uris[i],
                       g_queue_get_length (priv->play_queue) - 1);
    }

    if ((priv->mode & 0x80) == BOGNOR_QUEUE_MODE_SHUFFLE) {
        bq_make_shuffle_list(queue);
        if (priv->state == BOGNOR_QUEUE_STATE_STOPPED) {
            bognor_queue_set_index (queue, priv->shuffle_array[0], error);
            priv->current_index = 0;
        }
    } else {
        if (priv->current_position == NULL) {
            return bognor_queue_set_index (queue, 0, error);
        }
    }

    return TRUE;
}

gboolean
bognor_queue_insert_uris (BognorQueue *queue,
                          int          position,
                          int          count,
                          const char **uris,
                          const char **mimetypes,
                          GError     **error)
{
    BognorQueuePrivate *priv = queue->priv;
    GList *nth;
    int i;

    nth = g_queue_peek_nth_link (priv->play_queue, position);
    if (nth == NULL) {
        if ( error != NULL) {
            *error = g_error_new (BOGNOR_QUEUE_ERROR,
                                  BOGNOR_QUEUE_ERROR_OUT_OF_RANGE,
                                  "%d is out of range for queue with %d items",
                                  position,
                                  g_queue_get_length (priv->play_queue));
        }
        return FALSE;
    }

    /* FIXME: See issues in append_uris as well */
    for (i = 0; i < count; i++) {
        BgrItem *bgr;
        BognorQueueItem *item;

        bgr = bgr_tracker_client_get_item (priv->tracker, uris[i]);
        if (bgr == NULL) {
            item = bognor_queue_item_new (uris[i],
                                          mimetypes[i], NULL);
        } else {
            item = bognor_queue_item_new_from_item (bgr);
        }
        g_print ("Insert %p (uri=%s) at position %d\n", item, uris[i],
                 position + i);
        g_queue_insert_before (priv->play_queue, nth, item);
        g_signal_emit (queue, signals[ADDED], 0, uris[i], position + i);
    }

    if ((priv->mode & 0x80) == BOGNOR_QUEUE_MODE_SHUFFLE) {
        bq_make_shuffle_list(queue);
        if (priv->state == BOGNOR_QUEUE_STATE_STOPPED) {
            bognor_queue_set_index (queue, priv->shuffle_array[0], error);
            priv->current_index = 0;
        }
    } else {
        // sync current_index, esp, insert before current_index
        bq_sync_current_index(queue);
    }

    return TRUE;
}

gboolean
bognor_queue_remove_range (BognorQueue *queue,
                           int          index,
                           int          count,
                           GError     **error)
{
    BognorQueuePrivate *priv = queue->priv;
    BognorQueueClass *klass = BOGNOR_QUEUE_GET_CLASS (queue);
    int i;
    gboolean remove_current = FALSE;

    if (index >= g_queue_get_length (priv->play_queue) ||
        index + count > g_queue_get_length (priv->play_queue)) {
        if (error != NULL) {
            *error = g_error_new (BOGNOR_QUEUE_ERROR,
                                  BOGNOR_QUEUE_ERROR_OUT_OF_RANGE,
                                  "%d + %d is out of range for queue with %d "
                                  "items", index, count,
                                  g_queue_get_length (priv->play_queue));
        }
        return FALSE;
    }

    for (i = index; i < index + count; i++) {
        GList *nth;
        BognorQueueItem *item;

        g_print ("Removing item at position %d\n", i);
        nth = g_queue_pop_nth_link (priv->play_queue, index);
        if (nth == NULL) {
            g_warning ("Got NULL for item at %d\n", i);
            continue;
        }

        item = nth->data;

        /* stop the item playing if we are removing it */
        if (nth == priv->current_position) {
            bognor_queue_stop (queue, error);
            remove_current = TRUE;
        }

        /* we always remove the indexth item */
        g_signal_emit (queue, signals[REMOVED], 0,
                       bognor_queue_item_get_uri (item), index);

        g_object_unref (item);
    }

    int queue_len = g_queue_get_length(priv->play_queue);
    int idx;
    if (queue_len == 0) {
        priv->current_position = NULL;
        priv->current_index = BOGNOR_QUEUE_INDEX_END;
        return TRUE;
    }

    if (remove_current == TRUE) {
        if (priv->current_index >= queue_len) {
            idx = queue_len -1;
        } else {
            idx = priv->current_index;
        }
        if ((priv->mode & 0x80) != BOGNOR_QUEUE_MODE_SHUFFLE) {
            bognor_queue_set_position (queue, 0.0, error);
            bognor_queue_set_index (queue, idx, error);
        } else {
            bq_make_shuffle_list(queue);
            // empty queue is handled above
            bognor_queue_set_index (queue, priv->shuffle_array[idx], error);
            priv->current_index = idx;
        }
    } else {
        if ((priv->mode & 0x80) != BOGNOR_QUEUE_MODE_SHUFFLE) {
            bq_sync_current_index(queue);
        } else {
            bq_make_shuffle_list(queue);
            // find current item
            idx = g_queue_link_index(priv->play_queue, priv->current_position);
            int j;
            for (j = 0; j < queue_len; j++) {
                if ( idx == priv->shuffle_array[j] ) {
                    priv->current_index = j;
                    break;
                }
            }
        }
    }
    return TRUE;
}

/*
 * The move function moves the item from old_position to new_position. Once
 * done we have to update the current index, update that needs to be split in
 * 3 cases:
 *
 * The ascii diagrams below represent a queue with with items before (left)
 * and after (right) the move m(x,y). The stars represent the index and its
 * update.
 *
 * 1/ priv->current_index == old_position --> index = new_position
 *
 *      *                   *
 *      012345          012345
 *      012345  m(0,4)  123405
 *
 * 2/ old_position < current_index && new_position >= current_index --> index--
 *
 *        *              *
 *      012345          012345
 *      012345  m(0,4)  123405
 *
 *        *              *
 *      012345          012345
 *      012345  m(0,2)  120345
 *
 * 3/ old_position > current_index && new_position <= current_index --> index++
 *
 *        *                *
 *      012345          012345
 *      012345  m(4,0)  401235
 *
 *        *                *
 *      012345          012345
 *      012345  m(3,2)  013245
 *
 */
gboolean
bognor_queue_move_item (BognorQueue *queue,
                        int          old_position,
                        int          new_position,
                        GError     **error)
{
    BognorQueuePrivate *priv = queue->priv;
    GList *nth, *new;

    if (new_position < 0 ||
        new_position >= g_queue_get_length (priv->play_queue)) {
        if (error != NULL) {
            *error = g_error_new (BOGNOR_QUEUE_ERROR,
                                  BOGNOR_QUEUE_ERROR_OUT_OF_RANGE,
                                  "new position %d is out of range for queue "
                                  "with %d items", new_position,
                                  g_queue_get_length (priv->play_queue));
        }
        return FALSE;
    }

    /* priv->current_position is preserved by pop_nth_link / push_nth_link */
    nth = g_queue_pop_nth_link (priv->play_queue, old_position);
    if (nth == NULL) {
        if (error != NULL) {
            *error = g_error_new (BOGNOR_QUEUE_ERROR,
                                  BOGNOR_QUEUE_ERROR_OUT_OF_RANGE,
                                  "old position %d is out of range for queue "
                                  "with %d items", old_position,
                                  g_queue_get_length (priv->play_queue));
        }
        return FALSE;
    }

    g_queue_push_nth_link (priv->play_queue, new_position, nth);

    /* update priv->current_index if needed, see comment at the top of the
     * function */
    if (priv->current_index == old_position) {
        priv->current_index = new_position;
    } else if (old_position < priv->current_index &
               new_position >=  priv->current_index)
    {
        priv->current_index--;
    } else if (old_position > priv->current_index &&
               new_position <= priv->current_index)
    {
        priv->current_index++;
    }

    g_signal_emit (queue, signals[MOVED], 0, old_position, new_position,
                   priv->current_index);

    return TRUE;
}

gboolean
bognor_queue_set_index (BognorQueue *queue,
                        int          index,
                        GError     **error)
{
    BognorQueueClass *klass = BOGNOR_QUEUE_GET_CLASS (queue);
    BognorQueuePrivate *priv = queue->priv;
    BgrItem *item;

    g_print ("Setting index to %d\n", index);
    if (index == BOGNOR_QUEUE_INDEX_END) {
        priv->current_position = g_queue_peek_tail_link (priv->play_queue);
    } else {
        priv->current_position = g_queue_peek_nth_link (priv->play_queue, index);
    }

    if (priv->current_position == NULL) {
        if (error != NULL) {
            *error = g_error_new (BOGNOR_QUEUE_ERROR,
                                  BOGNOR_QUEUE_ERROR_OUT_OF_RANGE,
                                  "%d is out of range for queue with %d items",
                                  index, g_queue_get_length (priv->play_queue));
        }
        return FALSE;
    }

    klass->set_item (queue, priv->current_position->data);
    klass->add_item_to_recent (queue, (BognorQueueItem *) priv->current_position->data);

    item = bognor_queue_item_get_item (priv->current_position->data);
    bgr_tracker_client_update_item_use_count (priv->tracker, item);

    priv->current_index = index;

    if (index == BOGNOR_QUEUE_INDEX_END) {
        index = g_queue_get_length (priv->play_queue) - 1;
    }

    g_signal_emit (queue, signals[INDEX_CHANGED], 0, index);
    g_signal_emit (queue, signals[ITEM_CHANGED], 0,
                   priv->current_position->data);

    return TRUE;
}

gboolean
bognor_queue_get_current_index (BognorQueue *queue,
                                int         *index,
                                GError     **error)
{
    BognorQueuePrivate *priv = queue->priv;

    *index = priv->current_index;
    return TRUE;
}

gboolean
bognor_queue_get_index (BognorQueue *queue,
                        int         *index,
                        GError     **error)
{
    BognorQueuePrivate *priv = queue->priv;

    if( (priv->mode & 0x80) == BOGNOR_QUEUE_MODE_SHUFFLE) {
        *index = priv->shuffle_array[priv->current_index];
    } else {
        *index = priv->current_index;
    }
    return TRUE;
}

gboolean
bognor_queue_get_index_metadata (BognorQueue   *queue,
                                 int            index,
                                 char         **title,
                                 char         **artist,
                                 char         **album,
                                 GError        **error)
{
    BognorQueuePrivate *priv = queue->priv;
    BognorQueueItem *item;
    BgrItem *bgr;
    const char *str;

    item = g_queue_peek_nth (priv->play_queue, index);
    if (item == NULL) {
        if (error != NULL) {
            *error = g_error_new (BOGNOR_QUEUE_ERROR,
                                  BOGNOR_QUEUE_ERROR_OUT_OF_RANGE,
                                  "%d is out of range for queue with %d items",
                                  index, g_queue_get_length (priv->play_queue));
        }
        return FALSE;
    }

    bgr = bognor_queue_item_get_item(item);
    if(bgr) {
        *title = g_strdup(bgr_item_get_metadata (bgr, BGR_ITEM_METADATA_TITLE));
        *album = g_strdup(bgr_item_get_metadata (bgr, BGR_ITEM_METADATA_ALBUM));
        *artist = g_strdup(bgr_item_get_metadata (bgr, BGR_ITEM_METADATA_ARTIST));
    } else {
        if (error != NULL) {
            *error = g_error_new (BOGNOR_QUEUE_ERROR,
                                  BOGNOR_QUEUE_ERROR_EMPTY,
                                  "TRACKER: metadata is not available for item %d", index);
        }
        return FALSE;
    }
    return TRUE;
}

gboolean
bognor_queue_get_next_metadata (BognorQueue   *queue,
                                char         **title,
                                char         **artist,
                                char         **album,
                                GError       **error)
{
    BognorQueuePrivate *priv = queue->priv;
    BognorQueueItem *item;
    int index = bognor_queue_next_index(queue);
    if (index == -1) {
        if (error != NULL) {
            *error = g_error_new (BOGNOR_QUEUE_ERROR,
                                  BOGNOR_QUEUE_ERROR_EMPTY,
                                  "no more next");
        }
        return FALSE;
    } else {
        return bognor_queue_get_index_metadata(queue, index, title, artist, album, error);
    }
}

gboolean
bognor_queue_get_index_uri (BognorQueue *queue,
                            int          index,
                            char       **uri,
                            char       **mimetype,
                            GError     **error)
{
    BognorQueuePrivate *priv = queue->priv;
    BognorQueueItem *item;

    item = g_queue_peek_nth (priv->play_queue, index);
    if (item == NULL) {
        if (error != NULL) {
            *error = g_error_new (BOGNOR_QUEUE_ERROR,
                                  BOGNOR_QUEUE_ERROR_OUT_OF_RANGE,
                                  "%d is out of range for queue with %d items",
                                  index, g_queue_get_length (priv->play_queue));
        }
        return FALSE;
    }

    *uri = g_strdup (bognor_queue_item_get_uri (item));
    *mimetype = g_strdup (bognor_queue_item_get_mimetype (item));

    return TRUE;
}

gboolean
bognor_queue_set_repeat_mode (BognorQueue *queue,
                              int          mode,
                              GError     **error)
{
    BognorQueuePrivate *priv = queue->priv;

    if ((mode & 0x80) == BOGNOR_QUEUE_MODE_SHUFFLE) {
        bq_make_shuffle_list(queue);
        if (priv->state == BOGNOR_QUEUE_STATE_STOPPED && (priv->shuffle_array_size > 0)) {
            bognor_queue_set_index (queue, priv->shuffle_array[0], error);
            priv->current_index = 0;
        }
    }
    priv->mode = mode;
    return TRUE;
}

gboolean
bognor_queue_get_repeat_mode (BognorQueue *queue,
                              int         *mode,
                              GError     **error)
{
    BognorQueuePrivate *priv = queue->priv;

    *mode = priv->mode;
    return TRUE;
}

gboolean
bognor_queue_get_duration (BognorQueue *queue,
                           int         *duration,
                           GError     **error)
{
    BognorQueuePrivate *priv = queue->priv;
    if (priv->current_position == NULL) {
        *duration = 0;
    } else {
        *duration = bognor_queue_item_get_duration(priv->current_position->data);
    }
    return TRUE;
}

gboolean
bognor_queue_list_uris (BognorQueue *queue,
                        char      ***uris,
                        GError     **error)
{
    BognorQueuePrivate *priv = queue->priv;
    char **uri_array;
    GList *u = NULL;
    int n_uris, i;

    n_uris = g_queue_get_length (priv->play_queue);
    uri_array = g_new (char *, n_uris + 1);

    for (u = priv->play_queue->head, i = 0; u; u = u->next, i++) {
        BognorQueueItem *item = u->data;

        uri_array[i] = g_strdup (bognor_queue_item_get_uri (item));
    }

    /* NULL terminate the array */
    uri_array[n_uris] = NULL;

    *uris = uri_array;

    return TRUE;
}

gboolean
bognor_queue_get_state (BognorQueue *queue,
                        int         *state,
                        GError     **error)
{
    BognorQueuePrivate *priv = queue->priv;

    *state = priv->state;
    return TRUE;
}

gboolean
bognor_queue_get_name (BognorQueue *queue,
                       char       **name,
                       GError     **error)
{
    BognorQueuePrivate *priv = queue->priv;

    *name = g_strdup (priv->name);
    return TRUE;
}

int
bognor_queue_get_count (BognorQueue *queue)
{
    BognorQueuePrivate *priv = queue->priv;

    return g_queue_get_length(priv->play_queue);
}

int
bognor_queue_next_index (BognorQueue *queue)
{
    BognorQueuePrivate *priv = queue->priv;
    int index;

    if ((priv->mode & 0x7f) == BOGNOR_QUEUE_MODE_SINGLE_REPEATING) {
        if( (priv->mode & 0x80) == BOGNOR_QUEUE_MODE_SHUFFLE) {
            return priv->shuffle_array[priv->current_index];
        } else {
            return priv->current_index;
        }
    }

    if (priv->current_index == (bognor_queue_get_count(queue)-1)) {
        // last song
        if ((priv->mode & 0x7f) == BOGNOR_QUEUE_MODE_NORMAL) {
            // play no more
            return -1;
        } else {
            // repeat mode
            if( (priv->mode & 0x80) == BOGNOR_QUEUE_MODE_SHUFFLE) {
                return priv->shuffle_array[0];
            } else {
                return 0;
            }
        }
    }

    index = priv->current_index+1;
    if( (priv->mode & 0x80) == BOGNOR_QUEUE_MODE_SHUFFLE) {
        // shuffle
        return priv->shuffle_array[index];
    } else {
        // shuffle
        return index;
    }
}

