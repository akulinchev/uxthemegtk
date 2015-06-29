/*
 * This file is part of UxThemeGTK.
 *
 * Copyright © 2015 Ivan Akulinchev
 *
 * UxThemeGTK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * UxThemeGTK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with UxThemeGTK.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "header.h"

#include <vsstyle.h>

#include <wine/debug.h>

#include <gtk/gtk.h>

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

static GtkStyleContext *context = NULL;

static void draw_header_item(cairo_t *cr, int state_id, int width, int height)
{
    GtkStateFlags state = GTK_STATE_FLAG_NORMAL;

    if (state_id == HIS_HOT)
        state = GTK_STATE_FLAG_PRELIGHT;
    else if (state_id == HIS_PRESSED)
        state = GTK_STATE_FLAG_ACTIVE;

    gtk_style_context_set_state(context, state);

    gtk_render_background(context, cr, 0, 0, width, height);
    gtk_render_frame(context, cr, 0, 0, width, height);
}

void uxgtk_header_init(GdkScreen *screen)
{
    GtkWidgetPath *path = gtk_widget_path_new();
    int pos = gtk_widget_path_append_type(path, GTK_TYPE_TREE_VIEW);

    gtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_VIEW);
    gtk_widget_path_iter_add_region(path, pos, GTK_STYLE_REGION_COLUMN_HEADER, 0);

    pos = gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
    gtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_BUTTON);

    context = gtk_style_context_new();
    gtk_style_context_set_path(context, path);
    gtk_style_context_set_screen(context, screen);
}

void uxgtk_header_uninit(void)
{
    g_object_unref(context);
}

void uxgtk_header_draw_background(cairo_t *cr,
                                  int part_id, int state_id,
                                  int width, int height)
{
    gtk_style_context_save(context);

    if (part_id == HP_HEADERITEM)
        draw_header_item(cr, state_id, width, height);
    else
        WINE_FIXME("Unsupported header part %d.\n", part_id);

    gtk_style_context_restore(context);
}

BOOL uxgtk_header_is_part_defined(int part_id, int state_id)
{
    /* comctl32.dll uses only HP_HEADERITEM */
    return part_id == HP_HEADERITEM;
}

/* vim: set expandtab tabstop=8 shiftwidth=4 softtabstop=4: */
