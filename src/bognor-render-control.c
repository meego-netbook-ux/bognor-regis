#include "bognor-render-control.h"

enum {
    PROP_0,
};

enum {
    LAST_SIGNAL,
};

struct _BognorRenderControlPrivate {
    int dummy;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), BOGNOR_TYPE_RENDER_CONTROL, BognorRenderControlPrivate))
G_DEFINE_TYPE (BognorRenderControl, bognor_render_control, GUPNP_TYPE_SERVICE);
static guint32 signals[LAST_SIGNAL] = {0,};

static void
bognor_render_control_finalize (GObject *object)
{
    BognorRenderControl *self = (BognorRenderControl *) object;

    G_OBJECT_CLASS (bognor_render_control_parent_class)->finalize (object);
}

static void
bognor_render_control_dispose (GObject *object)
{
    BognorRenderControl *self = (BognorRenderControl *) object;

    G_OBJECT_CLASS (bognor_render_control_parent_class)->dispose (object);
}

static void
bognor_render_control_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    BognorRenderControl *self = (BognorRenderControl *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bognor_render_control_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    BognorRenderControl *self = (BognorRenderControl *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bognor_render_control_class_init (BognorRenderControlClass *klass)
{
    GObjectClass *o_class = (GObjectClass *) klass;
    GUPnPServiceClass *s_class = (GUPnPServiceClass *) klass;

    o_class->dispose = bognor_render_control_dispose;
    o_class->finalize = bognor_render_control_finalize;
    o_class->set_property = bognor_render_control_set_property;
    o_class->get_property = bognor_render_control_get_property;

    g_type_class_add_private (klass, sizeof (BognorRenderControlPrivate));
}

static void
query_variable_cb (GUPnPService        *service,
                   char                *variable,
                   gpointer             value,
                   BognorRenderControl *control)
{
    g_print ("[Render Control] Query %s\n", variable);
}

static void
bognor_render_control_init (BognorRenderControl *self)
{
    BognorRenderControlPrivate *priv = GET_PRIVATE (self);

    self->priv = priv;

    g_signal_connect (self, "query-variable",
                      G_CALLBACK (query_variable_cb), self);
}

