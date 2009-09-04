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

#include "common/system.h"
#include "common/events.h"
#include "graphics/cursorman.h"
#include "graphics/video/avi_player.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/debug.h"	// for g_debug_sleeptime_factor
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/gfx/gfx_gui.h"
#include "sci/gfx/gfx_widgets.h"
#include "sci/gfx/gfx_state_internal.h"	// required for GfxContainer, GfxPort, GfxVisual
#include "sci/gfx/seq_decoder.h"

namespace Sci {

#undef DEBUG_LSRECT

// This is the real width of a text with a specified width of 0
#define MAX_TEXT_WIDTH_MAGIC_VALUE 192

// Graph subfunctions
enum {
	K_GRAPH_GET_COLORS_NR = 2,
	K_GRAPH_DRAW_LINE = 4,
	K_GRAPH_SAVE_BOX = 7,
	K_GRAPH_RESTORE_BOX = 8,
	K_GRAPH_FILL_BOX_BACKGROUND = 9,
	K_GRAPH_FILL_BOX_FOREGROUND = 10,
	K_GRAPH_FILL_BOX_ANY = 11,
	K_GRAPH_UPDATE_BOX = 12,
	K_GRAPH_REDRAW_BOX = 13,
	K_GRAPH_ADJUST_PRIORITY = 14
};

// Control types and flags
enum {
	K_CONTROL_BUTTON = 1,
	K_CONTROL_TEXT = 2,
	K_CONTROL_EDIT = 3,
	K_CONTROL_ICON = 4,
	K_CONTROL_CONTROL = 6,
	K_CONTROL_CONTROL_ALIAS = 7,
	K_CONTROL_BOX = 10
};

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

#if 0
// Used for debugging
#define FULL_INSPECTION()\
	if (s->visual) \
		s->visual->print(s->visual, 0);
#endif

#define ASSERT(x) { \
	int val = !!(x); \
	if (!val) { \
		error("Fatal error condition on \"" #x "\""); \
		BREAKPOINT(); \
		vm_handle_fatal_error(s, __LINE__, __FILE__); \
	} \
}

static inline int sign_extend_byte(int value) {
	if (value & 0x80)
		return value - 256;
	else
		return value;
}

static void assert_primary_widget_lists(EngineState *s) {
	if (!s->dyn_views) {
		rect_t bounds = s->picture_port->_bounds;

		s->dyn_views = gfxw_new_list(bounds, GFXW_LIST_SORTED);
		s->dyn_views->_flags |= GFXW_FLAG_IMMUNE_TO_SNAPSHOTS;
		ADD_TO_CURRENT_PICTURE_PORT(s->dyn_views);
	}

	if (!s->drop_views) {
		rect_t bounds = s->picture_port->_bounds;

		s->drop_views = gfxw_new_list(bounds, GFXW_LIST_SORTED);
		s->drop_views->_flags |= GFXW_FLAG_IMMUNE_TO_SNAPSHOTS;
		ADD_TO_CURRENT_PICTURE_PORT(s->drop_views);
	}
}

static void reparentize_primary_widget_lists(EngineState *s, GfxPort *newport) {
	if (!newport)
		newport = s->picture_port;

	if (s->dyn_views) {
		gfxw_remove_widget_from_container(s->dyn_views->_parent, s->dyn_views);

		newport->add((GfxContainer *)newport, s->dyn_views);
	}
}

int _find_view_priority(EngineState *s, int y) {
	/*if (s->version <= SCI_VERSION_LTU_PRIORITY_OB1)
		++y; */

	if (s->pic_priority_table) { // SCI01 priority table set?
		int j;
		for (j = 0; j < 15; j++)
			if (y < s->pic_priority_table[j+1])
				return j;
		return 14; // Maximum
	} else {
		if (!((SciEngine*)g_engine)->getKernel()->usesOldGfxFunctions())
			return SCI0_VIEW_PRIORITY_14_ZONES(y);
		else
			return SCI0_VIEW_PRIORITY(y) == 15 ? 14 : SCI0_VIEW_PRIORITY(y);
	}
}

int _find_priority_band(EngineState *s, int nr) {
	if (!((SciEngine*)g_engine)->getKernel()->usesOldGfxFunctions() && (nr < 0 || nr > 14)) {
		if (nr == 15)
			return 0xffff;
		else {
			warning("Attempt to get priority band %d", nr);
		}
		return 0;
	}

	if (((SciEngine*)g_engine)->getKernel()->usesOldGfxFunctions() && (nr < 0 || nr > 15)) {
		warning("Attempt to get priority band %d", nr);
		return 0;
	}

	if (s->pic_priority_table) // SCI01 priority table set?
		return s->pic_priority_table[nr];
	else {
		int retval;

		if (!((SciEngine*)g_engine)->getKernel()->usesOldGfxFunctions())
			retval = SCI0_PRIORITY_BAND_FIRST_14_ZONES(nr);
		else
			retval = SCI0_PRIORITY_BAND_FIRST(nr);

/*		if (s->version <= SCI_VERSION_LTU_PRIORITY_OB1)
			--retval; */
		return retval;
	}
}

reg_t graph_save_box(EngineState *s, rect_t area) {
	reg_t handle = kalloc(s->segMan, "graph_save_box()", sizeof(gfxw_snapshot_t *));
	gfxw_snapshot_t **ptr = (gfxw_snapshot_t **)kmem(s->segMan, handle);

	// FIXME: gfxw_make_snapshot returns a pointer. Now why do we store a
	// pointer to real memory inside the SCI heap?
	// If we save and the load again, this cannot work in general.
	// This seems like bad design. Either the snapshot data itself should be
	// stored in the heap, or a unique persistent id.
	*ptr = gfxw_make_snapshot(s->visual, area);

	return handle;
}

void graph_restore_box(EngineState *s, reg_t handle) {
	gfxw_snapshot_t **ptr;
	int port_nr = s->port->_ID;

	if (!handle.segment) {
		warning("Attempt to restore box with zero handle");
		return;
	}

	ptr = (gfxw_snapshot_t **)kmem(s->segMan, handle);

	if (!ptr) {
		warning("Attempt to restore invalid handle %04x:%04x", PRINT_REG(handle));
		return;
	}

	while (port_nr > 2 && !(s->port->_flags & GFXW_FLAG_IMMUNE_TO_SNAPSHOTS) && (gfxw_widget_matches_snapshot(*ptr, s->port))) {
		// This shouldn't ever happen, actually, since windows (ports w/ ID > 2) should all be immune
		GfxPort *newport = s->visual->getPort(port_nr);
		error("Port %d is not immune against snapshots", s->port->_ID);
		port_nr--;
		if (newport)
			s->port = newport;
	}

	if (s->dyn_views && gfxw_widget_matches_snapshot(*ptr, s->dyn_views->_parent)) {
		GfxContainer *parent = s->dyn_views->_parent;

		do {
			parent = parent->_parent;
		} while (parent && (gfxw_widget_matches_snapshot(*ptr, parent)));

		if (!parent) {
			error("Attempted widget mass destruction by a snapshot");
		}

		reparentize_primary_widget_lists(s, (GfxPort *) parent);
	}


	if (!ptr) {
		error("Attempt to restore invalid snaphot with handle %04x:%04x", PRINT_REG(handle));
		return;
	}

	gfxw_restore_snapshot(s->visual, *ptr);
	free(*ptr);
	*ptr = NULL;

	kfree(s->segMan, handle);
}

PaletteEntry get_pic_color(EngineState *s, int color) {
	if (!s->resMan->isVGA())
		return s->ega_colors[color].visual;

	if (color == -1 || color == 255)     // -1 occurs in Eco Quest 1. Not sure if this is the best approach, but it seems to work
		return PaletteEntry(255,255,255);
	else if (color < s->gfx_state->gfxResMan->getColorCount())
		return s->gfx_state->gfxResMan->getColor(color);
	else {
		// Happens in the beginning of EcoQuest 2, when the dialog box of the customs officer shows up
		warning("Color index %d out of bounds for pic %d (%d max)", color, s->gfx_state->pic_nr, s->gfx_state->gfxResMan->getColorCount());
		return PaletteEntry(0,0,0);
	}
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

static reg_t kSetCursorSci0(EngineState *s, int, int argc, reg_t *argv) {
	int16 cursor = argv[0].toSint16();

	if ((argc >= 2) && (argv[1].toSint16() == 0))
		cursor = GFXOP_NO_POINTER;

	gfxop_set_pointer_cursor(s->gfx_state, cursor);

	// Set pointer position, if requested
	if (argc >= 4) {
		Common::Point newPos = Common::Point(argv[2].toSint16() + s->port->_bounds.x, argv[3].toSint16() + s->port->_bounds.y);
		gfxop_set_pointer_position(s->gfx_state, newPos);
	}

	return s->r_acc;
}

static reg_t kSetCursorSci11(EngineState *s, int, int argc, reg_t *argv) {
	Common::Point *hotspot = NULL;

	switch (argc) {
	case 1:
		CursorMan.showMouse(argv[0].toSint16() != 0);
		break;
	case 2:
		gfxop_set_pointer_position(s->gfx_state,
				   Common::Point(argv[0].toUint16() + s->port->_bounds.x, argv[1].toUint16() + s->port->_bounds.y));
		break;
	case 4: {
		int16 top = argv[0].toSint16();
		int16 left = argv[1].toSint16();
		int16 bottom = argv[2].toSint16();
		int16 right = argv[3].toSint16();

		if ((right >= left) && (bottom >= top)) {
			Common::Rect rect = Common::Rect(left, top, right + 1, bottom + 1);
			gfxop_set_pointer_zone(s->gfx_state, rect);
		} else {
			warning("kSetCursor: Ignoring invalid mouse zone (%i, %i)-(%i, %i)", left, top, right, bottom);
		}
		break;
	}
	case 5:
	case 9:
		hotspot = new Common::Point(argv[3].toSint16(), argv[4].toSint16());
		// Fallthrough
	case 3:
		gfxop_set_pointer_view(s->gfx_state, argv[0].toUint16(), argv[1].toUint16(), argv[2].toUint16(), hotspot);
		if (hotspot)
			delete hotspot;
		break;
	default :
		warning("kSetCursor: Unhandled case: %d arguments given", argc);
		break;
	}
	return s->r_acc;
}

reg_t kSetCursor(EngineState *s, int, int argc, reg_t *argv) {
	switch (s->detectSetCursorType()) {
	case SCI_VERSION_0_EARLY:
		return kSetCursorSci0(s, WAS_FUNCT_NR, argc, argv);
	case SCI_VERSION_1_1:
		return kSetCursorSci11(s, WAS_FUNCT_NR, argc, argv);
	default:
		warning("Unknown SetCursor type");
		return NULL_REG;
	}
}

reg_t kMoveCursor(EngineState *s, int, int argc, reg_t *argv) {
	Common::Point newPos;

	newPos = s->gfx_state->pointer_pos;

	if (argc == 1) {
		// Case ignored on IBM PC
	} else {
		newPos.x = argv[0].toSint16() + s->port->zone.x;
		newPos.y = argv[1].toSint16() + s->port->zone.y;

		if (newPos.x > s->port->zone.x + s->port->zone.width)
			newPos.x = s->port->zone.x + s->port->zone.width;
		if (newPos.y > s->port->zone.y + s->port->zone.height)
			newPos.y = s->port->zone.y + s->port->zone.height;

		if (newPos.x < 0) newPos.x = 0;
		if (newPos.y < 0) newPos.y = 0;
	}

	gfxop_set_pointer_position(s->gfx_state, newPos);

	return s->r_acc;
}

reg_t kShow(EngineState *s, int, int argc, reg_t *argv) {
	int old_map = s->pic_visible_map;

	s->pic_visible_map = (argc > 0) ? (gfx_map_mask_t) argv[0].toUint16() : GFX_MASK_VISUAL;

	switch (s->pic_visible_map) {

	case GFX_MASK_VISUAL:
	case GFX_MASK_PRIORITY:
	case GFX_MASK_CONTROL:
		gfxop_set_visible_map(s->gfx_state, s->pic_visible_map);
		if (old_map != s->pic_visible_map) {

			if (s->pic_visible_map == GFX_MASK_VISUAL) // Full widget redraw
				s->visual->draw(Common::Point(0, 0));

			gfxop_update(s->gfx_state);
			debugC(2, kDebugLevelGraphics, "Switching visible map to %x\n", s->pic_visible_map);
		}
		break;

	default:
		warning("Show(%x) selects unknown map", s->pic_visible_map);

	}

	s->pic_not_valid = 2;
	return s->r_acc;
}

reg_t kPicNotValid(EngineState *s, int, int argc, reg_t *argv) {
	s->r_acc = make_reg(0, s->pic_not_valid);
	if (argc)
		s->pic_not_valid = (byte)argv[0].toUint16();

	return s->r_acc;
}

void _k_redraw_box(EngineState *s, int x1, int y1, int x2, int y2) {
	warning("_k_redraw_box(): Unimplemented");
#if 0
	int i;
	ViewObject *list = s->dyn_views;

	printf("Reanimating views\n", s->dyn_views_nr);

	for (i = 0;i < s->dyn_views_nr;i++) {
		*(list[i].underBitsp) = graph_save_box(s, list[i].nsLeft, list[i].nsTop, list[i].nsRight - list[i].nsLeft,
												list[i].nsBottom - list[i].nsTop, SCI_MAP_VISUAL | SCI_MAP_PRIORITY);
		draw_view0(s->pic, s->ports[0], list[i].nsLeft, list[i].nsTop, list[i].priority, list[i].loop,
		           list[i].cel, 0, list[i].view);
	}

	graph_update_box(s, x1, y1, x2 - x1, y2 - y1);

	for (i = 0;i < s->dyn_views_nr;i++)	{
		graph_restore_box(s, *(list[i].underBitsp));
		list[i].underBits = 0;
	}
#endif
}

void _k_graph_rebuild_port_with_color(EngineState *s, gfx_color_t newbgcolor) {
	GfxPort *port = s->port;
	GfxPort *newport;

	newport = sciw_new_window(s, port->zone, port->_font, port->_color, newbgcolor,
	                          s->titlebar_port->_font, s->ega_colors[15], s->ega_colors[8],
	                          port->title_text, port->port_flags & ~kWindowTransparent);

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

static bool activated_icon_bar = false;	// FIXME: Avoid non-const global vars
static int port_origin_x = 0;	// FIXME: Avoid non-const global vars
static int port_origin_y = 0;	// FIXME: Avoid non-const global vars

reg_t kGraph(EngineState *s, int, int argc, reg_t *argv) {
	rect_t area;
	GfxPort *port = s->port;
	int redraw_port = 0;

	area = gfx_rect(argv[2].toSint16(), argv[1].toSint16() , argv[4].toSint16(), argv[3].toSint16());

	area.width = area.width - area.x; // Since the actual coordinates are absolute
	area.height = area.height - area.y;

	switch (argv[0].toSint16()) {

	case K_GRAPH_GET_COLORS_NR:

		return make_reg(0, !s->resMan->isVGA() ? 0x10 : 0x100);
		break;

	case K_GRAPH_DRAW_LINE: {
		int16 priority = (argc > 6) ? argv[6].toSint16() : -1;
		int16 control = (argc > 7) ? argv[7].toSint16() : -1;
		gfx_color_t gfxcolor = graph_map_color(s, argv[5].toSint16(), priority, control);

		debugC(2, kDebugLevelGraphics, "draw_line((%d, %d), (%d, %d), col=%d, p=%d, c=%d, mask=%d)\n",
		          argv[2].toSint16(), argv[1].toSint16(), argv[4].toSint16(), argv[3].toSint16(), argv[5].toSint16(), priority, control, gfxcolor.mask);

		redraw_port = 1;

		// Note: it's quite possible that the coordinates of the line will *not* form a valid rectangle (e.g. it might
		// have negative width/height). The actual dirty rectangle is constructed in gfxdr_add_dirty().
		// FIXME/TODO: We need to change the semantics of this call, so that no fake rectangles are used. As it is, it's
		// not possible change rect_t to Common::Rect, as we assume that Common::Rect forms a *valid* rectangle.
		ADD_TO_CURRENT_PICTURE_PORT(gfxw_new_line(Common::Point(argv[2].toSint16(), argv[1].toSint16()), Common::Point(argv[4].toSint16(), argv[3].toSint16()),
		                               gfxcolor, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL));

	}
	break;

	case K_GRAPH_SAVE_BOX:

		area.x += s->port->zone.x + port_origin_x;
		area.y += s->port->zone.y + port_origin_y;
		area.width += -port_origin_x;
		area.height += -port_origin_y;

		return(graph_save_box(s, area));
		break;

	case K_GRAPH_RESTORE_BOX:

		graph_restore_box(s, argv[1]);
		break;

	case K_GRAPH_FILL_BOX_BACKGROUND:

		_k_graph_rebuild_port_with_color(s, port->_bgcolor);
		port = s->port;

		redraw_port = 1;
		break;

	case K_GRAPH_FILL_BOX_FOREGROUND:

		_k_graph_rebuild_port_with_color(s, port->_color);
		port = s->port;

		redraw_port = 1;
		break;

	case K_GRAPH_FILL_BOX_ANY: {
		int16 priority = (argc > 7) ? argv[7].toSint16() : -1;
		int16 control = (argc > 8) ? argv[8].toSint16() : -1;
		gfx_color_t color = graph_map_color(s, argv[6].toSint16(), priority, control);

		color.mask = (byte)argv[5].toUint16();

		debugC(2, kDebugLevelGraphics, "fill_box_any((%d, %d), (%d, %d), col=%d, p=%d, c=%d, mask=%d)\n",
		          argv[2].toSint16(), argv[1].toSint16(), argv[4].toSint16(), argv[3].toSint16(), argv[6].toSint16(), priority, control, argv[5].toUint16());

		// FIXME/TODO: this is not right, as some of the dialogs are drawn *behind* some widgets. But at least it works for now
		//ADD_TO_CURRENT_PICTURE_PORT(gfxw_new_box(s->gfx_state, area, color, color, GFX_BOX_SHADE_FLAT));	// old code
		s->picture_port->add((GfxContainer *)s->picture_port, gfxw_new_box(s->gfx_state, area, color, color, GFX_BOX_SHADE_FLAT));

	}
	break;

	case K_GRAPH_UPDATE_BOX: {

		debugC(2, kDebugLevelGraphics, "update_box(%d, %d, %d, %d)\n", argv[1].toSint16(), argv[2].toSint16(), argv[3].toSint16(), argv[4].toSint16());

		area.x += s->port->zone.x;
		area.y += s->port->zone.y;

		gfxop_update_box(s->gfx_state, area);

	}
	break;

	case K_GRAPH_REDRAW_BOX: {


		debugC(2, kDebugLevelGraphics, "redraw_box(%d, %d, %d, %d)\n", argv[1].toSint16(), argv[2].toSint16(), argv[3].toSint16(), argv[4].toSint16());

		area.x += s->port->zone.x;
		area.y += s->port->zone.y;

		if (s->dyn_views && s->dyn_views->_parent == (GfxContainer *)s->port)
			s->dyn_views->draw(Common::Point(0, 0));

		gfxop_update_box(s->gfx_state, area);

	}

	break;

	case K_GRAPH_ADJUST_PRIORITY:

		debugC(2, kDebugLevelGraphics, "adjust_priority(%d, %d)\n", argv[1].toSint16(), argv[2].toSint16());
		s->priority_first = argv[1].toSint16() - 10;
		s->priority_last = argv[2].toSint16() - 10;
		break;

	default:

		warning("Unhandled Graph() operation %04x", argv[0].toSint16());

	}

	if (redraw_port)
		FULL_REDRAW();

	gfxop_update(s->gfx_state);

	return s->r_acc;
}

reg_t kTextSize(EngineState *s, int, int argc, reg_t *argv) {
	int width, height;
	char *text = argv[1].segment ? (char *) kernelDerefBulkPtr(s->segMan, argv[1], 0) : NULL;
	const char *sep = NULL; 
	reg_t *dest = kernelDerefRegPtr(s->segMan, argv[0], 4);
	int maxwidth = (argc > 3) ? argv[3].toUint16() : 0;
	int font_nr = argv[2].toUint16();

	if ((argc > 4) && (argv[4].segment))
		sep = (const char *)kernelDerefBulkPtr(s->segMan, argv[4], 0);

	if (maxwidth < 0)
		maxwidth = 0;

	dest[0] = dest[1] = NULL_REG;

	if (!text || !*text || !dest) { // Empty text
		dest[2] = dest[3] = make_reg(0, 0);
		debugC(2, kDebugLevelStrings, "GetTextSize: Empty string\n");
		return s->r_acc;
	}

	gfxop_get_text_params(s->gfx_state, font_nr, s->strSplit(text, sep).c_str(), maxwidth ? maxwidth : MAX_TEXT_WIDTH_MAGIC_VALUE,
	                                 &width, &height, 0, NULL, NULL, NULL);
	debugC(2, kDebugLevelStrings, "GetTextSize '%s' -> %dx%d\n", text, width, height);

	dest[2] = make_reg(0, height);
//	dest[3] = make_reg(0, maxwidth? maxwidth : width);
	dest[3] = make_reg(0, width);

	return s->r_acc;
}

reg_t kWait(EngineState *s, int, int argc, reg_t *argv) {
	uint32 time;
	int sleep_time = argv[0].toUint16();

	time = g_system->getMillis();
	s->r_acc = make_reg(0, ((long)time - (long)s->last_wait_time) * 60 / 1000);
	s->last_wait_time = time;

	sleep_time *= g_debug_sleeptime_factor;
	gfxop_sleep(s->gfx_state, sleep_time * 1000 / 60);

	// Reset speed throttler: Game is playing along nicely anyway
	if (sleep_time > 0)
		s->speedThrottler->reset();

	return s->r_acc;
}

reg_t kCoordPri(EngineState *s, int, int argc, reg_t *argv) {
	int y = argv[0].toSint16();

	return make_reg(0, _find_view_priority(s, y));
}

reg_t kPriCoord(EngineState *s, int, int argc, reg_t *argv) {
	int priority = argv[0].toSint16();

	return make_reg(0, _find_priority_band(s, priority));
}

void _k_dirloop(reg_t obj, uint16 angle, EngineState *s, int argc, reg_t *argv) {
	SegManager *segManager = s->segMan;
	int view = GET_SEL32V(obj, view);
	int signal = GET_SEL32V(obj, signal);
	int loop;
	int maxloops;
	bool oldScriptHeader = (s->resMan->sciVersion() == SCI_VERSION_0_EARLY);

	if (signal & _K_VIEW_SIG_FLAG_DOESNT_TURN)
		return;

	angle %= 360;

	if (!oldScriptHeader) {
		if (angle < 45)
			loop = 3;
		else if (angle < 136)
			loop = 0;
		else if (angle < 225)
			loop = 2;
		else if (angle < 316)
			loop = 1;
		else
			loop = 3;
	} else {
		if (angle >= 330 || angle <= 30)
			loop = 3;
		else if (angle <= 150)
			loop = 0;
		else if (angle <= 210)
			loop = 2;
		else if (angle < 330)
			loop = 1;
		else loop = 0xffff;
	}

	maxloops = gfxop_lookup_view_get_loops(s->gfx_state, view);

	if ((loop > 1) && (maxloops < 4))
		return;

	PUT_SEL32V(obj, loop, loop);
}

reg_t kDirLoop(EngineState *s, int, int argc, reg_t *argv) {
	_k_dirloop(argv[0], argv[1].toUint16(), s, argc, argv);

	return s->r_acc;
}

#define GASEOUS_VIEW_MASK_ACTIVE (_K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_IGNORE_ACTOR)
#define GASEOUS_VIEW_MASK_PASSIVE (_K_VIEW_SIG_FLAG_NO_UPDATE | _K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_IGNORE_ACTOR)

static Common::Rect nsrect_clip(EngineState *s, int y, Common::Rect retval, int priority);

static int collides_with(EngineState *s, Common::Rect area, reg_t other_obj, int use_nsrect, int view_mask, int argc, reg_t *argv) {
	SegManager *segManager = s->segMan;
	int other_signal = GET_SEL32V(other_obj, signal);
	int other_priority = GET_SEL32V(other_obj, priority);
	int y = (int16)GET_SEL32V(other_obj, y);
	Common::Rect other_area;

	if (use_nsrect) {
		other_area = get_nsrect(s, other_obj, 0);
		other_area = nsrect_clip(s, y, other_area, other_priority);
	} else {
		other_area.left = GET_SEL32V(other_obj, brLeft);
		other_area.right = GET_SEL32V(other_obj, brRight);
		other_area.top = GET_SEL32V(other_obj, brTop);
		other_area.bottom = GET_SEL32V(other_obj, brBottom);
	}

	if (other_area.right < 0 || other_area.bottom < 0 || area.right < 0 || area.bottom < 0)
		return 0; // Out of scope

	if (other_area.left >= 320 || other_area.top >= 190 || area.right >= 320 || area.bottom >= 190)
		return 0; // Out of scope

	debugC(2, kDebugLevelBresen, "OtherSignal=%04x, z=%04x obj=%04x:%04x\n", other_signal, (other_signal & view_mask), PRINT_REG(other_obj));

	if ((other_signal & (view_mask)) == 0) {
		// check whether the other object ignores actors

		debugC(2, kDebugLevelBresen, "  against (%d,%d) to (%d,%d)\n", other_area.left, other_area.top, other_area.right, other_area.bottom);

		if (area.intersects(other_area))
			return 1;
		/* CR (from :Bob Heitman:) Collision rects have Mac semantics, ((0,0),(1,1)) only
		** covers the coordinate (0,0) */
	}

	debugC(2, kDebugLevelBresen, " (no)\n");
	return 0;
}

reg_t kCanBeHere(EngineState *s, int, int argc, reg_t *argv) {
	SegManager *segManager = s->segMan;
	reg_t obj = argv[0];
	reg_t cliplist_ref = (argc > 1) ? argv[1] : NULL_REG;
	List *cliplist = NULL;
	GfxPort *port = s->picture_port;
	uint16 signal;
	int retval;

	Common::Rect abs_zone;
	rect_t zone;
	uint16 edgehit;
	uint16 illegal_bits;

	abs_zone.left = (int16)GET_SEL32V(obj, brLeft);
	abs_zone.right = (int16)GET_SEL32V(obj, brRight);
	abs_zone.top = (int16)GET_SEL32V(obj, brTop);
	abs_zone.bottom = (int16)GET_SEL32V(obj, brBottom);

	zone = gfx_rect(abs_zone.left + port->zone.x, abs_zone.top + port->zone.y, abs_zone.width(), abs_zone.height());

	signal = GET_SEL32V(obj, signal);
	debugC(2, kDebugLevelBresen, "Checking collision: (%d,%d) to (%d,%d) ([%d..%d]x[%d..%d]), obj=%04x:%04x, sig=%04x, cliplist=%04x:%04x\n",
	          GFX_PRINT_RECT(zone), abs_zone.left, abs_zone.right, abs_zone.top, abs_zone.bottom,
	          PRINT_REG(obj), signal, PRINT_REG(cliplist_ref));

	illegal_bits = GET_SEL32V(obj, illegalBits);

	retval = !(illegal_bits & (edgehit = gfxop_scan_bitmask(s->gfx_state, zone, GFX_MASK_CONTROL)));

	debugC(2, kDebugLevelBresen, "edgehit = %04x (illegalBits %04x)\n", edgehit, illegal_bits);
	if (retval == 0) {
		debugC(2, kDebugLevelBresen, " -> %04x\n", retval);
		return not_register(s, NULL_REG); // Can't BeHere
	}

	retval = 0;

	if ((illegal_bits & 0x8000) // If we are vulnerable to those views at all...
	        && s->dyn_views) { // ...check against all stop-updated dynviews
		GfxDynView *widget = (GfxDynView *)s->dyn_views->_contents;

		debugC(2, kDebugLevelBresen, "Checking vs dynviews:\n");

		while (widget) {
			if (widget->_ID && (widget->signal & _K_VIEW_SIG_FLAG_STOPUPD)
			        && ((widget->_ID != obj.segment) || (widget->_subID != obj.offset))
			        && is_object(s->segMan, make_reg(widget->_ID, widget->_subID)))
				if (collides_with(s, abs_zone, make_reg(widget->_ID, widget->_subID), 1, GASEOUS_VIEW_MASK_ACTIVE, argc, argv))
					return not_register(s, NULL_REG);

			widget = (GfxDynView *)widget->_next;
		}
	}

	if (signal & GASEOUS_VIEW_MASK_ACTIVE) {
		retval = signal & GASEOUS_VIEW_MASK_ACTIVE; // CanBeHere- it's either being disposed, or it ignores actors anyway
		debugC(2, kDebugLevelBresen, " -> %04x\n", retval);
		return not_register(s, make_reg(0, retval)); // CanBeHere
	}

	if (cliplist_ref.segment)
		cliplist = lookup_list(s, cliplist_ref);

	if (cliplist) {
		Node *node = lookup_node(s, cliplist->first);

		retval = 0; // Assume that we Can'tBeHere...

		while (node) { // Check each object in the list against our bounding rectangle
			reg_t other_obj = node->value;
			debugC(2, kDebugLevelBresen, "  comparing against %04x:%04x\n", PRINT_REG(other_obj));

			if (!is_object(s->segMan, other_obj)) {
				warning("CanBeHere() cliplist contains non-object %04x:%04x", PRINT_REG(other_obj));
			} else if (other_obj != obj) { // Clipping against yourself is not recommended

				if (collides_with(s, abs_zone, other_obj, 0, GASEOUS_VIEW_MASK_PASSIVE, argc, argv)) {
					debugC(2, kDebugLevelBresen, " -> %04x\n", retval);
					return not_register(s, NULL_REG);
				}

			} // if (other_obj != obj)
			node = lookup_node(s, node->succ); // move on
		}
	}

	if (!retval)
		retval = 1;
	debugC(2, kDebugLevelBresen, " -> %04x\n", retval);

	return not_register(s, make_reg(0, retval));
}  // CanBeHere

reg_t kIsItSkip(EngineState *s, int, int argc, reg_t *argv) {
	int view = argv[0].toSint16();
	int loop = argv[1].toSint16();
	int cel = argv[2].toSint16();
	int y = argv[3].toUint16();
	int x = argv[4].toUint16();
	gfxr_view_t *res = NULL;
	gfx_pixmap_t *pxm = NULL;

	res = s->gfx_state->gfxResMan->getView(view, &loop, &cel, 0);

	if (!res) {
		warning("[GFX] Attempt to get cel parameters for invalid view %d", view);
		return make_reg(0, -1);
	}

	pxm = res->loops[loop].cels[cel];
	if (x > pxm->index_width)
		x = pxm->index_width - 1;
	if (y > pxm->index_height)
		y = pxm->index_height - 1;

	return make_reg(0, pxm->index_data[y * pxm->index_width + x] == pxm->color_key);
}

reg_t kCelHigh(EngineState *s, int, int argc, reg_t *argv) {
	int view = argv[0].toSint16();
	int loop = argv[1].toSint16();
	int cel = argv[2].toSint16();
	int height, width;
	Common::Point offset;

	if (argc != 3) {
		warning("CelHigh called with %d parameters", argc);
	}

	gfxop_get_cel_parameters(s->gfx_state, view, loop, cel, &width, &height, &offset);
	return make_reg(0, height);
}

reg_t kCelWide(EngineState *s, int, int argc, reg_t *argv) {
	int view = argv[0].toSint16();
	int loop = argv[1].toSint16();
	int cel = argv[2].toSint16();
	int height, width;
	Common::Point offset;

	if (argc != 3) {
		warning("CelHigh called with %d parameters", argc);
	}

	gfxop_get_cel_parameters(s->gfx_state, view, loop, cel, &width, &height, &offset);
	return make_reg(0, width);
}

reg_t kNumLoops(EngineState *s, int, int argc, reg_t *argv) {
	SegManager *segManager = s->segMan;
	reg_t obj = argv[0];
	int view = GET_SEL32V(obj, view);
	int loops_nr = gfxop_lookup_view_get_loops(s->gfx_state, view);

	if (loops_nr < 0) {
		error("view.%d (0x%x) not found", view, view);
		return NULL_REG;
	}

	debugC(2, kDebugLevelGraphics, "NumLoops(view.%d) = %d\n", view, loops_nr);

	return make_reg(0, loops_nr);
}

reg_t kNumCels(EngineState *s, int, int argc, reg_t *argv) {
	SegManager *segManager = s->segMan;
	reg_t obj = argv[0];
	int loop = GET_SEL32V(obj, loop);
	int view = GET_SEL32V(obj, view);
	int cel = 0xffff;

	gfxop_check_cel(s->gfx_state, view, &loop, &cel);

	debugC(2, kDebugLevelGraphics, "NumCels(view.%d, %d) = %d\n", view, loop, cel + 1);

	return make_reg(0, cel + 1);
}

reg_t kOnControl(EngineState *s, int, int argc, reg_t *argv) {
	int arg = 0;
	gfx_map_mask_t map;
	int xstart, ystart;
	int xlen = 1, ylen = 1;

	if (argc == 2 || argc == 4)
		map = GFX_MASK_CONTROL;
	else {
		arg = 1;
		map = (gfx_map_mask_t) argv[0].toSint16();
	}

	ystart = argv[arg + 1].toSint16();
	xstart = argv[arg].toSint16();

	if (argc > 3) {
		ylen = argv[arg + 3].toSint16() - ystart;
		xlen = argv[arg + 2].toSint16() - xstart;
	}

	return make_reg(0, gfxop_scan_bitmask(s->gfx_state, gfx_rect(xstart, ystart + 10, xlen, ylen), map));
}

void _k_view_list_free_backgrounds(EngineState *s, ViewObject *list, int list_nr);

#define K_DRAWPIC_FLAG_MIRRORED (1 << 14)

reg_t kDrawPic(EngineState *s, int, int argc, reg_t *argv) {
	drawn_pic_t dp;
	bool add_to_pic = (argc > 2) ? !argv[2].toSint16() : false;
	gfx_color_t transparent = s->wm_port->_bgcolor;
	int picFlags = DRAWPIC01_FLAG_FILL_NORMALLY;

	if (((SciEngine*)g_engine)->getKernel()->usesOldGfxFunctions())
		add_to_pic = (argc > 2) ? argv[2].toSint16() : false;

	dp.nr = argv[0].toSint16();
	dp.palette = (argc > 3) ? argv[3].toSint16() : 0;

	if ((argc > 1) && (argv[1].toUint16() & K_DRAWPIC_FLAG_MIRRORED))
		picFlags |= DRAWPIC1_FLAG_MIRRORED;

	gfxop_disable_dirty_frames(s->gfx_state);

	if (NULL != s->old_screen) {
		gfxop_free_pixmap(s->gfx_state, s->old_screen);
	}

	s->old_screen = gfxop_grab_pixmap(s->gfx_state, gfx_rect(0, 10, 320, 190));

	debugC(2, kDebugLevelGraphics, "Drawing pic.%03d\n", argv[0].toSint16());

	if (add_to_pic) {
		s->_pics.push_back(dp);
		gfxop_add_to_pic(s->gfx_state, dp.nr, picFlags, dp.palette);
	} else {
		s->_pics.clear();
		s->_pics.push_back(dp);
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

	if (argc > 1)
		s->pic_animate = argv[1].toSint16() & 0xff; // The animation used during kAnimate() later on

	s->dyn_views = NULL;
	s->drop_views = NULL;

	s->priority_first = 42;

	if (((SciEngine*)g_engine)->getKernel()->usesOldGfxFunctions())
		s->priority_last = 200;
	else
		s->priority_last = 190;

	s->pic_not_valid = 1;
	s->pic_is_new = 1;

	return s->r_acc;

}

Common::Rect set_base(EngineState *s, reg_t object) {
	SegManager *segManager = s->segMan;
	int x, y, original_y, z, ystep, xsize, ysize;
	int xbase, ybase, xend, yend;
	int view, loop, cel;
	int oldloop, oldcel;
	int xmod = 0, ymod = 0;
	Common::Rect retval;

	x = (int16)GET_SEL32V(object, x);
	original_y = y = (int16)GET_SEL32V(object, y);

	if (((SciEngine*)g_engine)->getKernel()->_selectorMap.z > -1)
		z = (int16)GET_SEL32V(object, z);
	else
		z = 0;

	y -= z; // Subtract z offset

	ystep = (int16)GET_SEL32V(object, yStep);

	view = (int16)GET_SEL32V(object, view);
	oldloop = loop = sign_extend_byte(GET_SEL32V(object, loop));
	oldcel = cel = sign_extend_byte(GET_SEL32V(object, cel));

	Common::Point offset = Common::Point(0, 0);

	if (loop != oldloop) {
		loop = 0;
		PUT_SEL32V(object, loop, 0);
		debugC(2, kDebugLevelGraphics, "Resetting loop for %04x:%04x!\n", PRINT_REG(object));
	}

	if (cel != oldcel) {
		cel = 0;
		PUT_SEL32V(object, cel, 0);
	}

	gfxop_get_cel_parameters(s->gfx_state, view, loop, cel, &xsize, &ysize, &offset);

	xmod = offset.x;
	ymod = offset.y;

	xbase = x - xmod - (xsize >> 1);
	xend = xbase + xsize;
	yend = y /* - ymod */ + 1;
	ybase = yend - ystep;

	debugC(2, kDebugLevelBaseSetter, "(%d,%d)+/-(%d,%d), (%d x %d) -> (%d, %d) to (%d, %d)\n",
	          x, y, xmod, ymod, xsize, ysize, xbase, ybase, xend, yend);

	retval.left = xbase;
	retval.top = ybase;
	retval.right = xend;
	retval.bottom = yend;

	return retval;
}

void _k_base_setter(EngineState *s, reg_t object) {
	SegManager *segManager = s->segMan;
	Common::Rect absrect = set_base(s, object);

	if (lookup_selector(s->segMan, object, ((SciEngine*)g_engine)->getKernel()->_selectorMap.brLeft, NULL, NULL) != kSelectorVariable)
		return; // non-fatal

	// Note: there was a check here for a very old version of SCI, which supposedly needed
	// to subtract 1 from absrect.top. The original check was for version 0.000.256, which
	// does not exist (earliest one was KQ4 SCI, version 0.000.274). This code is left here
	// for reference only
#if 0
	if (s->resMan->sciVersion() <= SCI_VERSION_0)
		--absrect.top; // Compensate for early SCI OB1 'bug'
#endif

	PUT_SEL32V(object, brLeft, absrect.left);
	PUT_SEL32V(object, brRight, absrect.right);
	PUT_SEL32V(object, brTop, absrect.top);
	PUT_SEL32V(object, brBottom, absrect.bottom);
}

reg_t kBaseSetter(EngineState *s, int, int argc, reg_t *argv) {
	reg_t object = argv[0];

	_k_base_setter(s, object);

	return s->r_acc;
} // kBaseSetter

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

Common::Rect get_nsrect(EngineState *s, reg_t object, byte clip) {
	SegManager *segManager = s->segMan;
	int x, y, z;
	int view, loop, cel;
	Common::Rect retval;

	x = (int16)GET_SEL32V(object, x);
	y = (int16)GET_SEL32V(object, y);

	if (((SciEngine*)g_engine)->getKernel()->_selectorMap.z > -1)
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

static void _k_set_now_seen(EngineState *s, reg_t object) {
	SegManager *segManager = s->segMan;
	Common::Rect absrect = get_nsrect(s, object, 0);

	if (lookup_selector(s->segMan, object, ((SciEngine*)g_engine)->getKernel()->_selectorMap.nsTop, NULL, NULL) != kSelectorVariable) {
		return;
	} // This isn't fatal

	PUT_SEL32V(object, nsLeft, absrect.left);
	PUT_SEL32V(object, nsRight, absrect.right);
	PUT_SEL32V(object, nsTop, absrect.top);
	PUT_SEL32V(object, nsBottom, absrect.bottom);
}

reg_t kSetNowSeen(EngineState *s, int, int argc, reg_t *argv) {
	reg_t object = argv[0];

	_k_set_now_seen(s, object);

	return s->r_acc;
}

reg_t kPalette(EngineState *s, int, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case 1:
		debug(5, "STUB: kPalette() effect 1, direct palette set");
		break;
	case 2:
		debug(5, "STUB: kPalette() effect 2, set flag to colors");
		break;
	case 3:
		debug(5, "STUB: kPalette() effect 3, clear flag to colors");
		break;
	case 4:	{	// Set palette intensity
#if 0
		// Colors 0 (black) and 255 (white) cannot be changed
		int16 from = CLIP<int16>(1, 255, argv[2].toUint16());
		int16 to = CLIP<int16>(1, 255, argv[3].toUint16());
		int16 intensity = argv[4].toUint16();

		if (argc < 5 || argv[5].toUint16() == 0) {
			s->gfx_state->gfxResMan->setPaletteIntensity(from, to, intensity);
		} else {
			warning("kPalette: argv[5] != 0");
		}

		return s->r_acc;
#endif
		debug(5, "STUB: kPalette() effect 4, set color intensity");
		break;
		}
	case 5: {	// Find closest color
		int r = argv[1].toUint16();
		int g = argv[2].toUint16();
		int b = argv[3].toUint16();

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
		return make_reg(0, bestindex);
	}
	case 6:
		debug(5, "STUB: kPalette() effect 6, animate palette");
		break;
	case 7:
		debug(5, "STUB: kPalette() effect 7, save palette to heap");
		break;
	case 8:
		debug(5, "STUB: kPalette() effect 8, set stored palette");
		break;
	default:
		warning("kPalette(): Unimplemented subfunction: %d", argv[0].toUint16());
	}

	return s->r_acc;
}

reg_t kPalVary(EngineState *s, int, int argc, reg_t *argv) {
	warning("STUB: kPalVary()");
	return NULL_REG;
}

static void _k_draw_control(EngineState *s, reg_t obj, int inverse);

static void _k_disable_delete_for_now(EngineState *s, reg_t obj) {
	SegManager *segManager = s->segMan;
	reg_t text_pos = GET_SEL32(obj, text);
	char *text = text_pos.isNull() ? NULL : (char *)s->segMan->dereference(text_pos, NULL);
	int type = GET_SEL32V(obj, type);
	int state = GET_SEL32V(obj, state);

	/*
	 * WORKAROUND: The function is a "prevent the user from doing something
	 * nasty" type of thing, and goes back to the ugly way in which savegame
	 * deletion is implemented in SCI (and even worse in SQ4/Floppy, for
	 * which the workaround is intended). The result is basically that you
	 * can't implement savegame deletion for SQ4/Floppy unless you duplicate
	 * the exact naming scheme of savefiles (i.e. savefiles must be named
	 * SQ4SG.<number>) and the exact file format of the savegame index
	 * (SQ4SG.DIR). From the earlier discussions on file I/O handling -
	 * before as well as after the merge - I gather that this is not an
	 * option.
	 *
	 * SQ4/Floppy is special, being the first game to implement savegame
	 * deletion at all. For later games, we manage to implement deletion by
	 * using gross hacks in kDeviceInfo() (essentially repurposing a few
	 * subfunctions). I decided at the time that SQ4/Floppy was not worth the
	 * effort (see above), and to simply disable the delete functionality for
	 * that game - bringing the save/load dialog on a par with SCI0.
	 */
	if (type == K_CONTROL_BUTTON && text && (s->_gameName == "sq4") &&
			s->resMan->sciVersion() < SCI_VERSION_1_1 && !strcmp(text, " Delete ")) {
		PUT_SEL32V(obj, state, (state | kControlStateDisabled) & ~kControlStateEnabled);
	}
}

reg_t kDrawControl(EngineState *s, int, int argc, reg_t *argv) {
	reg_t obj = argv[0];

	_k_disable_delete_for_now(s, obj);
	_k_draw_control(s, obj, 0);
	FULL_REDRAW();
	return NULL_REG;
}

reg_t kHiliteControl(EngineState *s, int, int argc, reg_t *argv) {
	reg_t obj = argv[0];

	_k_draw_control(s, obj, 1);

	return s->r_acc;
}

void update_cursor_limits(int *display_offset, int *cursor, int max_displayed) {
	if (*cursor < *display_offset + 4) {
		if (*cursor < 8)
			*display_offset = 0;
		else
			*display_offset = *cursor - 8;
	} else if (*cursor - *display_offset > max_displayed - 8)
		*display_offset = 12 + *cursor - max_displayed;
}

#define _K_EDIT_DELETE \
	if (cursor < textlen) { \
		memmove(text + cursor, text + cursor + 1, textlen - cursor +1); \
	}

#define _K_EDIT_BACKSPACE \
	if (cursor) { \
		--cursor;    \
		memmove(text + cursor, text + cursor + 1, textlen - cursor +1); \
		--textlen; \
	}

reg_t kEditControl(EngineState *s, int, int argc, reg_t *argv) {
	SegManager *segManager = s->segMan;
	reg_t obj = argv[0];
	reg_t event = argv[1];

	if (obj.segment) {
		uint16 ct_type = GET_SEL32V(obj, type);
		switch (ct_type) {

		case 0:
			break; // NOP

		case K_CONTROL_EDIT:
			if (event.segment && ((GET_SEL32V(event, type)) == SCI_EVT_KEYBOARD)) {
				int max_displayed = GET_SEL32V(obj, max);
				int max = max_displayed;
				int cursor = GET_SEL32V(obj, cursor);
				int modifiers = GET_SEL32V(event, modifiers);
				int key = GET_SEL32V(event, message);
				reg_t text_pos = GET_SEL32(obj, text);
				int display_offset = 0;

				char *text = (char *)s->segMan->dereference(text_pos, NULL);
				int textlen;

				if (!text) {
					warning("Could not draw control: %04x:%04x does not reference text", PRINT_REG(text_pos));
					return s->r_acc;
				}

				textlen = strlen(text);

				cursor += display_offset;

				if (cursor > textlen)
					cursor = textlen;

				if (modifiers & SCI_EVM_CTRL) {

					switch (tolower((char)key)) {
					case 'a':
						cursor = 0;
						break;
					case 'e':
						cursor = textlen;
						break;
					case 'f':
						if (cursor < textlen) ++cursor;
						break;
					case 'b':
						if (cursor > 0) --cursor;
						break;
					case 'k':
						text[cursor] = 0;
						break; // Terminate string
					case 'h':
						_K_EDIT_BACKSPACE;
						break;
					case 'd':
						_K_EDIT_DELETE;
						break;
					}
					PUT_SEL32V(event, claimed, 1);

				} else if (modifiers & SCI_EVM_ALT) { // Ctrl has precedence over Alt
					switch (key) {
					case 0x2100 /* A-f */:
						while ((cursor < textlen) && (text[cursor++] != ' '))
							;
						break;
					case 0x3000 /* A-b */:
						while ((cursor > 0) && (text[--cursor - 1] != ' '))
							;
						break;
					case 0x2000 /* A-d */: {
						while ((cursor < textlen) && (text[cursor] == ' ')) {
							_K_EDIT_DELETE;
							textlen--;
						}
						while ((cursor < textlen) && (text[cursor] != ' ')) {
							_K_EDIT_DELETE;
							textlen--;
						}
						break;
					}
					}
					PUT_SEL32V(event, claimed, 1);
				} else if (key < 31) {
					PUT_SEL32V(event, claimed, 1);
					switch (key) {
					case SCI_K_BACKSPACE:
						_K_EDIT_BACKSPACE;
						break;
					default:
						PUT_SEL32V(event, claimed, 0);
					}
				} else if (key & 0xff00) {
					switch (key) {
					case SCI_K_HOME:
						cursor = 0;
						break;
					case SCI_K_END:
						cursor = textlen;
						break;
					case SCI_K_RIGHT:
						if (cursor + 1 <= textlen)
							++cursor;
						break;
					case SCI_K_LEFT:
						if (cursor > 0)
							--cursor;
						break;
					case SCI_K_DELETE:
						_K_EDIT_DELETE;
						break;
					}
					PUT_SEL32V(event, claimed, 1);
				} else if ((key > 31) && (key < 128)) {
					int inserting = (modifiers & SCI_EVM_INSERT);

					modifiers &= ~(SCI_EVM_RSHIFT | SCI_EVM_LSHIFT | SCI_EVM_CAPSLOCK);

					if (cursor == textlen) {
						if (textlen < max) {
							text[cursor++] = key;
							text[cursor] = 0; // Terminate string
						}
					} else if (inserting) {
						if (textlen < max) {
							int i;

							for (i = textlen + 2; i >= cursor; i--)
								text[i] = text[i - 1];
							text[cursor++] = key;

						}
					} else { // Overwriting
						text[cursor++] = key;
					}

					if (max_displayed < max)
						update_cursor_limits(&display_offset, &cursor, max_displayed);

					cursor -= display_offset;

					PUT_SEL32V(event, claimed, 1);
				}

				PUT_SEL32V(obj, cursor, cursor); // Write back cursor position
			}

		case K_CONTROL_ICON:
		case K_CONTROL_BOX:
		case K_CONTROL_BUTTON:
			if (event.segment) PUT_SEL32V(event, claimed, 1);
			_k_draw_control(s, obj, 0);
			return NULL_REG;
			break;

		case K_CONTROL_TEXT: {
			int state = GET_SEL32V(obj, state);
			PUT_SEL32V(obj, state, state | kControlStateDitherFramed);
			_k_draw_control(s, obj, 0);
			PUT_SEL32V(obj, state, state);
		}
		break;

		default:
			warning("Attempt to edit control type %d", ct_type);
		}
	}

	return s->r_acc;
}

static void _k_draw_control(EngineState *s, reg_t obj, int inverse) {
	SegManager *segManager = s->segMan;
	int x = (int16)GET_SEL32V(obj, nsLeft);
	int y = (int16)GET_SEL32V(obj, nsTop);
	int xl = (int16)GET_SEL32V(obj, nsRight) - x;
	int yl = (int16)GET_SEL32V(obj, nsBottom) - y;
	rect_t area = gfx_rect(x, y, xl, yl);

	int font_nr = GET_SEL32V(obj, font);
	reg_t text_pos = GET_SEL32(obj, text);
	const char *text = text_pos.isNull() ? NULL : (char *)s->segMan->dereference(text_pos, NULL);
	int view = GET_SEL32V(obj, view);
	int cel = sign_extend_byte(GET_SEL32V(obj, cel));
	int loop = sign_extend_byte(GET_SEL32V(obj, loop));
	gfx_alignment_t mode;

	int type = GET_SEL32V(obj, type);
	int state = GET_SEL32V(obj, state);
	int cursor;
	int max;

	switch (type) {
	case K_CONTROL_BUTTON:
		debugC(2, kDebugLevelGraphics, "drawing button %04x:%04x to %d,%d\n", PRINT_REG(obj), x, y);
		ADD_TO_CURRENT_PICTURE_PORT(sciw_new_button_control(s->port, obj, area, s->strSplit(text, NULL).c_str(), font_nr,
		                          (int8)(state & kControlStateFramed), (int8)inverse, (int8)(state & kControlStateDisabled)));
		break;

	case K_CONTROL_TEXT:
		mode = (gfx_alignment_t) GET_SEL32V(obj, mode);

		debugC(2, kDebugLevelGraphics, "drawing text %04x:%04x to %d,%d, mode=%d\n", PRINT_REG(obj), x, y, mode);

		ADD_TO_CURRENT_PICTURE_PORT(sciw_new_text_control(s->port, obj, area, s->strSplit(text).c_str(), font_nr, mode,
									(int8)(!!(state & kControlStateDitherFramed)), (int8)inverse));
		break;

	case K_CONTROL_EDIT:
		debugC(2, kDebugLevelGraphics, "drawing edit control %04x:%04x to %d,%d\n", PRINT_REG(obj), x, y);

		max = GET_SEL32V(obj, max);
		cursor = GET_SEL32V(obj, cursor);

		if (cursor > (signed)strlen(text))
			cursor = strlen(text);

//		update_cursor_limits(&s->save_dir_edit_offset, &cursor, max);	FIXME: get rid of this?
		ADD_TO_CURRENT_PICTURE_PORT(sciw_new_edit_control(s->port, obj, area, text, font_nr, (unsigned)cursor, (int8)inverse));
		break;

	case K_CONTROL_ICON:

		debugC(2, kDebugLevelGraphics, "drawing icon control %04x:%04x to %d,%d\n", PRINT_REG(obj), x, y - 1);

		ADD_TO_CURRENT_PICTURE_PORT(sciw_new_icon_control(s->port, obj, area, view, loop, cel,
		                          (int8)(state & kControlStateFramed), (int8)inverse));
		break;

	case K_CONTROL_CONTROL:
	case K_CONTROL_CONTROL_ALIAS: {
		const char **entries_list = NULL;
		const char *seeker;
		int entries_nr;
		int lsTop = GET_SEL32V(obj, lsTop) - text_pos.offset;
		int list_top = 0;
		int selection = 0;
		int entry_size = GET_SEL32V(obj, x);
		int i;

		debugC(2, kDebugLevelGraphics, "drawing list control %04x:%04x to %d,%d, diff %d\n", PRINT_REG(obj), x, y, SCI_MAX_SAVENAME_LENGTH);
		cursor = GET_SEL32V(obj, cursor) - text_pos.offset;

		entries_nr = 0;
		seeker = text;
		while (seeker[0]) { // Count string entries in NULL terminated string list
			++entries_nr;
			seeker += entry_size;
		}

		if (entries_nr) { // determine list_top, selection, and the entries_list
			seeker = text;
			entries_list = (const char**)malloc(sizeof(char *) * entries_nr);
			for (i = 0; i < entries_nr; i++) {
				entries_list[i] = seeker;
				seeker += entry_size	;
				if ((seeker - text) == lsTop)
					list_top = i + 1;
				if ((seeker - text) == cursor)
					selection = i + 1;
			}
		}

		ADD_TO_CURRENT_PICTURE_PORT(sciw_new_list_control(s->port, obj, area, font_nr, entries_list, entries_nr,
		                          list_top, selection, (int8)inverse));
		if (entries_nr)
			free(entries_list);
	}
	break;

	case K_CONTROL_BOX:
		break;

	default:
		warning("Unknown control type: %d at %04x:%04x, at (%d, %d) size %d x %d",
		         type, PRINT_REG(obj), x, y, xl, yl);
	}

	if (!s->pic_not_valid) {
		FULL_REDRAW();
	}
}


static void draw_rect_to_control_map(EngineState *s, Common::Rect abs_zone) {
	GfxBox *box;
	gfx_color_t color;

	gfxop_set_color(s->gfx_state, &color, -1, -1, -1, -1, -1, 0xf);

	debugC(2, kDebugLevelGraphics, "    adding control block (%d,%d)to(%d,%d)\n", abs_zone.left, abs_zone.top, abs_zone.right, abs_zone.bottom);

	box = gfxw_new_box(s->gfx_state, gfx_rect(abs_zone.left, abs_zone.top, abs_zone.width(),
						abs_zone.height()), color, color, GFX_BOX_SHADE_FLAT);

	assert_primary_widget_lists(s);

	ADD_TO_CURRENT_PICTURE_PORT(box);
}

static void draw_obj_to_control_map(EngineState *s, GfxDynView *view) {
	reg_t obj = make_reg(view->_ID, view->_subID);

	if (!is_object(s->segMan, obj))
		warning("View %d does not contain valid object reference %04x:%04x", view->_ID, PRINT_REG(obj));

	reg_t* sp = view->signalp.getPointer(s->segMan);
	if (!(sp && (sp->offset & _K_VIEW_SIG_FLAG_IGNORE_ACTOR))) {
		Common::Rect abs_zone = get_nsrect(s, make_reg(view->_ID, view->_subID), 1);
		draw_rect_to_control_map(s, abs_zone);
	}
}

static void _k_view_list_do_postdraw(EngineState *s, GfxList *list) {
	SegManager *segManager = s->segMan;
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
			int has_nsrect = lookup_selector(s->segMan, obj, ((SciEngine*)g_engine)->getKernel()->_selectorMap.nsBottom, NULL, NULL) == kSelectorVariable;

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
				fprintf(stderr, "Not lsRecting %04x:%04x because %d\n", PRINT_REG(obj), lookup_selector(s->segMan, obj, ((SciEngine*)g_engine)->getKernel()->_selectorMap.nsBottom, NULL, NULL));
#endif

			if (widget->signal & _K_VIEW_SIG_FLAG_HIDDEN)
				widget->signal |= _K_VIEW_SIG_FLAG_REMOVE;
		}
#ifdef DEBUG_LSRECT
		fprintf(stderr, "obj %04x:%04x has pflags %x\n", PRINT_REG(obj), (widget->signal & (_K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_NO_UPDATE)));
#endif

		reg_t* sp = widget->signalp.getPointer(s->segMan);
		if (sp) {
			*sp = make_reg(0, widget->signal & 0xffff); /* Write back signal */
		}

		widget = (GfxDynView *)widget->_next;
	}
}

void _k_view_list_mark_free(EngineState *s, reg_t off) {
	if (s->dyn_views) {

		GfxDynView *w = (GfxDynView *)s->dyn_views->_contents;

		while (w) {
			if (w->_ID == off.segment
			        && w->_subID == off.offset) {
				w->under_bitsp.obj = NULL_REG;
			}

			w = (GfxDynView *)w->_next;
		}
	}
}

static bool _k_animate_ran = false;	// FIXME: Avoid non-const global vars

int _k_view_list_dispose_loop(EngineState *s, List *list, GfxDynView *widget, int argc, reg_t *argv) {
// disposes all list members flagged for disposal
// returns non-zero IFF views were dropped
	int signal;
	int dropped = 0;
	SegManager *segManager = s->segMan;

	_k_animate_ran = false;

	if (widget) {
		int retval;
		// Recurse:
		retval = _k_view_list_dispose_loop(s, list, (GfxDynView *)widget->_next, argc, argv);

		if (retval == -1) // Bail out on annihilation, rely on re-start from Animate()
			return -1;

		if (GFXW_IS_DYN_VIEW(widget) && (widget->_ID != GFXW_NO_ID)) {
			signal = widget->signalp.getPointer(segManager)->offset;
			if (signal & _K_VIEW_SIG_FLAG_DISPOSE_ME) {
				reg_t obj = make_reg(widget->_ID, widget->_subID);
				reg_t under_bits = NULL_REG;

				if (!is_object(s->segMan, obj)) {
					error("Non-object %04x:%04x present in view list during delete time", PRINT_REG(obj));
					obj = NULL_REG;
				} else {
					reg_t *ubp = widget->under_bitsp.getPointer(segManager);
					if (ubp) { // Is there a bg picture left to clean?
						reg_t mem_handle = *ubp;

						if (mem_handle.segment) {
							if (!kfree(s->segMan, mem_handle)) {
								*ubp = make_reg(0, widget->under_bits = 0);
							} else {
								warning("Treating viewobj %04x:%04x as no longer present", PRINT_REG(obj));
								obj = NULL_REG;
							}
						}
					}
				}
				if (is_object(segManager, obj)) {
					if (invoke_selector(INV_SEL(obj, delete_, kContinueOnInvalidSelector), 0))
						warning("Object at %04x:%04x requested deletion, but does not have a delete funcselector", PRINT_REG(obj));
					if (_k_animate_ran) {
						warning("Object at %04x:%04x invoked kAnimate() during deletion", PRINT_REG(obj));
						return dropped;
					}

					reg_t *ubp = widget->under_bitsp.getPointer(segManager);
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

						draw_obj_to_control_map(s, widget);
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

enum {
	_K_MAKE_VIEW_LIST_CYCLE = 1,
	_K_MAKE_VIEW_LIST_CALC_PRIORITY = 2,
	_K_MAKE_VIEW_LIST_DRAW_TO_CONTROL_MAP = 4
};

static GfxDynView *_k_make_dynview_obj(EngineState *s, reg_t obj, int options, int nr, int argc, reg_t *argv) {
	SegManager *segManager = s->segMan;
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

	if (((SciEngine*)g_engine)->getKernel()->_selectorMap.palette)
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
	if (lookup_selector(s->segMan, obj, ((SciEngine*)g_engine)->getKernel()->_selectorMap.underBits, &(under_bitsp), NULL) != kSelectorVariable) {
		under_bitsp.obj = NULL_REG;
		under_bits = NULL_REG;
		debugC(2, kDebugLevelGraphics, "Object at %04x:%04x has no underBits\n", PRINT_REG(obj));
	} else
		under_bits = *under_bitsp.getPointer(s->segMan);

	ObjVarRef signalp;
	if (lookup_selector(s->segMan, obj, ((SciEngine*)g_engine)->getKernel()->_selectorMap.signal, &(signalp), NULL) != kSelectorVariable) {
		signalp.obj = NULL_REG;
		signal = 0;
		debugC(2, kDebugLevelGraphics, "Object at %04x:%04x has no signal selector\n", PRINT_REG(obj));
	} else {
		signal = signalp.getPointer(s->segMan)->offset;
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

static void _k_make_view_list(EngineState *s, GfxList **widget_list, List *list, int options, int argc, reg_t *argv) {
/* Creates a view_list from a node list in heap space. Returns the list, stores the
** number of list entries in *list_nr. Calls doit for each entry if cycle is set.
** argc, argv should be the same as in the calling kernel function.
*/
	SegManager *segManager = s->segMan;
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

		tempWidget = _k_make_dynview_obj(s, obj, options, sequence_nr--, argc, argv);
		if (tempWidget)
			(*widget_list)->add((GfxContainer *)(*widget_list), tempWidget);

		node = lookup_node(s, next_node); // Next node
	}

	widget = (GfxDynView *)(*widget_list)->_contents;

	while (widget) { // Read back widget values
		reg_t *sp = widget->signalp.getPointer(s->segMan);
		if (sp)
			widget->signal = sp->offset;

		widget = (GfxDynView *)widget->_next;
	}
}

static void _k_prepare_view_list(EngineState *s, GfxList *list, int options) {
	SegManager *segManager = s->segMan;
	GfxDynView *view = (GfxDynView *) list->_contents;
	while (view) {
		reg_t obj = make_reg(view->_ID, view->_subID);
		int priority, _priority;
		int has_nsrect = (view->_ID <= 0) ? 0 : lookup_selector(s->segMan, obj, ((SciEngine*)g_engine)->getKernel()->_selectorMap.nsBottom, NULL, NULL) == kSelectorVariable;
		int oldsignal = view->signal;

		_k_set_now_seen(s, obj);
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
			draw_obj_to_control_map(s, view);

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

static void _k_update_signals_in_view_list(GfxList *old_list, GfxList *new_list) {
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

static void _k_view_list_kryptonize(GfxWidget *v) {
	if (v) {
		v->_flags &= ~GFXW_FLAG_IMMUNE_TO_SNAPSHOTS;
		_k_view_list_kryptonize(v->_next);
	}
}

static void _k_raise_topmost_in_view_list(EngineState *s, GfxList *list, GfxDynView *view) {
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

		_k_raise_topmost_in_view_list(s, list, next);
	}
}

static void _k_redraw_view_list(EngineState *s, GfxList *list) {
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

void _k_draw_view_list(EngineState *s, GfxList *list, int flags) {
	// Draws list_nr members of list to s->pic.
	GfxDynView *widget = (GfxDynView *) list->_contents;

	if ((GfxContainer *)s->port != (GfxContainer *)s->dyn_views->_parent)
		return; // Return if the pictures are meant for a different port

	while (widget) {
		if (flags & _K_DRAW_VIEW_LIST_PICVIEW)
			widget = gfxw_picviewize_dynview(widget);

		if (GFXW_IS_DYN_VIEW(widget) && widget->_ID) {
			uint16 signal = (flags & _K_DRAW_VIEW_LIST_USE_SIGNAL) ? widget->signalp.getPointer(s->segMan)->offset : 0;

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

					*widget->signalp.getPointer(s->segMan) = make_reg(0, signal); // Write the changes back
				};

			} // ...if we're drawing disposeables and this one is disposeable, or if we're drawing non-
			  // disposeables and this one isn't disposeable
		}

		widget = (GfxDynView *)widget->_next;
	} // while (widget)

}

reg_t kAddToPic(EngineState *s, int, int argc, reg_t *argv) {
	GfxList *pic_views;
	reg_t list_ref = argv[0];

	assert_primary_widget_lists(s);

	if (argc > 1) {
		int view, cel, loop, x, y, priority, control;
		GfxWidget *widget;

		view = argv[0].toUint16();
		loop = argv[1].toUint16();
		cel = argv[2].toUint16();
		x = argv[3].toSint16();
		y = argv[4].toSint16() + 1 /* magic + 1 */;
		priority = argv[5].toSint16();
		control = argv[6].toSint16();

		widget = gfxw_new_dyn_view(s->gfx_state, Common::Point(x, y), 0, view, loop, cel, 0,
		                                priority, -1 /* No priority */ , ALIGN_CENTER, ALIGN_BOTTOM, 0);

		if (!widget) {
			error("Attempt to single-add invalid picview (%d/%d/%d)", view, loop, cel);
		} else {
			widget->_ID = -1;
			if (control >= 0) {
				Common::Rect abs_zone = nsrect_clip(s, y, calculate_nsrect(s, x, y, view, loop, cel), priority);
				draw_rect_to_control_map(s, abs_zone);
			}
			ADD_TO_CURRENT_PICTURE_PORT(gfxw_picviewize_dynview((GfxDynView *) widget));
		}
	} else {
		List *list;

		if (!list_ref.segment) {
			warning("Attempt to AddToPic single non-list: %04x:%04x", PRINT_REG(list_ref));
			return s->r_acc;
		}

		list = lookup_list(s, list_ref);

		pic_views = gfxw_new_list(s->picture_port->_bounds, 1);

		debugC(2, kDebugLevelGraphics, "Preparing picview list...\n");
		_k_make_view_list(s, &pic_views, list, 0, argc, argv);
		_k_prepare_view_list(s, pic_views, _K_MAKE_VIEW_LIST_DRAW_TO_CONTROL_MAP);
		// Store pic views for later re-use

		debugC(2, kDebugLevelGraphics, "Drawing picview list...\n");
		ADD_TO_CURRENT_PICTURE_PORT(pic_views);
		_k_draw_view_list(s, pic_views, _K_DRAW_VIEW_LIST_NONDISPOSEABLE | _K_DRAW_VIEW_LIST_DISPOSEABLE | _K_DRAW_VIEW_LIST_PICVIEW);
		// Draw relative to the bottom center
		debugC(2, kDebugLevelGraphics, "Returning.\n");
	}
	reparentize_primary_widget_lists(s, s->port);

	return s->r_acc;
}

reg_t kGetPort(EngineState *s, int, int argc, reg_t *argv) {
	return make_reg(0, s->port->_ID);
}

reg_t kSetPort(EngineState *s, int, int argc, reg_t *argv) {
	if (activated_icon_bar && argc == 6) {
		port_origin_x = port_origin_y = 0;
		activated_icon_bar = false;
		return s->r_acc;
	}

	switch (argc) {
	case 1 : {
		unsigned int port_nr = argv[0].toSint16();
		GfxPort *new_port;

		/* We depart from official semantics here, sorry!
		   Reasoning: Sierra SCI does not clip ports while we do.
		   Therefore a draw to the titlebar port (which is the
		   official semantics) would cut off the lower part of the
		   icons in an SCI1 icon bar. Instead we have an
		   iconbar_port that does not exist in SSCI. */
		if (port_nr == (unsigned int) - 1) port_nr = s->iconbar_port->_ID;

		new_port = s->visual->getPort(port_nr);

		if (!new_port) {
			warning("Invalid port %04x requested", port_nr);
			return NULL_REG;
		}

		s->port->draw(gfxw_point_zero); // Update the port we're leaving
		s->port = new_port;
		return s->r_acc;
	}
	case 6 : {
		port_origin_y = argv[0].toSint16();
		port_origin_x = argv[1].toSint16();

		if (argv[0].toSint16() == -10) {
			s->port->draw(gfxw_point_zero); // Update the port we're leaving
			s->port = s->iconbar_port;
			activated_icon_bar = true;
			return s->r_acc;
		}

		// Notify the graphics resource manager that the pic port bounds changed
		s->gfx_state->gfxResMan->changePortBounds(argv[5].toUint16(), argv[4].toUint16(), argv[3].toUint16() + argv[5].toUint16(), argv[2].toUint16() + argv[4].toUint16());

		// LSL6 calls kSetPort to extend the screen to draw the GUI. If we free all resources
		// here, the background picture is freed too, and this makes everything a big mess.
		// FIXME/TODO: This code really needs to be rewritten to conform to the original behavior
		if (s->_gameName != "lsl6") {
			s->gfx_state->pic_port_bounds = gfx_rect(argv[5].toUint16(), argv[4].toUint16(), argv[3].toUint16(), argv[2].toUint16());

			// FIXME: Should really only invalidate all loaded pic resources here;
			// this is overkill
			s->gfx_state->gfxResMan->freeAllResources();
		} else {
			// WORKAROUND for LSL6
			printf("SetPort case 6 called in LSL6. Origin: %d, %d - Clip rect: %d, %d, %d, %d\n", argv[1].toSint16(), argv[0].toSint16(), argv[5].toUint16(), argv[4].toUint16(), argv[3].toUint16(), argv[2].toUint16());
		}

		break;
	}
	default :
		error("SetPort was called with %d parameters", argc);
		break;
	}

	return NULL_REG;
}

reg_t kDrawCel(EngineState *s, int, int argc, reg_t *argv) {
	int view = argv[0].toSint16();
	int loop = argv[1].toSint16();
	int cel = argv[2].toSint16();
	int x = argv[3].toSint16();
	int y = argv[4].toSint16();
	int priority = (argc > 5) ? argv[5].toSint16() : -1;
	GfxView *new_view;

	gfxop_check_cel(s->gfx_state, view, &loop, &cel);

	debugC(2, kDebugLevelGraphics, "DrawCel((%d,%d), (view.%d, %d, %d), p=%d)\n", x, y, view, loop, cel, priority);

	new_view = gfxw_new_view(s->gfx_state, Common::Point(x, y), view, loop, cel, 0, priority, -1,
	                         ALIGN_LEFT, ALIGN_TOP, GFXW_VIEW_FLAG_DONT_MODIFY_OFFSET);

	ADD_TO_CURRENT_PICTURE_PORT(new_view);
	FULL_REDRAW();

	return s->r_acc;
}

reg_t kDisposeWindow(EngineState *s, int, int argc, reg_t *argv) {
	unsigned int goner_nr = argv[0].toSint16();
	GfxPort *goner;
	GfxPort *pred;

	goner = s->visual->getPort(goner_nr);
	if ((goner_nr < 3) || (goner == NULL)) {
		error("Removal of invalid window %04x requested", goner_nr);
		return s->r_acc;
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

	debugC(2, kDebugLevelGraphics, "Activating port %d after disposing window %d\n", id, goner_nr);
	s->port = (id >= 0) ? s->visual->_portRefs[id] : 0;

	if (!s->port)
		s->port = gfxw_find_default_port(s->visual);

	gfxop_update(s->gfx_state);

	return s->r_acc;
}

reg_t kNewWindow(EngineState *s, int, int argc, reg_t *argv) {
	GfxPort *window;
	int x, y, xl, yl, flags;
	gfx_color_t bgcolor;
	gfx_color_t fgcolor;
	gfx_color_t black;
	gfx_color_t lWhite;
	int priority;
	int argextra = argc == 13 ? 4 : 0; // Triggers in PQ3 and SCI1.1 games

	y = argv[0].toSint16();
	x = argv[1].toSint16();
	yl = argv[2].toSint16() - y;
	xl = argv[3].toSint16() - x;

	y += s->wm_port->_bounds.y;

	if (x + xl > 319)
		x -= ((x + xl) - 319);

	flags = argv[5 + argextra].toSint16();

	priority = (argc > 6 + argextra) ? argv[6 + argextra].toSint16() : -1;
	bgcolor.mask = 0;

	int16 bgColor = (argc > 8 + argextra) ? argv[8 + argextra].toSint16() : 255;

	if (bgColor >= 0) {
		if (!s->resMan->isVGA())
			bgcolor.visual = get_pic_color(s, MIN<int>(bgColor, 15));
		else
			bgcolor.visual = get_pic_color(s, bgColor);
		bgcolor.mask = GFX_MASK_VISUAL;
	} else {
		bgcolor.visual = PaletteEntry(0,0,0);
	}

	bgcolor.priority = priority;
	bgcolor.mask |= priority >= 0 ? GFX_MASK_PRIORITY : 0;
	bgcolor.alpha = 0;
	bgcolor.control = -1;
	debugC(2, kDebugLevelGraphics, "New window with params %d, %d, %d, %d\n", argv[0].toSint16(), argv[1].toSint16(), argv[2].toSint16(), argv[3].toSint16());

	int16 visualColor = (argc > 7 + argextra) ? argv[7 + argextra].toSint16() : 0;
	fgcolor.visual = get_pic_color(s, visualColor);
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
	const char *title = argv[4 + argextra].segment ? kernelDerefString(s->segMan, argv[4 + argextra]) : NULL;

	window = sciw_new_window(s, gfx_rect(x, y, xl, yl), s->titlebar_port->_font, fgcolor, bgcolor,
							s->titlebar_port->_font, lWhite, black, title ? s->strSplit(title, NULL).c_str() : NULL, flags);

	// PQ3 and SCI1.1 games have the interpreter store underBits implicitly
	if (argextra)
		gfxw_port_auto_restore_background(s->visual, window, gfx_rect(argv[5].toSint16(), argv[4].toSint16() + s->wm_port->_bounds.y, argv[7].toSint16() - argv[5].toSint16(), argv[6].toSint16() - argv[4].toSint16()));

	ADD_TO_WINDOW_PORT(window);
	FULL_REDRAW();

	window->draw(gfxw_point_zero);
	gfxop_update(s->gfx_state);

	s->port = window; // Set active port

	return make_reg(0, window->_ID);
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

static void animate_do_animation(EngineState *s, int argc, reg_t *argv) {
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

reg_t kAnimate(EngineState *s, int, int argc, reg_t *argv) {
	// Animations are supposed to take a maximum of animation_delay milliseconds.
	reg_t cast_list_ref = (argc > 0) ? argv[0] : NULL_REG;
	int cycle = (argc > 1) ? argv[1].toUint16() : 0;
	List *cast_list = NULL;
	int open_animation = 0;

	process_sound_events(s); // Take care of incoming events (kAnimate is called semi-regularly)
	_k_animate_ran = true; // Used by some of the invoked functions to check for recursion, which may,
						// after all, damage the cast list

	if (cast_list_ref.segment) {
		cast_list = lookup_list(s, cast_list_ref);
		if (!cast_list)
			return s->r_acc;
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

		_k_make_view_list(s, &(templist), cast_list, (cycle ? _K_MAKE_VIEW_LIST_CYCLE : 0)
		                  | _K_MAKE_VIEW_LIST_CALC_PRIORITY, argc, argv);

		// Make sure that none of the doits() did something evil
		assert_primary_widget_lists(s);

		if (!s->dyn_views->_contents // Only reparentize empty dynview list
		        && (((GfxContainer *)s->port != (GfxContainer *)s->dyn_views->_parent) // If dynviews are on other port...
		            || (s->dyn_views->_next))) // ... or not on top of the view list
			reparentize_primary_widget_lists(s, s->port);
		// End of doit() recovery code

		if (s->pic_is_new) { // Happens if DrawPic() is executed by a dynview (yes, that happens)
			kAnimate(s, WAS_FUNCT_NR, argc, argv); /* Tail-recurse */
			return s->r_acc;
		}

		debugC(2, kDebugLevelGraphics, "Handling Dynviews (..step 9 inclusive):\n");
		_k_prepare_view_list(s, templist, _K_MAKE_VIEW_LIST_CALC_PRIORITY);

		if (s->pic_not_valid) {
			debugC(2, kDebugLevelGraphics, "PicNotValid=%d -> Subalgorithm:\n", s->pic_not_valid);
			_k_redraw_view_list(s, templist);
		}

		_k_update_signals_in_view_list(s->dyn_views, templist);
		s->dyn_views->tag();

		_k_raise_topmost_in_view_list(s, s->dyn_views, (GfxDynView *)templist->_contents);

		delete templist;
		s->dyn_views->free_tagged((GfxContainer *)s->dyn_views); // Free obsolete dynviews
	} // if (cast_list)

	if (open_animation) {
		gfxop_clear_box(s->gfx_state, gfx_rect(0, 10, 320, 190)); // Propagate pic
		s->visual->add_dirty_abs((GfxContainer *)s->visual, gfx_rect_fullscreen, 0);
		// Mark screen as dirty so picviews will be drawn correctly
		FULL_REDRAW();

		animate_do_animation(s, argc, argv);
	} // if (open_animation)

	if (cast_list) {
		int retval;
		int reparentize = 0;

		s->pic_not_valid = 0;

		_k_view_list_do_postdraw(s, s->dyn_views);

		// _k_view_list_dispose_loop() returns -1 if it requested a re-start, so we do just that.
		while ((retval = _k_view_list_dispose_loop(s, cast_list, (GfxDynView *) s->dyn_views->_contents, argc, argv) < 0))
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
	return s->r_acc;
}

#define SHAKE_DOWN 1
#define SHAKE_RIGHT 2

reg_t kShakeScreen(EngineState *s, int, int argc, reg_t *argv) {
	int shakes = (argc > 0) ? argv[0].toSint16() : 1;
	int directions = (argc > 1) ? argv[1].toSint16() : 1;
	gfx_pixmap_t *screen = gfxop_grab_pixmap(s->gfx_state, gfx_rect(0, 0, 320, 200));
	int i;

	if (directions & ~3)
		debugC(2, kDebugLevelGraphics, "ShakeScreen(): Direction bits are %x (unknown)\n", directions);

	gfxop_set_clip_zone(s->gfx_state, gfx_rect_fullscreen);

	for (i = 0; i < shakes; i++) {
		int shake_down = (directions & SHAKE_DOWN) ? 10 : 0;
		int shake_right = (directions & SHAKE_RIGHT) ? 10 : 0;

		if (directions & SHAKE_DOWN)
			gfxop_draw_box(s->gfx_state, gfx_rect(0, 0, 320, 10), s->ega_colors[0], s->ega_colors[0], GFX_BOX_SHADE_FLAT);

		if (directions & SHAKE_RIGHT)
			gfxop_draw_box(s->gfx_state, gfx_rect(0, 0, 10, 200), s->ega_colors[0], s->ega_colors[0], GFX_BOX_SHADE_FLAT);

		gfxop_draw_pixmap(s->gfx_state, screen, gfx_rect(0, 0, 320 - shake_right, 200 - shake_down),
		                  Common::Point(shake_right, shake_down));

		gfxop_update(s->gfx_state);
		gfxop_sleep(s->gfx_state, 50);

		gfxop_draw_pixmap(s->gfx_state, screen, gfx_rect(0, 0, 320, 200), Common::Point(0, 0));
		gfxop_update(s->gfx_state);
		gfxop_sleep(s->gfx_state, 50);
	}

	gfxop_free_pixmap(s->gfx_state, screen);
	gfxop_update(s->gfx_state);
	return s->r_acc;
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

reg_t kDisplay(EngineState *s, int, int argc, reg_t *argv) {
	int argpt;
	reg_t textp = argv[0];
	int index = (argc > 1) ? argv[1].toUint16() : 0;
	int temp;
	bool save_under = false;
	gfx_color_t transparent = { PaletteEntry(), 0, -1, -1, 0 };
	char *text;
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

	if (textp.segment) {
		argpt = 1;
		text = (char *)kernelDerefBulkPtr(s->segMan, textp, 0);
	} else {
		argpt = 2;
		text = kernel_lookup_text(s, textp, index);
	}

	if (!text) {
		error("Display with invalid reference %04x:%04x", PRINT_REG(textp));
		return NULL_REG;
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
			return s->r_acc;

		case K_DONT_UPDATE_IMMEDIATELY:

			update_immediately = false;
			debugC(2, kDebugLevelGraphics, "Display: set_dont_update()\n");
			argpt++;
			break;

		default:
			debugC(2, kDebugLevelGraphics, "Unknown Display() command %x\n", argv[argpt - 1].toUint16());
			return NULL_REG;
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
		return NULL_REG;
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

	return s->r_acc;
}

static reg_t kShowMovie_Windows(EngineState *s, int argc, reg_t *argv) {
	const char *filename = kernelDerefString(s->segMan, argv[1]);
	
	Graphics::AVIPlayer *player = new Graphics::AVIPlayer(g_system);
	
	if (!player->open(filename)) {
		warning("Failed to open movie file %s", filename);
		return s->r_acc;
	}
	
	uint32 startTime = g_system->getMillis();
	bool play = true;
	
	while (play && player->getCurFrame() < player->getFrameCount()) {
		uint32 elapsed = g_system->getMillis() - startTime;
		
		if (elapsed >= player->getCurFrame() * 1000 / player->getFrameRate()) {
			Graphics::Surface *surface = player->getNextFrame();

			Palette *palette = NULL;
			
			if (player->dirtyPalette()) {
				byte *rawPalette = player->getPalette();
				gfx_pixmap_color_t *colors = new gfx_pixmap_color_t[256];

				for (uint16 i = 0; i < 256; i++) {
					colors[i].r = rawPalette[i * 4];
					colors[i].g = rawPalette[i * 4 + 1];
					colors[i].b = rawPalette[i * 4 + 2];
					colors[i].global_index = i;
				}

				palette = new Palette(colors, 256);
				palette->forceInto(s->gfx_state->driver->getMode()->palette);
			}
				
			if (surface) {
				// Allocate a pixmap
				gfx_pixmap_t *pixmap = gfx_new_pixmap(surface->w, surface->h, 0, 0, 0);
				assert(pixmap);
				gfx_pixmap_alloc_index_data(pixmap);
	
				// Copy data from the surface
				memcpy(pixmap->index_data, surface->pixels, surface->w * surface->h);
				pixmap->xoffset = (g_system->getWidth() - surface->w) / 2;
				pixmap->yoffset = (g_system->getHeight() - surface->h) / 2;
				pixmap->palette = palette;
				
				// Copy the frame to the screen
				gfx_xlate_pixmap(pixmap, s->gfx_state->driver->getMode());
				gfxop_draw_pixmap(s->gfx_state, pixmap, gfx_rect(0, 0, 320, 200), Common::Point(pixmap->xoffset, pixmap->yoffset));
				gfxop_update_box(s->gfx_state, gfx_rect(0, 0, 320, 200));
				gfx_free_pixmap(pixmap);
				
				// Surface is freed when the codec in the video is deleted
			}
		}
		
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
				case Common::EVENT_QUIT:
					play = false;
					quit_vm();
					break;
				default:
					break;
			}
		}
		
		g_system->delayMillis(10);
	}

	delete player;

	return s->r_acc;
}

static reg_t kShowMovie_DOS(EngineState *s, int argc, reg_t *argv) {
	const char *filename = kernelDerefString(s->segMan, argv[0]);
	int delay = argv[1].toUint16(); // Time between frames in ticks
	int frameNr = 0;
	SeqDecoder seq;

	if (!seq.loadFile(filename) && !seq.loadFile(Common::String("SEQ/") + filename)) {
		warning("Failed to open movie file %s", filename);
		return s->r_acc;
	}

	bool play = true;
	while (play) {
		uint32 startTime = g_system->getMillis();

		gfx_pixmap_t *pixmap = seq.getFrame(play);

		if (frameNr++ == 0)
			pixmap->palette->forceInto(s->gfx_state->driver->getMode()->palette);

		gfx_xlate_pixmap(pixmap, s->gfx_state->driver->getMode());
		gfxop_draw_pixmap(s->gfx_state, pixmap, gfx_rect(0, 0, 320, 200), Common::Point(pixmap->xoffset, pixmap->yoffset));
		gfxop_update_box(s->gfx_state, gfx_rect(0, 0, 320, 200));
		gfx_free_pixmap(pixmap);

		// Wait before showing the next frame
		while (play && (g_system->getMillis() < startTime + (delay * 1000 / 60))) {
			// FIXME: we should probably make a function that handles quitting in these kinds of situations
			Common::Event curEvent;
			Common::EventManager *eventMan = g_system->getEventManager();

			// Process quit events
			while (eventMan->pollEvent(curEvent)) {
				if (curEvent.type == Common::EVENT_QUIT) {
					play = false;
					quit_vm();
				}
			}

			g_system->delayMillis(10);
		}
	}

	return s->r_acc;
}

reg_t kShowMovie(EngineState *s, int, int argc, reg_t *argv) {
	// KQ6 Windows calls this with one argument. It doesn't seem
	// to have a purpose...
	if (argc == 1)
		return NULL_REG;

	// The Windows and DOS versions use different video format as well
	// as a different argument set.
	if (argv[0].toUint16() == 0)
		return kShowMovie_Windows(s, argc, argv);

	return kShowMovie_DOS(s, argc, argv);
}

reg_t kSetVideoMode(EngineState *s, int, int argc, reg_t *argv) {
	// This call is used for KQ6's intro. It has one parameter, which is
	// 1 when the intro begins, and 0 when it ends. It is suspected that
	// this is actually a flag to enable video planar memory access, as
	// the video decoder in KQ6 is specifically written for the planar
	// memory model. Planar memory mode access was used for VGA "Mode X"
	// (320x240 resolution, although the intro in KQ6 is 320x200).
	// Refer to http://en.wikipedia.org/wiki/Mode_X

	warning("STUB: SetVideoMode %d", argv[0].toUint16());

	return s->r_acc;
}

} // End of namespace Sci
