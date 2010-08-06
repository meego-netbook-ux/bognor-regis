#include "bgr-item.h"

enum {
    PROP_0,
};

enum {
    LAST_SIGNAL,
};

struct _BgrItemPrivate {
    char *uri;
    char *mimetype;
    GHashTable *metadata;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), BGR_TYPE_ITEM, BgrItemPrivate))
G_DEFINE_TYPE (BgrItem, bgr_item, G_TYPE_OBJECT);
static guint32 signals[LAST_SIGNAL] = {0,};

static void
bgr_item_finalize (GObject *object)
{
    BgrItem *self = (BgrItem *) object;

    G_OBJECT_CLASS (bgr_item_parent_class)->finalize (object);
}

static void
bgr_item_dispose (GObject *object)
{
    BgrItem *self = (BgrItem *) object;

    G_OBJECT_CLASS (bgr_item_parent_class)->dispose (object);
}

static void
bgr_item_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
    BgrItem *self = (BgrItem *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bgr_item_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
    BgrItem *self = (BgrItem *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bgr_item_class_init (BgrItemClass *klass)
{
    GObjectClass *o_class = (GObjectClass *) klass;

    o_class->dispose = bgr_item_dispose;
    o_class->finalize = bgr_item_finalize;
    o_class->set_property = bgr_item_set_property;
    o_class->get_property = bgr_item_get_property;

    g_type_class_add_private (klass, sizeof (BgrItemPrivate));
}

static void
bgr_item_init (BgrItem *self)
{
    BgrItemPrivate *priv = GET_PRIVATE (self);

    self->priv = priv;
    priv->metadata = g_hash_table_new_full (g_str_hash, g_str_equal,
                                            g_free, g_free);
}

BgrItem *
bgr_item_new (const char *uri,
              const char *mimetype)
{
    BgrItem *item;
    BgrItemPrivate *priv;

    item = g_object_new (BGR_TYPE_ITEM, NULL);
    priv = item->priv;

    g_print ("%s - %s\n-------\n", uri, mimetype);
    priv->uri = g_strdup (uri);
    priv->mimetype = g_strdup (mimetype);

    return item;
}

const char *
bgr_item_get_uri (BgrItem *item)
{
    BgrItemPrivate *priv = item->priv;

    return priv->uri;
}

const char *
bgr_item_get_mimetype (BgrItem *item)
{
    BgrItemPrivate *priv = item->priv;

    return priv->mimetype;
}

void
bgr_item_set_metadata (BgrItem    *item,
                       const char *key,
                       const char *value)
{
    BgrItemPrivate *priv = item->priv;

    if (value == NULL || *value == '\0') {
        return;
    }

    g_print ("   %s: %s\n", key, value);
    g_hash_table_insert (priv->metadata, g_strdup (key), g_strdup (value));
}

const char *
bgr_item_get_metadata (BgrItem    *item,
                       const char *key)
{
    BgrItemPrivate *priv = item->priv;

    return g_hash_table_lookup (priv->metadata, key);
}
