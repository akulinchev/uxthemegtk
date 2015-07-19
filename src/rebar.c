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

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

typedef struct _rebar_theme
{
    uxgtk_theme_t base;

    GtkWidget *window;
    GtkWidget *toolbar;
} rebar_theme_t;

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height);
static BOOL is_part_defined(int part_id, int state_id);
static void destroy(uxgtk_theme_t *theme);

static const uxgtk_theme_vtable_t rebar_vtable = {
    NULL, /* get_color */
    draw_background,
    NULL, /* get_part_size */
    is_part_defined,
    destroy
};

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height)
{
    rebar_theme_t *rebar_theme = (rebar_theme_t *)theme;

    switch (part_id)
    {
        case 0:
        case RP_BACKGROUND:
        {
            GtkStyleContext *context = pgtk_widget_get_style_context(rebar_theme->toolbar);
            pgtk_render_background(context, cr, 0, 0, width, height);
            return;
        }
    }

    FIXME("Unsupported rebar part %d.\n", part_id);
}

static BOOL is_part_defined(int part_id, int state_id)
{
    return (part_id == 0 || part_id == RP_BACKGROUND);
}

static void destroy(uxgtk_theme_t *theme)
{
    /* Destroy the toplevel widget */
    pgtk_widget_destroy(((rebar_theme_t *)theme)->window);

    free(theme);
}

uxgtk_theme_t *uxgtk_rebar_theme_create(void)
{
    rebar_theme_t *theme;

    TRACE("()\n");

    theme = malloc(sizeof(rebar_theme_t));
    memset(theme, 0, sizeof(rebar_theme_t));

    theme->base.vtable = &rebar_vtable;

    theme->window = pgtk_window_new(GTK_WINDOW_TOPLEVEL);
    theme->toolbar = pgtk_toolbar_new();

    pgtk_container_add((GtkContainer*)theme->window, theme->toolbar);

    return &theme->base;
}
