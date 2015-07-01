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

#ifndef UXTHEMEGTK_BUTTON_H
#define UXTHEMEGTK_BUTTON_H

#include <windef.h>

#include <gdk/gdk.h>

void uxgtk_button_init(GdkScreen *screen);
void uxgtk_button_uninit(void);
HRESULT uxgtk_button_get_color(int part_id, int state_id,
                               int prop_id, GdkRGBA *rgba);
void uxgtk_button_draw_background(cairo_t *cr,
                                  int part_id, int state_id,
                                  int width, int height);
HRESULT uxgtk_button_get_part_size(int part_id, int state_id,
                                   RECT *rect, SIZE *size);
BOOL uxgtk_button_is_part_defined(int part_id, int state_id);

#endif /* UXTHEMEGTK_BUTTON_H */

/* vim: set expandtab tabstop=8 shiftwidth=4 softtabstop=4: */
