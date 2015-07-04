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

static GtkWidget *window = NULL;
static GtkWidget *menubar = NULL;
static GtkWidget *menuitem = NULL;
static GtkWidget *menu = NULL;

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
            return pgtk_widget_get_style_context(menubar);

        case MENU_POPUPBACKGROUND:
            return pgtk_widget_get_style_context(menu);

        case MENU_POPUPITEM:
            return pgtk_widget_get_style_context(menuitem);

        default:
            FIXME("Unsupported menu part %d.\n", part_id);
            break;
    }

    return pgtk_widget_get_style_context(menubar);
}

void uxgtk_menu_init(void)
{
    TRACE("()\n");

    window = pgtk_window_new(GTK_WINDOW_TOPLEVEL);
    menubar = pgtk_menu_bar_new();
    menuitem = pgtk_menu_item_new();
    menu = pgtk_menu_new();

    pgtk_container_add((GtkContainer*)window, menubar);
    pgtk_menu_shell_append((GtkMenuShell*)menubar, menuitem);
    pgtk_menu_item_set_submenu((GtkMenuItem*)menuitem, menu);
}

void uxgtk_menu_uninit(void)
{
    TRACE("()\n");
    pgtk_widget_destroy(window);
}

HRESULT uxgtk_menu_get_color(int part_id, int state_id, int prop_id, GdkRGBA *rgba)
{
    GtkStyleContext *context = get_style_context(part_id);
    GtkStateFlags state = get_state_flags(part_id, state_id);

    TRACE("(%d, %d)\n",  part_id, state_id);

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
