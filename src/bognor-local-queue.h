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

#ifndef __BOGNOR_LOCAL_QUEUE_H__
#define __BOGNOR_LOCAL_QUEUE_H__

#include "bognor-queue.h"
#include "bgr-tracker-client.h"

G_BEGIN_DECLS

#define BOGNOR_TYPE_LOCAL_QUEUE                                         \
   (bognor_local_queue_get_type())
#define BOGNOR_LOCAL_QUEUE(obj)                                         \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                BOGNOR_TYPE_LOCAL_QUEUE,                \
                                BognorLocalQueue))
#define BOGNOR_LOCAL_QUEUE_CLASS(klass)                                 \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             BOGNOR_TYPE_LOCAL_QUEUE,                   \
                             BognorLocalQueueClass))
#define IS_BOGNOR_LOCAL_QUEUE(obj)                                      \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                BOGNOR_TYPE_LOCAL_QUEUE))
#define IS_BOGNOR_LOCAL_QUEUE_CLASS(klass)                              \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             BOGNOR_TYPE_LOCAL_QUEUE))
#define BOGNOR_LOCAL_QUEUE_GET_CLASS(obj)                               \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               BOGNOR_TYPE_LOCAL_QUEUE,                 \
                               BognorLocalQueueClass))

typedef struct _BognorLocalQueuePrivate BognorLocalQueuePrivate;
typedef struct _BognorLocalQueue      BognorLocalQueue;
typedef struct _BognorLocalQueueClass BognorLocalQueueClass;

struct _BognorLocalQueue
{
    BognorQueue parent;

    BognorLocalQueuePrivate *priv;
};

struct _BognorLocalQueueClass
{
    BognorQueueClass parent_class;
};

GType bognor_local_queue_get_type (void) G_GNUC_CONST;
BognorLocalQueue *bognor_local_queue_new (BgrTrackerClient *tracker);

G_END_DECLS

#endif /* __BOGNOR_LOCAL_QUEUE_H__ */
