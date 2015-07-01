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

#include "listbox.h"

#include <vsstyle.h>

#include <wine/debug.h>

#include <gtk/gtk.h>

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

static GtkStyleContext *context = NULL;

void uxgtk_listbox_init(GdkScreen *screen)
{
    GtkWidgetPath *path = gtk_widget_path_new();
    int pos = gtk_widget_path_append_type(path, GTK_TYPE_SCROLLED_WINDOW);

    gtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_VIEW);
    gtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_FRAME);

    context = gtk_style_context_new();
    gtk_style_context_set_path(context, path);
    gtk_style_context_set_screen(context, screen);
}

void uxgtk_listbox_uninit(void)
{
    g_object_unref(context);
}

void uxgtk_listbox_draw_background(cairo_t *cr,
                                   int part_id, int state_id,
                                   int width, int height)
{
    switch (part_id) {
    case 0:
    case LBCP_BORDER_HSCROLL:
    case LBCP_BORDER_HVSCROLL:
    case LBCP_BORDER_NOSCROLL:
    case LBCP_BORDER_VSCROLL:
        gtk_render_background(context, cr, 0, 0, width, height);
        gtk_render_frame(context, cr, 0, 0, width, height);
        break;

    default:
        WINE_FIXME("Unsupported listbox part %d.\n", part_id);
    }
}

BOOL uxgtk_listbox_is_part_defined(int part_id, int state_id)
{
    /* >= 0 because comctl32.dll always sends 0. LBCP_ITEM is not used. */
    return part_id >= 0 && part_id < LBCP_ITEM;
}

/* vim: set expandtab tabstop=8 shiftwidth=4 softtabstop=4: */
