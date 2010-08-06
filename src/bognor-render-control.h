#ifndef __BOGNOR_RENDER_CONTROL_H__
#define __BOGNOR_RENDER_CONTROL_H__

#include <libgupnp/gupnp-service.h>

G_BEGIN_DECLS

#define BOGNOR_TYPE_RENDER_CONTROL                                      \
   (bognor_render_control_get_type())
#define BOGNOR_RENDER_CONTROL(obj)                                      \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                BOGNOR_TYPE_RENDER_CONTROL,             \
                                BognorRenderControl))
#define BOGNOR_RENDER_CONTROL_CLASS(klass)                              \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             BOGNOR_TYPE_RENDER_CONTROL,                \
                             BognorRenderControlClass))
#define IS_BOGNOR_RENDER_CONTROL(obj)                                   \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                BOGNOR_TYPE_RENDER_CONTROL))
#define IS_BOGNOR_RENDER_CONTROL_CLASS(klass)                           \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             BOGNOR_TYPE_RENDER_CONTROL))
#define BOGNOR_RENDER_CONTROL_GET_CLASS(obj)                            \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               BOGNOR_TYPE_RENDER_CONTROL,              \
                               BognorRenderControlClass))

typedef struct _BognorRenderControlPrivate BognorRenderControlPrivate;
typedef struct _BognorRenderControl      BognorRenderControl;
typedef struct _BognorRenderControlClass BognorRenderControlClass;

struct _BognorRenderControl
{
    GUPnPService parent;

    BognorRenderControlPrivate *priv;
};

struct _BognorRenderControlClass
{
    GUPnPServiceClass parent_class;
};

GType bognor_render_control_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __BOGNOR_RENDER_CONTROL_H__ */
