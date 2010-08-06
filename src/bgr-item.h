#ifndef __BGR_ITEM_H__
#define __BGR_ITEM_H__

#include <glib-object.h>


G_BEGIN_DECLS

#define BGR_TYPE_ITEM                                                   \
   (bgr_item_get_type())
#define BGR_ITEM(obj)                                                   \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                BGR_TYPE_ITEM,                          \
                                BgrItem))
#define BGR_ITEM_CLASS(klass)                                           \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             BGR_TYPE_ITEM,                             \
                             BgrItemClass))
#define IS_BGR_ITEM(obj)                                                \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                BGR_TYPE_ITEM))
#define IS_BGR_ITEM_CLASS(klass)                                        \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             BGR_TYPE_ITEM))
#define BGR_ITEM_GET_CLASS(obj)                                         \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               BGR_TYPE_ITEM,                           \
                               BgrItemClass))

#define BGR_ITEM_METADATA_ARTIST "artist"
#define BGR_ITEM_METADATA_ALBUM "album"
#define BGR_ITEM_METADATA_TITLE "title"
#define BGR_ITEM_METADATA_USE_COUNT "use-count"

typedef struct _BgrItemPrivate BgrItemPrivate;
typedef struct _BgrItem      BgrItem;
typedef struct _BgrItemClass BgrItemClass;

struct _BgrItem
{
    GObject parent;

    BgrItemPrivate *priv;
};

struct _BgrItemClass
{
    GObjectClass parent_class;
};

GType bgr_item_get_type (void) G_GNUC_CONST;

const char *bgr_item_get_uri (BgrItem *item);
const char *bgr_item_get_mimetype (BgrItem *item);

void bgr_item_set_metadata (BgrItem    *item,
                            const char *key,
                            const char *value);
const char *bgr_item_get_metadata (BgrItem    *item,
                                   const char *key);

G_END_DECLS

#endif /* __BGR_ITEM_H__ */
