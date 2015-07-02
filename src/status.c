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
#include <winerror.h>
#include <wine/debug.h>

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

static int grip_width = 0, grip_height = 0;
static GtkStyleContext *context = NULL;

static void draw_pane(cairo_t *cr, int width, int height)
{
    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_BACKGROUND);
    pgtk_render_background(context, cr, 0, 0, width, height);
}

static void draw_gripper(cairo_t *cr, int width, int height)
{
    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_GRIP);
    pgtk_style_context_set_junction_sides(context, GTK_JUNCTION_CORNER_BOTTOMRIGHT);
    pgtk_render_handle(context, cr, 0, 0, width, height);
}

void uxgtk_status_init(GdkScreen *screen)
{
    GtkWidgetPath *path;
 
    TRACE("(%p)\n", screen);

    path = pgtk_widget_path_new();

    pgtk_widget_path_append_type(path, pgtk_window_get_type());

    context = pgtk_style_context_new();
    pgtk_style_context_set_path(context, path);
    pgtk_style_context_set_screen(context, screen);

    pgtk_style_context_get_style(context,
                                 "resize-grip-width", &grip_width,
                                 "resize-grip-height", &grip_height,
                                 NULL);

    TRACE("-GtkWindow-resize-grip-width: %d\n", grip_width);
    TRACE("-GtkWindow-resize-grip-height: %d\n", grip_height);
}

void uxgtk_status_uninit(void)
{
    TRACE("()\n");
    pg_object_unref(context);
}

void uxgtk_status_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height)
{
    TRACE("(%p, %d, %d, %d, %d)\n", cr, part_id, state_id, width, height);

    pgtk_style_context_save(context);

    switch (part_id)
    {
        case 0:
        case SP_PANE:
        case SP_GRIPPERPANE:
            draw_pane(cr, width, height);
            break;

        case SP_GRIPPER:
            draw_gripper(cr, width, height);
            break;

        default:
            FIXME("Unknown status part %d.\n", part_id);
            break;
    }

    pgtk_style_context_restore(context);
}

HRESULT uxgtk_status_get_part_size(int part_id, int state_id, RECT *rect, SIZE *size)
{
    TRACE("(%d, %d, %p, %p)\n", part_id, state_id, rect, size);

    if (part_id != SP_GRIPPER)
        return E_FAIL;

    size->cx = grip_width;
    size->cy = grip_height;

    return S_OK;
}

BOOL uxgtk_status_is_part_defined(int part_id, int state_id)
{
    TRACE("(%d, %d)\n", part_id, state_id);

    /* comctl32.dll thinks SP_PANE == 0 */
    return (part_id >= 0 && part_id <= SP_GRIPPER);
}
