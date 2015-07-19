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
#include <vssym32.h>
#include <winerror.h>
#include <wine/debug.h>

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

typedef struct _edit_theme
{
    uxgtk_theme_t base;

    GtkWidget *window;
    GtkWidget *entry;
} edit_theme_t;

static HRESULT get_color(uxgtk_theme_t *theme, int part_id, int state_id,
                         int prop_id, GdkRGBA *rgba);
static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height);
static BOOL is_part_defined(int part_id, int state_id);
static void destroy(uxgtk_theme_t *theme);

static const uxgtk_theme_vtable_t edit_vtable = {
    get_color,
    draw_background,
    NULL, /* get_part_size */
    is_part_defined,
    destroy
};

static inline GtkStateFlags get_text_state_flags(int state_id)
{
    switch (state_id)
    {
        case ETS_NORMAL:
            return GTK_STATE_FLAG_NORMAL;

        case ETS_HOT:
            return GTK_STATE_FLAG_PRELIGHT;

        case ETS_SELECTED:
            return GTK_STATE_FLAG_SELECTED;

        case ETS_DISABLED:
            return GTK_STATE_FLAG_INSENSITIVE;

        case ETS_FOCUSED:
            return GTK_STATE_FLAG_FOCUSED;

        case ETS_READONLY:
            return GTK_STATE_FLAG_INSENSITIVE;

        default:
            FIXME("Unknown edit text state %d.\n", state_id);
            break;
    }

    return GTK_STATE_FLAG_NORMAL;
}

static void draw_edit_text(edit_theme_t *theme, cairo_t *cr, int state_id, int width, int height)
{
    GtkStateFlags state = get_text_state_flags(state_id);
    GtkStyleContext *context = pgtk_widget_get_style_context(theme->entry);

    pgtk_style_context_save(context);

    pgtk_style_context_set_state(context, state);

    pgtk_render_background(context, cr, 0, 0, width, height);
    pgtk_render_frame(context, cr, 0, 0, width, height);

    pgtk_style_context_restore(context);
}

static HRESULT get_color(uxgtk_theme_t *theme, int part_id, int state_id, int prop_id, GdkRGBA *rgba)
{
    edit_theme_t *edit_theme = (edit_theme_t *)theme;
    GtkStateFlags state = get_text_state_flags(state_id);
    GtkStyleContext *context = pgtk_widget_get_style_context(edit_theme->entry);

    switch (prop_id)
    {
        case TMT_FILLCOLOR:
            pgtk_style_context_add_class(context, GTK_STYLE_CLASS_VIEW);
            pgtk_style_context_get_background_color(context, state, rgba);
            pgtk_style_context_remove_class(context, GTK_STYLE_CLASS_VIEW);
            return S_OK;

        case TMT_TEXTCOLOR:
            pgtk_style_context_get_color(context, state, rgba);
            return S_OK;

        default:
            FIXME("Unsupported property %d.\n", prop_id);
            break;
    }

    return E_FAIL;
}

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height)
{
    edit_theme_t *edit_theme = (edit_theme_t *)theme;

    if (part_id == EP_EDITTEXT)
        draw_edit_text(edit_theme, cr, state_id, width, height);
    else
        FIXME("Unsupported edit part %d.\n", part_id);
}

static BOOL is_part_defined(int part_id, int state_id)
{
    /* comstl32.dll uses only EP_EDITTEXT */
    return (part_id == EP_EDITTEXT && state_id < ETS_ASSIST);
}

static void destroy(uxgtk_theme_t *theme)
{
    /* Destroy the toplevel widget */
    pgtk_widget_destroy(((edit_theme_t *)theme)->window);

    free(theme);
}

uxgtk_theme_t *uxgtk_edit_theme_create(void)
{
    edit_theme_t *theme;

    TRACE("()\n");

    theme = malloc(sizeof(edit_theme_t));
    memset(theme, 0, sizeof(edit_theme_t));

    theme->base.vtable = &edit_vtable;

    theme->window = pgtk_window_new(GTK_WINDOW_TOPLEVEL);
    theme->entry = pgtk_entry_new();

    pgtk_container_add((GtkContainer*)theme->window, theme->entry);

    return &theme->base;
}
