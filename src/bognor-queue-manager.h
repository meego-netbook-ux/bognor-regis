/*
 * Bognor-Regis - a media player/queue daemon.
 * Copyright © 2009, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __BOGNOR_QUEUE_MANAGER_H__
#define __BOGNOR_QUEUE_MANAGER_H__

#include <glib-object.h>
#include <dbus/dbus-glib.h>

#include "bgr-tracker-client.h"
#include "bognor-queue.h"

G_BEGIN_DECLS

#define BOGNOR_TYPE_QUEUE_MANAGER                                       \
   (bognor_queue_manager_get_type())
#define BOGNOR_QUEUE_MANAGER(obj)                                       \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                BOGNOR_TYPE_QUEUE_MANAGER,              \
                                BognorQueueManager))
#define BOGNOR_QUEUE_MANAGER_CLASS(klass)                               \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             BOGNOR_TYPE_QUEUE_MANAGER,                 \
                             BognorQueueManagerClass))
#define IS_BOGNOR_QUEUE_MANAGER(obj)                                    \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                BOGNOR_TYPE_QUEUE_MANAGER))
#define IS_BOGNOR_QUEUE_MANAGER_CLASS(klass)                            \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             BOGNOR_TYPE_QUEUE_MANAGER))
#define BOGNOR_QUEUE_MANAGER_GET_CLASS(obj)                             \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               BOGNOR_TYPE_QUEUE_MANAGER,               \
                               BognorQueueManagerClass))

typedef enum _BognorQueueManagerAction {
    BOGNOR_QUEUE_MANAGER_ACTION_PLAY,
    BOGNOR_QUEUE_MANAGER_ACTION_PAUSE,
    BOGNOR_QUEUE_MANAGER_ACTION_NEXT,
    BOGNOR_QUEUE_MANAGER_ACTION_PREVIOUS
} BognorQueueManagerAction;

typedef struct _BognorQueueManagerPrivate BognorQueueManagerPrivate;
typedef struct _BognorQueueManager      BognorQueueManager;
typedef struct _BognorQueueManagerClass BognorQueueManagerClass;

struct _BognorQueueManager
{
    GObject parent;

    BognorQueueManagerPrivate *priv;
};

struct _BognorQueueManagerClass
{
    GObjectClass parent_class;
};

GType bognor_queue_manager_get_type (void) G_GNUC_CONST;
BognorQueueManager *bognor_queue_manager_new (DBusGConnection  *connection,
                                              BgrTrackerClient *tracker);
void bognor_queue_manager_action (BognorQueueManager      *manager,
                                  BognorQueueManagerAction action);
BognorQueue *bognor_queue_manager_get_local_queue (BognorQueueManager *manager);

G_END_DECLS

#endif /* __BOGNOR_QUEUE_MANAGER_H__ */
