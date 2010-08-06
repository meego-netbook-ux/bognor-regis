#include <glib.h>

#include <libgupnp-av/gupnp-av.h>

#include "bgr-item.h"
#include "bognor-queue-item.h"

enum {
    PROP_0,
};

enum {
    DURATION_CHANGED,
    POSITION_CHANGED,
    LAST_SIGNAL,
};

struct _BognorQueueItemPrivate {
    BgrItem *item;
    char *metadata;
    char *uri;
    char *mimetype;
    int duration; /* In seconds */
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), BOGNOR_TYPE_QUEUE_ITEM, BognorQueueItemPrivate))
G_DEFINE_TYPE (BognorQueueItem, bognor_queue_item, G_TYPE_OBJECT);
static guint32 signals[LAST_SIGNAL] = {0,};

static void
bognor_queue_item_finalize (GObject *object)
{
    BognorQueueItem *self = (BognorQueueItem *) object;
    BognorQueueItemPrivate *priv = self->priv;

    g_free (priv->uri);
    g_free (priv->mimetype);
    g_free (priv->metadata);

    G_OBJECT_CLASS (bognor_queue_item_parent_class)->finalize (object);
}

static void
bognor_queue_item_dispose (GObject *object)
{
    BognorQueueItem *self = (BognorQueueItem *) object;
    BognorQueueItemPrivate *priv = self->priv;

    if (priv->item) {
        g_object_unref (priv->item);
        priv->item = NULL;
        priv->uri = NULL;
        priv->mimetype = NULL;
    }

    G_OBJECT_CLASS (bognor_queue_item_parent_class)->dispose (object);
}

static void
bognor_queue_item_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
    BognorQueueItem *self = (BognorQueueItem *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bognor_queue_item_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
    BognorQueueItem *self = (BognorQueueItem *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bognor_queue_item_class_init (BognorQueueItemClass *klass)
{
    GObjectClass *o_class = (GObjectClass *) klass;

    o_class->dispose = bognor_queue_item_dispose;
    o_class->finalize = bognor_queue_item_finalize;
    o_class->set_property = bognor_queue_item_set_property;
    o_class->get_property = bognor_queue_item_get_property;

    g_type_class_add_private (klass, sizeof (BognorQueueItemPrivate));

    signals[DURATION_CHANGED] = g_signal_new ("duration-changed",
                                              G_TYPE_FROM_CLASS (klass),
                                              G_SIGNAL_RUN_FIRST |
                                              G_SIGNAL_NO_RECURSE,
                                              0, NULL, NULL,
                                              g_cclosure_marshal_VOID__INT,
                                              G_TYPE_NONE, 1, G_TYPE_INT);
    signals[POSITION_CHANGED] = g_signal_new ("position-changed",
                                              G_TYPE_FROM_CLASS (klass),
                                              G_SIGNAL_RUN_FIRST |
                                              G_SIGNAL_NO_RECURSE,
                                              0, NULL, NULL,
                                              g_cclosure_marshal_VOID__INT,
                                              G_TYPE_NONE, 1, G_TYPE_INT);
}

static void
bognor_queue_item_init (BognorQueueItem *self)
{
    BognorQueueItemPrivate *priv = GET_PRIVATE (self);
    self->priv = priv;
}

static char *
bgr_to_didl_metadata (BgrItem *bgr)
{
    GUPnPDIDLLiteWriter *writer;
    GUPnPDIDLLiteItem *item;
    GUPnPDIDLLiteObject *object;
    const char *str, *artist;
    char *didl;

    writer = gupnp_didl_lite_writer_new (NULL);
    item = gupnp_didl_lite_writer_add_item (writer);
    object = (GUPnPDIDLLiteObject *) item;

    str = bgr_item_get_metadata (bgr, BGR_ITEM_METADATA_TITLE);
    if (str) {
        gupnp_didl_lite_object_set_title (object, str);
    } else {
        char *basename = g_path_get_basename (bgr_item_get_uri (bgr));

        gupnp_didl_lite_object_set_title (object, basename);
        g_free (basename);
    }

    artist = bgr_item_get_metadata (bgr, BGR_ITEM_METADATA_ARTIST);
    if (artist) {
        gupnp_didl_lite_object_set_artist (object, artist);
    }

    str = bgr_item_get_metadata (bgr, BGR_ITEM_METADATA_ALBUM);
    if (str) {
        gupnp_didl_lite_object_set_album (object, str);
    }

    didl = gupnp_didl_lite_writer_get_string (writer);
    g_object_unref (writer);

    return didl;
}

BognorQueueItem *
bognor_queue_item_new_from_item (BgrItem *bgr)
{
    BognorQueueItem *item;
    BognorQueueItemPrivate *priv;

    item = g_object_new (BOGNOR_TYPE_QUEUE_ITEM, NULL);
    priv = item->priv;

    priv->item = g_object_ref (bgr);

    /* We don't duplicate these here as they're not freed
       if priv->item is !NULL */
    priv->uri = (char *) bgr_item_get_uri (bgr);
    priv->mimetype = (char *) bgr_item_get_mimetype (bgr);

    priv->metadata = bgr_to_didl_metadata (bgr);
    priv->duration = 0;

    return item;
}

BognorQueueItem *
bognor_queue_item_new (const char *uri,
                       const char *mimetype,
                       const char *metadata)
{
    BognorQueueItem *item;
    BognorQueueItemPrivate *priv;

    item = g_object_new (BOGNOR_TYPE_QUEUE_ITEM, NULL);
    priv = item->priv;

    priv->item = NULL;
    priv->uri = g_strdup (uri);
    priv->mimetype = g_strdup (mimetype);
    priv->metadata = g_strdup (metadata);
    priv->duration = 0;

    return item;
}

void
bognor_queue_item_set_duration (BognorQueueItem *item,
                                int              duration)
{
    BognorQueueItemPrivate *priv = item->priv;

    if (priv->duration == duration) {
        return;
    }

    priv->duration = duration;
    g_signal_emit (item, signals[DURATION_CHANGED], 0, duration);
}

int
bognor_queue_item_get_duration (BognorQueueItem *item)
{
    BognorQueueItemPrivate *priv = item->priv;

    return priv->duration;
}

const char *
bognor_queue_item_get_uri (BognorQueueItem *item)
{
    BognorQueueItemPrivate *priv = item->priv;

    return priv->uri;
}

const char *
bognor_queue_item_get_mimetype (BognorQueueItem *item)
{
    BognorQueueItemPrivate *priv = item->priv;

    return priv->mimetype;
}

const char *
bognor_queue_item_get_metadata (BognorQueueItem *item)
{
    BognorQueueItemPrivate *priv = item->priv;

    return priv->metadata;
}

BgrItem *
bognor_queue_item_get_item (BognorQueueItem *item)
{
    BognorQueueItemPrivate *priv = item->priv;

    return priv->item;
}
