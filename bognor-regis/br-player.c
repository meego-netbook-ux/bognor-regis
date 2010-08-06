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

#include "config.h"
#include "br-marshal.h"
#include "br-player.h"

enum {
    SHOW_URI,
    COMPLETED,
    CHANGED,
    LAST_SIGNAL
};

struct _BrPlayerPrivate {
    gboolean can_show_video;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), BR_TYPE_PLAYER, BrPlayerPrivate))
G_DEFINE_TYPE (BrPlayer, br_player, G_TYPE_OBJECT);
static guint32 signals[LAST_SIGNAL] = {0, };

static gboolean bognor_player_show_uri (BrPlayer   *player,
                                        const char *uri,
                                        const char *mimetype,
                                        GError    **error);
static gboolean bognor_player_can_show_video (BrPlayer *player,
                                              gboolean *OUT_can_show_video,
                                              GError  **error);

#include "bognor-player-glue.h"

static void
br_player_finalize (GObject *object)
{
    G_OBJECT_CLASS (br_player_parent_class)->finalize (object);
}

static void
br_player_dispose (GObject *object)
{
    G_OBJECT_CLASS (br_player_parent_class)->dispose (object);
}

static gboolean
bognor_player_show_uri (BrPlayer   *player,
                        const char *uri,
                        const char *mimetype,
                        GError    **error)
{
    g_signal_emit (player, signals[SHOW_URI], 0, uri, mimetype);
    return TRUE;
}

static gboolean
bognor_player_can_show_video (BrPlayer *player,
                              gboolean *OUT_can_show_video,
                              GError  **error)
{
    BrPlayerPrivate *priv = player->priv;

    *OUT_can_show_video = priv->can_show_video;
    return TRUE;
}

static void
br_player_class_init (BrPlayerClass *klass)
{
    GObjectClass *o_class = (GObjectClass *)klass;

    o_class->dispose = br_player_dispose;
    o_class->finalize = br_player_finalize;

    g_type_class_add_private (klass, sizeof (BrPlayerPrivate));

    dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass),
                                     &dbus_glib_bognor_player_object_info);

    signals[SHOW_URI] = g_signal_new ("show-uri",
                                      G_TYPE_FROM_CLASS (klass),
                                      G_SIGNAL_RUN_FIRST |
                                      G_SIGNAL_NO_RECURSE, 0, NULL, NULL,
                                      br_marshal_VOID__STRING_STRING,
                                      G_TYPE_NONE, 2, G_TYPE_STRING,
                                      G_TYPE_STRING);
    signals[COMPLETED] = g_signal_new ("uri-completed",
                                       G_TYPE_FROM_CLASS (klass),
                                       G_SIGNAL_RUN_FIRST |
                                       G_SIGNAL_NO_RECURSE, 0, NULL, NULL,
                                       g_cclosure_marshal_VOID__STRING,
                                       G_TYPE_NONE, 1, G_TYPE_STRING);
    signals[CHANGED] = g_signal_new ("can-show-video-changed",
                                     G_TYPE_FROM_CLASS (klass),
                                     G_SIGNAL_RUN_FIRST |
                                     G_SIGNAL_NO_RECURSE, 0, NULL, NULL,
                                     g_cclosure_marshal_VOID__BOOLEAN,
                                     G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
}

static void
br_player_init (BrPlayer *self)
{
    BrPlayerPrivate *priv;

    self->priv = GET_PRIVATE (self);
    priv = self->priv;

    priv->can_show_video = FALSE;
}

void
br_player_set_can_show_video (BrPlayer *player,
                              gboolean  can_show_video)
{
    BrPlayerPrivate *priv = player->priv;

    priv->can_show_video = can_show_video;
    g_signal_emit (player, signals[CHANGED], 0, can_show_video);
}

void
br_player_uri_completed (BrPlayer   *player,
                         const char *uri)
{
    g_signal_emit (player, signals[COMPLETED], 0, uri);
}
