/*
 * Copyright (c) 2005 Joshua Oreman
 *
 * This file is a part of TTK.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _BIGICONUI_MENU_H_
#define _BIGICONUI_MENU_H_

#include <ttk.h>

TWidget *bigiconui_new_menu_widget (ttk_menu_item *items, ttk_font font, int w, int h);
ttk_menu_item *bigiconui_menu_get_item (TWidget *_this, int i);
ttk_menu_item *bigiconui_menu_get_item_called (TWidget *_this, const char *s);
ttk_menu_item *bigiconui_menu_get_selected_item (TWidget *_this);
void bigiconui_menu_set_closeable (TWidget *_this, int closeable);
void bigiconui_menu_set_i18nable (TWidget *_this, int i18nable);
void bigiconui_menu_sort (TWidget *_this);
void bigiconui_menu_sort_my_way (TWidget *_this, int (*cmp)(const void *, const void *));
void bigiconui_menu_flash (ttk_menu_item *item, int nflashes);
void bigiconui_menu_append (TWidget *_this, ttk_menu_item *item);
void bigiconui_menu_insert (TWidget *_this, ttk_menu_item *item, int pos);
void bigiconui_menu_remove (TWidget *_this, int pos);
void bigiconui_menu_remove_by_ptr (TWidget *_this, ttk_menu_item *item);
void bigiconui_menu_remove_by_name (TWidget *_this, const char *name);
void bigiconui_menu_clear (TWidget *_this);
void bigiconui_menu_item_updated (TWidget *, ttk_menu_item *); // call whenever you edit that item
void bigiconui_menu_updated (TWidget *_this); // call whenever you edit the item list somehow;
                                       // FORGETS ABOUT ALL ADDED ITEMS!

void bigiconui_menu_draw (TWidget *_this, ttk_surface srf);
int bigiconui_menu_scroll (TWidget *_this, int dir);
int bigiconui_menu_down (TWidget *_this, int button);
int bigiconui_menu_button (TWidget *_this, int button, int time);
int bigiconui_menu_frame (TWidget *_this);
void bigiconui_menu_free (TWidget *_this); // You don't need to call this, just call ttk_free_widget()

TWindow *bigiconuix_mh_sub (struct ttk_menu_item *item);
void *bigiconuix_md_sub (struct ttk_menu_item *submenu);

#endif
