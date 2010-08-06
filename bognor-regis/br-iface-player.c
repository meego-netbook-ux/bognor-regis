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

#include <glib.h>

#include <dbus/dbus-glib.h>
#include "br-iface-player.h"

enum {
    COMPLETED,
    CHANGED,
    POSITION_CHANGED,
    LAST_SIGNAL
};

static guint32 signals[LAST_SIGNAL] = {0, };

static gboolean br_iface_player_play (BrIfacePlayer *player,
                                      GError       **error);
static gboolean br_iface_player_stop (BrIfacePlayer *player,
                                      GError       **error);
static gboolean br_iface_player_set_position (BrIfacePlayer *player,
                                              double         position,
                                              GError       **error);
static gboolean br_iface_player_show_uri (BrIfacePlayer *player,
                                          const char    *uri,
                                          const char    *mimetype,
                                          GError       **error);
static gboolean br_iface_player_can_show_visual (BrIfacePlayer *player,
                                                 gboolean      *can_show_visual,
                                                 GError       **error);
static gboolean br_iface_player_force_visual_mode (BrIfacePlayer *player,
                                                   GError       **error);
#include "br-iface-player-glue.h"

static void
br_iface_player_base_init (gpointer klass)
{
    static gboolean initialized = FALSE;

    if (initialized) {
        return;
    }
    initialized = TRUE;

    signals[COMPLETED] = g_signal_new ("uri-completed",
                                       G_OBJECT_CLASS_TYPE (klass),
                                       G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                                       g_cclosure_marshal_VOID__STRING,
                                       G_TYPE_NONE, 1, G_TYPE_STRING);
    signals[CHANGED] = g_signal_new ("can-show-visual-changed",
                                     G_OBJECT_CLASS_TYPE (klass),
                                     G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                                     g_cclosure_marshal_VOID__BOOLEAN,
                                     G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
    signals[POSITION_CHANGED] = g_signal_new ("position-changed",
                                              G_OBJECT_CLASS_TYPE (klass),
                                              G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                                              g_cclosure_marshal_VOID__DOUBLE,
                                              G_TYPE_NONE, 1, G_TYPE_DOUBLE);
    dbus_g_object_type_install_info (br_iface_player_get_type (),
                                     &dbus_glib_br_iface_player_object_info);
}

GType
br_iface_player_get_type (void)
{
    static GType type = 0;

    if (!type) {
        const GTypeInfo info = {
            sizeof (BrIfacePlayerClass),
            br_iface_player_base_init,
            NULL,
        };

        type = g_type_register_static (G_TYPE_INTERFACE,
                                       "BrIfacePlayer", &info, 0);
    }

    return type;
}

static gboolean
br_iface_player_play (BrIfacePlayer *player,
                      GError       **error)
{
    return BR_IFACE_PLAYER_GET_CLASS (player)->play (player, error);
}

static gboolean
br_iface_player_stop (BrIfacePlayer *player,
                      GError       **error)
{
    return BR_IFACE_PLAYER_GET_CLASS (player)->stop (player, error);
}

static gboolean
br_iface_player_set_position (BrIfacePlayer *player,
                              double         position,
                              GError       **error)
{
    return BR_IFACE_PLAYER_GET_CLASS (player)->set_position (player,
                                                             position,
                                                             error);
}

static gboolean
br_iface_player_show_uri (BrIfacePlayer *player,
                          const char    *uri,
                          const char    *mimetype,
                          GError       **error)
{
    return BR_IFACE_PLAYER_GET_CLASS (player)->show_uri (player, uri,
                                                         mimetype, error);
}

static gboolean
br_iface_player_can_show_visual (BrIfacePlayer *player,
                                 gboolean      *can_show_visual,
                                 GError       **error)
{
    return BR_IFACE_PLAYER_GET_CLASS (player)->can_show_visual (player,
                                                                can_show_visual,
                                                                error);
}

static gboolean
br_iface_player_force_visual_mode (BrIfacePlayer *player,
                                   GError       **error)
{
    return BR_IFACE_PLAYER_GET_CLASS (player)->force_visual_mode (player,
                                                                  error);
}

void
br_iface_player_emit_uri_completed (BrIfacePlayer *player,
                                    const char    *uri)
{
    g_signal_emit (player, signals[COMPLETED], 0, uri);
}

void
br_iface_player_emit_can_show_visual_changed (BrIfacePlayer *player,
                                              gboolean       can_show)
{
    g_signal_emit (player, signals[CHANGED], 0, can_show);
}

void
br_iface_player_emit_position_changed (BrIfacePlayer *player,
                                       double         position)
{
    g_signal_emit (player, signals[POSITION_CHANGED], 0, position);
}
