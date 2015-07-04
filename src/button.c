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

static int indicator_size = 0;

static GtkWidget *window = NULL;
static GtkWidget *fixed = NULL;
static GtkWidget *button = NULL;
static GtkWidget *check = NULL;
static GtkWidget *radio = NULL;
static GtkWidget *frame = NULL;
static GtkWidget *label = NULL;
static GtkWidget *button_label = NULL;
static GtkWidget *check_label = NULL;
static GtkWidget *radio_label = NULL;

static inline GtkStateFlags get_push_button_state_flags(int state_id)
{
    switch (state_id)
    {
        case PBS_NORMAL:
            return GTK_STATE_FLAG_NORMAL;

        case PBS_HOT:
            return GTK_STATE_FLAG_PRELIGHT;

        case PBS_PRESSED:
            return GTK_STATE_FLAG_ACTIVE;

        case PBS_DISABLED:
            return GTK_STATE_FLAG_INSENSITIVE;

        case PBS_DEFAULTED:
            return GTK_STATE_FLAG_FOCUSED; /* TODO: GTK_STYLE_CLASS_DEFAULT */

        default:
            FIXME("Unsupported push button state %d.\n", state_id);
            break;
    }

    return GTK_STATE_FLAG_NORMAL;
}

static inline GtkStateFlags get_radio_button_state_flags(int state_id)
{
    switch (state_id)
    {
        case RBS_UNCHECKEDNORMAL:
            return GTK_STATE_FLAG_NORMAL;

        case RBS_UNCHECKEDHOT:
            return GTK_STATE_FLAG_PRELIGHT;

        case RBS_UNCHECKEDPRESSED:
            return GTK_STATE_FLAG_ACTIVE;

        case RBS_UNCHECKEDDISABLED:
            return GTK_STATE_FLAG_INSENSITIVE;

        case RBS_CHECKEDNORMAL:
            return GTK_STATE_FLAG_NORMAL | GTK_STATE_FLAG_ACTIVE;

        case RBS_CHECKEDHOT:
            return GTK_STATE_FLAG_PRELIGHT | GTK_STATE_FLAG_ACTIVE;

        case RBS_CHECKEDPRESSED:
            return GTK_STATE_FLAG_ACTIVE;

        case RBS_CHECKEDDISABLED:
            return GTK_STATE_FLAG_INSENSITIVE | GTK_STATE_FLAG_ACTIVE;

        default:
            FIXME("Unsupported radio button state %d.\n", state_id);
            break;
    }

    return GTK_STATE_FLAG_NORMAL;
}

static inline GtkStateFlags get_checkbox_state_flags(int state_id)
{
    switch (state_id)
    {
        case CBS_UNCHECKEDNORMAL:
            return GTK_STATE_FLAG_NORMAL;

        case CBS_UNCHECKEDHOT:
            return GTK_STATE_FLAG_PRELIGHT;

        case CBS_UNCHECKEDPRESSED:
            return GTK_STATE_FLAG_SELECTED;

        case CBS_UNCHECKEDDISABLED:
            return GTK_STATE_FLAG_INSENSITIVE;

        case CBS_CHECKEDNORMAL:
            return GTK_STATE_FLAG_NORMAL | GTK_STATE_FLAG_ACTIVE;

        case CBS_CHECKEDHOT:
            return GTK_STATE_FLAG_PRELIGHT | GTK_STATE_FLAG_ACTIVE;

        case CBS_CHECKEDPRESSED:
            return GTK_STATE_FLAG_SELECTED | GTK_STATE_FLAG_ACTIVE;

        case CBS_CHECKEDDISABLED:
            return GTK_STATE_FLAG_INSENSITIVE | GTK_STATE_FLAG_ACTIVE;

        case CBS_MIXEDNORMAL:
            return GTK_STATE_FLAG_NORMAL | GTK_STATE_FLAG_INCONSISTENT;

        case CBS_MIXEDHOT:
            return GTK_STATE_FLAG_PRELIGHT | GTK_STATE_FLAG_INCONSISTENT;

        case CBS_MIXEDPRESSED:
            return GTK_STATE_FLAG_ACTIVE | GTK_STATE_FLAG_INCONSISTENT;

        case CBS_MIXEDDISABLED:
            return GTK_STATE_FLAG_INSENSITIVE | GTK_STATE_FLAG_INCONSISTENT;

        default:
            ERR("Unsupported checkbox state %d.\n", state_id);
            break;
    }

    return GTK_STATE_FLAG_NORMAL;
}

static inline GtkStateFlags get_groupbox_state_flags(int state_id)
{
    switch (state_id)
    {
        case GBS_NORMAL:
            return GTK_STATE_FLAG_NORMAL;

        case GBS_DISABLED:
            return GTK_STATE_FLAG_INSENSITIVE;

        default:
            FIXME("Unknown groupbox state %d.\n", state_id);
            break;
    }

    return GTK_STATE_FLAG_NORMAL;
}

static void get_border_color(int part_id, int state_id, GdkRGBA *rgba)
{
    GtkStyleContext *context = pgtk_widget_get_style_context(frame);
    GtkStateFlags state = 0;

    pgtk_style_context_save(context);

    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_FRAME);

    switch (part_id)
    {
        case BP_PUSHBUTTON:
            state = get_push_button_state_flags(state_id);
            break;

        case BP_RADIOBUTTON:
            state = get_radio_button_state_flags(state_id);
            break;

        case BP_CHECKBOX:
            state = get_checkbox_state_flags(state_id);
            break;

        case BP_GROUPBOX:
            state = get_groupbox_state_flags(state_id);
            break;

        default:
            FIXME("Unsupported button part %d.\n", part_id);
            break;
    }

    pgtk_style_context_get_border_color(context, state, rgba);

    pgtk_style_context_restore(context);
}

static void get_text_color(int part_id, int state_id, GdkRGBA *rgba)
{
    GtkStyleContext *context = NULL;
    GtkStateFlags state = 0;

    switch (part_id)
    {
        case BP_PUSHBUTTON:
            context = pgtk_widget_get_style_context(button_label);
            state = get_push_button_state_flags(state_id);
            break;

        case BP_RADIOBUTTON:
            context = pgtk_widget_get_style_context(radio_label);
            state = get_radio_button_state_flags(state_id);
            break;

        case BP_CHECKBOX:
            context = pgtk_widget_get_style_context(check_label);
            state = get_checkbox_state_flags(state_id);
            break;

        case BP_GROUPBOX:
            context = pgtk_widget_get_style_context(label);
            state = get_groupbox_state_flags(state_id);
            break;

        default:
            FIXME("Unsupported button part %d.\n", part_id);
            break;
    }

    pgtk_style_context_get_color(context, state, rgba);
}

static void draw_push_button(cairo_t *cr, int state_id, int width, int height)
{
    GtkStyleContext *context = pgtk_widget_get_style_context(button);
    GtkStateFlags state = get_push_button_state_flags(state_id);

    pgtk_style_context_save(context);

    pgtk_style_context_set_state(context, state);

    if (state == GTK_STATE_FLAG_FOCUSED)
        pgtk_style_context_add_class(context, GTK_STYLE_CLASS_DEFAULT);

    pgtk_render_background(context, cr, 0, 0, width, height);
    pgtk_render_frame(context, cr, 0, 0, width, height);

    pgtk_style_context_restore(context);
}

static void draw_radio_button(cairo_t *cr, int state_id)
{
    GtkStyleContext *context = pgtk_widget_get_style_context(radio);
    GtkStateFlags state = get_radio_button_state_flags(state_id);

    pgtk_style_context_save(context);

    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_RADIO);

    pgtk_style_context_set_state(context, state);

    pgtk_render_option(context, cr, 0, 0, indicator_size, indicator_size);

    pgtk_style_context_restore(context);
}

static void draw_checkbox(cairo_t *cr, int state_id)
{
    GtkStyleContext *context = pgtk_widget_get_style_context(check);
    GtkStateFlags state = get_checkbox_state_flags(state_id);

    pgtk_style_context_save(context);

    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_CHECK);

    pgtk_style_context_set_state(context, state);

    pgtk_render_check(context, cr, 0, 0, indicator_size, indicator_size);

    pgtk_style_context_restore(context);
}

void uxgtk_button_init(void)
{
    TRACE("()\n");

    window = pgtk_window_new(GTK_WINDOW_TOPLEVEL);
    fixed = pgtk_fixed_new();
    button = pgtk_button_new();
    check = pgtk_check_button_new();
    radio = pgtk_radio_button_new(NULL);
    frame = pgtk_frame_new(NULL);
    label = pgtk_label_new(NULL);
    button_label = pgtk_label_new(NULL);
    check_label = pgtk_label_new(NULL);
    radio_label = pgtk_label_new(NULL);

    pgtk_container_add((GtkContainer*)window, fixed);
    pgtk_container_add((GtkContainer*)fixed, button);
    pgtk_container_add((GtkContainer*)fixed, check);
    pgtk_container_add((GtkContainer*)fixed, radio);
    pgtk_container_add((GtkContainer*)fixed, frame);
    pgtk_container_add((GtkContainer*)fixed, label);
    pgtk_container_add((GtkContainer*)button, button_label);
    pgtk_container_add((GtkContainer*)check, check_label);
    pgtk_container_add((GtkContainer*)radio, radio_label);

    /* Used for both check- and radiobuttons */
    pgtk_widget_style_get(check, "indicator-size", &indicator_size, NULL);

    TRACE("-GtkCheckButton-indicator-size: %d\n", indicator_size);
}

void uxgtk_button_uninit(void)
{
    TRACE("()\n");
    pgtk_widget_destroy(window);
}

HRESULT uxgtk_button_get_color(int part_id, int state_id, int prop_id, GdkRGBA *rgba)
{
    TRACE("(%d, %d, %d, %p)\n", part_id, state_id, prop_id, rgba);

    switch (prop_id)
    {
        case TMT_BORDERCOLOR:
            get_border_color(part_id, state_id, rgba);
            break;

        case TMT_TEXTCOLOR:
            get_text_color(part_id, state_id, rgba);
            break;

        default:
            FIXME("Unsupported property %d.\n", prop_id);
            return E_FAIL;
    }

    return S_OK;
}

void uxgtk_button_draw_background(cairo_t *cr, int part_id, int state_id,
                                  int width, int height)
{
    TRACE("(%p, %d, %d, %d, %d)\n", cr, part_id, state_id, width, height);

    switch (part_id)
    {
        case BP_PUSHBUTTON:
            draw_push_button(cr, state_id, width, height);
            break;

        case BP_RADIOBUTTON:
            draw_radio_button(cr, state_id);
            break;

        case BP_CHECKBOX:
            draw_checkbox(cr, state_id);
            break;

        case BP_GROUPBOX:
            break; /* GNOME applications don't draw a group box */

        default:
            FIXME("Unsupported button part %d.\n", part_id);
            break;
    }
}

HRESULT uxgtk_button_get_part_size(int part_id, int state_id, RECT *rect, SIZE *size)
{
    TRACE("(%d, %d, %p, %p)\n", part_id, state_id, rect, size);

    switch (part_id)
    {
        case BP_CHECKBOX:
        case BP_RADIOBUTTON:
            size->cx = indicator_size;
            size->cy = indicator_size;
            return S_OK;

        default:
            FIXME("Unsupported button part %d.\n", part_id);
            break;
    }

    return E_FAIL;
}

BOOL uxgtk_button_is_part_defined(int part_id, int state_id)
{
    TRACE("(%d, %d)\n", part_id, state_id);

    /* Some undocumented checkbox states are not supported */
    if (part_id == BP_CHECKBOX)
        return (state_id < CBS_IMPLICITNORMAL);

    /* Command buttons are not implemented */
    return (part_id > 0 && part_id < BP_COMMANDLINK);
}
