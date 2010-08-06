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

#ifndef __BR_IFACE_PLAYER_H__
#define __BR_IFACE_PLAYER_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define BR_TYPE_IFACE_PLAYER (br_iface_player_get_type ())
#define BR_IFACE_PLAYER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), BR_TYPE_IFACE_PLAYER, BrIfacePlayer))
#define BR_IFACE_PLAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), BR_TYPE_IFACE_PLAYER, BrIfacePlayerClass))
#define BR_IFACE_PLAYER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), BR_TYPE_IFACE_PLAYER, BrIfacePlayerClass))

typedef struct _BrIfacePlayer BrIfacePlayer;
typedef struct _BrIfacePlayerClass BrIfacePlayerClass;

struct _BrIfacePlayerClass {
    GTypeInterface base_iface;

    gboolean (* play) (BrIfacePlayer *player,
                       GError       **error);
    gboolean (* stop) (BrIfacePlayer *player,
                       GError       **error);

    gboolean (* set_position) (BrIfacePlayer *player,
                               double         position,
                               GError       **error);
    gboolean (* show_uri) (BrIfacePlayer *player,
                           const char    *uri,
                           const char    *mimetype,
                           GError       **error);
    gboolean (* can_show_visual) (BrIfacePlayer *player,
                                  gboolean      *can_show_visual,
                                  GError       **error);
    gboolean (* force_visual_mode) (BrIfacePlayer *player,
                                    GError       **error);

    /* signals */
    void (*can_show_visual_changed) (BrIfacePlayer *player,
                                     gboolean       can_show);
    void (*uri_completed) (BrIfacePlayer *player,
                           const char    *uri);
    void (*position_changed) (BrIfacePlayer *player,
                              double         position);
};

GType br_iface_player_get_type (void);

void br_iface_player_emit_uri_completed (BrIfacePlayer *player,
                                         const char    *uri);
void br_iface_player_emit_can_show_visual_changed (BrIfacePlayer *player,
                                                   gboolean       can_show);
void br_iface_player_emit_position_changed (BrIfacePlayer *player,
                                            double         position);



G_END_DECLS

#endif
