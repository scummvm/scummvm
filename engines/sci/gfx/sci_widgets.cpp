/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/include/gfx_operations.h"
#include "sci/include/gfx_widgets.h"
#include "sci/include/engine.h"
#include "sci/include/menubar.h"
#include "sci/include/sci_widgets.h"

namespace Sci {

#define SCI_SPECIAL_CHAR_ARROW_UP 0x18
#define SCI_SPECIAL_CHAR_ARROW_DOWN 0x19

static void clear_titlebar(gfxw_port_t *titlebar) {
	if (titlebar->contents) {
		titlebar->contents->widfree(titlebar->contents);
		titlebar->contents = NULL;
		titlebar->nextpp = &(titlebar->contents);
	}
}

static gfxw_list_t *make_titlebar_list(state_t *s, rect_t bounds, gfxw_port_t *status_bar) {
	gfx_color_t color = status_bar->bgcolor;
	gfxw_list_t *list;
	gfxw_box_t *bgbox;


	list = gfxw_new_list(status_bar->bounds, 0);
	bgbox = gfxw_new_box(s->gfx_state, gfx_rect(0, 0, status_bar->bounds.xl, status_bar->bounds.yl - 1),
	                     color, color, GFX_BOX_SHADE_FLAT);

	list->add((gfxw_container_t *) list, (gfxw_widget_t *) bgbox);

	return list;
}

static gfxw_list_t *finish_titlebar_list(state_t *s, gfxw_list_t *list, gfxw_port_t *status_bar) {
	gfx_color_t black = s->ega_colors[0];
	gfxw_primitive_t *line;

	line = gfxw_new_line(Common::Point(0, status_bar->bounds.yl - 1), Common::Point(status_bar->bounds.xl, status_bar->bounds.yl - 1),
	                     black, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL);
	list->add((gfxw_container_t *)list, (gfxw_widget_t *)line);

	return list;
}

void sciw_set_status_bar(state_t *s, gfxw_port_t *status_bar, char *text, int fgcolor, int bgcolor) {
	gfx_state_t *state;
	gfxw_list_t *list;
	gfx_color_t bg = status_bar->bgcolor;
	gfx_color_t fg = status_bar->color;
	gfx_color_t black = s->ega_colors[0];

	if (!status_bar->visual) {
		GFXERROR("Attempt to change title bar without visual");
		return;
	}

	state = status_bar->visual->gfx_state;

	if (!state) {
		GFXERROR("Attempt to change title bar with stateless visual");
		return;
	}

	clear_titlebar(status_bar);

	if (text) {
		gfxw_text_t *textw = gfxw_new_text(state, gfx_rect(0, 0, status_bar->bounds.xl, status_bar->bounds.yl),
		                                   status_bar->font_nr, text, ALIGN_LEFT, ALIGN_CENTER,
		                                   fg, fg, bg, GFXR_FONT_FLAG_NO_NEWLINES);

		list = make_titlebar_list(s, status_bar->bounds, status_bar);

		list->add((gfxw_container_t *)list, (gfxw_widget_t *)textw);

	} else {
		gfxw_box_t *bgbox = gfxw_new_box(state, gfx_rect(0, 0, status_bar->bounds.xl, status_bar->bounds.yl - 1),
		                                 black, black, GFX_BOX_SHADE_FLAT);

		list = gfxw_new_list(status_bar->bounds, 0);

		list->add((gfxw_container_t *)list, (gfxw_widget_t *)bgbox);
	}

	list->add(GFXWC(status_bar), GFXW(list));
	finish_titlebar_list(s, list, status_bar);

	status_bar->draw(GFXW(status_bar), gfxw_point_zero);
	gfxop_update(state);
}

static void sciw_make_window_fit(rect_t *rect, gfxw_port_t *parent) {
	// This window is meant to cover the whole screen, so we allow it to go through.
	if (rect->xl == 319 && rect->yl == 189) return;

	if (rect->x + rect->xl > parent->bounds.x + parent->bounds.xl)
		rect->x -= (rect->x + rect->xl) - (parent->bounds.x + parent->bounds.xl) + 2;

	if (rect->y + rect->yl > parent->bounds.y + parent->bounds.yl)
		rect->y -= (rect->y + rect->yl) - (parent->bounds.y + parent->bounds.yl) + 2;
}

gfxw_port_t *sciw_new_window(state_t *s, rect_t area, int font, gfx_color_t color, gfx_color_t bgcolor,
                int title_font, gfx_color_t title_color, gfx_color_t title_bgcolor, const char *title, int flags) {
	gfxw_visual_t *visual = s->visual;
	gfx_state_t *state = s->gfx_state;
	int shadow_offset = 2;
	rect_t frame;
	gfx_color_t black	= {{0, 0, 0, 0}, 0, 0, 0, 0};
	gfxw_port_t *win;
	gfxw_list_t *decorations;
//	int xextra = !(flags & WINDOW_FLAG_NOFRAME) ? 1 : 0;
//	int yextra = !(flags & WINDOW_FLAG_NOFRAME) ? 2 : 0;

	if (area.xl == 319 && area.yl == 189) {
		flags |= WINDOW_FLAG_NOFRAME;
		// The below line makes the points bar in QfG2 work, but breaks
		// the one in QfG1. Hm.
		if ((byte)bgcolor.priority == 255) /* Yep, QfG2 */
			area.y += 3;
	}

	/*
	if (area.y + area.yl > visual->bounds.y + visual->bounds.yl) {
		area.y -= (area.y + area.yl) - (visual->bounds.y + visual->bounds.yl) + yextra;
	}

	if (area.x + area.xl > visual->bounds.x + visual->bounds.xl) {
		area.x -= (area.x + area.xl) - (visual->bounds.x + visual->bounds.xl) + xextra;
	}
	*/

	if (flags & WINDOW_FLAG_TITLE)
		area. y += 10;

	if (!(flags & (WINDOW_FLAG_TITLE | WINDOW_FLAG_NOFRAME)))
		area.yl -= 1; // Normal windows are drawn one pixel too small.

	sciw_make_window_fit(&area, s->wm_port);
	win = gfxw_new_port(visual, s->wm_port, area, color, bgcolor);

	win->font_nr = font;
	win->title_text = title;
	win->port_flags = flags;

	win->flags |= GFXW_FLAG_IMMUNE_TO_SNAPSHOTS;

	if (flags & WINDOW_FLAG_DONTDRAW)
		flags = WINDOW_FLAG_TRANSPARENT | WINDOW_FLAG_NOFRAME;

	if (flags == (WINDOW_FLAG_TRANSPARENT | WINDOW_FLAG_NOFRAME))
		return win; // Fully transparent window

	if (flags & WINDOW_FLAG_TITLE)
		frame = gfx_rect(area.x - 1, area.y - 10, area.xl + 2, area.yl + 11);
	else
		frame = gfx_rect(area.x - 1, area.y - 1, area.xl + 2, area.yl + 2);

	// Set visible window boundaries
	win->bounds = gfx_rect(frame.x, frame.y, frame.xl + shadow_offset, frame.yl + shadow_offset);

	decorations = gfxw_new_list(gfx_rect(frame.x, frame.y, frame.xl + 1 + shadow_offset, frame.yl + 1 + shadow_offset), 0);

	if (!(flags & WINDOW_FLAG_TRANSPARENT)) {
		// Draw window background
		win->port_bg = (gfxw_widget_t *)gfxw_new_box(state, gfx_rect(1, (flags & WINDOW_FLAG_TITLE) ? 10 : 1,
		                        area.xl, area.yl), bgcolor, bgcolor, GFX_BOX_SHADE_FLAT);
		decorations->add((gfxw_container_t *) decorations, win->port_bg);
		win->flags |= GFXW_FLAG_OPAQUE;
	}

	if (flags & WINDOW_FLAG_TITLE) {
		// Add window title
		rect_t title_rect = gfx_rect(1, 1, area.xl, 8);

		decorations->add((gfxw_container_t *)decorations, (gfxw_widget_t *)
						gfxw_new_box(state, title_rect, title_bgcolor, title_bgcolor, GFX_BOX_SHADE_FLAT));

		decorations->add((gfxw_container_t *)decorations, (gfxw_widget_t *)
						gfxw_new_text(state, title_rect, title_font, title, ALIGN_CENTER, ALIGN_CENTER, title_color, title_color,
						title_bgcolor, GFXR_FONT_FLAG_NO_NEWLINES));
	}

	if (!(flags & WINDOW_FLAG_NOFRAME)) {
		// Draw backdrop shadow

		if (!(flags & WINDOW_FLAG_NO_DROP_SHADOW)) {
			if (gfxop_set_color(state, &black, 0, 0, 0, 0x80, bgcolor.priority, -1)) {
				GFXERROR("Could not get black/semitrans color entry");
				return NULL;
			}

			decorations->add((gfxw_container_t *)decorations, (gfxw_widget_t *)
			                 gfxw_new_box(state, gfx_rect(shadow_offset + 1, frame.yl - 1,
							 frame.xl - 4, shadow_offset), black, black, GFX_BOX_SHADE_FLAT));

			decorations->add((gfxw_container_t *)decorations, (gfxw_widget_t *)
			                 gfxw_new_box(state, gfx_rect(frame.xl - 1, shadow_offset + 1,
							 shadow_offset, frame.yl - 2), black, black, GFX_BOX_SHADE_FLAT));
		}

		// Draw frame

		if (gfxop_set_color(state, &black, 0, 0, 0, 0, bgcolor.priority, -1)) {
			GFXERROR("Could not get black color entry");
			return NULL;
		}

		if (!(flags & WINDOW_FLAG_NO_DROP_SHADOW)) {

			decorations->add((gfxw_container_t *)decorations, (gfxw_widget_t *)
			                 gfxw_new_rect(gfx_rect(0, 0, frame.xl - 1, frame.yl - 1), black, GFX_LINE_MODE_FINE, GFX_LINE_STYLE_NORMAL));

			if (flags & WINDOW_FLAG_TITLE)
				decorations->add((gfxw_container_t *)decorations, (gfxw_widget_t *)gfxw_new_line(Common::Point(1, 9),
									Common::Point(frame.xl - 2, 9), black, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL));
		} else {
			decorations->add((gfxw_container_t *)decorations, (gfxw_widget_t *)
			                 gfxw_new_rect(gfx_rect(0, 0, frame.xl, frame.yl),  black, GFX_LINE_MODE_FINE, GFX_LINE_STYLE_NORMAL));
		}
	}

	win->decorations = decorations;
	decorations->parent = GFXWC(win);

	return win;
}

//*** Controls ***

static inline rect_t _move_and_extend_rect(rect_t rect, Common::Point point, int yplus) {
	return gfx_rect(rect.x + point.x, rect.y + point.y, rect.xl + 1, rect.yl + yplus);
}

gfxw_list_t *_sciw_add_text_to_list(gfxw_list_t *list, gfxw_port_t *port, rect_t zone, char *text,
                       int font, gfx_alignment_t align, char framed, char inverse, int flags, char gray_text) {
	gfx_color_t *color1, *color2, *bgcolor;

	if (inverse) {
		color1 = color2 = &(port->bgcolor);
		bgcolor = &(port->color);
	} else if (gray_text) {
		bgcolor = color1 = &(port->bgcolor);
		color2 = &(port->color);
	} else {
		color1 = color2 = &(port->color);
		bgcolor = &(port->bgcolor);
	}

	list->add(GFXWC(list), GFXW(gfxw_new_text(port->visual->gfx_state, zone, font, text, align, ALIGN_TOP,
	                            *color1, *color2, *bgcolor, flags)));

	zone.xl--;
	zone.yl -= 2;

	if (framed) {
		list->add(GFXWC(list), GFXW(gfxw_new_rect(zone, *color2, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_STIPPLED)));
	}

	return list;
}

gfxw_list_t *sciw_new_button_control(gfxw_port_t *port, reg_t ID, rect_t zone, char *text, int font, char selected, char inverse, char grayed_out) {
	gfx_color_t *frame_col = (inverse) ? &(port->bgcolor) : &(port->color);
	gfxw_list_t *list;

	zone.x--;
	zone.y--;
	zone.xl++;
	zone.yl++;

	list = gfxw_new_list(_move_and_extend_rect(zone, Common::Point(port->zone.x, port->zone.y), 1), 0);

	gfxw_set_id(GFXW(list), ID.segment, ID.offset);

	zone.x = 0;
	zone.y = 0;

	if (inverse)
		list->add(GFXWC(list), GFXW(gfxw_new_box(NULL, gfx_rect(zone.x, zone.y, zone.xl + 1, zone.yl + 1),
		                            port->color, port->color, GFX_BOX_SHADE_FLAT)));

	if (!inverse)
		list = _sciw_add_text_to_list(list, port, gfx_rect(zone.x + 1, zone.y + 2, zone.xl - 1, zone.yl),
		                              text, font, ALIGN_CENTER, 0, inverse, GFXR_FONT_FLAG_EAT_TRAILING_LF, grayed_out);

	if (!inverse)
		list->add(GFXWC(list),
		          GFXW(gfxw_new_rect(zone, *frame_col, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL)));

	if (inverse)
		list = _sciw_add_text_to_list(list, port, gfx_rect(zone.x + 1, zone.y + 2, zone.xl - 1, zone.yl),
		                              text, font, ALIGN_CENTER, 0, inverse, GFXR_FONT_FLAG_EAT_TRAILING_LF, grayed_out);

	if (selected)
		list->add(GFXWC(list),
		          GFXW(gfxw_new_rect(gfx_rect(zone.x + 1, zone.y + 1, zone.xl - 2, zone.yl - 2),
		                             *frame_col, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL)));

	return list;
}

gfxw_list_t *sciw_new_text_control(gfxw_port_t *port, reg_t ID, rect_t zone, char *text, int font,
								   gfx_alignment_t align, char framed, char inverse) {
	gfxw_list_t *list = gfxw_new_list(_move_and_extend_rect(zone, Common::Point(port->zone.x, port->zone.y), 2), 0);

	gfxw_set_id(GFXW(list), ID.segment, ID.offset);

	zone.x = 0;
	zone.y = 0;

	return _sciw_add_text_to_list(list, port, zone, text, font, align, framed, inverse, 0, port->gray_text);
}

gfxw_list_t *sciw_new_edit_control(gfxw_port_t *port, reg_t ID, rect_t zone, char *text, int font, unsigned int cursor,
								   char inverse) {
	gfxw_text_t *text_handle;
	long draw_cursor;
	long foo;

	gfxw_list_t *list;
	int cursor_height = gfxop_get_font_height(port->visual->gfx_state, font);

	zone.x--;
	zone.y--;
	zone.xl++;
	zone.yl++;

	list = gfxw_new_list(_move_and_extend_rect(zone, Common::Point(port->zone.x, port->zone.y), 1), 0);
	gfxw_set_id(GFXW(list), ID.segment, ID.offset);
	zone.x = 1;
	zone.y = 1;

	sci_gettime(&foo, &draw_cursor);
	draw_cursor = draw_cursor > 500000;

	if (!draw_cursor) {
		text_handle = gfxw_new_text(port->visual->gfx_state, zone, font, text, ALIGN_LEFT, ALIGN_TOP,
		                            port->color, port->color, port->bgcolor, GFXR_FONT_FLAG_NO_NEWLINES);

		list->add(GFXWC(list), GFXW(text_handle));
	} else {
		char *textdup = (char *)sci_malloc(strlen(text) + 1);

		strncpy(textdup, text, cursor);

		if (cursor <= strlen(text))
			textdup[cursor] = 0; // terminate

		if (cursor > 0) {
			text_handle = gfxw_new_text(port->visual->gfx_state, zone, font, textdup, ALIGN_LEFT, ALIGN_TOP,
			                            port->color, port->color, port->bgcolor, GFXR_FONT_FLAG_NO_NEWLINES);

			list->add(GFXWC(list), GFXW(text_handle));
			zone.x += text_handle->width;
		}

		if (cursor < strlen(text)) {
			textdup[0] = text[cursor];
			textdup[1] = 0;
			text_handle =  gfxw_new_text(port->visual->gfx_state, zone, font, textdup, ALIGN_LEFT, ALIGN_TOP,
			                             port->bgcolor, port->bgcolor, port->color, GFXR_FONT_FLAG_NO_NEWLINES);
			list->add(GFXWC(list), GFXW(text_handle));
			zone.x += text_handle->width;
		};

		if (cursor + 1 < strlen(text)) {
			text_handle = gfxw_new_text(port->visual->gfx_state, zone, font, text + cursor + 1, ALIGN_LEFT, ALIGN_TOP,
			                            port->color, port->color, port->bgcolor, GFXR_FONT_FLAG_NO_NEWLINES);
			list->add(GFXWC(list), GFXW(text_handle));
			zone.x += text_handle->width;
		};

		if (cursor == strlen(text))
			list->add(GFXWC(list), GFXW(gfxw_new_line(Common::Point(zone.x, zone.y), Common::Point(zone.x, zone.y + cursor_height - 1),
			                            port->color, GFX_LINE_MODE_FAST, GFX_LINE_STYLE_NORMAL)));
		free(textdup);
	}

	zone.x = zone.y = 0;

	list->add(GFXWC(list), GFXW(gfxw_new_rect(zone, port->color, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL)));

	return list;
}

gfxw_list_t *sciw_new_icon_control(gfxw_port_t *port, reg_t ID, rect_t zone, int view, int loop, int cel,
                      char frame, char inverse) {
	gfxw_list_t *list = gfxw_new_list(_move_and_extend_rect(zone, Common::Point(port->zone.x, port->zone.y), 1), 0);
	gfxw_widget_t *icon;
	gfxw_set_id(GFXW(list), ID.segment, ID.offset);

	if (!port->visual) {
		GFXERROR("Attempting to create icon control for virtual port");
		return NULL;
	}

	zone.x = 0;
	zone.y = 0;

	icon = GFXW(gfxw_new_view(port->visual->gfx_state, Common::Point(zone.x, zone.y), view, loop, cel, 0, -1, -1,
	                          ALIGN_LEFT, ALIGN_TOP, GFXW_VIEW_FLAG_DONT_MODIFY_OFFSET));

	if (!icon) {
		GFXERROR("Attempt to create icon control with cel %d/%d/%d (invalid)\n", view, loop, cel);
		return NULL;
	}

	list->flags |= GFXW_FLAG_MULTI_ID;

	list->add(GFXWC(list), icon);

	return list;
}

gfxw_list_t *sciw_new_list_control(gfxw_port_t *port, reg_t ID, rect_t zone, int font_nr, char **entries_list,
                      int entries_nr, int list_top, int selection, char inverse) {
	gfxw_list_t *list;

	char arr_up[2], arr_down[2];
	int i;

	int font_height;
	int columns;

	zone.x--;
	zone.y--;
	zone.xl++;
	zone.yl++;

	list = gfxw_new_list(_move_and_extend_rect(zone, Common::Point(port->zone.x, port->zone.y), 1), 0);

	font_height = gfxop_get_font_height(port->visual->gfx_state, font_nr);
	columns = (zone.yl - 20);

	if (font_height <= 0) {
		GFXERROR("Attempt to create list control with invalid font %d\n", font_nr);
		list->widfree(GFXW(list));
		return NULL;
	}

	columns /= font_height;

	gfxw_set_id(GFXW(list), ID.segment, ID.offset);

	arr_up[0] = SCI_SPECIAL_CHAR_ARROW_UP;
	arr_down[0] = SCI_SPECIAL_CHAR_ARROW_DOWN;
	arr_up[1] = arr_down[1] = 0;

	zone.x = 1;
	zone.y = 11;

	// Draw text

	for (i = list_top; columns-- && i < entries_nr; i++) {
		if (i != selection)
			list->add(GFXWC(list),
			          GFXW(gfxw_new_text(port->visual->gfx_state, gfx_rect(zone.x, zone.y, zone.xl - 2, font_height),
			                             font_nr, entries_list[i], ALIGN_LEFT, ALIGN_TOP,
			                             port->color, port->color, port->bgcolor, GFXR_FONT_FLAG_NO_NEWLINES)));
		else {
			list->add(GFXWC(list), GFXW(gfxw_new_box(port->visual->gfx_state, gfx_rect(zone.x, zone.y, zone.xl - 1, font_height),
			                            port->color, port->color, GFX_BOX_SHADE_FLAT)));
			list->add(GFXWC(list), GFXW(gfxw_new_text(port->visual->gfx_state, gfx_rect(zone.x, zone.y, zone.xl - 2, font_height),
			                             font_nr, entries_list[i], ALIGN_LEFT, ALIGN_TOP,
			                             port->bgcolor, port->bgcolor, port->color, GFXR_FONT_FLAG_NO_NEWLINES)));
		}

		zone.y += font_height;
	}

	// Draw frames

	zone.x = 0;
	zone.y = 0;

	// Add up arrow
	list->add(GFXWC(list), GFXW(gfxw_new_text(port->visual->gfx_state, gfx_rect(1, 0, zone.xl - 2, 8),
	                             port->font_nr, arr_up, ALIGN_CENTER, ALIGN_CENTER,
	                             port->color, port->color, port->bgcolor, 0)));

	// Add down arrow
	list->add(GFXWC(list), GFXW(gfxw_new_text(port->visual->gfx_state, gfx_rect(1, zone.yl - 9, zone.xl - 2, 8),
	                             port->font_nr, arr_down, ALIGN_CENTER, ALIGN_CENTER,
	                             port->color, port->color, port->bgcolor, 0)));

	if (list_top & 1) { // Hack to work around aggressive caching
		list->add(GFXWC(list), GFXW(gfxw_new_rect(zone, port->color, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL)));
		list->add(GFXWC(list), GFXW(gfxw_new_rect(gfx_rect(zone.x, zone.y + 10, zone.xl, zone.yl - 20),
		                             port->color, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL)));
	} else {
		list->add(GFXWC(list),
		          GFXW(gfxw_new_rect(gfx_rect(zone.x, zone.y, zone.xl, zone.yl - 10),
		                             port->color, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL)));
		list->add(GFXWC(list),
		          GFXW(gfxw_new_rect(gfx_rect(zone.x, zone.y + 10, zone.xl, zone.yl - 10),
		                             port->color, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL)));
	}

	return list;
}

void sciw_set_menubar(state_t *s, gfxw_port_t *status_bar, menubar_t *menubar, int selection) {
	gfxw_list_t *list = make_titlebar_list(s, status_bar->bounds, status_bar);
	int offset = MENU_LEFT_BORDER;
	int i;

	clear_titlebar(status_bar);

	for (i = 0; i < menubar->menus_nr; i++) {
		menu_t *menu = menubar->menus + i;
		int width = menu->title_width + (MENU_BORDER_SIZE * 2);

		if (i == selection) {
			list->add(GFXWC(list), GFXW(gfxw_new_box(status_bar->visual->gfx_state, gfx_rect(offset, 0, width, MENU_BAR_HEIGHT),
			                            status_bar->color, status_bar->color, GFX_BOX_SHADE_FLAT)));
			list->add(GFXWC(list), GFXW(gfxw_new_text(s->gfx_state, gfx_rect(offset, 0, width, MENU_BAR_HEIGHT),
			                             status_bar->font_nr, menu->title, ALIGN_CENTER, ALIGN_CENTER,
			                             status_bar->bgcolor, status_bar->bgcolor, status_bar->color, GFXR_FONT_FLAG_NO_NEWLINES)));
		} else
			list->add(GFXWC(list), GFXW(gfxw_new_text(s->gfx_state, gfx_rect(offset, 0, width, MENU_BAR_HEIGHT),
			                             status_bar->font_nr, menu->title, ALIGN_CENTER, ALIGN_CENTER,
			                             status_bar->color, status_bar->color, status_bar->bgcolor, GFXR_FONT_FLAG_NO_NEWLINES)));
		offset += width;
	}

	status_bar->add(GFXWC(status_bar), GFXW(list));
	finish_titlebar_list(s, list, status_bar);
}

gfxw_port_t *sciw_new_menu(state_t *s, gfxw_port_t *status_bar, menubar_t *menubar, int selection) {
	gfxw_port_t *retval;
	menu_t *menu = menubar->menus + selection;
	rect_t area = gfx_rect(MENU_LEFT_BORDER, 10, 0, 0);
	int i;

	if (selection < -1)
		return NULL;

	if (selection >= menubar->menus_nr) {
		GFXERROR("Attempt to make menu #%d of %d\n", selection, menubar->menus_nr);
		return NULL;
	}

	for (i = 0; i < selection; i++)
		area.x += menubar->menus[i].title_width;

	area.xl = menu->width - 1;
	area.yl = menu->items_nr * 10;

	retval = sciw_new_window(s, area, status_bar->font_nr, status_bar->color, status_bar->bgcolor,
	                         0, status_bar->color, status_bar->bgcolor, NULL, WINDOW_FLAG_NO_DROP_SHADOW | WINDOW_FLAG_TRANSPARENT);

	retval->set_visual(GFXW(retval), s->visual);

	for (i = 0; i < menu->items_nr; i++)
		sciw_unselect_item(s, retval, menu, i);

	return retval;
}

#define MAGIC_ID_OFFSET 0x2000

static inline gfx_color_t un_prioritize(gfx_color_t col) {
	col.priority = -1;
	col.mask &= ~GFX_MASK_PRIORITY;

	return col;
}

gfxw_widget_t *_make_menu_entry(menu_item_t *item, int offset, int width, gfxw_port_t *port, gfx_color_t color, gfx_color_t bgcolor, int ID, int gray) {
	rect_t area = gfx_rect(MENU_BOX_LEFT_PADDING, 0, width - MENU_BOX_LEFT_PADDING, 10);
	rect_t list_area = gfx_rect(port->zone.x, area.y + offset + port->zone.y, width, area.yl);
	gfxw_list_t *list = (gfxw_list_t *) gfxw_set_id(GFXW(gfxw_new_list(list_area, 0)), ID, GFXW_NO_ID);
	gfx_color_t xcolor = {{0, 0, 0, 0}, 0, 0, 0, 0};

	color = un_prioritize(color);
	bgcolor = un_prioritize(bgcolor);

	xcolor = gray ? color : bgcolor;

	list->add(GFXWC(list), GFXW(gfxw_new_box(port->visual->gfx_state, area, bgcolor, bgcolor, GFX_BOX_SHADE_FLAT)));
	list->add(GFXWC(list), GFXW(gfxw_new_text(port->visual->gfx_state, area, port->font_nr, item->text, ALIGN_LEFT, ALIGN_CENTER,
	                            color, xcolor, bgcolor, GFXR_FONT_FLAG_NO_NEWLINES)));

	if (item->keytext) {
		area.xl -= MENU_BOX_RIGHT_PADDING;
		list->add(GFXWC(list), GFXW(gfxw_new_text(port->visual->gfx_state, area, port->font_nr, item->keytext, ALIGN_RIGHT, ALIGN_CENTER,
		                            color, xcolor, bgcolor, GFXR_FONT_FLAG_NO_NEWLINES)));
	}

	return GFXW(list);
}

gfxw_widget_t *_make_menu_hbar(int offset, int width, gfxw_port_t *port, gfx_color_t color, gfx_color_t bgcolor, int ID) {
	rect_t area = gfx_rect(0, 0, width, 10);
	rect_t list_area = gfx_rect(area.x + port->zone.x, area.y + offset + port->zone.y, area.xl, area.yl);
	gfxw_list_t *list = (gfxw_list_t *) gfxw_set_id(GFXW(gfxw_new_list(list_area, 0)), ID, GFXW_NO_ID);

	color = un_prioritize(color);
	bgcolor = un_prioritize(bgcolor);

	list->add(GFXWC(list), GFXW(gfxw_new_box(port->visual->gfx_state, area, bgcolor, bgcolor, GFX_BOX_SHADE_FLAT)));
	list->add(GFXWC(list), GFXW(gfxw_new_line(Common::Point(0, 5), Common::Point(width, 5), color,
	                            GFX_LINE_MODE_FAST, GFX_LINE_STYLE_STIPPLED)));

	return GFXW(list);
}

gfxw_port_t *sciw_unselect_item(state_t *s, gfxw_port_t *menu_port, menu_t *menu, int selection) {
	menu_item_t *item = menu->items + selection;

	if (selection < 0 || selection >= menu->items_nr)
		return menu_port;

	if (item->type == MENU_TYPE_NORMAL)
		menu_port->add(GFXWC(menu_port), GFXW(_make_menu_entry(item, selection * 10, menu_port->zone.xl + 1,
		                                      menu_port, menu_port->color, menu_port->bgcolor, selection + MAGIC_ID_OFFSET,
		                                      item->enabled)));
	else
		menu_port->add(GFXWC(menu_port), GFXW(_make_menu_hbar(selection * 10, menu_port->zone.xl + 1,
		                                      menu_port, menu_port->color, menu_port->bgcolor, selection + MAGIC_ID_OFFSET)));

	return menu_port;
}

gfxw_port_t *sciw_select_item(state_t *s, gfxw_port_t *menu_port, menu_t *menu, int selection) {
	menu_item_t *item = menu->items + selection;

	if (selection < 0 || selection >= menu->items_nr)
		return menu_port;

	if (item->type == MENU_TYPE_NORMAL)
		menu_port->add(GFXWC(menu_port), GFXW(_make_menu_entry(item, selection * 10, menu_port->zone.xl + 1,
		                                      menu_port, menu_port->bgcolor, menu_port->color, selection + MAGIC_ID_OFFSET,
		                                      item->enabled)));
	else
		menu_port->add(GFXWC(menu_port), GFXW(_make_menu_hbar(selection * 10, menu_port->zone.xl + 1,
		                                      menu_port, menu_port->bgcolor, menu_port->color, selection + MAGIC_ID_OFFSET)));

	return menu_port;
}

} // End of namespace Sci
