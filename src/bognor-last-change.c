#include <glib.h>

#include <libgupnp/gupnp-service.h>
#include "bognor-last-change.h"

BognorLastChange *
bognor_last_change_new (GUPnPService *service)
{
    BognorLastChange *lc;

    lc = g_new0 (BognorLastChange, 1);
    lc->events = g_hash_table_new_full (g_str_hash, g_str_equal,
                                        g_free, g_free);
    lc->builder = g_string_new ("");
    lc->service = service;

    return lc;
}

void
bognor_last_change_free (BognorLastChange *lc)
{
    g_hash_table_destroy (lc->events);
    g_string_free (lc->builder, TRUE);

    if (lc->limiter > 0) {
        g_source_remove (lc->limiter);
    }

    g_free (lc);
}

static gboolean
emit_notify (gpointer data)
{
    BognorLastChange *lc = (BognorLastChange *) data;
    char *event;

    event = bognor_last_change_finish (lc);
    gupnp_service_notify (lc->service,
                          "LastChange", G_TYPE_STRING, event,
                          NULL);

    /* Reset */
    g_string_erase (lc->builder, 0, -1);
    g_hash_table_remove_all (lc->events);
    lc->limiter = 0;

    return FALSE;
}

static void
ensure_timeout (BognorLastChange *lc)
{
    if (lc->service == NULL || lc->limiter > 0) {
        return;
    }

    lc->limiter = g_timeout_add (200, emit_notify, lc);
}

void
bognor_last_change_add (BognorLastChange *lc,
                        const char       *variable,
                        const char       *value)
{
    char *log;

    log = g_strdup_printf ("<%s val=\"%s\"/>", variable, value);
    g_hash_table_replace (lc->events, g_strdup (variable), log);
    ensure_timeout (lc);
}

static void
append_event (gpointer key,
              gpointer value,
              gpointer data)
{
    BognorLastChange *lc = (BognorLastChange *) data;

    g_string_append (lc->builder, (char *) value);
}

const char *
bognor_last_change_finish (BognorLastChange *lc)
{
    g_string_append (lc->builder, "<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT_RCS\"><InstanceID val=\"0\">");
    g_hash_table_foreach (lc->events, append_event, lc);
    g_string_append (lc->builder, "</InstanceID></Event>");

    return lc->builder->str;
}
