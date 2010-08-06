#include <libgupnp-av/gupnp-av.h>

#include "bognor-queue.h"
#include "bognor-last-change.h"
#include "bognor-av-transport.h"

enum {
    PROP_0,
};

enum {
    LAST_SIGNAL,
};

struct _BognorAvTransportPrivate {
    BognorQueue *queue;
    BognorQueueItem *item;
    BognorLastChange *lc;
};

static const char *states[BOGNOR_QUEUE_LAST_STATE] = {
    "STOPPED",
    "PLAYING"
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), BOGNOR_TYPE_AV_TRANSPORT, BognorAvTransportPrivate))
G_DEFINE_TYPE (BognorAvTransport, bognor_av_transport, GUPNP_TYPE_SERVICE);

static void
bognor_av_transport_finalize (GObject *object)
{
    BognorAvTransport *self = (BognorAvTransport *) object;

    G_OBJECT_CLASS (bognor_av_transport_parent_class)->finalize (object);
}

static void
bognor_av_transport_dispose (GObject *object)
{
    BognorAvTransport *self = (BognorAvTransport *) object;

    G_OBJECT_CLASS (bognor_av_transport_parent_class)->dispose (object);
}

static void
bognor_av_transport_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
    BognorAvTransport *self = (BognorAvTransport *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bognor_av_transport_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
    BognorAvTransport *self = (BognorAvTransport *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bognor_av_transport_class_init (BognorAvTransportClass *klass)
{
    GObjectClass *o_class = (GObjectClass *) klass;
    GUPnPServiceClass *s_class = (GUPnPServiceClass *) klass;

    o_class->dispose = bognor_av_transport_dispose;
    o_class->finalize = bognor_av_transport_finalize;
    o_class->set_property = bognor_av_transport_set_property;
    o_class->get_property = bognor_av_transport_get_property;

    g_type_class_add_private (klass, sizeof (BognorAvTransportPrivate));
}

static void
query_variable_cb (GUPnPService      *service,
                   char              *variable,
                   GValue            *value,
                   BognorAvTransport *transport)
{
    BognorAvTransportPrivate *priv = transport->priv;

    g_print ("[AV Transport] Query %s\n", variable);
    if (g_str_equal (variable, "LastChange")) {
        BognorLastChange *lc = bognor_last_change_new (NULL);
        const char *event;
        BognorQueueState state;
        char *str;
        int idx;

        bognor_queue_get_state (priv->queue, (int *) &state, NULL);

        bognor_last_change_add (lc, "TransportState", states[state]);
        bognor_last_change_add (lc, "TransportStatus", "OK");
        bognor_last_change_add (lc, "PlaybackStorageMedium", "NOT_IMPLEMENTED");
        bognor_last_change_add (lc, "RecordStorageMedium", "NOT_IMPLEMENTED");
        bognor_last_change_add (lc, "PossiblePlaybackStorageMedia", "NOT_IMPLEMENTED");
        bognor_last_change_add (lc, "PossibleRecordStorageMedia", "NOT_IMPLEMENTED");
        bognor_last_change_add (lc, "CurrentPlayMode", "Normal");
        bognor_last_change_add (lc, "TransportPlaySpeed", "1");
        bognor_last_change_add (lc, "RecordMediumWriteStatus", "NOT_IMPLEMENTED");
        bognor_last_change_add (lc, "CurrentRecordQualityMode", "NOT_IMPLEMENTED");
        bognor_last_change_add (lc, "PossibleRecordQualityMode", "NOT_IMPLEMENTED");

        str = g_strdup_printf ("%d", bognor_queue_get_count (priv->queue));
        bognor_last_change_add (lc, "NumberOfTracks", str);
        g_free (str);

        bognor_queue_get_index (priv->queue, &idx, NULL);
        str = g_strdup_printf ("%d", idx);
        bognor_last_change_add (lc, "CurrentTrack", str);
        g_free (str);

        bognor_last_change_add (lc, "CurrentTrackDuration", "0:00:00");
        bognor_last_change_add (lc, "CurrentMediaDuration", "0:00:00");

        if (priv->item) {
            bognor_last_change_add (lc, "CurrentTrackMetadata",
                                    bognor_queue_item_get_metadata (priv->item));
        } else {
            bognor_last_change_add (lc, "CurrentTrackMetadata", "");
        }

        if (priv->item) {
            bognor_last_change_add (lc, "CurrentTrackURI",
                                    bognor_queue_item_get_uri (priv->item));
        } else {
            bognor_last_change_add (lc, "CurrentTrackURI", "");
        }

        bognor_last_change_add (lc, "AVTransportURI", "");
        bognor_last_change_add (lc, "NextAVTransportURI", "");

        event = bognor_last_change_finish (lc);
        g_print ("LastChange: %s\n", event);

        g_value_init (value, G_TYPE_STRING);
        g_value_set_string (value, event);

        bognor_last_change_free (lc);
    }
}

static gboolean
check_instance_id (GUPnPServiceAction *action)
{
    int instance_id;

    gupnp_service_action_get (action,
                              "InstanceID", G_TYPE_INT, &instance_id,
                              NULL);

    if (instance_id != 0) {
        gupnp_service_action_return_error (action, 718, "Invalid InstanceID");
        return FALSE;
    }

    return TRUE;
}

struct _set_av_transport_uri_data {
    BognorAvTransport *transport;
    GUPnPServiceAction *action;
    char *uri;
    char *metadata;
};

static void
set_av_transport_object_avail_cb (GUPnPDIDLLiteParser               *parser,
                                  GUPnPDIDLLiteObject               *object,
                                  struct _set_av_transport_uri_data *data)
{
    BognorAvTransportPrivate *priv = data->transport->priv;
    GList *res;
    GUPnPProtocolInfo *info;
    const char *mimetype;
    const char *uris[1] = {0,}, *mimetypes[1] = {0,};

    res = gupnp_didl_lite_object_get_resources (object);
    if (res == NULL) {
        gupnp_service_action_return_error (data->action,
                                           714, "Illegal MIME type");
        return;
    }

    info = gupnp_didl_lite_resource_get_protocol_info (res->data);
    mimetype = gupnp_protocol_info_get_mime_type (info);

    if (g_str_has_prefix (mimetype, "audio/") == FALSE) {
        gupnp_service_action_return_error (data->action,
                                           714, "Illegal MIME type");
        return;
    }

    uris[0] = data->uri;
    mimetypes[0] = mimetype;
    bognor_queue_append_uris (priv->queue, 1, uris, mimetypes, NULL);
    bognor_queue_set_index (priv->queue, BOGNOR_QUEUE_INDEX_END, NULL);

    gupnp_service_action_return (data->action);
}

static void
set_av_transport_uri_cb (GUPnPService       *service,
                         GUPnPServiceAction *action,
                         BognorAvTransport  *transport)
{
    BognorAvTransportPrivate *priv = transport->priv;
    struct _set_av_transport_uri_data data;
    GUPnPDIDLLiteParser *parser;
    char *uri, *metadata;
    GError *error = NULL;

    if (check_instance_id (action) == FALSE) {
        return;
    }

    gupnp_service_action_get (action,
                              "CurrentURI", G_TYPE_STRING, &uri,
                              "CurrentURIMetaData", G_TYPE_STRING, &metadata,
                              NULL);
    g_print ("Uri: %s\nMetadata: %s\n", uri, metadata);

    data.transport = transport;
    data.action = action;
    data.uri = uri;
    data.metadata = metadata;

    parser = gupnp_didl_lite_parser_new ();
    g_signal_connect (parser, "object-available",
                      G_CALLBACK (set_av_transport_object_avail_cb), &data);
    gupnp_didl_lite_parser_parse_didl (parser, metadata, &error);
    if (error != NULL) {
        g_warning ("Error parsing DIDL: %s\n", metadata);
        g_error_free (error);
        g_free (uri);
        g_free (metadata);

        gupnp_service_action_return_error (action, 714, "Illegal MIME type");
        return;
    }

    g_free (uri);
    g_free (metadata);
}

static void
get_media_info_cb (GUPnPService *service,
                   GUPnPServiceAction *action,
                   BognorAvTransport *transport)
{
    if (check_instance_id (action) == FALSE) {
        return;
    }

    g_print ("[AV Transport] Get media info\n");
    gupnp_service_action_return (action);
}

static void
get_transport_info_cb (GUPnPService *service,
                       GUPnPServiceAction *action,
                       BognorAvTransport *transport)
{
    if (check_instance_id (action) == FALSE) {
        return;
    }

    g_print ("[AV Transport] Get transport info\n");
    gupnp_service_action_return (action);
}

static char *
seconds_to_duration (guint seconds)
{
    int hours, minutes;

    hours = seconds / 3600;
    seconds -= (hours * 3600);

    minutes = seconds / 60;
    seconds -= (minutes * 60);

    return g_strdup_printf ("%d:%02d:%02d", hours, minutes, seconds);
}

static void
get_position_info_cb (GUPnPService       *service,
                      GUPnPServiceAction *action,
                      BognorAvTransport  *transport)
{
    BognorAvTransportPrivate *priv = transport->priv;
    int seconds, position, idx;
    double p;
    char *duration, *pos;
    const char *metadata, *uri;

    if (check_instance_id (action) == FALSE) {
        return;
    }

    if (priv->item) {
        seconds = bognor_queue_item_get_duration (priv->item);
        metadata = bognor_queue_item_get_metadata (priv->item);
        uri = bognor_queue_item_get_uri (priv->item);
    } else {
        gupnp_service_action_return_error (action, 402, "Invalid Args");
        return;
    }

    duration = seconds_to_duration (seconds);

    p = 0.0;
    bognor_queue_get_position (priv->queue, &p, NULL);
    position = seconds * p;
    pos = seconds_to_duration (position);

    bognor_queue_get_index (priv->queue, &idx, NULL);
    gupnp_service_action_set (action,
                              "Track", G_TYPE_INT, idx,
                              "TrackDuration", G_TYPE_STRING, duration,
                              "TrackMetadata", G_TYPE_STRING,
                              metadata ? metadata : "",
                              "TrackURI", G_TYPE_STRING,
                              uri ? uri : "",
                              "RelTime", G_TYPE_STRING, pos,
                              "AbsTime", G_TYPE_STRING, pos,
                              "RelCount", G_TYPE_INT, G_MAXINT,
                              "AbsCount", G_TYPE_INT, G_MAXINT,
                              NULL);

    g_free (pos);
    g_free (duration);

    gupnp_service_action_return (action);
}

static void
get_device_capabilities_cb (GUPnPService *service,
                            GUPnPServiceAction *action,
                            BognorAvTransport *transport)
{
    if (check_instance_id (action) == FALSE) {
        return;
    }

    g_print ("[AV Transport] Get device capabilities\n");
    gupnp_service_action_return (action);
}

static void
get_transport_settings_cb (GUPnPService *service,
                           GUPnPServiceAction *action,
                           BognorAvTransport *transport)
{
    if (check_instance_id (action) == FALSE) {
        return;
    }

    g_print ("[AV Transport] Get transport settings\n");
    gupnp_service_action_return (action);
}

static void
stop_cb (GUPnPService *service,
         GUPnPServiceAction *action,
         BognorAvTransport *transport)
{
    BognorAvTransportPrivate *priv = transport->priv;

    g_print ("[AV Transport] - Stop\n");
    if (check_instance_id (action) == FALSE) {
        return;
    }

    bognor_queue_stop (priv->queue, NULL);
    gupnp_service_action_return (action);
}

static void
play_cb (GUPnPService *service,
         GUPnPServiceAction *action,
         BognorAvTransport *transport)
{
    BognorAvTransportPrivate *priv = transport->priv;

    g_print ("[AV Transport] - Play\n");
    if (check_instance_id (action) == FALSE) {
        return;
    }

    bognor_queue_play (priv->queue, NULL);
    gupnp_service_action_return (action);
}

static void
pause_cb (GUPnPService *service,
          GUPnPServiceAction *action,
          BognorAvTransport *transport)
{
    BognorAvTransportPrivate *priv = transport->priv;

    g_print ("[AV Transport] - Pause\n");
    if (check_instance_id (action) == FALSE) {
        return;
    }

    bognor_queue_stop (priv->queue, NULL);
    gupnp_service_action_return (action);
}

static void
seek_cb (GUPnPService *service,
         GUPnPServiceAction *action,
         BognorAvTransport *transport)
{
    BognorAvTransportPrivate *priv = transport->priv;
    char *unit, *target;

    g_print ("[AV Transport] - Seek\n");
    if (check_instance_id (action) == FALSE) {
        return;
    }

    gupnp_service_action_get (action,
                              "Unit", G_TYPE_STRING, &unit,
                              "Target", G_TYPE_STRING, &target,
                              NULL);
    if (g_str_equal (unit, "ABS_TIME")) {
        int hours, minutes, seconds;
        int duration, position;

        sscanf (target, "%d:%2d:%2d*s", &hours, &minutes, &seconds);

        position = hours * 3600 + minutes * 60 + seconds;
        if (priv->item) {
            duration = bognor_queue_item_get_duration (priv->item);
        } else {
            duration = -1;
        }

        if (duration == -1) {
            gupnp_service_action_return_error (action, 710,
                                               "Seek mode not supported");
            return;
        }

        bognor_queue_set_position (priv->queue,
                                   (double) position / (double) duration, NULL);
    }

    gupnp_service_action_return (action);
}

static void
next_cb (GUPnPService *service,
         GUPnPServiceAction *action,
         BognorAvTransport *transport)
{
    BognorAvTransportPrivate *priv = transport->priv;

    g_print ("[AV Transport] - Next\n");
    if (check_instance_id (action) == FALSE) {
        return;
    }

    bognor_queue_next (priv->queue, NULL);
    gupnp_service_action_return (action);
}

static void
previous_cb (GUPnPService *service,
             GUPnPServiceAction *action,
             BognorAvTransport *transport)
{
    BognorAvTransportPrivate *priv = transport->priv;

    if (check_instance_id (action) == FALSE) {
        return;
    }

    bognor_queue_previous (priv->queue, NULL);
    gupnp_service_action_return (action);
}

static void
bognor_av_transport_init (BognorAvTransport *self)
{
    BognorAvTransportPrivate *priv = GET_PRIVATE (self);

    self->priv = priv;

    priv->lc = bognor_last_change_new ((GUPnPService *) self);

    g_signal_connect (self, "query-variable",
                      G_CALLBACK (query_variable_cb), self);
    g_signal_connect (self, "action-invoked::SetAVTransportURI",
                      G_CALLBACK (set_av_transport_uri_cb), self);
    g_signal_connect (self, "action-invoked::GetMediaInfo",
                      G_CALLBACK (get_media_info_cb), self);
    g_signal_connect (self, "action-invoked::GetTransportInfo",
                      G_CALLBACK (get_transport_info_cb), self);
    g_signal_connect (self, "action-invoked::GetPositionInfo",
                      G_CALLBACK (get_position_info_cb), self);
    g_signal_connect (self, "action-invoked::GetDeviceCapabilities",
                      G_CALLBACK (get_device_capabilities_cb), self);
    g_signal_connect (self, "action-invoked::GetTransportSettings",
                      G_CALLBACK (get_transport_settings_cb), self);
    g_signal_connect (self, "action-invoked::Stop",
                      G_CALLBACK (stop_cb), self);
    g_signal_connect (self, "action-invoked::Play",
                      G_CALLBACK (play_cb), self);
    g_signal_connect (self, "action-invoked::Pause",
                      G_CALLBACK (pause_cb), self);
    g_signal_connect (self, "action-invoked::Seek",
                      G_CALLBACK (seek_cb), self);
    g_signal_connect (self, "action-invoked::Next",
                      G_CALLBACK (next_cb), self);
    g_signal_connect (self, "action-invoked::Previous",
                      G_CALLBACK (previous_cb), self);
}

static void
current_item_destroyed (gpointer data,
                        GObject *dead_object)
{
    BognorAvTransport *transport = (BognorAvTransport *) data;
    BognorAvTransportPrivate *priv = transport->priv;

    priv->item = NULL;
}

static void
queue_item_changed (BognorQueue       *queue,
                    BognorQueueItem   *item,
                    BognorAvTransport *transport)
{
    BognorAvTransportPrivate *priv = transport->priv;

    if (priv->item) {
        g_object_weak_unref ((GObject *) priv->item,
                             current_item_destroyed, transport);
    }

    priv->item = item;
    g_object_weak_ref ((GObject *) priv->item,
                       current_item_destroyed, transport);

    if (priv->item) {
        bognor_last_change_add (priv->lc, "CurrentTrackMetadata",
                                bognor_queue_item_get_metadata (priv->item));
    } else {
        bognor_last_change_add (priv->lc, "CurrentTrackMetadata", "");
    }

    if (priv->item) {
        bognor_last_change_add (priv->lc, "CurrentTrackURI",
                                bognor_queue_item_get_uri (priv->item));
    } else {
        bognor_last_change_add (priv->lc, "CurrentTrackURI", "");
    }

    bognor_last_change_add (priv->lc, "AVTransportURI", "");
    bognor_last_change_add (priv->lc, "NextAVTransportURI", "");
}

static void
state_changed (BognorQueue       *queue,
               BognorQueueState   state,
               BognorAvTransport *transport)
{
    BognorAvTransportPrivate *priv = transport->priv;

    bognor_last_change_add (priv->lc, "TransportState", states[state]);
}

void
bognor_av_transport_set_queue (BognorAvTransport *transport,
                               BognorQueue       *queue)
{
    BognorAvTransportPrivate *priv = transport->priv;

    priv->queue = queue;
    g_signal_connect (queue, "item-changed",
                      G_CALLBACK (queue_item_changed), transport);
    g_signal_connect (queue, "state-changed",
                      G_CALLBACK (state_changed), transport);
}
