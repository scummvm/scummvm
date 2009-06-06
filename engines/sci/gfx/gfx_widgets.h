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

/* Graphical state management */

#ifndef SCI_GFX_GFX_WIDGETS_H
#define SCI_GFX_GFX_WIDGETS_H

#include "common/rect.h"

#include "sci/gfx/gfx_system.h"
#include "sci/gfx/operations.h"

namespace Sci {

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

GfxBox *gfxw_new_box(GfxState *state, rect_t area, gfx_color_t color1, gfx_color_t color2, gfx_box_shade_t shade_type);
/* Creates a new box
** Parameters: (GfxState *) state: The (optional) state
**             (rect_t) area: The box's dimensions, relative to its container widget
**             (gfx_color_t) color1: The primary color
**             (gfx_color_t) color1: The secondary color (ignored if shading is disabled)
**             (gfx_box_shade_t) shade_type: The shade type for the box
** Returns   : (GfxBox *) The resulting box widget
** The graphics state- if non-NULL- is used here for some optimizations.
*/

GfxPrimitive *gfxw_new_rect(rect_t rect, gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style);
/* Creates a new rectangle
** Parameters: (rect_t) rect: The rectangle area
**             (gfx_color_t) color: The rectangle's color
**             (gfx_line_mode_t) line_mode: The line mode for the lines that make up the rectangle
**             (gfx_line_style_t) line_style: The rectangle's lines' style
** Returns   : (GfxPrimitive *) The newly allocated rectangle widget (a Primitive)
*/

GfxPrimitive *gfxw_new_line(Common::Point start, Common::Point end, gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style);
/* Creates a new line
** Parameters: (Common::Point * Common::Point) (start, line): The line origin and end point
**             (gfx_color_t) color: The line's color
**             (gfx_line_mode_t) line_mode: The line mode to use for drawing
**             (gfx_line_style_t) line_style: The line style
** Returns   : (GfxPrimitive *) The newly allocated line widget (a Primitive)
*/


/* Whether the view should be static */
#define GFXW_VIEW_FLAG_STATIC (1 << 0)

/* Whether the view should _not_ apply its x/y offset modifyers */
#define GFXW_VIEW_FLAG_DONT_MODIFY_OFFSET (1 << 1)

GfxView *gfxw_new_view(GfxState *state, Common::Point pos, int view, int loop, int cel, int palette, int priority, int control,
	gfx_alignment_t halign, gfx_alignment_t valign, int flags);
/* Creates a new view (a cel, actually)
** Parameters: (GfxState *) state: The graphics state
**             (Common::Point) pos: The position to place the view at
**             (int x int x int) view, loop, cel: The global cel ID
**             (int) priority: The priority to use for drawing, or -1 for none
**             (int) control: The value to write to the control map, or -1 for none
**             (gfx_alignment_t x gfx_alignment_t) halign, valign: Horizontal and vertical
**                                                                 cel alignment
**             (int) flags: Any combination of GFXW_VIEW_FLAGs
** Returns   : (gfxw_cel_t *) A newly allocated cel according to the specs
*/

GfxDynView *gfxw_new_dyn_view(GfxState *state, Common::Point pos, int z, int view, int loop, int cel, int palette,
	int priority, int control, gfx_alignment_t halign, gfx_alignment_t valign, int sequence);
/* Creates a new dyn view
** Parameters: (GfxState *) state: The graphics state
**             (Common::Point) pos: The position to place the dynamic view at
**             (int) z: The z coordinate
**             (int x int x int) view, loop, cel: The global cel ID
**             (int) priority: The priority to use for drawing, or -1 for none
**             (int) control: The value to write to the control map, or -1 for none
**             (gfx_alignment_t x gfx_alignment_t) halign, valign: Horizontal and vertical
**                                                                 cel alignment
**             (int) sequence: Sequence number: When sorting dynviews, this number is
**                             considered last for sorting (ascending order)
** Returns   : (gfxw_cel_t *) A newly allocated cel according to the specs
** Dynamic views are non-pic views with a unique global identifyer. This allows for drawing
** optimizations when they move or change shape.
*/

GfxText *gfxw_new_text(GfxState *state, rect_t area, int font, const char *text, gfx_alignment_t halign,
	gfx_alignment_t valign, gfx_color_t color1, gfx_color_t color2,
	gfx_color_t bgcolor, int flags);
/* Creates a new text widget
** Parameters: (GfxState *) state: The state the text is to be calculated from
**             (rect_t) area: The area the text is to be confined to (the yl value is only
**                            relevant for text aligment, though)
**             (int) font: The number of the font to use
**             (gfx_alignment_t x gfx_alignment_t) halign, valign: Horizontal and
**                                                                 vertical text alignment
**             (gfx_color_t x gfx_color_t) color1, color2: Text foreground colors (if not equal,
**                                                         The foreground is dithered between them)
**             (gfx_color_t) bgcolor: Text background color
**             (int) flags: GFXR_FONT_FLAGs, orred together (see gfx_resource.h)
** Returns   : (GfxText *) The resulting text widget
*/

void gfxw_text_info(GfxState *state, GfxText *text, int *lines_nr,
	int *lineheight, int *offset);
/* Determines text widget meta-information
** Parameters: (GfxState *) state: The state to operate on
**             (gfx_text_t *) text: The widget to query
** Returns   : (int) lines_nr: Number of lines used in the text
**             (int) lineheight: Pixel height (SCI scale) of each text line
**             (int) offset: Pixel offset (SCI scale) of the space after the
**                           last character in the last line
*/

GfxWidget *gfxw_set_id(GfxWidget *widget, int ID, int subID);
/* Sets a widget's ID
** Parmaeters: (GfxWidget *) widget: The widget whose ID should be set
**             (int x int) ID, subID: The ID to set
** Returns   : (GfxWidget *) widget
** A widget ID is unique within the container it is stored in, if and only if it was
** added to that container with gfxw_add().
** This function handles widget = NULL gracefully (by doing nothing and returning NULL).
*/

GfxWidget *gfxw_remove_id(GfxContainer *container, int ID, int subID);
/* Finds a widget with a specific ID in a container and removes it from there
** Parameters: (GfxContainer *) container: The container to search in
**             (int) ID: The ID to look for
**             (int) subID: The subID to look for, or GFXW_NO_ID for any
** Returns   : (GfxWidget *) The resulting widget or NULL if no match was found
** Search is non-recursive; widgets with IDs hidden in subcontainers will not be found.
*/


GfxDynView *gfxw_dyn_view_set_params(GfxDynView *widget, int under_bits, const ObjVarRef& under_bitsp, int signal, const ObjVarRef& signalp);
/* Initializes a dyn view's interpreter attributes
** Parameters: (GfxDynView *) widget: The widget affected
**             (int x void * x int x void *) under_bits, inder_bitsp, signal, signalp: Interpreter-dependant data
** Returns   : (GfxDynView *) widget
*/

GfxWidget *gfxw_hide_widget(GfxWidget *widget);
/* Makes a widget invisible without removing it from the list of widgets
** Parameters: (GfxWidget *) widget: The widget to invisibilize
** Returns   : (GfxWidget *) widget
** Has no effect on invisible widgets
*/

GfxWidget *gfxw_show_widget(GfxWidget *widget);
/* Makes an invisible widget reappear
** Parameters: (GfxWidget *) widget: The widget to show again
** Returns   : (GfxWidget *) widget
** Does not affect visible widgets
*/

GfxWidget *gfxw_abandon_widget(GfxWidget *widget);
/* Marks a widget as "abandoned"
** Parameters: (GfxWidget *) widget: The widget to abandon
** Returns   : (GfxWidget *) widget
*/

/*-- Container types --*/

#define GFXW_LIST_UNSORTED 0
#define GFXW_LIST_SORTED 1

GfxList *gfxw_new_list(rect_t area, int sorted);
/* Creates a new list widget
** Parameters: (rect_t) area: The area covered by the list (absolute position)
**             (int) sorted: Whether the list should be a sorted list
** Returns   : (GfxList *) A newly allocated list widget
** List widgets are also referred to as Display Lists.
*/

GfxPort *gfxw_find_default_port(GfxVisual *visual);
/* Retrieves the default port from a visual
** Parameters: (GfxVisual *) visual: The visual the port should be retrieved from
** Returns   : (GfxPort *) The default port, or NULL if no port is present
** The 'default port' is the last port to be instantiated; usually the topmost
** or highest-ranking port.
*/

void gfxw_port_set_auto_restore(GfxVisual *visual, GfxPort *window, rect_t auto_rect);
/* Sets rectangle to be restored upon port removal
** Parameters: (state_t *) s: The state to operate on
**             (GfxPort *) window: The affected window
**             (rect_t) auto_rect: The area to restore
** Returns   : (void)
*/

GfxPort *gfxw_remove_port(GfxVisual *visual, GfxPort *port);
/* Removes a port from a visual
** Parameters: (GfxVisual *) visual: The visual the port should be removed from
**             (GfxPort *) port: The port to remove
** Returns   : (GfxPort *) port's parent port, or NULL if it had none
*/

void gfxw_remove_widget_from_container(GfxContainer *container, GfxWidget *widget);
/* Removes the widget from the specified port
** Parameters: (GfxContainer *) container: The container it should be removed from
**             (GfxWidget *) widget: The widget to remove
** Returns   : (void)
*/

gfxw_snapshot_t *gfxw_make_snapshot(GfxVisual *visual, rect_t area);
/* Makes a "snapshot" of a visual
** Parameters: (GfxVisual *) visual: The visual a snapshot is to be taken of
**             (rect_t) area: The area a snapshot should be taken of
** Returns   : (gfxw_snapshot_t *) The resulting, newly allocated snapshot
** It's not really a full qualified snaphot, though. See gfxw_restore_snapshot
** for a full discussion.
** This operation also increases the global serial number counter by one.
*/

int gfxw_widget_matches_snapshot(gfxw_snapshot_t *snapshot, GfxWidget *widget);
/* Predicate to test whether a widget would be destroyed by applying a snapshot
** Parameters: (gfxw_snapshot_t *) snapshot: The snapshot to test against
**             (GfxWidget *) widget: The widget to test
** Retunrrs  : (int) An appropriate boolean value
*/

gfxw_snapshot_t *gfxw_restore_snapshot(GfxVisual *visual, gfxw_snapshot_t *snapshot);
/* Restores a snapshot to a visual
** Parameters: (GfxVisual *) visual: The visual to operate on
**             (gfxw_snapshot_t *) snapshot: The snapshot to restore
** Returns   : (gfxw_snapshot_t *) snapshot (still needs to be freed)
** The snapshot is not really restored; only more recent widgets touching
** the snapshotted area are destroyed.
*/

void gfxw_annihilate(GfxWidget *widget);
/* As widget->widfree(widget), but destroys all overlapping widgets
** Parameters: (GfxWidget *) widget: The widget to use
** Returns   : (void)
** This operation calls widget->widfree(widget), but it also destroys
** all widgets with a higher or equal priority drawn after this widget.
*/

GfxDynView *gfxw_picviewize_dynview(GfxDynView *dynview);
/* Turns a dynview into a picview
** Parameters: (GfxDynView *) dynview: The victim
** Returns   : (GfxDynView *) The victim, after his transformation
** The only changes are in function and type variables, actually.
*/

void gfxw_port_auto_restore_background(GfxVisual *visual, GfxPort *window, rect_t auto_rect);
/* Tags a window widget as automatically restoring the visual background upon removal
** Parameters: (gfx_visual_t *) visual: The base visual
**             (GfxPort *) window: The window to tag
**             (rect_t) auto_rect: The background to remember
** Also records the specified background rectangle, for later recovery
*/

} // End of namespace Sci

#endif // SCI_GFX_GFX_WIDGETS_H
