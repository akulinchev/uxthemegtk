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

#include <stdlib.h>

#include <vsstyle.h>
#include <winerror.h>
#include <wine/debug.h>

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

typedef struct _status_theme
{
    uxgtk_theme_t base;

    int grip_width;
    int grip_height;

    GtkWidget *window;
} status_theme_t;

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height);
static HRESULT get_part_size(uxgtk_theme_t *theme, int part_id, int state_id,
                             RECT *rect, SIZE *size);
static BOOL is_part_defined(int part_id, int state_id);
static void destroy(uxgtk_theme_t *theme);

static const uxgtk_theme_vtable_t status_vtable = {
    NULL, /* get_color */
    draw_background,
    get_part_size,
    is_part_defined,
    destroy
};

static void draw_pane(status_theme_t *theme, cairo_t *cr, int width, int height)
{
    GtkStyleContext *context = pgtk_widget_get_style_context(theme->window);
    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_BACKGROUND);
    pgtk_render_background(context, cr, 0, 0, width, height);
}

static void draw_gripper(status_theme_t *theme, cairo_t *cr, int width, int height)
{
    GtkStyleContext *context = pgtk_widget_get_style_context(theme->window);

    pgtk_style_context_save(context);

    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_GRIP);
    pgtk_style_context_set_junction_sides(context, GTK_JUNCTION_CORNER_BOTTOMRIGHT);

    pgtk_render_handle(context, cr, 0, 0, width, height);

    pgtk_style_context_restore(context);
}

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height)
{
    status_theme_t *status_theme = (status_theme_t *)theme;

    switch (part_id)
    {
        case 0:
        case SP_PANE:
        case SP_GRIPPERPANE:
            draw_pane(status_theme, cr, width, height);
            break;

        case SP_GRIPPER:
            draw_gripper(status_theme, cr, width, height);
            break;

        default:
            FIXME("Unknown status part %d.\n", part_id);
            break;
    }
}

static HRESULT get_part_size(uxgtk_theme_t *theme, int part_id, int state_id,
                             RECT *rect, SIZE *size)
{
    status_theme_t *status_theme = (status_theme_t *)theme;

    if (part_id != SP_GRIPPER)
        return E_FAIL;

    size->cx = status_theme->grip_width;
    size->cy = status_theme->grip_height;

    return S_OK;
}

static BOOL is_part_defined(int part_id, int state_id)
{
    /* comctl32.dll thinks SP_PANE == 0 */
    return (part_id >= 0 && part_id <= SP_GRIPPER);
}

static void destroy(uxgtk_theme_t *theme)
{
    pgtk_widget_destroy(((status_theme_t *)theme)->window);

    free(theme);
}

uxgtk_theme_t *uxgtk_status_theme_create(void)
{
    status_theme_t *theme;

    TRACE("()\n");

    theme = malloc(sizeof(status_theme_t));
    memset(theme, 0, sizeof(status_theme_t));

    theme->base.vtable = &status_vtable;

    theme->window = pgtk_window_new(GTK_WINDOW_TOPLEVEL);

    pgtk_widget_style_get(theme->window,
                          "resize-grip-width", &theme->grip_width,
                          "resize-grip-height", &theme->grip_height,
                          NULL);

    TRACE("-GtkWindow-resize-grip-width: %d\n", theme->grip_width);
    TRACE("-GtkWindow-resize-grip-height: %d\n", theme->grip_height);

    return &theme->base;
}
