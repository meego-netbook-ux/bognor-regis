#ifndef __BOGNOR_LAST_CHANGE_H__
#define __BOGNOR_LAST_CHANGE_H__

#include <glib.h>
#include <libgupnp/gupnp-service.h>

typedef struct _BognorLastChange {
    GUPnPService *service;
    GHashTable *events;
    GString *builder;

    guint32 limiter;
} BognorLastChange;

BognorLastChange *bognor_last_change_new (GUPnPService *service);
void bognor_last_change_free (BognorLastChange *lc);
void bognor_last_change_add (BognorLastChange *lc,
                             const char       *variable,
                             const char       *value);
const char *bognor_last_change_finish (BognorLastChange *lc);

#endif



