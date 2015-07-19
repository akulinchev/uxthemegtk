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
#include "vssym32.h"
#include "winerror.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

typedef struct _window_theme
{
    uxgtk_theme_t base;

    GtkWidget *window;
} window_theme_t;

static HRESULT get_color(uxgtk_theme_t *theme, int part_id, int state_id,
                         int prop_id, GdkRGBA *rgba);
static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height);
static BOOL is_part_defined(int part_id, int state_id);
static void destroy(uxgtk_theme_t *theme);

static const uxgtk_theme_vtable_t window_vtable = {
    get_color,
    draw_background,
    NULL, /* get_part_size */
    is_part_defined,
    destroy
};

static HRESULT get_fill_color(window_theme_t *theme, int part_id, int state_id, GdkRGBA *rgba)
{
    GtkStateFlags state;
    GtkStyleContext *context;

    switch (part_id)
    {
        case WP_DIALOG:
            state = GTK_STATE_FLAG_NORMAL;
            context = pgtk_widget_get_style_context(theme->window);
            break;

        default:
            FIXME("Unsupported window part %d.\n", part_id);
            return E_NOTIMPL;
    }

    pgtk_style_context_get_background_color(context, state, rgba);

    return S_OK;
}

static HRESULT get_text_color(window_theme_t *theme, int part_id, int state_id, GdkRGBA *rgba)
{
    GtkStateFlags state;
    GtkStyleContext *context;

    switch (part_id)
    {
        case WP_DIALOG:
            state = GTK_STATE_FLAG_NORMAL;
            context = pgtk_widget_get_style_context(theme->window);
            break;

        default:
            FIXME("Unsupported window part %d.\n", part_id);
            return E_NOTIMPL;
    }

    pgtk_style_context_get_color(context, state, rgba);

    return S_OK;
}

static HRESULT get_color(uxgtk_theme_t *theme, int part_id, int state_id,
                         int prop_id, GdkRGBA *rgba)
{
    window_theme_t *window_theme = (window_theme_t *)theme;

    switch (prop_id)
    {
        case TMT_FILLCOLOR:
            return get_fill_color(window_theme, part_id, state_id, rgba);

        case TMT_TEXTCOLOR:
            return get_text_color(window_theme, part_id, state_id, rgba);

        default:
            FIXME("Unsupported property %d.\n", prop_id);
            return E_FAIL;
    }

    return S_OK;
}

static void draw_dialog(window_theme_t *theme, cairo_t *cr, int state_id, int width, int height)
{
    GtkStyleContext *context = pgtk_widget_get_style_context(theme->window);

    pgtk_render_background(context, cr, 0, 0, width, height);
}

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height)
{
    window_theme_t *window_theme = (window_theme_t *)theme;

    switch (part_id)
    {
        case WP_DIALOG:
            draw_dialog(window_theme, cr, state_id, width, height);
            return;
    }

    FIXME("Unsupported window part %d.\n", part_id);
}

static BOOL is_part_defined(int part_id, int state_id)
{
    return (part_id == WP_DIALOG);
}

static void destroy(uxgtk_theme_t *theme)
{
    pgtk_widget_destroy(((window_theme_t *)theme)->window);

    free(theme);
}

uxgtk_theme_t *uxgtk_window_theme_create(void)
{
    window_theme_t *theme;
    GtkStyleContext *context;

    TRACE("()\n");

    theme = malloc(sizeof(window_theme_t));
    memset(theme, 0, sizeof(window_theme_t));

    theme->base.vtable = &window_vtable;

    theme->window = pgtk_window_new(GTK_WINDOW_TOPLEVEL);

    context = pgtk_widget_get_style_context(theme->window);
    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_BACKGROUND);

    return &theme->base;
}
