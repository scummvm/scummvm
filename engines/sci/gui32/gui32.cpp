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

SciGui32::SciGui32(OSystem *system, EngineState *state, SciGuiScreen *screen, SciGuiPalette *palette)
	: _system(system), s(state) {
}

SciGui32::~SciGui32() {
}

void SciGui32::init(bool oldGfxFunctions) {
	_usesOldGfxFunctions = oldGfxFunctions;
	_k_animate_ran = false;
	activated_icon_bar = false;
	port_origin_x = 0;
	port_origin_y = 0;
}

void SciGui32::wait(int16 ticks) {
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

void SciGui32::setPort(uint16 portPtr) {
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

void SciGui32::setPortPic(Common::Rect rect, int16 picTop, int16 picLeft) {
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

	// LSL6 calls kSetPort to extend the screen to draw the Gui. If we free all resources
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

reg_t SciGui32::getPort() {
	return make_reg(0, s->port->_ID);
}

void SciGui32::globalToLocal(int16 *x, int16 *y) {
	*x = *x - s->port->zone.x;
	*y = *y - s->port->zone.y;
}

void SciGui32::localToGlobal(int16 *x, int16 *y) {
	*x = *x + s->port->zone.x;
	*y = *y + s->port->zone.y;
}

reg_t SciGui32::newWindow(Common::Rect dims, Common::Rect restoreRect, uint16 style, int16 priority, int16 colorPen, int16 colorBack, const char *title) {
	GfxPort *window;
	int x, y, xl, yl;
	gfx_color_t bgcolor;
	gfx_color_t fgcolor;
	gfx_color_t black;
	gfx_color_t lWhite;

	y = dims.top;
	x = dims.left;
	yl = dims.height();
	xl = dims.width();

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
	debugC(2, kDebugLevelGraphics, "New window with params %d, %d, %d, %d\n", dims.top, dims.left, dims.height(), dims.width());

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
	if (restoreRect.top != 0 && restoreRect.left != 0 && restoreRect.height() != 0 && restoreRect.width() != 0)
		gfxw_port_auto_restore_background(s->visual, window, gfx_rect(restoreRect.left, restoreRect.top + s->wm_port->_bounds.y, restoreRect.width(), restoreRect.height()));

	ADD_TO_WINDOW_PORT(window);
	FULL_REDRAW();

	window->draw(gfxw_point_zero);
	gfxop_update(s->gfx_state);

	s->port = window; // Set active port

	return make_reg(0, window->_ID);
}

void SciGui32::disposeWindow(uint16 windowPtr, int16 arg2) {
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

void SciGui32::display(const char *text, int argc, reg_t *argv) {
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

void SciGui32::textSize(const char *text, int16 fontId, int16 maxWidth, int16 *textWidth, int16 *textHeight) {
	int width, height;
	if (maxWidth < 0)
		maxWidth = 0;
	gfxop_get_text_params(s->gfx_state, fontId, text, maxWidth ? maxWidth : MAX_TEXT_WIDTH_MAGIC_VALUE,
	                                 &width, &height, 0, NULL, NULL, NULL);
	*textWidth = width; *textHeight = height;
}

void SciGui32::textFonts(int argc, reg_t *argv) {
	// stub
}

void SciGui32::textColors(int argc, reg_t *argv) {
	// stub
}

void SciGui32::drawStatus(const char *text, int16 colorPen, int16 colorBack) {
	s->titlebar_port->_color.visual = get_pic_color(s, colorPen);
	s->titlebar_port->_color.mask = GFX_MASK_VISUAL;
	s->titlebar_port->_bgcolor.visual = get_pic_color(s, colorBack);
	s->titlebar_port->_bgcolor.mask = GFX_MASK_VISUAL;

	s->status_bar_foreground = colorPen;
	s->status_bar_background = colorBack;
	s->_statusBarText = text;

	sciw_set_status_bar(s, s->titlebar_port, s->_statusBarText, colorPen, colorBack);

	gfxop_update(s->gfx_state);
}

void SciGui32::drawPicture(GuiResourceId pictureId, uint16 showStyle, uint16 flags, int16 EGApaletteNo) {
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

void SciGui32::drawCel(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo) {
	int loop = loopNo;
	int cel = celNo;
	GfxView *new_view;

	gfxop_check_cel(s->gfx_state, viewId, &loop, &cel);

	debugC(2, kDebugLevelGraphics, "DrawCel((%d,%d), (view.%d, %d, %d), p=%d)\n", leftPos, topPos, viewId, loop, cel, priority);

	new_view = gfxw_new_view(s->gfx_state, Common::Point(leftPos, topPos), viewId, loop, cel, 0, priority, -1,
	                         ALIGN_LEFT, ALIGN_TOP, GFXW_VIEW_FLAG_DONT_MODIFY_OFFSET);

	ADD_TO_CURRENT_PICTURE_PORT(new_view);
	FULL_REDRAW();
}

void SciGui32::drawControlButton(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 style, bool hilite) {
	rect_t area = gfx_rect(rect.left, rect.top, rect.width(), rect.height());

	ADD_TO_CURRENT_PICTURE_PORT(sciw_new_button_control(s->port, obj, area, text, fontId,
		(int8)(style & kControlStateFramed), (int8)hilite, (int8)(style & kControlStateDisabled)));
	if (!s->pic_not_valid) FULL_REDRAW();
}

void SciGui32::drawControlText(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 mode, int16 style, bool hilite) {
	rect_t area = gfx_rect(rect.left, rect.top, rect.width(), rect.height());

	ADD_TO_CURRENT_PICTURE_PORT(sciw_new_text_control(s->port, obj, area, text, fontId, (gfx_alignment_t) mode,
								(int8)(!!(style & kControlStateDitherFramed)), (int8)hilite));
	if (!s->pic_not_valid) FULL_REDRAW();
}

void SciGui32::drawControlIcon(Common::Rect rect, reg_t obj, GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo cellNo, int16 style, bool hilite) {
	rect_t area = gfx_rect(rect.left, rect.top, rect.width(), rect.height());

	ADD_TO_CURRENT_PICTURE_PORT(sciw_new_icon_control(s->port, obj, area, viewId, loopNo, cellNo,
								(int8)(style & kControlStateFramed), (int8)hilite));
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

void SciGui32::graphFillBoxForeground(Common::Rect rect) {
	_k_graph_rebuild_port_with_color(s, s->port->_color);
	//port = _s->port;

	FULL_REDRAW();
}

void SciGui32::graphFillBoxBackground(Common::Rect rect) {
	_k_graph_rebuild_port_with_color(s, s->port->_bgcolor);
	//port = _s->port;

	FULL_REDRAW();
}

void SciGui32::graphFillBox(Common::Rect rect, uint16 colorMask, int16 color, int16 priority, int16 control) {
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

void SciGui32::graphDrawLine(Common::Point startPoint, Common::Point endPoint, int16 color, int16 priority, int16 control) {
	gfx_color_t gfxcolor = graph_map_color(s, color, priority, control);

	debugC(2, kDebugLevelGraphics, "draw_line((%d, %d), (%d, %d), col=%d, p=%d, c=%d, mask=%d)\n",
	          startPoint.x, startPoint.y, endPoint.x, endPoint.y, color, priority, control, gfxcolor.mask);

	ADD_TO_CURRENT_PICTURE_PORT(gfxw_new_line(startPoint, endPoint,
	                               gfxcolor, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL));
	FULL_REDRAW();
}

reg_t SciGui32::graphSaveBox(Common::Rect rect, uint16 flags) {
	rect_t area;
	area.x = rect.left + s->port->zone.x + port_origin_x;
	area.y = rect.top + s->port->zone.y + port_origin_y;
	area.width = rect.width() - port_origin_x;
	area.height = rect.height() - port_origin_y;

	return graph_save_box(s, area);
}

void SciGui32::graphRestoreBox(reg_t handle) {
	graph_restore_box(s, handle);
}

void SciGui32::paletteSet(int resourceNo, int flags) {
	//warning("STUB");
}

int16 SciGui32::paletteFind(int r, int g, int b) {
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

void SciGui32::paletteSetIntensity(int fromColor, int toColor, int intensity, bool setPalette) {
#if 0
	s->gfx_state->gfxResMan->setPaletteIntensity(fromColor, toColor, intensity);
#endif
}

void SciGui32::paletteAnimate(int fromColor, int toColor, int speed) {
	//warning("STUB");
}

int16 SciGui32::onControl(byte screenMask, Common::Rect rect) {
	gfx_map_mask_t map = (gfx_map_mask_t)screenMask;
	rect_t gfxrect = gfx_rect(rect.left, rect.top + 10, rect.width(), rect.height());

	return gfxop_scan_bitmask(s->gfx_state, gfxrect, map);
// old code, just for reference
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

enum {
	_K_MAKE_VIEW_LIST_CYCLE = 1,
	_K_MAKE_VIEW_LIST_CALC_PRIORITY = 2,
	_K_MAKE_VIEW_LIST_DRAW_TO_CONTROL_MAP = 4
};

static inline int sign_extend_byte(int value) {
	if (value & 0x80)
		return value - 256;
	else
		return value;
}

static Common::Rect nsrect_clip(EngineState *s, int y, Common::Rect retval, int priority) {
	int pri_top;

	if (priority == -1)
		priority = _find_view_priority(s, y);

	pri_top = _find_priority_band(s, priority) + 1;
	// +1: Don't know why, but this seems to be happening

	if (retval.top < pri_top)
		retval.top = pri_top;

	if (retval.bottom < retval.top)
		retval.top = retval.bottom - 1;

	return retval;
}

static Common::Rect calculate_nsrect(EngineState *s, int x, int y, int view, int loop, int cel) {
	int xbase, ybase, xend, yend, xsize, ysize;
	int xmod = 0, ymod = 0;
	Common::Rect retval(0, 0, 0, 0);

	Common::Point offset = Common::Point(0, 0);

	gfxop_get_cel_parameters(s->gfx_state, view, loop, cel, &xsize, &ysize, &offset);

	xmod = offset.x;
	ymod = offset.y;

	xbase = x - xmod - (xsize >> 1);
	xend = xbase + xsize;
	yend = y - ymod + 1; // +1: magic modifier
	ybase = yend - ysize;

	retval.left = xbase;
	retval.top = ybase;
	retval.right = xend;
	retval.bottom = yend;

	return retval;
}

Common::Rect get_nsrect32(EngineState *s, reg_t object, byte clip) {
	SegManager *segMan = s->_segMan;
	int x, y, z;
	int view, loop, cel;
	Common::Rect retval;

	x = (int16)GET_SEL32V(object, x);
	y = (int16)GET_SEL32V(object, y);

	if (s->_kernel->_selectorCache.z > -1)
		z = (int16)GET_SEL32V(object, z);
	else
		z = 0;

	y -= z; // Subtract z offset

	view = (int16)GET_SEL32V(object, view);
	loop = sign_extend_byte((int16)GET_SEL32V(object, loop));
	cel = sign_extend_byte((int16)GET_SEL32V(object, cel));

	retval = calculate_nsrect(s, x, y, view, loop, cel);

	if (clip) {
		int priority = (int16)GET_SEL32V(object, priority);
		return nsrect_clip(s, y, retval, priority);
	}

	return retval;
}

GfxDynView *SciGui32::_k_make_dynview_obj(reg_t obj, int options, int nr, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	short oldloop, oldcel;
	int cel, loop, view_nr = (int16)GET_SEL32V(obj, view);
	int palette;
	int signal;
	reg_t under_bits;
	Common::Point pos;
	int z;
	GfxDynView *widget;

	debugC(2, kDebugLevelGraphics, " - Adding %04x:%04x\n", PRINT_REG(obj));

	obj = obj;

	pos.x = (int16)GET_SEL32V(obj, x);
	pos.y = (int16)GET_SEL32V(obj, y);

	pos.y++; // magic: Sierra appears to do something like this

	z = (int16)GET_SEL32V(obj, z);

	// !-- nsRect used to be checked here!
	loop = oldloop = sign_extend_byte(GET_SEL32V(obj, loop));
	cel = oldcel = sign_extend_byte(GET_SEL32V(obj, cel));

	if (s->_kernel->_selectorCache.palette)
		palette = GET_SEL32V(obj, palette);
	else
		palette = 0;

	// Clip loop and cel, write back if neccessary
	gfxop_check_cel(s->gfx_state, view_nr, &loop, &cel);

	if (loop != oldloop)
		loop = 0;
	if (cel != oldcel)
		cel = 0;

	if (oldloop != loop)
		PUT_SEL32V(obj, loop, loop);

	if (oldcel != cel) {
		PUT_SEL32V(obj, cel, cel);
	}

	ObjVarRef under_bitsp;
	if (lookup_selector(s->_segMan, obj, s->_kernel->_selectorCache.underBits, &(under_bitsp), NULL) != kSelectorVariable) {
		under_bitsp.obj = NULL_REG;
		under_bits = NULL_REG;
		debugC(2, kDebugLevelGraphics, "Object at %04x:%04x has no underBits\n", PRINT_REG(obj));
	} else
		under_bits = *under_bitsp.getPointer(s->_segMan);

	ObjVarRef signalp;
	if (lookup_selector(s->_segMan, obj, s->_kernel->_selectorCache.signal, &(signalp), NULL) != kSelectorVariable) {
		signalp.obj = NULL_REG;
		signal = 0;
		debugC(2, kDebugLevelGraphics, "Object at %04x:%04x has no signal selector\n", PRINT_REG(obj));
	} else {
		signal = signalp.getPointer(s->_segMan)->offset;
		debugC(2, kDebugLevelGraphics, "    with signal = %04x\n", signal);
	}

	widget = gfxw_new_dyn_view(s->gfx_state, pos, z, view_nr, loop, cel, palette, -1, -1, ALIGN_CENTER, ALIGN_BOTTOM, nr);

	if (widget) {
		widget = (GfxDynView *) gfxw_set_id(widget, obj.segment, obj.offset);
		widget = gfxw_dyn_view_set_params(widget, under_bits.segment, under_bitsp, signal, signalp);
		widget->_flags |= GFXW_FLAG_IMMUNE_TO_SNAPSHOTS; // Only works the first time 'round'

		return widget;
	} else {
		warning("Could not generate dynview widget for %d/%d/%d", view_nr, loop, cel);
		return NULL;
	}
}

void SciGui32::_k_make_view_list(GfxList **widget_list, List *list, int options, int argc, reg_t *argv) {
/* Creates a view_list from a node list in heap space. Returns the list, stores the
** number of list entries in *list_nr. Calls doit for each entry if cycle is set.
** argc, argv should be the same as in the calling kernel function.
*/
	SegManager *segMan = s->_segMan;
	Node *node;
	int sequence_nr = 0;
	GfxDynView *widget;

	if (!*widget_list) {
		error("make_view_list with widget_list == ()");
	};

	assert_primary_widget_lists(s);
	// In case one of the views' doit() does a DrawPic...
	// Yes, this _does_ happen!

	if (!list) { // list sanity check
		error("Attempt to make list from non-list");
	}

	reg_t next_node = list->first;
	node = lookup_node(s, next_node);
	while (node) {
		reg_t obj = node->value; // The object we're using
		GfxDynView *tempWidget;

		if (options & _K_MAKE_VIEW_LIST_CYCLE) {
			unsigned int signal = GET_SEL32V(obj, signal);

			if (!(signal & _K_VIEW_SIG_FLAG_FROZEN)) {

				debugC(2, kDebugLevelGraphics, "  invoking %04x:%04x::doit()\n", PRINT_REG(obj));
				invoke_selector(INV_SEL(obj, doit, kContinueOnInvalidSelector), 0); // Call obj::doit() if neccessary


				// Lookup node again, since the NodeTable it was in may
				// have been re-allocated.
				node = lookup_node(s, next_node);
			}
		}

		next_node = node->succ; // In case the cast list was changed

		if (list->first.segment == 0 && list->first.offset == 0) // The cast list was completely emptied!
			break;

		tempWidget = _k_make_dynview_obj(obj, options, sequence_nr--, argc, argv);
		if (tempWidget)
			(*widget_list)->add((GfxContainer *)(*widget_list), tempWidget);

		node = lookup_node(s, next_node); // Next node
	}

	widget = (GfxDynView *)(*widget_list)->_contents;

	while (widget) { // Read back widget values
		reg_t *sp = widget->signalp.getPointer(s->_segMan);
		if (sp)
			widget->signal = sp->offset;

		widget = (GfxDynView *)widget->_next;
	}
}

void SciGui32::draw_rect_to_control_map(Common::Rect abs_zone) {
	GfxBox *box;
	gfx_color_t color;

	gfxop_set_color(s->gfx_state, &color, -1, -1, -1, -1, -1, 0xf);

	debugC(2, kDebugLevelGraphics, "    adding control block (%d,%d)to(%d,%d)\n", abs_zone.left, abs_zone.top, abs_zone.right, abs_zone.bottom);

	box = gfxw_new_box(s->gfx_state, gfx_rect(abs_zone.left, abs_zone.top, abs_zone.width(),
						abs_zone.height()), color, color, GFX_BOX_SHADE_FLAT);

	assert_primary_widget_lists(s);

	ADD_TO_CURRENT_PICTURE_PORT(box);
}

void SciGui32::draw_obj_to_control_map(GfxDynView *view) {
	reg_t obj = make_reg(view->_ID, view->_subID);

	if (!s->_segMan->isObject(obj))
		warning("View %d does not contain valid object reference %04x:%04x", view->_ID, PRINT_REG(obj));

	reg_t* sp = view->signalp.getPointer(s->_segMan);
	if (!(sp && (sp->offset & _K_VIEW_SIG_FLAG_IGNORE_ACTOR))) {
		Common::Rect abs_zone = get_nsrect32(s, make_reg(view->_ID, view->_subID), 1);
		draw_rect_to_control_map(abs_zone);
	}
}

int SciGui32::_k_view_list_dispose_loop(List *list, GfxDynView *widget, int argc, reg_t *argv) {
// disposes all list members flagged for disposal
// returns non-zero IFF views were dropped
	int signal;
	int dropped = 0;
	SegManager *segMan = s->_segMan;

	_k_animate_ran = false;

	if (widget) {
		int retval;
		// Recurse:
		retval = _k_view_list_dispose_loop(list, (GfxDynView *)widget->_next, argc, argv);

		if (retval == -1) // Bail out on annihilation, rely on re-start from Animate()
			return -1;

		if (GFXW_IS_DYN_VIEW(widget) && (widget->_ID != GFXW_NO_ID)) {
			signal = widget->signalp.getPointer(segMan)->offset;
			if (signal & _K_VIEW_SIG_FLAG_DISPOSE_ME) {
				reg_t obj = make_reg(widget->_ID, widget->_subID);
				reg_t under_bits = NULL_REG;

				if (!s->_segMan->isObject(obj)) {
					error("Non-object %04x:%04x present in view list during delete time", PRINT_REG(obj));
					obj = NULL_REG;
				} else {
					reg_t *ubp = widget->under_bitsp.getPointer(segMan);
					if (ubp) { // Is there a bg picture left to clean?
						reg_t mem_handle = *ubp;

						if (mem_handle.segment) {
							if (!kfree(s->_segMan, mem_handle)) {
								*ubp = make_reg(0, widget->under_bits = 0);
							} else {
								warning("Treating viewobj %04x:%04x as no longer present", PRINT_REG(obj));
								obj = NULL_REG;
							}
						}
					}
				}
				if (segMan->isObject(obj)) {
					if (invoke_selector(INV_SEL(obj, delete_, kContinueOnInvalidSelector), 0))
						warning("Object at %04x:%04x requested deletion, but does not have a delete funcselector", PRINT_REG(obj));
					if (_k_animate_ran) {
						warning("Object at %04x:%04x invoked kAnimate() during deletion", PRINT_REG(obj));
						return dropped;
					}

					reg_t *ubp = widget->under_bitsp.getPointer(segMan);
					if (ubp)
						under_bits = *ubp;

					if (under_bits.segment) {
						*ubp = make_reg(0, 0);
						graph_restore_box(s, under_bits);
					}

					debugC(2, kDebugLevelGraphics, "Freeing %04x:%04x with signal=%04x\n", PRINT_REG(obj), signal);

					if (!(signal & _K_VIEW_SIG_FLAG_HIDDEN)) {
						debugC(2, kDebugLevelGraphics, "Adding view at %04x:%04x to background\n", PRINT_REG(obj));
						if (!(gfxw_remove_id(widget->_parent, widget->_ID, widget->_subID) == widget)) {
							error("Attempt to remove view with ID %x:%x from list failed", widget->_ID, widget->_subID);
						}

						s->drop_views->add((GfxContainer *)s->drop_views, gfxw_picviewize_dynview(widget));

						draw_obj_to_control_map(widget);
						widget->draw_bounds.y += s->dyn_views->_bounds.y - widget->_parent->_bounds.y;
						widget->draw_bounds.x += s->dyn_views->_bounds.x - widget->_parent->_bounds.x;
						dropped = 1;
					} else {
						debugC(2, kDebugLevelGraphics, "Deleting view at %04x:%04x\n", PRINT_REG(obj));
						widget->_flags |= GFXW_FLAG_VISIBLE;
						gfxw_annihilate(widget);
						return -1; // restart: Done in Animate()
					}
				}
			}
		}

	}

	return dropped;
}

void SciGui32::_k_set_now_seen(reg_t object) {
	SegManager *segMan = s->_segMan;
	Common::Rect absrect = get_nsrect32(s, object, 0);

	if (lookup_selector(s->_segMan, object, s->_kernel->_selectorCache.nsTop, NULL, NULL) != kSelectorVariable) {
		return;
	} // This isn't fatal

	PUT_SEL32V(object, nsLeft, absrect.left);
	PUT_SEL32V(object, nsRight, absrect.right);
	PUT_SEL32V(object, nsTop, absrect.top);
	PUT_SEL32V(object, nsBottom, absrect.bottom);
}

void SciGui32::_k_prepare_view_list(GfxList *list, int options) {
	SegManager *segMan = s->_segMan;
	GfxDynView *view = (GfxDynView *) list->_contents;
	while (view) {
		reg_t obj = make_reg(view->_ID, view->_subID);
		int priority, _priority;
		int has_nsrect = (view->_ID <= 0) ? 0 : lookup_selector(s->_segMan, obj, s->_kernel->_selectorCache.nsBottom, NULL, NULL) == kSelectorVariable;
		int oldsignal = view->signal;

		_k_set_now_seen(obj);
		_priority = /*GET_SELECTOR(obj, y); */((view->_pos.y));
		_priority = _find_view_priority(s, _priority - 1);

		if (options & _K_MAKE_VIEW_LIST_DRAW_TO_CONTROL_MAP) { // Picview
			priority = (int16)GET_SEL32V(obj, priority);
			if (priority < 0)
				priority = _priority; // Always for picviews
		} else { // Dynview
			if (has_nsrect && !(view->signal & _K_VIEW_SIG_FLAG_FIX_PRI_ON)) { // Calculate priority
				if (options & _K_MAKE_VIEW_LIST_CALC_PRIORITY)
					PUT_SEL32V(obj, priority, _priority);

				priority = _priority;

			} else // DON'T calculate the priority
				priority = (int16)GET_SEL32V(obj, priority);
		}

		view->_color.priority = priority;

		if (priority > -1)
			view->_color.mask |= GFX_MASK_PRIORITY;
		else
			view->_color.mask &= ~GFX_MASK_PRIORITY;

		// CR (from :Bob Heitman:) stopupdated views (like pic views) have
		// their clipped nsRect drawn to the control map
		if (view->signal & _K_VIEW_SIG_FLAG_STOP_UPDATE) {
			view->signal |= _K_VIEW_SIG_FLAG_STOPUPD;
			debugC(2, kDebugLevelGraphics, "Setting magic STOP_UPD for %04x:%04x\n", PRINT_REG(obj));
		}

		if ((options & _K_MAKE_VIEW_LIST_DRAW_TO_CONTROL_MAP))
			draw_obj_to_control_map(view);

		// Extreme Pattern Matching ugliness ahead...
		if (view->signal & _K_VIEW_SIG_FLAG_NO_UPDATE) {
			if (((view->signal & (_K_VIEW_SIG_FLAG_UPDATED | _K_VIEW_SIG_FLAG_FORCE_UPDATE))) // 9.1.1.1
			        || ((view->signal & (_K_VIEW_SIG_FLAG_HIDDEN | _K_VIEW_SIG_FLAG_REMOVE)) == _K_VIEW_SIG_FLAG_HIDDEN)
			        || ((view->signal & (_K_VIEW_SIG_FLAG_HIDDEN | _K_VIEW_SIG_FLAG_REMOVE)) == _K_VIEW_SIG_FLAG_REMOVE) // 9.1.1.2
			        || ((view->signal & (_K_VIEW_SIG_FLAG_HIDDEN | _K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_ALWAYS_UPDATE)) == _K_VIEW_SIG_FLAG_ALWAYS_UPDATE) // 9.1.1.3
			        || ((view->signal & (_K_VIEW_SIG_FLAG_HIDDEN | _K_VIEW_SIG_FLAG_ALWAYS_UPDATE)) == (_K_VIEW_SIG_FLAG_HIDDEN | _K_VIEW_SIG_FLAG_ALWAYS_UPDATE))) { // 9.1.1.4
				s->pic_not_valid++;
				view->signal &= ~_K_VIEW_SIG_FLAG_STOP_UPDATE;
			}

			else if (((view->signal & (_K_VIEW_SIG_FLAG_HIDDEN | _K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_ALWAYS_UPDATE)) == 0)
			         || ((view->signal & (_K_VIEW_SIG_FLAG_HIDDEN | _K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_ALWAYS_UPDATE)) == (_K_VIEW_SIG_FLAG_HIDDEN | _K_VIEW_SIG_FLAG_REMOVE))
			         || ((view->signal & (_K_VIEW_SIG_FLAG_HIDDEN | _K_VIEW_SIG_FLAG_ALWAYS_UPDATE)) == (_K_VIEW_SIG_FLAG_HIDDEN | _K_VIEW_SIG_FLAG_ALWAYS_UPDATE))
			         || ((view->signal & (_K_VIEW_SIG_FLAG_HIDDEN | _K_VIEW_SIG_FLAG_ALWAYS_UPDATE)) == _K_VIEW_SIG_FLAG_HIDDEN)) {
				view->signal &= ~_K_VIEW_SIG_FLAG_STOP_UPDATE;
			}
		} else {
			if (view->signal & _K_VIEW_SIG_FLAG_STOP_UPDATE) {
				s->pic_not_valid++;
				view->signal &= ~_K_VIEW_SIG_FLAG_FORCE_UPDATE;
			} else { // if not STOP_UPDATE
				if (view->signal & _K_VIEW_SIG_FLAG_ALWAYS_UPDATE)
					s->pic_not_valid++;
				view->signal &= ~_K_VIEW_SIG_FLAG_FORCE_UPDATE;
			}
		}

		debugC(2, kDebugLevelGraphics, "  dv[%04x:%04x]: signal %04x -> %04x\n", PRINT_REG(obj), oldsignal, view->signal);

		// Never happens
/*		if (view->signal & 0) {
			view->signal &= ~_K_VIEW_SIG_FLAG_STOPUPD;
			fprintf(stderr, "Unsetting magic StopUpd for view %04x:%04x\n", PRINT_REG(obj));
		} */

		view = (GfxDynView *)view->_next;
	}
}

void SciGui32::_k_update_signals_in_view_list(GfxList *old_list, GfxList *new_list) {
	// O(n^2)... a bit painful, but much faster than the redraws it helps prevent
	GfxDynView *old_widget = (GfxDynView *)old_list->_contents;

	/* Traverses all old widgets, updates them with signals from the new widgets.
	** This is done to avoid evil hacks in widget.c; widgets with unique IDs are
	** replaced there iff they are NOT equal_to a new widget with the same ID.
	** If they were replaced every time, we'd be doing far too many redraws.
	*/

	while (old_widget) {
		GfxDynView *new_widget = (GfxDynView *) new_list->_contents;

		while (new_widget
		        && (new_widget->_ID != old_widget->_ID
		            || new_widget->_subID != old_widget->_subID))
			new_widget = (GfxDynView *)new_widget->_next;

		if (new_widget) {
			int carry = old_widget->signal & _K_VIEW_SIG_FLAG_STOPUPD;
			// Transfer 'stopupd' flag

			if ((new_widget->_pos.x != old_widget->_pos.x)
			        || (new_widget->_pos.y != old_widget->_pos.y)
					// No idea why this is supposed to be bad
/*			        || (new_widget->z != old_widget->z)
			        || (new_widget->view != old_widget->view)
			        || (new_widget->loop != old_widget->loop)
			        || (new_widget->cel != old_widget->cel)
			        */)
				carry = 0;

			old_widget->signal = new_widget->signal |= carry;
		}

		old_widget = (GfxDynView *)old_widget->_next;
	}
}

void SciGui32::_k_view_list_kryptonize(GfxWidget *v) {
	if (v) {
		v->_flags &= ~GFXW_FLAG_IMMUNE_TO_SNAPSHOTS;
		_k_view_list_kryptonize(v->_next);
	}
}

void SciGui32::_k_raise_topmost_in_view_list(GfxList *list, GfxDynView *view) {
	if (view) {
		GfxDynView *next = (GfxDynView *)view->_next;

		// step 11
		if ((view->signal & (_K_VIEW_SIG_FLAG_NO_UPDATE | _K_VIEW_SIG_FLAG_HIDDEN | _K_VIEW_SIG_FLAG_ALWAYS_UPDATE)) == 0) {
			debugC(2, kDebugLevelGraphics, "Forcing precedence 2 at [%04x:%04x] with %04x\n", PRINT_REG(make_reg(view->_ID, view->_subID)), view->signal);
			view->force_precedence = 2;

			if ((view->signal & (_K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_HIDDEN)) == _K_VIEW_SIG_FLAG_REMOVE) {
				view->signal &= ~_K_VIEW_SIG_FLAG_REMOVE;
			}
		}

		gfxw_remove_widget_from_container(view->_parent, view);

		if (view->signal & _K_VIEW_SIG_FLAG_HIDDEN)
			gfxw_hide_widget(view);
		else
			gfxw_show_widget(view);

		list->add((GfxContainer *)list, view);

		_k_raise_topmost_in_view_list(list, next);
	}
}

void SciGui32::_k_redraw_view_list(GfxList *list) {
	GfxDynView *view = (GfxDynView *) list->_contents;
	while (view) {

		debugC(2, kDebugLevelGraphics, "  dv[%04x:%04x]: signal %04x\n", PRINT_REG(make_reg(view->_ID, view->_subID)), view->signal);

		// step 1 of subalgorithm
		if (view->signal & _K_VIEW_SIG_FLAG_NO_UPDATE) {
			if (view->signal & _K_VIEW_SIG_FLAG_FORCE_UPDATE)
				view->signal &= ~_K_VIEW_SIG_FLAG_FORCE_UPDATE;

			if (view->signal & _K_VIEW_SIG_FLAG_UPDATED)
				view->signal &= ~(_K_VIEW_SIG_FLAG_UPDATED | _K_VIEW_SIG_FLAG_NO_UPDATE);
		} else { // NO_UPD is not set
			if (view->signal & _K_VIEW_SIG_FLAG_STOP_UPDATE) {
				view->signal &= ~_K_VIEW_SIG_FLAG_STOP_UPDATE;
				view->signal |= _K_VIEW_SIG_FLAG_NO_UPDATE;
			}
		}

		debugC(2, kDebugLevelGraphics, "    at substep 6: signal %04x\n", view->signal);

		if (view->signal & _K_VIEW_SIG_FLAG_ALWAYS_UPDATE)
			view->signal &= ~(_K_VIEW_SIG_FLAG_STOP_UPDATE | _K_VIEW_SIG_FLAG_UPDATED | _K_VIEW_SIG_FLAG_NO_UPDATE | _K_VIEW_SIG_FLAG_FORCE_UPDATE);

		debugC(2, kDebugLevelGraphics, "    at substep 11/14: signal %04x\n", view->signal);

		if (view->signal & _K_VIEW_SIG_FLAG_NO_UPDATE) {
			if (view->signal & _K_VIEW_SIG_FLAG_HIDDEN)
				view->signal |= _K_VIEW_SIG_FLAG_REMOVE;
			else
				view->signal &= ~_K_VIEW_SIG_FLAG_REMOVE;
		} else if (!(view->signal & _K_VIEW_SIG_FLAG_HIDDEN))
			view->force_precedence = 1;

		debugC(2, kDebugLevelGraphics, "    -> signal %04x\n", view->signal);

		view = (GfxDynView *)view->_next;
	}
}

// Flags for _k_draw_view_list
// Whether some magic with the base object's "signal" selector should be done:
#define _K_DRAW_VIEW_LIST_USE_SIGNAL 1
// This flag draws all views with the "DISPOSE_ME" flag set:
#define _K_DRAW_VIEW_LIST_DISPOSEABLE 2
// Use this one to draw all views with "DISPOSE_ME" NOT set:
#define _K_DRAW_VIEW_LIST_NONDISPOSEABLE 4
// Draw as picviews
#define _K_DRAW_VIEW_LIST_PICVIEW 8

void SciGui32::_k_draw_view_list(GfxList *list, int flags) {
	// Draws list_nr members of list to s->pic.
	GfxDynView *widget = (GfxDynView *) list->_contents;

	if ((GfxContainer *)s->port != (GfxContainer *)s->dyn_views->_parent)
		return; // Return if the pictures are meant for a different port

	while (widget) {
		if (flags & _K_DRAW_VIEW_LIST_PICVIEW)
			widget = gfxw_picviewize_dynview(widget);

		if (GFXW_IS_DYN_VIEW(widget) && widget->_ID) {
			uint16 signal = (flags & _K_DRAW_VIEW_LIST_USE_SIGNAL) ? widget->signalp.getPointer(s->_segMan)->offset : 0;

			if (signal & _K_VIEW_SIG_FLAG_HIDDEN)
				gfxw_hide_widget(widget);
			else
				gfxw_show_widget(widget);

			if (!(flags & _K_DRAW_VIEW_LIST_USE_SIGNAL)
			        || ((flags & _K_DRAW_VIEW_LIST_DISPOSEABLE) && (signal & _K_VIEW_SIG_FLAG_DISPOSE_ME))
			        || ((flags & _K_DRAW_VIEW_LIST_NONDISPOSEABLE) && !(signal & _K_VIEW_SIG_FLAG_DISPOSE_ME))) {

				if (flags & _K_DRAW_VIEW_LIST_USE_SIGNAL) {
					signal &= ~(_K_VIEW_SIG_FLAG_STOP_UPDATE | _K_VIEW_SIG_FLAG_UPDATED | _K_VIEW_SIG_FLAG_NO_UPDATE | _K_VIEW_SIG_FLAG_FORCE_UPDATE);
					// Clear all of those flags

					if (signal & _K_VIEW_SIG_FLAG_HIDDEN)
						gfxw_hide_widget(widget);
					else
						gfxw_show_widget(widget);

					*widget->signalp.getPointer(s->_segMan) = make_reg(0, signal); // Write the changes back
				};

			} // ...if we're drawing disposeables and this one is disposeable, or if we're drawing non-
			  // disposeables and this one isn't disposeable
		}

		widget = (GfxDynView *)widget->_next;
	} // while (widget)
}

void SciGui32::_k_view_list_do_postdraw(GfxList *list) {
	SegManager *segMan = s->_segMan;
	GfxDynView *widget = (GfxDynView *) list->_contents;

	while (widget) {
		reg_t obj = make_reg(widget->_ID, widget->_subID);

		/*
		 * this fixes a few problems, but doesn't match SSCI's logic.
		 * The semantics of the private flag need to be verified before this can be uncommented.
		 * Fixes bug #326 (CB1, ego falls down stairs)
		 * if ((widget->signal & (_K_VIEW_SIG_FLAG_PRIVATE | _K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_NO_UPDATE)) == _K_VIEW_SIG_FLAG_PRIVATE) {
		 */
		if ((widget->signal & (_K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_NO_UPDATE)) == 0) {
			int has_nsrect = lookup_selector(s->_segMan, obj, s->_kernel->_selectorCache.nsBottom, NULL, NULL) == kSelectorVariable;

			if (has_nsrect) {
				int temp;

				temp = GET_SEL32V(obj, nsLeft);
				PUT_SEL32V(obj, lsLeft, temp);

				temp = GET_SEL32V(obj, nsRight);
				PUT_SEL32V(obj, lsRight, temp);

				temp = GET_SEL32V(obj, nsTop);
				PUT_SEL32V(obj, lsTop, temp);

				temp = GET_SEL32V(obj, nsBottom);
				PUT_SEL32V(obj, lsBottom, temp);
#ifdef DEBUG_LSRECT
				fprintf(stderr, "lsRected %04x:%04x\n", PRINT_REG(obj));
#endif
			}
#ifdef DEBUG_LSRECT
			else
				fprintf(stderr, "Not lsRecting %04x:%04x because %d\n", PRINT_REG(obj), lookup_selector(s->_segMan, obj, s->_kernel->_selectorCache.nsBottom, NULL, NULL));
#endif

			if (widget->signal & _K_VIEW_SIG_FLAG_HIDDEN)
				widget->signal |= _K_VIEW_SIG_FLAG_REMOVE;
		}
#ifdef DEBUG_LSRECT
		fprintf(stderr, "obj %04x:%04x has pflags %x\n", PRINT_REG(obj), (widget->signal & (_K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_NO_UPDATE)));
#endif

		reg_t* sp = widget->signalp.getPointer(s->_segMan);
		if (sp) {
			*sp = make_reg(0, widget->signal & 0xffff); /* Write back signal */
		}

		widget = (GfxDynView *)widget->_next;
	}
}

#define K_ANIMATE_CENTER_OPEN_H  0 // horizontally open from center
#define K_ANIMATE_CENTER_OPEN_V  1 // vertically open from center
#define K_ANIMATE_RIGHT_OPEN     2 // open from right
#define K_ANIMATE_LEFT_OPEN      3 // open from left
#define K_ANIMATE_BOTTOM_OPEN    4 // open from bottom
#define K_ANIMATE_TOP_OPEN       5 // open from top
#define K_ANIMATE_BORDER_OPEN_F  6 // open from edges to center
#define K_ANIMATE_CENTER_OPEN_F  7 // open from center to edges
#define K_ANIMATE_OPEN_CHECKERS  8 // open random checkboard
#define K_ANIMATE_BORDER_CLOSE_H_CENTER_OPEN_H  9 // horizontally close to center,reopen from center
#define K_ANIMATE_BORDER_CLOSE_V_CENTER_OPEN_V 10 // vertically close to center, reopen from center
#define K_ANIMATE_LEFT_CLOSE_RIGHT_OPEN        11 // close to right, reopen from right
#define K_ANIMATE_RIGHT_CLOSE_LEFT_OPEN        12 // close to left,  reopen from left
#define K_ANIMATE_TOP_CLOSE_BOTTOM_OPEN        13 // close to bottom, reopen from bottom
#define K_ANIMATE_BOTTOM_CLOSE_TOP_OPEN        14 // close to top, reopen from top
#define K_ANIMATE_CENTER_CLOSE_F_BORDER_OPEN_F 15 // close from center to edges,
// reopen from edges to center
#define K_ANIMATE_BORDER_CLOSE_F_CENTER_OPEN_F 16 // close from edges to center, reopen from
// center to edges */
#define K_ANIMATE_CLOSE_CHECKERS_OPEN_CHECKERS 17 // close random checkboard, reopen
#define K_ANIMATE_PALETTE_FADEOUT_FADEIN       0x1e
#define K_ANIMATE_SCROLL_LEFT                  0x28
#define K_ANIMATE_SCROLL_RIGHT                 0x29
#define K_ANIMATE_SCROLL_DOWN                  0x2a
#define K_ANIMATE_SCROLL_UP                    0x2b

#define GRAPH_BLANK_BOX(s, x, y, xl, yl, color) gfxop_fill_box(s->gfx_state, \
	gfx_rect(x, (((y) < 10)? 10 : (y)), xl, (((y) < 10)? ((y) - 10) : 0) + (yl)), s->ega_colors[color]);

#define GRAPH_UPDATE_BOX(s, x, y, xl, yl) gfxop_draw_pixmap(s->gfx_state, newscreen, \
	gfx_rect(x, (((y) < 10)? 10 : (y)) - 10, xl, (((y) < 10)? ((y) - 10) : 0) + (yl)), Common::Point(x, ((y) < 10)? 10 : (y) ));

void SciGui32::animate_do_animation(int argc, reg_t *argv) {
	long animation_delay = 5;
	int i, remaining_checkers;
	int update_counter;
	// Number of animation steps to perform betwen updates for transition animations
	int animation_granularity = 4;
	int granularity0 = animation_granularity << 1;
	int granularity1 = animation_granularity;
	int granularity2 = animation_granularity >> 2;
	int granularity3 = animation_granularity >> 4;
	char checkers[32 * 19];
	gfx_pixmap_t *newscreen = gfxop_grab_pixmap(s->gfx_state, gfx_rect(0, 10, 320, 190));

	if (!granularity2)
		granularity2 = 1;
	if (!granularity3)
		granularity3 = 1;

	gfxop_set_clip_zone(s->gfx_state, gfx_rect_fullscreen);

	if (!newscreen) {
		error("Failed to allocate 'newscreen'");
		return;
	}

	gfxop_draw_pixmap(s->gfx_state, s->old_screen, gfx_rect(0, 0, 320, 190), Common::Point(0, 10));
	gfxop_update_box(s->gfx_state, gfx_rect(0, 0, 320, 200));

	//debugC(2, kDebugLevelGraphics, "Animating pic opening type %x\n", s->pic_animate);

	gfxop_enable_dirty_frames(s->gfx_state);

	switch (s->pic_animate) {
	case K_ANIMATE_BORDER_CLOSE_H_CENTER_OPEN_H :
		for (i = 0; i < 159 + granularity1; i += granularity1) {
			GRAPH_BLANK_BOX(s, i, 10, granularity1, 190, 0);
			gfxop_update(s->gfx_state);
			GRAPH_BLANK_BOX(s, 319 - i, 10, granularity1, 190, 0);
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, animation_delay / 1000);
			process_sound_events(s);
		}
		GRAPH_BLANK_BOX(s, 0, 10, 320, 190, 0);

	case K_ANIMATE_CENTER_OPEN_H :

		for (i = 159; i >= 1 - granularity1; i -= granularity1) {
			GRAPH_UPDATE_BOX(s, i, 10, granularity1, 190);
			gfxop_update(s->gfx_state);
			GRAPH_UPDATE_BOX(s, 319 - i, 10, granularity1, 190);
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, animation_delay / 1000);
			process_sound_events(s);
		}
		break;


	case K_ANIMATE_BORDER_CLOSE_V_CENTER_OPEN_V :

		for (i = 0; i < 94 + granularity2; i += granularity2) {
			GRAPH_BLANK_BOX(s, 0, i + 10, 320, granularity2, 0);
			gfxop_update(s->gfx_state);
			GRAPH_BLANK_BOX(s, 0, 199 - i, 320, granularity2, 0);
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, 2 * animation_delay / 1000);
			process_sound_events(s);
		}
		GRAPH_BLANK_BOX(s, 0, 10, 320, 190, 0);

	case K_ANIMATE_CENTER_OPEN_V :

		for (i = 94; i >= 1 - granularity2; i -= granularity2) {
			GRAPH_UPDATE_BOX(s, 0, i + 10, 320, granularity2);
			gfxop_update(s->gfx_state);
			GRAPH_UPDATE_BOX(s, 0, 199 - i, 320, granularity2);
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, 2 * animation_delay / 1000);
			process_sound_events(s);
		}
		break;


	case K_ANIMATE_LEFT_CLOSE_RIGHT_OPEN :

		for (i = 0; i < 319 + granularity0; i += granularity0) {
			GRAPH_BLANK_BOX(s, i, 10, granularity0, 190, 0);
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, animation_delay / 2 / 1000);
			process_sound_events(s);
		}
		GRAPH_BLANK_BOX(s, 0, 10, 320, 190, 0);

	case K_ANIMATE_RIGHT_OPEN :
		for (i = 319; i >= 1 - granularity0; i -= granularity0) {
			GRAPH_UPDATE_BOX(s, i, 10, granularity0, 190);
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, animation_delay / 2 / 1000);
			process_sound_events(s);
		}
		break;


	case K_ANIMATE_RIGHT_CLOSE_LEFT_OPEN :

		for (i = 319; i >= 1 - granularity0; i -= granularity0) {
			GRAPH_BLANK_BOX(s, i, 10, granularity0, 190, 0);
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, animation_delay / 2 / 1000);
			process_sound_events(s);
		}
		GRAPH_BLANK_BOX(s, 0, 10, 320, 190, 0);

	case K_ANIMATE_LEFT_OPEN :

		for (i = 0; i < 319 + granularity0; i += granularity0) {
			GRAPH_UPDATE_BOX(s, i, 10, granularity0, 190);
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, animation_delay / 2 / 1000);
			process_sound_events(s);
		}
		break;


	case K_ANIMATE_TOP_CLOSE_BOTTOM_OPEN :

		for (i = 10; i < 199 + granularity1; i += granularity1) {
			GRAPH_BLANK_BOX(s, 0, i, 320, granularity1, 0);
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, animation_delay / 1000);
			process_sound_events(s);
		}
		GRAPH_BLANK_BOX(s, 0, 10, 320, 190, 0);

	case K_ANIMATE_BOTTOM_OPEN :

		for (i = 199; i >= 11 - granularity1; i -= granularity1) {
			GRAPH_UPDATE_BOX(s, 0, i, 320, granularity1);
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, animation_delay / 1000);
			process_sound_events(s);
		}
		break;


	case K_ANIMATE_BOTTOM_CLOSE_TOP_OPEN :

		for (i = 199; i >= 11 - granularity1; i -= granularity1) {
			GRAPH_BLANK_BOX(s, 0, i, 320, granularity1, 0);
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, animation_delay / 1000);
			process_sound_events(s);
		}
		GRAPH_BLANK_BOX(s, 0, 10, 320, 190, 0);

	case K_ANIMATE_TOP_OPEN :

		for (i = 10; i < 199 + granularity1; i += granularity1) {
			GRAPH_UPDATE_BOX(s, 0, i, 320, granularity1);
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, animation_delay / 1000);
			process_sound_events(s);
		}
		break;


	case K_ANIMATE_CENTER_CLOSE_F_BORDER_OPEN_F :

		for (i = 31; i >= 1 - granularity3; i -= granularity3) {
			int real_i = (i < 0) ? 0 : i;
			int height_l = 3 * (granularity3 - real_i + i);
			int width_l = 5 * (granularity3 - real_i + i);
			int height = real_i * 3;
			int width = real_i * 5;

			GRAPH_BLANK_BOX(s, width, 10 + height, width_l, 190 - 2 * height, 0);
			gfxop_update(s->gfx_state);
			GRAPH_BLANK_BOX(s, 320 - width_l - width, 10 + height, width_l, 190 - 2 * height, 0);
			gfxop_update(s->gfx_state);

			GRAPH_BLANK_BOX(s, width, 10 + height, 320 - 2 * width, height_l, 0);
			gfxop_update(s->gfx_state);
			GRAPH_BLANK_BOX(s, width, 200 - height_l - height, 320 - 2 * width, height_l, 0);
			gfxop_update(s->gfx_state);

			gfxop_sleep(s->gfx_state, 4 * animation_delay / 1000);
			process_sound_events(s);
		}

	case K_ANIMATE_BORDER_OPEN_F :

		for (i = 0; i < 31 + granularity3; i += granularity3) {
			int real_i = (i < 0) ? 0 : i;
			int height_l = 3 * (granularity3 - real_i + i);
			int width_l = 5 * (granularity3 - real_i + i);
			int height = real_i * 3;
			int width = real_i * 5;

			GRAPH_UPDATE_BOX(s, width, 10 + height, width_l, 190 - 2 * height);
			gfxop_update(s->gfx_state);
			GRAPH_UPDATE_BOX(s, 320 - width_l - width, 10 + height, width_l, 190 - 2 * height);
			gfxop_update(s->gfx_state);

			GRAPH_UPDATE_BOX(s, width, 10 + height, 320 - 2 * width, height_l);
			gfxop_update(s->gfx_state);
			GRAPH_UPDATE_BOX(s, width, 200 - height_l - height, 320 - 2 * width, height_l);
			gfxop_update(s->gfx_state);

			gfxop_sleep(s->gfx_state, 4 * animation_delay / 1000);
			process_sound_events(s);
		}

		break;

	case K_ANIMATE_BORDER_CLOSE_F_CENTER_OPEN_F :

		for (i = 0; i < 31 + granularity3; i += granularity3) {
			int real_i = (i < 0) ? 0 : i;
			int height_l = 3 * (granularity3 - real_i + i);
			int width_l = 5 * (granularity3 - real_i + i);
			int height = real_i * 3;
			int width = real_i * 5;

			GRAPH_BLANK_BOX(s, width, 10 + height, width_l, 190 - 2 * height, 0);
			gfxop_update(s->gfx_state);
			GRAPH_BLANK_BOX(s, 320 - width_l - width, 10 + height, width_l, 190 - 2 * height, 0);
			gfxop_update(s->gfx_state);

			GRAPH_BLANK_BOX(s, width, 10 + height, 320 - 2 * width, height_l, 0);
			gfxop_update(s->gfx_state);
			GRAPH_BLANK_BOX(s, width, 200 - height_l - height, 320 - 2 * width, height_l, 0);
			gfxop_update(s->gfx_state);

			gfxop_sleep(s->gfx_state, 7 * animation_delay / 1000);
			process_sound_events(s);
		}

	case K_ANIMATE_CENTER_OPEN_F :

		for (i = 31; i >= 1 - granularity3; i -= granularity3) {
			int real_i = (i < 0) ? 0 : i;
			int height_l = 3 * (granularity3 - real_i + i);
			int width_l = 5 * (granularity3 - real_i + i);
			int height = real_i * 3;
			int width = real_i * 5;

			GRAPH_UPDATE_BOX(s, width, 10 + height, width_l, 190 - 2 * height);
			gfxop_update(s->gfx_state);
			GRAPH_UPDATE_BOX(s, 320 - width_l - width, 10 + height, width_l, 190 - 2*height);
			gfxop_update(s->gfx_state);

			GRAPH_UPDATE_BOX(s, width, 10 + height, 320 - 2 * width, height_l);
			gfxop_update(s->gfx_state);
			GRAPH_UPDATE_BOX(s, width, 200 - height_l - height, 320 - 2 * width, height_l);
			gfxop_update(s->gfx_state);

			gfxop_sleep(s->gfx_state, 7 * animation_delay / 1000);
			process_sound_events(s);
		}
		break;

	case K_ANIMATE_PALETTE_FADEOUT_FADEIN:
		warning("TODO: Palette fadeout/fadein");
		GRAPH_UPDATE_BOX(s, 0, 10, 320, 190);
		break;

	case K_ANIMATE_CLOSE_CHECKERS_OPEN_CHECKERS :

		memset(checkers, 0, sizeof(checkers));
		remaining_checkers = 19 * 32;
		update_counter = granularity1;

		while (remaining_checkers) {
			int x, y, checker = 1 + (int)(1.0 * remaining_checkers * rand() / (RAND_MAX + 1.0));
			i = -1;

			while (checker)
				if (checkers[++i] == 0)
					--checker;
			checkers[i] = 1; // Mark checker as used

			x = i % 32;
			y = i / 32;

			GRAPH_BLANK_BOX(s, x * 10, 10 + y * 10, 10, 10, 0);
			if (!(update_counter--) || (remaining_checkers == 1)) {
				gfxop_update(s->gfx_state);
				update_counter = granularity1;
			}

			if (remaining_checkers & 1) {
				gfxop_sleep(s->gfx_state, animation_delay / 4 / 1000);
			}

			--remaining_checkers;
			process_sound_events(s);
		}

	case K_ANIMATE_OPEN_CHECKERS :

		memset(checkers, 0, sizeof(checkers));
		remaining_checkers = 19 * 32;
		update_counter = granularity1;

		while (remaining_checkers) {
			int x, y, checker = 1 + (int)(1.0 * remaining_checkers * rand() / (RAND_MAX + 1.0));
			i = -1;

			while (checker)
				if (checkers[++i] == 0) --checker;
			checkers[i] = 1; // Mark checker as used

			x = i % 32;
			y = i / 32;

			GRAPH_UPDATE_BOX(s, x * 10, 10 + y * 10, 10, 10);

			if (!(update_counter--) || (remaining_checkers == 1)) {
				gfxop_update(s->gfx_state);
				update_counter = granularity1;
			}

			if (remaining_checkers & 1) {
				gfxop_sleep(s->gfx_state, animation_delay / 4 / 1000);
			}

			--remaining_checkers;
			process_sound_events(s);
		}
		break;


	case K_ANIMATE_SCROLL_LEFT :

		for (i = 0; i < 319; i += granularity0) {
			gfxop_draw_pixmap(s->gfx_state, newscreen, gfx_rect(320 - i, 0, i, 190), Common::Point(0, 10));
			gfxop_draw_pixmap(s->gfx_state, s->old_screen, gfx_rect(0, 0, 320 - i, 190), Common::Point(i, 10));
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, (animation_delay >> 3) / 1000);
		}
		GRAPH_UPDATE_BOX(s, 0, 10, 320, 190);
		break;

	case K_ANIMATE_SCROLL_RIGHT :

		for (i = 0; i < 319; i += granularity0) {
			gfxop_draw_pixmap(s->gfx_state, newscreen, gfx_rect(0, 0, i, 190), Common::Point(319 - i, 10));
			gfxop_draw_pixmap(s->gfx_state, s->old_screen, gfx_rect(i, 0, 320 - i, 190), Common::Point(0, 10));
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, (animation_delay >> 3) / 1000);
		}
		GRAPH_UPDATE_BOX(s, 0, 10, 320, 190);
		break;

	case K_ANIMATE_SCROLL_UP :

		for (i = 0; i < 189; i += granularity0) {
			gfxop_draw_pixmap(s->gfx_state, newscreen, gfx_rect(0, 190 - i, 320, i), Common::Point(0, 10));
			gfxop_draw_pixmap(s->gfx_state, s->old_screen, gfx_rect(0, 0, 320, 190 - i), Common::Point(0, 10 + i));
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, (animation_delay >> 3) / 1000);
		}
		GRAPH_UPDATE_BOX(s, 0, 10, 320, 190);
		break;

	case K_ANIMATE_SCROLL_DOWN :

		for (i = 0; i < 189; i += granularity0) {
			gfxop_draw_pixmap(s->gfx_state, newscreen, gfx_rect(0, 0, 320, i), Common::Point(0, 200 - i));
			gfxop_draw_pixmap(s->gfx_state, s->old_screen, gfx_rect(0, i, 320, 190 - i), Common::Point(0, 10));
			gfxop_update(s->gfx_state);
			gfxop_sleep(s->gfx_state, (animation_delay >> 3) / 1000);
		}
		GRAPH_UPDATE_BOX(s, 0, 10, 320, 190);
		break;

	default:
		warning("Unknown opening animation 0x%02x", s->pic_animate);
		GRAPH_UPDATE_BOX(s, 0, 10, 320, 190);

	}

	gfxop_free_pixmap(s->gfx_state, s->old_screen);
	gfxop_free_pixmap(s->gfx_state, newscreen);
	s->old_screen = NULL;
}

void SciGui32::animate(reg_t listReference, bool cycle, int argc, reg_t *argv) {
	// Animations are supposed to take a maximum of animation_delay milliseconds.
	List *cast_list = NULL;
	int open_animation = 0;

	process_sound_events(s); // Take care of incoming events (kAnimate is called semi-regularly)
	_k_animate_ran = true; // Used by some of the invoked functions to check for recursion, which may,
						// after all, damage the cast list

	if (listReference.segment) {
		cast_list = lookup_list(s, listReference);
		if (!cast_list)
			return;
	}

	open_animation = (s->pic_is_new) && (s->pic_not_valid);
	s->pic_is_new = 0;

	assert_primary_widget_lists(s);

	if (!s->dyn_views->_contents // Only reparentize empty dynview list
	        && (((GfxContainer *)s->port != (GfxContainer *)s->dyn_views->_parent) // If dynviews are on other port...
	            || (s->dyn_views->_next))) // ... or not on top of the view list
		reparentize_primary_widget_lists(s, s->port);

	if (cast_list) {
		GfxList *templist = gfxw_new_list(s->dyn_views->_bounds, 0);

		_k_make_view_list(&(templist), cast_list, (cycle ? _K_MAKE_VIEW_LIST_CYCLE : 0)
		                  | _K_MAKE_VIEW_LIST_CALC_PRIORITY, argc, (reg_t *)argv);

		// Make sure that none of the doits() did something evil
		assert_primary_widget_lists(s);

		if (!s->dyn_views->_contents // Only reparentize empty dynview list
		        && (((GfxContainer *)s->port != (GfxContainer *)s->dyn_views->_parent) // If dynviews are on other port...
		            || (s->dyn_views->_next))) // ... or not on top of the view list
			reparentize_primary_widget_lists(s, s->port);
		// End of doit() recovery code

		if (s->pic_is_new) { // Happens if DrawPic() is executed by a dynview (yes, that happens)
			animate(listReference, cycle, argc, argv); /* Tail-recurse */
			return;
		}

		debugC(2, kDebugLevelGraphics, "Handling Dynviews (..step 9 inclusive):\n");
		_k_prepare_view_list(templist, _K_MAKE_VIEW_LIST_CALC_PRIORITY);

		if (s->pic_not_valid) {
			debugC(2, kDebugLevelGraphics, "PicNotValid=%d -> Subalgorithm:\n", s->pic_not_valid);
			_k_redraw_view_list(templist);
		}

		_k_update_signals_in_view_list(s->dyn_views, templist);
		s->dyn_views->tag();

		_k_raise_topmost_in_view_list(s->dyn_views, (GfxDynView *)templist->_contents);

		delete templist;
		s->dyn_views->free_tagged((GfxContainer *)s->dyn_views); // Free obsolete dynviews
	} // if (cast_list)

	if (open_animation) {
		gfxop_clear_box(s->gfx_state, gfx_rect(0, 10, 320, 190)); // Propagate pic
		s->visual->add_dirty_abs((GfxContainer *)s->visual, gfx_rect_fullscreen, 0);
		// Mark screen as dirty so picviews will be drawn correctly
		FULL_REDRAW();

		animate_do_animation(argc, (reg_t*)argv);
	} // if (open_animation)

	if (cast_list) {
		int retval;
		int reparentize = 0;

		s->pic_not_valid = 0;

		_k_view_list_do_postdraw(s->dyn_views);

		// _k_view_list_dispose_loop() returns -1 if it requested a re-start, so we do just that.
		while ((retval = _k_view_list_dispose_loop(cast_list, (GfxDynView *) s->dyn_views->_contents, argc, (reg_t *)argv) < 0))
			reparentize = 1;

		if (s->drop_views->_contents) {
			s->drop_views = gfxw_new_list(s->dyn_views->_bounds, GFXW_LIST_SORTED);
			s->drop_views->_flags |= GFXW_FLAG_IMMUNE_TO_SNAPSHOTS;
			ADD_TO_CURRENT_PICTURE_PORT(s->drop_views);
		} else {
			assert(s->drop_views);
			gfxw_remove_widget_from_container(s->drop_views->_parent, s->drop_views);
			ADD_TO_CURRENT_PICTURE_PORT(s->drop_views);
		}

		if ((reparentize | retval)
		        && ((GfxContainer *)s->port == (GfxContainer *)s->dyn_views->_parent) // If dynviews are on the same port...
		        && (s->dyn_views->_next)) // ... and not on top of the view list...
			reparentize_primary_widget_lists(s, s->port); // ...then reparentize.

		_k_view_list_kryptonize(s->dyn_views->_contents);
	}

	FULL_REDRAW();
}

void SciGui32::addToPicList(reg_t listReference, int argc, reg_t *argv) {
	List *list;
	GfxList *pic_views;

	assert_primary_widget_lists(s);

	if (!listReference.segment) {
		warning("Attempt to AddToPic single non-list: %04x:%04x", PRINT_REG(listReference));
		return;
	}

	list = lookup_list(s, listReference);

	pic_views = gfxw_new_list(s->picture_port->_bounds, 1);

	debugC(2, kDebugLevelGraphics, "Preparing picview list...\n");
	_k_make_view_list(&pic_views, list, 0, argc, argv);
	_k_prepare_view_list(pic_views, _K_MAKE_VIEW_LIST_DRAW_TO_CONTROL_MAP);
	// Store pic views for later re-use

	debugC(2, kDebugLevelGraphics, "Drawing picview list...\n");
	ADD_TO_CURRENT_PICTURE_PORT(pic_views);
	_k_draw_view_list(pic_views, _K_DRAW_VIEW_LIST_NONDISPOSEABLE | _K_DRAW_VIEW_LIST_DISPOSEABLE | _K_DRAW_VIEW_LIST_PICVIEW);
	// Draw relative to the bottom center
	debugC(2, kDebugLevelGraphics, "Returning.\n");

	reparentize_primary_widget_lists(s, s->port);
}

void SciGui32::addToPicView(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 leftPos, int16 topPos, int16 priority, int16 control) {
	assert_primary_widget_lists(s);

	GfxWidget *widget;

	topPos++; // magic +1

	widget = gfxw_new_dyn_view(s->gfx_state, Common::Point(leftPos, topPos), 0, viewId, loopNo, celNo, 0, priority, -1 /* No priority */ , ALIGN_CENTER, ALIGN_BOTTOM, 0);

	if (!widget) {
		error("Attempt to single-add invalid picview (%d/%d/%d)", viewId, loopNo, celNo);
	} else {
		widget->_ID = -1;
		if (control >= 0) {
			Common::Rect abs_zone = nsrect_clip(s, topPos, calculate_nsrect(s, leftPos, topPos, viewId, loopNo, celNo), priority);
			draw_rect_to_control_map(abs_zone);
		}
		ADD_TO_CURRENT_PICTURE_PORT(gfxw_picviewize_dynview((GfxDynView *) widget));
	}
	return;
}

void SciGui32::setNowSeen(reg_t objectReference) {
	_k_set_now_seen(objectReference);
}


void SciGui32::moveCursor(int16 x, int16 y, int16 scaleFactor) {
	Common::Point newPos;

	// newPos = s->gfx_state->pointer_pos;

	newPos.x = x + s->port->zone.x;
	newPos.y = y + s->port->zone.y;

	if (newPos.x > s->port->zone.x + s->port->zone.width)
		newPos.x = s->port->zone.x + s->port->zone.width;
	if (newPos.y > s->port->zone.y + s->port->zone.height)
		newPos.y = s->port->zone.y + s->port->zone.height;

	if (newPos.x < 0)
		newPos.x = 0;
	if (newPos.y < 0)
		newPos.y = 0;

	if (x > 320 || y > 200) {
		debug("[GFX] Attempt to place pointer at invalid coordinates (%d, %d)\n", x, y);
		return; // Not fatal
	}

	g_system->warpMouse(x * scaleFactor, y * scaleFactor);

	// Trigger event reading to make sure the mouse coordinates will
	// actually have changed the next time we read them.
	gfxop_get_event(s->gfx_state, SCI_EVT_PEEK);
}

} // End of namespace Sci
