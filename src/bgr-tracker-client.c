#include <libtracker-client/tracker-client.h>
#include "bgr-item.h"
#include "bgr-tracker-client.h"

enum {
    PROP_0,
};

enum {
    LAST_SIGNAL,
};

struct _BgrTrackerClientPrivate {
    TrackerClient *client;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), BGR_TYPE_TRACKER_CLIENT, BgrTrackerClientPrivate))
G_DEFINE_TYPE (BgrTrackerClient, bgr_tracker_client, G_TYPE_OBJECT);

#define URI 0
#define MIMETYPE 1
#define DATE 2
#define TITLE 3
#define ARTIST 4
#define ALBUM 5
#define TRACK 6
#define USE 7
#define ITEM_QUERY "SELECT nie:url(?uri) nie:mimeType(?uri) nie:contentCreated(?uri) nie:title(?uri) nmm:artistName(nmm:performer(?uri)) nmm:albumTitle(nmm:musicAlbum(?uri)) nmm:trackNumber(?uri) nie:usageCounter(?uri)" \
    "WHERE { ?uri nie:url \"%s\" }"

#define UPDATE_QUERY "DELETE { ?uri nie:usageCounter ?count } "\
    " WHERE { ?uri nie:usageCounter ?count ; "                 \
    "              nie:isStoredAs ?as ."                       \
    "         ?as nie:url \"%s\" . }"                          \
    " INSERT { ?uri nie:usageCounter %d } "                    \
    " WHERE { ?uri nie:isStoredAs ?as ."                       \
    "         ?as nie:url \"%s\" }"

static void
bgr_tracker_client_finalize (GObject *object)
{
    BgrTrackerClient *self = (BgrTrackerClient *) object;

    G_OBJECT_CLASS (bgr_tracker_client_parent_class)->finalize (object);
}

static void
bgr_tracker_client_dispose (GObject *object)
{
    BgrTrackerClient *self = (BgrTrackerClient *) object;

    G_OBJECT_CLASS (bgr_tracker_client_parent_class)->dispose (object);
}

static void
bgr_tracker_client_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    BgrTrackerClient *self = (BgrTrackerClient *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bgr_tracker_client_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
    BgrTrackerClient *self = (BgrTrackerClient *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bgr_tracker_client_class_init (BgrTrackerClientClass *klass)
{
    GObjectClass *o_class = (GObjectClass *) klass;

    o_class->dispose = bgr_tracker_client_dispose;
    o_class->finalize = bgr_tracker_client_finalize;
    o_class->set_property = bgr_tracker_client_set_property;
    o_class->get_property = bgr_tracker_client_get_property;

    g_type_class_add_private (klass, sizeof (BgrTrackerClientPrivate));
}

static void
bgr_tracker_client_init (BgrTrackerClient *self)
{
    BgrTrackerClientPrivate *priv = GET_PRIVATE (self);

    self->priv = priv;
    priv->client = tracker_client_new (FALSE, G_MAXINT);
}

BgrItem *
bgr_tracker_client_get_item (BgrTrackerClient *client,
                             const char       *uri)
{
    BgrTrackerClientPrivate *priv = client->priv;
    BgrItem *item;
    GError *error = NULL;
    GPtrArray *results;
    char *query;
    char **result;

    query = g_strdup_printf (ITEM_QUERY, uri);
    results = tracker_resources_sparql_query (priv->client, query, &error);
    if (error != NULL) {
        g_warning ("Error querying for %s: %s", uri, error->message);
        g_error_free (error);

        return NULL;
    }

    if (results == NULL) {
        return NULL;
    }

    if (results->len == 0) {
        g_ptr_array_free (results, TRUE);
        return NULL;
    }

    result = results->pdata[0];
    item = bgr_item_new (result[URI], result[MIMETYPE]);
    bgr_item_set_metadata (item, BGR_ITEM_METADATA_TITLE, result[TITLE]);
    bgr_item_set_metadata (item, BGR_ITEM_METADATA_ARTIST, result[ARTIST]);
    bgr_item_set_metadata (item, BGR_ITEM_METADATA_ALBUM, result[ALBUM]);

    if (result[USE] == NULL || result[USE] == '\0') {
        bgr_item_set_metadata (item, BGR_ITEM_METADATA_USE_COUNT, "0");
    } else {
        bgr_item_set_metadata (item, BGR_ITEM_METADATA_USE_COUNT, result[USE]);
    }

    g_strfreev (result);
    g_ptr_array_free (results, TRUE);

    return item;
}

void
bgr_tracker_client_update_item_use_count (BgrTrackerClient *client,
                                          BgrItem          *item)
{
    BgrTrackerClientPrivate *priv = client->priv;
    GError *error = NULL;
    const char *use_count;
    int count;
    char *query;

    g_return_if_fail (item != NULL);

    use_count = bgr_item_get_metadata (item, BGR_ITEM_METADATA_USE_COUNT);
    if (use_count == NULL) {
        count = 1;
    } else {
        count = atoi (use_count) + 1;
    }

    query = g_strdup_printf (UPDATE_QUERY, bgr_item_get_uri (item), count,
                             bgr_item_get_uri (item));

    /* FIXME: async? */
    tracker_resources_sparql_update (priv->client, query, &error);

    if (error != NULL) {
        g_warning ("Error updating %s: %s", bgr_item_get_uri (item),
                   error->message);
        g_error_free (error);
    }

    g_free (query);
}
