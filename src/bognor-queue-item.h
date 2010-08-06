#ifndef __BOGNOR_QUEUE_ITEM_H__
#define __BOGNOR_QUEUE_ITEM_H__

#include <glib-object.h>
#include "bgr-item.h"

G_BEGIN_DECLS

#define BOGNOR_TYPE_QUEUE_ITEM                                          \
   (bognor_queue_item_get_type())
#define BOGNOR_QUEUE_ITEM(obj)                                          \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                BOGNOR_TYPE_QUEUE_ITEM,                 \
                                BognorQueueItem))
#define BOGNOR_QUEUE_ITEM_CLASS(klass)                                  \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             BOGNOR_TYPE_QUEUE_ITEM,                    \
                             BognorQueueItemClass))
#define IS_BOGNOR_QUEUE_ITEM(obj)                                       \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                BOGNOR_TYPE_QUEUE_ITEM))
#define IS_BOGNOR_QUEUE_ITEM_CLASS(klass)                               \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             BOGNOR_TYPE_QUEUE_ITEM))
#define BOGNOR_QUEUE_ITEM_GET_CLASS(obj)                                \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               BOGNOR_TYPE_QUEUE_ITEM,                  \
                               BognorQueueItemClass))

typedef struct _BognorQueueItemPrivate BognorQueueItemPrivate;
typedef struct _BognorQueueItem      BognorQueueItem;
typedef struct _BognorQueueItemClass BognorQueueItemClass;

struct _BognorQueueItem
{
    GObject parent;

    BognorQueueItemPrivate *priv;
};

struct _BognorQueueItemClass
{
    GObjectClass parent_class;
};

GType bognor_queue_item_get_type (void); G_GNUC_CONST
BognorQueueItem *bognor_queue_item_new_from_item (BgrItem *bgr);
BognorQueueItem *bognor_queue_item_new (const char *uri,
                                        const char *mimetype,
                                        const char *metadata);
void bognor_queue_item_set_duration (BognorQueueItem *item,
                                     int              duration);
int bognor_queue_item_get_duration (BognorQueueItem *item);

void bognor_queue_item_set_position (BognorQueueItem *item,
                                     int              position);
int bognor_queue_item_get_position (BognorQueueItem *item);

const char *bognor_queue_item_get_uri (BognorQueueItem *item);
const char *bognor_queue_item_get_mimetype (BognorQueueItem *item);
const char *bognor_queue_item_get_metadata (BognorQueueItem *item);
BgrItem *bognor_queue_item_get_item (BognorQueueItem *item);

G_END_DECLS

#endif /* __BOGNOR_QUEUE_ITEM_H__ */
