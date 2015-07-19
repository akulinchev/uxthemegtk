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
#include <wine/debug.h>

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

typedef struct _combobox_theme
{
    uxgtk_theme_t base;

    int arrow_size;
    float arrow_scaling;

    GtkWidget *combobox;
    GtkWidget *button;
    GtkWidget *entry;
    GtkWidget *arrow;
} combobox_theme_t;

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height);
static BOOL is_part_defined(int part_id, int state_id);
static void destroy(uxgtk_theme_t *theme);

static const uxgtk_theme_vtable_t combobox_vtable = {
    NULL, /* get_color */
    draw_background,
    NULL, /* get_part_size */
    is_part_defined,
    destroy
};

static inline GtkStateFlags get_border_state_flags(int state_id)
{
    switch (state_id)
    {
        case CBB_NORMAL:
            return GTK_STATE_FLAG_NORMAL;

        case CBB_HOT:
            return GTK_STATE_FLAG_PRELIGHT;

        case CBB_FOCUSED:
            return GTK_STATE_FLAG_FOCUSED;

        case CBB_DISABLED:
            return GTK_STATE_FLAG_INSENSITIVE;

        default:
            FIXME("Unknown edit text state %d.\n", state_id);
            break;
    }

    return GTK_STATE_FLAG_NORMAL;
}

static inline GtkStateFlags get_dropdown_button_state_flags(int state_id)
{
    switch (state_id)
    {
        case CBXS_NORMAL:
            return GTK_STATE_FLAG_NORMAL;

        case CBXS_HOT:
            return GTK_STATE_FLAG_PRELIGHT;

        case CBXS_PRESSED:
            return GTK_STATE_FLAG_ACTIVE;

        case CBXS_DISABLED:
            return GTK_STATE_FLAG_INSENSITIVE;

        default:
            ERR("Unknown combobox dropdown button state %d.\n", state_id);
            break;
    }

    return GTK_STATE_FLAG_NORMAL;
}

static void iter_callback(GtkWidget *widget, gpointer data)
{
    if (pg_type_check_instance_is_a((GTypeInstance*)widget, pgtk_toggle_button_get_type()))
        ((combobox_theme_t *)data)->button = widget;
}

static void draw_border(combobox_theme_t *theme, cairo_t *cr, int state_id, int width, int height)
{
    GtkStateFlags state = get_border_state_flags(state_id);
    GtkStyleContext *context = pgtk_widget_get_style_context(theme->entry);

    pgtk_style_context_save(context);

    pgtk_style_context_set_state(context, state);

    pgtk_render_background(context, cr, 0, 0, width, height);
    pgtk_render_frame(context, cr, 0, 0, width, height);

    pgtk_style_context_restore(context);
}

static void draw_dropdown_button(combobox_theme_t *theme, cairo_t *cr, int state_id,
                                 int width, int height)
{
    int arrow_x, arrow_y, arrow_width;
    GtkStateFlags state = get_dropdown_button_state_flags(state_id);
    GtkStyleContext *button_context = pgtk_widget_get_style_context(theme->button);
    GtkStyleContext *arrow_context = pgtk_widget_get_style_context(theme->arrow);

    pgtk_style_context_save(button_context);

    pgtk_style_context_set_state(button_context, state);

    /* Render it with another size to remove a gap */
    pgtk_render_background(button_context, cr, 0, -2, width + 2, height + 4);
    pgtk_render_frame(button_context, cr, 0, -2, width + 2, height + 4);

    pgtk_style_context_restore(button_context);

    pgtk_style_context_save(arrow_context);

    pgtk_style_context_set_state(arrow_context, state);

    arrow_width = theme->arrow_size * theme->arrow_scaling;

    arrow_x = (width - arrow_width + 3) / 2;
    arrow_y = (height - arrow_width) / 2;

    pgtk_render_arrow(arrow_context, cr, G_PI, arrow_x, arrow_y, arrow_width);

    pgtk_style_context_restore(arrow_context);
}

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height)
{
    combobox_theme_t *combobox_theme = (combobox_theme_t *)theme;

    switch (part_id)
    {
        case 0:
        case CP_BORDER:
            draw_border(combobox_theme, cr, state_id, width, height);
            break;

        case CP_DROPDOWNBUTTON:
            draw_dropdown_button(combobox_theme, cr, state_id, width, height);
            break;

        default:
            FIXME("Unsupported combobox part %d.\n", part_id);
            break;
    }
}

static BOOL is_part_defined(int part_id, int state_id)
{
    return (part_id == 0 || part_id == CP_DROPDOWNBUTTON);
}

static void destroy(uxgtk_theme_t *theme)
{
    combobox_theme_t *combobox_theme = (combobox_theme_t *)theme;

    /* Destroy the toplevel widgets */
    pgtk_widget_destroy(combobox_theme->combobox);
    pgtk_widget_destroy(combobox_theme->entry);

    free(theme);
}

uxgtk_theme_t *uxgtk_combobox_theme_create(void)
{
    combobox_theme_t *theme;

    TRACE("()\n");

    theme = malloc(sizeof(combobox_theme_t));
    memset(theme, 0, sizeof(combobox_theme_t));

    theme->base.vtable = &combobox_vtable;

    /* I use a simple entry because .combobox-entry has no right border sometimes */
    theme->entry = pgtk_entry_new();
    theme->combobox = pgtk_combo_box_new_with_entry();

    /* Extract button */
    pgtk_container_forall((GtkContainer*)theme->combobox, iter_callback, theme);

    theme->arrow = pgtk_bin_get_child((GtkBin*)theme->button);

    pgtk_widget_style_get(theme->combobox,
                          "arrow-size", &theme->arrow_size,
                          "arrow-scaling", &theme->arrow_scaling,
                          NULL);

    /* A workaround for old themes like Ambiance */
    if (theme->arrow_scaling == 1)
        theme->arrow_scaling = 0.6;

    TRACE("-GtkComboBox-arrow-scaling: %f\n", theme->arrow_scaling);
    TRACE("-GtkComboBox-arrow-size: %d\n", theme->arrow_size);

    return &theme->base;
}
