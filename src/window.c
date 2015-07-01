/*
 * GTK uxtheme implementation
 *
 * Copyright (C) 2015 Ivan Akulinchev
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "uxthemegtk_internal.h"

#include <vsstyle.h>
#include <vssym32.h>
#include <winerror.h>
#include <wine/debug.h>

#include <gtk/gtk.h>

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

static GtkStyleContext *context = NULL;

void uxgtk_window_init(GdkScreen *screen)
{
    GtkWidgetPath *path = gtk_widget_path_new();
    int pos = gtk_widget_path_append_type(path, GTK_TYPE_WINDOW);

    gtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_BACKGROUND);

    context = gtk_style_context_new();
    gtk_style_context_set_path(context, path);
    gtk_style_context_set_screen(context, screen);
}

void uxgtk_window_uninit(void)
{
    g_object_unref(context);
}

HRESULT uxgtk_window_get_color(int part_id, int state_id,
                               int prop_id, GdkRGBA *rgba)
{
    switch (prop_id) {
    case TMT_FILLCOLOR:
        gtk_style_context_get_background_color(context, GTK_STATE_FLAG_NORMAL, rgba);
        break;

    case TMT_TEXTCOLOR:
        gtk_style_context_get_color(context, GTK_STATE_FLAG_NORMAL, rgba);
        break;

    default:
        WINE_FIXME("Unknown property %d.\n", prop_id);
        return E_FAIL;
    }

    return S_OK;
}

void uxgtk_window_draw_background(cairo_t *cr,
                                  int part_id, int state_id,
                                  int width, int height)
{
    if (part_id == WP_DIALOG)
        gtk_render_background(context, cr, 0, 0, width, height);
    else
        WINE_FIXME("Unsupported window part %d.\n", part_id);
}

BOOL uxgtk_window_is_part_defined(int part_id, int state_id)
{
    return part_id == WP_DIALOG;
}

/* vim: set expandtab tabstop=8 shiftwidth=4 softtabstop=4: */
