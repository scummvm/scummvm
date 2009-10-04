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

#include "common/util.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/tools.h"
#include "sci/debug.h"	// for g_debug_sleeptime_factor
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/gfx/gfx_gui.h"
#include "sci/gfx/gfx_widgets.h"
#include "sci/gfx/gfx_state_internal.h"	// required for GfxContainer, GfxPort, GfxVisual
#include "sci/gui32/gui32.h"

// This is the real width of a text with a specified width of 0
#define MAX_TEXT_WIDTH_MAGIC_VALUE 192

#define ADD_TO_CURRENT_PORT(widget) \
	{if (s->port)				   \
		s->port->add((GfxContainer *)s->port, widget); \
	else \
		s->picture_port->add((GfxContainer *)s->visual, widget);}

#define ADD_TO_CURRENT_PICTURE_PORT(widget) \
	{if (s->port)				   \
		s->port->add((GfxContainer *)s->port, widget); \
	else \
		s->picture_port->add((GfxContainer *)s->picture_port, widget);}

#define ADD_TO_WINDOW_PORT(widget) \
	s->wm_port->add((GfxContainer *)s->wm_port, widget);

#define FULL_REDRAW()\
	if (s->visual) \
		s->visual->draw(gfxw_point_zero); \
	gfxop_update(s->gfx_state);

#define K_DRAWPIC_FLAG_MIRRORED (1 << 14)

namespace Sci {

SciGUI32::SciGUI32(OSystem *system, EngineState *state)
	: _system(system), s(state) {
}

SciGUI32::~SciGUI32() {
}

void SciGUI32::init(bool oldGfxFunctions) {
	_usesOldGfxFunctions = oldGfxFunctions;
	activated_icon_bar = false;
	port_origin_x = 0;
	port_origin_y = 0;
}

int16 SciGUI32::getTimeTicks() {
	uint32 start_time;
	start_time = _system->getMillis() - s->game_start_time;
	return start_time * 60 / 1000;
}

void SciGUI32::wait(int16 ticks) {
	uint32 time;

	time = g_system->getMillis();
	s->r_acc = make_reg(0, ((long)time - (long)s->last_wait_time) * 60 / 1000);
	s->last_wait_time = time;

	ticks *= g_debug_sleeptime_factor;
	gfxop_sleep(s->gfx_state, ticks * 1000 / 60);


	// Reset speed throttler: Game is playing along nicely anyway
	if (ticks > 0)
		s->speedThrottler->reset();
}

void SciGUI32::setPort(uint16 portPtr) {
	GfxPort *new_port;

	/* We depart from official semantics here, sorry!
	   Reasoning: Sierra SCI does not clip ports while we do.
	   Therefore a draw to the titlebar port (which is the
	   official semantics) would cut off the lower part of the
	   icons in an SCI1 icon bar. Instead we have an
	   iconbar_port that does not exist in SSCI. */
	if (portPtr == 65535) portPtr = s->iconbar_port->_ID;

	new_port = s->visual->getPort(portPtr);

	if (!new_port) {
		warning("Invalid port %04x requested", portPtr);
		return;
	}

	s->port->draw(gfxw_point_zero); // Update the port we're leaving
	s->port = new_port;
}

void SciGUI32::setPortPic(Common::Rect rect, int16 picTop, int16 picLeft) {
	if (activated_icon_bar) {
		port_origin_x = port_origin_y = 0;
		activated_icon_bar = false;
		return;
	}
	port_origin_y = rect.top;
	port_origin_x = rect.left;

	if (rect.top == -10) {
		s->port->draw(gfxw_point_zero); // Update the port we're leaving
		s->port = s->iconbar_port;
		activated_icon_bar = true;
		return;
	}

	// Notify the graphics resource manager that the pic port bounds changed
	s->gfx_state->gfxResMan->changePortBounds(picLeft, picTop, rect.right + picLeft, rect.bottom + picTop);

	// LSL6 calls kSetPort to extend the screen to draw the GUI. If we free all resources
	// here, the background picture is freed too, and this makes everything a big mess.
	// FIXME/TODO: This code really needs to be rewritten to conform to the original behavior
	if (s->_gameName != "lsl6") {
		s->gfx_state->pic_port_bounds = gfx_rect(picLeft, picTop, rect.right, rect.bottom);

		// FIXME: Should really only invalidate all loaded pic resources here;
		// this is overkill
		s->gfx_state->gfxResMan->freeAllResources();
	} else {
		// WORKAROUND for LSL6
		warning("SetPort case 6 called in LSL6.");
	}
}

reg_t SciGUI32::getPort() {
	return make_reg(0, s->port->_ID);
}

void SciGUI32::globalToLocal(int16 *x, int16 *y) {
	*x = *x - s->port->zone.x;
	*y = *y - s->port->zone.y;
}

void SciGUI32::localToGlobal(int16 *x, int16 *y) {
	*x = *x + s->port->zone.x;
	*y = *y + s->port->zone.y;
}

reg_t SciGUI32::newWindow(Common::Rect rect1, Common::Rect rect2, uint16 style, int16 priority, int16 colorPen, int16 colorBack, const char *title) {
	GfxPort *window;
	int x, y, xl, yl;
	gfx_color_t bgcolor;
	gfx_color_t fgcolor;
	gfx_color_t black;
	gfx_color_t lWhite;

	y = rect1.top;
	x = rect1.left;
	yl = rect1.height();
	xl = rect1.width();

	y += s->wm_port->_bounds.y;

	if (x + xl > 319)
		x -= ((x + xl) - 319);

	bgcolor.mask = 0;

	if (colorBack >= 0) {
		if (!s->resMan->isVGA())
			bgcolor.visual = get_pic_color(s, MIN<int>(colorBack, 15));
		else
			bgcolor.visual = get_pic_color(s, colorBack);
		bgcolor.mask = GFX_MASK_VISUAL;
	} else {
		bgcolor.visual = PaletteEntry(0,0,0);
	}

	bgcolor.priority = priority;
	bgcolor.mask |= priority >= 0 ? GFX_MASK_PRIORITY : 0;
	bgcolor.alpha = 0;
	bgcolor.control = -1;
	debugC(2, kDebugLevelGraphics, "New window with params %d, %d, %d, %d\n", rect1.top, rect1.left, rect1.height(), rect1.width());

	fgcolor.visual = get_pic_color(s, colorPen);
	fgcolor.mask = GFX_MASK_VISUAL;
	fgcolor.control = -1;
	fgcolor.priority = -1;
	fgcolor.alpha = 0;
	black.visual = get_pic_color(s, 0);
	black.mask = GFX_MASK_VISUAL;
	black.alpha = 0;
	black.control = -1;
	black.priority = -1;
	lWhite.visual = get_pic_color(s, !s->resMan->isVGA() ? 15 : 255);
	lWhite.mask = GFX_MASK_VISUAL;
	lWhite.alpha = 0;
	lWhite.priority = -1;
	lWhite.control = -1;

	window = sciw_new_window(s, gfx_rect(x, y, xl, yl), s->titlebar_port->_font, fgcolor, bgcolor,
							s->titlebar_port->_font, lWhite, black, title ? s->strSplit(title, NULL).c_str() : NULL, style);

	// PQ3 and SCI1.1 games have the interpreter store underBits implicitly
	if (rect2.top != 0 && rect2.left != 0 && rect2.height() != 0 && rect2.width() != 0)
		gfxw_port_auto_restore_background(s->visual, window, gfx_rect(rect2.left, rect2.top + s->wm_port->_bounds.y, rect2.width(), rect2.height()));

	ADD_TO_WINDOW_PORT(window);
	FULL_REDRAW();

	window->draw(gfxw_point_zero);
	gfxop_update(s->gfx_state);

	s->port = window; // Set active port

	return make_reg(0, window->_ID);
}

void SciGUI32::disposeWindow(uint16 windowPtr, int16 arg2) {
	GfxPort *goner;
	GfxPort *pred;

	goner = s->visual->getPort(windowPtr);
	if ((windowPtr < 3) || (goner == NULL)) {
		error("Removal of invalid window %04x requested", windowPtr);
		return;
	}

	if (s->dyn_views && (GfxContainer *)s->dyn_views->_parent == (GfxContainer *)goner) {
		reparentize_primary_widget_lists(s, (GfxPort *) goner->_parent);
	}

	if (s->drop_views && (GfxContainer *)s->drop_views->_parent == (GfxContainer *)goner)
		s->drop_views = NULL; // Kill it

	pred = gfxw_remove_port(s->visual, goner);

	if (goner == s->port) // Did we kill the active port?
		s->port = pred;

	// Find the last port that exists and that isn't marked no-switch
	int id = s->visual->_portRefs.size() - 1;
	while (id > 0 && (!s->visual->_portRefs[id] || (s->visual->_portRefs[id]->_flags & GFXW_FLAG_NO_IMPLICIT_SWITCH)))
		id--;

	debugC(2, kDebugLevelGraphics, "Activating port %d after disposing window %d\n", id, windowPtr);
	s->port = (id >= 0) ? s->visual->_portRefs[id] : 0;

	if (!s->port)
		s->port = gfxw_find_default_port(s->visual);

	gfxop_update(s->gfx_state);
}

#define K_DISPLAY_SET_COORDS 100
#define K_DISPLAY_SET_ALIGNMENT 101
#define K_DISPLAY_SET_COLOR 102
#define K_DISPLAY_SET_BGCOLOR 103
#define K_DISPLAY_SET_GRAYTEXT 104
#define K_DISPLAY_SET_FONT 105
#define K_DISPLAY_WIDTH 106
#define K_DISPLAY_SAVE_UNDER 107
#define K_DISPLAY_RESTORE_UNDER 108
#define K_DONT_UPDATE_IMMEDIATELY 121

void SciGUI32::display(const char *text, int argc, reg_t *argv) {
	int argpt = 0;
	int temp;
	bool save_under = false;
	gfx_color_t transparent = { PaletteEntry(), 0, -1, -1, 0 };
	GfxPort *port = (s->port) ? s->port : s->picture_port;
	bool update_immediately = true;

	gfx_color_t color0, *color1, bg_color;
	gfx_alignment_t halign = ALIGN_LEFT;
	rect_t area = gfx_rect(port->draw_pos.x, port->draw_pos.y, 320 - port->draw_pos.x, 200 - port->draw_pos.y);
	int gray = port->gray_text;
	int font_nr = port->_font;
	GfxText *text_handle;

	color0 = port->_color;
	bg_color = port->_bgcolor;
	// TODO: in SCI1VGA the default colors for text and background are #0 (black)
	// SCI0 case should be checked
	if (s->resMan->isVGA()) {
		// This priority check fixes the colors in the menus in KQ5
		// TODO/FIXME: Is this correct?
		if (color0.priority >= 0)
			color0.visual = get_pic_color(s, 0);
		if (bg_color.priority >= 0)
			bg_color.visual = get_pic_color(s, 0);
	}

	while (argpt < argc) {
		switch (argv[argpt++].toUint16()) {

		case K_DISPLAY_SET_COORDS:

			area.x = argv[argpt++].toUint16();
			area.y = argv[argpt++].toUint16();
			debugC(2, kDebugLevelGraphics, "Display: set_coords(%d, %d)\n", area.x, area.y);
			break;

		case K_DISPLAY_SET_ALIGNMENT:

			halign = (gfx_alignment_t)argv[argpt++].toSint16();
			debugC(2, kDebugLevelGraphics, "Display: set_align(%d)\n", halign);
			break;

		case K_DISPLAY_SET_COLOR:

			temp = argv[argpt++].toSint16();
			debugC(2, kDebugLevelGraphics, "Display: set_color(%d)\n", temp);
			if (!s->resMan->isVGA() && temp >= 0 && temp <= 15)
				color0 = (s->ega_colors[temp]);
			else
				if (s->resMan->isVGA() && temp >= 0 && temp < 256) {
					color0.visual = get_pic_color(s, temp);
					color0.mask = GFX_MASK_VISUAL;
				} else
					if (temp == -1)
						color0 = transparent;
					else
						warning("Display: Attempt to set invalid fg color %d", temp);
			break;

		case K_DISPLAY_SET_BGCOLOR:

			temp = argv[argpt++].toSint16();
			debugC(2, kDebugLevelGraphics, "Display: set_bg_color(%d)\n", temp);
			if (!s->resMan->isVGA() && temp >= 0 && temp <= 15)
				bg_color = s->ega_colors[temp];
			else
				if (s->resMan->isVGA() && temp >= 0 && temp <= 256) {
					bg_color.visual = get_pic_color(s, temp);
					bg_color.mask = GFX_MASK_VISUAL;
				} else
					if (temp == -1)
						bg_color = transparent;
					else
						warning("Display: Attempt to set invalid fg color %d", temp);
			break;

		case K_DISPLAY_SET_GRAYTEXT:

			gray = argv[argpt++].toSint16();
			debugC(2, kDebugLevelGraphics, "Display: set_graytext(%d)\n", gray);
			break;

		case K_DISPLAY_SET_FONT:

			font_nr = argv[argpt++].toUint16();

			debugC(2, kDebugLevelGraphics, "Display: set_font(\"font.%03d\")\n", font_nr);
			break;

		case K_DISPLAY_WIDTH:

			area.width = argv[argpt++].toUint16();
			if (area.width == 0)
				area.width = MAX_TEXT_WIDTH_MAGIC_VALUE;

			debugC(2, kDebugLevelGraphics, "Display: set_width(%d)\n", area.width);
			break;

		case K_DISPLAY_SAVE_UNDER:

			save_under = true;
			debugC(2, kDebugLevelGraphics, "Display: set_save_under()\n");
			break;

		case K_DISPLAY_RESTORE_UNDER:

			debugC(2, kDebugLevelGraphics, "Display: restore_under(%04x)\n", argv[argpt].toUint16());
			graph_restore_box(s, argv[argpt++]);
			update_immediately = true;
			argpt++;
			return;

		case K_DONT_UPDATE_IMMEDIATELY:

			update_immediately = false;
			debugC(2, kDebugLevelGraphics, "Display: set_dont_update()\n");
			argpt++;
			break;

		default:
			debugC(2, kDebugLevelGraphics, "Unknown Display() command %x\n", argv[argpt - 1].toUint16());
			return;
		}
	}

	if (halign == ALIGN_LEFT) {
		// If the text does not fit on the screen, move it to the left and upwards until it does
		gfxop_get_text_params(s->gfx_state, font_nr, text, area.width, &area.width, &area.height, 0, NULL, NULL, NULL);

		// Make the text fit on the screen
		if (area.x + area.width > 320)
			area.x += 320 - area.x - area.width; // Plus negative number = subtraction

		if (area.y + area.height > 200)
			area.y += 200 - area.y - area.height; // Plus negative number = subtraction
	} else {
		// If the text does not fit on the screen, clip it till it does
		if (area.x + area.width > s->gfx_state->pic_port_bounds.width)
			area.width = s->gfx_state->pic_port_bounds.width - area.x;

		if (area.y + area.height > s->gfx_state->pic_port_bounds.height)
			area.height = s->gfx_state->pic_port_bounds.height - area.y;
	}

	if (gray)
		color1 = &bg_color;
	else
		color1 = &color0;

	assert_primary_widget_lists(s);

	text_handle = gfxw_new_text(s->gfx_state, area, font_nr, s->strSplit(text).c_str(), halign, ALIGN_TOP, color0, *color1, bg_color, 0);

	if (!text_handle) {
		error("Display: Failed to create text widget");
		return;
	}

	if (save_under) {    // Backup
		rect_t save_area = text_handle->_bounds;
		save_area.x += port->_bounds.x;
		save_area.y += port->_bounds.y;

		s->r_acc = graph_save_box(s, save_area);
		text_handle->_serial++; // This is evil!

		debugC(2, kDebugLevelGraphics, "Saving (%d, %d) size (%d, %d) as %04x:%04x\n", save_area.x, save_area.y, save_area.width, save_area.height, PRINT_REG(s->r_acc));
	}

	debugC(2, kDebugLevelGraphics, "Display: Commiting text '%s'\n", text);

	//ADD_TO_CURRENT_PICTURE_PORT(text_handle);

	ADD_TO_CURRENT_PICTURE_PORT(text_handle);
	if ((!s->pic_not_valid) && update_immediately) { // Refresh if drawn to valid picture
		FULL_REDRAW();
		debugC(2, kDebugLevelGraphics, "Refreshing display...\n");
	}
}

void SciGUI32::textSize(const char *text, int16 fontId, int16 maxWidth, int16 *textWidth, int16 *textHeight) {
	int width, height;
	if (maxWidth < 0)
		maxWidth = 0;
	gfxop_get_text_params(s->gfx_state, fontId, text, maxWidth ? maxWidth : MAX_TEXT_WIDTH_MAGIC_VALUE,
	                                 &width, &height, 0, NULL, NULL, NULL);
	*textWidth = width; *textHeight = height;
}

void SciGUI32::textFonts(int argc, reg_t *argv) {
	// stub
}

void SciGUI32::textColors(int argc, reg_t *argv) {
	// stub
}

void SciGUI32::drawPicture(sciResourceId pictureId, uint16 showStyle, uint16 flags, int16 EGApaletteNo) {
	drawn_pic_t dp;
	gfx_color_t transparent = s->wm_port->_bgcolor;
	int picFlags = DRAWPIC01_FLAG_FILL_NORMALLY;
	bool add_to_pic = flags;

	dp.nr = pictureId;
	if (EGApaletteNo != -1) {
		dp.palette = EGApaletteNo;
	} else {
		dp.palette = 0;
	}

	if (showStyle & K_DRAWPIC_FLAG_MIRRORED)
		picFlags |= DRAWPIC1_FLAG_MIRRORED;

	gfxop_disable_dirty_frames(s->gfx_state);

	if (NULL != s->old_screen) {
		gfxop_free_pixmap(s->gfx_state, s->old_screen);
	}

	s->old_screen = gfxop_grab_pixmap(s->gfx_state, gfx_rect(0, 10, 320, 190));

	debugC(2, kDebugLevelGraphics, "Drawing pic.%03d\n", pictureId);
	if (add_to_pic) {
		gfxop_add_to_pic(s->gfx_state, dp.nr, picFlags, dp.palette);
	} else {
		gfxop_new_pic(s->gfx_state, dp.nr, picFlags, dp.palette);
	}

	delete s->wm_port;
	delete s->picture_port;
	delete s->iconbar_port;

	s->wm_port = new GfxPort(s->visual, s->gfx_state->pic_port_bounds, s->ega_colors[0], transparent);
	s->picture_port = new GfxPort(s->visual, s->gfx_state->pic_port_bounds, s->ega_colors[0], transparent);

	s->iconbar_port = new GfxPort(s->visual, gfx_rect(0, 0, 320, 200), s->ega_colors[0], transparent);
	s->iconbar_port->_flags |= GFXW_FLAG_NO_IMPLICIT_SWITCH;

	s->visual->add((GfxContainer *)s->visual, s->picture_port);
	s->visual->add((GfxContainer *)s->visual, s->wm_port);
	s->visual->add((GfxContainer *)s->visual, s->iconbar_port);

	s->port = s->picture_port;

	s->pic_priority_table = gfxop_get_pic_metainfo(s->gfx_state);

	s->pic_animate = showStyle & 0xff; // The animation used during kAnimate() later on

	s->dyn_views = NULL;
	s->drop_views = NULL;

	s->priority_first = 42;

	if (_usesOldGfxFunctions)
		s->priority_last = 200;
	else
		s->priority_last = 190;

	s->pic_not_valid = 1;
	s->pic_is_new = 1;
}

void SciGUI32::drawCell(sciResourceId viewId, uint16 loopNo, uint16 cellNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo) {
	int loop = loopNo;
	int cel = cellNo;
	GfxView *new_view;

	gfxop_check_cel(s->gfx_state, viewId, &loop, &cel);

	debugC(2, kDebugLevelGraphics, "DrawCel((%d,%d), (view.%d, %d, %d), p=%d)\n", leftPos, topPos, viewId, loop, cel, priority);

	new_view = gfxw_new_view(s->gfx_state, Common::Point(leftPos, topPos), viewId, loop, cel, 0, priority, -1,
	                         ALIGN_LEFT, ALIGN_TOP, GFXW_VIEW_FLAG_DONT_MODIFY_OFFSET);

	ADD_TO_CURRENT_PICTURE_PORT(new_view);
	FULL_REDRAW();
}

void SciGUI32::drawControlButton(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 style, bool inverse) {
	rect_t area = gfx_rect(rect.left, rect.top, rect.width(), rect.height());

	ADD_TO_CURRENT_PICTURE_PORT(sciw_new_button_control(s->port, obj, area, text, fontId,
		(int8)(style & kControlStateFramed), (int8)inverse, (int8)(style & kControlStateDisabled)));
	if (!s->pic_not_valid) FULL_REDRAW();
}

void SciGUI32::drawControlText(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 mode, int16 style, bool inverse) {
	rect_t area = gfx_rect(rect.left, rect.top, rect.width(), rect.height());

	ADD_TO_CURRENT_PICTURE_PORT(sciw_new_text_control(s->port, obj, area, text, fontId, (gfx_alignment_t) mode,
								(int8)(!!(style & kControlStateDitherFramed)), (int8)inverse));
	if (!s->pic_not_valid) FULL_REDRAW();
}

static gfx_color_t graph_map_color(EngineState *s, int color, int priority, int control) {
	gfx_color_t retval;

	if (!s->resMan->isVGA()) {
		retval = s->ega_colors[(color >=0 && color < 16)? color : 0];
		gfxop_set_color(s->gfx_state, &retval, (color < 0) ? -1 : retval.visual.r, retval.visual.g, retval.visual.b,
		                (color == -1) ? 255 : 0, priority, control);
	} else {
		retval.visual = get_pic_color(s, color);
		retval.alpha = 0;
		retval.priority = priority;
		retval.control = control;
		retval.mask = GFX_MASK_VISUAL | ((priority >= 0) ? GFX_MASK_PRIORITY : 0) | ((control >= 0) ? GFX_MASK_CONTROL : 0);
	};

	return retval;
}

void _k_graph_rebuild_port_with_color(EngineState *s, gfx_color_t newbgcolor) {
	GfxPort *port = s->port;
	GfxPort *newport;

	newport = sciw_new_window(s, port->zone, port->_font, port->_color, newbgcolor,
	                          s->titlebar_port->_font, s->ega_colors[15], s->ega_colors[8],
	                          port->_title_text.c_str(), port->port_flags & ~kWindowTransparent);

	if (s->dyn_views) {
		int found = 0;
		GfxContainer *parent = s->dyn_views->_parent;

		while (parent && !(found |= (parent == port)))
			parent = parent->_parent;

		s->dyn_views = NULL;
	}

	port->_parent->add((GfxContainer *)port->_parent, newport);
	delete port;
}

void SciGUI32::graphFillBoxForeground(Common::Rect rect) {
	_k_graph_rebuild_port_with_color(s, s->port->_color);
	//port = _s->port;

	FULL_REDRAW();
}

void SciGUI32::graphFillBoxBackground(Common::Rect rect) {
	_k_graph_rebuild_port_with_color(s, s->port->_bgcolor);
	//port = _s->port;

	FULL_REDRAW();
}

void SciGUI32::graphFillBox(Common::Rect rect, uint16 colorMask, int16 color, int16 priority, int16 control) {
	gfx_color_t fillColor = graph_map_color(s, color, priority, control);
	fillColor.mask = (byte)colorMask;
	rect_t area = gfx_rect(rect.left, rect.top, rect.width(), rect.height());

	//debugC(2, kDebugLevelGraphics, "fill_box_any((%d, %d), (%d, %d), col=%d, p=%d, c=%d, mask=%d)\n",
	//          argv[2].toSint16(), argv[1].toSint16(), argv[4].toSint16(), argv[3].toSint16(), argv[6].toSint16(), priority, control, argv[5].toUint16());

	// FIXME/TODO: this is not right, as some of the dialogs are drawn *behind* some widgets. But at least it works for now
	//ADD_TO_CURRENT_PICTURE_PORT(gfxw_new_box(s->gfx_state, area, color, color, GFX_BOX_SHADE_FLAT));	// old code

	// FillBox seems to be meant again s->port instead of s->picture_port, at least in QfG3
//		warning("Fillbox");
//		ADD_TO_CURRENT_PICTURE_PORT(gfxw_new_box(s->gfx_state, area, color, color, GFX_BOX_SHADE_FLAT));
	s->picture_port->add((GfxContainer *)s->picture_port, gfxw_new_box(s->gfx_state, area, fillColor, fillColor, GFX_BOX_SHADE_FLAT));
}

void SciGUI32::graphDrawLine(Common::Rect rect, int16 color, int16 priority, int16 control) {
	gfx_color_t gfxcolor = graph_map_color(s, color, priority, control);

	debugC(2, kDebugLevelGraphics, "draw_line((%d, %d), (%d, %d), col=%d, p=%d, c=%d, mask=%d)\n",
	          rect.left, rect.top, rect.right, rect.bottom, color, priority, control, gfxcolor.mask);

	// Note: it's quite possible that the coordinates of the line will *not* form a valid rectangle (e.g. it might
	// have negative width/height). The actual dirty rectangle is constructed in gfxdr_add_dirty().
	// FIXME/TODO: We need to change the semantics of this call, so that no fake rectangles are used. As it is, it's
	// not possible change rect_t to Common::Rect, as we assume that Common::Rect forms a *valid* rectangle.
	ADD_TO_CURRENT_PICTURE_PORT(gfxw_new_line(Common::Point(rect.left, rect.top), Common::Point(rect.right, rect.bottom),
	                               gfxcolor, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL));
	FULL_REDRAW();
}

reg_t SciGUI32::graphSaveBox(Common::Rect rect, uint16 flags) {
	rect_t area;
	area.x = rect.left + s->port->zone.x + port_origin_x;
	area.y = rect.top + s->port->zone.y + port_origin_y;
	area.width = rect.width() - port_origin_x;
	area.height = rect.height() - port_origin_y;

	return graph_save_box(s, area);
}

void SciGUI32::graphRestoreBox(reg_t handle) {
	graph_restore_box(s, handle);
}

void SciGUI32::paletteSet(int resourceNo, int flags) {
	//warning("STUB");
}

int16 SciGUI32::paletteFind(int r, int g, int b) {
	int i, delta, bestindex = -1, bestdelta = 200000;

	for (i = 0; i < s->gfx_state->gfxResMan->getColorCount(); i++) {
		int dr = abs(s->gfx_state->gfxResMan->getColor(i).r - r);
		int dg = abs(s->gfx_state->gfxResMan->getColor(i).g - g);
		int db = abs(s->gfx_state->gfxResMan->getColor(i).b - b);

		delta = dr * dr + dg * dg + db * db;

		if (delta < bestdelta) {
			bestdelta = delta;
			bestindex = i;
		}
	}
	// Don't warn about inexact mappings -- it's actually the
	// rule rather than the exception
	return bestindex;
}

void SciGUI32::paletteAnimate(int fromColor, int toColor, int speed) {
	warning("STUB");
}

int16 SciGUI32::onControl(byte screenMask, Common::Rect rect) {
	gfx_map_mask_t map = (gfx_map_mask_t)screenMask;
	rect_t gfxrect = gfx_rect(rect.left, rect.top + 10, rect.width(), rect.height());

	return gfxop_scan_bitmask(s->gfx_state, gfxrect, map);
// old code
//	int xstart, ystart;
//	int xlen = 1, ylen = 1;
//
//	if (argc == 2 || argc == 4)
//		map = GFX_MASK_CONTROL;
//	else {
//		arg = 1;
//		map = (gfx_map_mask_t) argv[0].toSint16();
//	}
//
//	ystart = argv[arg + 1].toSint16();
//	xstart = argv[arg].toSint16();
//
//	if (argc > 3) {
//		ylen = argv[arg + 3].toSint16() - ystart;
//		xlen = argv[arg + 2].toSint16() - xstart;
//	}
//
//	return make_reg(0, gfxop_scan_bitmask(s->gfx_state, gfx_rect(xstart, ystart + 10, xlen, ylen), map));
}

void SciGUI32::moveCursor(int16 x, int16 y) {
	Common::Point newPos;

	// newPos = s->gfx_state->pointer_pos;

	newPos.x = x + s->port->zone.x;
	newPos.y = y + s->port->zone.y;

	if (newPos.x > s->port->zone.x + s->port->zone.width)
		newPos.x = s->port->zone.x + s->port->zone.width;
	if (newPos.y > s->port->zone.y + s->port->zone.height)
		newPos.y = s->port->zone.y + s->port->zone.height;

	if (newPos.x < 0) newPos.x = 0;
	if (newPos.y < 0) newPos.y = 0;
	gfxop_set_pointer_position(s->gfx_state, newPos);
}

} // End of namespace Sci
