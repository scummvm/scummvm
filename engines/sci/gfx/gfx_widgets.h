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


#ifndef SCI_GFX_GFX_WIDGETS_H
#define SCI_GFX_GFX_WIDGETS_H

#include "common/rect.h"

#include "sci/gfx/gfx_system.h"
#include "sci/gfx/operations.h"

namespace Sci {
/** @name Widget Graphical State Management */
/** @{ */

struct GfxState;
struct GfxBox;
struct GfxDynView;
struct GfxContainer;
struct GfxList;
struct GfxPort;
struct GfxPrimitive;
struct gfxw_snapshot_t;
struct GfxText;
struct GfxView;
struct GfxVisual;
struct GfxWidget;


/* Enable the next line to keep a list of pointers to all widgets, with up to the specified amount
** of members (/SLOW/) */
//#define GFXW_DEBUG_WIDGETS 2048

/* Our strategy for dirty rectangle management */
#define GFXW_DIRTY_STRATEGY GFXOP_DIRTY_FRAMES_CLUSTERS

/* Terminology
**
** Two special terms are used in here: /equivalent/ and /clear/. Their meanings
** in this context are as follows:
**
** /clear/: Clearing a widget means overwriting the space it occupies in the back
** buffer with data from the static buffer. This affects both the visual and the
** priority buffer, the static buffer (and any effect the widget may have had on
** it) is not touched.
**
** /equivalent/: Two Widgets A and B are equivalent if and only if either of the
** following conditions is met:
** a) Both A and B are text widgets, and they occupy the same bounding rectangle.
** b) Both A and B are dynview widgets, and they have the same unique ID
** Note that /equivalent/ is not really an equivalence relation- while it is ob-
** viously transitive and symmetrical, it is not reflexive (e.g. a box widget
** is not /equivalent/ to itself), although this might be a nice addition for the
** future.
*/


/*********************************/
/* Fundamental widget operations */
/*********************************/


/* gfxw_point_zero is declared in gfx/widgets.cpp */
extern Common::Point gfxw_point_zero;

/*********************/
/* Widget operations */
/*********************/

/* These are for documentation purposes only. The actual declarations are in
** gfx_state_internal.h.
**
**
** **************************
** ** Container operations **
** **************************
**
**
** -- free_tagged(GfxContainer *self)
** Frees all tagged resources in the container
** Parameters: (GfxContainer *) self: self reference
** Returns   : (int) 0
** The container itself is never freed in this way.
**
**
** -- free_contents(GfxContainer *self)
** Frees all resources contained in the container
** Parameters: (GfxContainer *) self: self reference
** Returns   : (int) 0
**
**
** -- add_dirty_abs(GfxContainer *self, rect_t dirty, int propagate)
** Adds a dirty rectangle to the container's list of dirty rects
** Parameters: (GfxContainer *) self: self reference
**             (rect_t) dirty: The rectangular screen area that is to be flagged
**                             as dirty, absolute to the screen
**             (int) propagate: Whether the dirty rect should be propagated to the
**                              widget's parents
** Returns   : (int) 0
** Transparent containers will usually pass this value to their next ancestor,
** because areas below them might have to be redrawn.
** The dirty rectangle management strategy is defined in this file in
** GFXW_DIRTY_STRATEGY.
**
**
** -- add_dirty_rel(GfxContainer *self, rect_t dirty, int propagate)
** Adds a dirty rectangle to the container's list of dirty rects
** Parameters: (GfxContainer *) self: self reference
**             (rect_t) dirty: The rectangular screen area that is to be flagged
**                             as dirty, relative to the widget
**             (int) propagate: Whether the dirty rect should be propagated to the
**                              widget's parents
** Returns   : (int) 0
** Transparent containers will usually pass this value to their next ancestor,
** because areas below them might have to be redrawn.
** The dirty rectangle management strategy is defined in this file in
** GFXW_DIRTY_STRATEGY.
**
**
** -- add(GfxContainer *self, GfxWidget *widget)
** Adds a widget to the list of contained widgets
** Parameters: (GfxContainer *) self: self reference
**             (GfxWidget *) widget: The widget to add
** Returns   : (int) 0
** Sorted lists sort their content into the list rather than adding it to the
** end.
*/


/***************************/
/* Basic widget generation */
/***************************/

/*-- Primitive types --*/

/**
 * Creates a new box
 *
 * The graphics state, if non-NULL, is used here for some optimizations.
 *
 * @param[in] state			The (optional) state
 * @param[in] area			The box's dimensions, relative to its container
 * 							widget
 * @param[in] color1		The primary color
 * @param[in] color2		The secondary color (ignored if shading is disabled)
 * @param[in] shade_type	The shade type for the box
 * @return					The resulting box widget
 */
GfxBox *gfxw_new_box(GfxState *state, rect_t area, gfx_color_t color1, gfx_color_t color2, gfx_box_shade_t shade_type);

/**
 * Creates a new rectangle
 *
 * @param[in] rect			The rectangle area
 * @param[in] color			The rectangle's color
 * @param[in] line_mode		The line mode for the lines that make up the
 * 							rectangle
 * @param[in] line_style	The rectangle's lines' style
 * @return					The newly allocated rectangle widget (a Primitive)
 */
GfxPrimitive *gfxw_new_rect(rect_t rect, gfx_color_t color,
	gfx_line_mode_t line_mode, gfx_line_style_t line_style);

/**
 * Creates a new line
 *
 * @param[in] start			The line's origin
 * @param[in] end			The line's end point
 * @param[in] color			The line's color
 * @param[in] line_mode		The line mode to use for drawing
 * @param[in] line_style	The line style
 * @return					The newly allocated line widget (a Primitive)
 */
GfxPrimitive *gfxw_new_line(Common::Point start, Common::Point end,
	gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style);

/** View flags */
enum {
	GFXW_VIEW_FLAG_STATIC = (1 << 0), /**< Whether the view should be static */
	GFXW_VIEW_FLAG_DONT_MODIFY_OFFSET = (1 << 1) /**< Whether the view should _not_ apply its x/y offset modifyers */
};

/**
 * Creates a new view (a cel, actually)
 *
 * @param[in] state		The graphics state
 * @param[in] pos		The position to place the view at
 * @param[in] view		The global cel ID
 * @param[in] loop		The global cel ID
 * @param[in] cel		The global cel ID
 * @param[in] palette	The palette to use
 * @param[in] priority	The priority to use for drawing, or -1 for none
 * @param[in] control	The value to write to the control map, or -1 for none
 * @param[in] halign	Horizontal cel alignment
 * @param[in] valign	Vertical cel alignment
 * @param[in] flags		Any combination of GFXW_VIEW_FLAGs
 * @return				A newly allocated cel according to the specs
 */
GfxView *gfxw_new_view(GfxState *state, Common::Point pos, int view, int loop,
	int cel, int palette, int priority, int control, gfx_alignment_t halign,
	gfx_alignment_t valign, int flags);


/**
 * Creates a new dyn view
 *
 * Dynamic views are non-pic views with a unique global identifyer. This allows for drawing optimizations when they move or change shape.
 *
 * @param[in] state		The graphics state
 * @param[in] pos		The position to place the dynamic view at
 * @param[in] z			The z coordinate
 * @param[in] view		The global cel ID
 * @param[in] loop		The global cel ID
 * @param[in] cel		The global cel ID
 * @param[in] palette	The palette to use
 * @param[in] priority	The priority to use for drawing, or -1 for none
 * @param[in] control	The value to write to the control map, or -1 for none
 * @param[in] halign	Horizontal cel alignment
 * @param[in] valign	Vertical cel alignment
 * @param[in] sequence	Sequence number: When sorting dynviews, this number is
 * 						considered last for sorting (ascending order)
 * @return				A newly allocated cel according to the specs
 */
GfxDynView *gfxw_new_dyn_view(GfxState *state, Common::Point pos, int z,
	int view, int loop, int cel, int palette, int priority, int control,
	gfx_alignment_t halign, gfx_alignment_t valign, int sequence);

/**
 * Creates a new text widget
 *
 * @param[in] state		The state the text is to be calculated from
 * @param[in] area		The area the text is to be confined to (the yl value is
 * 						only relevant for text aligment, though)
 * @param[in] font		The number of the font to use
 * @param[in] text		String to put in text widget
 * @param[in] halign	Horizontal text alignment
 * @param[in] valign	Vertical text alignment
 * @param[in] color1	Text foreground colors (if not equal, the foreground is
 * 						dithered between them)
 * @param[in] color2	Text foreground colors (if not equal, the foreground is
 * 						dithered between them)
 * @param[in] bgcolor	Text background color
 * @param[in] flags		GFXR_FONT_FLAGs, orred together (see gfx_resource.h)
 * @return				The resulting text widget
 */
GfxText *gfxw_new_text(GfxState *state, rect_t area, int font, const char *text,
	gfx_alignment_t halign, gfx_alignment_t valign, gfx_color_t color1,
	gfx_color_t color2,	gfx_color_t bgcolor, int flags);

/**
 * Determines text widget meta-information
 *
 * @param[in]  state		The state to operate on
 * @param[in]  text			The widget to query
 * @param[out] lines_nr		Number of lines used in the text
 * @param[out] lineheight	Pixel height (SCI scale) of each text line
 * @param[out] offset		Pixel offset (SCI scale) of the space after the last
 * 							character in the last line
 */
void gfxw_text_info(GfxState *state, GfxText *text, int *lines_nr,
	int *lineheight, int *offset);

/**
 * Sets a widget's ID
 *
 * A widget ID is unique within the container it is stored in, if and only if it
 * was added to that container with gfxw_add(). This function handles widget ==
 * NULL gracefully (by doing nothing and returning NULL).
 *
 * @param[in] widget	The widget whose ID should be set
 * @param[in] ID		The ID to set
 * @param[in] subID		The ID to set
 * @return				The widget
 */
GfxWidget *gfxw_set_id(GfxWidget *widget, int ID, int subID);

/**
 * Finds a widget with a specific ID in a container and removes it from there
 *
 * Search is non-recursive; widgets with IDs hidden in subcontainers will not
 * be found.
 *
 * @param[in] container	The container to search in
 * @param[in] ID		The ID to look for
 * @param[in] subID		The subID to look for, or GFXW_NO_ID for any
 * @return				The resulting widget or NULL if no match was found
 */
GfxWidget *gfxw_remove_id(GfxContainer *container, int ID, int subID);

/**
 * Initializes a dyn view's interpreter attributes
 *
 * @param[in] widget		The widget affected
 * @param[in] under_bits	Interpreter-dependant data
 * @param[in] under_bitsp	Interpreter-dependant data
 * @param[in] signal		Interpreter-dependant data
 * @param[in] signalp		Interpreter-dependant data
 * @return					The widget
 */
GfxDynView *gfxw_dyn_view_set_params(GfxDynView *widget, int under_bits,
	const ObjVarRef& under_bitsp, int signal, const ObjVarRef& signalp);

/**
 * Makes a widget invisible without removing it from the list of widgets
 *
 * Has no effect on invisible widgets
 *
 * @param[in] widget	The widget to invisibilize
 * @return				The widget
 */
GfxWidget *gfxw_hide_widget(GfxWidget *widget);

/**
 * Makes an invisible widget reappear
 *
 * Does not affect visible widgets
 *
 * @param[in] widget	The widget to show again
 * @return				The widget
 */
GfxWidget *gfxw_show_widget(GfxWidget *widget);

/**
 * Marks a widget as "abandoned"
 *
 * @param[in] widget	The widget to abandon
 * @return				The widget
 */
GfxWidget *gfxw_abandon_widget(GfxWidget *widget);

/** Container types */
enum {
	GFXW_LIST_UNSORTED = 0,
	GFXW_LIST_SORTED = 1
};

/**
 * Creates a new list widget
 *
 * List widgets are also referred to as Display Lists.
 *
 * @param[in] area		The area covered by the list (absolute position)
 * @param[in] sorted	Whether the list should be a sorted list
 * @return				A newly allocated list widget
 */
GfxList *gfxw_new_list(rect_t area, int sorted);

/**
 * Retrieves the default port from a visual
 *
 * The 'default port' is the last port to be instantiated; usually the topmost
 * or highest-ranking port.
 *
 * @param[in] visual	The visual the port should be retrieved from
 * @return				The default port, or NULL if no port is present
 */
GfxPort *gfxw_find_default_port(GfxVisual *visual);

/**
 * Sets rectangle to be restored upon port removal
 *
 * @param[in] visual	The visual to operate on
 * @param[in] window	The affected window
 * @param[in] auto_rect	The area to restore
 */
void gfxw_port_set_auto_restore(GfxVisual *visual, GfxPort *window, rect_t auto_rect);

/**
 * Removes a port from a visual
 *
 * @param[in] visual	The visual the port should be removed from
 * @param[in] port		The port to remove
 * @return				port's parent port, or NULL if it had none
 */
GfxPort *gfxw_remove_port(GfxVisual *visual, GfxPort *port);

/**
 * Removes the widget from the specified port
 *
 * @param[in] container	The container it should be removed from
 * @param[in] widget	The widget to remove
 */
void gfxw_remove_widget_from_container(GfxContainer *container, GfxWidget *widget);

/**
 * Makes a "snapshot" of a visual
 *
 * It's not really a full qualified snaphot, though. See gfxw_restore_snapshot
 * for a full discussion. This operation also increases the global serial number
 * counter by one.
 *
 * @param[in] visual	The visual a snapshot is to be taken of
 * @param[in] area		The area a snapshot should be taken of
 * @return				The resulting, newly allocated snapshot
 */
gfxw_snapshot_t *gfxw_make_snapshot(GfxVisual *visual, rect_t area);

/**
 * Predicate to test whether a widget would be destroyed by applying a snapshot
 *
 * @param[in] snapshot	The snapshot to test against
 * @param[in] widget	The widget to test
 * @return				An appropriate boolean value
 */
int gfxw_widget_matches_snapshot(gfxw_snapshot_t *snapshot, GfxWidget *widget);

/**
 * Restores a snapshot to a visual
 *
 * The snapshot is not really restored; only more recent widgets touching
 * the snapshotted area are destroyed.
 *
 * @param[in] visual	The visual to operate on
 * @param[in] snapshot	The snapshot to restore
 * @return				The snapshot (still needs to be freed)
 */
gfxw_snapshot_t *gfxw_restore_snapshot(GfxVisual *visual, gfxw_snapshot_t *snapshot);

/**
 * As widget->widfree(widget), but destroys all overlapping widgets
 *
 * This operation calls widget->widfree(widget), but it also destroys all
 * widgets with a higher or equal priority drawn after this widget.
 *
 * @param[in] widget	The widget to use
 */
void gfxw_annihilate(GfxWidget *widget);

/**
 * Turns a dynview into a picview
 *
 * The only changes are in function and type variables, actually.
 *
 * @param[in] dynview	The victim
 * @return				The victim, after his transformation
 */
GfxDynView *gfxw_picviewize_dynview(GfxDynView *dynview);

/**
 * Tags a window widget as automatically restoring the visual background
 * upon removal.
 *
 * Also records the specified background rectangle, for later recovery.
 *
 * @param[in] visual	The base visual
 * @param[in] window	The window to tag
 * @param[in] auto_rect	The background to remember

 */
void gfxw_port_auto_restore_background(GfxVisual *visual, GfxPort *window,
	rect_t auto_rect);

/** @} */
} // End of namespace Sci

#endif // SCI_GFX_GFX_WIDGETS_H
