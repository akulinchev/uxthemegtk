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

#ifndef UXTHEMEGTK_STATUS_H
#define UXTHEMEGTK_STATUS_H

#include <windef.h>

#include <gdk/gdk.h>

void uxgtk_status_init(GdkScreen *screen);
void uxgtk_status_uninit(void);
void uxgtk_status_draw_background(cairo_t *cr,
                                  int part_id, int state_id,
                                  int width, int height);
HRESULT uxgtk_status_get_part_size(int part_id, int state_id,
                                   RECT *rect, SIZE *size);
BOOL uxgtk_status_is_part_defined(int part_id, int state_id);

#endif /* UXTHEMEGTK_STATUS_H */

/* vim: set expandtab tabstop=8 shiftwidth=4 softtabstop=4: */