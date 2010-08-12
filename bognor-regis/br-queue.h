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

#ifndef __BR_QUEUE_H__
#define __BR_QUEUE_H__

#include <glib-object.h>


G_BEGIN_DECLS

#define BR_TYPE_QUEUE                                                   \
   (br_queue_get_type())
#define BR_QUEUE(obj)                                                   \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                BR_TYPE_QUEUE,                          \
                                BrQueue))
#define BR_QUEUE_CLASS(klass)                                           \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             BR_TYPE_QUEUE,                             \
                             BrQueueClass))
#define IS_BR_QUEUE(obj)                                                \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                BR_TYPE_QUEUE))
#define IS_BR_QUEUE_CLASS(klass)                                        \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             BR_TYPE_QUEUE))
#define BR_QUEUE_GET_CLASS(obj)                                         \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               BR_TYPE_QUEUE,                           \
                               BrQueueClass))

#define BR_QUEUE_DBUS_INTERFACE "org.moblin.BognorRegis.Queue"
#define BR_QUEUE_INDEX_END -1

typedef enum _BrQueueState {
    BR_QUEUE_STATE_STOPPED,
    BR_QUEUE_STATE_PLAYING,
} BrQueueState;

typedef struct _BrQueuePrivate BrQueuePrivate;
typedef struct _BrQueue      BrQueue;
typedef struct _BrQueueClass BrQueueClass;

typedef void (*BrQueueListUrisCallback) (BrQueue       *queue,
                                         const char   **uris,
                                         const GError  *error,
                                         gpointer       userdata);
typedef void (*BrQueueGetIndexCallback) (BrQueue       *queue,
                                         int            index,
                                         const GError  *error,
                                         gpointer       userdata);
typedef void (*BrQueueGetMetadataCallback) (BrQueue      *queue,
                                            const char   *title,
                                            const char   *artist,
                                            const char   *album,
                                            const GError *error,
                                            gpointer      userdata);
typedef void (*BrQueueGetNameCallback) (BrQueue      *queue,
                                        const char   *name,
                                        const GError *error,
                                        gpointer      userdata);
typedef void (*BrQueueGetPositionCallback) (BrQueue      *queue,
                                            double        position,
                                            const GError *error,
                                            gpointer      userdata);
typedef void (*BrQueueGetMuteCallback) (BrQueue      *queue,
                                        gboolean      mute,
                                        const GError *error,
                                        gpointer      userdata);
typedef void (*BrQueueGetVolumeCallback) (BrQueue      *queue,
                                          double        volume,
                                          const GError *error,
                                          gpointer      userdata);
typedef void (*BrQueueGetStateCallback) (BrQueue      *queue,
                                         BrQueueState  state,
                                         const GError *error,
                                         gpointer      userdata);
typedef void (*BrQueueGetIndexUriCallback) (BrQueue      *queue,
                                            const char   *uri,
                                            const char   *mimetype,
                                            const GError *error,
                                            gpointer      userdata);
typedef void (*BrQueueGetDurationCallback) (BrQueue      *queue,
                                            int           duration_in_sec,
                                            const GError *error,
                                            gpointer      userdata);
typedef void (*BrQueueGetRepeatModeCallback) (BrQueue      *queue,
                                              const int     repeat_mode,
                                              const GError *error,
                                              gpointer      userdata);

struct _BrQueue
{
    GObject parent;

    BrQueuePrivate *priv;
};

struct _BrQueueClass
{
    GObjectClass parent_class;
};

GType br_queue_get_type (void) G_GNUC_CONST;

BrQueue *br_queue_new (const char *service_name,
                       const char *object_path);
BrQueue *br_queue_new_local (void);

void br_queue_play (BrQueue *queue);
void br_queue_stop (BrQueue *queue);
void br_queue_next (BrQueue *queue);
void br_queue_previous (BrQueue *queue);

void br_queue_set_position (BrQueue    *queue,
                            double      position);
void br_queue_get_position (BrQueue                   *queue,
                            BrQueueGetPositionCallback cb,
                            gpointer                   userdata);
void br_queue_set_mute (BrQueue    *queue,
                        gboolean    mute);
void br_queue_get_mute (BrQueue                   *queue,
                        BrQueueGetMuteCallback     cb,
                        gpointer                   userdata);
void br_queue_set_volume (BrQueue    *queue,
                          double      volume);
void br_queue_get_volume (BrQueue                   *queue,
                          BrQueueGetVolumeCallback   cb,
                          gpointer                   userdata);
void br_queue_get_state (BrQueue                *queue,
                         BrQueueGetStateCallback cb,
                         gpointer                userdata);
void br_queue_set_index (BrQueue *queue,
                         int      index);
void br_queue_get_index (BrQueue                 *queue,
                         BrQueueGetIndexCallback  cb,
                         gpointer                 userdata);
void br_queue_get_index_metadata (BrQueue                    *queue,
                                  int                         index,
                                  BrQueueGetMetadataCallback  cb,
                                  gpointer                    userdata);
void br_queue_get_next_metadata (BrQueue                    *queue,
                                 BrQueueGetMetadataCallback  cb,
                                 gpointer                    userdata);
void br_queue_get_duration (BrQueue                    *queue,
                            BrQueueGetDurationCallback  cb,
                            gpointer                    userdata);
void br_queue_get_repeat_mode (BrQueue                      *queue,
                               BrQueueGetRepeatModeCallback  cb,
                               gpointer                      userdata);
void br_queue_set_repeat_mode(BrQueue *queue,
                              int      mode);
void br_queue_get_index_uri (BrQueue                   *queue,
                             int                        index,
                             BrQueueGetIndexUriCallback cb,
                             gpointer                   userdata);

void br_queue_append_uris (BrQueue     *queue,
                           int          count,
                           const char **uri,
                           const char **mimetype);
void br_queue_insert_uris (BrQueue     *queue,
                           int          index,
                           int          count,
                           const char **uri,
                           const char **mimetype);
void br_queue_remove_range (BrQueue    *queue,
                            int         index,
                            int         count);

void br_queue_move_item (BrQueue *queue,
                         int      old_position,
                         int      new_position);

void br_queue_list_uris (BrQueue                *queue,
                         BrQueueListUrisCallback cb,
                         gpointer                userdata);
void br_queue_get_name (BrQueue               *queue,
                        BrQueueGetNameCallback cb,
                        gpointer               userdata);

G_END_DECLS

#endif /* __BR_QUEUE_H__ */
