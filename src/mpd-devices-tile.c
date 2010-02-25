
/*
 * Copyright © 2010 Intel Corp.
 *
 * Authors: Rob Staudinger <robert.staudinger@intel.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdbool.h>

#include "mpd-default-device-tile.h"
#include "mpd-devices-tile.h"
#include "config.h"

G_DEFINE_TYPE (MpdDevicesTile, mpd_devices_tile, MX_TYPE_SCROLL_VIEW)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MPD_TYPE_DEVICES_TILE, MpdDevicesTilePrivate))

enum
{
  REQUEST_HIDE,

  LAST_SIGNAL
};

typedef struct
{
  ClutterActor *vbox;
} MpdDevicesTilePrivate;

static unsigned int _signals[LAST_SIGNAL] = { 0, };

static void
_device_tile_request_hide_cb (ClutterActor    *tile,
                              MpdDevicesTile  *self)
{
  g_signal_emit_by_name (self, "request-hide");
}

static void
_dispose (GObject *object)
{
  G_OBJECT_CLASS (mpd_devices_tile_parent_class)->dispose (object);
}

static void
mpd_devices_tile_class_init (MpdDevicesTileClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MpdDevicesTilePrivate));

  object_class->dispose = _dispose;

  /* Signals */

  _signals[REQUEST_HIDE] = g_signal_new ("request-hide",
                                         G_TYPE_FROM_CLASS (klass),
                                         G_SIGNAL_RUN_LAST,
                                         0, NULL, NULL,
                                         g_cclosure_marshal_VOID__VOID,
                                         G_TYPE_NONE, 0);
}

static void
mpd_devices_tile_init (MpdDevicesTile *self)
{
  MpdDevicesTilePrivate *priv = GET_PRIVATE (self);
  ClutterActor *tile;

  priv->vbox = mx_box_layout_new ();
  mx_box_layout_set_pack_start (MX_BOX_LAYOUT (priv->vbox), true);
  mx_box_layout_set_vertical (MX_BOX_LAYOUT (priv->vbox), true);
  clutter_container_add_actor (CLUTTER_CONTAINER (self), priv->vbox);

  tile = mpd_default_device_tile_new ();
  g_signal_connect (tile, "request-hide",
                    G_CALLBACK (_device_tile_request_hide_cb), self);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->vbox), tile);
}

ClutterActor *
mpd_devices_tile_new (void)
{
  return g_object_new (MPD_TYPE_DEVICES_TILE, NULL);
}

