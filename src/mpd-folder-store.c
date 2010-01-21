
/*
 * Copyright (c) 2010 Intel Corp.
 *
 * Author: Robert Staudinger <robert.staudinger@intel.com>
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

#include <gio/gio.h>
#include "mpd-folder-store.h"

#define MPD_FOLDER_STORE_ERROR mpd_folder_store_error_quark()

static GQuark
mpd_folder_store_error_quark (void)
{
  static GQuark _quark = 0;
  if (!_quark)
    _quark = g_quark_from_static_string ("mpd-folder-store-error");
  return _quark;
}

G_DEFINE_TYPE (MpdFolderStore, mpd_folder_store, CLUTTER_TYPE_LIST_MODEL)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MPD_TYPE_FOLDER_STORE, MpdFolderStorePrivate))

typedef struct
{
  GFileMonitor *monitor;
} MpdFolderStorePrivate;

static void
_changed (GFileMonitor      *monitor,
          GFile             *file,
          GFile             *other_file,
          GFileMonitorEvent  event_type,
          MpdFolderStore    *self)
{
  gchar   *path;
  GError  *error = NULL;

  path = g_file_get_path (file);
  mpd_folder_store_load_file (self, path, &error);
  g_free (path);
  if (error)
  {
    g_warning ("%s : %s", G_STRLOC, error->message);
    g_clear_error (&error);
  }
}

static void
_dispose (GObject *object)
{
  MpdFolderStorePrivate *priv = GET_PRIVATE (object);

  if (priv->monitor)
  {
    g_object_unref (priv->monitor);
    priv->monitor = NULL;
  }

  G_OBJECT_CLASS (mpd_folder_store_parent_class)->dispose (object);
}

static void
mpd_folder_store_class_init (MpdFolderStoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MpdFolderStorePrivate));

  object_class->dispose = _dispose;
}

static void
mpd_folder_store_init (MpdFolderStore *self)
{
}

ClutterModel *
mpd_folder_store_new (void)
{
  ClutterModel *model;
  GType         types[] = { G_TYPE_STRING, G_TYPE_STRING };

  model = g_object_new (MPD_TYPE_FOLDER_STORE, NULL);

  /* Column 0 ... URI
   * Column 1 ... Label or NULL */
  clutter_model_set_types (model, 2, types);

  return model;
}

gboolean
mpd_folder_store_load_file (MpdFolderStore   *self,
                            gchar const      *filename,
                            GError          **error)
{
  MpdFolderStorePrivate *priv = GET_PRIVATE (self);
  GFile   *file;
  gchar   *contents = NULL;
  gsize    length = 0;
  GError  *monitor_error = NULL;

  /* Reset store. */

  if (priv->monitor)
  {
    g_object_unref (priv->monitor);
    priv->monitor = NULL;
  }

  while (clutter_model_get_n_rows (CLUTTER_MODEL (self)) > 0)
  {
    clutter_model_remove (CLUTTER_MODEL (self), 0);
  }

  /* Load file. */

  if (!g_file_get_contents (filename, &contents, &length, error))
  {
    return FALSE;
  }

  if (length == 0)
  {
    if (error)
      *error = g_error_new (MPD_FOLDER_STORE_ERROR,
                            MPD_FOLDER_STORE_ERROR_BOOKMARKS_FILE_EMPTY,
                            "%s : Bookmarks file '%s' is empty",
                            G_STRLOC,
                            filename);
    return FALSE;
  }

  /* Parse content. */

  if (contents)
  {
    gchar **lines = g_strsplit (contents, "\n", -1);
    gchar **iter = lines;

    while (*iter)
    {
      gchar **line = g_strsplit (*iter, " ", 2);
      if (line && line[0])
      {
        const gchar *uri = line[0];
        gchar       *label = NULL;

        if (line[1])
          label = g_strdup (line[1]);
        else
          label = g_path_get_basename (uri);

        /* Insert into model. */
        clutter_model_append (CLUTTER_MODEL (self),
                              0, uri,
                              1, label,
                              -1);
        g_free (label);
      }
      g_strfreev (line);
      iter++;
    }
    g_strfreev (lines);
    g_free (contents);
  }

  /* Set up monitor. */

  file = g_file_new_for_path (filename);
  priv->monitor = g_file_monitor (file, G_FILE_MONITOR_NONE, NULL,
                                  &monitor_error);
  if (monitor_error)
  {
    g_warning ("%s : %s", G_STRLOC, monitor_error->message);
    g_clear_error (&monitor_error);
  } else {
    g_signal_connect (priv->monitor, "changed", G_CALLBACK (_changed), self);
  }

  return TRUE;
}
