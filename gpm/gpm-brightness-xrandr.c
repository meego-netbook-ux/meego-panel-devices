/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2008 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include <glib/gi18n.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <gdk/gdkx.h>

#include "egg-discrete.h"
#include "gpm-brightness-xrandr.h"

#define GPM_BRIGHTNESS_DIM_INTERVAL	5 /* ms */

/**
 * gpm_brightness_get_step:
 * @levels: The number of levels supported
 * Return value: the amount of hardware steps to do on each increment or decrement
 **/
static guint
gpm_brightness_get_step (guint levels)
{
	/* macbook pro has a bazzillion brightness levels, do in 5% steps */
	if (levels > 20)
		return levels / 20;
	return 1;
}


#define GPM_BRIGHTNESS_XRANDR_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GPM_TYPE_BRIGHTNESS_XRANDR, GpmBrightnessXRandRPrivate))

struct GpmBrightnessXRandRPrivate
{
	guint			 last_set_hw;
	Atom			 backlight;
	Display			*dpy;
	guint			 shared_value;
	gboolean		 has_extension;
#if (RANDR_MAJOR == 1 && RANDR_MINOR >= 3)
	gboolean		 has_randr13;
#endif
	gboolean		 hw_changed;
	/* A cache of XRRScreenResources is used as XRRGetScreenResources is expensive */
	GPtrArray		*resources;
};

enum {
	BRIGHTNESS_CHANGED,
	LAST_SIGNAL
};

typedef enum {
	ACTION_BACKLIGHT_GET,
	ACTION_BACKLIGHT_SET,
	ACTION_BACKLIGHT_INC,
	ACTION_BACKLIGHT_DEC
} GpmXRandROp;

G_DEFINE_TYPE (GpmBrightnessXRandR, gpm_brightness_xrandr, G_TYPE_OBJECT)
static guint signals [LAST_SIGNAL] = { 0 };

/**
 * gpm_brightness_xrandr_output_get_internal:
 **/
static gboolean
gpm_brightness_xrandr_output_get_internal (GpmBrightnessXRandR *brightness, RROutput output, guint *cur)
{
	unsigned long nitems;
	unsigned long bytes_after;
	guint *prop;
	Atom actual_type;
	int actual_format;
	gboolean ret = FALSE;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);

	if (XRRGetOutputProperty (brightness->priv->dpy, output, brightness->priv->backlight,
				  0, 4, False, False, None,
				  &actual_type, &actual_format,
				  &nitems, &bytes_after, ((unsigned char **)&prop)) != Success) {
		g_debug ("failed to get property");
		return FALSE;
	}
	if (actual_type == XA_INTEGER && nitems == 1 && actual_format == 32) {
		memcpy (cur, prop, sizeof (guint));
		ret = TRUE;
	}
	XFree (prop);
	return ret;
}

/**
 * gpm_brightness_xrandr_output_set_internal:
 **/
static gboolean
gpm_brightness_xrandr_output_set_internal (GpmBrightnessXRandR *brightness, RROutput output, guint value)
{
	gboolean ret = TRUE;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);

	/* don't abort on error */
	gdk_error_trap_push ();
	XRRChangeOutputProperty (brightness->priv->dpy, output, brightness->priv->backlight, XA_INTEGER, 32,
				 PropModeReplace, (unsigned char *) &value, 1);
	XFlush (brightness->priv->dpy);
	gdk_flush ();
	if (gdk_error_trap_pop ()) {
		g_warning ("failed to XRRChangeOutputProperty for brightness %i", value);
		ret = FALSE;
	}
	/* we changed the hardware */
	if (ret)
		brightness->priv->hw_changed = TRUE;
	return ret;
}

/**
 * gpm_brightness_xrandr_setup_display:
 **/
static gboolean
gpm_brightness_xrandr_setup_display (GpmBrightnessXRandR *brightness)
{
	gint major, minor;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);

	/* get the display */
	brightness->priv->dpy = GDK_DISPLAY();
	if (!brightness->priv->dpy) {
		g_critical ("Cannot open display");
		return FALSE;
	}
	/* is XRandR new enough? */
	if (!XRRQueryVersion (brightness->priv->dpy, &major, &minor)) {
		g_debug ("RandR extension missing");
		return FALSE;
	}
	if (major < 1 || (major == 1 && minor < 2)) {
		g_debug ("RandR version %d.%d too old", major, minor);
		return FALSE;
	}
	/* can we support BACKLIGHT */
	brightness->priv->backlight = XInternAtom (brightness->priv->dpy, "BACKLIGHT", True);
	if (brightness->priv->backlight == None) {
		g_debug ("No outputs have backlight property");
		return FALSE;
	}
	return TRUE;
}

#if (RANDR_MAJOR == 1 && RANDR_MINOR >= 3)
/**
 * gpm_brightness_xrandr_setup_version: Check whether xserver really supports xrandr-1.3 features.
 **/
static gboolean
gpm_brightness_xrandr_setup_version (GpmBrightnessXRandR *brightness)
{
	gint major, minor;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);

	/* get the display */
	brightness->priv->dpy = GDK_DISPLAY();
	if (!brightness->priv->dpy) {
		g_critical ("Cannot open display");
		return FALSE;
	}
	if (!XRRQueryVersion (brightness->priv->dpy, &major, &minor)) {
		return FALSE;
	}
	if (major == 1 && minor < 3) {
		g_debug ("RandR version %d.%d does not support XRRGetScreenResourcesCurrent", major, minor);
		return FALSE;
	}
	return TRUE;
}
#endif

/**
 * gpm_brightness_xrandr_output_get_limits:
 **/
static gboolean
gpm_brightness_xrandr_output_get_limits (GpmBrightnessXRandR *brightness, RROutput output,
					 guint *min, guint *max)
{
	XRRPropertyInfo *info;
	gboolean ret = TRUE;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);

	info = XRRQueryOutputProperty (brightness->priv->dpy, output, brightness->priv->backlight);
	if (info == NULL) {
		g_debug ("could not get output property");
		return FALSE;
	}
	if (!info->range || info->num_values != 2) {
		g_debug ("was not range");
		ret = FALSE;
		goto out;
	}
	*min = info->values[0];
	*max = info->values[1];
out:
	XFree (info);
	return ret;
}

/**
 * gpm_brightness_xrandr_output_get_percentage:
 **/
static gboolean
gpm_brightness_xrandr_output_get_percentage (GpmBrightnessXRandR *brightness, RROutput output)
{
	guint cur;
	gboolean ret;
	guint min, max;
	guint percentage;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);

	ret = gpm_brightness_xrandr_output_get_internal (brightness, output, &cur);
	if (!ret)
		return FALSE;
	ret = gpm_brightness_xrandr_output_get_limits (brightness, output, &min, &max);
	if (!ret || min == max)
		return FALSE;
	g_debug ("hard value=%i, min=%i, max=%i", cur, min, max);
	percentage = egg_discrete_to_percent (cur, (max-min)+1);
	g_debug ("percentage %i", percentage);
	brightness->priv->shared_value = percentage;
	return TRUE;
}

/**
 * gpm_brightness_xrandr_output_down:
 **/
static gboolean
gpm_brightness_xrandr_output_down (GpmBrightnessXRandR *brightness, RROutput output)
{
	guint cur;
	guint step;
	gboolean ret;
	guint min, max;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);

	ret = gpm_brightness_xrandr_output_get_internal (brightness, output, &cur);
	if (!ret)
		return FALSE;
	ret = gpm_brightness_xrandr_output_get_limits (brightness, output, &min, &max);
	if (!ret || min == max)
		return FALSE;
	g_debug ("hard value=%i, min=%i, max=%i", cur, min, max);
	if (cur == min) {
		g_debug ("already min");
		return TRUE;
	}
	step = gpm_brightness_get_step ((max-min)+1);
	if (cur < step) {
		g_debug ("truncating to %i", min);
		cur = min;
	} else {
		cur -= step;
	}
	ret = gpm_brightness_xrandr_output_set_internal (brightness, output, cur);
	return ret;
}

/**
 * gpm_brightness_xrandr_output_up:
 **/
static gboolean
gpm_brightness_xrandr_output_up (GpmBrightnessXRandR *brightness, RROutput output)
{
	guint cur;
	gboolean ret;
	guint min, max;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);

	ret = gpm_brightness_xrandr_output_get_internal (brightness, output, &cur);
	if (!ret)
		return FALSE;
	ret = gpm_brightness_xrandr_output_get_limits (brightness, output, &min, &max);
	if (!ret || min == max)
		return FALSE;
	g_debug ("hard value=%i, min=%i, max=%i", cur, min, max);
	if (cur == max) {
		g_debug ("already max");
		return TRUE;
	}
	cur += gpm_brightness_get_step ((max-min)+1);
	if (cur > max) {
		g_debug ("truncating to %i", max);
		cur = max;
	}
	ret = gpm_brightness_xrandr_output_set_internal (brightness, output, cur);
	return ret;
}

/**
 * gpm_brightness_xrandr_output_set:
 **/
static gboolean
gpm_brightness_xrandr_output_set (GpmBrightnessXRandR *brightness, RROutput output)
{
	guint cur;
	gboolean ret;
	guint min, max;
	gint i;
	gint shared_value_abs;
	guint step;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);

	ret = gpm_brightness_xrandr_output_get_internal (brightness, output, &cur);
	if (!ret)
		return FALSE;
	ret = gpm_brightness_xrandr_output_get_limits (brightness, output, &min, &max);
	if (!ret || min == max)
		return FALSE;

	shared_value_abs = egg_discrete_from_percent (brightness->priv->shared_value, (max-min)+1);
	g_debug ("percent=%i, absolute=%i", brightness->priv->shared_value, shared_value_abs);

	g_debug ("hard value=%i, min=%i, max=%i", cur, min, max);
	if (shared_value_abs > (gint) max)
		shared_value_abs = max;
	if (shared_value_abs < (gint) min)
		shared_value_abs = min;
	if ((gint) cur == shared_value_abs) {
		g_debug ("already set %i", cur);
		return TRUE;
	}

	/* step the correct way */
	if ((gint) cur < shared_value_abs) {

		/* some adaptors have a large number of steps */
		step = gpm_brightness_get_step (shared_value_abs - cur);
		g_debug ("using step of %i", step);

		/* going up */
		for (i=cur; i<=shared_value_abs; i+=step) {
			ret = gpm_brightness_xrandr_output_set_internal (brightness, output, i);
			if (!ret)
				break;
			if ((gint) cur != shared_value_abs)
				g_usleep (1000 * GPM_BRIGHTNESS_DIM_INTERVAL);
		}
	} else {

		/* some adaptors have a large number of steps */
		step = gpm_brightness_get_step (cur - shared_value_abs);
		g_debug ("using step of %i", step);

		/* going down */
		for (i=cur; i>=shared_value_abs; i-=step) {
			ret = gpm_brightness_xrandr_output_set_internal (brightness, output, i);
			if (!ret)
				break;
			if ((gint) cur != shared_value_abs)
				g_usleep (1000 * GPM_BRIGHTNESS_DIM_INTERVAL);
		}
	}
	return TRUE;
}

/**
 * gpm_brightness_xrandr_foreach_resource:
 **/
static gboolean
gpm_brightness_xrandr_foreach_resource (GpmBrightnessXRandR *brightness, GpmXRandROp op, XRRScreenResources *resources)
{
	gint i;
	gboolean ret;
	gboolean success_any = FALSE;
	RROutput output;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);

	/* do for each output */
	for (i=0; i<resources->noutput; i++) {
		output = resources->outputs[i];
		g_debug ("resource %i of %i", i+1, resources->noutput);
		if (op==ACTION_BACKLIGHT_GET) {
			ret = gpm_brightness_xrandr_output_get_percentage (brightness, output);
		} else if (op==ACTION_BACKLIGHT_INC) {
			ret = gpm_brightness_xrandr_output_up (brightness, output);
		} else if (op==ACTION_BACKLIGHT_DEC) {
			ret = gpm_brightness_xrandr_output_down (brightness, output);
		} else if (op==ACTION_BACKLIGHT_SET) {
			ret = gpm_brightness_xrandr_output_set (brightness, output);
		} else {
			ret = FALSE;
			g_warning ("op not known");
		}
		if (ret) {
			success_any = TRUE;
		}
	}
	return success_any;
}

/**
 * gpm_brightness_xrandr_foreach_screen:
 **/
static gboolean
gpm_brightness_xrandr_foreach_screen (GpmBrightnessXRandR *brightness, GpmXRandROp op)
{
	guint i;
	guint length;
	XRRScreenResources *resource;
	gboolean ret;
	gboolean success_any = FALSE;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);

	/* do for each screen */
	length = brightness->priv->resources->len;
	for (i=0; i<length; i++) {
		resource = (XRRScreenResources *) g_ptr_array_index (brightness->priv->resources, i);
		g_debug ("using resource %p", resource);
		ret = gpm_brightness_xrandr_foreach_resource (brightness, op, resource);
		if (ret)
			success_any = TRUE;
	}
	XSync (brightness->priv->dpy, False);
	return success_any;
}

/**
 * gpm_brightness_xrandr_set:
 * @brightness: This brightness class instance
 * @percentage: The percentage brightness
 * @hw_changed: If the hardware was changed, i.e. the brightness changed
 * Return value: %TRUE if success, %FALSE if there was an error
 **/
gboolean
gpm_brightness_xrandr_set (GpmBrightnessXRandR *brightness, guint percentage, gboolean *hw_changed)
{
	gboolean ret;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);
	g_return_val_if_fail (hw_changed != NULL, FALSE);

	brightness->priv->shared_value = percentage;

	/* reset to not-changed */
	brightness->priv->hw_changed = FALSE;
	ret = gpm_brightness_xrandr_foreach_screen (brightness, ACTION_BACKLIGHT_SET);

	/* did the hardware have to be modified? */
	*hw_changed = brightness->priv->hw_changed;
	return ret;
}

/**
 * gpm_brightness_xrandr_get:
 * @brightness: This brightness class instance
 * @percentage: Value to retrieve
 * Return value: %TRUE if success, %FALSE if there was an error
 **/
gboolean
gpm_brightness_xrandr_get (GpmBrightnessXRandR *brightness, guint *percentage)
{
	gboolean ret;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);
	g_return_val_if_fail (percentage != NULL, FALSE);

	ret = gpm_brightness_xrandr_foreach_screen (brightness, ACTION_BACKLIGHT_GET);
	*percentage = brightness->priv->shared_value;
	return ret;
}

/**
 * gpm_brightness_xrandr_up:
 * @brightness: This brightness class instance
 * @hw_changed: If the hardware was changed, i.e. the brightness changed
 * Return value: %TRUE if success, %FALSE if there was an error
 *
 * If possible, put the brightness of the LCD up one unit.
 **/
gboolean
gpm_brightness_xrandr_up (GpmBrightnessXRandR *brightness, gboolean *hw_changed)
{
	gboolean ret;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);
	g_return_val_if_fail (hw_changed != NULL, FALSE);

	/* reset to not-changed */
	brightness->priv->hw_changed = FALSE;
	ret = gpm_brightness_xrandr_foreach_screen (brightness, ACTION_BACKLIGHT_INC);

	/* did the hardware have to be modified? */
	*hw_changed = brightness->priv->hw_changed;
	return ret;
}

/**
 * gpm_brightness_xrandr_down:
 * @brightness: This brightness class instance
 * @hw_changed: If the hardware was changed, i.e. the brightness changed
 * Return value: %TRUE if success, %FALSE if there was an error
 *
 * If possible, put the brightness of the LCD down one unit.
 **/
gboolean
gpm_brightness_xrandr_down (GpmBrightnessXRandR *brightness, gboolean *hw_changed)
{
	gboolean ret;

	g_return_val_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness), FALSE);
	g_return_val_if_fail (hw_changed != NULL, FALSE);

	/* reset to not-changed */
	brightness->priv->hw_changed = FALSE;
	ret = gpm_brightness_xrandr_foreach_screen (brightness, ACTION_BACKLIGHT_DEC);

	/* did the hardware have to be modified? */
	*hw_changed = brightness->priv->hw_changed;
	return ret;
}

/**
 * gpm_brightness_xrandr_has_hw:
 **/
gboolean
gpm_brightness_xrandr_has_hw (GpmBrightnessXRandR *brightness)
{
	return brightness->priv->has_extension;
}

/**
 * gpm_brightness_xrandr_may_have_changed:
 **/
static void
gpm_brightness_xrandr_may_have_changed (GpmBrightnessXRandR *brightness)
{
	gboolean ret;
	guint percentage;
	ret = gpm_brightness_xrandr_get (brightness, &percentage);
	if (!ret) {
		g_warning ("failed to get output");
		return;
	}
	g_debug ("emitting brightness-changed (%i)", percentage);
	g_signal_emit (brightness, signals [BRIGHTNESS_CHANGED], 0, percentage);
}

/**
 * gpm_brightness_xrandr_filter_xevents:
 **/
static GdkFilterReturn
gpm_brightness_xrandr_filter_xevents (GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
	GpmBrightnessXRandR *brightness = GPM_BRIGHTNESS_XRANDR (data);
	if (event->type == GDK_NOTHING)
		return GDK_FILTER_CONTINUE;
	gpm_brightness_xrandr_may_have_changed (brightness);
	return GDK_FILTER_CONTINUE;
}


static void gpm_brightness_xrandr_update_cache (GpmBrightnessXRandR *brightness);

/**
 * gpm_brightness_monitors_changed:
 **/
static void
gpm_brightness_monitors_changed (GdkScreen *screen, GpmBrightnessXRandR *brightness)
{
	g_return_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness));
	gpm_brightness_xrandr_update_cache (brightness);
}

/**
 * gpm_brightness_xrandr_update_cache:
 **/
static void
gpm_brightness_xrandr_update_cache (GpmBrightnessXRandR *brightness)
{
	guint length;
	gint screen;
	Window root;
	GdkScreen *gscreen;
	GdkDisplay *display;
	XRRScreenResources *resource;

	g_return_if_fail (GPM_IS_BRIGHTNESS_XRANDR (brightness));

	/* invalidate and remove all the previous entries */
	length = brightness->priv->resources->len;
	if (length > 0)
		g_ptr_array_set_size (brightness->priv->resources, 0);

	/* do for each screen */
	display = gdk_display_get_default ();
	length = ScreenCount (brightness->priv->dpy);
	for (screen = 0; screen < (gint) length; screen++) {
		g_debug ("screen %i of %i", screen+1, length);
		gscreen = gdk_display_get_screen (display, screen);

		/* if we have not setup the changed on the monitor, set it here */
		if (g_object_get_data (G_OBJECT (gscreen), "gpk-set-monitors-changed") == NULL) {
			g_debug ("watching ::monitors_changed on %p", gscreen);
			g_object_set_data (G_OBJECT (gscreen), "gpk-set-monitors-changed", (gpointer) "true");
			g_signal_connect (G_OBJECT (gscreen), "monitors_changed",
					  G_CALLBACK (gpm_brightness_monitors_changed), brightness);
		}

		root = RootWindow (brightness->priv->dpy, screen);
		/* XRRGetScreenResourcesCurrent is less expensive than
		   XRRGetScreenResources, however it is available only
		   in RandR 1.3 or higher and of course xserver needs
		   to support it.
		*/
#if (RANDR_MAJOR == 1 && RANDR_MINOR >= 3)
		if (brightness->priv->has_randr13)
			resource = XRRGetScreenResourcesCurrent (brightness->priv->dpy, root);
		else
			resource = XRRGetScreenResources (brightness->priv->dpy, root);
#else
		resource = XRRGetScreenResources (brightness->priv->dpy, root);
#endif
		if (resource != NULL) {
			g_debug ("adding resource %p", resource);
			g_ptr_array_add (brightness->priv->resources, resource);
		}
	}
}

/**
 * gpm_brightness_xrandr_finalize:
 **/
static void
gpm_brightness_xrandr_finalize (GObject *object)
{
	GpmBrightnessXRandR *brightness;
	g_return_if_fail (object != NULL);
	g_return_if_fail (GPM_IS_BRIGHTNESS_XRANDR (object));
	brightness = GPM_BRIGHTNESS_XRANDR (object);

	g_ptr_array_unref (brightness->priv->resources);

	G_OBJECT_CLASS (gpm_brightness_xrandr_parent_class)->finalize (object);
}

/**
 * gpm_brightness_xrandr_class_init:
 **/
static void
gpm_brightness_xrandr_class_init (GpmBrightnessXRandRClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gpm_brightness_xrandr_finalize;

	signals [BRIGHTNESS_CHANGED] =
		g_signal_new ("brightness-changed",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GpmBrightnessXRandRClass, brightness_changed),
			      NULL, NULL, g_cclosure_marshal_VOID__UINT,
			      G_TYPE_NONE, 1, G_TYPE_UINT);

	g_type_class_add_private (klass, sizeof (GpmBrightnessXRandRPrivate));
}

/**
 * gpm_brightness_xrandr_init:
 * @brightness: This brightness class instance
 **/
static void
gpm_brightness_xrandr_init (GpmBrightnessXRandR *brightness)
{
	GdkScreen *screen;
	GdkWindow *window;
	GdkDisplay *display;
	int event_base;
	int ignore;

	brightness->priv = GPM_BRIGHTNESS_XRANDR_GET_PRIVATE (brightness);
	brightness->priv->hw_changed = FALSE;
	brightness->priv->resources = g_ptr_array_new_with_free_func ((GDestroyNotify) XRRFreeScreenResources);

	/* can we do this */
	brightness->priv->has_extension = gpm_brightness_xrandr_setup_display (brightness);
#if (RANDR_MAJOR == 1 && RANDR_MINOR >= 3)
	brightness->priv->has_randr13 = gpm_brightness_xrandr_setup_version (brightness);
#endif
	if (brightness->priv->has_extension == FALSE) {
		g_debug ("no XRANDR extension, so aborting init");
		return;
	}

	screen = gdk_screen_get_default ();
	window = gdk_screen_get_root_window (screen);
	display = gdk_display_get_default ();

	/* as we a filtering by a window, we have to add an event type */
	if (!XRRQueryExtension (GDK_DISPLAY(), &event_base, &ignore)) {
		g_critical ("can't get event_base for XRR");
	}
	gdk_x11_register_standard_event_type (display, event_base, RRNotify + 1);
	gdk_window_add_filter (window, gpm_brightness_xrandr_filter_xevents, (gpointer) brightness);

	/* don't abort on error */
	gdk_error_trap_push ();
	XRRSelectInput (GDK_DISPLAY(), GDK_WINDOW_XID (window),
			RRScreenChangeNotifyMask |
			RROutputPropertyNotifyMask); /* <--- the only one we need, but see rh:345551 */
	gdk_flush ();
	if (gdk_error_trap_pop ())
		g_warning ("failed to select XRRSelectInput");

	/* create cache of XRRScreenResources as XRRGetScreenResources() is slow */
	gpm_brightness_xrandr_update_cache (brightness);
}

/**
 * gpm_brightness_xrandr_new:
 * Return value: A new brightness class instance.
 * Can return NULL if no suitable hardware is found.
 **/
GpmBrightnessXRandR *
gpm_brightness_xrandr_new (void)
{
	GpmBrightnessXRandR *brightness;
	brightness = g_object_new (GPM_TYPE_BRIGHTNESS_XRANDR, NULL);
	return GPM_BRIGHTNESS_XRANDR (brightness);
}

