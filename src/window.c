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

static GtkWidget *window = NULL;

void uxgtk_window_init(void)
{
    GtkStyleContext *context;

    TRACE("()\n");

    window = pgtk_window_new(GTK_WINDOW_TOPLEVEL);

    context = pgtk_widget_get_style_context(window);

    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_BACKGROUND);
}

void uxgtk_window_uninit(void)
{
    TRACE("()\n");
    pgtk_widget_destroy(window);
}

HRESULT uxgtk_window_get_color(int part_id, int state_id, int prop_id, GdkRGBA *rgba)
{
    GtkStyleContext *context;

    TRACE("()\n");

    context = pgtk_widget_get_style_context(window);

    switch (prop_id)
    {
        case TMT_FILLCOLOR:
            pgtk_style_context_get_background_color(context, GTK_STATE_FLAG_NORMAL, rgba);
            break;

        case TMT_TEXTCOLOR:
            pgtk_style_context_get_color(context, GTK_STATE_FLAG_NORMAL, rgba);
            break;

        default:
            FIXME("Unsupported property %d.\n", prop_id);
            return E_FAIL;
    }

    return S_OK;
}

void uxgtk_window_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height)
{
    GtkStyleContext *context;

    TRACE("(%p, %d, %d, %d, %d)\n", cr, part_id, state_id, width, height);

    context = pgtk_widget_get_style_context(window);

    if (part_id == WP_DIALOG)
        pgtk_render_background(context, cr, 0, 0, width, height);
    else
        FIXME("Unsupported window part %d.\n", part_id);
}

BOOL uxgtk_window_is_part_defined(int part_id, int state_id)
{
    TRACE("(%d, %d)\n", part_id, state_id);
    return (part_id == WP_DIALOG);
}
