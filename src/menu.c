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

static GtkStyleContext *menu_context = NULL;
static GtkStyleContext *menubar_context = NULL;
static GtkStyleContext *menuitem_context = NULL;
static GtkStyleContext *window_context = NULL;

static inline GtkStateFlags get_popup_item_state_flags(int state_id)
{
    switch (state_id)
    {
        case MPI_NORMAL:
            return GTK_STATE_FLAG_NORMAL;

        case MPI_HOT:
            return GTK_STATE_FLAG_PRELIGHT;

        case MPI_DISABLED:
            return GTK_STATE_FLAG_INSENSITIVE;

        case MPI_DISABLEDHOT: /* WAT??? */
            return GTK_STATE_FLAG_INSENSITIVE | GTK_STATE_FLAG_PRELIGHT;

        default:
            FIXME("Unknown popup item state %d.\n", state_id);
            break;
    }

    return GTK_STATE_FLAG_NORMAL;
}

static inline GtkStateFlags get_state_flags(int part_id, int state_id)
{
    switch (part_id)
    {
        case MENU_BARBACKGROUND:
        case MENU_POPUPBACKGROUND:
            return GTK_STATE_FLAG_NORMAL;

        case MENU_POPUPITEM:
            return get_popup_item_state_flags(state_id);

        default:
            FIXME("Unknown menu part %d.\n", part_id);
            break;
    }

    return GTK_STATE_FLAG_NORMAL;
}

static inline GtkStyleContext *get_style_context(int part_id)
{
    switch (part_id)
    {
        case MENU_BARBACKGROUND:
            return menubar_context;

        case MENU_POPUPBACKGROUND:
            return menu_context;

        case MENU_POPUPITEM:
            return menuitem_context;

        default:
            FIXME("Unknown menu part %d.\n", part_id);
            break;
    }

    return menubar_context;
}

void uxgtk_menu_init(GdkScreen *screen)
{
    GtkWidgetPath *path = NULL;
    int pos = 0;

    TRACE("(%p)\n", screen);

    /* GtkWindow.background */
    path = pgtk_widget_path_new();
    pos = pgtk_widget_path_append_type(path, pgtk_window_get_type());
    pgtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_BACKGROUND);
    window_context = pgtk_style_context_new();
    pgtk_style_context_set_path(window_context, path);
    pgtk_style_context_set_screen(window_context, screen);

    /* GtkWindow.background GtkMenuBar.menubar */
    path = pgtk_widget_path_new();
    pos = pgtk_widget_path_append_type(path, pgtk_menu_bar_get_type());
    pgtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_MENUBAR);
    menubar_context = pgtk_style_context_new();
    pgtk_style_context_set_path(menubar_context, path);
    pgtk_style_context_set_screen(menubar_context, screen);
    pgtk_style_context_set_parent(menubar_context, window_context);

    /* GtkWindow.background GtkMenuBar.menubar GtkMenu.menu */
    path = pgtk_widget_path_new();
    pos = pgtk_widget_path_append_type(path, pgtk_menu_get_type());
    pgtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_MENU);
    menu_context = pgtk_style_context_new();
    pgtk_style_context_set_path(menu_context, path);
    pgtk_style_context_set_screen(menu_context, screen);
    pgtk_style_context_set_parent(menu_context, menubar_context);

    /* GtkWindow.background GtkMenuBar.menubar GtkMenu.menu GtkMenuItem.menuitem */
    path = pgtk_widget_path_new();
    pos = pgtk_widget_path_append_type(path, pgtk_menu_item_get_type());
    pgtk_widget_path_iter_add_class(path, pos, GTK_STYLE_CLASS_MENUITEM);
    menuitem_context = pgtk_style_context_new();
    pgtk_style_context_set_path(menuitem_context, path);
    pgtk_style_context_set_screen(menuitem_context, screen);
    pgtk_style_context_set_parent(menuitem_context, menu_context);
}

void uxgtk_menu_uninit(void)
{
    TRACE("()\n");

    pgtk_style_context_set_parent(menuitem_context, NULL);
    pg_object_unref(menuitem_context);
    pgtk_style_context_set_parent(menu_context, NULL);
    pg_object_unref(menu_context);
    pgtk_style_context_set_parent(menubar_context, NULL);
    pg_object_unref(menubar_context);
    pg_object_unref(window_context);
}

HRESULT uxgtk_menu_get_color(int part_id, int state_id, int prop_id, GdkRGBA *rgba)
{
    GtkStyleContext *context = get_style_context(part_id);
    GtkStateFlags state = get_state_flags(part_id, state_id);

    TRACE("(%d, %d, %d, %p)\n", part_id, state_id, prop_id, rgba);

    switch (prop_id)
    {
        case TMT_FILLCOLOR:
            pgtk_style_context_get_background_color(context, state, rgba);
            break;

        case TMT_TEXTCOLOR:
            pgtk_style_context_get_color(context, state, rgba);
            break;

        default:
            FIXME("Unsupported property %d.\n", prop_id);
            return E_FAIL;
    }

    return S_OK;
}
