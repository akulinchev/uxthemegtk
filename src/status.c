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

#include "status.h"

#include <vsstyle.h>
#include <winerror.h>

#include <wine/debug.h>

#include <gtk/gtk.h>

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

static int grip_width = 0, grip_height = 0;
static GtkStyleContext *context = NULL;

static void draw_pane(cairo_t *cr, int width, int height)
{
    gtk_style_context_add_class(context, GTK_STYLE_CLASS_BACKGROUND);
    gtk_render_background(context, cr, 0, 0, width, height);
}

static void draw_gripper(cairo_t *cr, int width, int height)
{
    gtk_style_context_add_class(context, GTK_STYLE_CLASS_GRIP);
    gtk_style_context_set_junction_sides(context, GTK_JUNCTION_CORNER_BOTTOMRIGHT);
    gtk_render_handle(context, cr, 0, 0, width, height);
}

void uxgtk_status_init(GdkScreen *screen)
{
    GtkWidgetPath *path = gtk_widget_path_new();

    gtk_widget_path_append_type(path, GTK_TYPE_WINDOW);

    context = gtk_style_context_new();
    gtk_style_context_set_path(context, path);
    gtk_style_context_set_screen(context, screen);

    gtk_style_context_get_style(context,
                                "resize-grip-width", &grip_width,
                                "resize-grip-height", &grip_height,
                                NULL);

    WINE_TRACE("-GtkWindow-resize-grip-width: %d;\n"
               "-GtkWindow-resize-grip-height: %d;\n",
               grip_width, grip_height);
}

void uxgtk_status_uninit(void)
{
    g_object_unref(context);
}

void uxgtk_status_draw_background(cairo_t *cr,
                                  int part_id, int state_id,
                                  int width, int height)
{
    gtk_style_context_save(context);

    switch (part_id) {
    case 0:
    case SP_PANE:
    case SP_GRIPPERPANE:
        draw_pane(cr, width, height);
        break;

    case SP_GRIPPER:
        draw_gripper(cr, width, height);
        break;

    default:
        WINE_FIXME("Unknown status part %d.\n", part_id);
    }

    gtk_style_context_restore(context);
}

HRESULT uxgtk_status_get_part_size(int part_id, int state_id,
                                   RECT *rect, SIZE *size)
{
    if (part_id != SP_GRIPPER)
        return E_FAIL;

    size->cx = grip_width;
    size->cy = grip_height;

    return S_OK;
}

BOOL uxgtk_status_is_part_defined(int part_id, int state_id)
{
    /* comctl32.dll thinks SP_PANE == 0 */
    return part_id >= 0 && part_id <= SP_GRIPPER;
}

/* vim: set expandtab tabstop=8 shiftwidth=4 softtabstop=4: */
