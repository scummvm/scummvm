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

#include "sci/sci_memory.h"
#include "sci/gfx/gfx_widgets.h"
#include "sci/gfx/gfx_state_internal.h"

namespace Sci {

#undef GFXW_DEBUG_DIRTY // Enable to debug dirty rectangle propagation (writes to stderr)

#ifdef GFXW_DEBUG_DIRTY
#  define DDIRTY fprintf(stderr, "%s:%5d| ", __FILE__, __LINE__); fprintf
#else
#  define DDIRTY if (0) fprintf
#endif

Common::Point gfxw_point_zero(0, 0);

#define MAX_SERIAL_NUMBER 0x7fffffff
static int widget_serial_number_counter = 0x10000; // Avoid confusion with IDs

#ifdef GFXW_DEBUG_WIDGETS

gfxw_widget_t *debug_widgets[GFXW_DEBUG_WIDGETS];
int debug_widget_pos = 0;

static void _gfxw_debug_add_widget(gfxw_widget_t *widget) {
	if (debug_widget_pos == GFXW_DEBUG_WIDGETS) {
		GFXERROR("WIDGET DEBUG: Allocated the maximum number of %d widgets- Aborting!\n", GFXW_DEBUG_WIDGETS);
		BREAKPOINT();
	}
	debug_widgets[debug_widget_pos++] = widget;
}

static void _gfxw_debug_remove_widget(gfxw_widget_t *widget) {
	int i;
	int found = 0;
	for (i = 0; i < debug_widget_pos; i++) {
		if (debug_widgets[i] == widget) {
			memmove(debug_widgets + i, debug_widgets + i + 1, (sizeof(gfxw_widget_t *)) * (debug_widget_pos - i - 1));
			debug_widgets[debug_widget_pos--] = NULL;
			found++;
		}
	}

	if (found > 1) {
		GFXERROR("While removing widget: Found it %d times!\n", found);
		BREAKPOINT();
	}

	if (found == 0) {
		GFXERROR("Attempted removal of unregistered widget!\n");
		BREAKPOINT();
	}
}
#else // !GFXW_DEBUG_WIDGETS
#define _gfxw_debug_add_widget(a)
#define _gfxw_debug_remove_widget(a)
#endif


static void indent(int indentation) {
	for (int i = 0; i < indentation; i++)
		sciprintf("    ");
}

static void _gfxw_print_widget(gfxw_widget_t *widget, int indentation) {
	unsigned int i;
	char flags_list[] = "VOCDTMI";

	indent(indentation);

	if (widget->magic == GFXW_MAGIC_VALID) {
		if (widget->visual)
			sciprintf("v ");
		else
			sciprintf("NoVis ");
	} else if (widget->magic == GFXW_MAGIC_INVALID)
		sciprintf("INVALID ");

	sciprintf("S%08x", widget->serial);

	if (widget->ID != GFXW_NO_ID) {
		sciprintf("#%x", widget->ID);

		if (widget->subID != GFXW_NO_ID)
			sciprintf(":%x ", widget->subID);
		else
			sciprintf(" ");
	}

	sciprintf("[(%d,%d)(%dx%d)]", widget->bounds.x, widget->bounds.y, widget->bounds.width, widget->bounds.height);

	for (i = 0; i < strlen(flags_list); i++)
		if (widget->flags & (1 << i))
			sciprintf("%c", flags_list[i]);

	sciprintf(" ");
}

static int _gfxwop_print_empty(gfxw_widget_t *widget, int indentation) {
	_gfxw_print_widget(widget, indentation);
	sciprintf("<untyped #%d>", widget->type);

	return 0;
}

gfxw_widget_t *_gfxw_new_widget(int size, gfxw_widget_type_t type) {
	gfxw_widget_t *widget = (gfxw_widget_t*)sci_malloc(size);
#ifdef SATISFY_PURIFY
	memset(widget, 0, size);
#endif

	widget->magic = GFXW_MAGIC_VALID;
	widget->parent = NULL;
	widget->visual = NULL;
	widget->next = NULL;
	widget->type = type;
	widget->bounds = gfx_rect(0, 0, 0, 0);
	widget->flags = GFXW_FLAG_DIRTY;
	widget->ID = GFXW_NO_ID;
	widget->subID = GFXW_NO_ID;
	widget->serial = widget_serial_number_counter++;
	widget->widget_priority = -1;

	widget_serial_number_counter &= MAX_SERIAL_NUMBER;

	widget->draw = NULL;
	widget->widfree = NULL;
	widget->tag = NULL;
	widget->print = _gfxwop_print_empty;
	widget->should_replace = NULL;
	widget->compare_to = widget->equals = widget->superarea_of = NULL;

	_gfxw_debug_add_widget(widget);

	return widget;
}

static int verify_widget(gfxw_widget_t *widget) {
	if (!widget) {
		GFXERROR("Attempt to use NULL widget\n");
#ifdef GFXW_DEBUG_WIDGETS
		BREAKPOINT();
#endif
		return 1;
	} else if (widget->magic != GFXW_MAGIC_VALID) {
		if (widget->magic == GFXW_MAGIC_INVALID) {
			GFXERROR("Attempt to use invalidated widget\n");
		} else {
			GFXERROR("Attempt to use non-widget\n");
		}
#ifdef GFXW_DEBUG_WIDGETS
		BREAKPOINT();
#endif
		return 1;
	}
	return 0;
}

#define VERIFY_WIDGET(w) \
	if (verify_widget((gfxw_widget_t *)(w))) { GFXERROR("Error occured while validating widget\n"); }

static void _gfxw_unallocate_widget(gfx_state_t *state, gfxw_widget_t *widget) {
	if (GFXW_IS_TEXT(widget)) {
		gfxw_text_t *text = (gfxw_text_t *) widget;

		if (text->text_handle) {
			if (!state) {
				GFXERROR("Attempt to free text without supplying mode to free it from!\n");
				BREAKPOINT();
			} else {
				gfxop_free_text(state, text->text_handle);
				text->text_handle = NULL;
			}
		}
	}

	widget->magic = GFXW_MAGIC_INVALID;
	free(widget);
	_gfxw_debug_remove_widget(widget);
}

#define GFX_ASSERT(_x) \
{ \
	int retval = (_x); \
	if (retval == GFX_ERROR) { \
		GFXERROR("Error occured while drawing widget!\n"); \
		return 1; \
	} else if (retval == GFX_FATAL) { \
		error("Fatal error occured while drawing widget!\nGraphics state invalid; aborting program..."); \
	} \
}

//********** Widgets *************

// Base class operations and common stuff

// Assertion for drawing
#define DRAW_ASSERT(widget, exp_type) \
	if (!(widget)) { \
		sciprintf("L%d: NULL widget", __LINE__); \
		return 1; \
	} \
	if (!(widget)->print) { \
		sciprintf("L%d: Widget of type %d does not have print function", __LINE__, (widget)->type); \
	} \
	if ((widget)->type != (exp_type)) { \
		sciprintf("L%d: Error in widget: Expected type " # exp_type "(%d) but got %d\n", __LINE__, exp_type, (widget)->type); \
		sciprintf("Erroneous widget: "); \
		widget->print(widget, 4); \
		sciprintf("\n"); \
		return 1; \
	} \
	if (!(widget->flags & GFXW_FLAG_VISIBLE)) \
		return 0; \
	if (!(widget->type == GFXW_VISUAL || widget->visual)) { \
		sciprintf("L%d: Error while drawing widget: Widget has no visual\n", __LINE__); \
		sciprintf("Erroneous widget: "); \
		widget->print(widget, 1); \
		sciprintf("\n"); \
		return 1; \
	}

static int _color_equals(gfx_color_t a, gfx_color_t b) {
	if (a.mask != b.mask)
		return 0;

	if (a.mask & GFX_MASK_VISUAL) {
		if (a.visual.r != b.visual.r || a.visual.g != b.visual.g || a.visual.b != b.visual.b || a.alpha != b.alpha)
			return 0;
	}

	if (a.mask & GFX_MASK_PRIORITY)
		if (a.priority != b.priority)
			return 0;

	if (a.mask & GFX_MASK_CONTROL)
		if (a.control != b.control)
			return 0;

	return 1;
}

static int _gfxwop_basic_set_visual(gfxw_widget_t *widget, gfxw_visual_t *visual) {
	widget->visual = visual;

	if (widget->parent) {
		DDIRTY(stderr, "basic_set_visual: DOWNWARDS rel(%d,%d,%d,%d, 1)\n", GFX_PRINT_RECT(widget->bounds));
		widget->parent->add_dirty_rel(widget->parent, widget->bounds, 1);
	}

	return 0;
}

static int _gfxwop_basic_should_replace(gfxw_widget_t *widget, gfxw_widget_t *other) {
	return 0;
}

static void _gfxw_set_ops(gfxw_widget_t *widget, gfxw_point_op *draw, gfxw_op *free, gfxw_op *tag, gfxw_op_int *print,
	gfxw_bin_op *compare_to, gfxw_bin_op *equals, gfxw_bin_op *superarea_of) {
	widget->draw = draw;
	widget->widfree = free;
	widget->tag = tag;
	widget->print = print;
	widget->compare_to = compare_to;
	widget->equals = equals;
	widget->superarea_of = superarea_of;

	widget->should_replace = _gfxwop_basic_should_replace;
	widget->set_visual = _gfxwop_basic_set_visual;
}

void gfxw_remove_widget_from_container(gfxw_container_t *container, gfxw_widget_t *widget) {
	gfxw_widget_t **seekerp;

	if (!container) {
		GFXERROR("Attempt to remove widget from NULL container!\n");
		BREAKPOINT();
	}

	seekerp = &(container->contents);

	if (GFXW_IS_LIST(widget) && GFXW_IS_PORT(container)) {
		gfxw_port_t *port = (gfxw_port_t *) container;
		if (port->decorations == (gfxw_list_t *) widget) {
			port->decorations = NULL;
			return;
		}
	}

	while (*seekerp && *seekerp != widget)
		seekerp = &((*seekerp)->next);

	if (!*seekerp) {
		GFXERROR("Internal error: Attempt to remove widget from container it was not contained in!\n");
		sciprintf("Widget:");
		widget->print(GFXW(widget), 1);
		sciprintf("Container:");
		widget->print(GFXW(container), 1);
		BREAKPOINT();
		return;
	}

	if (container->nextpp == &(widget->next))
		container->nextpp = seekerp;

	*seekerp = widget->next; // Remove it
	widget->parent = NULL;
	widget->next = NULL;
}

static int _gfxwop_basic_free(gfxw_widget_t *widget) {
	gfxw_visual_t *visual = widget->visual;
	gfx_state_t *state = (visual) ? visual->gfx_state : NULL;

	DDIRTY(stderr, "BASIC-FREE: SomeAddDirty\n");

	if (widget->parent) {
		if (GFXW_IS_CONTAINER(widget))
			widget->parent->add_dirty_abs(widget->parent, widget->bounds, 1);
		else
			widget->parent->add_dirty_rel(widget->parent, widget->bounds, 1);

		gfxw_remove_widget_from_container(widget->parent, widget);
	}

	_gfxw_unallocate_widget(state, widget);

	return 0;
}

static int _gfxwop_basic_tag(gfxw_widget_t *widget) {
	widget->flags |= GFXW_FLAG_TAGGED;

	return 0;
}

static int _gfxwop_basic_compare_to(gfxw_widget_t *widget, gfxw_widget_t *other) {
	return 1;
}

static int _gfxwop_basic_equals(gfxw_widget_t *widget, gfxw_widget_t *other) {
	return 0;
}

static int _gfxwop_basic_superarea_of(gfxw_widget_t *widget, gfxw_widget_t *other) {
	return (widget == other);
}

//*** Boxes ***

static rect_t _move_rect(rect_t rect, Common::Point point) {
	return gfx_rect(rect.x + point.x, rect.y + point.y, rect.width, rect.height);
}

static void _split_rect(rect_t rect, Common::Point *p1, Common::Point *p2) {
	p1->x = rect.x;
	p1->y = rect.y;
	p2->x = rect.x + rect.width;
	p2->y = rect.y + rect.height;
}

static Common::Point _move_point(rect_t rect, Common::Point point) {
	return Common::Point(rect.x + point.x, rect.y + point.y);
}

static int _gfxwop_box_draw(gfxw_widget_t *widget, Common::Point pos) {
	gfxw_box_t *box = (gfxw_box_t *) widget;
	DRAW_ASSERT(widget, GFXW_BOX);
	GFX_ASSERT(gfxop_draw_box(box->visual->gfx_state, _move_rect(box->bounds, pos), box->color1, box->color2, box->shade_type));

	return 0;
}

static int _gfxwop_box_print(gfxw_widget_t *widget, int indentation) {
	_gfxw_print_widget(widget, indentation);
	sciprintf("BOX");
	return 0;
}

static int _gfxwop_box_superarea_of(gfxw_widget_t *widget, gfxw_widget_t *other) {
	gfxw_box_t *box = (gfxw_box_t *) widget;

	if (box->color1.alpha)
		return 0;

	if (box->shade_type != GFX_BOX_SHADE_FLAT && box->color2.alpha)
		return 0;

	// Note: the check for box->bounds and other->bounds is NOT the same as contains()
	// in Common::Rect (this one includes equality too)
	if (!(box->bounds.x <= other->bounds.x && box->bounds.y <= other->bounds.y &&
		  box->bounds.x + box->bounds.width >= other->bounds.x + other->bounds.width && 
		  box->bounds.y + box->bounds.height >= other->bounds.y + other->bounds.height))
		return 0;

	return 1;
}

static int _gfxwop_box_equals(gfxw_widget_t *widget, gfxw_widget_t *other) {
	gfxw_box_t *wbox = (gfxw_box_t *)widget, *obox;
	if (other->type != GFXW_BOX)
		return 0;

	obox = (gfxw_box_t *) other;

	if (!toCommonRect(wbox->bounds).equals(toCommonRect(obox->bounds)))
		return 0;

	if (!_color_equals(wbox->color1, obox->color1))
		return 0;

	if (wbox->shade_type != obox->shade_type)
		return 0;

	if (wbox->shade_type != GFX_BOX_SHADE_FLAT
	        && _color_equals(wbox->color2, obox->color2))
		return 0;

	return 1;
}

void _gfxw_set_ops_BOX(gfxw_widget_t *widget) {
	_gfxw_set_ops(GFXW(widget), _gfxwop_box_draw, _gfxwop_basic_free, _gfxwop_basic_tag, _gfxwop_box_print,
	              _gfxwop_basic_compare_to, _gfxwop_box_equals, _gfxwop_box_superarea_of);
}

static int _gfxw_color_get_priority(gfx_color_t color) {
	return (color.mask & GFX_MASK_PRIORITY) ? color.priority : -1;
}

gfxw_box_t *gfxw_new_box(gfx_state_t *state, rect_t area, gfx_color_t color1, gfx_color_t color2, gfx_box_shade_t shade_type) {
	gfxw_box_t *widget = (gfxw_box_t *)_gfxw_new_widget(sizeof(gfxw_box_t), GFXW_BOX);

	widget->widget_priority = _gfxw_color_get_priority(color1);
	widget->bounds = area;
	widget->color1 = color1;
	widget->color2 = color2;
	widget->shade_type = shade_type;

	widget->flags |= GFXW_FLAG_VISIBLE;

	if ((color1.mask & GFX_MASK_VISUAL) && ((state && (state->driver->mode->palette)) || (!color1.alpha && !color2.alpha)))
		widget->flags |= GFXW_FLAG_OPAQUE;

	_gfxw_set_ops_BOX(GFXW(widget));

	return widget;
}

static gfxw_primitive_t *_gfxw_new_primitive(rect_t area, gfx_color_t color, gfx_line_mode_t mode,
													gfx_line_style_t style, gfxw_widget_type_t type) {
	gfxw_primitive_t *widget = (gfxw_primitive_t *)_gfxw_new_widget(sizeof(gfxw_primitive_t), type);

	widget->widget_priority = _gfxw_color_get_priority(color);
	widget->bounds = area;
	widget->color = color;
	widget->line_mode = mode;
	widget->line_style = style;

	widget->flags |= GFXW_FLAG_VISIBLE;
	return widget;
}

//*** Rectangles ***

static int _gfxwop_primitive_equals(gfxw_widget_t *widget, gfxw_widget_t *other) {
	gfxw_primitive_t *wprim = (gfxw_primitive_t *) widget, *oprim;
	if (widget->type != other->type)
		return 0;

	oprim = (gfxw_primitive_t *) other;

	if (!toCommonRect(wprim->bounds).equals(toCommonRect(oprim->bounds)))
		return 0;

	if (!_color_equals(wprim->color, oprim->color))
		return 0;

	if (wprim->line_mode != oprim->line_mode)
		return 0;

	if (wprim->line_style != oprim->line_style)
		return 0;

	return 1;
}

static int _gfxwop_rect_draw(gfxw_widget_t *widget, Common::Point pos) {
	gfxw_primitive_t *rect = (gfxw_primitive_t *) widget;
	DRAW_ASSERT(widget, GFXW_RECT);

	GFX_ASSERT(gfxop_draw_rectangle(rect->visual->gfx_state, gfx_rect(rect->bounds.x + pos.x, rect->bounds.y + pos.y,
	                                         rect->bounds.width - 1, rect->bounds.height - 1), rect->color, rect->line_mode, rect->line_style));
	return 0;
}

static int _gfxwop_rect_print(gfxw_widget_t *rect, int indentation) {
	_gfxw_print_widget(GFXW(rect), indentation);
	sciprintf("RECT");

	return 0;
}

void _gfxw_set_ops_RECT(gfxw_widget_t *prim) {
	_gfxw_set_ops(GFXW(prim), _gfxwop_rect_draw, _gfxwop_basic_free, _gfxwop_basic_tag, _gfxwop_rect_print,
	              _gfxwop_basic_compare_to, _gfxwop_primitive_equals, _gfxwop_basic_superarea_of);
}

gfxw_primitive_t *gfxw_new_rect(rect_t rect, gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style) {
	gfxw_primitive_t *prim = _gfxw_new_primitive(rect, color, line_mode, line_style, GFXW_RECT);
	prim->bounds.width++;
	prim->bounds.height++; // Since it is actually one pixel bigger in each direction

	_gfxw_set_ops_RECT(GFXW(prim));

	return prim;
}

//*** Lines ***

static int _gfxwop_line_draw(gfxw_widget_t *widget, Common::Point pos) {
	gfxw_primitive_t *line = (gfxw_primitive_t *)widget;
	rect_t linepos = widget->bounds;
	Common::Point p1, p2;

	linepos.width--;
	linepos.height--;

	DRAW_ASSERT(widget, GFXW_LINE);

	_split_rect(_move_rect(linepos, pos), &p1, &p2);
	GFX_ASSERT(gfxop_draw_line(line->visual->gfx_state, p1, p2, line->color, line->line_mode, line->line_style));
	return 0;
}

static int _gfxwop_line_print(gfxw_widget_t *widget, int indentation) {
	_gfxw_print_widget(widget, indentation);

	return 0;
}

void _gfxw_set_ops_LINE(gfxw_widget_t *prim) {
	_gfxw_set_ops(GFXW(prim), _gfxwop_line_draw, _gfxwop_basic_free, _gfxwop_basic_tag, _gfxwop_line_print,
	              _gfxwop_basic_compare_to, _gfxwop_primitive_equals, _gfxwop_basic_superarea_of);
}

gfxw_primitive_t *gfxw_new_line(Common::Point start, Common::Point end, gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style) {
	gfxw_primitive_t *prim;

	// SCI can draw lines inversely. We convert inverse lines to normal ones here, because the resulting rectangles are invalid
	if (end.x < start.x || end.y < start.y) {
		SWAP(start.x, end.x);
		SWAP(start.y, end.y);
	}

	// Encode into internal representation
	rect_t line = gfx_rect(start.x, start.y, end.x - start.x, end.y - start.y);

	line.width++;
	line.height++;

	prim = _gfxw_new_primitive(line, color, line_mode, line_style, GFXW_LINE);

	_gfxw_set_ops_LINE(GFXW(prim));

	return prim;
}

//*** Views and static views ***

gfxw_view_t *_gfxw_new_simple_view(gfx_state_t *state, Common::Point pos, int view, int loop, int cel, int palette, int priority, int control,
								   gfx_alignment_t halign, gfx_alignment_t valign, int size, gfxw_widget_type_t type) {
	gfxw_view_t *widget;
	int width, height;
	Common::Point offset;

	if (!state) {
		GFXERROR("Attempt to create view widget with NULL state!\n");
		return NULL;
	}

	if (gfxop_get_cel_parameters(state, view, loop, cel, &width, &height, &offset)) {
		GFXERROR("Attempt to retrieve cel parameters for (%d/%d/%d) failed (Maybe the values weren't checked beforehand?)\n",
		         view, cel, loop);
		return NULL;
	}

	widget = (gfxw_view_t *)_gfxw_new_widget(size, type);

	widget->widget_priority = priority;
	widget->pos = pos;
	widget->color.mask = ((priority < 0) ? 0 : GFX_MASK_PRIORITY) | ((control < 0) ? 0 : GFX_MASK_CONTROL);
	widget->color.priority = priority;
	widget->color.control = control;
	widget->view = view;
	widget->loop = loop;
	widget->cel = cel;
	widget->palette = palette;

	if (halign == ALIGN_CENTER)
		widget->pos.x -= width >> 1;
	else if (halign == ALIGN_RIGHT)
		widget->pos.x -= width;

	if (valign == ALIGN_CENTER)
		widget->pos.y -= height >> 1;
	else if (valign == ALIGN_BOTTOM)
		widget->pos.y -= height;

	widget->bounds = gfx_rect(widget->pos.x - offset.x, widget->pos.y - offset.y, width, height);

	widget->flags |= GFXW_FLAG_VISIBLE;

	return widget;
}

int _gfxwop_view_draw(gfxw_widget_t *widget, Common::Point pos) {
	gfxw_view_t *view = (gfxw_view_t *)widget;
	DRAW_ASSERT(widget, GFXW_VIEW);

	GFX_ASSERT(gfxop_draw_cel(view->visual->gfx_state, view->view, view->loop, view->cel,
				Common::Point(view->pos.x + pos.x, view->pos.y + pos.y), view->color, view->palette));

	return 0;
}

static int _gfxwop_static_view_draw(gfxw_widget_t *widget, Common::Point pos) {
	gfxw_view_t *view = (gfxw_view_t *)widget;
	DRAW_ASSERT(widget, GFXW_VIEW);

	GFX_ASSERT(gfxop_draw_cel_static(view->visual->gfx_state, view->view, view->loop,
	                                 view->cel, _move_point(view->bounds, pos), view->color, view->palette));

	return 0;
}

static int _w_gfxwop_view_print(gfxw_widget_t *widget, const char *name, int indentation) {
	gfxw_view_t *view = (gfxw_view_t *)widget;
	_gfxw_print_widget(widget, indentation);

	sciprintf("%s", name);
	sciprintf("(%d/%d/%d)@(%d,%d)[p:%d,c:%d]", view->view, view->loop, view->cel, view->pos.x, view->pos.y,
	          (view->color.mask & GFX_MASK_PRIORITY) ? view->color.priority : -1,
	          (view->color.mask & GFX_MASK_CONTROL) ? view->color.control : -1);

	return 0;
}

static int _gfxwop_view_print(gfxw_widget_t *widget, int indentation) {
	return _w_gfxwop_view_print(widget, "VIEW", indentation);
}

static int _gfxwop_static_view_print(gfxw_widget_t *widget, int indentation) {
	return _w_gfxwop_view_print(widget, "PICVIEW", indentation);
}

void _gfxw_set_ops_VIEW(gfxw_widget_t *view, char stat) {
	_gfxw_set_ops(GFXW(view), (stat) ? _gfxwop_static_view_draw : _gfxwop_view_draw, _gfxwop_basic_free,
	              _gfxwop_basic_tag, (stat) ? _gfxwop_static_view_print : _gfxwop_view_print,
	              _gfxwop_basic_compare_to, _gfxwop_basic_equals, _gfxwop_basic_superarea_of);
}

gfxw_view_t *gfxw_new_view(gfx_state_t *state, Common::Point pos, int view_nr, int loop, int cel, int palette, int priority, int control,
	gfx_alignment_t halign, gfx_alignment_t valign, int flags) {
	gfxw_view_t *view;

	if (flags & GFXW_VIEW_FLAG_DONT_MODIFY_OFFSET) {
		int foo;
		Common::Point offset;
		gfxop_get_cel_parameters(state, view_nr, loop, cel, &foo, &foo, &offset);
		pos.x += offset.x;
		pos.y += offset.y;
	}

	view = _gfxw_new_simple_view(state, pos, view_nr, loop, cel, palette, priority, control, halign, valign,
	                             sizeof(gfxw_view_t), (flags & GFXW_VIEW_FLAG_STATIC) ? GFXW_STATIC_VIEW : GFXW_VIEW);

	_gfxw_set_ops_VIEW(GFXW(view), (char)(flags & GFXW_VIEW_FLAG_STATIC));

	return view;
}

//*** Dynamic Views ***

static int _gfxwop_dyn_view_draw(gfxw_widget_t *widget, Common::Point pos) {
	gfxw_dyn_view_t *view = (gfxw_dyn_view_t *) widget;
	DRAW_ASSERT(widget, GFXW_DYN_VIEW);

	GFX_ASSERT(gfxop_draw_cel(view->visual->gfx_state, view->view, view->loop,
	                          view->cel, _move_point(view->draw_bounds, pos), view->color, view->palette));

	/*
	  gfx_color_t red;
	  red.visual.r = 0xff;
	  red.visual.g = red.visual.b = 0;
	  red.mask = GFX_MASK_VISUAL;
	  GFX_ASSERT(gfxop_draw_rectangle(view->visual->gfx_state,
	  gfx_rect(view->bounds.x + pos.x, view->bounds.y + pos.y, view->bounds.width - 1, view->bounds.height - 1), red, 0, 0));
	*/

	return 0;

}

static int _gfxwop_draw_nop(gfxw_widget_t *widget, Common::Point pos) {
	return 0;
}

static int _gfxwop_pic_view_draw(gfxw_widget_t *widget, Common::Point pos) {
	gfxw_dyn_view_t *view = (gfxw_dyn_view_t *) widget;
	DRAW_ASSERT(widget, GFXW_PIC_VIEW);

	GFX_ASSERT(gfxop_set_clip_zone(view->visual->gfx_state, view->parent->zone));
	GFX_ASSERT(gfxop_draw_cel_static_clipped(view->visual->gfx_state, view->view, view->loop,
	           view->cel, _move_point(view->draw_bounds, pos), view->color, view->palette));

	// Draw again on the back buffer
	GFX_ASSERT(gfxop_draw_cel(view->visual->gfx_state, view->view, view->loop, view->cel,
	                          _move_point(view->draw_bounds, pos), view->color, view->palette));


	widget->draw = _gfxwop_draw_nop; // No more drawing needs to be done

	return 0;
}

static int _gfxwop_some_view_print(gfxw_widget_t *widget, int indentation, const char *type_string) {
	gfxw_dyn_view_t *view = (gfxw_dyn_view_t *)widget;

	_gfxw_print_widget(widget, indentation);

	sciprintf("%s", type_string);
	sciprintf(" SORT=%d z=%d seq=%d (%d/%d/%d)@(%d,%d)[p:%d,c:%d]; sig[%04x@%p]", view->force_precedence, view->z,
	          view->sequence, view->view, view->loop, view->cel, view->pos.x, view->pos.y,
	          (view->color.mask & GFX_MASK_PRIORITY) ? view->color.priority : -1,
	          (view->color.mask & GFX_MASK_CONTROL) ? view->color.control : -1, view->signal, view->signalp);

	return 0;
}

static int _gfxwop_dyn_view_print(gfxw_widget_t *widget, int indentation) {
	return _gfxwop_some_view_print(widget, indentation, "DYNVIEW");
}

static int _gfxwop_pic_view_print(gfxw_widget_t *widget, int indentation) {
	return _gfxwop_some_view_print(widget, indentation, "PICVIEW");
}

static int _gfxwop_dyn_view_equals(gfxw_widget_t *widget, gfxw_widget_t *other) {
	gfxw_dyn_view_t *wview = (gfxw_dyn_view_t *)widget, *oview;
	if (!GFXW_IS_DYN_VIEW(other))
		return 0;

	oview = (gfxw_dyn_view_t *)other;

	if (wview->pos.x != oview->pos.x || wview->pos.y != oview->pos.y || wview->z != oview->z)
		return 0;

	if (wview->view != oview->view || wview->loop != oview->loop || wview->cel != oview->cel)
		return 0;

	if (!_color_equals(wview->color, oview->color))
		return 0;

	if (wview->flags != oview->flags)
		return 0;

	return 1;
}

static int _gfxwop_dyn_view_compare_to(gfxw_widget_t *widget, gfxw_widget_t *other) {
	int retval;
	gfxw_dyn_view_t *wview = (gfxw_dyn_view_t *) widget, *oview;
	if (!GFXW_IS_DYN_VIEW(other))
		return 1;

	oview = (gfxw_dyn_view_t *) other;

	retval = wview->force_precedence - oview->force_precedence;
	if (retval)
		return retval;

	retval = wview->pos.y - oview->pos.y;
	if (retval)
		return retval;

	retval = (wview->z - oview->z);
	if (retval)
		return retval;

	return -(wview->sequence - oview->sequence);
}

void _gfxw_set_ops_DYNVIEW(gfxw_widget_t *widget) {
	_gfxw_set_ops(GFXW(widget), _gfxwop_dyn_view_draw, _gfxwop_basic_free, _gfxwop_basic_tag,
	              _gfxwop_dyn_view_print, _gfxwop_dyn_view_compare_to, _gfxwop_dyn_view_equals, _gfxwop_basic_superarea_of);
}

void _gfxw_set_ops_PICVIEW(gfxw_widget_t *widget) {
	_gfxw_set_ops_DYNVIEW(widget);
	widget->draw = _gfxwop_pic_view_draw;
	widget->print = _gfxwop_pic_view_print;
}

gfxw_dyn_view_t *gfxw_new_dyn_view(gfx_state_t *state, Common::Point pos, int z, int view, int loop, int cel, int palette, int priority, int control,
	gfx_alignment_t halign, gfx_alignment_t valign, int sequence) {
	gfxw_dyn_view_t *widget;
	int width, height;
	int xalignmod, yalignmod;
	Common::Point offset;

	if (!state) {
		GFXERROR("Attempt to create view widget with NULL state!\n");
		return NULL;
	}

	if (gfxop_get_cel_parameters(state, view, loop, cel, &width, &height, &offset)) {
		GFXERROR("Attempt to retrieve cel parameters for (%d/%d/%d) failed (Maybe the values weren't checked beforehand?)\n",
		         view, cel, loop);
		return NULL;
	}

	widget = (gfxw_dyn_view_t *)_gfxw_new_widget(sizeof(gfxw_dyn_view_t), GFXW_DYN_VIEW);

	widget->pos = pos;
	widget->color.mask = ((priority < 0) ? 0 : GFX_MASK_PRIORITY) | ((control < 0) ? 0 : GFX_MASK_CONTROL);
	widget->widget_priority = priority;
	widget->color.priority = priority;
	widget->color.control = control;
	widget->color.alpha = 0;
	widget->color.visual = PaletteEntry(0,0,0); // FIXME: black!
	widget->view = view;
	widget->loop = loop;
	widget->cel = cel;
	widget->sequence = sequence;
	widget->force_precedence = 0;
	widget->palette = palette;

	if (halign == ALIGN_CENTER)
		xalignmod = width >> 1;
	else if (halign == ALIGN_RIGHT)
		xalignmod = width;
	else
		xalignmod = 0;

	if (valign == ALIGN_CENTER)
		yalignmod = height >> 1;
	else if (valign == ALIGN_BOTTOM)
		yalignmod = height;
	else
		yalignmod = 0;

	widget->z = z;

	widget->draw_bounds = gfx_rect(widget->pos.x - xalignmod, widget->pos.y - yalignmod - z, width, height);
	widget->bounds = gfx_rect(widget->pos.x - offset.x - xalignmod, widget->pos.y - offset.y - yalignmod - z, width, height);

	widget->flags |= GFXW_FLAG_VISIBLE;

	_gfxw_set_ops_DYNVIEW(GFXW(widget));

	widget->signalp = NULL;
	widget->signal = 0;

	return widget;
}

//*** Text ***

static int _gfxwop_text_free(gfxw_widget_t *widget) {
	gfxw_text_t *text = (gfxw_text_t *)widget;
	free(text->text);

	return _gfxwop_basic_free(widget);
}

static int _gfxwop_text_draw(gfxw_widget_t *widget, Common::Point pos) {
	gfxw_text_t *text = (gfxw_text_t *)widget;
	DRAW_ASSERT(widget, GFXW_TEXT);

	GFX_ASSERT(gfxop_draw_text(text->visual->gfx_state, text->text_handle, _move_rect(text->bounds, pos)));

	return 0;
}

static int _gfxwop_text_alloc_and_draw(gfxw_widget_t *widget, Common::Point pos) {
	gfxw_text_t *text = (gfxw_text_t *)widget;
	DRAW_ASSERT(widget, GFXW_TEXT);

	text->text_handle = gfxop_new_text(widget->visual->gfx_state, text->font_nr, text->text, text->bounds.width,
	                   text->halign, text->valign, text->color1, text->color2, text->bgcolor, text->text_flags);

	text->draw = _gfxwop_text_draw;

	return _gfxwop_text_draw(widget, pos);
}

static int _gfxwop_text_print(gfxw_widget_t *widget, int indentation) {
	_gfxw_print_widget(widget, indentation);
	sciprintf("TEXT:'%s'", ((gfxw_text_t *)widget)->text);

	return 0;
}

static int _gfxwop_text_equals(gfxw_widget_t *widget, gfxw_widget_t *other) {
	gfxw_text_t *wtext = (gfxw_text_t *)widget, *otext;
	if (other->type != GFXW_TEXT)
		return 0;

	otext = (gfxw_text_t *)other;

	if ((wtext->bounds.x != otext->bounds.x) || (wtext->bounds.y != otext->bounds.y))
		return 0;

	if (wtext->halign != otext->halign || wtext->valign != otext->valign)
		return 0;

	if (wtext->text_flags != otext->text_flags)
		return 0;

	if (wtext->font_nr != otext->font_nr)
		return 0;

	/* if (!(_color_equals(wtext->color1, otext->color1) && _color_equals(wtext->color2, otext->color2)
			&& _color_equals(wtext->bgcolor, otext->bgcolor)))
		return 0; */

	return 1;
}

static int _gfxwop_text_should_replace(gfxw_widget_t *widget, gfxw_widget_t *other) {
	gfxw_text_t *wtext = (gfxw_text_t *)widget, *otext;

	if (other->type != GFXW_TEXT)
		return 0;

	otext = (gfxw_text_t *)other;

	return strcmp(wtext->text, otext->text);
}

static int _gfxwop_text_compare_to(gfxw_widget_t *widget, gfxw_widget_t *other) {
	return 1;
}

void _gfxw_set_ops_TEXT(gfxw_widget_t *widget) {
	_gfxw_set_ops(GFXW(widget), _gfxwop_text_alloc_and_draw, _gfxwop_text_free, _gfxwop_basic_tag,
	              _gfxwop_text_print, _gfxwop_text_compare_to, _gfxwop_text_equals,
	              _gfxwop_basic_superarea_of);
	widget->should_replace = _gfxwop_text_should_replace;
}

gfxw_text_t *gfxw_new_text(gfx_state_t *state, rect_t area, int font, const char *text, gfx_alignment_t halign,
	gfx_alignment_t valign, gfx_color_t color1, gfx_color_t color2, gfx_color_t bgcolor, int text_flags) {
	gfxw_text_t *widget = (gfxw_text_t *)_gfxw_new_widget(sizeof(gfxw_text_t), GFXW_TEXT);

	widget->widget_priority = _gfxw_color_get_priority(color1);
	widget->font_nr = font;
	widget->text = (char *)sci_malloc(strlen(text) + 1);
	widget->halign = halign;
	widget->valign = valign;
	widget->color1 = color1;
	widget->color2 = color2;
	widget->bgcolor = bgcolor;
	widget->text_flags = text_flags;
	widget->text_handle = NULL;

	strcpy(widget->text, text);

	gfxop_get_text_params(state, font, text, area.width, &(widget->width), &(widget->height), text_flags,
	                      &(widget->lines_nr), &(widget->lineheight), &(widget->lastline_width));

	/* FIXME: Window is too big
	area.x += _calc_needmove(halign, area.width, widget->width);
	area.y += _calc_needmove(valign, area.height, widget->height);
	*/

	if (halign == ALIGN_LEFT)
		area.width = widget->width;
	if (valign == ALIGN_TOP)
		area.height = widget->height;

	widget->bounds = area;

	widget->flags |= GFXW_FLAG_VISIBLE;

	_gfxw_set_ops_TEXT(GFXW(widget));

	return widget;
}

void gfxw_text_info(gfx_state_t *state, gfxw_text_t *text, int *lines, int *lineheight, int *offset) {
	if (lines)
		*lines = text->lines_nr;
	if (lineheight)
		*lineheight = text->lineheight;
	if (offset)
		*offset = text->lastline_width;
}

//-- Container types --

static int _gfxwop_container_add_dirty_rel(gfxw_container_t *cont, rect_t rect, int propagate) {
	DDIRTY(stderr, "->container_add_dirty_rel(%d,%d,%d,%d, %d)\n", GFX_PRINT_RECT(rect), propagate);
	return cont->add_dirty_abs(cont, _move_rect(rect, Common::Point(cont->zone.x, cont->zone.y)), propagate);
}

static void _gfxw_set_container_ops(gfxw_container_t *container, gfxw_point_op *draw, gfxw_op *free, gfxw_op *tag,
	gfxw_op_int *print, gfxw_bin_op *compare_to, gfxw_bin_op *equals,
	gfxw_bin_op *superarea_of, gfxw_visual_op *set_visual,
	gfxw_unary_container_op *free_tagged, gfxw_unary_container_op *free_contents,
	gfxw_rect_op *add_dirty, gfxw_container_op *add) {
	_gfxw_set_ops(GFXW(container), draw, free, tag, print, compare_to, equals, superarea_of);

	container->free_tagged = free_tagged;
	container->free_contents = free_contents;
	container->add_dirty_abs = add_dirty;
	container->add_dirty_rel = _gfxwop_container_add_dirty_rel;
	container->add = add;
	container->set_visual = set_visual;
}

static int _w_gfxwop_container_print_contents(const char *name, gfxw_widget_t *widget, int indentation) {
	gfxw_widget_t *seeker = widget;

	indent(indentation);

	sciprintf("--%s:\n", name);

	while (seeker) {
		seeker->print(seeker, indentation + 1);
		sciprintf("\n");
		seeker = seeker->next;
	}

	return 0;
}

static int _w_gfxwop_container_print(gfxw_widget_t *widget, int indentation) {
	gfx_dirty_rect_t *dirty;
	gfxw_container_t *container = (gfxw_container_t *)widget;
	if (!GFXW_IS_CONTAINER(widget)) {
		GFXERROR("_w_gfxwop_container_print() called on type %d widget\n", widget->type);
		return 1;
	}

	sciprintf(" viszone=((%d,%d),(%dx%d))\n", container->zone.x, container->zone.y,
	          container->zone.width, container->zone.height);

	indent(indentation);
	sciprintf("--dirty:\n");

	dirty = container->dirty;
	while (dirty) {
		indent(indentation + 1);
		sciprintf("dirty(%d,%d, (%dx%d))\n", dirty->rect.x, dirty->rect.y, dirty->rect.width, dirty->rect.height);
		dirty = dirty->next;
	}

	_w_gfxwop_container_print_contents("contents", container->contents, indentation);

	return 0;
}

gfxw_container_t *_gfxw_new_container_widget(rect_t area, int size, gfxw_widget_type_t type) {
	gfxw_container_t *widget = (gfxw_container_t *)_gfxw_new_widget(size, type);

	widget->bounds = widget->zone = area;
	widget->contents = NULL;
	widget->nextpp = &(widget->contents);
	widget->dirty = NULL;

	widget->flags |= GFXW_FLAG_VISIBLE | GFXW_FLAG_CONTAINER;

	return widget;
}

static void recursively_free_dirty_rects(gfx_dirty_rect_t *dirty) {
	if (dirty) {
		recursively_free_dirty_rects(dirty->next);
		free(dirty);
	}
}

int ti = 0;

static int _gfxw_dirty_rect_overlaps_normal_rect(rect_t port_zone, rect_t bounds, rect_t dirty) {
	bounds.x += port_zone.x;
	bounds.y += port_zone.y;

	return gfx_rects_overlap(bounds, dirty);
}

static int _gfxwop_container_draw_contents(gfxw_widget_t *widget, gfxw_widget_t *contents) {
	gfxw_container_t *container = (gfxw_container_t *)widget;
	gfx_dirty_rect_t *dirty = container->dirty;
	gfx_state_t *gfx_state = (widget->visual) ? widget->visual->gfx_state : ((gfxw_visual_t *) widget)->gfx_state;
	int draw_ports;
	rect_t nullzone = {0, 0, 0, 0};

	if (!contents)
		return 0;

	while (dirty) {
		gfxw_widget_t *seeker = contents;

		while (seeker) {
			if (_gfxw_dirty_rect_overlaps_normal_rect(GFXW_IS_CONTAINER(seeker) ? nullzone : container->zone,
			        // Containers have absolute coordinates, reflect this.
			        seeker->bounds, dirty->rect)) {

				if (GFXW_IS_CONTAINER(seeker)) {// Propagate dirty rectangles /upwards/
					DDIRTY(stderr, "container_draw_contents: propagate upwards (%d,%d,%d,%d ,0)\n", GFX_PRINT_RECT(dirty->rect));
					((gfxw_container_t *)seeker)->add_dirty_abs((gfxw_container_t *)seeker, dirty->rect, 0);
				}

				seeker->flags |= GFXW_FLAG_DIRTY;
			}

			seeker = seeker->next;
		}

		dirty = dirty->next;
	}

	// The draw loop is executed twice: Once for normal data, and once for ports.
	for (draw_ports = 0; draw_ports < 2; draw_ports++) {
		dirty = container->dirty;

		while (dirty) {
			gfxw_widget_t *seeker = contents;
			while (seeker && (draw_ports || !GFXW_IS_PORT(seeker))) {
				rect_t small_rect;
				byte draw_noncontainers;

				memcpy(&small_rect, &(dirty->rect), sizeof(rect_t));
				draw_noncontainers = !_gfxop_clip(&small_rect, container->bounds);

				if (seeker->flags & GFXW_FLAG_DIRTY) {

					if (!GFXW_IS_CONTAINER(seeker) && draw_noncontainers) {
						GFX_ASSERT(gfxop_set_clip_zone(gfx_state, small_rect));
					}
					/* Clip zone must be reset after each element, because we might
					** descend into containers.
					** Doing this is relatively cheap, though. */
					if (draw_noncontainers || GFXW_IS_CONTAINER(seeker))
						seeker->draw(seeker, Common::Point(container->zone.x, container->zone.y));

					if (!dirty->next)
						seeker->flags &= ~GFXW_FLAG_DIRTY;
				}

				seeker = seeker->next;
			}
			dirty = dirty->next;
		}
	}
	// Remember that the dirty rects should be freed afterwards!

	return 0;
}

static int _gfxwop_container_free(gfxw_widget_t *widget) {
	gfxw_container_t *container = (gfxw_container_t *)widget;
	gfxw_widget_t *seeker = container->contents;

	while (seeker) {
		gfxw_widget_t *next = seeker->next;
		seeker->widfree(seeker);
		seeker = next;
	}

	recursively_free_dirty_rects(container->dirty);
	container->dirty = NULL;

	return _gfxwop_basic_free(widget);
}

static int _gfxwop_container_tag(gfxw_widget_t *widget) {
	gfxw_container_t *container = (gfxw_container_t *) widget;
	gfxw_widget_t *seeker = container->contents;

	while (seeker) {
		seeker->tag(seeker);
		seeker = seeker->next;
	}

	return 0;
}

static int _w_gfxwop_container_set_visual_contents(gfxw_widget_t *contents, gfxw_visual_t *visual) {
	while (contents) {
		contents->set_visual(contents, visual);
		contents = contents->next;
	}
	return 0;
}

static int _gfxwop_container_set_visual(gfxw_widget_t *widget, gfxw_visual_t *visual) {
	gfxw_container_t *container = (gfxw_container_t *) widget;

	container->visual = visual;
	if (widget->parent) {
		if (!(GFXW_IS_LIST(widget) && !GFXWC(widget)->contents)) {
			DDIRTY(stderr, "set_visual::DOWNWARDS abs(%d,%d,%d,%d, 1)\n", GFX_PRINT_RECT(widget->bounds));
			widget->parent->add_dirty_abs(widget->parent, widget->bounds, 1);
		}
	}

	return _w_gfxwop_container_set_visual_contents(container->contents, visual);
}

static int _gfxwop_container_free_tagged(gfxw_container_t *container) {
	gfxw_widget_t *seekerp = (container->contents);

	while (seekerp) {
		gfxw_widget_t *redshirt = seekerp;

		if (redshirt->flags & GFXW_FLAG_TAGGED) {
			seekerp = (redshirt->next);
			redshirt->widfree(redshirt);
		} else
			seekerp = (seekerp)->next;
	}

	return 0;
}

static int _gfxwop_container_free_contents(gfxw_container_t *container) {
	gfxw_widget_t *seeker = container->contents;

	while (seeker) {
		gfxw_widget_t *next = seeker->next;
		seeker->widfree(seeker);
		seeker = next;
	}
	return 0;
}

static void _gfxw_dirtify_container(gfxw_container_t *container, gfxw_widget_t *widget) {
	if (GFXW_IS_CONTAINER(widget))
		container->add_dirty_abs(GFXWC(container), widget->bounds, 1);
	else
		container->add_dirty_rel(GFXWC(container), widget->bounds, 1);
}

static int _parentize_widget(gfxw_container_t *container, gfxw_widget_t *widget) {
	if (widget->parent) {
		GFXERROR("_gfxwop_container_add(): Attempt to give second parent node to widget!\nWidget:");
		widget->print(GFXW(widget), 3);
		sciprintf("\nContainer:");
		container->print(GFXW(container), 3);

		return 1;
	}

	widget->parent = GFXWC(container);

	if (GFXW_IS_VISUAL(container))
		widget->set_visual(widget, (gfxw_visual_t *) container);
	else if (container->visual)
		widget->set_visual(widget, container->visual);

	return 0;
}

static int _gfxw_container_id_equals(gfxw_container_t *container, gfxw_widget_t *widget) {
	gfxw_widget_t **seekerp = &(container->contents);

	if (GFXW_IS_PORT(widget))
		return 0;

	if (widget->ID == GFXW_NO_ID)
		return 0;

	while (*seekerp && ((*seekerp)->ID != widget->ID || (*seekerp)->subID != widget->subID))
		seekerp = &((*seekerp)->next);

	if (!*seekerp)
		return 0;

	if ((*seekerp)->equals(*seekerp, widget) && !(*seekerp)->should_replace(*seekerp, widget)) {
		widget->widfree(widget);
		(*seekerp)->flags &= ~GFXW_FLAG_TAGGED;
		return 1;
	} else {
		if (!(widget->flags & GFXW_FLAG_MULTI_ID))
			(*seekerp)->widfree(*seekerp);
		return 0;
	}
}

static int _gfxwop_container_add_dirty(gfxw_container_t *container, rect_t dirty, int propagate) {
#if 0
	// This code has been disabled because containers may contain sub-containers with
	// bounds greater than their own.
	if (_gfxop_clip(&dirty, container->bounds))
		return 0;
#endif

	DDIRTY(stderr, "Effectively adding dirty %d,%d,%d,%d %d to ID %d\n", GFX_PRINT_RECT(dirty), propagate, container->ID);
	container->dirty = gfxdr_add_dirty(container->dirty, dirty, GFXW_DIRTY_STRATEGY);
	return 0;
}

static int _gfxwop_container_add(gfxw_container_t *container, gfxw_widget_t *widget) {
	if (_gfxw_container_id_equals(container, widget))
		return 0;

	if (_parentize_widget(container, widget))
		return 1;

	if (!(GFXW_IS_LIST(widget) && (!GFXWC(widget)->contents))) { // Don't dirtify self on empty lists
		DDIRTY(stderr, "container_add: dirtify DOWNWARDS (%d,%d,%d,%d, 1)\n", GFX_PRINT_RECT(widget->bounds));
		_gfxw_dirtify_container(container, widget);
	}

	*(container->nextpp) = widget;
	container->nextpp = &(widget->next);

	return 0;
}

//*** Lists and sorted lists ***

static int _gfxwop_list_draw(gfxw_widget_t *list, Common::Point pos) {
	DRAW_ASSERT(list, GFXW_LIST);

	_gfxwop_container_draw_contents(list, ((gfxw_list_t *)list)->contents);
	recursively_free_dirty_rects(GFXWC(list)->dirty);
	GFXWC(list)->dirty = NULL;
	list->flags &= ~GFXW_FLAG_DIRTY;

	return 0;
}

static int _gfxwop_sorted_list_draw(gfxw_widget_t *list, Common::Point pos) {
	DRAW_ASSERT(list, GFXW_SORTED_LIST);

	_gfxwop_container_draw_contents(list, ((gfxw_list_t *)list)->contents);
	recursively_free_dirty_rects(GFXWC(list)->dirty);
	GFXWC(list)->dirty = NULL;

	return 0;
}

static int _w_gfxwop_list_print(gfxw_widget_t *list, const char *name, int indentation) {
	_gfxw_print_widget(list, indentation);
	sciprintf("%s", name);

	return _w_gfxwop_container_print(list, indentation);
}

static int _gfxwop_list_print(gfxw_widget_t *list, int indentation) {
	return _w_gfxwop_list_print(list, "LIST", indentation);
}

static int _gfxwop_sorted_list_print(gfxw_widget_t *list, int indentation) {
	return _w_gfxwop_list_print(list, "SORTED_LIST", indentation);
}

#if 0
struct gfxw_widget_list {
	gfxw_widget_t *widget;
	struct gfxw_widget_list *next;
};

static struct gfxw_widtet_list *_gfxw_make_widget_list_recursive(gfxw_widget_t *widget) {
	gfxw_widget_list *node;

	if (!widget)
		return NULL;

	node = sci_malloc(sizeof(struct gfxw_widget_list));
	node->widget = widget;
	node->next = _gfxw_make_widget_list_recursive(widget->next);

	return node;
}

static struct gfxw_widget_list *_gfxw_make_widget_list(gfxw_container_t *container) {
	return _gfxw_make_widget_list_recursive(container->contents);
}
#endif

static int _gfxwop_list_equals(gfxw_widget_t *widget, gfxw_widget_t *other) {
	// Requires identical order of list elements.
	gfxw_list_t *wlist, *olist;

	if (widget->type != other->type)
		return 0;

	if (!GFXW_IS_LIST(widget)) {
		GFXWARN("_gfxwop_list_equals(): Method called on non-list!\n");
		widget->print(widget, 0);
		sciprintf("\n");
		return 0;
	}

	wlist = (gfxw_list_t *)widget;
	olist = (gfxw_list_t *)other;

	if (memcmp(&(wlist->bounds), &(olist->bounds), sizeof(rect_t)))
		return 0;

	widget = wlist->contents;
	other = olist->contents;

	while (widget && other) {
		if (!(widget->equals(widget, other) && !widget->should_replace(widget, other)))
			return 0;

		widget = widget->next;
		other = other->next;
	}

	return (!widget && !other); // True if both are finished now
}

static int _gfxwop_list_add_dirty(gfxw_container_t *container, rect_t dirty, int propagate) {
	// Lists add dirty boxes to both themselves and their parenting port/visual

	container->flags |= GFXW_FLAG_DIRTY;

	DDIRTY(stderr, "list_add_dirty %d,%d,%d,%d %d\n", GFX_PRINT_RECT(dirty), propagate);
	if (propagate)
		if (container->parent) {
			DDIRTY(stderr, "->PROPAGATING\n");
			container->parent->add_dirty_abs(container->parent, dirty, 1);
		}

	return _gfxwop_container_add_dirty(container, dirty, propagate);
}

int _gfxwop_ordered_add(gfxw_container_t *container, gfxw_widget_t *widget, int compare_all) {
	// O(n)
	gfxw_widget_t **seekerp = &(container->contents);

	if (widget->next) {
		GFXERROR("_gfxwop_sorted_list_add(): Attempt to add widget to two lists!\nWidget:");
		widget->print(GFXW(widget), 3);
		sciprintf("\nList:");
		container->print(GFXW(container), 3);
		BREAKPOINT();

		return 1;
	}

	if (_gfxw_container_id_equals(container, widget))
		return 0;

	while (*seekerp && (compare_all || (widget->compare_to(widget, *seekerp) >= 0))) {

		if (widget->equals(GFXW(widget), GFXW(*seekerp))) {
			if (compare_all) {
				if ((*seekerp)->visual)
					(*seekerp)->widfree(GFXW(*seekerp)); // If it's a fresh widget
				else
					gfxw_annihilate(GFXW(*seekerp));

				return _gfxwop_ordered_add(container, widget, compare_all); // We might have destroyed the container's contents
			} else {
				widget->next = (*seekerp)->next;
				(*seekerp)->widfree(GFXW(*seekerp));
				*seekerp = widget;
				return (_parentize_widget(container, widget));
			}
		}

		if (*seekerp)
			seekerp = &((*seekerp)->next);
	}

	widget->next = *seekerp;
	*seekerp = widget;

	return _parentize_widget(container, widget);
}

static int _gfxwop_sorted_list_add(gfxw_container_t *container, gfxw_widget_t *widget) {
	// O(n)
	return _gfxwop_ordered_add(container, widget, 0);
}

void _gfxw_set_ops_LIST(gfxw_container_t *list, char sorted) {
	_gfxw_set_container_ops((gfxw_container_t *)list, sorted ? _gfxwop_sorted_list_draw : _gfxwop_list_draw,
	                        _gfxwop_container_free, _gfxwop_container_tag,
	                        sorted ? _gfxwop_sorted_list_print : _gfxwop_list_print,
	                        _gfxwop_basic_compare_to, sorted ? _gfxwop_basic_equals : _gfxwop_list_equals,
	                        _gfxwop_basic_superarea_of, _gfxwop_container_set_visual,
	                        _gfxwop_container_free_tagged, _gfxwop_container_free_contents,
	                        _gfxwop_list_add_dirty, sorted ? _gfxwop_sorted_list_add : _gfxwop_container_add);
}

gfxw_list_t *gfxw_new_list(rect_t area, int sorted) {
	gfxw_list_t *list = (gfxw_list_t *) _gfxw_new_container_widget(area, sizeof(gfxw_list_t),
	                    sorted ? GFXW_SORTED_LIST : GFXW_LIST);

	_gfxw_set_ops_LIST(GFXWC(list), (char)sorted);

	return list;
}

//*** Visuals ***

static int _gfxwop_visual_draw(gfxw_widget_t *widget, Common::Point pos) {
	gfxw_visual_t *visual = (gfxw_visual_t *) widget;
	gfx_dirty_rect_t *dirty = visual->dirty;
	DRAW_ASSERT(widget, GFXW_VISUAL);

	while (dirty) {
		int err = gfxop_clear_box(visual->gfx_state, dirty->rect);

		if (err) {
			GFXERROR("Error while clearing dirty rect (%d,%d,(%dx%d))\n", dirty->rect.x,
			         dirty->rect.y, dirty->rect.width, dirty->rect.height);
			if (err == GFX_FATAL)
				return err;
		}

		dirty = dirty->next;
	}

	_gfxwop_container_draw_contents(widget, visual->contents);

	recursively_free_dirty_rects(visual->dirty);
	visual->dirty = NULL;
	widget->flags &= ~GFXW_FLAG_DIRTY;

	return 0;
}

static int _gfxwop_visual_free(gfxw_widget_t *widget) {
	gfxw_visual_t *visual = (gfxw_visual_t *) widget;
	gfxw_port_t **portrefs;
	int retval;

	if (!GFXW_IS_VISUAL(visual)) {
		GFXERROR("_gfxwop_visual_free() called on non-visual!Widget was: ");
		widget->print(widget, 3);
		return 1;
	}

	portrefs = visual->port_refs;

	retval = _gfxwop_container_free(widget);

	free(portrefs);

	return 0;
}

static int _gfxwop_visual_print(gfxw_widget_t *widget, int indentation) {
	int i;
	int comma = 0;
	gfxw_visual_t *visual = (gfxw_visual_t *) widget;

	if (!GFXW_IS_VISUAL(visual)) {
		GFXERROR("_gfxwop_visual_free() called on non-visual!Widget was: ");
		widget->print(widget, 3);
		return 1;
	}

	_gfxw_print_widget(widget, indentation);
	sciprintf("VISUAL; ports={");
	for (i = 0; i < visual->port_refs_nr; i++)
		if (visual->port_refs[i]) {
			if (comma)
				sciprintf(",");
			else
				comma = 1;

			sciprintf("%d", i);
		}
	sciprintf("}\n");

	return _w_gfxwop_container_print(widget, indentation);
}

static int _gfxwop_visual_set_visual(gfxw_widget_t *self, gfxw_visual_t *visual) {
	if (self != GFXW(visual)) {
		GFXWARN("Attempt to set a visual's parent visual to something else!\n");
	} else {
		GFXWARN("Attempt to set a visual's parent visual!\n");
	}

	return 1;
}

void _gfxw_set_ops_VISUAL(gfxw_container_t *visual) {
	_gfxw_set_container_ops((gfxw_container_t *)visual, _gfxwop_visual_draw, _gfxwop_visual_free,
	                        _gfxwop_container_tag, _gfxwop_visual_print, _gfxwop_basic_compare_to,
	                        _gfxwop_basic_equals, _gfxwop_basic_superarea_of, _gfxwop_visual_set_visual,
	                        _gfxwop_container_free_tagged, _gfxwop_container_free_contents,
	                        _gfxwop_container_add_dirty, _gfxwop_container_add);
}

gfxw_visual_t *gfxw_new_visual(gfx_state_t *state, int font) {
	gfxw_visual_t *visual = (gfxw_visual_t *) _gfxw_new_container_widget(gfx_rect(0, 0, 320, 200), sizeof(gfxw_visual_t), GFXW_VISUAL);

	visual->font_nr = font;
	visual->gfx_state = state;

	visual->port_refs_nr = 16;
	visual->port_refs = (gfxw_port_t **)sci_calloc(sizeof(gfxw_port_t), visual->port_refs_nr);

	_gfxw_set_ops_VISUAL(GFXWC(visual));

	return visual;
}

static int _visual_find_free_ID(gfxw_visual_t *visual) {
	int id = 0;
	int newports = 16;

	while (visual->port_refs[id] && id < visual->port_refs_nr)
		id++;

	if (id == visual->port_refs_nr) { // Out of ports?
		visual->port_refs_nr += newports;
		visual->port_refs = (gfxw_port_t**)sci_realloc(visual->port_refs, visual->port_refs_nr);
		memset(visual->port_refs + id, 0, newports * sizeof(gfxw_port_t *)); // Clear new port refs
	}

	return id;
}

static int _gfxwop_add_dirty_rects(gfxw_container_t *dest, gfx_dirty_rect_t *src) {
	DDIRTY(stderr, "Adding multiple dirty to #%d\n", dest->ID);
	if (src) {
		dest->dirty = gfxdr_add_dirty(dest->dirty, src->rect, GFXW_DIRTY_STRATEGY);
		_gfxwop_add_dirty_rects(dest, src->next);
	}

	return 0;
}

//*** Ports ***

static int _gfxwop_port_draw(gfxw_widget_t *widget, Common::Point pos) {
	gfxw_port_t *port = (gfxw_port_t *) widget;
	DRAW_ASSERT(widget, GFXW_PORT);

	if (port->decorations) {
		DDIRTY(stderr, "Getting/applying deco dirty (multi)\n");
		_gfxwop_add_dirty_rects(GFXWC(port->decorations), port->dirty);
		if (port->decorations->draw(GFXW(port->decorations), gfxw_point_zero)) {
			port->decorations->dirty = NULL;
			return 1;
		}
		port->decorations->dirty = NULL;
	}

	_gfxwop_container_draw_contents(widget, port->contents);

	recursively_free_dirty_rects(port->dirty);
	port->dirty = NULL;
	widget->flags &= ~GFXW_FLAG_DIRTY;

	return 0;
}

static int _gfxwop_port_free(gfxw_widget_t *widget) {
	gfxw_port_t *port = (gfxw_port_t *) widget;

	if (port->visual) {
		gfxw_visual_t *visual = port->visual;
		int ID = port->ID;

		if (ID < 0 || ID >= visual->port_refs_nr) {
			GFXWARN("Attempt to free port #%d; allowed: [0..%d]!\n", ID, visual->port_refs_nr);
			return GFX_ERROR;
		}

		if (visual->port_refs[ID] != port) {
			GFXWARN("While freeing port %d: Port is at %p, but port list indicates %p", ID, (void *)port, (void *)visual->port_refs[ID]);
		} else
			visual->port_refs[ID] = NULL;

	}

	if (port->decorations)
		port->decorations->widfree(GFXW(port->decorations));

	return _gfxwop_container_free(widget);
}

static int _gfxwop_port_print(gfxw_widget_t *widget, int indentation) {
	gfxw_port_t *port = (gfxw_port_t *)widget;

	_gfxw_print_widget(widget, indentation);
	sciprintf("PORT");
	sciprintf(" font=%d drawpos=(%d,%d)", port->font_nr, port->draw_pos.x, port->draw_pos.y);
	if (port->gray_text)
		sciprintf(" (gray)");
	_w_gfxwop_container_print(GFXW(port), indentation);

	return _w_gfxwop_container_print_contents("decorations", GFXW(port->decorations), indentation);
}

static int _gfxwop_port_superarea_of(gfxw_widget_t *self, gfxw_widget_t *other) {
	gfxw_port_t *port = (gfxw_port_t *) self;

	if (!port->port_bg)
		return _gfxwop_basic_superarea_of(self, other);

	return port->port_bg->superarea_of(port->port_bg, other);
}

static int _gfxwop_port_set_visual(gfxw_widget_t *widget, gfxw_visual_t *visual) {
	gfxw_list_t *decorations = ((gfxw_port_t *) widget)->decorations;
	widget->visual = visual;

	if (decorations)
		if (decorations->set_visual(GFXW(decorations), visual)) {
			GFXWARN("Setting the visual for decorations failed for port ");
			widget->print(widget, 1);
			return 1;
		}

	return _gfxwop_container_set_visual(widget, visual);
}

static int _gfxwop_port_add_dirty(gfxw_container_t *widget, rect_t dirty, int propagate) {
	gfxw_port_t *self = (gfxw_port_t *) widget;

	self->flags |= GFXW_FLAG_DIRTY;

	_gfxwop_container_add_dirty(widget, dirty, propagate);

	DDIRTY(stderr, "Added dirty to ID %d\n", widget->ID);
	DDIRTY(stderr, "dirty= (%d,%d,%d,%d) bounds (%d,%d,%d,%d)\n", dirty.x, dirty.x, dirty.width, dirty.height,
	       widget->bounds.x, widget->bounds.y, widget->bounds.width, widget->bounds.height);
#if 0
	// FIXME: This is a worthwhile optimization
	if (self->port_bg) {
		gfxw_widget_t foo;

		foo.bounds = dirty; // Yeah, sub-elegant, I know
		foo.bounds.x -= self->zone.x;
		foo.bounds.y -= self->zone.y;
		if (self->port_bg->superarea_of(self->port_bg, &foo)) {
			gfxw_container_t *parent = self->parent;
			while (parent) {
				fprintf(stderr, "Dirtifying parent id %d\n", parent->ID);
				parent->flags |= GFXW_FLAG_DIRTY;
				parent = parent->parent;
			}
			return 0;
		}
	} // else propagate to the parent, since we're not 'catching' the dirty rect
#endif

	if (propagate)
		if (self->parent) {
			DDIRTY(stderr, "PROPAGATE\n");
			return self->parent->add_dirty_abs(self->parent, dirty, 1);
		}

	return 0;
}

static int _gfxwop_port_add(gfxw_container_t *container, gfxw_widget_t *widget) {
	// O(n)
	return _gfxwop_ordered_add(container, widget, 1);
}

void _gfxw_set_ops_PORT(gfxw_container_t *widget) {
	_gfxw_set_container_ops((gfxw_container_t *)widget, _gfxwop_port_draw, _gfxwop_port_free, _gfxwop_container_tag,
	                        _gfxwop_port_print, _gfxwop_basic_compare_to, _gfxwop_basic_equals, _gfxwop_port_superarea_of,
	                        _gfxwop_port_set_visual, _gfxwop_container_free_tagged, _gfxwop_container_free_contents,
	                        _gfxwop_port_add_dirty, _gfxwop_port_add);
}

gfxw_port_t *gfxw_new_port(gfxw_visual_t *visual, gfxw_port_t *predecessor, rect_t area, gfx_color_t fgcolor, gfx_color_t bgcolor) {
	gfxw_port_t *widget = (gfxw_port_t *)_gfxw_new_container_widget(area, sizeof(gfxw_port_t), GFXW_PORT);

	VERIFY_WIDGET(visual);

	widget->port_bg = NULL;
	widget->parent = NULL;
	widget->decorations = NULL;
	widget->title_text = NULL;
	widget->draw_pos = Common::Point(0, 0);
	widget->gray_text = 0;
	widget->color = fgcolor;
	widget->bgcolor = bgcolor;
	widget->font_nr = visual->font_nr;
	widget->ID = _visual_find_free_ID(visual);
	widget->chrono_port = 0;
	visual->port_refs[widget->ID] = widget;

	_gfxw_set_ops_PORT(GFXWC(widget));

	return widget;
}

void gfxw_port_auto_restore_background(gfxw_visual_t *visual, gfxw_port_t *window, rect_t auto_rect) {
	window->port_flags |= WINDOW_FLAG_AUTO_RESTORE;
	window->restore_snap = gfxw_make_snapshot(visual, auto_rect);
}

gfxw_port_t *gfxw_remove_port(gfxw_visual_t *visual, gfxw_port_t *port) {
	gfxw_port_t *parent;
	VERIFY_WIDGET(visual);
	VERIFY_WIDGET(port);

	if (!visual->contents) {
		GFXWARN("Attempt to remove port from empty visual\n");
		return NULL;
	}

	parent = (gfxw_port_t *)port->parent;
	if (port->port_flags & WINDOW_FLAG_AUTO_RESTORE)
		gfxw_restore_snapshot(visual, port->restore_snap);

	if (port->widfree(GFXW(port)))
		return parent;

	while (parent && !GFXW_IS_PORT(parent))
		parent = (gfxw_port_t *)parent->parent; // Ascend through ancestors

	return parent;
}

gfxw_port_t *gfxw_find_port(gfxw_visual_t *visual, int ID) {
	if (ID < 0 || ID >= visual->port_refs_nr)
		return NULL;

	return visual->port_refs[ID];
}

gfxw_port_t *gfxw_find_default_port(gfxw_visual_t *visual) {
	int id = visual->port_refs_nr;

	while (id--) {
		gfxw_port_t *port = visual->port_refs[id];

		if (port)
			return port;
	}

	return NULL;
}

// - other functions -

gfxw_widget_t *gfxw_set_id(gfxw_widget_t *widget, int ID, int subID) {
	if (widget) {
		widget->ID = ID;
		widget->subID = subID;
	}

	return widget;
}

gfxw_dyn_view_t *gfxw_dyn_view_set_params(gfxw_dyn_view_t *widget, int under_bits, void *under_bitsp, int signal, void *signalp) {
	if (!widget)
		return NULL;

	widget->under_bits = under_bits;
	widget->under_bitsp = under_bitsp;
	widget->signal = signal;
	widget->signalp = signalp;

	return widget;
}

gfxw_widget_t *gfxw_remove_id(gfxw_container_t *container, int ID, int subID) {
	gfxw_widget_t **wp = &(container->contents);

	while (*wp) {
		if ((*wp)->ID == ID && (subID == GFXW_NO_ID || (*wp)->subID == subID)) {
			gfxw_widget_t *widget = *wp;

			*wp = (*wp)->next;
			widget->next = NULL;
			widget->parent = NULL;
			widget->visual = NULL;

			return widget;
		}

		wp = &((*wp)->next);
	}

	return NULL;
}

gfxw_widget_t *gfxw_hide_widget(gfxw_widget_t *widget) {
	if (widget->flags & GFXW_FLAG_VISIBLE) {
		widget->flags &= ~GFXW_FLAG_VISIBLE;

		if (widget->parent)
			widget->parent->add_dirty_rel(widget->parent, widget->bounds, 1);
	}

	return widget;
}

gfxw_widget_t *gfxw_show_widget(gfxw_widget_t *widget) {
	if (!(widget->flags & GFXW_FLAG_VISIBLE)) {
		widget->flags |= GFXW_FLAG_VISIBLE;

		if (widget->parent)
			widget->parent->add_dirty_rel(widget->parent, widget->bounds, 1);
	}

	return widget;
}

gfxw_snapshot_t *gfxw_make_snapshot(gfxw_visual_t *visual, rect_t area) {
	gfxw_snapshot_t *retval = (gfxw_snapshot_t*)sci_malloc(sizeof(gfxw_snapshot_t));

	retval->serial = widget_serial_number_counter++;

	retval->area = area;

	// Work around subset semantics in gfx_rect_subset.
	// This fixes the help icon in LSL5. */
	if (retval->area.width == 320) retval->area.width = 321;

	return retval;
}

int gfxw_widget_matches_snapshot(gfxw_snapshot_t *snapshot, gfxw_widget_t *widget) {
	int free_below = (snapshot->serial < widget_serial_number_counter) ? 0 : widget_serial_number_counter;
	int free_above_eq = snapshot->serial;
	rect_t bounds = widget->bounds;

	if (!GFXW_IS_CONTAINER(widget) && widget->parent) {
		bounds.x += widget->parent->bounds.x;
		bounds.y += widget->parent->bounds.y;
	}

	// Note: the check for snapshot->area and bounds is NOT the same as contains() in Common::Rect
	// (this one includes equality too)
	return ((widget->serial >= free_above_eq || widget->serial < free_below) && 
			(snapshot->area.x <= bounds.x && snapshot->area.y <= bounds.y &&
			 snapshot->area.x + snapshot->area.width >= bounds.x + bounds.width && 
			 snapshot->area.y + snapshot->area.height >= bounds.y + bounds.height));
}

#define MAGIC_FREE_NUMBER -42

void _gfxw_free_contents_appropriately(gfxw_container_t *container, gfxw_snapshot_t *snapshot, int priority) {
	gfxw_widget_t *widget = container->contents;

	while (widget) {
		gfxw_widget_t *next = widget->next;

		if (gfxw_widget_matches_snapshot(snapshot, widget) && !(widget->flags & GFXW_FLAG_IMMUNE_TO_SNAPSHOTS)
		        && (priority == MAGIC_FREE_NUMBER || priority <= widget->widget_priority || widget->widget_priority == -1)) {
			widget->widfree(widget);
		} else {
			if (GFXW_IS_CONTAINER(widget))
				_gfxw_free_contents_appropriately(GFXWC(widget), snapshot, priority);
		}

		widget = next;
	}
}

gfxw_snapshot_t *gfxw_restore_snapshot(gfxw_visual_t *visual, gfxw_snapshot_t *snapshot) {
	_gfxw_free_contents_appropriately(GFXWC(visual), snapshot, MAGIC_FREE_NUMBER);

	return snapshot;
}

void gfxw_annihilate(gfxw_widget_t *widget) {
	gfxw_visual_t *visual = widget->visual;
	int widget_priority = 0;
	int free_overdrawn = 0;

	gfxw_snapshot_t snapshot;
	if (!GFXW_IS_CONTAINER(widget) && widget->parent && visual && (widget->flags & GFXW_FLAG_VISIBLE)) {
		snapshot.serial = 0;
		snapshot.area = widget->bounds;
		snapshot.area.x += widget->parent->zone.x;
		snapshot.area.y += widget->parent->zone.y;
		free_overdrawn = 1;
		widget_priority = widget->widget_priority;
	}

	widget->widfree(GFXW(widget));

	if (free_overdrawn)
		_gfxw_free_contents_appropriately(GFXWC(visual), &snapshot, widget_priority);
}

gfxw_dyn_view_t *gfxw_picviewize_dynview(gfxw_dyn_view_t *dynview) {
	dynview->type = GFXW_PIC_VIEW;
	dynview->flags |= GFXW_FLAG_DIRTY;

	_gfxw_set_ops_PICVIEW(GFXW(dynview));

	if (dynview->parent)
		_gfxw_dirtify_container(dynview->parent, GFXW(dynview));

	return dynview;
}

// Chrono-Ports (tm)

gfxw_port_t *gfxw_get_chrono_port(gfxw_visual_t *visual, gfxw_list_t **temp_widgets_list, int flags) {
	gfxw_port_t *result = NULL;
	gfx_color_t transparent = { PaletteEntry(), 0, -1, -1, 0};
	int id = 0;

	if (!(flags & GFXW_CHRONO_NON_TOPMOST)) {
		result = gfxw_find_default_port(visual);
	} else {
		id = visual->port_refs_nr;
		while (id >= 0 && (!visual->port_refs[id] || !visual->port_refs[id]->chrono_port))
			id--;

		if (id >= 0)
			result = visual->port_refs[id];
	}

	if (!result || !result->chrono_port) {
		if (flags & GFXW_CHRONO_NO_CREATE)
			return NULL;
		result = gfxw_new_port(visual, NULL, gfx_rect(0, 0, 320, 200), transparent, transparent);
		*temp_widgets_list = gfxw_new_list(gfx_rect(0, 0, 320, 200), 1);
		result->add(GFXWC(result), GFXW(*temp_widgets_list));
		result->chrono_port = 1;
		if (temp_widgets_list)
			*temp_widgets_list = GFXWC(result->contents);
		return result;
	};

	if (temp_widgets_list)
		*temp_widgets_list = GFXWC(result->contents);

	return result;
}

static int gfxw_check_chrono_overlaps(gfxw_port_t *chrono, gfxw_widget_t *widget) {
	gfxw_widget_t *seeker = GFXWC(chrono->contents)->contents;

	while (seeker) {
		if (toCommonRect(seeker->bounds).equals(toCommonRect(widget->bounds))) {
			gfxw_annihilate(GFXW(seeker));
			return 1;
		}

		seeker = seeker->next;
	}

	return 0;
}

void gfxw_add_to_chrono(gfxw_visual_t *visual, gfxw_widget_t *widget) {
	gfxw_list_t *tw;
	gfxw_port_t *chrono = gfxw_get_chrono_port(visual, &tw, 0);

	gfxw_check_chrono_overlaps(chrono, widget);
	chrono->add(GFXWC(chrono), widget);
}

static gfxw_widget_t *gfxw_widget_intersects_chrono(gfxw_list_t *tw, gfxw_widget_t *widget) {
	gfxw_widget_t *seeker;

	assert(tw->type == GFXW_SORTED_LIST);

	seeker = tw->contents;
	while (seeker) {
		Common::Point origin;
		rect_t bounds = widget->bounds;

		bounds = widget->bounds;
		origin.x = seeker->parent->zone.x;
		origin.y = seeker->parent->zone.y;
		bounds.x += origin.x;
		bounds.y += origin.y;

		if (gfx_rects_overlap(bounds, seeker->bounds))
			return seeker;

		seeker = seeker->next;
	}

	return 0;
}

void gfxw_widget_reparent_chrono(gfxw_visual_t *visual, gfxw_widget_t *view, gfxw_list_t *target) {
	gfxw_list_t *tw;
	gfxw_port_t *chrono;
	gfxw_widget_t *intersector;

	chrono = gfxw_get_chrono_port(visual, &tw, GFXW_CHRONO_NO_CREATE);
	if (chrono == NULL)
		return;

	intersector = gfxw_widget_intersects_chrono(tw, view);
	if (intersector) {
		Common::Point origin = Common::Point(intersector->parent->zone.x, intersector->parent->zone.y);

		gfxw_remove_widget_from_container(GFXWC(chrono), GFXW(tw));
		gfxw_remove_widget_from_container(GFXWC(chrono->parent), GFXW(chrono));
		gfxw_annihilate(GFXW(chrono));

		tw->zone.x += origin.x;
		tw->zone.y += origin.y;

		target->add(GFXWC(target), GFXW(tw));
	}
}

void gfxw_widget_kill_chrono(gfxw_visual_t *visual, int window) {
	int i;

	for (i = window; i < visual->port_refs_nr ; i++) {
		if (visual->port_refs[i] && visual->port_refs[i]->chrono_port)
			gfxw_annihilate(GFXW(visual->port_refs[i]));
	}
}

} // End of namespace Sci
