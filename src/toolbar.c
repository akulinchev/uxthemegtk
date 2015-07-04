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
#include <wine/debug.h>

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

static GtkWidget *separator = NULL;

static void draw_button(cairo_t *cr, int state_id, int width, int height)
{
    if (state_id == TS_HOT)
        uxgtk_button_draw_background(cr, BP_PUSHBUTTON, PBS_HOT, width, height);
    else if (state_id == TS_PRESSED)
        uxgtk_button_draw_background(cr, BP_PUSHBUTTON, PBS_PRESSED, width, height);
}

static void draw_separator(cairo_t *cr, int part_id, int width, int height)
{
    int x1, x2, y1, y2;
    GtkStyleContext *context = pgtk_widget_get_style_context(separator);

    if (part_id == TP_SEPARATOR) /* TP_SEPARATORVERT ? */
    {
        y1 = 0;
        y2 = height;
        x1 = x2 = width/2;
    }
    else
    {
        x1 = 0;
        x2 = width;
        y1 = y2 = height/2;
    }

    pgtk_render_line(context, cr, x1, y1, x2, y2);
}

void uxgtk_toolbar_init(void)
{
    TRACE("()\n");
    separator = (GtkWidget*)pgtk_separator_tool_item_new();
}

void uxgtk_toolbar_uninit(void)
{
    TRACE("()\n");
    pgtk_widget_destroy(separator);
}

void uxgtk_toolbar_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height)
{
    TRACE("(%p, %d, %d, %d, %d)\n", cr, part_id, state_id, width, height);

    switch (part_id)
    {
        case TP_BUTTON:
            draw_button(cr, state_id, width, height);
            break;

        case TP_SEPARATOR:
        case TP_SEPARATORVERT:
            draw_separator(cr, part_id, width, height);
            break;

        default:
            FIXME("Unknown toolbar part %d.\n", part_id);
            break;
    }
}

BOOL uxgtk_toolbar_is_part_defined(int part_id, int state_id)
{
    TRACE("(%d, %d)\n", part_id, state_id);
    return (part_id == TP_BUTTON || part_id == TP_SEPARATOR || part_id == TP_SEPARATORVERT);
}
