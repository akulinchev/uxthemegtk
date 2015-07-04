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

static float arrow_scaling = 0.0;
static int arrow_size = 0;

static GtkWidget *combobox = NULL;
static GtkWidget *button = NULL;
static GtkWidget *entry = NULL;
static GtkWidget *arrow = NULL;

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
        button = widget;
    else if (pg_type_check_instance_is_a((GTypeInstance*)widget, pgtk_entry_get_type()))
        entry = widget;
}

static void draw_border(cairo_t *cr, int state_id, int width, int height)
{
    /* I use an edit control here because .combobox-entry has no right border sometimes */
    int state = (state_id == CBB_DISABLED) ? ETS_DISABLED : ETS_NORMAL;
    uxgtk_edit_draw_background(cr, EP_EDITTEXT, state, width, height);
}

static void draw_dropdown_button(cairo_t *cr, int state_id, int width, int height)
{
    int arrow_x, arrow_y, arrow_width;
    GtkStateFlags state = get_dropdown_button_state_flags(state_id);
    GtkStyleContext *button_context = pgtk_widget_get_style_context(button);
    GtkStyleContext *arrow_context = pgtk_widget_get_style_context(arrow);

    pgtk_style_context_save(button_context);

    pgtk_style_context_set_state(button_context, state);

    /* Render it with another size to remove a gap */
    pgtk_render_background(button_context, cr, 0, -2, width + 2, height + 4);
    pgtk_render_frame(button_context, cr, 0, -2, width + 2, height + 4);

    pgtk_style_context_restore(button_context);

    pgtk_style_context_save(arrow_context);

    pgtk_style_context_set_state(arrow_context, state);

    arrow_width = arrow_size * arrow_scaling;

    arrow_x = (width - arrow_width + 3) / 2;
    arrow_y = (height - arrow_width) / 2;

    pgtk_render_arrow(arrow_context, cr, G_PI, arrow_x, arrow_y, arrow_width);

    pgtk_style_context_restore(arrow_context);
}

void uxgtk_combobox_init(void)
{
    TRACE("()\n");

    combobox = pgtk_combo_box_new_with_entry();

    /* Extract entry and button */
    pgtk_container_forall((GtkContainer*)combobox, iter_callback, NULL);

    arrow = pgtk_bin_get_child((GtkBin*)button);

    pgtk_widget_style_get(combobox,
                          "arrow-scaling", &arrow_scaling,
                          "arrow-size", &arrow_size,
                          NULL);

    if (arrow_scaling == 1)
        arrow_scaling = 0.6; /* Hello Ambiance */

    TRACE("-GtkComboBox-arrow-scaling: %f\n", arrow_scaling);
    TRACE("-GtkComboBox-arrow-size: %d\n", arrow_size);
}

void uxgtk_combobox_uninit(void)
{
    TRACE("()\n");
    pgtk_widget_destroy(combobox);
}

void uxgtk_combobox_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height)
{
    TRACE("(%p, %d, %d, %d, %d)\n", cr, part_id, state_id, width, height);

    switch (part_id)
    {
        case 0:
        case CP_BORDER:
            draw_border(cr, state_id, width, height);
            break;

        case CP_DROPDOWNBUTTON:
            draw_dropdown_button(cr, state_id, width, height);
            break;

        default:
            FIXME("Unsupported combobox part %d.\n", part_id);
            break;
    }
}

BOOL uxgtk_combobox_is_part_defined(int part_id, int state_id)
{
    TRACE("(%d, %d)\n", part_id, state_id);
    return (part_id == 0 || part_id == CP_DROPDOWNBUTTON);
}
