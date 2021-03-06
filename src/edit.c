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

#include <assert.h>
#include <stdlib.h>

#include "vsstyle.h"
#include "vssym32.h"
#include "winerror.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

typedef struct _edit_theme
{
    uxgtk_theme_t base;

    GtkWidget *entry;
} edit_theme_t;

static HRESULT get_color(uxgtk_theme_t *theme, int part_id, int state_id,
                         int prop_id, GdkRGBA *rgba);
static HRESULT draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                               int width, int height);
static BOOL is_part_defined(int part_id, int state_id);

static const uxgtk_theme_vtable_t edit_vtable = {
    get_color,
    draw_background,
    NULL, /* get_part_size */
    is_part_defined
};

static GtkStateFlags get_text_state_flags(int state_id)
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
    }

    FIXME("Unknown edit text state %d.\n", state_id);
    return GTK_STATE_FLAG_NORMAL;
}

static HRESULT get_fill_color(edit_theme_t *theme, int part_id, int state_id, GdkRGBA *rgba)
{
    GtkStateFlags state;
    GtkStyleContext *context;

    assert(theme != NULL);

    switch (part_id)
    {
        case EP_EDITTEXT:
            state = get_text_state_flags(state_id);
            context = pgtk_widget_get_style_context(theme->entry);
            break;

        default:
            FIXME("Unsupported button part %d.\n", part_id);
            return E_NOTIMPL;
    }

    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_VIEW);
    pgtk_style_context_get_background_color(context, state, rgba);
    pgtk_style_context_remove_class(context, GTK_STYLE_CLASS_VIEW);

    return S_OK;
}

static HRESULT get_text_color(edit_theme_t *theme, int part_id, int state_id, GdkRGBA *rgba)
{
    GtkStateFlags state;
    GtkStyleContext *context;

    assert(theme != NULL);

    switch (part_id)
    {
        case EP_EDITTEXT:
            state = get_text_state_flags(state_id);
            context = pgtk_widget_get_style_context(theme->entry);
            break;

        default:
            FIXME("Unsupported button part %d.\n", part_id);
            return E_NOTIMPL;
    }

    pgtk_style_context_get_color(context, state, rgba);

    return S_OK;
}

static HRESULT get_color(uxgtk_theme_t *theme, int part_id, int state_id,
                         int prop_id, GdkRGBA *rgba)
{
    edit_theme_t *edit_theme = (edit_theme_t *)theme;

    switch (prop_id)
    {
        case TMT_FILLCOLOR:
            return get_fill_color(edit_theme, part_id, state_id, rgba);

        case TMT_TEXTCOLOR:
            return get_text_color(edit_theme, part_id, state_id, rgba);
    }

    FIXME("Unsupported edit color %d.\n", prop_id);
    return E_NOTIMPL;
}

static HRESULT draw_text(edit_theme_t *theme, cairo_t *cr, int state_id, int width, int height)
{
    GtkStyleContext *context;
    GtkStateFlags state = get_text_state_flags(state_id);

    assert(theme != NULL);

    context = pgtk_widget_get_style_context(theme->entry);

    pgtk_style_context_save(context);

    pgtk_style_context_set_state(context, state);

    pgtk_render_background(context, cr, 0, 0, width, height);
    pgtk_render_frame(context, cr, 0, 0, width, height);

    pgtk_style_context_restore(context);

    return S_OK;
}

static HRESULT draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                               int width, int height)
{
    edit_theme_t *edit_theme = (edit_theme_t *)theme;

    switch (part_id)
    {
        case EP_EDITTEXT:
            return draw_text(edit_theme, cr, state_id, width, height);
    }

    FIXME("Unsupported edit part %d.\n", part_id);
    return E_NOTIMPL;
}

static BOOL is_part_defined(int part_id, int state_id)
{
    return (part_id == EP_EDITTEXT && state_id < ETS_ASSIST);
}

uxgtk_theme_t *uxgtk_edit_theme_create(void)
{
    edit_theme_t *theme;

    TRACE("()\n");

    theme = malloc(sizeof(edit_theme_t));
    memset(theme, 0, sizeof(edit_theme_t));

    uxgtk_theme_init(&theme->base, &edit_vtable);

    theme->entry = pgtk_entry_new();

    pgtk_container_add((GtkContainer *)theme->base.layout, theme->entry);

    return &theme->base;
}
