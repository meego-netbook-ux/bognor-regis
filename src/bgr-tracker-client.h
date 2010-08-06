#ifndef __BGR_TRACKER_CLIENT_H__
#define __BGR_TRACKER_CLIENT_H__

#include <glib-object.h>

#include "bgr-item.h"

G_BEGIN_DECLS

#define BGR_TYPE_TRACKER_CLIENT                                         \
   (bgr_tracker_client_get_type())
#define BGR_TRACKER_CLIENT(obj)                                         \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                BGR_TYPE_TRACKER_CLIENT,                \
                                BgrTrackerClient))
#define BGR_TRACKER_CLIENT_CLASS(klass)                                 \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             BGR_TYPE_TRACKER_CLIENT,                   \
                             BgrTrackerClientClass))
#define IS_BGR_TRACKER_CLIENT(obj)                                      \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                BGR_TYPE_TRACKER_CLIENT))
#define IS_BGR_TRACKER_CLIENT_CLASS(klass)                              \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             BGR_TYPE_TRACKER_CLIENT))
#define BGR_TRACKER_CLIENT_GET_CLASS(obj)                               \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               BGR_TYPE_TRACKER_CLIENT,                 \
                               BgrTrackerClientClass))

typedef struct _BgrTrackerClientPrivate BgrTrackerClientPrivate;
typedef struct _BgrTrackerClient      BgrTrackerClient;
typedef struct _BgrTrackerClientClass BgrTrackerClientClass;

struct _BgrTrackerClient
{
    GObject parent;

    BgrTrackerClientPrivate *priv;
};

struct _BgrTrackerClientClass
{
    GObjectClass parent_class;
};

GType bgr_tracker_client_get_type (void) G_GNUC_CONST;

BgrItem *bgr_tracker_client_get_item (BgrTrackerClient *client,
                                      const char       *uri);

G_END_DECLS

#endif /* __BGR_TRACKER_CLIENT_H__ */
