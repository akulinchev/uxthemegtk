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

#include "toolbar.h"

#include <vsstyle.h>

#include <wine/debug.h>

#include <gtk/gtk.h>

#include "button.h"

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

GtkStyleContext *context = NULL;

static void draw_button(cairo_t *cr, int state_id, int width, int height)
{
    if (state_id == TS_HOT)
        uxgtk_button_draw_background(cr, BP_PUSHBUTTON, PBS_HOT, width, height);
    else if (state_id == TS_PRESSED)
        uxgtk_button_draw_background(cr, BP_PUSHBUTTON, PBS_PRESSED, width, height);
}

static void draw_separator(cairo_t *cr, int part_id, int width, int height)
{
    int x1, x2, y1, y2;

    if (part_id == TP_SEPARATOR) { /* TP_SEPARATORVERT ? */
        y1 = 0;
        y2 = height;
        x1 = x2 = width/2;
    } else {
        x1 = 0;
        x2 = width;
        y1 = y2 = height/2;
    }

    gtk_render_line(context, cr, x1, y1, x2, y2);
}

void uxgtk_toolbar_init(GdkScreen *screen)
{
    GtkWidgetPath *path = gtk_widget_path_new();
    int pos = gtk_widget_path_append_type(path, GTK_TYPE_SEPARATOR);
    gtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_SEPARATOR);

    context = gtk_style_context_new();
    gtk_style_context_set_path(context, path);
    gtk_style_context_set_screen(context, screen);
}

void uxgtk_toolbar_uninit(void)
{
    g_object_unref(context);
}

void uxgtk_toolbar_draw_background(cairo_t *cr,
                                   int part_id, int state_id,
                                   int width, int height)
{
    switch (part_id) {
    case TP_BUTTON:
        draw_button(cr, state_id, width, height);
        break;

    case TP_SEPARATOR:
    case TP_SEPARATORVERT:
        draw_separator(cr, part_id, width, height);
        break;

    default:
        WINE_FIXME("Unknown toolbar part %d.\n", part_id);
    }
}

BOOL uxgtk_toolbar_is_part_defined(int part_id, int state_id)
{
    return part_id == TP_BUTTON || part_id == TP_SEPARATOR || part_id == TP_SEPARATORVERT;
}

/* vim: set expandtab tabstop=8 shiftwidth=4 softtabstop=4: */
