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

typedef struct _listbox_theme
{
    uxgtk_theme_t base;

    GtkWidget *scrolled_window;
} listbox_theme_t;

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height);
static BOOL is_part_defined(int part_id, int state_id);
static void destroy(uxgtk_theme_t *theme);

static const uxgtk_theme_vtable_t listbox_vtable = {
    NULL, /* get_color */
    draw_background,
    NULL, /* get_part_size */
    is_part_defined,
    destroy
};

static void draw_border(listbox_theme_t *theme, cairo_t *cr, int width, int height)
{
    GtkStyleContext *context = pgtk_widget_get_style_context(theme->scrolled_window);

    pgtk_style_context_save(context);

    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_VIEW);
    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_FRAME);

    pgtk_render_background(context, cr, 0, 0, width, height);
    pgtk_render_frame(context, cr, 0, 0, width, height);

    pgtk_style_context_restore(context);
}

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height)
{
    listbox_theme_t *listbox_theme = (listbox_theme_t *)theme;

    switch (part_id)
    {
        case 0:
        case LBCP_BORDER_HSCROLL:
        case LBCP_BORDER_HVSCROLL:
        case LBCP_BORDER_NOSCROLL:
        case LBCP_BORDER_VSCROLL:
            draw_border(listbox_theme, cr, width, height);
            return;
    }

    FIXME("Unsupported listbox part %d.\n", part_id);
}

static BOOL is_part_defined(int part_id, int state_id)
{
    return (part_id >= 0 && part_id < LBCP_ITEM);
}

static void destroy(uxgtk_theme_t *theme)
{
    pgtk_widget_destroy(((listbox_theme_t *)theme)->scrolled_window);

    free(theme);
}

uxgtk_theme_t *uxgtk_listbox_theme_create(void)
{
    listbox_theme_t *theme;

    TRACE("()\n");

    theme = malloc(sizeof(listbox_theme_t));
    memset(theme, 0, sizeof(listbox_theme_t));

    theme->base.vtable = &listbox_vtable;

    theme->scrolled_window = pgtk_scrolled_window_new(NULL, NULL);

    return &theme->base;
}
