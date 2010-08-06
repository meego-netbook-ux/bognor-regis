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

#include <glib.h>
#include <glib/gi18n.h>
#include <gst/gst.h>
#include <gtk/gtk.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "bgr-tracker-client.h"
#include "bognor-queue-item.h"
#include "bognor-local-queue.h"
#include "bognor-player-bindings.h"

enum {
    PROP_0,
};

struct _BognorLocalQueuePrivate {
    BognorQueueItem *current_item;
    GstElement *playbin;
    GstState audio_state;
    gboolean audio_set;

    guint32 tracker_id;
    double position;

    GtkRecentManager *recent_manager;
    gboolean error_occured;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), BOGNOR_TYPE_LOCAL_QUEUE, BognorLocalQueuePrivate))
G_DEFINE_TYPE (BognorLocalQueue, bognor_local_queue, BOGNOR_TYPE_QUEUE);

static void make_playbin (BognorLocalQueue *queue);

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

static void
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
    data.mime_type = (char *) bognor_queue_item_get_mimetype (item);
    data.app_name = "Hornsey";
    data.app_exec = "hornsey %u";
    data.groups = NULL;
    data.is_private = FALSE;

    ret = gtk_recent_manager_add_full (priv->recent_manager,
                                       bognor_queue_item_get_uri (item),
                                       &data);
    if (ret == FALSE) {
        g_warning ("Error registering recent use of %s\n",
                   bognor_queue_item_get_uri (item));
    }
}

static void
set_item (BognorQueue     *queue,
          BognorQueueItem *item)
{
    BognorLocalQueue *local = (BognorLocalQueue *) queue;
    BognorLocalQueuePrivate *priv = local->priv;
    gboolean ret;
    GstState state;

    if (item == NULL) {
        priv->audio_set = FALSE;
        g_object_set (priv->playbin,
                      "uri", "",
                      NULL);

        if (priv->audio_state == GST_STATE_PLAYING) {
            gst_element_set_state (priv->playbin, GST_STATE_PAUSED);
            priv->audio_state = GST_STATE_PAUSED;
        }

        priv->current_item = NULL;
        return;
    }

    priv->current_item = item;

    if (priv->error_occured) {
        state = GST_STATE_PLAYING;
        priv->error_occured = FALSE;
    } else {
        gst_element_get_state (priv->playbin, &state, NULL,
                               GST_CLOCK_TIME_NONE);
    }

    gst_element_set_state (priv->playbin, GST_STATE_READY);
    /* Audio is played locally */
    g_object_set (priv->playbin,
                  "uri", bognor_queue_item_get_uri (item),
                  NULL);
    priv->audio_set = TRUE;
    gst_element_set_state (priv->playbin, state);

    return;
}

static void
set_playing (BognorQueue *queue,
             gboolean     playing)
{
    BognorLocalQueue *local = (BognorLocalQueue *) queue;
    BognorLocalQueuePrivate *priv = local->priv;

    if (priv->audio_set == FALSE) {
        return;
    }

    if (playing) {
        if (priv->audio_state != GST_STATE_PLAYING) {
            gst_element_set_state (priv->playbin, GST_STATE_PLAYING);
            priv->audio_state = GST_STATE_PLAYING;
        }
    } else {
        gst_element_set_state (priv->playbin, GST_STATE_PAUSED);
        priv->audio_state = GST_STATE_PAUSED;
    }

    return;
}

static void
set_mute (BognorQueue *queue,
          gboolean     mute)
{
if (mute)
    g_print("true set mute %d\n", mute);
else
    g_print("false set mute %d\n", mute);

    BognorLocalQueue *local = (BognorLocalQueue *) queue;
    BognorLocalQueuePrivate *priv = local->priv;
    g_object_set(G_OBJECT(priv->playbin), "mute", mute, NULL);
}

static gboolean
get_mute (BognorQueue *queue)
{
    BognorLocalQueue *local = (BognorLocalQueue *) queue;
    BognorLocalQueuePrivate *priv = local->priv;
    gboolean mute;
    g_object_get(G_OBJECT(priv->playbin), "mute", &mute, NULL);
    return mute;
}

static void
set_volume (BognorQueue *queue,
            double       volume)
{
    BognorLocalQueue *local = (BognorLocalQueue *) queue;
    BognorLocalQueuePrivate *priv = local->priv;
    g_object_set(G_OBJECT(priv->playbin), "volume", volume, NULL);
    g_print("set volume %f\n", volume);
}

static double
get_volume (BognorQueue *queue)
{
    BognorLocalQueue *local = (BognorLocalQueue *) queue;
    BognorLocalQueuePrivate *priv = local->priv;
    double v;
    g_object_get(G_OBJECT(priv->playbin), "volume", &v, NULL);
    g_print("get volume %f\n", v);
    return v;
}

static void
set_position (BognorQueue *queue,
              double       position)
{
    BognorLocalQueue *local = (BognorLocalQueue *) queue;
    BognorLocalQueuePrivate *priv = local->priv;
    GstFormat format = GST_FORMAT_TIME;
    gint64 dur;

    if (gst_element_query_duration (priv->playbin, &format, &dur)) {
        gst_element_seek_simple (priv->playbin, format,
                                 GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
                                 (gint64) (dur * position));
    } else {
        g_warning ("Query duration failed");
    }

    return;
}

static double
get_position (BognorQueue *queue)
{
    BognorLocalQueue *local = (BognorLocalQueue *) queue;
    BognorLocalQueuePrivate *priv = local->priv;

    return priv->position;
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

    q_class->set_item = set_item;
    q_class->set_playing = set_playing;
    q_class->set_position = set_position;
    q_class->get_position = get_position;
    q_class->set_volume = set_volume;
    q_class->get_volume = get_volume;
    q_class->set_mute = set_mute;
    q_class->get_mute = get_mute;
    q_class->add_item_to_recent = add_item_to_recent;

    g_type_class_add_private (klass, sizeof (BognorLocalQueuePrivate));
}

static gboolean
gst_get_position (gpointer userdata)
{
    BognorQueue *queue = (BognorQueue *) userdata;
    BognorLocalQueue *local = (BognorLocalQueue *) userdata;
    BognorLocalQueuePrivate *priv = local->priv;
    double position = 0.0;
    GstFormat format = GST_FORMAT_TIME;
    gint64 cur, dur = 0;

    if (gst_element_query_duration (priv->playbin, &format, &dur)) {
        if (gst_element_query_position (priv->playbin, &format, &cur)) {
            position = ((double) cur) / (double) dur;
        } else {
            g_warning ("Query position failed");
        }
    } else {
        g_print ("Query duration failed");
    }

    if (priv->current_item) {
        bognor_queue_item_set_duration (priv->current_item, dur / GST_SECOND);
    }

    priv->position = position;
    bognor_queue_emit_position_changed (queue, position);

    return TRUE;
}

static gboolean
bus_callback (GstBus     *bus,
              GstMessage *message,
              gpointer    data)
{
    BognorQueue *queue = (BognorQueue *) data;
    BognorLocalQueue *local = (BognorLocalQueue *) data;
    BognorLocalQueuePrivate *priv = local->priv;
    GstState oldstate, newstate;
    GstFormat format;
    int i;

    switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR: {
        GError *error;
        char *debug;
        int mode;

        gst_message_parse_error (message, &error, &debug);

        g_warning ("Error while playing: %s: %s", error->message, debug);

        if (error->domain == g_quark_from_string ("gst-stream-error-quark") &&
            error->code == 6 /* GST_STREAM_ERROR_CODEC_NOT_FOUND */) {
            if (priv->current_item) {
                bognor_queue_notify_unknown_format (queue, priv->current_item);
            }
        }

        g_error_free (error);
        g_free (debug);

        /* When there's been an error, tear down the playbin and
           rebuild */
        gst_element_set_state (priv->playbin, GST_STATE_NULL);
        g_object_unref (priv->playbin);

        make_playbin (local);
        priv->audio_state = GST_STATE_READY;
        // bognor_queue_get_index(queue, &i, NULL);
        bognor_queue_next (queue, NULL);
        // bognor_queue_remove_range(queue, i, 1, NULL); /* remove from queue */
        bognor_queue_get_current_index(queue, &i, NULL);
        bognor_queue_get_repeat_mode(queue, &mode, NULL);
        if (((mode & 0x08) != BOGNOR_QUEUE_MODE_REPEATING) && (i==0)) {
        } else {
            set_playing(queue, TRUE);
        }

        /* priv->error_occured = TRUE; */
        break;
    }

    case GST_MESSAGE_EOS:
        /* Once the local GStreamer queue is done we want the next audio */
        bognor_queue_next (queue, NULL);

        break;

    case GST_MESSAGE_STATE_CHANGED:
        gst_message_parse_state_changed (message, &oldstate, &newstate, NULL);
        if (newstate == GST_STATE_PLAYING) {
            if (priv->tracker_id == 0) {
                priv->tracker_id = g_timeout_add_seconds (1, gst_get_position,
                                                          queue);
            }
        } else {
            if (priv->tracker_id > 0) {
                g_source_remove (priv->tracker_id);
                priv->tracker_id = 0;
            }
        }
        break;

    default:
        break;
    }

    return TRUE;
}

static void
make_playbin (BognorLocalQueue *queue)
{
    BognorLocalQueuePrivate *priv = queue->priv;
    GstElement *fakesink;
    GstBus *bus;

    priv->playbin = gst_element_factory_make ("playbin2", "playbin");
    priv->error_occured = FALSE;
    priv->audio_set = FALSE;

    fakesink = gst_element_factory_make ("fakesink", "video_sink");
    g_object_set (priv->playbin,
                  "video-sink", fakesink,
                  NULL);

    bus = gst_pipeline_get_bus (GST_PIPELINE (priv->playbin));
    gst_bus_add_watch (bus, bus_callback, queue);
    gst_object_unref (bus);
}

static void
bognor_local_queue_init (BognorLocalQueue *self)
{
    BognorLocalQueuePrivate *priv;

    priv = self->priv = GET_PRIVATE (self);
    priv->recent_manager = gtk_recent_manager_get_default ();

    make_playbin (self);
}

BognorLocalQueue *
bognor_local_queue_new (BgrTrackerClient *tracker)
{
    BognorLocalQueue *q;

    q = g_object_new (BOGNOR_TYPE_LOCAL_QUEUE,
                      "name", _("Playqueue"),
                      "tracker", tracker,
                      NULL);

    return q;
}
