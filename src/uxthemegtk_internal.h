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

#ifndef UXTHEMEGTK_INTERNAL_H
#define UXTHEMEGTK_INTERNAL_H

#include <windef.h>
#include <gtk/gtk.h>

#define MAKE_FUNCPTR(f) extern typeof(f) * p##f DECLSPEC_HIDDEN
MAKE_FUNCPTR(cairo_create);
MAKE_FUNCPTR(cairo_destroy);
MAKE_FUNCPTR(cairo_image_surface_create);
MAKE_FUNCPTR(cairo_image_surface_get_data);
MAKE_FUNCPTR(cairo_image_surface_get_stride);
MAKE_FUNCPTR(cairo_surface_destroy);
MAKE_FUNCPTR(cairo_surface_flush);
MAKE_FUNCPTR(g_type_check_instance_is_a);
MAKE_FUNCPTR(gtk_bin_get_child);
MAKE_FUNCPTR(gtk_button_new);
MAKE_FUNCPTR(gtk_check_button_new);
MAKE_FUNCPTR(gtk_combo_box_new_with_entry);
MAKE_FUNCPTR(gtk_container_add);
MAKE_FUNCPTR(gtk_container_forall);
MAKE_FUNCPTR(gtk_entry_get_type);
MAKE_FUNCPTR(gtk_entry_new);
MAKE_FUNCPTR(gtk_fixed_new);
MAKE_FUNCPTR(gtk_frame_new);
MAKE_FUNCPTR(gtk_init);
MAKE_FUNCPTR(gtk_label_new);
MAKE_FUNCPTR(gtk_menu_bar_new);
MAKE_FUNCPTR(gtk_menu_item_new);
MAKE_FUNCPTR(gtk_menu_item_set_submenu);
MAKE_FUNCPTR(gtk_menu_new);
MAKE_FUNCPTR(gtk_menu_shell_append);
MAKE_FUNCPTR(gtk_notebook_new);
MAKE_FUNCPTR(gtk_radio_button_new);
MAKE_FUNCPTR(gtk_render_arrow);
MAKE_FUNCPTR(gtk_render_background);
MAKE_FUNCPTR(gtk_render_check);
MAKE_FUNCPTR(gtk_render_frame);
MAKE_FUNCPTR(gtk_render_handle);
MAKE_FUNCPTR(gtk_render_line);
MAKE_FUNCPTR(gtk_render_option);
MAKE_FUNCPTR(gtk_render_slider);
MAKE_FUNCPTR(gtk_scale_new);
MAKE_FUNCPTR(gtk_scrolled_window_new);
MAKE_FUNCPTR(gtk_separator_tool_item_new);
MAKE_FUNCPTR(gtk_style_context_add_class);
MAKE_FUNCPTR(gtk_style_context_add_region);
MAKE_FUNCPTR(gtk_style_context_get_background_color);
MAKE_FUNCPTR(gtk_style_context_get_border_color);
MAKE_FUNCPTR(gtk_style_context_get_color);
MAKE_FUNCPTR(gtk_style_context_get_style);
MAKE_FUNCPTR(gtk_style_context_remove_class);
MAKE_FUNCPTR(gtk_style_context_restore);
MAKE_FUNCPTR(gtk_style_context_save);
MAKE_FUNCPTR(gtk_style_context_set_junction_sides);
MAKE_FUNCPTR(gtk_style_context_set_state);
MAKE_FUNCPTR(gtk_toggle_button_get_type);
MAKE_FUNCPTR(gtk_tree_view_append_column);
MAKE_FUNCPTR(gtk_tree_view_column_get_button);
MAKE_FUNCPTR(gtk_tree_view_column_new);
MAKE_FUNCPTR(gtk_tree_view_get_column);
MAKE_FUNCPTR(gtk_tree_view_new);
MAKE_FUNCPTR(gtk_widget_destroy);
MAKE_FUNCPTR(gtk_widget_get_style_context);
MAKE_FUNCPTR(gtk_widget_style_get);
MAKE_FUNCPTR(gtk_window_new);
#undef MAKE_FUNCPTR

/* Button */
void uxgtk_button_init(void);
void uxgtk_button_uninit(void);
void uxgtk_button_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height);
BOOL uxgtk_button_is_part_defined(int part_id, int state_id);
HRESULT uxgtk_button_get_color(int part_id, int state_id, int prop_id, GdkRGBA *rgba);
HRESULT uxgtk_button_get_part_size(int part_id, int state_id, RECT *rect, SIZE *size);

/* Combobox */
void uxgtk_combobox_init(void);
void uxgtk_combobox_uninit(void);
void uxgtk_combobox_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height);
BOOL uxgtk_combobox_is_part_defined(int part_id, int state_id);

/* Edit */
void uxgtk_edit_init(void);
void uxgtk_edit_uninit(void);
void uxgtk_edit_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height);
BOOL uxgtk_edit_is_part_defined(int part_id, int state_id);
HRESULT uxgtk_edit_get_color(int part_id, int state_id, int prop_id, GdkRGBA *rgba);

/* Header */
void uxgtk_header_init(void);
void uxgtk_header_uninit(void);
void uxgtk_header_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height);
BOOL uxgtk_header_is_part_defined(int part_id, int state_id);

/* Listbox */
void uxgtk_listbox_init(void);
void uxgtk_listbox_uninit(void);
void uxgtk_listbox_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height);
BOOL uxgtk_listbox_is_part_defined(int part_id, int state_id);

/* Listview */
void uxgtk_listview_init(void);
void uxgtk_listview_uninit(void);
void uxgtk_listview_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height);
BOOL uxgtk_listview_is_part_defined(int part_id, int state_id);

/* Menu */
void uxgtk_menu_init(void);
void uxgtk_menu_uninit(void);
HRESULT uxgtk_menu_get_color(int part_id, int state_id, int prop_id, GdkRGBA *rgba);

/* Rebar */
void uxgtk_rebar_init(void);
void uxgtk_rebar_uninit(void);
void uxgtk_rebar_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height);
BOOL uxgtk_rebar_is_part_defined(int part_id, int state_id);

/* Status */
void uxgtk_status_init(void);
void uxgtk_status_uninit(void);
void uxgtk_status_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height);
BOOL uxgtk_status_is_part_defined(int part_id, int state_id);
HRESULT uxgtk_status_get_part_size(int part_id, int state_id, RECT *rect, SIZE *size);

/* Tab */
void uxgtk_tab_init(void);
void uxgtk_tab_uninit(void);
void uxgtk_tab_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height);
BOOL uxgtk_tab_is_part_defined(int part_id, int state_id);

/* Toolbar */
void uxgtk_toolbar_init(void);
void uxgtk_toolbar_uninit(void);
void uxgtk_toolbar_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height);
BOOL uxgtk_toolbar_is_part_defined(int part_id, int state_id);

/* Trackbar */
void uxgtk_trackbar_init(void);
void uxgtk_trackbar_uninit(void);
void uxgtk_trackbar_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height);
BOOL uxgtk_trackbar_is_part_defined(int part_id, int state_id);

/* Window */
void uxgtk_window_init(void);
void uxgtk_window_uninit(void);
void uxgtk_window_draw_background(cairo_t *cr, int part_id, int state_id, int width, int height);
BOOL uxgtk_window_is_part_defined(int part_id, int state_id);
HRESULT uxgtk_window_get_color(int part_id, int state_id, int prop_id, GdkRGBA *rgba);

#endif /* UXTHEMEGTK_INTERNAL_H */
