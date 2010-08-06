#ifndef __BOGNOR_AV_TRANSPORT_H__
#define __BOGNOR_AV_TRANSPORT_H__

#include <libgupnp/gupnp-service.h>

#include "bognor-queue.h"

G_BEGIN_DECLS

#define BOGNOR_TYPE_AV_TRANSPORT                                        \
   (bognor_av_transport_get_type())
#define BOGNOR_AV_TRANSPORT(obj)                                        \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                BOGNOR_TYPE_AV_TRANSPORT,               \
                                BognorAvTransport))
#define BOGNOR_AV_TRANSPORT_CLASS(klass)                                \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             BOGNOR_TYPE_AV_TRANSPORT,                  \
                             BognorAvTransportClass))
#define IS_BOGNOR_AV_TRANSPORT(obj)                                     \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                BOGNOR_TYPE_AV_TRANSPORT))
#define IS_BOGNOR_AV_TRANSPORT_CLASS(klass)                             \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             BOGNOR_TYPE_AV_TRANSPORT))
#define BOGNOR_AV_TRANSPORT_GET_CLASS(obj)                              \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               BOGNOR_TYPE_AV_TRANSPORT,                \
                               BognorAvTransportClass))

typedef struct _BognorAvTransportPrivate BognorAvTransportPrivate;
typedef struct _BognorAvTransport      BognorAvTransport;
typedef struct _BognorAvTransportClass BognorAvTransportClass;

struct _BognorAvTransport
{
    GUPnPService parent;

    BognorAvTransportPrivate *priv;
};

struct _BognorAvTransportClass
{
    GUPnPServiceClass parent_class;
};

GType bognor_av_transport_get_type (void) G_GNUC_CONST;
void bognor_av_transport_set_queue (BognorAvTransport *transport,
                                    BognorQueue       *queue);

G_END_DECLS

#endif /* __BOGNOR_AV_TRANSPORT_H__ */
