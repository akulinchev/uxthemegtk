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
static GtkWidget *window = NULL;

static void draw_pane(cairo_t *cr, int width, int height)
{
    GtkStyleContext *context = pgtk_widget_get_style_context(window);
    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_BACKGROUND);
    pgtk_render_background(context, cr, 0, 0, width, height);
}

static void draw_gripper(cairo_t *cr, int width, int height)
{
    GtkStyleContext *context = pgtk_widget_get_style_context(window);

    pgtk_style_context_save(context);

    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_GRIP);
    pgtk_style_context_set_junction_sides(context, GTK_JUNCTION_CORNER_BOTTOMRIGHT);

    pgtk_render_handle(context, cr, 0, 0, width, height);

    pgtk_style_context_restore(context);
}

void uxgtk_status_init(void)
{
    TRACE("()\n");

    window = pgtk_window_new(GTK_WINDOW_TOPLEVEL);

    pgtk_widget_style_get(window,
                          "resize-grip-width", &grip_width,
                          "resize-grip-height", &grip_height,
                          NULL);

    TRACE("-GtkWindow-resize-grip-width: %d\n", grip_width);
    TRACE("-GtkWindow-resize-grip-height: %d\n", grip_height);
}

void uxgtk_status_uninit(void)
{
    TRACE("()\n");
    pgtk_widget_destroy(window);
}

void uxgtk_status_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height)
{
    TRACE("(%p, %d, %d, %d, %d)\n", cr, part_id, state_id, width, height);

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
