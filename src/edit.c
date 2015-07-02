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
#include <vssym32.h>
#include <winerror.h>
#include <wine/debug.h>

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

static GtkStyleContext *context = NULL;

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

static void draw_edit_text(cairo_t *cr, int state_id, int width, int height)
{
    GtkStateFlags state = get_text_state_flags(state_id);

    pgtk_style_context_set_state(context, state);

    pgtk_render_background(context, cr, 0, 0, width, height);
    pgtk_render_frame(context, cr, 0, 0, width, height);
}

void uxgtk_edit_init(GdkScreen *screen)
{
    GtkWidgetPath *path;
    int pos;

    TRACE("(%p)\n", screen);

    path = pgtk_widget_path_new();
    pos = pgtk_widget_path_append_type(path, pgtk_entry_get_type());

    pgtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_ENTRY);

    context = pgtk_style_context_new();
    pgtk_style_context_set_path(context, path);
    pgtk_style_context_set_screen(context, screen);
}

void uxgtk_edit_uninit(void)
{
    TRACE("()\n");
    pg_object_unref(context);
}

HRESULT uxgtk_edit_get_color(int part_id, int state_id, int prop_id, GdkRGBA *rgba)
{
    GtkStateFlags state = get_text_state_flags(state_id);

    TRACE("(%d, %d, %d, %p)\n", part_id, state_id, prop_id, rgba);

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

void uxgtk_edit_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height)
{
    TRACE("(%p, %d, %d, %d, %d)\n", cr, part_id, state_id, width, height);

    pgtk_style_context_save(context);

    if (part_id == EP_EDITTEXT)
        draw_edit_text(cr, state_id, width, height);
    else
        FIXME("Unsupported edit part %d.\n", part_id);

    pgtk_style_context_restore(context);
}

BOOL uxgtk_edit_is_part_defined(int part_id, int state_id)
{
    TRACE("(%d, %d)\n", part_id, state_id);

    /* comstl32.dll uses only EP_EDITTEXT */
    return (part_id == EP_EDITTEXT && state_id < ETS_ASSIST);
}
