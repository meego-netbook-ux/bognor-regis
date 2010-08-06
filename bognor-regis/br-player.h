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

#ifndef __BR_PLAYER_H__
#define __BR_PLAYER_H__

#include <glib-object.h>


G_BEGIN_DECLS

#define BR_TYPE_PLAYER                                                  \
   (br_player_get_type())
#define BR_PLAYER(obj)                                                  \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                BR_TYPE_PLAYER,                         \
                                BrPlayer))
#define BR_PLAYER_CLASS(klass)                                          \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             BR_TYPE_PLAYER,                            \
                             BrPlayerClass))
#define IS_BR_PLAYER(obj)                                               \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                BR_TYPE_PLAYER))
#define IS_BR_PLAYER_CLASS(klass)                                       \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             BR_TYPE_PLAYER))
#define BR_PLAYER_GET_CLASS(obj)                                        \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               BR_TYPE_PLAYER,                          \
                               BrPlayerClass))

typedef struct _BrPlayerPrivate BrPlayerPrivate;
typedef struct _BrPlayer      BrPlayer;
typedef struct _BrPlayerClass BrPlayerClass;

struct _BrPlayer
{
    GObject parent;

    BrPlayerPrivate *priv;
};

struct _BrPlayerClass
{
    GObjectClass parent_class;
};

GType br_player_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __BR_PLAYER_H__ */
