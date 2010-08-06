#ifndef __BOGNOR_SOURCE_MANAGER_H__
#define __BOGNOR_SOURCE_MANAGER_H__

#include <glib-object.h>
#include <bickley/bkl-item.h>

G_BEGIN_DECLS

#define BOGNOR_TYPE_SOURCE_MANAGER                                      \
   (bognor_source_manager_get_type())
#define BOGNOR_SOURCE_MANAGER(obj)                                      \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                BOGNOR_TYPE_SOURCE_MANAGER,             \
                                BognorSourceManager))
#define BOGNOR_SOURCE_MANAGER_CLASS(klass)                              \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             BOGNOR_TYPE_SOURCE_MANAGER,                \
                             BognorSourceManagerClass))
#define IS_BOGNOR_SOURCE_MANAGER(obj)                                   \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                BOGNOR_TYPE_SOURCE_MANAGER))
#define IS_BOGNOR_SOURCE_MANAGER_CLASS(klass)                           \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             BOGNOR_TYPE_SOURCE_MANAGER))
#define BOGNOR_SOURCE_MANAGER_GET_CLASS(obj)                            \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               BOGNOR_TYPE_SOURCE_MANAGER,              \
                               BognorSourceManagerClass))

typedef struct _BognorSourceManagerPrivate BognorSourceManagerPrivate;
typedef struct _BognorSourceManager      BognorSourceManager;
typedef struct _BognorSourceManagerClass BognorSourceManagerClass;

struct _BognorSourceManager
{
    GObject parent;

    BognorSourceManagerPrivate *priv;
};

struct _BognorSourceManagerClass
{
    GObjectClass parent_class;
};

GType bognor_source_manager_get_type (void) G_GNUC_CONST;
BklItem *bognor_source_manager_find_item (BognorSourceManager *self,
                                          const char          *uri);

G_END_DECLS

#endif /* __BOGNOR_SOURCE_MANAGER_H__ */
