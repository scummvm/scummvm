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

#include "sci/include/sciresource.h"
#include "sci/include/engine.h"
#include "sci/include/gfx_widgets.h"
#include "sci/engine/sci_graphics.h"
#include "sci/include/sci_widgets.h"

namespace Sci {

#undef DEBUG_LSRECT

// Graph subfunctions
#define K_GRAPH_GET_COLORS_NR 2
#define K_GRAPH_DRAW_LINE 4
#define K_GRAPH_SAVE_BOX 7
#define K_GRAPH_RESTORE_BOX 8
#define K_GRAPH_FILL_BOX_BACKGROUND 9
#define K_GRAPH_FILL_BOX_FOREGROUND 10
#define K_GRAPH_FILL_BOX_ANY 11
#define K_GRAPH_UPDATE_BOX 12
#define K_GRAPH_REDRAW_BOX 13
#define K_GRAPH_ADJUST_PRIORITY 14

// Control types and flags
#define K_CONTROL_BUTTON 1
#define K_CONTROL_TEXT 2
#define K_CONTROL_EDIT 3
#define K_CONTROL_ICON 4
#define K_CONTROL_CONTROL 6
#define K_CONTROL_CONTROL_ALIAS 7
#define K_CONTROL_BOX 10


#define ADD_TO_CURRENT_PORT(widget) \
  {if (s->port)				   \
       s->port->add(GFXWC(s->port), GFXW(widget)); \
  else \
       s->picture_port->add(GFXWC(s->visual), GFXW(widget));}

#define ADD_TO_CURRENT_PICTURE_PORT(widget) \
  {if (s->port)				   \
       s->port->add(GFXWC(s->port), GFXW(widget)); \
  else \
       s->picture_port->add(GFXWC(s->picture_port), GFXW(widget));}

#define ADD_TO_WINDOW_PORT(widget) \
       s->wm_port->add(GFXWC(s->wm_port), GFXW(widget));

#define ADD_TO_CURRENT_FG_WIDGETS(widget) \
  ADD_TO_CURRENT_PICTURE_PORT(widget)

#define ADD_TO_CURRENT_BG_WIDGETS(widget) \
  ADD_TO_CURRENT_PICTURE_PORT(widget)

#define FULL_REDRAW()\
  if (s->visual) \
       s->visual->draw(GFXW(s->visual), gfxw_point_zero); \
  gfxop_update(s->gfx_state);

#define FULL_INSPECTION()\
  if (s->visual) \
       s->visual->print(GFXW(s->visual), 0);


#define GFX_ASSERT(x) { \
	int val = !!(x); \
	if (val) { \
		if (val == GFX_ERROR) \
			warning("GFX subsystem returned error on \"" #x "\""); \
		else {\
			error("GFX subsystem fatal error condition on \"" #x "\""); \
			vm_handle_fatal_error(s, __LINE__, __FILE__); \
		} \
	}\
}

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
		rect_t bounds = s->picture_port->bounds;

		s->dyn_views = gfxw_new_list(bounds, GFXW_LIST_SORTED);
		s->dyn_views->flags |= GFXW_FLAG_IMMUNE_TO_SNAPSHOTS;
		ADD_TO_CURRENT_PICTURE_PORT(s->dyn_views);
	}

	if (!s->drop_views) {
		rect_t bounds = s->picture_port->bounds;

		s->drop_views = gfxw_new_list(bounds, GFXW_LIST_SORTED);
		s->drop_views->flags |= GFXW_FLAG_IMMUNE_TO_SNAPSHOTS;
		ADD_TO_CURRENT_PICTURE_PORT(s->drop_views);
	}
}

static void reparentize_primary_widget_lists(EngineState *s, gfxw_port_t *newport) {
	if (!newport)
		newport = s->picture_port;

	if (s->dyn_views) {
		gfxw_remove_widget_from_container(s->dyn_views->parent, GFXW(s->dyn_views));

		newport->add(GFXWC(newport), GFXW(s->dyn_views));
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
		if (s->version >= SCI_VERSION_FTU_PRIORITY_14_ZONES)
			return SCI0_VIEW_PRIORITY_14_ZONES(y);
		else
			return SCI0_VIEW_PRIORITY(y) == 15 ? 14 : SCI0_VIEW_PRIORITY(y);
	}
}

int _find_priority_band(EngineState *s, int nr) {
	if (s->version >= SCI_VERSION_FTU_PRIORITY_14_ZONES && (nr < 0 || nr > 14)) {
		if (nr == 15)
			return 0xffff;
		else {
			warning("Attempt to get priority band %d", nr);
		}
		return 0;
	}

	if (s->version < SCI_VERSION_FTU_PRIORITY_14_ZONES && (nr < 0 || nr > 15)) {
		warning("Attempt to get priority band %d", nr);
		return 0;
	}

	if (s->pic_priority_table) // SCI01 priority table set?
		return s->pic_priority_table[nr];
	else {
		int retval;

		if (s->version >= SCI_VERSION_FTU_PRIORITY_14_ZONES)
			retval = SCI0_PRIORITY_BAND_FIRST_14_ZONES(nr);
		else
			retval = SCI0_PRIORITY_BAND_FIRST(nr);

/*		if (s->version <= SCI_VERSION_LTU_PRIORITY_OB1)
			--retval; */
		return retval;
	}
}

reg_t graph_save_box(EngineState *s, rect_t area) {
	reg_t handle = kalloc(s, "graph_save_box()", sizeof(gfxw_snapshot_t *));
	gfxw_snapshot_t **ptr = (gfxw_snapshot_t **) kmem(s, handle);

	*ptr = gfxw_make_snapshot(s->visual, area);

	return handle;
}

void graph_restore_box(EngineState *s, reg_t handle) {
	gfxw_snapshot_t **ptr;
	int port_nr = s->port->ID;

	if (!handle.segment) {
		warning("Attempt to restore box with zero handle");
		return;
	}

	ptr = (gfxw_snapshot_t **)kmem(s, handle);

	if (!ptr) {
		warning("Attempt to restore invalid handle "PREG, PRINT_REG(handle));
		return;
	}

	while (port_nr > 2 && !(s->port->flags & GFXW_FLAG_IMMUNE_TO_SNAPSHOTS) && (gfxw_widget_matches_snapshot(*ptr, GFXW(s->port)))) {
		// This shouldn't ever happen, actually, since windows (ports w/ ID > 2) should all be immune
		gfxw_port_t *newport = gfxw_find_port(s->visual, port_nr);
		error("Port %d is not immune against snapshots", s->port->ID);
		port_nr--;
		if (newport)
			s->port = newport;
	}

	if (s->dyn_views && gfxw_widget_matches_snapshot(*ptr, GFXW(s->dyn_views->parent))) {
		gfxw_container_t *parent = s->dyn_views->parent;

		do {
			parent = parent->parent;
		} while (parent && (gfxw_widget_matches_snapshot(*ptr, GFXW(parent))));

		if (!parent) {
			error("Attempted widget mass destruction by a snapshot");
			BREAKPOINT();
		}

		reparentize_primary_widget_lists(s, (gfxw_port_t *) parent);
	}


	if (!ptr) {
		error("Attempt to restore invalid snaphot with handle "PREG, PRINT_REG(handle));
		return;
	}

	gfxw_restore_snapshot(s->visual, *ptr);
	free(*ptr);
	*ptr = NULL;

	kfree(s, handle);
}

#if 0
#define KERNEL_COLOR_PALETTE s->gfx_state->pic->visual_map->colors
#define KERNEL_COLORS_NR s->gfx_state->pic->visual_map->colors_nr
#else
#define KERNEL_COLOR_PALETTE s->gfx_state->resstate->static_palette
#define KERNEL_COLORS_NR s->gfx_state->resstate->static_palette_entries
#endif

static gfx_pixmap_color_t white = {GFX_COLOR_INDEX_UNMAPPED, 255, 255, 255};

gfx_pixmap_color_t *get_pic_color(EngineState *s, int color) {
	if (s->resmgr->sci_version < SCI_VERSION_01_VGA)
		return &(s->ega_colors[color].visual);

	if (color == 255)
		return &white;
	else if (color < KERNEL_COLORS_NR)
		return &(KERNEL_COLOR_PALETTE[color]);
	else {
		error("Color index %d out of bounds for pic %d (%d max)", color, s->gfx_state->pic_nr, KERNEL_COLORS_NR);
		BREAKPOINT();
		return NULL;
	}
}

static gfx_color_t graph_map_color(EngineState *s, int color, int priority, int control) {
	gfx_color_t retval;

	if (s->resmgr->sci_version < SCI_VERSION_01_VGA) {
		retval = s->ega_colors[(color >=0 && color < 16)? color : 0];
		gfxop_set_color(s->gfx_state, &retval, (color < 0) ? -1 : retval.visual.r, retval.visual.g, retval.visual.b,
		                (color == -1) ? 255 : 0, priority, control);
	} else {
		retval.visual = *(get_pic_color(s, color));
		retval.alpha = 0;
		retval.priority = priority;
		retval.control = control;
		retval.mask = GFX_MASK_VISUAL | ((priority >= 0) ? GFX_MASK_PRIORITY : 0) | ((control >= 0) ? GFX_MASK_CONTROL : 0);
	};

	return retval;
}

reg_t kSetCursor_SCI11(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	switch (argc) {
	case 1 :
		if (UKPV(0) == 0) {
			s->save_mouse_pointer_view = s->mouse_pointer_view;
			s->save_mouse_pointer_loop = s->mouse_pointer_loop;
			s->save_mouse_pointer_cel = s->mouse_pointer_cel;
			s->mouse_pointer_view = s->mouse_pointer_loop = s->mouse_pointer_cel = -1;
			gfxop_set_pointer_cursor(s->gfx_state, GFXOP_NO_POINTER);
		} else {
			s->mouse_pointer_view = s->save_mouse_pointer_view;
			s->mouse_pointer_loop = s->save_mouse_pointer_loop;
			s->mouse_pointer_cel = s->save_mouse_pointer_cel;
		}
	case 2 : {
		Common::Point pt;
		pt.x = UKPV(0);
		pt.y = UKPV(1);

		GFX_ASSERT(gfxop_set_pointer_position(s->gfx_state, pt));
		break;
	}
	case 3 :
		GFX_ASSERT(gfxop_set_pointer_view(s->gfx_state, UKPV(0), UKPV(1), UKPV(2), NULL));
		s->mouse_pointer_view = UKPV(0);
		s->mouse_pointer_loop = UKPV(1);
		s->mouse_pointer_cel = UKPV(2);
		break;
	case 9 : {
		Common::Point hotspot = Common::Point(SKPV(3), SKPV(4));

//		sciprintf("Setting hotspot at %d/%d\n", hotspot.x, hotspot.y);

		gfxop_set_pointer_view(s->gfx_state, UKPV(0), UKPV(1), UKPV(2), &hotspot);
		break;
	}
	default :
		error("kSetCursor: Unhandled case: %d arguments given", argc);
		break;
	}
	return s->r_acc;
}

reg_t kSetCursor(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	if (s->version >= SCI_VERSION(1, 001, 000) ||
	        has_kernel_function(s, "MoveCursor")) {
		return kSetCursor_SCI11(s, funct_nr, argc, argv);
	}

	if (SKPV_OR_ALT(1, 1)) {
		s->mouse_pointer_view = SKPV(0);
	} else
		s->mouse_pointer_view = GFXOP_NO_POINTER;

	s->mouse_pointer_loop = s->mouse_pointer_cel = 0; // Not used with cursor-format pointers

	GFX_ASSERT(gfxop_set_pointer_cursor(s->gfx_state, s->mouse_pointer_view));

	if (argc > 2) {
		Common::Point newpos = Common::Point(SKPV(2) + s->port->bounds.x, SKPV(3) + s->port->bounds.y);
		GFX_ASSERT(gfxop_set_pointer_position(s->gfx_state, newpos));
	}

	return s->r_acc;
}

extern int oldx, oldy;

reg_t kMoveCursor(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	Common::Point newpos;
	static Common::Point oldpos(0, 0);

	newpos = s->gfx_state->pointer_pos;

	if (argc == 1) {
		// Case ignored on IBM PC
	} else {
		newpos.x = SKPV(0) + s->port->zone.x;
		newpos.y = SKPV(1) + s->port->zone.y;

		if (newpos.x > s->port->zone.x + s->port->zone.xl)
			newpos.x = s->port->zone.x + s->port->zone.xl;
		if (newpos.y > s->port->zone.y + s->port->zone.yl)
			newpos.y = s->port->zone.y + s->port->zone.yl;

		if (newpos.x < 0) newpos.x = 0;
		if (newpos.y < 0) newpos.y = 0;

		oldpos = newpos;
	}

	GFX_ASSERT(gfxop_set_pointer_position(s->gfx_state, newpos));

	return s->r_acc;
}

static inline void _ascertain_port_contents(gfxw_port_t *port) {
	if (!port->contents)
		port->contents = (gfxw_widget_t *) gfxw_new_list(port->bounds, 0);
}

reg_t kShow(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int old_map = s->pic_visible_map;

	s->pic_visible_map = (gfx_map_mask_t) UKPV_OR_ALT(0, 1);

	switch (s->pic_visible_map) {

	case GFX_MASK_VISUAL:
	case GFX_MASK_PRIORITY:
	case GFX_MASK_CONTROL:
		gfxop_set_visible_map(s->gfx_state, s->pic_visible_map);
		if (old_map != s->pic_visible_map) {

			if (s->pic_visible_map == GFX_MASK_VISUAL) // Full widget redraw
				s->visual->draw(GFXW(s->visual), Common::Point(0, 0));

			gfxop_update(s->gfx_state);
			sciprintf("Switching visible map to %x\n", s->pic_visible_map);
		}
		break;

	default:
		warning("Show(%x) selects unknown map", s->pic_visible_map);

	}

	s->pic_not_valid = 2;
	return s->r_acc;
}

reg_t kPicNotValid(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	s->r_acc = make_reg(0, s->pic_not_valid);
	if (argc)
		s->pic_not_valid = (byte)UKPV(0);

	return s->r_acc;
}

void _k_redraw_box(EngineState *s, int x1, int y1, int x2, int y2) {
	sciprintf("_k_redraw_box(): Unimplemented");
#if 0
	int i;
	view_object_t *list = s->dyn_views;

	sciprintf("Reanimating views\n", s->dyn_views_nr);


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
	gfxw_port_t *port = s->port;
	gfxw_port_t *newport;

	newport = sciw_new_window(s, port->zone, port->font_nr, port->color, newbgcolor,
	                          s->titlebar_port->font_nr, s->ega_colors[15], s->ega_colors[8],
	                          port->title_text, port->port_flags & ~WINDOW_FLAG_TRANSPARENT);

	if (s->dyn_views) {
		int found = 0;
		gfxw_container_t *parent = s->dyn_views->parent;

		while (parent && !(found |= (GFXW(parent) == GFXW(port))))
			parent = parent->parent;

		s->dyn_views = NULL;
	}

	port->parent->add(GFXWC(port->parent), GFXW(newport));
	port->widfree(GFXW(port));
}

static int activated_icon_bar;
static int port_origin_x;
static int port_origin_y;

reg_t kGraph(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	rect_t area;
	gfxw_port_t *port = s->port;
	int redraw_port = 0;

	area = gfx_rect(SKPV(2), SKPV(1) , SKPV(4), SKPV(3));

	area.xl = area.xl - area.x; // Since the actual coordinates are absolute
	area.yl = area.yl - area.y;

	switch (SKPV(0)) {

	case K_GRAPH_GET_COLORS_NR:

		return make_reg(0, (s->resmgr->sci_version < SCI_VERSION_01_VGA) ? 0x10 : 0x100);
		break;

	case K_GRAPH_DRAW_LINE: {

		gfx_color_t gfxcolor = graph_map_color(s, SKPV(5) & 0xf, SKPV_OR_ALT(6, -1), SKPV_OR_ALT(7, -1));

		SCIkdebug(SCIkGRAPHICS, "draw_line((%d, %d), (%d, %d), col=%d, p=%d, c=%d, mask=%d)\n",
		          SKPV(2), SKPV(1), SKPV(4), SKPV(3), SKPV(5), SKPV_OR_ALT(6, -1), SKPV_OR_ALT(7, -1), gfxcolor.mask);

		redraw_port = 1;
		ADD_TO_CURRENT_BG_WIDGETS(GFXW(gfxw_new_line(Common::Point(SKPV(2), SKPV(1)), Common::Point(SKPV(4), SKPV(3)),
		                               gfxcolor, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL)));

	}
	break;

	case K_GRAPH_SAVE_BOX:

		area.x += s->port->zone.x + port_origin_x;
		area.y += s->port->zone.y + port_origin_y;
		area.xl += -port_origin_x;
		area.yl += -port_origin_y;

		return(graph_save_box(s, area));
		break;

	case K_GRAPH_RESTORE_BOX:

		graph_restore_box(s, argv[1]);
		break;

	case K_GRAPH_FILL_BOX_BACKGROUND:

		_k_graph_rebuild_port_with_color(s, port->bgcolor);
		port = s->port;

		redraw_port = 1;
		break;

	case K_GRAPH_FILL_BOX_FOREGROUND:

		_k_graph_rebuild_port_with_color(s, port->color);
		port = s->port;

		redraw_port = 1;
		break;

	case K_GRAPH_FILL_BOX_ANY: {

		gfx_color_t color = graph_map_color(s, SKPV(6), SKPV_OR_ALT(7, -1), SKPV_OR_ALT(8, -1));

		color.mask = (byte)UKPV(5);

		SCIkdebug(SCIkGRAPHICS, "fill_box_any((%d, %d), (%d, %d), col=%d, p=%d, c=%d, mask=%d)\n",
		          SKPV(2), SKPV(1), SKPV(4), SKPV(3), SKPV(6), SKPV_OR_ALT(7, -1), SKPV_OR_ALT(8, -1), UKPV(5));

		ADD_TO_CURRENT_BG_WIDGETS(gfxw_new_box(s->gfx_state, area, color, color, GFX_BOX_SHADE_FLAT));

	}
	break;

	case K_GRAPH_UPDATE_BOX: {

		SCIkdebug(SCIkGRAPHICS, "update_box(%d, %d, %d, %d)\n", SKPV(1), SKPV(2), SKPV(3), SKPV(4));

		area.x += s->port->zone.x;
		area.y += s->port->zone.y;

		gfxop_update_box(s->gfx_state, area);

	}
	break;

	case K_GRAPH_REDRAW_BOX: {


		SCIkdebug(SCIkGRAPHICS, "redraw_box(%d, %d, %d, %d)\n", SKPV(1), SKPV(2), SKPV(3), SKPV(4));

		area.x += s->port->zone.x;
		area.y += s->port->zone.y;

		if (s->dyn_views && s->dyn_views->parent == GFXWC(s->port))
			s->dyn_views->draw(GFXW(s->dyn_views), Common::Point(0, 0));

		gfxop_update_box(s->gfx_state, area);

	}

	break;

	case K_GRAPH_ADJUST_PRIORITY:

		SCIkdebug(SCIkGRAPHICS, "adjust_priority(%d, %d)\n", SKPV(1), SKPV(2));
		s->priority_first = SKPV(1) - 10;
		s->priority_last = SKPV(2) - 10;
		break;

	default:

		warning("Unhandled Graph() operation %04x", SKPV(0));

	}

	if (redraw_port)
		FULL_REDRAW();

	gfxop_update(s->gfx_state);

	return s->r_acc;
}

reg_t kTextSize(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int width, height;
	char *text = argv[1].segment ? (char *) kernel_dereference_bulk_pointer(s, argv[1], 0) : NULL;
	reg_t *dest = kernel_dereference_reg_pointer(s, argv[0], 4);
	int maxwidth = KP_UINT(KP_ALT(3,  NULL_REG));
	int font_nr = KP_UINT(argv[2]);

	if (maxwidth < 0)
		maxwidth = 0;

	dest[0] = dest[1] = NULL_REG;

	if (!text || !*text || !dest) { // Empty text
		dest[2] = dest[3] = make_reg(0, 0);
		SCIkdebug(SCIkSTRINGS, "GetTextSize: Empty string\n");
		return s->r_acc;
	}

	GFX_ASSERT(gfxop_get_text_params(s->gfx_state, font_nr, text, maxwidth ? maxwidth : MAX_TEXT_WIDTH_MAGIC_VALUE,
	                                 &width, &height, 0, NULL, NULL, NULL));
	SCIkdebug(SCIkSTRINGS, "GetTextSize '%s' -> %dx%d\n", text, width, height);

	dest[2] = make_reg(0, height);
//	dest[3] = make_reg(0, maxwidth? maxwidth : width);
	dest[3] = make_reg(0, width);

	return s->r_acc;
}

int debug_sleeptime_factor = 1;

reg_t kWait(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	GTimeVal time;
	int sleep_time = UKPV(0);

	sci_get_current_time(&time);

	s->r_acc = make_reg(0, ((time.tv_usec - s->last_wait_time.tv_usec) * 60 / 1000000) + (time.tv_sec - s->last_wait_time.tv_sec) * 60);

	memcpy(&(s->last_wait_time), &time, sizeof(GTimeVal));

	// Reset optimization flags: Game is playing along nicely anyway
	s->kernel_opt_flags &= ~(KERNEL_OPT_FLAG_GOT_EVENT | KERNEL_OPT_FLAG_GOT_2NDEVENT);

	sleep_time *= debug_sleeptime_factor;
	GFX_ASSERT(gfxop_usleep(s->gfx_state, sleep_time * 1000000 / 60));

	return s->r_acc;
}

reg_t kCoordPri(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int y = SKPV(0);

	return make_reg(0, VIEW_PRIORITY(y));
}

reg_t kPriCoord(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int priority = SKPV(0);

	return make_reg(0, PRIORITY_BAND_FIRST(priority));
}

void _k_dirloop(reg_t obj, word angle, EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int view = GET_SEL32V(obj, view);
	int signal = GET_SEL32V(obj, signal);
	int loop;
	int maxloops;

	if (signal & _K_VIEW_SIG_FLAG_DOESNT_TURN)
		return;

	angle %= 360;

	if (s->version >= SCI_VERSION_FTU_2ND_ANGLES) {
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

	if (maxloops == GFX_ERROR) {
		error("Invalid view.%03d", view);
		return;
	} else if ((loop > 1) && (maxloops < 4))
		return;

	PUT_SEL32V(obj, loop, loop);
}

reg_t kDirLoop(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	_k_dirloop(argv[0], UKPV(1), s, funct_nr, argc, argv);

	return s->r_acc;
}

#define GASEOUS_VIEW_MASK_ACTIVE (_K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_IGNORE_ACTOR)
#define GASEOUS_VIEW_MASK_PASSIVE (_K_VIEW_SIG_FLAG_NO_UPDATE | _K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_IGNORE_ACTOR)

abs_rect_t set_base(EngineState *s, reg_t object);

inline abs_rect_t get_nsrect(EngineState *s, reg_t object, byte clip);

static inline abs_rect_t nsrect_clip(EngineState *s, int y, abs_rect_t retval, int priority);

static int collides_with(EngineState *s, abs_rect_t area, reg_t other_obj, int use_nsrect, int view_mask, int funct_nr, int argc, reg_t *argv) {
	int other_signal = GET_SEL32V(other_obj, signal);
	int other_priority = GET_SEL32V(other_obj, priority);
	int y = GET_SEL32SV(other_obj, y);
	abs_rect_t other_area;

	if (use_nsrect) {
		other_area = get_nsrect(s, other_obj, 0);
		other_area = nsrect_clip(s, y, other_area, other_priority);
	} else {
		other_area.x = GET_SEL32V(other_obj, brLeft);
		other_area.xend = GET_SEL32V(other_obj, brRight);
		other_area.y = GET_SEL32V(other_obj, brTop);
		other_area.yend = GET_SEL32V(other_obj, brBottom);
	}

	if (other_area.xend < 0 || other_area.yend < 0 || area.xend < 0 || area.yend < 0)
		return 0; // Out of scope

	if (other_area.x >= 320 || other_area.y >= 190 || area.xend >= 320 || area.yend >= 190)
		return 0; // Out of scope

	SCIkdebug(SCIkBRESEN, "OtherSignal=%04x, z=%04x obj="PREG"\n", other_signal, (other_signal & view_mask), PRINT_REG(other_obj));

	if ((other_signal & (view_mask)) == 0) {
		// check whether the other object ignores actors

		SCIkdebug(SCIkBRESEN, "  against (%d,%d) to (%d,%d)\n", other_area.x, other_area.y, other_area.xend, other_area.yend);

		if (((other_area.xend > area.x) && (other_area.x < area.xend)) // [other_x, other_xend] intersects [x, xend])?
		        && ((other_area.yend > area.y) && (other_area.y < area.yend))) // [other_y, other_yend] intersects [y, yend]?
			return 1;
		/* CR (from :Bob Heitman:) Collision rects have Mac semantics, ((0,0),(1,1)) only
		** covers the coordinate (0,0) */
	}

	SCIkdebug(SCIkBRESEN, " (no)\n");
	return 0;
}

reg_t kCanBeHere(EngineState *s, int funct_nr, int argc, reg_t * argv) {
	reg_t obj = argv[0];
	reg_t cliplist_ref = KP_ALT(1, NULL_REG);
	list_t *cliplist = NULL;
	gfxw_port_t *port = s->picture_port;
	word signal;
	int retval;

	abs_rect_t abs_zone;
	rect_t zone;
	word edgehit;
	word illegal_bits;

	abs_zone.x = GET_SEL32SV(obj, brLeft);
	abs_zone.xend = GET_SEL32SV(obj, brRight);
	abs_zone.y = GET_SEL32SV(obj, brTop);
	abs_zone.yend = GET_SEL32SV(obj, brBottom);

	zone = gfx_rect(abs_zone.x + port->zone.x, abs_zone.y + port->zone.y, abs_zone.xend - abs_zone.x, abs_zone.yend - abs_zone.y);

	signal = GET_SEL32V(obj, signal);
	SCIkdebug(SCIkBRESEN, "Checking collision: (%d,%d) to (%d,%d) ([%d..%d]x[%d..%d]), obj="PREG", sig=%04x, cliplist="PREG"\n",
	          GFX_PRINT_RECT(zone), abs_zone.x, abs_zone.xend, abs_zone.y, abs_zone.yend,
	          PRINT_REG(obj), signal, PRINT_REG(cliplist_ref));

	illegal_bits = GET_SEL32V(obj, illegalBits);

	retval = !(illegal_bits & (edgehit = gfxop_scan_bitmask(s->gfx_state, zone, GFX_MASK_CONTROL)));

	SCIkdebug(SCIkBRESEN, "edgehit = %04x (illegalBits %04x)\n", edgehit, illegal_bits);
	if (retval == 0) {
		SCIkdebug(SCIkBRESEN, " -> %04x\n", retval);
		return not_register(s, NULL_REG); // Can't BeHere
	}

	retval = 0;

	if ((illegal_bits & 0x8000) // If we are vulnerable to those views at all...
	        && s->dyn_views) { // ...check against all stop-updated dynviews
		gfxw_dyn_view_t *widget = (gfxw_dyn_view_t *) s->dyn_views->contents;

		SCIkdebug(SCIkBRESEN, "Checking vs dynviews:\n");

		while (widget) {
			if (widget->ID && (widget->signal & _K_VIEW_SIG_FLAG_FREESCI_STOPUPD)
			        && ((widget->ID != obj.segment) || (widget->subID != obj.offset))
			        && is_object(s, make_reg(widget->ID, widget->subID)))
				if (collides_with(s, abs_zone, make_reg(widget->ID, widget->subID), 1, GASEOUS_VIEW_MASK_ACTIVE, funct_nr, argc, argv))
					return not_register(s, NULL_REG);

			widget = (gfxw_dyn_view_t *) widget->next;
		}
	}

	if (signal & GASEOUS_VIEW_MASK_ACTIVE) {
		retval = signal & GASEOUS_VIEW_MASK_ACTIVE; // CanBeHere- it's either being disposed, or it ignores actors anyway
		SCIkdebug(SCIkBRESEN, " -> %04x\n", retval);
		return not_register(s, make_reg(0, retval)); // CanBeHere
	}

	if (cliplist_ref.segment)
		cliplist = LOOKUP_LIST(cliplist_ref);

	if (cliplist) {
		node_t *node = LOOKUP_NODE(cliplist->first);

		retval = 0; // Assume that we Can'tBeHere...

		while (node) { // Check each object in the list against our bounding rectangle
			reg_t other_obj = node->value;
			SCIkdebug(SCIkBRESEN, "  comparing against "PREG"\n", PRINT_REG(other_obj));

			if (!is_object(s, other_obj)) {
				warning("CanBeHere() cliplist contains non-object "PREG, PRINT_REG(other_obj));
			} else if (!REG_EQ(other_obj, obj)) { // Clipping against yourself is not recommended

				if (collides_with(s, abs_zone, other_obj, 0, GASEOUS_VIEW_MASK_PASSIVE, funct_nr, argc, argv)) {
					SCIkdebug(SCIkBRESEN, " -> %04x\n", retval);
					return not_register(s, NULL_REG);
				}

			} // if (other_obj != obj)
			node = LOOKUP_NODE(node->succ); // move on
		}
	}

	if (!retval)
		retval = 1;
	SCIkdebug(SCIkBRESEN, " -> %04x\n", retval);

	return not_register(s, make_reg(0, retval));
}  // CanBeHere

reg_t kIsItSkip(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int view = SKPV(0);
	int loop = SKPV(1);
	int cel = SKPV(2);
	int x = UKPV(3);
	int y = UKPV(4);
	gfxr_view_t *res = NULL;
	gfx_pixmap_t *pxm = NULL;

	if (!(res = gfxr_get_view(s->gfx_state->resstate, view, &loop, &cel, 0))) {
		GFXWARN("Attempt to get cel parameters for invalid view %d\n", view);
		return make_reg(0, -1);
	}

	pxm = res->loops[loop].cels[cel];
	if (x > pxm->index_xl)
		x = pxm->index_xl - 1;
	if (y > pxm->index_yl)
		y = pxm->index_yl - 1;

	return make_reg(0, pxm->index_data[y * pxm->index_xl + x] == pxm->color_key);
}

reg_t kCelHigh(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int view = SKPV(0);
	int loop = SKPV(1);
	int cel = SKPV(2);
	int height, width;
	Common::Point offset;

	if (argc != 3) {
		warning("CelHigh called with %d parameters", argc);
	}

	if (gfxop_get_cel_parameters(s->gfx_state, view, loop, cel, &width, &height, &offset)) {
		error("Invalid loop (%d) or cel (%d) in view.%d (0x%x), or view invalid", loop, cel, view, view);
		return NULL_REG;
	} else
		return make_reg(0, height);
}

reg_t kCelWide(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int view = SKPV(0);
	int loop = SKPV(1);
	int cel = SKPV(2);
	int height, width;
	Common::Point offset;

	if (argc != 3) {
		warning("CelHigh called with %d parameters", argc);
	}

	if (gfxop_get_cel_parameters(s->gfx_state, view, loop, cel, &width, &height, &offset)) {
		error("Invalid loop (%d) or cel (%d) in view.%d (0x%x), or view invalid", loop, cel, view, view);
		return NULL_REG;
	} else
		return make_reg(0, width);
}

reg_t kNumLoops(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t obj = argv[0];
	int view = GET_SEL32V(obj, view);
	int loops_nr = gfxop_lookup_view_get_loops(s->gfx_state, view);

	if (loops_nr < 0) {
		error("view.%d (0x%x) not found", view, view);
		return NULL_REG;
	}

	SCIkdebug(SCIkGRAPHICS, "NumLoops(view.%d) = %d\n", view, loops_nr);

	return make_reg(0, loops_nr);
}

reg_t kNumCels(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t obj = argv[0];
	int loop = GET_SEL32V(obj, loop);
	int view = GET_SEL32V(obj, view);
	int cel = 0xffff;


	if (gfxop_check_cel(s->gfx_state, view, &loop, &cel)) { 
		// OK, this is a hack and there's a
		// real function to calculate cel numbers...
		error("view.%d (0x%x) not found", view, view);
		return NULL_REG;
	}

	SCIkdebug(SCIkGRAPHICS, "NumCels(view.%d, %d) = %d\n", view, loop, cel + 1);

	return make_reg(0, cel + 1);
}

reg_t kOnControl(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int arg = 0;
	gfx_map_mask_t map;
	int xstart, ystart;
	int xlen = 1, ylen = 1;

	if (argc == 2 || argc == 4)
		map = GFX_MASK_CONTROL;
	else {
		arg = 1;
		map = (gfx_map_mask_t) SKPV(0);
	}

	ystart = SKPV(arg + 1);
	xstart = SKPV(arg);

	if (argc > 3) {
		ylen = SKPV(arg + 3) - ystart;
		xlen = SKPV(arg + 2) - xstart;
	}

	return make_reg(0, gfxop_scan_bitmask(s->gfx_state, gfx_rect(xstart, ystart + 10, xlen, ylen), map));
}

void _k_view_list_free_backgrounds(EngineState *s, view_object_t *list, int list_nr);

int sci01_priority_table_flags = 0;

reg_t kDrawPic(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int pic_nr = SKPV(0);
	int add_to_pic = 1;
	int palette = SKPV_OR_ALT(3, 0);
	gfx_color_t transparent = s->wm_port->bgcolor;

	CHECK_THIS_KERNEL_FUNCTION;

	if (s->version < SCI_VERSION_FTU_NEWER_DRAWPIC_PARAMETERS) {
		if (!SKPV_OR_ALT(2, 0))
			add_to_pic = 0;
	} else {
		if (SKPV_OR_ALT(2, 1))
			add_to_pic = 0;
	}

	gfxop_disable_dirty_frames(s->gfx_state);

	if (NULL != s->old_screen) {
		gfxop_free_pixmap(s->gfx_state, s->old_screen);
	}

	s->old_screen = gfxop_grab_pixmap(s->gfx_state, gfx_rect(0, 10, 320, 190));

	SCIkdebug(SCIkGRAPHICS, "Drawing pic.%03d\n", SKPV(0));

	if (!s->pics) {
		s->pics = (drawn_pic_t*)sci_malloc(sizeof(drawn_pic_t) * (s->pics_nr = 8));
		s->pics_drawn_nr = 0;
	}

	if (add_to_pic) {
		if (s->pics_nr == s->pics_drawn_nr) {
			s->pics_nr += 4;
			s->pics = (drawn_pic_t*)sci_realloc(s->pics, sizeof(drawn_pic_t) * s->pics_nr);
		}
		s->pics[s->pics_drawn_nr].palette = palette;
		s->pics[s->pics_drawn_nr++].nr = pic_nr;
		GFX_ASSERT(gfxop_add_to_pic(s->gfx_state, pic_nr, 1, palette));
	} else {
		s->pics_drawn_nr = 1;
		s->pics[0].nr = pic_nr;
		s->pics[0].palette = palette;
		GFX_ASSERT(gfxop_new_pic(s->gfx_state, pic_nr, 1, palette));
	}

	gfxw_widget_kill_chrono(s->visual, 0);
	s->wm_port->widfree(GFXW(s->wm_port));
	s->picture_port->widfree(GFXW(s->picture_port));
	s->iconbar_port->widfree(GFXW(s->iconbar_port));

	s->wm_port = gfxw_new_port(s->visual, NULL, s->gfx_state->options->pic_port_bounds, s->ega_colors[0], transparent);
	s->picture_port = gfxw_new_port(s->visual, NULL, s->gfx_state->options->pic_port_bounds, s->ega_colors[0], transparent);
	s->iconbar_port = gfxw_new_port(s->visual, NULL, gfx_rect(0, 0, 320, 200), s->ega_colors[0], transparent);
	s->iconbar_port->flags |= GFXW_FLAG_NO_IMPLICIT_SWITCH;

	s->visual->add(GFXWC(s->visual), GFXW(s->picture_port));
	s->visual->add(GFXWC(s->visual), GFXW(s->wm_port));
	s->visual->add(GFXWC(s->visual), GFXW(s->iconbar_port));

	s->port = s->picture_port;

	s->pic_priority_table = (int*)gfxop_get_pic_metainfo(s->gfx_state);

	if (sci01_priority_table_flags & 0x2) {
		if (s->pic_priority_table) {
			int i;
			printf("---------------------------\nPriority table:\n");
			for (i = 0; i < 16; i++)
				printf("\t%d:\t%d\n", i, s->pic_priority_table[i]);
			printf("---------------------------\n");
			error("Error");
		}
	}
	if (sci01_priority_table_flags & 0x1)
		s->pic_priority_table = NULL;

	if (argc > 1)
		s->pic_animate = SKPV(1); // The animation used during kAnimate() later on

	s->dyn_views = NULL;
	s->drop_views = NULL;

	s->priority_first = 42;

	if (s->version < SCI_VERSION_FTU_PRIORITY_14_ZONES)
		s->priority_last = 200;
	else
		s->priority_last = 190;

	s->pic_not_valid = 1;
	s->pic_is_new = 1;

	return s->r_acc;

}

abs_rect_t set_base(EngineState *s, reg_t object) {
	int x, y, original_y, z, ystep, xsize, ysize;
	int xbase, ybase, xend, yend;
	int view, loop, cel;
	int oldloop, oldcel;
	int xmod = 0, ymod = 0;
	abs_rect_t retval;

	x = GET_SEL32SV(object, x);
	original_y = y = GET_SEL32SV(object, y);

	if (s->selector_map.z > -1)
		z = GET_SEL32SV(object, z);
	else
		z = 0;

	y -= z; // Subtract z offset

	ystep = GET_SEL32SV(object, yStep);

	view = GET_SEL32SV(object, view);
	oldloop = loop = sign_extend_byte(GET_SEL32V(object, loop));
	oldcel = cel = sign_extend_byte(GET_SEL32V(object, cel));

	if (gfxop_check_cel(s->gfx_state, view, &loop, &cel)) {
		xsize = ysize = xmod = ymod = 0;
	} else {
		Common::Point offset = Common::Point(0, 0);

		if (loop != oldloop) {
			loop = 0;
			PUT_SEL32V(object, loop, 0);
			SCIkdebug(SCIkGRAPHICS, "Resetting loop for "PREG"", PRINT_REG(object));
		}

		if (cel != oldcel) {
			cel = 0;
			PUT_SEL32V(object, cel, 0);
		}

		gfxop_get_cel_parameters(s->gfx_state, view, loop, cel, &xsize, &ysize, &offset);

		xmod = offset.x;
		ymod = offset.y;
	}

	xbase = x - xmod - (xsize >> 1);
	xend = xbase + xsize;
	yend = y /* - ymod */ + 1;
	ybase = yend - ystep;

	SCIkdebug(SCIkBASESETTER, "(%d,%d)+/-(%d,%d), (%d x %d) -> (%d, %d) to (%d, %d)\n",
	          x, y, xmod, ymod, xsize, ysize, xbase, ybase, xend, yend);

	retval.x = xbase;
	retval.y = ybase;
	retval.xend = xend;
	retval.yend = yend;

	return retval;
}

void _k_base_setter(EngineState *s, reg_t object) {
	abs_rect_t absrect = set_base(s, object);

	if (lookup_selector(s, object, s->selector_map.brLeft, NULL, NULL) != SELECTOR_VARIABLE)
		return; // non-fatal

	if (s->version <= SCI_VERSION_LTU_BASE_OB1)
		--absrect.y; // Compensate for early SCI OB1 'bug'

	PUT_SEL32V(object, brLeft, absrect.x);
	PUT_SEL32V(object, brRight, absrect.xend);
	PUT_SEL32V(object, brTop, absrect.y);
	PUT_SEL32V(object, brBottom, absrect.yend);
}

reg_t kBaseSetter(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t object = argv[0];

	_k_base_setter(s, object);

	return s->r_acc;
} // kBaseSetter

static inline abs_rect_t nsrect_clip(EngineState *s, int y, abs_rect_t retval, int priority) {
	int pri_top;

	if (priority == -1)
		priority = VIEW_PRIORITY(y);

	pri_top = PRIORITY_BAND_FIRST(priority) + 1;
	// +1: Don't know why, but this seems to be happening

	if (retval.y < pri_top)
		retval.y = pri_top;

	if (retval.yend < retval.y)
		retval.y = retval.yend - 1;

	return retval;
}

inline abs_rect_t calculate_nsrect(EngineState *s, int x, int y, int view, int loop, int cel) {
	int xbase, ybase, xend, yend, xsize, ysize;
	int xmod = 0, ymod = 0;
	abs_rect_t retval = {0, 0, 0, 0};

	if (gfxop_check_cel(s->gfx_state, view, &loop, &cel)) {
		xsize = ysize = xmod = ymod = 0;
	} else {
		Common::Point offset = Common::Point(0, 0);

		gfxop_get_cel_parameters(s->gfx_state, view, loop, cel, &xsize, &ysize, &offset);

		xmod = offset.x;
		ymod = offset.y;
	}

	xbase = x - xmod - (xsize >> 1);
	xend = xbase + xsize;
	yend = y - ymod + 1; // +1: magic modifier
	ybase = yend - ysize;

	retval.x = xbase;
	retval.y = ybase;
	retval.xend = xend;
	retval.yend = yend;

	return retval;
}

inline abs_rect_t get_nsrect(EngineState *s, reg_t object, byte clip) {
	int x, y, z;
	int view, loop, cel;
	abs_rect_t retval;

	x = GET_SEL32SV(object, x);
	y = GET_SEL32SV(object, y);

	if (s->selector_map.z > -1)
		z = GET_SEL32SV(object, z);
	else
		z = 0;

	y -= z; // Subtract z offset

	view = GET_SEL32SV(object, view);
	loop = sign_extend_byte(GET_SEL32SV(object, loop));
	cel = sign_extend_byte(GET_SEL32SV(object, cel));

	retval = calculate_nsrect(s, x, y, view, loop, cel);

	if (clip) {
		int priority = GET_SEL32SV(object, priority);
		return nsrect_clip(s, y, retval, priority);
	}

	return retval;
}

static void _k_set_now_seen(EngineState *s, reg_t object) {
	abs_rect_t absrect = get_nsrect(s, object, 0);

	if (lookup_selector(s, object, s->selector_map.nsTop, NULL, NULL) != SELECTOR_VARIABLE) {
		return; 
	} // This isn't fatal

	PUT_SEL32V(object, nsLeft, absrect.x);
	PUT_SEL32V(object, nsRight, absrect.xend);
	PUT_SEL32V(object, nsTop, absrect.y);
	PUT_SEL32V(object, nsBottom, absrect.yend);
}

reg_t kSetNowSeen(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t object = argv[0];

	_k_set_now_seen(s, object);

	return s->r_acc;
} // kSetNowSeen

reg_t kPalette(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	switch (UKPV(0)) {
	case 5 : {
		int r = UKPV(1);
		int g = UKPV(2);
		int b = UKPV(3);

		int i, delta, bestindex = -1, bestdelta = 200000;

		for (i = 0; i < KERNEL_COLORS_NR; i++) {
			int dr = abs(KERNEL_COLOR_PALETTE[i].r - r);
			int dg = abs(KERNEL_COLOR_PALETTE[i].g - g);
			int db = abs(KERNEL_COLOR_PALETTE[i].b - b);

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

	case 4 :
	case 6 :
		break;
	default :
		warning("Unimplemented subfunction: %d", UKPV(0));
	}

	return s->r_acc;
}

static void _k_draw_control(EngineState *s, reg_t obj, int inverse);

static void _k_disable_delete_for_now(EngineState *s, reg_t obj) {
	reg_t text_pos = GET_SEL32(obj, text);
	char *text = IS_NULL_REG(text_pos) ? NULL : (char *) sm_dereference(&s->seg_manager, text_pos, NULL);
	int type = GET_SEL32V(obj, type);
	int state = GET_SEL32V(obj, state);

	if (type == K_CONTROL_BUTTON && text && !strcmp(s->game_name, "sq4") &&
			s->version < SCI_VERSION(1, 001, 000) && !strcmp(text, " Delete ")) {
		PUT_SEL32V(obj, state, (state | CONTROL_STATE_GRAY) & ~CONTROL_STATE_ENABLED);
	}
}

reg_t kDrawControl(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t obj = argv[0];

	_k_disable_delete_for_now(s, obj);
	_k_draw_control(s, obj, 0);
	FULL_REDRAW();
	return NULL_REG;
}

reg_t kHiliteControl(EngineState *s, int funct_nr, int argc, reg_t *argv) {
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

reg_t kEditControl(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t obj = argv[0];
	reg_t event = argv[1];

	if (obj.segment) {
		word ct_type = GET_SEL32V(obj, type);
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

				char *text = (char *) sm_dereference(&s->seg_manager, text_pos, NULL);
				int textlen;

				if (!text) {
					warning("Could not draw control: "PREG" does not reference text", PRINT_REG(text_pos));
					return s->r_acc;
				}

				if (REG_EQ(text_pos, s->save_dir_copy)) {
					max = MAX_SAVE_DIR_SIZE - 1;
					display_offset = s->save_dir_edit_offset;
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
						while ((cursor < textlen) && (text[cursor++] != ' '));
						break;
					case 0x3000 /* A-b */:
						while ((cursor > 0) && (text[--cursor - 1] != ' '));
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

					if (REG_EQ(text_pos, s->save_dir_copy))
						s->save_dir_edit_offset = display_offset;

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
			PUT_SEL32V(obj, state, state | CONTROL_STATE_DITHER_FRAMED);
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
	int x = GET_SEL32SV(obj, nsLeft);
	int y = GET_SEL32SV(obj, nsTop);
	int xl = GET_SEL32SV(obj, nsRight) - x;
	int yl = GET_SEL32SV(obj, nsBottom) - y;
	rect_t area = gfx_rect(x, y, xl, yl);

	int font_nr = GET_SEL32V(obj, font);
	reg_t text_pos = GET_SEL32(obj, text);
	char *text = IS_NULL_REG(text_pos) ? NULL : (char *)sm_dereference(&s->seg_manager, text_pos, NULL);
	int view = GET_SEL32V(obj, view);
	int cel = sign_extend_byte(GET_SEL32V(obj, cel));
	int loop = sign_extend_byte(GET_SEL32V(obj, loop));
	gfx_alignment_t mode;

	int type = GET_SEL32V(obj, type);
	int state = GET_SEL32V(obj, state);
	int cursor;
	int max;

	if (REG_EQ(text_pos, s->save_dir_copy)) {
		SCIkdebug(SCIkGRAPHICS, "Displaying the save_dir copy\n");
	}

	switch (type) {
	case K_CONTROL_BUTTON:
		SCIkdebug(SCIkGRAPHICS, "drawing button "PREG" to %d,%d\n", PRINT_REG(obj), x, y);
		ADD_TO_CURRENT_BG_WIDGETS(sciw_new_button_control(s->port, obj, area, text, font_nr,
		                          (gint8)(state & CONTROL_STATE_FRAMED), (gint8)inverse, (gint8)(state & CONTROL_STATE_GRAY)));
		break;

	case K_CONTROL_TEXT:
		mode = (gfx_alignment_t) GET_SEL32V(obj, mode);

		SCIkdebug(SCIkGRAPHICS, "drawing text "PREG" to %d,%d, mode=%d\n", PRINT_REG(obj), x, y, mode);

		ADD_TO_CURRENT_BG_WIDGETS(sciw_new_text_control(s->port, obj, area, text, font_nr, mode,
									(gint8)(!!(state & CONTROL_STATE_DITHER_FRAMED)), (gint8)inverse));
		break;

	case K_CONTROL_EDIT:
		SCIkdebug(SCIkGRAPHICS, "drawing edit control "PREG" to %d,%d\n", PRINT_REG(obj), x, y);

		max = GET_SEL32V(obj, max);
		cursor = GET_SEL32V(obj, cursor);

		if (cursor > (signed)strlen(text))
			cursor = strlen(text);

		if (REG_EQ(text_pos, s->save_dir_copy))
			update_cursor_limits(&s->save_dir_edit_offset, &cursor, max);

		update_cursor_limits(&s->save_dir_edit_offset, &cursor, max);
		ADD_TO_CURRENT_BG_WIDGETS(sciw_new_edit_control(s->port, obj, area, text, font_nr, (unsigned)cursor, (gint8)inverse));
		break;

	case K_CONTROL_ICON:

		SCIkdebug(SCIkGRAPHICS, "drawing icon control "PREG" to %d,%d\n", PRINT_REG(obj), x, y - 1);

		ADD_TO_CURRENT_BG_WIDGETS(sciw_new_icon_control(s->port, obj, area, view, loop, cel,
		                          (gint8)(state & CONTROL_STATE_FRAMED), (gint8)inverse));
		break;

	case K_CONTROL_CONTROL:
	case K_CONTROL_CONTROL_ALIAS: {
		char **entries_list = NULL;
		char *seeker;
		int entries_nr;
		int lsTop = GET_SEL32V(obj, lsTop) - text_pos.offset;
		int list_top = 0;
		int selection = 0;
		int entry_size = GET_SEL32V(obj, x);
		int i;

		SCIkdebug(SCIkGRAPHICS, "drawing list control %04x to %d,%d, diff %d\n", obj, x, y, SCI_MAX_SAVENAME_LENGTH);
		cursor = GET_SEL32V(obj, cursor) - text_pos.offset;

		entries_nr = 0;
		seeker = text;
		while (seeker[0]) { // Count string entries in NULL terminated string list
			++entries_nr;
			seeker += entry_size;
		}

		if (entries_nr) { // determine list_top, selection, and the entries_list
			seeker = text;
			entries_list = (char**)sci_malloc(sizeof(char *) * entries_nr);
			for (i = 0; i < entries_nr; i++) {
				entries_list[i] = seeker;
				seeker += entry_size	;
				if ((seeker - text) == lsTop)
					list_top = i + 1;
				if ((seeker - text) == cursor)
					selection = i + 1;
			}
		}

		ADD_TO_CURRENT_BG_WIDGETS(sciw_new_list_control(s->port, obj, area, font_nr, entries_list, entries_nr,
		                          list_top, selection, (gint8)inverse));
		if (entries_nr)
			free(entries_list);
	}
	break;

	case K_CONTROL_BOX:
		break;

	default:
		warning("Unknown control type: %d at "PREG", at (%d, %d) size %d x %d",
		         type, PRINT_REG(obj), x, y, xl, yl);
	}

	if (!s->pic_not_valid) {
		FULL_REDRAW();
	}
}


static void draw_rect_to_control_map(EngineState *s, abs_rect_t abs_zone) {
	gfxw_box_t *box;
	gfx_color_t color;

	gfxop_set_color(s->gfx_state, &color, -1, -1, -1, -1, -1, 0xf);

	SCIkdebug(SCIkGRAPHICS, "    adding control block (%d,%d)to(%d,%d)\n", abs_zone.x, abs_zone.y, abs_zone.xend, abs_zone.yend);

	box = gfxw_new_box(s->gfx_state, gfx_rect(abs_zone.x, abs_zone.y, abs_zone.xend - abs_zone.x, 
						abs_zone.yend - abs_zone.y), color, color, GFX_BOX_SHADE_FLAT);

	assert_primary_widget_lists(s);

	ADD_TO_CURRENT_PICTURE_PORT(box);
}

static inline void draw_obj_to_control_map(EngineState *s, gfxw_dyn_view_t *view) {
	reg_t obj = make_reg(view->ID, view->subID);

	if (!is_object(s, obj))
		warning("View %d does not contain valid object reference "PREG"", view->ID, PRINT_REG(obj));

	if (!(view->signalp && (((reg_t *)view->signalp)->offset & _K_VIEW_SIG_FLAG_IGNORE_ACTOR))) {
		abs_rect_t abs_zone = get_nsrect(s, make_reg(view->ID, view->subID), 1);
		draw_rect_to_control_map(s, abs_zone);
	}
}

static void _k_view_list_do_postdraw(EngineState *s, gfxw_list_t *list) {
	gfxw_dyn_view_t *widget = (gfxw_dyn_view_t *) list->contents;

	while (widget) {
		reg_t obj = make_reg(widget->ID, widget->subID);

		if (widget->type == GFXW_SORTED_LIST)
			_k_view_list_do_postdraw(s, GFXWC(widget));

		if (widget->type != GFXW_DYN_VIEW) {
			widget = (gfxw_dyn_view_t *) widget->next;
			continue;
		}

		/*
		 * this fixes a few problems, but doesn't match SSCI's logic.
		 * The semantics of the private flag need to be verified before this can be uncommented.
		 * Fixes bug #326 (CB1, ego falls down stairs)
		 * if ((widget->signal & (_K_VIEW_SIG_FLAG_FREESCI_PRIVATE | _K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_NO_UPDATE)) == _K_VIEW_SIG_FLAG_FREESCI_PRIVATE) {
		 */
		if ((widget->signal & (_K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_NO_UPDATE)) == 0) {
			int has_nsrect = lookup_selector(s, obj, s->selector_map.nsBottom, NULL, NULL) == SELECTOR_VARIABLE;

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
				error("lsRected "PREG"", PRINT_REG(obj));
#endif
			}
#ifdef DEBUG_LSRECT
			else 
				error("Not lsRecting "PREG" because %d", PRINT_REG(obj), lookup_selector(s, obj, s->selector_map.nsBottom, NULL, NULL));
#endif

			if (widget->signal & _K_VIEW_SIG_FLAG_HIDDEN)
				widget->signal |= _K_VIEW_SIG_FLAG_REMOVE;
		}
#ifdef DEBUG_LSRECT
		error("obj "PREG" has pflags %x", PRINT_REG(obj), (widget->signal & (_K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_NO_UPDATE)));
#endif

		if (widget->signalp) {
			*((reg_t *)(widget->signalp)) = make_reg(0, widget->signal & 0xffff); /* Write back signal */
		}

		widget = (gfxw_dyn_view_t *) widget->next;
	}
}

void _k_view_list_mark_free(EngineState *s, reg_t off) {
	if (s->dyn_views) {

		gfxw_dyn_view_t *w = (gfxw_dyn_view_t *) s->dyn_views->contents;

		while (w) {
			if (w->ID == off.segment
			        && w->subID == off.offset) {
				w->under_bitsp = NULL;
			}

			w = (gfxw_dyn_view_t *) w->next;
		}
	}
}

static int _k_animate_ran = 0;

int _k_view_list_dispose_loop(EngineState *s, list_t *list, gfxw_dyn_view_t *widget, int funct_nr, int argc, reg_t *argv) {
// disposes all list members flagged for disposal; funct_nr is the invoking kfunction
// returns non-zero IFF views were dropped
	int signal;
	int dropped = 0;

	_k_animate_ran = 0;

	if (widget) {
		int retval;
		// Recurse:
		retval = _k_view_list_dispose_loop(s, list, (gfxw_dyn_view_t *) widget->next, funct_nr, argc, argv);

		if (retval == -1) // Bail out on annihilation, rely on re-start from Animate()
			return -1;

		if (GFXW_IS_DYN_VIEW(widget) && (widget->ID != GFXW_NO_ID)) {
			signal = ((reg_t *)widget->signalp)->offset;
			if (signal & _K_VIEW_SIG_FLAG_DISPOSE_ME) {
				reg_t obj = make_reg(widget->ID, widget->subID);
				reg_t under_bits = NULL_REG;

				if (!is_object(s, obj)) {
					error("Non-object "PREG" present in view list during delete time", PRINT_REG(obj));
					obj = NULL_REG;
				} else
					if (widget->under_bitsp) { // Is there a bg picture left to clean?
						reg_t mem_handle = *((reg_t*)(widget->under_bitsp));

						if (mem_handle.segment) {
							if (!kfree(s, mem_handle)) {
								*((reg_t*)(widget->under_bitsp)) = make_reg(0, widget->under_bits = 0);
							} else {
								warning("Treating viewobj "PREG" as no longer present", PRINT_REG(obj));
								obj = NULL_REG;
							}
						}
					}

				if (is_object(s, obj)) {
					if (invoke_selector(INV_SEL(obj, delete_, 1), 0))
						warning("Object at "PREG" requested deletion, but does not have a delete funcselector", PRINT_REG(obj));
					if (_k_animate_ran) {
						warning("Object at "PREG" invoked kAnimate() during deletion", PRINT_REG(obj));
						return dropped;
					}

					if (widget->under_bitsp)
						under_bits = *((reg_t*)(widget->under_bitsp));

					if (under_bits.segment) {
						*((reg_t*)(widget->under_bitsp)) = make_reg(0, 0);
						graph_restore_box(s, under_bits);
					}

					SCIkdebug(SCIkGRAPHICS, "Freeing "PREG" with signal=%04x\n", PRINT_REG(obj), signal);

					if (!(signal & _K_VIEW_SIG_FLAG_HIDDEN)) {
						SCIkdebug(SCIkGRAPHICS, "Adding view at "PREG" to background\n", PRINT_REG(obj));
						if (!(gfxw_remove_id(widget->parent, widget->ID, widget->subID) == GFXW(widget))) {
							error("Attempt to remove view with ID %x:%x from list failed", widget->ID, widget->subID);
							BREAKPOINT();
						}

						s->drop_views->add(GFXWC(s->drop_views), GFXW(gfxw_picviewize_dynview(widget)));

						draw_obj_to_control_map(s, widget);
						widget->draw_bounds.y += s->dyn_views->bounds.y - widget->parent->bounds.y;
						widget->draw_bounds.x += s->dyn_views->bounds.x - widget->parent->bounds.x;
						dropped = 1;
					} else {
						SCIkdebug(SCIkGRAPHICS, "Deleting view at "PREG"\n", PRINT_REG(obj));
						widget->flags |= GFXW_FLAG_VISIBLE;
						gfxw_annihilate(GFXW(widget));
						return -1; // restart: Done in Animate()
					}
				}
			}
		}

	}

	return dropped;
}


#define _K_MAKE_VIEW_LIST_CYCLE 1
#define _K_MAKE_VIEW_LIST_CALC_PRIORITY 2
#define _K_MAKE_VIEW_LIST_DRAW_TO_CONTROL_MAP 4

static gfxw_dyn_view_t *_k_make_dynview_obj(EngineState *s, reg_t obj, int options, int nr, int funct_nr, int argc, reg_t *argv) {
	short oldloop, oldcel;
	int cel, loop, view_nr = GET_SEL32SV(obj, view);
	int palette;
	int signal;
	reg_t under_bits;
	reg_t *under_bitsp, *signalp;
	Common::Point pos;
	int z;
	gfxw_dyn_view_t *widget;

	SCIkdebug(SCIkGRAPHICS, " - Adding "PREG"\n", PRINT_REG(obj));

	obj = obj;

	pos.x = GET_SEL32SV(obj, x);
	pos.y = GET_SEL32SV(obj, y);

	pos.y++; // magic: Sierra appears to do something like this

	z = GET_SEL32SV(obj, z);

	// !-- nsRect used to be checked here!
	loop = oldloop = sign_extend_byte(GET_SEL32V(obj, loop));
	cel = oldcel = sign_extend_byte(GET_SEL32V(obj, cel));

	if (s->selector_map.palette)
		palette = GET_SEL32V(obj, palette);
	else
		palette = 0;

	// Clip loop and cel, write back if neccessary
	if (gfxop_check_cel(s->gfx_state, view_nr, &loop, &cel)) {
		return NULL;
	}

	if (loop != oldloop)
		loop = 0;
	if (cel != oldcel)
		cel = 0;

	if (oldloop != loop)
		PUT_SEL32V(obj, loop, loop);

	if (oldcel != cel) {
		PUT_SEL32V(obj, cel, cel);
	}

	if (lookup_selector(s, obj, s->selector_map.underBits, &(under_bitsp), NULL) != SELECTOR_VARIABLE) {
		under_bitsp = NULL;
		under_bits = NULL_REG;
		SCIkdebug(SCIkGRAPHICS, "Object at "PREG" has no underBits\n", PRINT_REG(obj));
	} else
		under_bits = *((reg_t *)under_bitsp);

	if (lookup_selector(s, obj, s->selector_map.signal, &(signalp), NULL) != SELECTOR_VARIABLE) {
		signalp = NULL;
		signal = 0;
		SCIkdebug(SCIkGRAPHICS, "Object at "PREG" has no signal selector\n", PRINT_REG(obj));
	} else {
		signal = signalp->offset;
		SCIkdebug(SCIkGRAPHICS, "    with signal = %04x\n", signal);
	}

	widget = gfxw_new_dyn_view(s->gfx_state, pos, z, view_nr, loop, cel, palette, -1, -1, ALIGN_CENTER, ALIGN_BOTTOM, nr);

	if (widget) {
		widget = (gfxw_dyn_view_t *) gfxw_set_id(GFXW(widget), obj.segment, obj.offset);
		widget = gfxw_dyn_view_set_params(widget, under_bits.segment, under_bitsp, signal, signalp);
		widget->flags |= GFXW_FLAG_IMMUNE_TO_SNAPSHOTS; // Only works the first time 'round'

		return widget;
	} else {
		warning("Could not generate dynview widget for %d/%d/%d", view_nr, loop, cel);
		return NULL;
	}
}

static void _k_make_view_list(EngineState *s, gfxw_list_t **widget_list, list_t *list, int options, int funct_nr, int argc, reg_t *argv) {
/* Creates a view_list from a node list in heap space. Returns the list, stores the
** number of list entries in *list_nr. Calls doit for each entry if cycle is set.
** argc, argv, funct_nr should be the same as in the calling kernel function.
*/
	node_t *node;
	int sequence_nr = 0;
	gfxw_dyn_view_t *widget;

	if (!*widget_list) {
		error("make_view_list with widget_list == ()");
		BREAKPOINT();
	};

	assert_primary_widget_lists(s);
	// In case one of the views' doit() does a DrawPic...
	// Yes, this _does_ happen!

	if (!list) { // list sanity check
		error("Attempt to make list from non-list");
		BREAKPOINT();
	}

	node = LOOKUP_NODE(list->first);
	while (node) {
		reg_t obj = node->value; // The object we're using
		reg_t next_node;
		gfxw_dyn_view_t *tempWidget;

		if (options & _K_MAKE_VIEW_LIST_CYCLE) {
			unsigned int signal = GET_SEL32V(obj, signal);

			if (!(signal & _K_VIEW_SIG_FLAG_FROZEN)) {

				SCIkdebug(SCIkGRAPHICS, "  invoking "PREG"::doit()\n", PRINT_REG(obj));
				invoke_selector(INV_SEL(obj, doit, 1), 0); // Call obj::doit() if neccessary
			}
		}

		next_node = node->succ; // In case the cast list was changed

		if (list->first.segment == 0 && list->first.offset == 0) // The cast list was completely emptied!
			break;

		tempWidget = _k_make_dynview_obj(s, obj, options, sequence_nr--, funct_nr, argc, argv);
		if (tempWidget)
			GFX_ASSERT((*widget_list)->add(GFXWC(*widget_list), GFXW(tempWidget)));

		node = LOOKUP_NODE(next_node); // Next node
	}

	widget = (gfxw_dyn_view_t *)(*widget_list)->contents;

	while (widget) { // Read back widget values
		if (widget->signalp)
			widget->signal = ((reg_t *)(widget->signalp))->offset;

		widget = (gfxw_dyn_view_t *) widget->next;
	}
}

static void _k_prepare_view_list(EngineState *s, gfxw_list_t *list, int options) {
	gfxw_dyn_view_t *view = (gfxw_dyn_view_t *) list->contents;
	while (view) {
		reg_t obj = make_reg(view->ID, view->subID);
		int priority, _priority;
		int has_nsrect = (view->ID <= 0) ? 0 : lookup_selector(s, obj, s->selector_map.nsBottom, NULL, NULL) == SELECTOR_VARIABLE;
		int oldsignal = view->signal;

		_k_set_now_seen(s, obj);
		_priority = /*GET_SELECTOR(obj, y); */((view->pos.y));
		_priority = _find_view_priority(s, _priority - 1);

		if (options & _K_MAKE_VIEW_LIST_DRAW_TO_CONTROL_MAP) { // Picview
			priority = GET_SEL32SV(obj, priority);
			if (priority < 0)
				priority = _priority; // Always for picviews
		} else { // Dynview
			if (has_nsrect && !(view->signal & _K_VIEW_SIG_FLAG_FIX_PRI_ON)) { // Calculate priority
				if (options & _K_MAKE_VIEW_LIST_CALC_PRIORITY)
					PUT_SEL32V(obj, priority, _priority);

				priority = _priority;

			} else // DON'T calculate the priority
				priority = GET_SEL32SV(obj, priority);
		}

		view->color.priority = priority;

		if (priority > -1)
			view->color.mask |= GFX_MASK_PRIORITY;
		else
			view->color.mask &= ~GFX_MASK_PRIORITY;

		// CR (from :Bob Heitman:) stopupdated views (like pic views) have
		// their clipped nsRect drawn to the control map
		if (view->signal & _K_VIEW_SIG_FLAG_STOP_UPDATE) {
			view->signal |= _K_VIEW_SIG_FLAG_FREESCI_STOPUPD;
			SCIkdebug(SCIkGRAPHICS, "Setting magic STOP_UPD for "PREG"\n", PRINT_REG(obj));
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

		SCIkdebug(SCIkGRAPHICS, "  dv["PREG"]: signal %04x -> %04x\n", PRINT_REG(obj), oldsignal, view->signal);

		// Never happens
/*		if (view->signal & 0) {
			view->signal &= ~_K_VIEW_SIG_FLAG_FREESCI_STOPUPD;
			error("Unsetting magic StopUpd for view "PREG"", PRINT_REG(obj));
		} */

		view = (gfxw_dyn_view_t *) view->next;
	}
}

static void _k_update_signals_in_view_list(gfxw_list_t *old_list, gfxw_list_t *new_list) {
	// O(n^2)... a bit painful, but much faster than the redraws it helps prevent
	gfxw_dyn_view_t *old_widget = (gfxw_dyn_view_t *) old_list->contents;

	/* Traverses all old widgets, updates them with signals from the new widgets.
	** This is done to avoid evil hacks in widget.c; widgets with unique IDs are
	** replaced there iff they are NOT equal_to a new widget with the same ID.
	** If they were replaced every time, we'd be doing far too many redraws.
	*/

	while (old_widget) {
		gfxw_dyn_view_t *new_widget = (gfxw_dyn_view_t *) new_list->contents;

		while (new_widget
		        && (new_widget->ID != old_widget->ID
		            || new_widget->subID != old_widget->subID))
			new_widget = (gfxw_dyn_view_t *) new_widget->next;

		if (new_widget) {
			int carry = old_widget->signal & _K_VIEW_SIG_FLAG_FREESCI_STOPUPD;
			// Transfer 'stopupd' flag

			if ((new_widget->pos.x != old_widget->pos.x)
			        || (new_widget->pos.y != old_widget->pos.y)
					// No idea why this is supposed to be bad
/*			        || (new_widget->z != old_widget->z)
			        || (new_widget->view != old_widget->view)
			        || (new_widget->loop != old_widget->loop)
			        || (new_widget->cel != old_widget->cel)
			        */)
				carry = 0;

			old_widget->signal = new_widget->signal |= carry;
		}

		old_widget = (gfxw_dyn_view_t *) old_widget->next;
	}
}

static void _k_view_list_kryptonize(gfxw_widget_t *v) {
	if (v) {
		v->flags &= ~GFXW_FLAG_IMMUNE_TO_SNAPSHOTS;
		_k_view_list_kryptonize(v->next);
	}
}

static void _k_raise_topmost_in_view_list(EngineState *s, gfxw_list_t *list, gfxw_dyn_view_t *view) {
	if (view) {
		gfxw_dyn_view_t *next = (gfxw_dyn_view_t *)view->next;

		// step 11
		if ((view->signal & (_K_VIEW_SIG_FLAG_NO_UPDATE | _K_VIEW_SIG_FLAG_HIDDEN | _K_VIEW_SIG_FLAG_ALWAYS_UPDATE)) == 0) {
			SCIkdebug(SCIkGRAPHICS, "Forcing precedence 2 at ["PREG"] with %04x\n", PRINT_REG(make_reg(view->ID, view->subID)), view->signal);
			view->force_precedence = 2;

			if ((view->signal & (_K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_HIDDEN)) == _K_VIEW_SIG_FLAG_REMOVE) {
				view->signal &= ~_K_VIEW_SIG_FLAG_REMOVE;
			}
		}

		gfxw_remove_widget_from_container(view->parent, GFXW(view));

		gfxw_widget_reparent_chrono(s->visual, GFXW(view), GFXWC(list));

		if (view->signal & _K_VIEW_SIG_FLAG_HIDDEN)
			gfxw_hide_widget(GFXW(view));
		else
			gfxw_show_widget(GFXW(view));

		list->add(GFXWC(list), GFXW(view));

		_k_raise_topmost_in_view_list(s, list, next);
	}
}

static void _k_redraw_view_list(EngineState *s, gfxw_list_t *list) {
	gfxw_dyn_view_t *view = (gfxw_dyn_view_t *) list->contents;
	while (view) {

		SCIkdebug(SCIkGRAPHICS, "  dv["PREG"]: signal %04x\n", make_reg(view->ID, view->subID), view->signal);

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

		SCIkdebug(SCIkGRAPHICS, "    at substep 6: signal %04x\n", view->signal);

		if (view->signal & _K_VIEW_SIG_FLAG_ALWAYS_UPDATE)
			view->signal &= ~(_K_VIEW_SIG_FLAG_STOP_UPDATE | _K_VIEW_SIG_FLAG_UPDATED | _K_VIEW_SIG_FLAG_NO_UPDATE | _K_VIEW_SIG_FLAG_FORCE_UPDATE);

		SCIkdebug(SCIkGRAPHICS, "    at substep 11/14: signal %04x\n", view->signal);

		if (view->signal & _K_VIEW_SIG_FLAG_NO_UPDATE) {
			if (view->signal & _K_VIEW_SIG_FLAG_HIDDEN)
				view->signal |= _K_VIEW_SIG_FLAG_REMOVE;
			else
				view->signal &= ~_K_VIEW_SIG_FLAG_REMOVE;
		} else if (!(view->signal & _K_VIEW_SIG_FLAG_HIDDEN))
			view->force_precedence = 1;

		SCIkdebug(SCIkGRAPHICS, "    -> signal %04x\n", view->signal);

		view = (gfxw_dyn_view_t *) view->next;
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

void _k_draw_view_list(EngineState *s, gfxw_list_t *list, int flags) {
	// Draws list_nr members of list to s->pic.
	gfxw_dyn_view_t *widget = (gfxw_dyn_view_t *) list->contents;

	if (GFXWC(s->port) != GFXWC(s->dyn_views->parent))
		return; // Return if the pictures are meant for a different port

	while (widget) {
		if (flags & _K_DRAW_VIEW_LIST_PICVIEW)
			widget = gfxw_picviewize_dynview(widget);

		if (GFXW_IS_DYN_VIEW(widget) && widget->ID) {
			word signal = (flags & _K_DRAW_VIEW_LIST_USE_SIGNAL) ? ((reg_t *)(widget->signalp))->offset : 0;

			if (signal & _K_VIEW_SIG_FLAG_HIDDEN)
				gfxw_hide_widget(GFXW(widget));
			else
				gfxw_show_widget(GFXW(widget));

			if (!(flags & _K_DRAW_VIEW_LIST_USE_SIGNAL)
			        || ((flags & _K_DRAW_VIEW_LIST_DISPOSEABLE) && (signal & _K_VIEW_SIG_FLAG_DISPOSE_ME))
			        || ((flags & _K_DRAW_VIEW_LIST_NONDISPOSEABLE) && !(signal & _K_VIEW_SIG_FLAG_DISPOSE_ME))) {

				if (flags & _K_DRAW_VIEW_LIST_USE_SIGNAL) {
					signal &= ~(_K_VIEW_SIG_FLAG_STOP_UPDATE | _K_VIEW_SIG_FLAG_UPDATED | _K_VIEW_SIG_FLAG_NO_UPDATE | _K_VIEW_SIG_FLAG_FORCE_UPDATE);
					// Clear all of those flags

					if (signal & _K_VIEW_SIG_FLAG_HIDDEN)
						gfxw_hide_widget(GFXW(widget));
					else
						gfxw_show_widget(GFXW(widget));

					*((reg_t *)(widget->signalp)) = make_reg(0, signal); // Write the changes back
				};

			} // ...if we're drawing disposeables and this one is disposeable, or if we're drawing non-
			  // disposeables and this one isn't disposeable
		}

		widget = (gfxw_dyn_view_t *) widget->next;
	} // while (widget)

}

reg_t kAddToPic(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	gfxw_list_t *pic_views;
	reg_t list_ref = argv[0];

	assert_primary_widget_lists(s);

	if (argc > 1) {
		int view, cel, loop, x, y, priority, control;
		gfxw_widget_t *widget;

		view = KP_UINT(argv[0]);
		loop = KP_UINT(argv[1]);
		cel = KP_UINT(argv[2]);
		x = KP_SINT(argv[3]);
		y = KP_SINT(argv[4]) + 1 /* magic + 1 */;
		priority = KP_SINT(argv[5]);
		control = KP_SINT(argv[6]);

		widget = GFXW(gfxw_new_dyn_view(s->gfx_state, Common::Point(x, y), 0, view, loop, cel, 0,
		                                priority, -1 /* No priority */ , ALIGN_CENTER, ALIGN_BOTTOM, 0));

		if (!widget) {
			error("Attempt to single-add invalid picview (%d/%d/%d)", view, loop, cel);
		} else {
			widget->ID = -1;
			if (control >= 0) {
				abs_rect_t abs_zone = nsrect_clip(s, y, calculate_nsrect(s, x, y, view, loop, cel), priority);
				draw_rect_to_control_map(s, abs_zone);
			}
			ADD_TO_CURRENT_PICTURE_PORT(gfxw_picviewize_dynview((gfxw_dyn_view_t *) widget));
		}
	} else {
		list_t *list;

		if (!list_ref.segment) {
			warning("Attempt to AddToPic single non-list: "PREG"", PRINT_REG(list_ref));
			return s->r_acc;
		}

		list = LOOKUP_LIST(list_ref);

		pic_views = gfxw_new_list(s->picture_port->bounds, 1);

		SCIkdebug(SCIkGRAPHICS, "Preparing picview list...\n");
		_k_make_view_list(s, &pic_views, list, 0, funct_nr, argc, argv);
		_k_prepare_view_list(s, pic_views, _K_MAKE_VIEW_LIST_DRAW_TO_CONTROL_MAP);
		// Store pic views for later re-use

		SCIkdebug(SCIkGRAPHICS, "Drawing picview list...\n");
		ADD_TO_CURRENT_PICTURE_PORT(pic_views);
		_k_draw_view_list(s, pic_views, _K_DRAW_VIEW_LIST_NONDISPOSEABLE | _K_DRAW_VIEW_LIST_DISPOSEABLE | _K_DRAW_VIEW_LIST_PICVIEW);
		// Draw relative to the bottom center
		SCIkdebug(SCIkGRAPHICS, "Returning.\n");
	}
	reparentize_primary_widget_lists(s, s->port);

	return s->r_acc;
}

reg_t kGetPort(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	return make_reg(0, s->port->ID);
}

reg_t kSetPort(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	if (activated_icon_bar && argc == 6) {
		port_origin_x = port_origin_y = 0;
		activated_icon_bar = 0;
		return s->r_acc;
	}

	switch (argc) {
	case 1 : {
		unsigned int port_nr = SKPV(0);
		gfxw_port_t *new_port;

		/* We depart from official semantics here, sorry!
		   Reasoning: Sierra SCI does not clip ports while we do.
		   Therefore a draw to the titlebar port (which is the
		   official semantics) would cut off the lower part of the
		   icons in an SCI1 icon bar. Instead we have an
		   iconbar_port that does not exist in SSCI. */
		if (port_nr == (unsigned int) - 1) port_nr = s->iconbar_port->ID;

		new_port = gfxw_find_port(s->visual, port_nr);

		if (!new_port) {
			error("Invalid port %04x requested", port_nr);
			return NULL_REG;
		}

		s->port->draw(GFXW(s->port), gfxw_point_zero); // Update the port we're leaving
		s->port = new_port;
		return s->r_acc;
	}
	case 6 : {
		port_origin_y = SKPV(0);
		port_origin_x = SKPV(1);

		if (SKPV(0) == -10) {
			s->port->draw(GFXW(s->port), gfxw_point_zero); // Update the port we're leaving
			s->port = s->iconbar_port;
			activated_icon_bar = 1;
			return s->r_acc;
		}

		s->gfx_state->options->pic_port_bounds = gfx_rect(UKPV(5), UKPV(4),
		        UKPV(3), UKPV(2));
		// FIXME: Should really only invalidate all loaded pic resources here;
		// this is overkill
		gfxr_free_all_resources(s->gfx_state->driver, s->gfx_state->resstate);

		break;
	}
	default :
		error("SetPort was called with %d parameters", argc);
		break;
	}

	return NULL_REG;
}

static inline void add_to_chrono(EngineState *s, gfxw_widget_t *widget) {
	gfxw_port_t *chrono_port;
	gfxw_list_t *tw;

	chrono_port = gfxw_get_chrono_port(s->visual, &tw, 0);
	tw->add(GFXWC(tw), widget);

	if (!chrono_port->parent)
		ADD_TO_CURRENT_PORT(chrono_port);
}

reg_t kDrawCel(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int view = SKPV(0);
	int loop = SKPV(1);
	int cel = SKPV(2);
	int x = SKPV(3);
	int y = SKPV(4);
	int priority = SKPV_OR_ALT(5, -1);
	gfxw_view_t *new_view;

/*
	if (!view) {
		error("Attempt to draw non-existing view.%03d", view);
		return;
	}
*/

	if (gfxop_check_cel(s->gfx_state, view, &loop, &cel)) {
		error("Attempt to draw non-existing view.%03d", view);
		return s->r_acc;
	}

	SCIkdebug(SCIkGRAPHICS, "DrawCel((%d,%d), (view.%d, %d, %d), p=%d)\n", x, y, view, loop, cel, priority);

	new_view = gfxw_new_view(s->gfx_state, Common::Point(x, y), view, loop, cel, 0, priority, -1,
	                         ALIGN_LEFT, ALIGN_TOP, GFXW_VIEW_FLAG_DONT_MODIFY_OFFSET);

#if 0
	add_to_chrono(s, GFXW(new_view));
#else
	ADD_TO_CURRENT_PICTURE_PORT(GFXW(new_view));
#endif
	FULL_REDRAW();

	return s->r_acc;
}

reg_t kDisposeWindow(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	unsigned int goner_nr = SKPV(0);
	gfxw_port_t *goner;
	gfxw_port_t *pred;
	int id = s->visual->port_refs_nr;

	gfxw_widget_kill_chrono(s->visual, goner_nr);
	goner = gfxw_find_port(s->visual, goner_nr);
	if ((goner_nr < 3) || (goner == NULL)) {
		error("Removal of invalid window %04x requested", goner_nr);
		return s->r_acc;
	}

	if (s->dyn_views && GFXWC(s->dyn_views->parent) == GFXWC(goner)) {
		reparentize_primary_widget_lists(s, (gfxw_port_t *) goner->parent);
	}

	if (s->drop_views && GFXWC(s->drop_views->parent) == GFXWC(goner))
		s->drop_views = NULL; // Kill it

	pred = gfxw_remove_port(s->visual, goner);

	if (goner == s->port) // Did we kill the active port?
		s->port = pred;

	// Find the last port that exists and that isn't marked no-switch
	while ((!s->visual->port_refs[id] && id >= 0) || (s->visual->port_refs[id]->flags & GFXW_FLAG_NO_IMPLICIT_SWITCH))
		id--;

	sciprintf("Activating port %d after disposing window %d\n", id, goner_nr);
	s->port = s->visual->port_refs[id];

	if (!s->port)
		s->port = gfxw_find_default_port(s->visual);

	gfxop_update(s->gfx_state);

	return s->r_acc;
}

reg_t kNewWindow(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	gfxw_port_t *window;
	int x, y, xl, yl, flags;
	gfx_color_t bgcolor;
	gfx_color_t fgcolor;
	gfx_color_t black;
	gfx_color_t lWhite;
	int priority;
	int argextra = argc == 13 ? 4 : 0; // Triggers in PQ3

	y = SKPV(0);
	x = SKPV(1);
	yl = SKPV(2) - y;
	xl = SKPV(3) - x;

	y += s->wm_port->bounds.y;

	if (x + xl > 319)
		x -= ((x + xl) - 319);

	flags = SKPV(5 + argextra);

	priority = SKPV_OR_ALT(6 + argextra, -1);
	bgcolor.mask = 0;

	if (SKPV_OR_ALT(8 + argextra, 255) >= 0) {
		if (s->resmgr->sci_version < SCI_VERSION_01_VGA)
			bgcolor.visual = *(get_pic_color(s, SKPV_OR_ALT(8 + argextra, 15)));
		else
			bgcolor.visual = *(get_pic_color(s, SKPV_OR_ALT(8 + argextra, 255)));
		bgcolor.mask = GFX_MASK_VISUAL;
	}

	bgcolor.priority = priority;
	bgcolor.mask |= priority >= 0 ? GFX_MASK_PRIORITY : 0;
	bgcolor.alpha = 0;
	SCIkdebug(SCIkGRAPHICS, "New window with params %d, %d, %d, %d\n", SKPV(0), SKPV(1), SKPV(2), SKPV(3));

	fgcolor.visual = *(get_pic_color(s, SKPV_OR_ALT(7 + argextra, 0)));
	fgcolor.mask = GFX_MASK_VISUAL;
	fgcolor.alpha = 0;
	black.visual = *(get_pic_color(s, 0));
	black.mask = GFX_MASK_VISUAL;
	black.alpha = 0;
	lWhite.visual = *(get_pic_color(s, s->resmgr->sci_version < SCI_VERSION_01_VGA ? 15 : 255)), lWhite.mask = GFX_MASK_VISUAL;
	lWhite.alpha = 0;

	window = sciw_new_window(s, gfx_rect(x, y, xl, yl), s->titlebar_port->font_nr, fgcolor, bgcolor,
							s->titlebar_port->font_nr, lWhite, black, argv[4 + argextra].segment ? 
							kernel_dereference_char_pointer(s, argv[4 + argextra], 0) : NULL, flags);

	// PQ3 has the interpreter store underBits implicitly.
	// The feature was promptly removed after its release, never to be seen again.
	if (argextra)
		gfxw_port_auto_restore_background(s->visual, window, gfx_rect(SKPV(5), SKPV(4), SKPV(7) - SKPV(5), SKPV(6) - SKPV(4)));

	ADD_TO_WINDOW_PORT(window);
	FULL_REDRAW();

	window->draw(GFXW(window), gfxw_point_zero);
	gfxop_update(s->gfx_state);

	s->port = window; // Set active port

	return make_reg(0, window->ID);
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
#define K_ANIMATE_SCROLL_LEFT 0x28
#define K_ANIMATE_SCROLL_RIGHT 0x29
#define K_ANIMATE_SCROLL_DOWN 0x2a
#define K_ANIMATE_SCROLL_UP 0x2b

#define K_ANIMATE_OPEN_SIMPLE 100 // No animation

#define GRAPH_BLANK_BOX(s, x, y, xl, yl, color) GFX_ASSERT(gfxop_fill_box(s->gfx_state, \
             gfx_rect(x, (((y) < 10)? 10 : (y)), xl, (((y) < 10)? ((y) - 10) : 0) + (yl)), s->ega_colors[color]));

#define GRAPH_UPDATE_BOX(s, x, y, xl, yl) GFX_ASSERT(gfxop_draw_pixmap(s->gfx_state, newscreen, \
             gfx_rect(x, (((y) < 10)? 10 : (y)) - 10, xl, (((y) < 10)? ((y) - 10) : 0) + (yl)), Common::Point(x, ((y) < 10)? 10 : (y) )));

static void animate_do_animation(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int i, remaining_checkers;
	int update_counter;
	int granularity0 = s->animation_granularity << 1;
	int granularity1 = s->animation_granularity;
	int granularity2 = s->animation_granularity >> 2;
	int granularity3 = s->animation_granularity >> 4;
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

	GFX_ASSERT(gfxop_draw_pixmap(s->gfx_state, s->old_screen, gfx_rect(0, 0, 320, 190), Common::Point(0, 10)));
	gfxop_update_box(s->gfx_state, gfx_rect(0, 0, 320, 200));

	//SCIkdebug(SCIkGRAPHICS, "Animating pic opening type %x\n", s->pic_animate);

	gfxop_enable_dirty_frames(s->gfx_state);

	if (s->animation_delay < 1)
		s->pic_animate = K_ANIMATE_OPEN_SIMPLE;

	switch (s->pic_animate) {
	case K_ANIMATE_BORDER_CLOSE_H_CENTER_OPEN_H :
		for (i = 0; i < 159 + granularity1; i += granularity1) {
			GRAPH_BLANK_BOX(s, i, 10, granularity1, 190, 0);
			gfxop_update(s->gfx_state);
			GRAPH_BLANK_BOX(s, 319 - i, 10, granularity1, 190, 0);
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, s->animation_delay);
			process_sound_events(s);
		}
		GRAPH_BLANK_BOX(s, 0, 10, 320, 190, 0);

	case K_ANIMATE_CENTER_OPEN_H :

		for (i = 159; i >= 1 - granularity1; i -= granularity1) {
			GRAPH_UPDATE_BOX(s, i, 10, granularity1, 190);
			gfxop_update(s->gfx_state);
			GRAPH_UPDATE_BOX(s, 319 - i, 10, granularity1, 190);
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, s->animation_delay);
			process_sound_events(s);
		}
		break;


	case K_ANIMATE_BORDER_CLOSE_V_CENTER_OPEN_V :

		for (i = 0; i < 94 + granularity2; i += granularity2) {
			GRAPH_BLANK_BOX(s, 0, i + 10, 320, granularity2, 0);
			gfxop_update(s->gfx_state);
			GRAPH_BLANK_BOX(s, 0, 199 - i, 320, granularity2, 0);
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, 2 * s->animation_delay);
			process_sound_events(s);
		}
		GRAPH_BLANK_BOX(s, 0, 10, 320, 190, 0);

	case K_ANIMATE_CENTER_OPEN_V :

		for (i = 94; i >= 1 - granularity2; i -= granularity2) {
			GRAPH_UPDATE_BOX(s, 0, i + 10, 320, granularity2);
			gfxop_update(s->gfx_state);
			GRAPH_UPDATE_BOX(s, 0, 199 - i, 320, granularity2);
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, 2 * s->animation_delay);
			process_sound_events(s);
		}
		break;


	case K_ANIMATE_LEFT_CLOSE_RIGHT_OPEN :

		for (i = 0; i < 319 + granularity0; i += granularity0) {
			GRAPH_BLANK_BOX(s, i, 10, granularity0, 190, 0);
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, s->animation_delay / 2);
			process_sound_events(s);
		}
		GRAPH_BLANK_BOX(s, 0, 10, 320, 190, 0);

	case K_ANIMATE_RIGHT_OPEN :
		for (i = 319; i >= 1 - granularity0; i -= granularity0) {
			GRAPH_UPDATE_BOX(s, i, 10, granularity0, 190);
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, s->animation_delay / 2);
			process_sound_events(s);
		}
		break;


	case K_ANIMATE_RIGHT_CLOSE_LEFT_OPEN :

		for (i = 319; i >= 1 - granularity0; i -= granularity0) {
			GRAPH_BLANK_BOX(s, i, 10, granularity0, 190, 0);
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, s->animation_delay / 2);
			process_sound_events(s);
		}
		GRAPH_BLANK_BOX(s, 0, 10, 320, 190, 0);

	case K_ANIMATE_LEFT_OPEN :

		for (i = 0; i < 319 + granularity0; i += granularity0) {
			GRAPH_UPDATE_BOX(s, i, 10, granularity0, 190);
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, s->animation_delay / 2);
			process_sound_events(s);
		}
		break;


	case K_ANIMATE_TOP_CLOSE_BOTTOM_OPEN :

		for (i = 10; i < 199 + granularity1; i += granularity1) {
			GRAPH_BLANK_BOX(s, 0, i, 320, granularity1, 0);
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, s->animation_delay);
			process_sound_events(s);
		}
		GRAPH_BLANK_BOX(s, 0, 10, 320, 190, 0);

	case K_ANIMATE_BOTTOM_OPEN :

		for (i = 199; i >= 11 - granularity1; i -= granularity1) {
			GRAPH_UPDATE_BOX(s, 0, i, 320, granularity1);
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, s->animation_delay);
			process_sound_events(s);
		}
		break;


	case K_ANIMATE_BOTTOM_CLOSE_TOP_OPEN :

		for (i = 199; i >= 11 - granularity1; i -= granularity1) {
			GRAPH_BLANK_BOX(s, 0, i, 320, granularity1, 0);
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, s->animation_delay);
			process_sound_events(s);
		}
		GRAPH_BLANK_BOX(s, 0, 10, 320, 190, 0);

	case K_ANIMATE_TOP_OPEN :

		for (i = 10; i < 199 + granularity1; i += granularity1) {
			GRAPH_UPDATE_BOX(s, 0, i, 320, granularity1);
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, s->animation_delay);
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

			gfxop_usleep(s->gfx_state, 4 * s->animation_delay);
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

			gfxop_usleep(s->gfx_state, 4 * s->animation_delay);
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

			gfxop_usleep(s->gfx_state, 7 * s->animation_delay);
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

			gfxop_usleep(s->gfx_state, 7 * s->animation_delay);
			process_sound_events(s);
		}
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
				gfxop_usleep(s->gfx_state, s->animation_delay / 4);
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
				gfxop_usleep(s->gfx_state, s->animation_delay / 4);
			}

			--remaining_checkers;
			process_sound_events(s);
		}
		break;


	case K_ANIMATE_SCROLL_LEFT :

		for (i = 0; i < 319; i += granularity0) {
			GFX_ASSERT(gfxop_draw_pixmap(s->gfx_state, newscreen, gfx_rect(320 - i, 0, i, 190), Common::Point(0, 10)));
			GFX_ASSERT(gfxop_draw_pixmap(s->gfx_state, s->old_screen, gfx_rect(0, 0, 320 - i, 190), Common::Point(i, 10)));
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, s->animation_delay >> 3);
		}
		GRAPH_UPDATE_BOX(s, 0, 10, 320, 190);
		break;

	case K_ANIMATE_SCROLL_RIGHT :

		for (i = 0; i < 319; i += granularity0) {
			GFX_ASSERT(gfxop_draw_pixmap(s->gfx_state, newscreen, gfx_rect(0, 0, i, 190), Common::Point(319 - i, 10)));
			GFX_ASSERT(gfxop_draw_pixmap(s->gfx_state, s->old_screen, gfx_rect(i, 0, 320 - i, 190), Common::Point(0, 10)));
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, s->animation_delay >> 3);
		}
		GRAPH_UPDATE_BOX(s, 0, 10, 320, 190);
		break;

	case K_ANIMATE_SCROLL_UP :

		for (i = 0; i < 189; i += granularity0) {
			GFX_ASSERT(gfxop_draw_pixmap(s->gfx_state, newscreen, gfx_rect(0, 190 - i, 320, i), Common::Point(0, 10)));
			GFX_ASSERT(gfxop_draw_pixmap(s->gfx_state, s->old_screen, gfx_rect(0, 0, 320, 190 - i), Common::Point(0, 10 + i)));
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, s->animation_delay >> 3);
		}
		GRAPH_UPDATE_BOX(s, 0, 10, 320, 190);
		break;

	case K_ANIMATE_SCROLL_DOWN :

		for (i = 0; i < 189; i += granularity0) {
			GFX_ASSERT(gfxop_draw_pixmap(s->gfx_state, newscreen, gfx_rect(0, 0, 320, i), Common::Point(0, 200 - i)));
			GFX_ASSERT(gfxop_draw_pixmap(s->gfx_state, s->old_screen, gfx_rect(0, i, 320, 190 - i), Common::Point(0, 10)));
			gfxop_update(s->gfx_state);
			gfxop_usleep(s->gfx_state, s->animation_delay >> 3);
		}
		GRAPH_UPDATE_BOX(s, 0, 10, 320, 190);
		break;

	default:
		if (s->pic_animate != K_ANIMATE_OPEN_SIMPLE)
			warning("Unknown opening animation 0x%02x", s->pic_animate);
		GRAPH_UPDATE_BOX(s, 0, 10, 320, 190);

	}

	GFX_ASSERT(gfxop_free_pixmap(s->gfx_state, s->old_screen));
	GFX_ASSERT(gfxop_free_pixmap(s->gfx_state, newscreen));
	s->old_screen = NULL;
}

reg_t kAnimate(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	// Animations are supposed to take a maximum of s->animation_delay milliseconds.
	reg_t cast_list_ref = KP_ALT(0, NULL_REG);
	int cycle = (KP_ALT(1, NULL_REG)).offset;
	list_t *cast_list = NULL;
	int open_animation = 0;

	process_sound_events(s); // Take care of incoming events (kAnimate is called semi-regularly)
	_k_animate_ran = 1; // Used by some of the invoked functions to check for recursion, which may,
						// after all, damage the cast list

	if (cast_list_ref.segment) {
		cast_list = LOOKUP_LIST(cast_list_ref);
		if (!cast_list)
			return s->r_acc;
	}

	open_animation = (s->pic_is_new) && (s->pic_not_valid);
	s->pic_is_new = 0;

	assert_primary_widget_lists(s);

	if (!s->dyn_views->contents // Only reparentize empty dynview list
	        && ((GFXWC(s->port) != GFXWC(s->dyn_views->parent)) // If dynviews are on other port...
	            || (s->dyn_views->next))) // ... or not on top of the view list
		reparentize_primary_widget_lists(s, s->port);

	if (cast_list) {
		gfxw_list_t *templist = gfxw_new_list(s->dyn_views->bounds, 0);

		_k_make_view_list(s, &(templist), cast_list, (cycle ? _K_MAKE_VIEW_LIST_CYCLE : 0)
		                  | _K_MAKE_VIEW_LIST_CALC_PRIORITY, funct_nr, argc, argv);

		// Make sure that none of the doits() did something evil
		assert_primary_widget_lists(s);

		if (!s->dyn_views->contents // Only reparentize empty dynview list
		        && ((GFXWC(s->port) != GFXWC(s->dyn_views->parent)) // If dynviews are on other port...
		            || (s->dyn_views->next))) // ... or not on top of the view list
			reparentize_primary_widget_lists(s, s->port);
		// End of doit() recovery code

		if (s->pic_is_new) { // Happens if DrawPic() is executed by a dynview (yes, that happens)
			kAnimate(s, funct_nr, argc, argv); /* Tail-recurse */
			return s->r_acc;
		}

		SCIkdebug(SCIkGRAPHICS, "Handling Dynviews (..step 9 inclusive):\n");
		_k_prepare_view_list(s, templist, _K_MAKE_VIEW_LIST_CALC_PRIORITY);

		if (s->pic_not_valid) {
			SCIkdebug(SCIkGRAPHICS, "PicNotValid=%d -> Subalgorithm:\n");
			_k_redraw_view_list(s, templist);
		}

		_k_update_signals_in_view_list(s->dyn_views, templist);
		s->dyn_views->tag(GFXW(s->dyn_views));

		_k_raise_topmost_in_view_list(s, s->dyn_views, (gfxw_dyn_view_t *)templist->contents);

		templist->widfree(GFXW(templist));
		s->dyn_views->free_tagged(GFXWC(s->dyn_views)); // Free obsolete dynviews
	} // if (cast_list)

	if (open_animation) {
		gfxop_clear_box(s->gfx_state, gfx_rect(0, 10, 320, 190)); // Propagate pic
		s->visual->add_dirty_abs(GFXWC(s->visual), gfx_rect_fullscreen, 0);
		// Mark screen as dirty so picviews will be drawn correctly
		FULL_REDRAW();

		animate_do_animation(s, funct_nr, argc, argv);
	} // if (open_animation)

	if (cast_list) {
		int retval;
		int reparentize = 0;

		s->pic_not_valid = 0;

		_k_view_list_do_postdraw(s, s->dyn_views);

		// _k_view_list_dispose_loop() returns -1 if it requested a re-start, so we do just that.
		while ((retval = _k_view_list_dispose_loop(s, cast_list, (gfxw_dyn_view_t *) s->dyn_views->contents, funct_nr, argc, argv) < 0))
			reparentize = 1;

		if (s->drop_views->contents) {
			s->drop_views = gfxw_new_list(s->dyn_views->bounds, GFXW_LIST_SORTED);
			s->drop_views->flags |= GFXW_FLAG_IMMUNE_TO_SNAPSHOTS;
			ADD_TO_CURRENT_PICTURE_PORT(s->drop_views);
		} else {
			assert(s->drop_views);
			gfxw_remove_widget_from_container(s->drop_views->parent, GFXW(s->drop_views));
			ADD_TO_CURRENT_PICTURE_PORT(s->drop_views);
		}

		if ((reparentize | retval)
		        && (GFXWC(s->port) == GFXWC(s->dyn_views->parent)) // If dynviews are on the same port...
		        && (s->dyn_views->next)) // ... and not on top of the view list...
			reparentize_primary_widget_lists(s, s->port); // ...then reparentize.

		_k_view_list_kryptonize(s->dyn_views->contents);
	}

	FULL_REDRAW();
	return s->r_acc;
}

#define SHAKE_DOWN 1
#define SHAKE_RIGHT 2

reg_t kShakeScreen(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int shakes = SKPV_OR_ALT(0, 1);
	int directions = SKPV_OR_ALT(1, 1);
	gfx_pixmap_t *screen = gfxop_grab_pixmap(s->gfx_state, gfx_rect(0, 0, 320, 200));
	int i;

	if (directions & ~3)
		SCIkdebug(SCIkGRAPHICS, "ShakeScreen(): Direction bits are %x (unknown)\n", directions);

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
		gfxop_usleep(s->gfx_state, 50000);

		gfxop_draw_pixmap(s->gfx_state, screen, gfx_rect(0, 0, 320, 200), Common::Point(0, 0));
		gfxop_update(s->gfx_state);
		gfxop_usleep(s->gfx_state, 50000);
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

reg_t kDisplay(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int argpt;
	reg_t textp = argv[0];
	int index = UKPV_OR_ALT(1, 0);
	int temp;
	int save_under = 0;
	gfx_color_t transparent = { { 0, 0, 0, 0 }, 0, 0, 0, 0 };
	char *text;
	gfxw_port_t *port = (s->port) ? s->port : s->picture_port;
	int update_immediately = 1;

	gfx_color_t color0, *color1, bg_color;
	gfx_alignment_t halign = ALIGN_LEFT;
	rect_t area = gfx_rect(port->draw_pos.x, port->draw_pos.y, 320 - port->draw_pos.x, 200 - port->draw_pos.y);
	int gray = port->gray_text;
	int font_nr = port->font_nr;
	gfxw_text_t *text_handle;

	color0 = port->color;
	bg_color = port->bgcolor;

	if (textp.segment) {
		argpt = 1;
		text = (char *)kernel_dereference_bulk_pointer(s, textp, 0);
	} else {
		argpt = 2;
		text = kernel_lookup_text(s, textp, index);
	}

	if (!text) {
		error("Display with invalid reference "PREG"", PRINT_REG(textp));
		return NULL_REG;
	}

	while (argpt < argc) {
		switch (UKPV(argpt++)) {

		case K_DISPLAY_SET_COORDS:

			area.x = UKPV(argpt++);
			area.y = UKPV(argpt++);
			SCIkdebug(SCIkGRAPHICS, "Display: set_coords(%d, %d)\n", area.x, area.y);
			break;

		case K_DISPLAY_SET_ALIGNMENT:

			halign = (gfx_alignment_t)KP_SINT(argv[argpt++]);
			SCIkdebug(SCIkGRAPHICS, "Display: set_align(%d)\n", halign);
			break;

		case K_DISPLAY_SET_COLOR:

			temp = KP_SINT(argv[argpt++]);
			SCIkdebug(SCIkGRAPHICS, "Display: set_color(%d)\n", temp);
			if ((s->resmgr->sci_version < SCI_VERSION_01_VGA) && temp >= 0 && temp <= 15)
				color0 = (s->ega_colors[temp]);
			else
				if ((s->resmgr->sci_version >= SCI_VERSION_01_VGA) && temp >= 0 && temp < 256) {
					color0.visual = *(get_pic_color(s, temp));
					color0.mask = GFX_MASK_VISUAL;
				} else
					if (temp == -1)
						color0 = transparent;
					else
						warning("Display: Attempt to set invalid fg color %d", temp);
			break;

		case K_DISPLAY_SET_BGCOLOR:

			temp = KP_SINT(argv[argpt++]);
			SCIkdebug(SCIkGRAPHICS, "Display: set_bg_color(%d)\n", temp);
			if ((s->resmgr->sci_version < SCI_VERSION_01_VGA) && temp >= 0 && temp <= 15)
				bg_color = s->ega_colors[temp];
			else
				if ((s->resmgr->sci_version >= SCI_VERSION_01_VGA) && temp >= 0 && temp <= 256) {
					bg_color.visual = *get_pic_color(s, temp);
					bg_color.mask = GFX_MASK_VISUAL;
				} else
					if (temp == -1)
						bg_color = transparent;
					else
						warning("Display: Attempt to set invalid fg color %d", temp);
			break;

		case K_DISPLAY_SET_GRAYTEXT:

			gray = KP_SINT(argv[argpt++]);
			SCIkdebug(SCIkGRAPHICS, "Display: set_graytext(%d)\n", gray);
			break;

		case K_DISPLAY_SET_FONT:

			font_nr = KP_UINT(argv[argpt++]);

			SCIkdebug(SCIkGRAPHICS, "Display: set_font(\"font.%03d\")\n", font_nr);
			break;

		case K_DISPLAY_WIDTH:

			area.xl = UKPV(argpt++);
			if (area.xl == 0)
				area.xl = MAX_TEXT_WIDTH_MAGIC_VALUE;

			SCIkdebug(SCIkGRAPHICS, "Display: set_width(%d)\n", area.xl);
			break;

		case K_DISPLAY_SAVE_UNDER:

			save_under = 1;
			SCIkdebug(SCIkGRAPHICS, "Display: set_save_under()\n");
			break;

		case K_DISPLAY_RESTORE_UNDER:

			SCIkdebug(SCIkGRAPHICS, "Display: restore_under(%04x)\n", UKPV(argpt));
			graph_restore_box(s, argv[argpt++]);
			update_immediately = 1;
			argpt++;
			return s->r_acc;

		case K_DONT_UPDATE_IMMEDIATELY:

			update_immediately = 0;
			SCIkdebug(SCIkGRAPHICS, "Display: set_dont_update()\n");
			argpt++;
			break;

		default:
			SCIkdebug(SCIkGRAPHICS, "Unknown Display() command %x\n", UKPV(argpt - 1));
			return NULL_REG;
		}
	}

	if (s->version >= SCI_VERSION_FTU_DISPLAY_COORDS_FUZZY) {
		if (halign == ALIGN_LEFT)
			GFX_ASSERT(gfxop_get_text_params(s->gfx_state, font_nr, text, area.xl, &area.xl, &area.yl, 0, NULL, NULL, NULL));

		// Make the text fit on the screen
		if (area.x + area.xl > 320)
			area.x += 320 - area.x - area.xl; // Plus negative number = subtraction

		if (area.y + area.yl > 200) {
			area.y += 200 - area.y - area.yl; // Plus negative number = subtraction
		}
	}

	if (gray)
		color1 = &bg_color;
	else
		color1 = &color0;

	assert_primary_widget_lists(s);

	text_handle = gfxw_new_text(s->gfx_state, area, font_nr, text, halign, ALIGN_TOP, color0, *color1, bg_color, 0);

	if (!text_handle) {
		error("Display: Failed to create text widget");
		return NULL_REG;
	}

	if (save_under) {    // Backup
		rect_t save_area = text_handle->bounds;
		save_area.x += port->bounds.x;
		save_area.y += port->bounds.y;

		s->r_acc = graph_save_box(s, save_area);
		text_handle->serial++; // This is evil!

		SCIkdebug(SCIkGRAPHICS, "Saving (%d, %d) size (%d, %d) as "PREG"\n", save_area.x, save_area.y, save_area.xl, save_area.yl, s->r_acc);
	}

	SCIkdebug(SCIkGRAPHICS, "Display: Commiting text '%s'\n", text);

	//ADD_TO_CURRENT_FG_WIDGETS(text_handle);

	ADD_TO_CURRENT_FG_WIDGETS(GFXW(text_handle));
	if ((!s->pic_not_valid) && update_immediately) { // Refresh if drawn to valid picture
		FULL_REDRAW();
		SCIkdebug(SCIkGRAPHICS, "Refreshing display...\n");
	}

	return s->r_acc;
}

} // End of namespace Sci
