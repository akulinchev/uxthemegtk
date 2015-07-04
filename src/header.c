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

static GtkWidget *treeview = NULL;

static void draw_header_item(cairo_t *cr, int state_id, int width, int height)
{
    GtkWidget *widget = pgtk_tree_view_column_get_button(
        pgtk_tree_view_get_column((GtkTreeView*)treeview, 1));
    GtkStateFlags state = GTK_STATE_FLAG_NORMAL;
    GtkStyleContext *context = pgtk_widget_get_style_context(widget);

    if (state_id == HIS_HOT)
        state = GTK_STATE_FLAG_PRELIGHT;
    else if (state_id == HIS_PRESSED)
        state = GTK_STATE_FLAG_ACTIVE;

    pgtk_style_context_save(context);

    pgtk_style_context_set_state(context, state);

    pgtk_render_background(context, cr, 0, 0, width, height);
    pgtk_render_frame(context, cr, 0, 0, width, height);

    pgtk_style_context_restore(context);
}

void uxgtk_header_init(void)
{
    TRACE("()\n");

    treeview = pgtk_tree_view_new();

    pgtk_tree_view_append_column((GtkTreeView*)treeview, pgtk_tree_view_column_new());
    pgtk_tree_view_append_column((GtkTreeView*)treeview, pgtk_tree_view_column_new());
    pgtk_tree_view_append_column((GtkTreeView*)treeview, pgtk_tree_view_column_new());
}

void uxgtk_header_uninit(void)
{
    TRACE("()\n");
    pgtk_widget_destroy(treeview);
}

void uxgtk_header_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height)
{
    TRACE("(%p, %d, %d, %d, %d)\n", cr, part_id, state_id, width, height);

    if (part_id == HP_HEADERITEM)
        draw_header_item(cr, state_id, width, height);
    else
        FIXME("Unsupported header part %d.\n", part_id);
}

BOOL uxgtk_header_is_part_defined(int part_id, int state_id)
{
    TRACE("(%d, %d)\n", part_id, state_id);

    /* comctl32.dll uses only HP_HEADERITEM */
    return (part_id == HP_HEADERITEM);
}
