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

typedef struct _header_theme
{
    uxgtk_theme_t base;

    GtkWidget *treeview;
} header_theme_t;

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height);
static BOOL is_part_defined(int part_id, int state_id);
static void destroy(uxgtk_theme_t *theme);

static const uxgtk_theme_vtable_t header_vtable = {
    NULL, /* get_color */
    draw_background,
    NULL, /* get_part_size */
    is_part_defined,
    destroy
};

static void draw_item(header_theme_t *theme, cairo_t *cr, int state_id, int width, int height)
{
    GtkWidget *widget = pgtk_tree_view_column_get_button(
        pgtk_tree_view_get_column((GtkTreeView*)theme->treeview, 1));
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

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height)
{
    header_theme_t *header_theme = (header_theme_t *)theme;

    switch (part_id)
    {
        case HP_HEADERITEM:
            draw_item(header_theme, cr, state_id, width, height);
            return;
    }

    FIXME("Unsupported header part %d.\n", part_id);
}

static BOOL is_part_defined(int part_id, int state_id)
{
    return (part_id == HP_HEADERITEM);
}

static void destroy(uxgtk_theme_t *theme)
{
    pgtk_widget_destroy(((header_theme_t *)theme)->treeview);

    free(theme);
}

uxgtk_theme_t *uxgtk_header_theme_create(void)
{
    header_theme_t *theme;

    TRACE("()\n");

    theme = malloc(sizeof(header_theme_t));
    memset(theme, 0, sizeof(header_theme_t));

    theme->base.vtable = &header_vtable;

    theme->treeview = pgtk_tree_view_new();

    pgtk_tree_view_append_column((GtkTreeView*)theme->treeview, pgtk_tree_view_column_new());
    pgtk_tree_view_append_column((GtkTreeView*)theme->treeview, pgtk_tree_view_column_new());
    pgtk_tree_view_append_column((GtkTreeView*)theme->treeview, pgtk_tree_view_column_new());

    return &theme->base;
}
