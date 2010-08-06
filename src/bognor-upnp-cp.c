#include <libgupnp/gupnp-device-info.h>
#include <libgupnp/gupnp-service-info.h>
#include <libgupnp/gupnp-root-device.h>
#include <libgupnp/gupnp-resource-factory.h>
#include <libgupnp/gupnp-context.h>

#include "bognor-queue.h"
#include "bognor-upnp-cp.h"
#include "bognor-av-transport.h"
#include "bognor-render-control.h"
#include "bognor-connection-manager.h"

enum {
    PROP_0,
};

enum {
    LAST_SIGNAL,
};

struct _BognorUpnpCpPrivate {
    GUPnPContext *ctxt;
    GUPnPRootDevice *root;
    GUPnPResourceFactory *factory;

    GUPnPServiceInfo *render_control;
    GUPnPServiceInfo *connection;
    GUPnPServiceInfo *av_transport;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), BOGNOR_TYPE_UPNP_CP, BognorUpnpCpPrivate))
G_DEFINE_TYPE (BognorUpnpCp, bognor_upnp_cp, G_TYPE_OBJECT);
static guint32 signals[LAST_SIGNAL] = {0,};

static void
bognor_upnp_cp_finalize (GObject *object)
{
    BognorUpnpCp *self = (BognorUpnpCp *) object;

    G_OBJECT_CLASS (bognor_upnp_cp_parent_class)->finalize (object);
}

static void
bognor_upnp_cp_dispose (GObject *object)
{
    BognorUpnpCp *self = (BognorUpnpCp *) object;

    G_OBJECT_CLASS (bognor_upnp_cp_parent_class)->dispose (object);
}

static void
bognor_upnp_cp_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    BognorUpnpCp *self = (BognorUpnpCp *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bognor_upnp_cp_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    BognorUpnpCp *self = (BognorUpnpCp *) object;

    switch (prop_id) {

    default:
        break;
    }
}

static void
bognor_upnp_cp_class_init (BognorUpnpCpClass *klass)
{
    GObjectClass *o_class = (GObjectClass *) klass;

    o_class->dispose = bognor_upnp_cp_dispose;
    o_class->finalize = bognor_upnp_cp_finalize;
    o_class->set_property = bognor_upnp_cp_set_property;
    o_class->get_property = bognor_upnp_cp_get_property;

    g_type_class_add_private (klass, sizeof (BognorUpnpCpPrivate));
}

static GUPnPXMLDoc *
make_device_description (GUPnPContext *context,
                         const char   *base_doc)
{
    GUPnPXMLDoc *doc;
    GError *error = NULL;
    xmlDoc *xmld;
    xmlNode *root;
    char *baseurl;

    doc = gupnp_xml_doc_new_from_path (base_doc, &error);
    if (error != NULL) {
        g_warning ("Error loading %s: %s", base_doc, error->message);
        g_error_free (error);
        return NULL;
    }

    /* Set the URLBase */
    xmld = doc->doc;
    if (xmld->children) {
        root = xmld->children;
    } else {
        g_warning ("%s has no root node", base_doc);
        return;
    }

    baseurl = g_strdup_printf ("http://%s:%u",
                               gupnp_context_get_host_ip (context),
                               gupnp_context_get_port (context));
    xmlNewTextChild (root, NULL, "URLBase", baseurl);
    g_free (baseurl);

    return doc;
}

static void
bognor_upnp_cp_init (BognorUpnpCp *self)
{
    BognorUpnpCpPrivate *priv = GET_PRIVATE (self);
    GUPnPXMLDoc *description;
    GError *error = NULL;

    self->priv = priv;
    priv->ctxt = gupnp_context_new (NULL, NULL, 0, &error);
    if (error) {
        g_warning ("Error creating the GUPnP context: %s", error->message);
        g_error_free (error);
        return;
    }

    priv->factory = gupnp_resource_factory_get_default ();
    gupnp_resource_factory_register_resource_type
        (priv->factory, "urn:schemas-upnp-org:service:AVTransport:1",
         BOGNOR_TYPE_AV_TRANSPORT);
    gupnp_resource_factory_register_resource_type
        (priv->factory, "urn:schemas-upnp-org:service:ConnectionManager:1",
         BOGNOR_TYPE_CONNECTION_MANAGER);
    gupnp_resource_factory_register_resource_type
        (priv->factory, "urn:schemas-upnp-org:service:RenderingControl:1",
         BOGNOR_TYPE_RENDER_CONTROL);

    description = make_device_description (priv->ctxt,
                                           SCPDDIR "/MediaRenderer2.xml");
    if (description == NULL) {
        return;
    }
    priv->root = gupnp_root_device_new_full (priv->ctxt, priv->factory,
                                             description, "",
                                             SCPDDIR);
    gupnp_root_device_set_available (priv->root, TRUE);

    priv->av_transport = gupnp_device_info_get_service
        (GUPNP_DEVICE_INFO (priv->root),
         "urn:schemas-upnp-org:service:AVTransport");
    priv->connection = gupnp_device_info_get_service
        (GUPNP_DEVICE_INFO (priv->root),
         "urn:schemas-upnp-org:service:ConnectionManager");
    priv->render_control = gupnp_device_info_get_service
        (GUPNP_DEVICE_INFO (priv->root),
         "urn:schemas-upnp-org:service:RenderingControl");
}

BognorUpnpCp *
bognor_upnp_cp_new (BognorQueue *queue)
{
    BognorUpnpCp *cp = g_object_new (BOGNOR_TYPE_UPNP_CP, NULL);
    BognorUpnpCpPrivate *priv;

    priv = cp->priv;

    if (priv->av_transport == NULL) {
        g_object_unref (cp);
        return NULL;
    }

    bognor_av_transport_set_queue (priv->av_transport, queue);

    return cp;
}
