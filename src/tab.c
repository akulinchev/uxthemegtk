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

static int tab_overlap = 0;
static GtkWidget *notebook = NULL;

static void draw_tab_item(cairo_t *cr, int part_id, int state_id, int width, int height)
{
    int x = 0, new_width = width, new_height = height;
    GtkStyleContext *context = pgtk_widget_get_style_context(notebook);
    GtkRegionFlags region = 0;

    pgtk_style_context_save(context);

    /* Emulate the "-GtkNotebook-tab-overlap" style property */
    if (part_id == TABP_TABITEM || part_id == TABP_TABITEMRIGHTEDGE)
    {
        x = -tab_overlap;
        new_width += tab_overlap;
    }

    /* Provide GTK a little bit more information about the tab position */
    if (part_id == TABP_TABITEMLEFTEDGE || part_id == TABP_TOPTABITEMLEFTEDGE)
        region = GTK_REGION_FIRST;
    else if (part_id == TABP_TABITEMRIGHTEDGE || part_id == TABP_TOPTABITEMRIGHTEDGE)
        region = GTK_REGION_LAST;
    else if (part_id == TABP_TABITEMBOTHEDGE || part_id == TABP_TOPTABITEMBOTHEDGE)
        region = GTK_REGION_ONLY;
    pgtk_style_context_add_region(context, GTK_STYLE_REGION_TAB, region);

    /* Some themes are not friendly with the TCS_MULTILINE tabs */
    pgtk_style_context_set_junction_sides(context, GTK_JUNCTION_BOTTOM);

    /* Active tabs have their own parts */
    if (part_id > TABP_TABITEMBOTHEDGE && part_id < TABP_PANE) {
        new_height--; /* Fix the active tab height */
        pgtk_style_context_set_state(context, GTK_STATE_FLAG_ACTIVE);
    }

    pgtk_render_background(context, cr, x, 0, new_width, new_height);
    pgtk_render_frame(context, cr, x, 0, new_width, new_height);

    pgtk_style_context_restore(context);
}

static void draw_tab_pane(cairo_t *cr, int width, int height)
{
    GtkStyleContext *context = pgtk_widget_get_style_context(notebook);

    pgtk_style_context_save(context);

    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_FRAME);

    /* Make the top corners square */
    pgtk_style_context_set_junction_sides(context, GTK_JUNCTION_TOP);

    pgtk_render_background(context, cr, 0, 0, width, height);
    pgtk_render_frame(context, cr, 0, 0, width, height);

    pgtk_style_context_restore(context);
}

static void draw_tab_body(cairo_t *cr, int width, int height)
{
    GtkStyleContext *context = pgtk_widget_get_style_context(notebook);

    /* Some borders are already drawned by draw_tab_pane */
    pgtk_render_background(context, cr, -4, -4, width + 4, height + 4);
}

void uxgtk_tab_init(void)
{
    GtkStyleContext *context;

    TRACE("()\n");

    notebook = pgtk_notebook_new();

    context = pgtk_widget_get_style_context(notebook);

    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_NOTEBOOK);
    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_TOP);

    pgtk_widget_style_get(notebook, "tab-overlap", &tab_overlap, NULL);

    TRACE("-GtkNotebook-tab-overlap: %d\n", tab_overlap);
}

void uxgtk_tab_uninit(void)
{
    TRACE("()\n");
    pgtk_widget_destroy(notebook);
}

void uxgtk_tab_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height)
{
    TRACE("(%p, %d, %d, %d, %d)\n", cr, part_id, state_id, width, height);

    /* Draw a dialog background to fix some themes like Ambiance */
    uxgtk_window_draw_background(cr, WP_DIALOG, 0, width, height - 1);

    switch (part_id)
    {
        case TABP_TABITEM:
        case TABP_TABITEMLEFTEDGE:
        case TABP_TABITEMRIGHTEDGE:
        case TABP_TABITEMBOTHEDGE:
        case TABP_TOPTABITEM:
        case TABP_TOPTABITEMLEFTEDGE:
        case TABP_TOPTABITEMRIGHTEDGE:
        case TABP_TOPTABITEMBOTHEDGE:
            draw_tab_item(cr, part_id, state_id, width, height);
            break;

        case TABP_PANE:
            draw_tab_pane(cr, width, height);
            break;

        case TABP_BODY:
            draw_tab_body(cr, width, height);
            break;

        default:
            FIXME("Unsupported tab part %d.\n", part_id);
            break;
    }
}

BOOL uxgtk_tab_is_part_defined(int part_id, int state_id)
{
    TRACE("(%d, %d)\n", part_id, state_id);

    /* TABP_AEROWIZARDBODY is not used by comctl32.dll */
    return (part_id > 0 && part_id < TABP_AEROWIZARDBODY);
}
