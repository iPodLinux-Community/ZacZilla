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

#include "ttk.h"
#include "menudata.h"
#include "bigiconmenu.h"
#include "iconui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NO_INTL
//extern char *gettext (const char *msgid);
#else
#define gettext(str) str
#endif

#ifndef MIN
#define MIN(x,y) (((x)<(y))?(x):(y))
#endif

#define _MAKETHIS  menu_data *data = (menu_data *)this->data

extern ttk_screeninfo *ttk_screen;

// Note:
// Variables called / starting with `xi' are indexes in the menu,
// including hidden items. Variables called / starting with `vi' are
// indexes in the imaginary "list" of *visible* items. ("Visible"
// as in "[item]->visible() returns 1", not "onscreen right now".)

static void MakeVIXI (TWidget *this) 
{
    int vi, xi;
    _MAKETHIS;

    for (vi = xi = 0; xi < data->items; xi++) {
        data->vixi[xi] = vi;
        data->xivi[vi] = xi;
        if (!data->menu[xi]->visible || data->menu[xi]->visible (data->menu[xi]))
            vi++;
    }
    data->vitems = vi;
    data->scroll = (vi > data->visible);
}

static void render (TWidget *this, int first, int n)
{
    int vi, xi, ofs, iofs;
    ttk_color menu_bg_color=0, menu_fg_color=0;
    ttk_color menu_selbg_color=0, menu_selfg_color=0;
    _MAKETHIS;
    int wid = this->w - 10*data->scroll;

    if (!data->itemsrf)  data->itemsrf  = calloc (data->allocation, sizeof(ttk_surface));
    if (!data->itemsrfI) data->itemsrfI = calloc (data->allocation, sizeof(ttk_surface));

    ofs = (data->itemheight - ttk_text_height (data->font)) / 2;
    iofs = (data->itemheight - 32) / 2;

    if (first >= data->items) {
	return;
    }
    
    if (!data->menu)
	return;

    /* a little bit of insurance in case the menu colors loaded bad
     * 
     * check the menu selection items to see if they're equal to eachother
     * and black.  If they are (Black box issue) then set them to be
     * something sane instead.
     */
    if( ttk_ap_getx( "menu.bg" ))
	    menu_bg_color = ttk_ap_getx( "menu.bg" )->color;
    if( ttk_ap_getx( "menu.fg" ))
	    menu_fg_color = ttk_ap_getx( "menu.fg" )->color;
    if( ttk_ap_getx( "menu.selbg" ))
	    menu_selbg_color = ttk_ap_getx ("menu.selbg")->color;
    if( ttk_ap_getx( "menu.selfg" ))
	    menu_selfg_color = ttk_ap_getx ("menu.selfg")->color;
    if( menu_bg_color == menu_fg_color ) {
	if( menu_bg_color == 0 || menu_bg_color == 3 ) { /* why 3? */
		menu_bg_color = ttk_makecol( WHITE );
		menu_fg_color = ttk_makecol( BLACK );
	}
    }
    if( menu_selbg_color == menu_selfg_color ) {
	if( menu_selbg_color == 0 || menu_selbg_color == 3 ) { /* why 3? */
		menu_selbg_color = ttk_makecol( DKGREY );
		menu_selfg_color = ttk_makecol( WHITE );
	}
    }

    for (xi = 0, vi = data->vixi[0]; data->menu[xi] && vi < first+n; xi++, vi = data->vixi[xi]) {
        char *truncname;

	if (vi < first) continue;

        data->menu[xi]->linewidth = this->w - 10*data->scroll;
        if (data->menu[xi]->flags & TTK_MENU_ICON)
            data->menu[xi]->linewidth -= 12;
        if (data->menu[xi]->choices)
            data->menu[xi]->linewidth -= ttk_text_width (data->font, data->menu[xi]->choices[data->menu[xi]->choice]) + 2;
        data->menu[xi]->linewidth -= 4;
        
        if ((data->menu[xi]->textwidth > data->menu[xi]->linewidth) && !(data->menu[xi]->flags & TTK_MENU_TEXT_SCROLLING))
            data->menu[xi]->flags |= TTK_MENU_TEXT_SLEFT;
        else if (data->menu[xi]->flags & TTK_MENU_TEXT_SCROLLING)
            data->menu[xi]->flags &= ~TTK_MENU_TEXT_SCROLLING;

	// Unselected
	if (data->itemsrf[xi]) ttk_free_surface (data->itemsrf[xi]);
	data->itemsrf[xi] = ttk_new_surface (data->menu[xi]->textwidth + 46,
					    data->itemheight, ttk_screen->bpp);
	ttk_fillrect (data->itemsrf[xi], 0, 0, data->menu[xi]->textwidth + 46, 
			data->itemheight, menu_bg_color );

        if (data->i18nable)
            truncname = strdup (gettext (data->menu[xi]->name));
        else
            truncname = strdup (data->menu[xi]->name);

        if (ttk_text_width (data->font, truncname) > data->menu[xi]->linewidth) {
            sprintf (truncname + strlen (truncname) - 4, "...");
            while (strlen (truncname) > 3 && ttk_text_width (data->font, truncname) > data->menu[xi]->linewidth) {
                sprintf (truncname + strlen (truncname) - 5, "...");
            }
        }
        
        ttk_text (data->itemsrf[xi], data->font, 43, ofs, menu_fg_color, truncname);
        iconui_draw_icon (data->itemsrf[xi], (char *)data->menu[xi]->name, 3, iofs);

	// Selected
	if (data->itemsrfI[xi]) ttk_free_surface (data->itemsrfI[xi]);
	data->itemsrfI[xi] = ttk_new_surface (data->menu[xi]->textwidth + 46,
					     data->itemheight, ttk_screen->bpp);
	if (ttk_ap_getx("menu.selbg")->type & TTK_AP_GRADIENT &&
			!(ttk_ap_getx("menu.selbg")->type & TTK_AP_GRAD_HORIZ))
	    ttk_ap_rect (data->itemsrfI[xi], ttk_ap_getx ("menu.selbg"), 0,
				0, data->menu[xi]->textwidth + 46 +
				ttk_ap_getx("menu.selbg")->rounding, data->itemheight); 
	else
	    ttk_fillrect (data->itemsrfI[xi], 0, 0, data->menu[xi]->textwidth + 46,
				data->itemheight, menu_selbg_color );
        if (data->i18nable)
            ttk_text (data->itemsrfI[xi], data->font, 43, ofs, 
			menu_selfg_color, gettext (data->menu[xi]->name));
        else
            ttk_text (data->itemsrfI[xi], data->font, 43, ofs, 
			menu_selfg_color, data->menu[xi]->name);
		
		iconui_draw_icon (data->itemsrfI[xi], (char *)data->menu[xi]->name, 3, iofs);
    }
}



ttk_menu_item *bigiconui_menu_get_item (TWidget *this, int xi) 
{
    _MAKETHIS;
    if (xi > data->items) return 0;
    if (data->menu) return data->menu[xi];
    return data->mlist + xi;
}

ttk_menu_item *bigiconui_menu_get_item_called (TWidget *this, const char *s)
{
    int xi = 0;
    _MAKETHIS;
    if (data->menu) {
	for (xi = 0; data->menu[xi] && data->menu[xi]->name; xi++) {
	    if (!strcmp (data->menu[xi]->name, s))
		return data->menu[xi];
	}
	return 0;
    }

    for (xi = 0; data->mlist[xi].name; xi++) {
	if (!strcmp (data->mlist[xi].name, s))
	    return data->mlist + xi;
    }
    return 0;
}

ttk_menu_item *bigiconui_menu_get_selected_item (TWidget *this) 
{
    _MAKETHIS;
    return data->menu[data->xivi[data->top + data->sel]];
}


void bigiconui_menu_item_updated (TWidget *this, ttk_menu_item *p)
{
    _MAKETHIS;
    int i;

    if (!this) this = p->menu;
    p->menu = this;

    if (p->choices) {
	const char **q = p->choices;
	p->nchoices = 0;
	while (*q) { q++; p->nchoices++; }
    }

    if (!p->choice && p->choiceget) {
	p->choice = p->choiceget (p, p->cdata);
	p->choiceget = 0;
    }
    
    p->menuwidth = this->w;
    p->menuheight = this->h;
    
    if (data->i18nable)
        p->textwidth = ttk_text_width (data->font, gettext (p->name)) + 4;
    else
        p->textwidth = ttk_text_width (data->font, p->name) + 4;
    
    p->iconflash = 3;
    p->flags |= TTK_MENU_ICON_FLASHOFF;
    p->iftime = 10;
    p->textflash = 0;

    p->textofs = 0;
    p->scrolldelay = 10;

    if (data->vitems > data->visible) {
	data->scroll = 1;
    }

#if 0
    if (data->menu) {
	for (i = 0; i < data->items; i++) {
	    if (p == data->menu[i])
		break;
	}
	if (i < data->items)
	    render (this, data->vixi[i], 1);
    }
#endif
}

static int menu_return_false (struct ttk_menu_item *item) { return 0; }

void bigiconui_menu_remove_by_name (TWidget *this, const char *name)
{
    _MAKETHIS;
    int xi;

    for (xi = 0; data->menu[xi]; xi++) {
	if (!strcmp (data->menu[xi]->name, name))
	    data->menu[xi]->visible = menu_return_false;
    }
    render (this, data->top, data->visible);
}

void bigiconui_menu_remove_by_ptr (TWidget *this, ttk_menu_item *item) 
{
    _MAKETHIS;

    item->visible = menu_return_false;
    render (this, data->top, data->visible);
}

void bigiconui_menu_remove (TWidget *this, int xi) 
{
    _MAKETHIS;
    
    data->menu[xi]->visible = menu_return_false;
    render (this, data->top, data->visible);
}

static void free_renderings (TWidget *this, int first, int n)
{
    int vi, xi;
    _MAKETHIS;
    
    if (first >= data->items) {
	fprintf (stderr, "Menu render started after end\n");
	return;
    }

    for (xi = 0, vi = data->vixi[0]; data->menu[xi] && vi < first+n; xi++, vi = data->vixi[xi]) {
	if (vi < first) continue;

	if (data->itemsrf[xi])  ttk_free_surface (data->itemsrf[xi]);
	if (data->itemsrfI[xi]) ttk_free_surface (data->itemsrfI[xi]);

	data->itemsrf[xi] = data->itemsrfI[xi] = 0;
    }
}


void bigiconui_menu_insert (TWidget *this, ttk_menu_item *item, int xi) 
{
    _MAKETHIS;
    
    if (xi >= data->items) {
        bigiconui_menu_append (this, item);
        return;
    }

    if (data->items >= data->allocation) {
	data->allocation = data->items + 50;
	data->menu = realloc (data->menu, sizeof(void*) * data->allocation);
        data->vixi = realloc (data->vixi, sizeof(int) * data->allocation);
        data->xivi = realloc (data->xivi, sizeof(int) * data->allocation);
	if (data->itemsrf) {
	    data->itemsrf = realloc (data->itemsrf, data->allocation * sizeof(ttk_surface));
	    memset (data->itemsrf + data->items, 0, (data->allocation - data->items) * sizeof(ttk_surface));
	}
	if (data->itemsrfI) {
	    data->itemsrfI = realloc (data->itemsrfI, data->allocation * sizeof(ttk_surface));
	    memset (data->itemsrfI + data->items, 0, (data->allocation - data->items) * sizeof(ttk_surface));
	}
    }

    memmove (data->menu + xi + 1, data->menu + xi, sizeof(void*) * (data->items - xi));
    if (data->itemsrf) memmove (data->itemsrf + xi + 1, data->itemsrf + xi, sizeof(ttk_surface) * (data->items - xi));
    if (data->itemsrfI) memmove (data->itemsrfI + xi + 1, data->itemsrfI + xi, sizeof (ttk_surface) * (data->items - xi));
    data->itemsrf[xi] = data->itemsrfI[xi] = 0;
    data->menu[xi] = item;
    data->items++;
    
    bigiconui_menu_item_updated (this, item);
    MakeVIXI (this);
    item->menudata = data;
    if (data->vixi[xi] - data->top - 1 <= data->visible) {
	render (this, data->top, data->visible);
    }    
}

void bigiconui_menu_append (TWidget *this, ttk_menu_item *item) 
{
    _MAKETHIS;
    
    if (data->items >= data->allocation-1) {
	data->allocation = data->items + 50;
	data->menu = realloc (data->menu, sizeof(void*) * data->allocation);
        data->vixi = realloc (data->vixi, sizeof(int) * data->allocation);
        data->xivi = realloc (data->xivi, sizeof(int) * data->allocation);
	if (data->itemsrf) {
	    data->itemsrf = realloc (data->itemsrf, data->allocation * sizeof(ttk_surface));
	    memset (data->itemsrf + data->items, 0, (data->allocation - data->items) * sizeof(ttk_surface));
	}
	if (data->itemsrfI) {
	    data->itemsrfI = realloc (data->itemsrfI, data->allocation * sizeof(ttk_surface));
	    memset (data->itemsrfI + data->items, 0, (data->allocation - data->items) * sizeof(ttk_surface));
	}
    }

    data->menu[data->items] = item;
    data->items++;
    data->menu[data->items] = 0;
    
    bigiconui_menu_item_updated (this, item);
    item->menudata = data;
    if (data->items < 2 || (data->menu[data->items - 2]->visible && !data->menu[data->items - 2]->visible (data->menu[data->items - 2]))) {
        MakeVIXI (this);
    } else {
        if (!item->visible || item->visible (item)) {
            data->vixi[data->items - 1] = data->vitems;
            data->xivi[data->vitems] = data->items - 1;
            data->vitems++;
        } else {
            data->vixi[data->items - 1] = data->vitems - 1;
        }
        data->scroll = (data->vitems > data->visible);
    }
    if (data->vitems - data->top - 1 <= data->visible) {
	render (this, data->top, data->visible);
    }
}

static int sort_compare_i18n (const void *a, const void *b) 
{
    ttk_menu_item *A = *(ttk_menu_item **)a, *B = *(ttk_menu_item **)b;
    
    return strcasecmp (gettext (A->name), gettext (B->name));
}
static int sort_compare (const void *a, const void *b) 
{
    ttk_menu_item *A = *(ttk_menu_item **)a, *B = *(ttk_menu_item **)b;
    return strcasecmp (A->name, B->name);
}
void bigiconui_menu_sort_my_way (TWidget *this, int (*cmp)(const void *, const void *))
{
    _MAKETHIS;
    
    qsort (data->menu, data->items, sizeof(void*), cmp);
    MakeVIXI (this);
    render (this, data->top, data->visible);
}
void bigiconui_menu_sort (TWidget *this) 
{
    _MAKETHIS;
    if (data->i18nable)
        bigiconui_menu_sort_my_way (this, sort_compare_i18n);
    else
        bigiconui_menu_sort_my_way (this, sort_compare);
}

void bigiconui_menu_updated (TWidget *this)
{
    ttk_menu_item *p, **q;
    _MAKETHIS;

    int olditems = data->items;

    p = data->mlist;
    data->items = 0;
    while (p && p->name) { p++; data->items++; }
    data->allocation = data->items + 50;
    if (data->vixi) free (data->vixi);
    data->vixi = calloc (data->allocation, sizeof(int));
    data->xivi = calloc (data->allocation, sizeof(int));

    data->scroll = (data->vitems > data->visible);

    if (data->menu) free (data->menu);
    data->menu = calloc (data->allocation, sizeof(void*));

    p = data->mlist;
    q = data->menu;
    while (p && p->name) {
	bigiconui_menu_item_updated (this, p);
	p->menudata = data;

	*q++ = p;
	p++;
    }
    *q = 0;
    MakeVIXI (this);

    if (data->scroll) {
	data->sheight = data->visible * this->h / data->items;
	data->spos = 0;
    }

    data->ds = 0;
}


int bigiconui_menu_frame (TWidget *this) 
{   
    int oldflags, oldflash;
    _MAKETHIS;

    if (!data->menu || !data->items) return 0;

    ttk_menu_item *selected = data->menu[data->xivi[data->top + data->sel]];

    data->ds++;

    if (selected->flags & TTK_MENU_TEXT_SCROLLING) {
	if (selected->scrolldelay) selected->scrolldelay--;

	if (!selected->scrolldelay) {
	    if (selected->flags & TTK_MENU_TEXT_SLEFT) {
		selected->textofs += 3;
	    } else {
		selected->textofs -= 3;
	    }

	    this->dirty++;
	    
	    if (selected->textofs <= 0) {
		selected->textofs = 0;
		selected->scrolldelay = 10;
		selected->flags &= ~TTK_MENU_TEXT_SRIGHT;
		selected->flags |= TTK_MENU_TEXT_SLEFT;
	    } else if (selected->textofs + selected->linewidth >= selected->textwidth) {
		selected->textofs = selected->textwidth - selected->linewidth;
		selected->scrolldelay = 10;
		selected->flags |= TTK_MENU_TEXT_SRIGHT;
		selected->flags &= ~TTK_MENU_TEXT_SLEFT;
	    }
	}
    }

    oldflags = selected->flags;
    oldflash = (selected->textflash << 8) | selected->iconflash;

    if (selected->iftime)
        selected->iftime--;
    if (!selected->iftime) {
        if (selected->flags & TTK_MENU_ICON_FLASHOFF) {
            selected->iconflash--;
            selected->iftime =
                (selected->iconflash == 0)? 9 :
                (selected->iconflash == 1 || selected->iconflash == 2)? 1 : 20;
            if (selected->iconflash == 0)
                selected->flags &= ~TTK_MENU_ICON_FLASHOFF;
        } else {
            selected->iconflash++;
            selected->iftime =
                (selected->iconflash == 1 || selected->iconflash == 2)? 1 :
                (selected->iconflash == 3)? 9 : 20;
            if (selected->iconflash >= 3)
                selected->flags |= TTK_MENU_ICON_FLASHOFF;
        }
    }

    if ((data->ds % 2) == 0 && selected->textflash)
        selected->textflash--;

    // Redraw every ~1s, in case menu items change visibility / fonts change
    if ((oldflags != selected->flags) || (oldflash != ((selected->textflash << 8) | selected->iconflash)) || !(data->ds % 10)) {
        if (!(data->ds % 10))
            MakeVIXI (this);
        this->dirty++;
    }

    return 0;
}


void bigiconui_menu_flash (ttk_menu_item *item, int n) 
{
    item->textflash = 2*n;
}


TWidget *bigiconui_new_menu_widget (ttk_menu_item *items, ttk_font font, int w, int h)
{
	int th;
    ttk_menu_item *p;
    TWidget *ret = ttk_new_widget (0, 1);
    menu_data *data = calloc (sizeof(menu_data), 1);
    
    if (h == (ttk_screen->h - ttk_screen->wy)) h--;

    ret->w = w;
    ret->h = h;
    ret->data = data;
    ret->focusable = 1;
    ret->draw = bigiconui_menu_draw;
    ret->frame = bigiconui_menu_frame;
    ret->down = bigiconui_menu_down;
    ret->button = bigiconui_menu_button;
    ret->scroll = bigiconui_menu_scroll;
    ret->destroy = bigiconui_menu_free;

	th = (ttk_text_height (font) + 4);
	if (th < 36) th = 36;
    data->visible = h / th;
    data->itemheight = h / data->visible;
    data->mlist = items;
    data->menu = 0;
    data->font = font;
    data->closeable = data->i18nable = 1;
    data->epoch = ttk_epoch;
    data->free_everything = !items;
    data->drawn = 0;

    ttk_widget_set_fps (ret, 10);

    bigiconui_menu_updated (ret);
    render (ret, 0, data->visible);

    ret->dirty = 1;
    return ret;
}

void bigiconui_menu_set_closeable (TWidget *this, int closeable) 
{
    _MAKETHIS;
    data->closeable = closeable;
}
void bigiconui_menu_set_i18nable (TWidget *this, int i18nable) 
{
    _MAKETHIS;
    data->i18nable = i18nable;
}

void bigiconui_menu_draw (TWidget *this, ttk_surface srf)
{
    _MAKETHIS;
    int i, y = this->y;
    int ofs = (data->itemheight - ttk_text_height (data->font)) / 2;
    int nivis = 0;
    int spos, sheight;
    int vi, xi;

    if (ttk_epoch > data->epoch) {
	int i;
	int th = (ttk_text_height (data->font) + 4);
	if (th < 36) th = 36;
	data->font = ttk_menufont;
	data->visible = this->h / th;
	data->itemheight = this->h / data->visible;
	for (i = 0; i < data->items; i++)
	    bigiconui_menu_item_updated (this, data->menu[i]);
	render (this, data->top, data->visible);
	data->epoch = ttk_epoch;
    }

    if (!data->drawn) {
        MakeVIXI (this);
        data->drawn = 1;
    }

    data->scroll = (data->vitems > data->visible);

    if (!data->items) {
	char *header = "No Items.";
	char *sub = "Press a button.";
	ttk_text (srf, data->font, (this->w - ttk_text_width(data->font, header)) / 2, 20, ttk_makecol (BLACK), header);
	ttk_text (srf, ttk_textfont, (this->w - ttk_text_width(ttk_textfont, sub)) / 2, 50, ttk_makecol (BLACK), sub);
	return;
    }

    for (i = 0; i < data->items; i++) {
	if (data->menu[i]->visible && // function, and
	    !data->menu[i]->visible (data->menu[i])) { // function returns 0
	    continue;
	}
	
	nivis++;
    }
    
    if (!data->menu) return;

    for (vi = data->top, xi = data->xivi[data->top];
	 data->menu[xi] && vi < MIN (data->top + data->visible, data->vitems);
	 vi++, xi = data->xivi[vi]) {
	ttk_color col;
        int selected = (vi == data->top + data->sel);

	if (data->menu[xi]->visible && !data->menu[xi]->visible (data->menu[xi]))
	    continue;

        if (data->menu[xi]->predraw)
            data->menu[xi]->predraw (data->menu[xi]);

	if (vi == data->top + data->sel && !(data->menu[xi]->textflash % 2)) {
	    if (!data->itemsrfI[xi]) {
		fprintf (stderr, "Null pointer (I)\n");
		abort();
	    }
	    
	    ttk_ap_fillrect (srf, ttk_ap_get ("menu.selbg"), this->x, y, this->x + this->w - 11*data->scroll,
			     y + data->itemheight);
	    ttk_blit_image_ex (data->itemsrfI[xi], data->menu[xi]->textofs, 0,
			       data->menu[xi]->linewidth, data->itemheight,
			       srf, this->x, y);
	    col = ttk_ap_getx ("menu.selfg") -> color;
	} else {
	    if (!data->itemsrf[xi]) {
		fprintf (stderr, "Null pointer (N)\n");
		abort();
	    }

	    ttk_ap_fillrect (srf, ttk_ap_get ("menu.bg"), this->x, y, this->x + this->w - 11*data->scroll,
			     y + data->itemheight);
	    ttk_blit_image_ex (data->itemsrf[xi], 0, 0, data->menu[xi]->linewidth, data->itemheight,
			       srf, this->x, y);
	    col = ttk_ap_getx ("menu.fg") -> color;
	}

	if (data->menu[xi]->choices) {
	    ttk_text (srf, data->font, this->x + this->w - 4 - 11*data->scroll -
		      ttk_text_width (data->font, data->menu[xi]->choices[data->menu[xi]->choice]),
		      y + ofs, col, data->menu[xi]->choices[data->menu[xi]->choice]);
	}

	if (data->menu[xi]->flags & TTK_MENU_ICON) {
            const char *prop =
                (!selected)? "menu.icon" :
                (data->menu[xi]->iconflash == 3)? "menu.icon3" :
                (data->menu[xi]->iconflash == 2)? "menu.icon2" :
                (data->menu[xi]->iconflash == 1)? "menu.icon1" :
                "menu.icon0";
            TApItem *ap = ttk_ap_get (prop);
            if (ap) {
                ttk_color bgcol = (selected? ttk_ap_getx ("menu.selbg")->color : ttk_ap_getx ("menu.bg")->color);
                
                switch (data->menu[xi]->flags & TTK_MENU_ICON) {
                case TTK_MENU_ICON_SUB:
                    ttk_draw_icon (ttk_icon_sub, srf, this->x + this->w + 1 - 11*data->scroll - 11, y + (data->itemheight - 13) / 2, ap, bgcol);
                    break;
                case TTK_MENU_ICON_EXE:
                    ttk_draw_icon (ttk_icon_exe, srf, this->x + this->w + 1 - 11*data->scroll - 11, y + (data->itemheight - 13) / 2, ap, bgcol);
                    break;
                case TTK_MENU_ICON_SND:
                    ttk_draw_icon (ttk_icon_spkr, srf, this->x + this->w + 1 - 11*data->scroll - 11, y + (data->itemheight - 13) / 2, ap, bgcol);
                    break;
                }
            }
        }

	y += data->itemheight;
    }

    if (data->scroll) {
	sheight = data->visible * (this->h) / nivis;
	spos = data->top * (this->h - 2*ttk_ap_getx ("header.line") -> spacing) / nivis - 1;

	if (sheight < 3) sheight = 3;

	ttk_ap_fillrect (srf, ttk_ap_get ("scroll.bg"), this->x + this->w - 10,
			 this->y + 2*ttk_ap_getx ("header.line") -> spacing,
			 this->x + this->w, this->y + this->h);
	ttk_ap_rect (srf, ttk_ap_get ("scroll.box"), this->x + this->w - 10,
		     this->y + 2*ttk_ap_getx ("header.line") -> spacing,
		     this->x + this->w, this->y + this->h);
	ttk_ap_fillrect (srf, ttk_ap_get ("scroll.bar"), this->x + this->w - 10,
			 this->y + 2*ttk_ap_getx ("header.line") -> spacing + spos + 1,
			 this->x + this->w,
			 this->y + ttk_ap_getx ("header.line") -> spacing + spos + sheight + 1);
    }
}


int bigiconui_menu_scroll (TWidget *this, int dir)
{
    _MAKETHIS;
    int oldtop, oldsel;
    TTK_SCROLLMOD (dir, 5);
    TTK_SCROLL_ACCEL(dir, 10, 50); /* TODO: Tune these constants */

    if (!data->menu || !data->items) return 0;

    oldtop = data->top;
    oldsel = data->sel;

    // Check whether we need to scroll the list. Adjust bounds accordingly.
    data->sel += dir;
    
    if (data->sel >= data->visible) {
	data->top += (data->sel - data->visible + 1);
	data->sel -= (data->sel - data->visible + 1);
    }
    if (data->sel < 0) {
	data->top += data->sel; // actually subtracts; sel is negative
	data->sel = 0;
    }
    
    // :TRICKY: order in this pair of ifs is important, and they
    // should not be else-ifs!
    
    if ((data->top + data->visible) > data->vitems) {
	data->sel -= data->vitems - (data->top + data->visible);
	data->top  = data->vitems - data->visible;
    }
    if (data->top < 0) {
	data->sel += data->top; // actually subtracts; top is negative
	data->top = 0;
    }
    if (data->sel < 0) {
	data->sel = 0;
    }
    if (data->sel >= MIN (data->visible, data->vitems)) {
	data->sel = MIN (data->visible, data->vitems) - 1;
    }
    
    data->spos = data->top * (this->h - 2*ttk_ap_getx ("header.line") -> spacing) / data->vitems;
    
    if ((oldtop != data->top) || (oldsel != data->sel)) {
	this->dirty++;
	data->menu[data->xivi[oldtop + oldsel]]->textofs = 0;

	if (oldtop != data->top) {
	    // Make sure the new items are rendered
	    if (oldtop < data->top) {
		free_renderings (this, oldtop, data->top - oldtop);
		render (this, oldtop + data->visible, data->top - oldtop);
	    } else {
		free_renderings (this, data->top + data->visible, oldtop - data->top);
		render (this, data->top, oldtop - data->top);
	    }
	}

	return TTK_EV_CLICK;
    }
    return 0;
}

int bigiconui_menu_button (TWidget *this, int button, int time)
{
    _MAKETHIS;
    ttk_menu_item *item;
    TWindow *sub;
    
    if (!data->menu || !data->items) {
        ttk_hide_window (this->win);
        return 0;
    }

    item = data->menu[data->xivi[data->top + data->sel]];

    switch (button) {
    case TTK_BUTTON_ACTION:
	if (item->choices) {
	    ++item->choice; item->choice %= item->nchoices;
	    if (item->choicechanged) item->choicechanged (item, item->cdata);
            render (this, data->top + data->sel, 1);
	    this->dirty++;
	    break;
	}
	
	// don't cache the directives
	if ((item->flags & TTK_MENU_MADESUB) && (item->sub < TTK_MENU_DESC_MAX) && item->makesub) {
	    item->sub = 0;
	    item->flags &= ~TTK_MENU_MADESUB;
	}

	// free-it window
	// If you set window->data to 0x12345678, the window will be recreated
	// anew each time it is selected. This is to support legacy code
	if ((item->flags & TTK_MENU_MADESUB) && item->sub && item->makesub &&
	    !(item->sub < TTK_MENU_DESC_MAX) && (item->sub->data == 0x12345678))
	{
	    ttk_free_window (item->sub);
	    item->sub = 0;
	    item->flags &= ~TTK_MENU_MADESUB;
	}

	if (!(item->flags & TTK_MENU_MADESUB)) {
	    if (item->makesub) {
		item->flags |= TTK_MENU_MADESUB; // so makesub can unset it if it needs to be called every time

		// In the special case that makesub() simply blasts the window onto the
		// screen itself, it can return TTK_MENU_ALREADYDONE and set item->sub
		// itself.
		sub = item->makesub (item);
		if (sub != TTK_MENU_ALREADYDONE) {
		    item->sub = sub;
		} else {
		    break;
		}
	    }
	}

	if (item->sub == TTK_MENU_DONOTHING) {
	    return 0;
	} else if (item->sub == TTK_MENU_REPLACE) {
	    ttk_hide_window (this->win);
	    return TTK_EV_CLICK;
	} else if (item->sub == TTK_MENU_UPONE) {
	    // FALLTHRU to menu button handling
	} else if (item->sub == TTK_MENU_UPALL) {
	    int r = -1;
	    while (ttk_windows->w->focus->draw == bigiconui_menu_draw &&
		   ((menu_data *)ttk_windows->w->focus->data)->closeable &&
		   (r = ttk_hide_window (ttk_windows->w)) != -1)
		ttk_click();
	    if (r == -1)
		return TTK_EV_DONE;

	    return 0;
	} else if (item->sub == TTK_MENU_QUIT) {
	    return TTK_EV_DONE;
	} else {
	    ttk_show_window (item->sub);
	    return TTK_EV_CLICK;
	}
    case TTK_BUTTON_MENU:
	if (data->closeable) {
	    if (ttk_hide_window (this->win) == -1) {
		return TTK_EV_DONE;
	    }
	    return TTK_EV_CLICK;
	}
	return 0;
    }
    return TTK_EV_UNUSED;
}

int bigiconui_menu_down (TWidget *this, int button)
{
    return TTK_EV_UNUSED;
}

// When you free a menu, all the windows it spawned are also freed.
// If its parent window was also a menu, it needs to be told that this one doesn't exist.
// Thus, you probably shouldn't free menus outside of special circumstances (e.g. you
// pop-up and free the parent menu yourself).
void bigiconui_menu_free (TWidget *this) 
{
    int i;
    _MAKETHIS;

    if (data->menu) {
	for (i = 0; data->menu[i]; i++) {
	    if (data->menu[i]->sub && data->menu[i]->sub != TTK_MENU_DONOTHING &&
		data->menu[i]->sub != TTK_MENU_UPONE && data->menu[i]->sub != TTK_MENU_UPALL &&
		data->menu[i]->sub != TTK_MENU_QUIT && data->menu[i]->sub != TTK_MENU_REPLACE)
		ttk_free_window (data->menu[i]->sub);
	    if (data->itemsrf[i]) ttk_free_surface (data->itemsrf[i]);
	    if (data->itemsrfI[i]) ttk_free_surface (data->itemsrfI[i]);
	    if (data->free_everything) {
                if (data->menu[i]->free_data)
                    free (data->menu[i]->data);
                if (data->menu[i]->free_name)
                    free ((char *)data->menu[i]->name);
                free (data->menu[i]);
            }
	}
	free (data->menu);
    }

    free (data);
}

void bigiconui_menu_clear (TWidget *this) 
{
    int i;
    _MAKETHIS;
    if (data->menu) {
        for (i = 0; data->menu[i]; i++) {
            if (data->menu[i]->sub && data->menu[i]->sub < TTK_MENU_DESC_MAX) {
                ttk_free_window (data->menu[i]->sub);
            }
            if (data->itemsrf[i]) { ttk_free_surface (data->itemsrf[i]); data->itemsrf[i] = 0; }
            if (data->itemsrfI[i]) { ttk_free_surface (data->itemsrf[i]); data->itemsrfI[i] = 0; }
            if (data->free_everything && data->menu[i]->free_data) free (data->menu[i]->data);
            if (data->free_everything && data->menu[i]->free_name) free ((char *)data->menu[i]->name);
            free (data->menu[i]);
            data->menu[i] = 0;
        }
    }
    data->items = 0;
    data->sel = data->top = 0;
    this->dirty++;
}

TWindow *bigiconuix_mh_sub (ttk_menu_item *item) 
{
    TWindow *ret = ttk_new_window();

    ttk_add_widget (ret, bigiconui_new_menu_widget (item->data, ttk_menufont,
					      item->menuwidth, item->menuheight));
    ttk_window_title (ret, gettext (item->name));
    ret->widgets->v->draw (ret->widgets->v, ret->srf);
    return ret;
}

void *bigiconuix_md_sub (ttk_menu_item *submenu) 
{
    return (void *)submenu;
}

