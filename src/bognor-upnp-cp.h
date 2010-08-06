#ifndef __BOGNOR_UPNP_CP_H__
#define __BOGNOR_UPNP_CP_H__

#include <glib-object.h>


G_BEGIN_DECLS

#define BOGNOR_TYPE_UPNP_CP                                             \
   (bognor_upnp_cp_get_type())
#define BOGNOR_UPNP_CP(obj)                                             \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                BOGNOR_TYPE_UPNP_CP,                    \
                                BognorUpnpCp))
#define BOGNOR_UPNP_CP_CLASS(klass)                                     \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             BOGNOR_TYPE_UPNP_CP,                       \
                             BognorUpnpCpClass))
#define IS_BOGNOR_UPNP_CP(obj)                                          \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                BOGNOR_TYPE_UPNP_CP))
#define IS_BOGNOR_UPNP_CP_CLASS(klass)                                  \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             BOGNOR_TYPE_UPNP_CP))
#define BOGNOR_UPNP_CP_GET_CLASS(obj)                                   \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               BOGNOR_TYPE_UPNP_CP,                     \
                               BognorUpnpCpClass))

typedef struct _BognorUpnpCpPrivate BognorUpnpCpPrivate;
typedef struct _BognorUpnpCp      BognorUpnpCp;
typedef struct _BognorUpnpCpClass BognorUpnpCpClass;

struct _BognorUpnpCp
{
    GObject parent;

    BognorUpnpCpPrivate *priv;
};

struct _BognorUpnpCpClass
{
    GObjectClass parent_class;
};

GType bognor_upnp_cp_get_type (void) G_GNUC_CONST;
BognorUpnpCp *bognor_upnp_cp_new (BognorQueue *queue);

G_END_DECLS

#endif /* __BOGNOR_UPNP_CP_H__ */
