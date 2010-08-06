#include <bickley/bkl-source-manager-client.h>
#include <bickley/bkl-source-client.h>

#include <bickley/bkl-item.h>

#include "bognor-source-manager.h"

enum {
    PROP_0,
};

enum {
    LAST_SIGNAL,
};

struct _BognorSourceManagerPrivate {
    BklSourceManagerClient *manager;

    BklSourceClient *local_source;
    GList *transient_sources;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), BOGNOR_TYPE_SOURCE_MANAGER, BognorSourceManagerPrivate))
G_DEFINE_TYPE (BognorSourceManager, bognor_source_manager, G_TYPE_OBJECT);

static void
bognor_source_manager_finalize (GObject *object)
{
    BognorSourceManager *self = (BognorSourceManager *) object;

    G_OBJECT_CLASS (bognor_source_manager_parent_class)->finalize (object);
}

static void
bognor_source_manager_dispose (GObject *object)
{
    BognorSourceManager *self = (BognorSourceManager *) object;

    G_OBJECT_CLASS (bognor_source_manager_parent_class)->dispose (object);
}

static void
bognor_source_manager_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
    BognorSourceManager *self = (BognorSourceManager *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bognor_source_manager_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
    BognorSourceManager *self = (BognorSourceManager *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bognor_source_manager_class_init (BognorSourceManagerClass *klass)
{
    GObjectClass *o_class = (GObjectClass *) klass;

    o_class->dispose = bognor_source_manager_dispose;
    o_class->finalize = bognor_source_manager_finalize;
    o_class->set_property = bognor_source_manager_set_property;
    o_class->get_property = bognor_source_manager_get_property;

    g_type_class_add_private (klass, sizeof (BognorSourceManagerPrivate));
}

static void
source_ready_cb (BklSourceClient     *source_client,
                 BognorSourceManager *manager)
{
    BognorSourceManagerPrivate *priv = manager->priv;

    priv->transient_sources = g_list_prepend (priv->transient_sources,
                                              source_client);
}

static void
source_added_cb (BklSourceManagerClient *client,
                 const char             *object_path,
                 BognorSourceManager    *manager)
{
    BklSourceClient *source;

    source = bkl_source_client_new (object_path);
    g_signal_connect (source, "ready",
                      G_CALLBACK (source_ready_cb), manager);
}

static void
source_removed_cb (BklSourceManagerClient *client,
                   const char             *object_path,
                   BognorSourceManager    *manager)
{
    BognorSourceManagerPrivate *priv = manager->priv;
    GList *s;

    for (s = priv->transient_sources; s; s = s->next) {
        BklSourceClient *client = s->data;

        if (g_str_equal (object_path,
                         bkl_source_client_get_path (client))) {
            g_object_unref (client);
            priv->transient_sources = g_list_delete_link (priv->transient_sources, s);
            break;
        }
    }
}

static void
local_source_ready (BklSourceClient     *client,
                    BognorSourceManager *manager)
{
    BognorSourceManagerPrivate *priv = manager->priv;

    priv->local_source = client;
}

static void
get_sources_reply (BklSourceManagerClient *client,
                   GList                  *sources,
                   GError                 *error,
                   gpointer                userdata)
{
    BognorSourceManager *manager = (BognorSourceManager *) userdata;
    BognorSourceManagerPrivate *priv = manager->priv;
    BklSourceClient *ls;
    GList *s;

    if (error != NULL) {
        g_warning ("Error getting sources: %s", error->message);
        g_error_free (error);
    }

    ls = bkl_source_client_new (BKL_LOCAL_SOURCE_PATH);
    g_signal_connect (ls, "ready",
                      G_CALLBACK (local_source_ready), manager);

    for (s = sources; s; s = s->next) {
        source_added_cb (client, s->data, manager);
    }

    g_list_free (sources);
}

static void
bkl_manager_ready_cb (BklSourceManagerClient *manager_client,
                      BognorSourceManager    *manager)
{
    BognorSourceManagerPrivate *priv = manager->priv;

    bkl_source_manager_client_get_sources (priv->manager,
                                           get_sources_reply, manager);
}

static void
bognor_source_manager_init (BognorSourceManager *self)
{
    BognorSourceManagerPrivate *priv = GET_PRIVATE (self);

    self->priv = priv;

    priv->manager = g_object_new (BKL_TYPE_SOURCE_MANAGER_CLIENT, NULL);
    g_signal_connect (priv->manager, "ready",
                      G_CALLBACK (bkl_manager_ready_cb), self);
    g_signal_connect (priv->manager, "source-added",
                      G_CALLBACK (source_added_cb), self);
    g_signal_connect (priv->manager, "source-removed",
                      G_CALLBACK (source_removed_cb), self);
}

static BklItem *
find_item (BklSourceClient *client,
           const char      *uri)
{
    BklDB *db = bkl_source_client_get_db (client);
    BklItem *item = NULL;
    GError *error = NULL;

    item = bkl_db_get_item (db, uri, &error);
    if (error != NULL) {
        if (error->code != KOZO_DB_ERROR_KEY_NOT_FOUND) {
            g_warning ("Error getting item for %s: %s", uri,
                       error->message);

            g_error_free (error);
            return NULL;
        }

        g_error_free (error);
    }

    return item;
}

BklItem *
bognor_source_manager_find_item (BognorSourceManager *self,
                                 const char          *uri)
{
    BognorSourceManagerPrivate *priv = self->priv;
    GList *s;
    BklItem *item;

    item = find_item (priv->local_source, uri);
    if (item) {
        return item;
    }

    for (s = priv->transient_sources; s; s = s->next) {
        BklSourceClient *client = s->data;

        item = find_item (client, uri);
        if (item) {
            return item;
        }
    }

    return NULL;
}
