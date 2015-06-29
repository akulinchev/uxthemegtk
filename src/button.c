/*
 * This file is part of UxThemeGTK.
 *
 * Copyright © 2015 Ivan Akulinchev
 *
 * UxThemeGTK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * UxThemeGTK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with UxThemeGTK.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "button.h"

#include <vsstyle.h>
#include <vssym32.h>
#include <winerror.h>

#include <wine/debug.h>

#include <gtk/gtk.h>

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

static int indicator_size = 0;

static GtkStyleContext *window_context = NULL;
static GtkStyleContext *button_context = NULL;
static GtkStyleContext *check_context = NULL;
static GtkStyleContext *radio_context = NULL;
static GtkStyleContext *label_context = NULL;

static inline GtkStateFlags get_push_button_state_flags(int state_id)
{
    switch (state_id) {
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
        WINE_FIXME("Unsupported push button state %d.\n", state_id);
    }

    return GTK_STATE_FLAG_NORMAL;
}

static inline GtkStateFlags get_radio_button_state_flags(int state_id)
{
    switch (state_id) {
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
        WINE_FIXME("Unsupported radio button state %d.\n", state_id);
    }

    return GTK_STATE_FLAG_NORMAL;
}

static inline GtkStateFlags get_checkbox_state_flags(int state_id)
{
    switch (state_id) {
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
        WINE_ERR("Unsupported checkbox state %d.\n", state_id);
    }

    return GTK_STATE_FLAG_NORMAL;
}

static inline GtkStateFlags get_groupbox_state_flags(int state_id)
{
    switch (state_id) {
    case GBS_NORMAL:
        return GTK_STATE_FLAG_NORMAL;

    case GBS_DISABLED:
        return GTK_STATE_FLAG_INSENSITIVE;

    default:
        WINE_FIXME("Unknown groupbox state %d.\n", state_id);
    }

    return GTK_STATE_FLAG_NORMAL;
}

static void get_border_color(GdkRGBA *rgba)
{
    gtk_style_context_save(button_context);

    /* Most of the themes draw a button border as an image, so we can't get
     * its color using the "border-color" style property. Try to add the "frame"
     * class, which is usually a real border. The "button" class can remove this
     * borders, so we remove it first.
     */
    gtk_style_context_remove_class(button_context, GTK_STYLE_CLASS_BUTTON);
    gtk_style_context_add_class(button_context, GTK_STYLE_CLASS_FRAME);
    gtk_style_context_get_border_color(button_context, GTK_STATE_FLAG_NORMAL, rgba);

    /* Fail. Render our frame in a small surface and try to get its color again. */
    if (rgba->alpha <= 0) {
        unsigned char *data;
        cairo_surface_t *surface =
            cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 10, 10);
        cairo_t *cr = cairo_create(surface);

        gtk_render_frame(button_context, cr, 0, 0, 10, 10);

        cairo_surface_flush(surface);
        data = cairo_image_surface_get_data(surface);

        /* x = 0, y = 4 */
        rgba->red = data[160] / 255.0;
        rgba->green = data[161] / 255.0;
        rgba->blue = data[162] / 255.0;
        rgba->alpha = data[163] / 255.0;

        cairo_destroy(cr);
        cairo_surface_destroy(surface);
    }

    gtk_style_context_restore(button_context);
}

static void get_text_color(int part_id, int state_id, GdkRGBA *rgba)
{
    GtkStateFlags state = 0;

    switch (part_id) {
    case BP_PUSHBUTTON:
        state = get_push_button_state_flags(state_id);
        /* GtkWindow.background GtkButton.button GtkLabel.label */
        gtk_style_context_set_parent(label_context, button_context);
        break;

    case BP_RADIOBUTTON:
        state = get_radio_button_state_flags(state_id);
        /* GtkWindow.background GtkRadioButton.radio GtkLabel.label */
        gtk_style_context_set_parent(label_context, radio_context);
        break;

    case BP_CHECKBOX:
        state = get_checkbox_state_flags(state_id);
        /* GtkWindow.background GtkCheckButton.check GtkLabel.label */
        gtk_style_context_set_parent(label_context, check_context);
        break;

    case BP_GROUPBOX:
        state = get_groupbox_state_flags(state_id);
        /* GtkWindow.background GtkLabel.label */
        gtk_style_context_set_parent(label_context, window_context);
        break;

    default:
        WINE_FIXME("Unsupported button part %d.\n", part_id);
    }

    gtk_style_context_get_color(label_context, state, rgba);

    /* GtkLabel.label */
    gtk_style_context_set_parent(label_context, NULL);
}

static void draw_push_button(cairo_t *cr, int state_id, int width, int height)
{
    GtkStateFlags state = get_push_button_state_flags(state_id);

    gtk_style_context_save(button_context);

    gtk_style_context_set_state(button_context, state);

    gtk_render_background(button_context, cr, 0, 0, width, height);
    gtk_render_frame(button_context, cr, 0, 0, width, height);

    gtk_style_context_restore(button_context);
}

static void draw_radio_button(cairo_t *cr, int state_id)
{
    GtkStateFlags state = get_radio_button_state_flags(state_id);

    gtk_style_context_save(radio_context);

    gtk_style_context_set_state(radio_context, state);

    gtk_render_option(radio_context, cr, 0, 0, indicator_size, indicator_size);

    gtk_style_context_restore(radio_context);
}

static void draw_checkbox(cairo_t *cr, int state_id)
{
    GtkStateFlags state = get_checkbox_state_flags(state_id);

    gtk_style_context_save(check_context);

    gtk_style_context_set_state(check_context, state);

    gtk_render_check(check_context, cr, 0, 0, indicator_size, indicator_size);

    gtk_style_context_restore(check_context);
}

void uxgtk_button_init(GdkScreen *screen)
{
    GtkWidgetPath *path = NULL;
    int pos = 0;

    /* GtkWindow.background */
    path = gtk_widget_path_new();
    pos = gtk_widget_path_append_type(path, GTK_TYPE_WINDOW);
    gtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_BACKGROUND);
    window_context = gtk_style_context_new();
    gtk_style_context_set_path(window_context, path);
    gtk_style_context_set_screen(window_context, screen);

    /* GtkWindow.background GtkButton.button */
    path = gtk_widget_path_new();
    pos = gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
    gtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_BUTTON);
    button_context = gtk_style_context_new();
    gtk_style_context_set_path(button_context, path);
    gtk_style_context_set_screen(button_context, screen);
    gtk_style_context_set_parent(button_context, window_context);

    /* GtkWindow.background GtkCheckButton.check */
    path = gtk_widget_path_new();
    pos = gtk_widget_path_append_type(path, GTK_TYPE_CHECK_BUTTON);
    gtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_CHECK);
    check_context = gtk_style_context_new();
    gtk_style_context_set_path(check_context, path);
    gtk_style_context_set_screen(check_context, screen);
    gtk_style_context_set_parent(check_context, window_context);

    /* GtkWindow.background GtkRadioButton.radio */
    path = gtk_widget_path_new();
    pos = gtk_widget_path_append_type(path, GTK_TYPE_RADIO_BUTTON);
    gtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_RADIO);
    radio_context = gtk_style_context_new();
    gtk_style_context_set_path(radio_context, path);
    gtk_style_context_set_screen(radio_context, screen);
    gtk_style_context_set_parent(radio_context, window_context);

    /* GtkLabel.label */
    path = gtk_widget_path_new();
    gtk_widget_path_append_type(path, GTK_TYPE_LABEL);
    label_context = gtk_style_context_new();
    gtk_style_context_set_path(label_context, path);
    gtk_style_context_set_screen(label_context, screen);

    /* Used for both check- and radiobuttons */
    gtk_style_context_get_style(check_context,
                                "indicator-size", &indicator_size,
                                NULL);

    WINE_TRACE("-GtkCheckButton-indicator-size: %d;\n", indicator_size);
}

void uxgtk_button_uninit(void)
{
    g_object_unref(label_context);
    gtk_style_context_set_parent(radio_context, NULL);
    g_object_unref(radio_context);
    gtk_style_context_set_parent(check_context, NULL);
    g_object_unref(check_context);
    gtk_style_context_set_parent(button_context, NULL);
    g_object_unref(button_context);
    g_object_unref(window_context);
}

HRESULT uxgtk_button_get_color(int part_id, int state_id,
                               int prop_id, GdkRGBA *rgba)
{
    switch (prop_id) {
    case TMT_BORDERCOLOR:
        get_border_color(rgba);
        break;

    case TMT_TEXTCOLOR:
        get_text_color(part_id, state_id, rgba);
        break;

    default:
        WINE_FIXME("Unsupported property %d.\n", prop_id);
        return E_FAIL;
    }

    return S_OK;
}

void uxgtk_button_draw_background(cairo_t *cr,
                                  int part_id, int state_id,
                                  int width, int height)
{
    switch (part_id) {
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
        WINE_FIXME("Unsupported button part %d.\n", part_id);
    }
}

HRESULT uxgtk_button_get_part_size(int part_id, int state_id,
                                   RECT *rect, SIZE *size)
{
    switch (part_id) {
    case BP_CHECKBOX:
    case BP_RADIOBUTTON:
        size->cx = indicator_size;
        size->cy = indicator_size;
        return S_OK;

    default:
        WINE_FIXME("Unsupported button part %d.\n", part_id);
    }

    return E_FAIL;
}

BOOL uxgtk_button_is_part_defined(int part_id, int state_id)
{
    /* Some undocumented checkbox states are not supported */
    if (part_id == BP_CHECKBOX)
        return state_id < CBS_IMPLICITNORMAL;

    /* Command buttons are not implemented */
    return part_id > 0 && part_id < BP_COMMANDLINK;
}

/* vim: set expandtab tabstop=8 shiftwidth=4 softtabstop=4: */
