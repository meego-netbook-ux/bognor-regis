#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <HXClientCFuncs.h>
#include <HXClientCallbacks.h>
#include <HXClientConstants.h>

#include "bognor-local-queue.h"
#include "bognor-player-bindings.h"

#define HELIX_TIMEOUT 200

enum {
    PROP_0,
};

struct _BognorLocalQueuePrivate {
    HXClientPlayerToken player;
    int audio_state;

    guint32 tracker_id;
    gint64 duration;
    guint32 position;
    guint32 length;

    GtkRecentManager *recent_manager;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), BOGNOR_TYPE_LOCAL_QUEUE, BognorLocalQueuePrivate))
G_DEFINE_TYPE (BognorLocalQueue, bognor_local_queue, BOGNOR_TYPE_QUEUE);

static gboolean
kick_helix (gpointer data)
{
    double percent = 0.0;
    BognorQueue *queue = (BognorQueue *) data;
    BognorLocalQueue *local = (BognorLocalQueue *) queue;
    BognorLocalQueuePrivate *priv = local->priv;

    priv->position = ClientPlayerGetPosition (priv->player);
    if (priv->length > 0)
	percent = (double)priv->position / (double)priv->length;

    //g_debug ("tick %f", percent);
    bognor_queue_emit_position_changed (queue, percent);
    ClientEngineProcessXEvent(NULL);
}

static void
OnLengthChanged(void* userInfo, UInt32 length)
{
    BognorLocalQueue *local = (BognorLocalQueue *) userInfo;
    BognorLocalQueuePrivate *priv = local->priv;

    g_debug ("OnLengthChanged: %i", length);
    priv->length = length;
}

static void
OnContentConcluded(void* userInfo)
{
    BognorQueue *queue = (BognorQueue *) userInfo;

    bognor_queue_play_next (queue);
}

static void
OnContentStateChanged(void* userInfo, int oldContentState, int newContentState)
{
    BognorLocalQueue *local = (BognorLocalQueue *) userInfo;
    BognorLocalQueuePrivate *priv = local->priv;

    priv->audio_state = newContentState;
    switch (newContentState)
    {
    case kContentStateNotLoaded:
	g_debug ("new state > not loaded");
	break;

    case kContentStateLoading:
	g_debug ("new state > loading");
	break;

    case kContentStatePaused:
	g_debug ("new state > paused");
	break;

    case kContentStateStopped:
	g_debug ("new state > stopped");
	break;

    case kContentStatePlaying:
	g_debug ("new state > playing");
	break;

    default:
	g_debug ("unexpected state: %i", newContentState);
	break;
    }
}

static void
OnErrorOccurred(void* userInfo,
                UInt32 hxCode,
                UInt32 userCode,
                const char* pErrorString,
                const char* pUserString,
                const char* pMoreInfoURL)
{
    BognorQueue *queue = (BognorQueue *) userInfo;
    BognorQueueItem *item;

    item = bognor_queue_get_current_item (queue);
    if (item && (hxCode == 0x80040011 || /* HXR_NO_RENDERER */
		 hxCode == 0x80040017) /* HXR_REQUEST_UPGRADE */) {
        bognor_queue_notify_unknown_format (queue, item->uri);
    }

    bognor_queue_play_next (queue);
    g_debug ("OnErrorOccurred: %s/%s/%s", pErrorString, pUserString, pMoreInfoURL);
}

static int
RequestUpgrade(void* userInfo, const char* pUrl, UInt32 numOfComponents,
	       const char* componentNames[], bool isBlocking)
{
    BognorQueue *queue = (BognorQueue *) userInfo;
    BognorQueueItem *item;

    item = bognor_queue_get_current_item (queue);
    if (item) {
        bognor_queue_notify_unknown_format (queue, item->uri);
    }

    g_debug ("RequestUpgrade: %s %i", pUrl, numOfComponents);
    return FALSE;
}

static int
HasComponent(void* userInfo, const char* componentName)
{
    g_debug ("HasComponent: %s", componentName);
    return FALSE;
}

static void
bognor_local_queue_finalize (GObject *object)
{
    G_OBJECT_CLASS (bognor_local_queue_parent_class)->finalize (object);
}

static void
bognor_local_queue_dispose (GObject *object)
{
    G_OBJECT_CLASS (bognor_local_queue_parent_class)->dispose (object);
}

static void
bognor_local_queue_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    switch (prop_id) {

    default:
        break;
    }
}

static void
bognor_local_queue_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
    switch (prop_id) {

    default:
        break;
    }
}

static gboolean
add_item_to_recent (BognorQueue     *queue,
                    BognorQueueItem *item)
{
    BognorLocalQueue *local = (BognorLocalQueue *) queue;
    BognorLocalQueuePrivate *priv = local->priv;
    GtkRecentData data;
    gboolean ret;

    /* FIXME: This should just run through bognor... */
    data.display_name = NULL;
    data.description = NULL;
    data.mime_type = item->mimetype;
    data.app_name = "Hornsey";
    data.app_exec = "hornsey %u";
    data.groups = NULL;
    data.is_private = FALSE;

    ret = gtk_recent_manager_add_full (priv->recent_manager, item->uri, &data);
    if (ret == FALSE) {
        g_warning ("Error registering recent use of %s\n", item->uri);
    }

    return ret;
}

static gboolean
set_uri (BognorQueue     *queue,
         BognorQueueItem *item)
{
    BognorLocalQueue *local = (BognorLocalQueue *) queue;
    BognorLocalQueuePrivate *priv = local->priv;
    gboolean ret;
    gint pre_audio_state;

    pre_audio_state = priv->audio_state;

    ClientPlayerStop(priv->player);
    ClientEngineProcessXEvent(NULL);
    if (item == NULL) {
	return TRUE;
    }

    ClientPlayerOpenURL(priv->player,item->uri, NULL);
    ClientEngineProcessXEvent(NULL);

    if (pre_audio_state == kContentStatePlaying) {
        ClientPlayerPlay(priv->player);
        ClientEngineProcessXEvent(NULL);
    }
    return TRUE;
}

static gboolean
set_playing (BognorQueue *queue,
             gboolean     playing)
{
    BognorLocalQueue *local = (BognorLocalQueue *) queue;
    BognorLocalQueuePrivate *priv = local->priv;

    if (playing) {
	ClientPlayerPlay(priv->player);
	if (priv->tracker_id == 0) {
	    priv->tracker_id = g_timeout_add (HELIX_TIMEOUT, kick_helix, queue);
	}
    } else {
	if (priv->tracker_id > 0) {
	    g_source_remove (priv->tracker_id);
	    priv->tracker_id = 0;
	}
	ClientPlayerPause(priv->player);
    }
    ClientEngineProcessXEvent(NULL);

    return TRUE;
}

static gboolean
set_position (BognorQueue *queue,
              double       position)
{
    BognorLocalQueue *local = (BognorLocalQueue *) queue;
    BognorLocalQueuePrivate *priv = local->priv;
    gint32 new_pos = 0;
    gint32 curr_pos = 0;

    if (priv->length == 0)
       return FALSE;

    new_pos = position * priv->length;

    curr_pos = ClientPlayerGetPosition(priv->player);

    if (curr_pos - new_pos > 1000 || curr_pos - new_pos <-1000)
        ClientPlayerSetPosition (priv->player, new_pos);
    else {
        printf("set_position ignored due to few difference curr_pos: %d,\t new_pos: %d\n",curr_pos,new_pos);
    }

    ClientEngineProcessXEvent(NULL);

    return TRUE;
}

static gboolean
get_position (BognorQueue *queue,
              double      *position)
{
    BognorLocalQueue *local = (BognorLocalQueue *) queue;
    BognorLocalQueuePrivate *priv = local->priv;

    if (priv->length == 0) {
	*position = 0;
    } else {
	guint32 current = ClientPlayerGetPosition (priv->player);
	*position = (double)current/(double)priv->length;
    }

    return TRUE;
}

static void
bognor_local_queue_class_init (BognorLocalQueueClass *klass)
{
    GObjectClass *o_class = (GObjectClass *) klass;
    BognorQueueClass *q_class = (BognorQueueClass *) klass;

    o_class->dispose = bognor_local_queue_dispose;
    o_class->finalize = bognor_local_queue_finalize;
    o_class->set_property = bognor_local_queue_set_property;
    o_class->get_property = bognor_local_queue_get_property;

    q_class->set_uri = set_uri;
    q_class->set_playing = set_playing;
    q_class->set_position = set_position;
    q_class->get_position = get_position;
    q_class->add_item_to_recent = add_item_to_recent;

    g_type_class_add_private (klass, sizeof (BognorLocalQueuePrivate));
}

static void
bognor_local_queue_init (BognorLocalQueue *self)
{
    BognorLocalQueuePrivate *priv;

    static const HXClientCallbacks callbacks =
	{
	    NULL, // OnVisualStateChanged
	    NULL, // OnIdealSizeChanged
	    OnLengthChanged,
	    NULL, // OnTitleChanged
	    NULL, // OnGroupsChanged
	    NULL, // OnGroupStarted
	    NULL, // OnContacting
	    NULL, // OnBuffering
	    OnContentStateChanged,
	    OnContentConcluded,
	    NULL, // OnStatusChanged
	    NULL, // OnVolumeChanged
	    NULL, // OnMuteChanged
	    NULL, // OnClipBandwidthChanged
	    OnErrorOccurred,
	    NULL, // GoToURL
	    NULL, // RequestAuthentication
	    RequestUpgrade,
	    HasComponent
	};

    bognor_queue_set_name ((BognorQueue *) self, _("Playqueue"));

    priv = self->priv = GET_PRIVATE (self);

    if (!getenv("HELIX_LIBS"))
        setenv("HELIX_LIBS", "/opt/real/RealPlayer");

    ClientPlayerCreate(&priv->player, NULL, self, &callbacks);

    priv->recent_manager = gtk_recent_manager_get_default ();
}

BognorLocalQueue *
bognor_local_queue_new (void)
{
    BognorLocalQueue *q;

    q = g_object_new (BOGNOR_TYPE_LOCAL_QUEUE, NULL);

    return q;
}
