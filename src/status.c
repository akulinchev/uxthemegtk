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

#include "uxthemegtk.h"

#include <stdlib.h>

#include "vsstyle.h"
#include "winerror.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

typedef struct _status_theme
{
    uxgtk_theme_t base;

    int grip_width;
    int grip_height;
} status_theme_t;

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height);
static HRESULT get_part_size(uxgtk_theme_t *theme, int part_id, int state_id,
                             RECT *rect, SIZE *size);
static BOOL is_part_defined(int part_id, int state_id);

static const uxgtk_theme_vtable_t status_vtable = {
    NULL, /* get_color */
    draw_background,
    get_part_size,
    is_part_defined
};

static void draw_pane(uxgtk_theme_t *theme, cairo_t *cr, int width, int height)
{
    GtkStyleContext *context = pgtk_widget_get_style_context(theme->window);

    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_BACKGROUND);
    pgtk_render_background(context, cr, 0, 0, width, height);
}

static void draw_gripper(uxgtk_theme_t *theme, cairo_t *cr, int width, int height)
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
    switch (part_id)
    {
        case 0:
        case SP_PANE:
        case SP_GRIPPERPANE:
            draw_pane(theme, cr, width, height);
            return;

        case SP_GRIPPER:
            draw_gripper(theme, cr, width, height);
            return;
    }

    ERR("Unknown status part %d.\n", part_id);
}

static HRESULT get_part_size(uxgtk_theme_t *theme, int part_id, int state_id,
                             RECT *rect, SIZE *size)
{
    status_theme_t *status_theme = (status_theme_t *)theme;

    switch (part_id)
    {
        case SP_GRIPPER:
            size->cx = status_theme->grip_width;
            size->cy = status_theme->grip_height;
            return S_OK;
    }

    FIXME("Unsupported status part %d.\n", part_id);
    return E_NOTIMPL;
}

static BOOL is_part_defined(int part_id, int state_id)
{
    return (part_id >= 0 && part_id <= SP_GRIPPER);
}

uxgtk_theme_t *uxgtk_status_theme_create(void)
{
    status_theme_t *theme;

    TRACE("()\n");

    theme = malloc(sizeof(status_theme_t));
    memset(theme, 0, sizeof(status_theme_t));

    uxgtk_theme_init(&theme->base, &status_vtable);

    pgtk_widget_style_get(theme->base.window,
                          "resize-grip-width", &theme->grip_width,
                          "resize-grip-height", &theme->grip_height,
                          NULL);

    TRACE("-GtkWindow-resize-grip-width: %d\n", theme->grip_width);
    TRACE("-GtkWindow-resize-grip-height: %d\n", theme->grip_height);

    return &theme->base;
}
