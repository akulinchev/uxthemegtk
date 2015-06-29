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

#include "rebar.h"

#include <vsstyle.h>

#include <wine/debug.h>

#include "window.h"

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

void uxgtk_rebar_init(GdkScreen *screen)
{
    /* Do nothing */
}

void uxgtk_rebar_uninit(void)
{
    /* Do nothing */
}

void uxgtk_rebar_draw_background(cairo_t *cr,
                                 int part_id, int state_id,
                                 int width, int height)
{
    if (part_id == 0) {
        /* I have tryed to draw a "primary" toolbar, but it looks ugly */
        uxgtk_window_draw_background(cr, WP_DIALOG, 0, width, height);
    } else {
        WINE_FIXME("Unsupported rebar part %d.\n", part_id);
    }
}

BOOL uxgtk_rebar_is_part_defined(int part_id, int state_id)
{
    return part_id == 0;
}

/* vim: set expandtab tabstop=8 shiftwidth=4 softtabstop=4: */