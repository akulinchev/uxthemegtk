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

static GtkStyleContext *context = NULL;

void uxgtk_listbox_init(GdkScreen *screen)
{
    GtkWidgetPath *path;
    int pos;

    TRACE("(%p)\n", screen);

    path = pgtk_widget_path_new();
    pos = pgtk_widget_path_append_type(path, pgtk_scrolled_window_get_type());

    pgtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_VIEW);
    pgtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_FRAME);

    context = pgtk_style_context_new();
    pgtk_style_context_set_path(context, path);
    pgtk_style_context_set_screen(context, screen);
}

void uxgtk_listbox_uninit(void)
{
    TRACE("()\n");
    pg_object_unref(context);
}

void uxgtk_listbox_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height)
{
    TRACE("(%p, %d, %d, %d, %d)\n", cr, part_id, state_id, width, height);

    switch (part_id) {
        case 0:
        case LBCP_BORDER_HSCROLL:
        case LBCP_BORDER_HVSCROLL:
        case LBCP_BORDER_NOSCROLL:
        case LBCP_BORDER_VSCROLL:
            pgtk_render_background(context, cr, 0, 0, width, height);
            pgtk_render_frame(context, cr, 0, 0, width, height);
            break;

        default:
            FIXME("Unsupported listbox part %d.\n", part_id);
            break;
    }
}

BOOL uxgtk_listbox_is_part_defined(int part_id, int state_id)
{
    TRACE("(%d, %d)\n", part_id, state_id);

    /* >= 0 because comctl32.dll always sends 0. LBCP_ITEM is not used. */
    return (part_id >= 0 && part_id < LBCP_ITEM);
}
