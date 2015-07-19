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

typedef struct _trackbar_theme
{
    uxgtk_theme_t base;

    int slider_length;
    int slider_width;

    GtkWidget *scale;
} trackbar_theme_t;

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height);
static BOOL is_part_defined(int part_id, int state_id);
static void destroy(uxgtk_theme_t *theme);

static const uxgtk_theme_vtable_t trackbar_vtable = {
    NULL, /* get_color */
    draw_background,
    NULL, /* get_part_size */
    is_part_defined,
    destroy
};

static void draw_track(trackbar_theme_t *theme, cairo_t *cr, int part_id, int width, int height)
{
    int x1, x2, y1, y2;
    GtkStyleContext *context = pgtk_widget_get_style_context(theme->scale);

    pgtk_style_context_save(context);

    if (part_id == TKP_TRACKVERT)
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

    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_SEPARATOR);

    pgtk_render_line(context, cr, x1, y1, x2, y2);

    pgtk_style_context_restore(context);
}

static void draw_thumb(trackbar_theme_t *theme, cairo_t *cr, int state_id, int width, int height)
{
    GtkStyleContext *context = pgtk_widget_get_style_context(theme->scale);
    GtkStateFlags state = GTK_STATE_FLAG_NORMAL;

    pgtk_style_context_save(context);

    if (state_id == TUS_HOT)
        state = GTK_STATE_FLAG_PRELIGHT;
    else if (state_id == TUBS_PRESSED)
        state = GTK_STATE_FLAG_ACTIVE;

    pgtk_style_context_set_state(context, state);

    if (width > height)
        if (theme->slider_length > theme->slider_width)
            pgtk_style_context_add_class(context, GTK_STYLE_CLASS_HORIZONTAL);
        else
            pgtk_style_context_add_class(context, GTK_STYLE_CLASS_VERTICAL);
    else
        if (theme->slider_length > theme->slider_width)
            pgtk_style_context_add_class(context, GTK_STYLE_CLASS_VERTICAL);
        else
            pgtk_style_context_add_class(context, GTK_STYLE_CLASS_HORIZONTAL);

    pgtk_style_context_add_class(context, GTK_STYLE_CLASS_SLIDER);

    pgtk_render_slider(context, cr, 0, 0, theme->slider_length, theme->slider_width,
                      GTK_ORIENTATION_HORIZONTAL);

    pgtk_style_context_restore(context);
}

static void draw_background(uxgtk_theme_t *theme, cairo_t *cr, int part_id, int state_id,
                            int width, int height)
{
    trackbar_theme_t *trackbar_theme = (trackbar_theme_t *)theme;

    switch (part_id)
    {
        case TKP_THUMB:
        case TKP_THUMBBOTTOM:
        case TKP_THUMBTOP:
        case TKP_THUMBVERT:
        case TKP_THUMBLEFT:
        case TKP_THUMBRIGHT:
            draw_thumb(trackbar_theme, cr, state_id, width, height);
            break;

        case TKP_TRACK:
        case TKP_TRACKVERT:
            draw_track(trackbar_theme, cr, part_id, width, height);
            break;

        default:
            FIXME("Unsupported trackbar part %d.\n", part_id);
            break;
    }
}

static BOOL is_part_defined(int part_id, int state_id)
{
    return (part_id > 0 && part_id < TKP_TICS);
}

static void destroy(uxgtk_theme_t *theme)
{
    pgtk_widget_destroy(((trackbar_theme_t *)theme)->scale);

    free(theme);
}

uxgtk_theme_t *uxgtk_trackbar_theme_create(void)
{
    trackbar_theme_t *theme;

    TRACE("()\n");

    theme = malloc(sizeof(trackbar_theme_t));
    memset(theme, 0, sizeof(trackbar_theme_t));

    theme->base.vtable = &trackbar_vtable;

    theme->scale = pgtk_scale_new(GTK_ORIENTATION_HORIZONTAL, NULL);

    pgtk_widget_style_get(theme->scale,
                          "slider-length", &theme->slider_length,
                          "slider-width", &theme->slider_width,
                          NULL);

    TRACE("-GtkScale-slider-length: %d\n", theme->slider_length);
    TRACE("-GtkScale-slider-width: %d\n", theme->slider_width);

    return &theme->base;
}
