#ifndef __BOGNOR_QUEUE_H__
#define __BOGNOR_QUEUE_H__

#include <glib-object.h>

#include "bognor-queue-item.h"

G_BEGIN_DECLS

#define BOGNOR_TYPE_QUEUE                                               \
   (bognor_queue_get_type())
#define BOGNOR_QUEUE(obj)                                               \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                BOGNOR_TYPE_QUEUE,                      \
                                BognorQueue))
#define BOGNOR_QUEUE_CLASS(klass)                                       \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             BOGNOR_TYPE_QUEUE,                         \
                             BognorQueueClass))
#define IS_BOGNOR_QUEUE(obj)                                            \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                BOGNOR_TYPE_QUEUE))
#define IS_BOGNOR_QUEUE_CLASS(klass)                                    \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             BOGNOR_TYPE_QUEUE))
#define BOGNOR_QUEUE_GET_CLASS(obj)                                     \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               BOGNOR_TYPE_QUEUE,                       \
                               BognorQueueClass))

#define BOGNOR_QUEUE_ERROR bognor_queue_error_quark ()
#define BOGNOR_QUEUE_INDEX_END -1

typedef enum {
    BOGNOR_QUEUE_ERROR_EMPTY,
    BOGNOR_QUEUE_ERROR_START,
    BOGNOR_QUEUE_ERROR_END,
    BOGNOR_QUEUE_ERROR_OUT_OF_RANGE,
} BognorQueueError;

typedef enum _BognorQueueState {
    BOGNOR_QUEUE_STATE_STOPPED,
    BOGNOR_QUEUE_STATE_PLAYING,
    BOGNOR_QUEUE_LAST_STATE
} BognorQueueState;

typedef enum _BognorQueueStatus {
    BOGNOR_QUEUE_STATUS_EMPTY,
    BOGNOR_QUEUE_STATUS_OK,
    BOGNOR_QUEUE_LAST_STATUS
} BognorQueueStatus;

typedef enum _BognorQueueMode {
    BOGNOR_QUEUE_MODE_NORMAL,
    BOGNOR_QUEUE_MODE_REPEATING,
    BOGNOR_QUEUE_MODE_SINGLE_REPEATING,
    BOGNOR_QUEUE_MODE_SHUFFLE = 1 << 7,
    BOGNOR_QUEUE_LAST_MODE
} BognorQueueMode;

typedef struct _BognorQueuePrivate BognorQueuePrivate;
typedef struct _BognorQueue      BognorQueue;
typedef struct _BognorQueueClass BognorQueueClass;

struct _BognorQueue
{
    GObject parent;

    BognorQueuePrivate *priv;
};

struct _BognorQueueClass
{
    GObjectClass parent_class;

    void (*set_item) (BognorQueue     *queue,
                      BognorQueueItem *item);
    void (*set_playing) (BognorQueue *queue,
                         gboolean     playing);
    void (*set_position) (BognorQueue *queue,
                          double       position);
    double (*get_position) (BognorQueue *queue);
    void (*set_mute) (BognorQueue *queue,
                      gboolean     mute);
    gboolean (*get_mute) (BognorQueue *queue);
    void (*set_volume) (BognorQueue *queue,
                        double       volume);
    double (*get_volume) (BognorQueue *queue);
    void (*add_item_to_recent) (BognorQueue     *queue,
                                BognorQueueItem *item);
};

GType bognor_queue_get_type (void) G_GNUC_CONST;

void bognor_queue_notify_unknown_format (BognorQueue     *queue,
                                         BognorQueueItem *item);
void bognor_queue_emit_position_changed (BognorQueue *queue,
                                         double       position);
gboolean bognor_queue_stop (BognorQueue      *queue,
                            GError          **error);
gboolean bognor_queue_play (BognorQueue      *queue,
                            GError          **error);
gboolean bognor_queue_next (BognorQueue     *queue,
                            GError          **error);
gboolean bognor_queue_previous (BognorQueue  *queue,
                                GError      **error);
gboolean bognor_queue_set_position (BognorQueue   *queue,
                                    double         position,
                                    GError       **error);
gboolean bognor_queue_get_position (BognorQueue   *queue,
                                    double        *position,
                                    GError       **error);
gboolean bognor_queue_set_mute (BognorQueue   *queue,
                                gboolean       mute,
                                GError       **error);
gboolean bognor_queue_get_mute (BognorQueue   *queue,
                                gboolean      *mute,
                                GError       **error);
gboolean bognor_queue_set_volume (BognorQueue   *queue,
                                  double         volume,
                                  GError       **error);
gboolean bognor_queue_get_volume (BognorQueue   *queue,
                                  double        *volume,
                                  GError       **error);
gboolean bognor_queue_append_uris (BognorQueue   *queue,
                                   int            count,
                                   const char   **uris,
                                   const char   **mimetypes,
                                   GError       **error);
gboolean bognor_queue_insert_uris (BognorQueue   *queue,
                                   int            position,
                                   int            count,
                                   const char   **uris,
                                   const char   **mimetype,
                                   GError       **error);
gboolean bognor_queue_remove_range (BognorQueue   *queue,
                                    int            index,
                                    int            count,
                                    GError       **error);
gboolean bognor_queue_move_item (BognorQueue   *queue,
                                 int            old_position,
                                 int            new_position,
                                 GError       **error);

gboolean bognor_queue_set_index (BognorQueue   *queue,
                                 int            index,
                                 GError       **error);
gboolean bognor_queue_get_current_index (BognorQueue   *queue,
                                         int           *index,
                                         GError       **error);
gboolean bognor_queue_get_index (BognorQueue   *queue,
                                 int           *index,
                                 GError       **error);
gboolean bognor_queue_get_index_metadata (BognorQueue   *queue,
                                          int            index,
                                          char         **title,
                                          char         **artist,
                                          char         **album,
                                          GError       **error);
gboolean bognor_queue_get_next_metadata (BognorQueue   *queue,
                                         char         **title,
                                         char         **artist,
                                         char         **album,
                                         GError       **error);
gboolean bognor_queue_set_repeat_mode (BognorQueue   *queue,
                                       int            mode,
                                       GError       **error);
gboolean bognor_queue_get_repeat_mode (BognorQueue   *queue,
                                       int           *mode,
                                       GError       **error);
gboolean bognor_queue_get_duration (BognorQueue      *queue,
                                    int              *duration,
                                    GError          **error);
gboolean bognor_queue_get_index_uri (BognorQueue    *queue,
                                     int             index,
                                     char          **uri,
                                     char          **mimetype,
                                     GError        **error);
gboolean bognor_queue_list_uris (BognorQueue        *queue,
                                 char             ***uris,
                                 GError            **error);
gboolean bognor_queue_get_state (BognorQueue        *queue,
                                 int                *state,
                                 GError            **error);
gboolean bognor_queue_get_name (BognorQueue          *queue,
                                char                **name,
                                GError              **error);
int bognor_queue_get_count (BognorQueue *queue);
int bognor_queue_next_index (BognorQueue *queue);

G_END_DECLS

#endif /* __BOGNOR_QUEUE_H__ */
