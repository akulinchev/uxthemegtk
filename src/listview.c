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

#include "listview.h"

#include <vsstyle.h>

#include <wine/debug.h>

#include "listbox.h"

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

void uxgtk_listview_init(GdkScreen *screen)
{
    /* Do nothing */
}

void uxgtk_listview_uninit(void)
{
    /* Do nothing */
}

void uxgtk_listview_draw_background(cairo_t *cr,
                                    int part_id, int state_id,
                                    int width, int height)
{
    if (part_id == 0)
        uxgtk_listbox_draw_background(cr, 0, 0, width, height);
    else
        WINE_FIXME("Unsupported listview part %d.\n", part_id);
}

BOOL uxgtk_listview_is_part_defined(int part_id, int state_id)
{
    /* comctl32.dll always sends 0. Other parts are currently not supported. */
    return part_id == 0;
}

/* vim: set expandtab tabstop=8 shiftwidth=4 softtabstop=4: */
