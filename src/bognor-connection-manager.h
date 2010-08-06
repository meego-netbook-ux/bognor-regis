#ifndef __BOGNOR_CONNECTION_MANAGER_H__
#define __BOGNOR_CONNECTION_MANAGER_H__

#include <libgupnp/gupnp-service.h>


G_BEGIN_DECLS

#define BOGNOR_TYPE_CONNECTION_MANAGER                                  \
   (bognor_connection_manager_get_type())
#define BOGNOR_CONNECTION_MANAGER(obj)                                  \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                BOGNOR_TYPE_CONNECTION_MANAGER,         \
                                BognorConnectionManager))
#define BOGNOR_CONNECTION_MANAGER_CLASS(klass)                          \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             BOGNOR_TYPE_CONNECTION_MANAGER,            \
                             BognorConnectionManagerClass))
#define IS_BOGNOR_CONNECTION_MANAGER(obj)                               \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                BOGNOR_TYPE_CONNECTION_MANAGER))
#define IS_BOGNOR_CONNECTION_MANAGER_CLASS(klass)                       \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             BOGNOR_TYPE_CONNECTION_MANAGER))
#define BOGNOR_CONNECTION_MANAGER_GET_CLASS(obj)                        \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               BOGNOR_TYPE_CONNECTION_MANAGER,          \
                               BognorConnectionManagerClass))

typedef struct _BognorConnectionManagerPrivate BognorConnectionManagerPrivate;
typedef struct _BognorConnectionManager      BognorConnectionManager;
typedef struct _BognorConnectionManagerClass BognorConnectionManagerClass;

struct _BognorConnectionManager
{
    GUPnPService parent;

    BognorConnectionManagerPrivate *priv;
};

struct _BognorConnectionManagerClass
{
    GUPnPServiceClass parent_class;
};

GType bognor_connection_manager_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __BOGNOR_CONNECTION_MANAGER_H__ */
