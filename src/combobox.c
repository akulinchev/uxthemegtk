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

#include "combobox.h"

#include <vsstyle.h>

#include <wine/debug.h>

#include <gtk/gtk.h>

#include "edit.h"

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

#define ARROW_DOWN G_PI

#define ARROW_WIDTH 10
#define ARROW_HEIGHT 12 /* Approximate value; used to find the center */

static GtkStyleContext *context = NULL;

static void draw_border(cairo_t *cr, int state_id, int width, int height)
{
    int state = state_id == CBXS_DISABLED ? ETS_DISABLED : ETS_NORMAL;
    uxgtk_edit_draw_background(cr, EP_EDITTEXT, state, width, height);
}

static void draw_dropdown_button(cairo_t *cr, int state_id,
                                 int width, int height)
{
    int arrow_x, arrow_y;
    GtkStateFlags state = state_id == CBXSR_DISABLED
        ? GTK_STATE_FLAG_INSENSITIVE : GTK_STATE_FLAG_NORMAL;

    gtk_style_context_save(context);

    gtk_style_context_set_state(context, state);

    /* It's hard to place a real button here. So, just draw an arrow. */
    arrow_x = (width - ARROW_WIDTH) / 2;
    arrow_y = (height - ARROW_HEIGHT) / 2;
    gtk_render_arrow(context, cr, ARROW_DOWN, arrow_x, arrow_y, ARROW_WIDTH);

    gtk_style_context_restore(context);
}

void uxgtk_combobox_init(GdkScreen *screen)
{
    GtkWidgetPath *path = gtk_widget_path_new();
    int pos = gtk_widget_path_append_type(path, GTK_TYPE_COMBO_BOX);

    gtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_COMBOBOX_ENTRY);

    pos = gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
    gtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_BUTTON);

    context = gtk_style_context_new();
    gtk_style_context_set_path(context, path);
    gtk_style_context_set_screen(context, screen);
}

void uxgtk_combobox_uninit(void)
{
    g_object_unref(context);
}

void uxgtk_combobox_draw_background(cairo_t *cr,
                                    int part_id, int state_id,
                                    int width, int height)
{
    switch (part_id) {
    case 0:
    case CP_BORDER:
        draw_border(cr, state_id, width, height);
        break;

    case CP_DROPDOWNBUTTON:
        draw_dropdown_button(cr, state_id, width, height);
        break;

    default:
        WINE_FIXME("Unsupported combobox part %d.\n", part_id);
    }
}

BOOL uxgtk_combobox_is_part_defined(int part_id, int state_id)
{
    return part_id == 0 || part_id == CP_DROPDOWNBUTTON;
}

/* vim: set expandtab tabstop=8 shiftwidth=4 softtabstop=4: */
