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
#include "vssym32.h"
#include "winerror.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

typedef struct _menu_theme
{
    uxgtk_theme_t base;

    GtkWidget *window;
    GtkWidget *menubar;
    GtkWidget *menuitem;
    GtkWidget *menu;
} menu_theme_t;

static HRESULT get_color(uxgtk_theme_t *theme, int part_id, int state_id,
                         int prop_id, GdkRGBA *rgba);
static void destroy(uxgtk_theme_t *theme);

static const uxgtk_theme_vtable_t menu_vtable = {
    get_color,
    NULL, /* draw_background */
    NULL, /* get_part_size */
    NULL, /* is_part_defined */
    destroy
};

static GtkStateFlags get_popup_item_state_flags(int state_id)
{
    switch (state_id)
    {
        case MPI_NORMAL:
            return GTK_STATE_FLAG_NORMAL;

        case MPI_HOT:
            return GTK_STATE_FLAG_PRELIGHT;

        case MPI_DISABLED:
            return GTK_STATE_FLAG_INSENSITIVE;

        case MPI_DISABLEDHOT:
            return GTK_STATE_FLAG_INSENSITIVE | GTK_STATE_FLAG_PRELIGHT;
    }

    ERR("Unknown menu popup item state %d.\n", state_id);
    return GTK_STATE_FLAG_NORMAL;
}

static HRESULT get_fill_color(menu_theme_t *theme, int part_id, int state_id, GdkRGBA *rgba)
{
    GtkStateFlags state;
    GtkStyleContext *context;

    switch (part_id)
    {
        case MENU_BARBACKGROUND:
            state = GTK_STATE_FLAG_NORMAL;
            context = pgtk_widget_get_style_context(theme->menubar);
            break;

        case MENU_POPUPBACKGROUND:
            state = GTK_STATE_FLAG_NORMAL;
            context = pgtk_widget_get_style_context(theme->menu);
            break;

        case MENU_POPUPITEM:
            state = get_popup_item_state_flags(state_id);
            context = pgtk_widget_get_style_context(theme->menuitem);
            break;

        default:
            FIXME("Unsupported menu part %d.\n", part_id);
            return E_NOTIMPL;
    }

    pgtk_style_context_get_background_color(context, state, rgba);

    return S_OK;
}

static HRESULT get_text_color(menu_theme_t *theme, int part_id, int state_id, GdkRGBA *rgba)
{
    GtkStateFlags state;
    GtkStyleContext *context;

    switch (part_id)
    {
        case MENU_BARBACKGROUND:
            state = GTK_STATE_FLAG_NORMAL;
            context = pgtk_widget_get_style_context(theme->menubar);
            break;

        case MENU_POPUPBACKGROUND:
            state = GTK_STATE_FLAG_NORMAL;
            context = pgtk_widget_get_style_context(theme->menu);
            break;

        case MENU_POPUPITEM:
            state = get_popup_item_state_flags(state_id);
            context = pgtk_widget_get_style_context(theme->menuitem);
            break;

        default:
            FIXME("Unsupported menu part %d.\n", part_id);
            return E_NOTIMPL;
    }

    pgtk_style_context_get_color(context, state, rgba);

    return S_OK;
}

static HRESULT get_color(uxgtk_theme_t *theme, int part_id, int state_id,
                         int prop_id, GdkRGBA *rgba)
{
    menu_theme_t *menu_theme = (menu_theme_t *)theme;

    switch (prop_id)
    {
        case TMT_FILLCOLOR:
            return get_fill_color(menu_theme, part_id, state_id, rgba);

        case TMT_TEXTCOLOR:
            return get_text_color(menu_theme, part_id, state_id, rgba);
    }

    FIXME("Unsupported menu color %d.\n", prop_id);
    return E_NOTIMPL;
}

static void destroy(uxgtk_theme_t *theme)
{
    /* Destroy the toplevel widget */
    pgtk_widget_destroy(((menu_theme_t *)theme)->window);

    free(theme);
}

uxgtk_theme_t *uxgtk_menu_theme_create(void)
{
    menu_theme_t *theme;

    TRACE("()\n");

    theme = malloc(sizeof(menu_theme_t));
    memset(theme, 0, sizeof(menu_theme_t));

    theme->base.vtable = &menu_vtable;

    theme->window = pgtk_window_new(GTK_WINDOW_TOPLEVEL);
    theme->menubar = pgtk_menu_bar_new();
    theme->menuitem = pgtk_menu_item_new();
    theme->menu = pgtk_menu_new();

    pgtk_container_add((GtkContainer*)theme->window, theme->menubar);
    pgtk_menu_shell_append((GtkMenuShell*)theme->menubar, theme->menuitem);
    pgtk_menu_item_set_submenu((GtkMenuItem*)theme->menuitem, theme->menu);

    return &theme->base;
}
