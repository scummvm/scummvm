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

struct gfx_state_t;
struct gfxw_box_t;
struct gfxw_dyn_view_t;
struct gfxw_container_t;
typedef gfxw_container_t gfxw_list_t;
struct gfxw_port_t;
struct gfxw_primitive_t;
struct gfxw_snapshot_t;
struct gfxw_text_t;
struct gfxw_view_t;
struct gfxw_visual_t;
struct gfxw_widget_t;


/* Enable the next line to keep a list of pointers to all widgets, with up to the specified amount
** of members (/SLOW/) */
/* #define GFXW_DEBUG_WIDGETS 2048 */

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


#define GFXW(foo) ((gfxw_widget_t *) foo)
/* Typecast an arbitrary widget to gfxw_widget_t*. Might eventually be changed to do tests as well. */

#define GFXWC(foo) ((gfxw_container_t *) foo)
/* Typecasts a container widget to gfxw_container_widget_t *. */

/* gfxw_point_zero is declared in gfx/widgets.c */
extern Common::Point gfxw_point_zero;

/*********************/
/* Widget operations */
/*********************/

/* These are for documentation purposes only. The actual declarations are in
** gfx_state_internal.h.
**
**
**
** -- draw(gfxw_widget_t *self, Common::Point pos)
** Draws the widget.
** Parameters: (gfxw_widget_t *) self: self reference
**             (Common::Point) pos: The position to draw to (added to the widget's
**                            internal position)
** Returns   : (int) 0
** The widget is drawn iff it is flagged as dirty. Invoking this operation on
** a container widget will recursively draw all of its contents.
**
**
** -- widfree(gfxw_widget_t *self)
** Frees all memory associated to the widget
** Parameters: (gfxw_widget_t *) self: self reference
** Returns   : (int) 0
** The widget automatically removes itself from its owner, if it has one.
** Invoking this operation on a container will recursively free all of its
** contents.
**
**
** -- tag(gfxw_widget_t *self)
** Tags the specified widget
** Parameters: (gfxw_widget_t *) self: self reference
** Returns   : (int) 0
** If invoked on a container widget, this will also tag all of the container's
** contents (but not the contents' contents!)
**
**
** -- print(gfxw_widget_t *self, int indentation)
** Prints a string representation of the widget with sciprintf
** Parameters: (gfxw_widget_t *) self: self reference
**             (int) indentation: Number of double spaces to indent
** Returns   ; (int) 0
** Will recursively print all of the widget's contents if the widget contains
** further sub-widgets
**
**
** -- compare_to(gfxw_widget_t *self, gfxw_widget_t *other)
** Compares two compareable widgets by their screen position
** Parameters: (gfxw_widget_t *) self: self reference
**             (gfxw_widget_t *) other: other widget
** Returns   : (int) <0, 0, or >0 if other is, respectively, less than, equal
**                   to, or greater than self
** This comparison only applies to some widgets; compare_to(a,a)=0 is not
** guaranteed. It may be used for sorting for all widgets.
**
**
** -- equals(gfxw_widget_t *self, gfxw_widget_t *other)
** Compares two compareable widgets for equality
** Parameters: (gfxw_widget_t *) self: self reference
**             (gfxw_widget_t *) other: other widget
** Returns   : (int) 0 if the widgets are not equal, != 0 if they match
** This operation checks whether two widgets describe the same graphical data.
** It is used to determine whether a new widget should be discarded because it
** describes the same graphical data as an old widget that has already been
** drawn. For lists, it also checks whether all contents are in an identical
** order.
**
**
** -- should_replace(gfxw_widget_t *self, gfxw_widget_t *other)
** Compares two compareable widgets for equality
** Parameters: (gfxw_widget_t *) self: self reference
**             (gfxw_widget_t *) other: other widget
** Returns   : (int) 0 if 'self' should be kept, != 0 if it should be replaced
**             by the 'other'
** When 'equals' returns true, this means that no new widget will be added.
** However, in some cases newer widgets may contain information that should
** cause the older widget to be removed nonetheless; this is indicated by this
** function.
**
**
** -- superarea_of(gfxw_widget_t *self, gfxw_widget_t *other)
** Tests whether drawing self after other would reduce all traces of other
** Parameters: (gfxw_widget_t *) self: self reference
**             (gxfw_widget_t *) other: The widget to compare for containment
** Returns   : (int) 1 if self is superarea_of other, 0 otherwise
**
**
** -- set_visual(gfxw_widget_t *self)
** Sets the visual for the widget
** Parameters: (gfxw_widget_t *) self: self reference
** Returns   : (int) 0
** This function is called by container->add() and need not be invoked explicitly.
** It also makes sure that dirty rectangles are passed to parent containers.
**
**
**
** **************************
** ** Container operations **
** **************************
**
**
** -- free_tagged(gfxw_container_t *self)
** Frees all tagged resources in the container
** Parameters: (gfxw_container_t *) self: self reference
** Returns   : (int) 0
** The container itself is never freed in this way.
**
**
** -- free_contents(gfxw_container_t *self)
** Frees all resources contained in the container
** Parameters: (gfxw_container_t *) self: self reference
** Returns   : (int) 0
**
**
** -- add_dirty_abs(gfxw_container_t *self, rect_t dirty, int propagate)
** Adds a dirty rectangle to the container's list of dirty rects
** Parameters: (gfxw_container_t *) self: self reference
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
** -- add_dirty_rel(gfxw_container_t *self, rect_t dirty, int propagate)
** Adds a dirty rectangle to the container's list of dirty rects
** Parameters: (gfxw_container_t *) self: self reference
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
** -- add(gfxw_container_t *self, gfxw_widget_t *widget)
** Adds a widget to the list of contained widgets
** Parameters: (gfxw_container_t *) self: self reference
**             (gfxw_widget_t *) widget: The widget to add
** Returns   : (int) 0
** Sorted lists sort their content into the list rather than adding it to the
** end.
*/


/***************************/
/* Basic widget generation */
/***************************/

/*-- Primitive types --*/

gfxw_box_t *gfxw_new_box(gfx_state_t *state, rect_t area, gfx_color_t color1, gfx_color_t color2, gfx_box_shade_t shade_type);
/* Creates a new box
** Parameters: (gfx_state_t *) state: The (optional) state
**             (rect_t) area: The box's dimensions, relative to its container widget
**             (gfx_color_t) color1: The primary color
**             (gfx_color_t) color1: The secondary color (ignored if shading is disabled)
**             (gfx_box_shade_t) shade_type: The shade type for the box
** Returns   : (gfxw_box_t *) The resulting box widget
** The graphics state- if non-NULL- is used here for some optimizations.
*/

gfxw_primitive_t *gfxw_new_rect(rect_t rect, gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style);
/* Creates a new rectangle
** Parameters: (rect_t) rect: The rectangle area
**             (gfx_color_t) color: The rectangle's color
**             (gfx_line_mode_t) line_mode: The line mode for the lines that make up the rectangle
**             (gfx_line_style_t) line_style: The rectangle's lines' style
** Returns   : (gfxw_primitive_t *) The newly allocated rectangle widget (a Primitive)
*/

gfxw_primitive_t *gfxw_new_line(Common::Point start, Common::Point end, gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style);
/* Creates a new line
** Parameters: (Common::Point * Common::Point) (start, line): The line origin and end point
**             (gfx_color_t) color: The line's color
**             (gfx_line_mode_t) line_mode: The line mode to use for drawing
**             (gfx_line_style_t) line_style: The line style
** Returns   : (gfxw_primitive_t *) The newly allocated line widget (a Primitive)
*/


/* Whether the view should be static */
#define GFXW_VIEW_FLAG_STATIC (1 << 0)

/* Whether the view should _not_ apply its x/y offset modifyers */
#define GFXW_VIEW_FLAG_DONT_MODIFY_OFFSET (1 << 1)

gfxw_view_t *gfxw_new_view(gfx_state_t *state, Common::Point pos, int view, int loop, int cel, int palette, int priority, int control,
	gfx_alignment_t halign, gfx_alignment_t valign, int flags);
/* Creates a new view (a cel, actually)
** Parameters: (gfx_state_t *) state: The graphics state
**             (Common::Point) pos: The position to place the view at
**             (int x int x int) view, loop, cel: The global cel ID
**             (int) priority: The priority to use for drawing, or -1 for none
**             (int) control: The value to write to the control map, or -1 for none
**             (gfx_alignment_t x gfx_alignment_t) halign, valign: Horizontal and vertical
**                                                                 cel alignment
**             (int) flags: Any combination of GFXW_VIEW_FLAGs
** Returns   : (gfxw_cel_t *) A newly allocated cel according to the specs
*/

gfxw_dyn_view_t *gfxw_new_dyn_view(gfx_state_t *state, Common::Point pos, int z, int view, int loop, int cel, int palette,
	int priority, int control, gfx_alignment_t halign, gfx_alignment_t valign, int sequence);
/* Creates a new dyn view
** Parameters: (gfx_state_t *) state: The graphics state
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

gfxw_text_t *gfxw_new_text(gfx_state_t *state, rect_t area, int font, const char *text, gfx_alignment_t halign,
	gfx_alignment_t valign, gfx_color_t color1, gfx_color_t color2,
	gfx_color_t bgcolor, int flags);
/* Creates a new text widget
** Parameters: (gfx_state_t *) state: The state the text is to be calculated from
**             (rect_t) area: The area the text is to be confined to (the yl value is only
**                            relevant for text aligment, though)
**             (int) font: The number of the font to use
**             (gfx_alignment_t x gfx_alignment_t) halign, valign: Horizontal and
**                                                                 vertical text alignment
**             (gfx_color_t x gfx_color_t) color1, color2: Text foreground colors (if not equal,
**                                                         The foreground is dithered between them)
**             (gfx_color_t) bgcolor: Text background color
**             (int) flags: GFXR_FONT_FLAGs, orred together (see gfx_resource.h)
** Returns   : (gfxw_text_t *) The resulting text widget
*/

void gfxw_text_info(gfx_state_t *state, gfxw_text_t *text, int *lines_nr,
	int *lineheight, int *offset);
/* Determines text widget meta-information
** Parameters: (gfx_state_t *) state: The state to operate on
**             (gfx_text_t *) text: The widget to query
** Returns   : (int) lines_nr: Number of lines used in the text
**             (int) lineheight: Pixel height (SCI scale) of each text line
**             (int) offset: Pixel offset (SCI scale) of the space after the
**                           last character in the last line
*/

gfxw_widget_t *gfxw_set_id(gfxw_widget_t *widget, int ID, int subID);
/* Sets a widget's ID
** Parmaeters: (gfxw_widget_t *) widget: The widget whose ID should be set
**             (int x int) ID, subID: The ID to set
** Returns   : (gfxw_widget_t *) widget
** A widget ID is unique within the container it is stored in, if and only if it was
** added to that container with gfxw_add().
** This function handles widget = NULL gracefully (by doing nothing and returning NULL).
*/

gfxw_widget_t *gfxw_remove_id(gfxw_container_t *container, int ID, int subID);
/* Finds a widget with a specific ID in a container and removes it from there
** Parameters: (gfxw_container_t *) container: The container to search in
**             (int) ID: The ID to look for
**             (int) subID: The subID to look for, or GFXW_NO_ID for any
** Returns   : (gfxw_widget_t *) The resulting widget or NULL if no match was found
** Search is non-recursive; widgets with IDs hidden in subcontainers will not be found.
*/


gfxw_dyn_view_t *gfxw_dyn_view_set_params(gfxw_dyn_view_t *widget, int under_bits, void *under_bitsp, int signal, void *signalp);
/* Initializes a dyn view's interpreter attributes
** Parameters: (gfxw_dyn_view_t *) widget: The widget affected
**             (int x void * x int x void *) under_bits, inder_bitsp, signal, signalp: Interpreter-dependant data
** Returns   : (gfxw_dyn_view_t *) widget
*/

gfxw_widget_t *gfxw_hide_widget(gfxw_widget_t *widget);
/* Makes a widget invisible without removing it from the list of widgets
** Parameters: (gfxw_widget_t *) widget: The widget to invisibilize
** Returns   : (gfxw_widget_t *) widget
** Has no effect on invisible widgets
*/

gfxw_widget_t *gfxw_show_widget(gfxw_widget_t *widget);
/* Makes an invisible widget reappear
** Parameters: (gfxw_widget_t *) widget: The widget to show again
** Returns   : (gfxw_widget_t *) widget
** Does not affect visible widgets
*/

gfxw_widget_t *gfxw_abandon_widget(gfxw_widget_t *widget);
/* Marks a widget as "abandoned"
** Parameters: (gfxw_widget_t *) widget: The widget to abandon
** Returns   : (gfxw_widget_t *) widget
*/

/*-- Container types --*/

#define GFXW_LIST_UNSORTED 0
#define GFXW_LIST_SORTED 1

gfxw_list_t *gfxw_new_list(rect_t area, int sorted);
/* Creates a new list widget
** Parameters: (rect_t) area: The area covered by the list (absolute position)
**             (int) sorted: Whether the list should be a sorted list
** Returns   : (gfxw_list_t *) A newly allocated list widget
** List widgets are also referred to as Display Lists.
*/

gfxw_visual_t *gfxw_new_visual(gfx_state_t *state, int font);
/* Creates a new visual widget
** Parameters: (gfx_state_t *) state: The graphics state
**             (int) font: The default font number for contained ports
** Returns   : (gfxw_list_t *) A newly allocated visual widget
** Visual widgets are containers for port widgets.
*/


gfxw_port_t *gfxw_new_port(gfxw_visual_t *visual, gfxw_port_t *predecessor, rect_t area, gfx_color_t fgcolor, gfx_color_t bgcolor);
/* Creates a new port widget with the default settings
** Paramaters: (gfxw_visual_t *) visual: The visual the port is added to
**             (gfxw_port_t *) predecessor: The port's predecessor
**             (rect_t) area: The screen area covered by the port (absolute position)
**             (gfx_color_t) fgcolor: Foreground drawing color
**             (gfx_color_t) bgcolor: Background color
** Returns   : (gfxw_port_t *) A newly allocated port widget
** A port differentiates itself from a list in that it contains additional information,
** and an optional title (stored in a display list).
** Ports are assigned implicit IDs identifying their position within the port stack.
*/

gfxw_port_t *gfxw_find_port(gfxw_visual_t *visual, int ID);
/* Retrieves a port with the specified ID
** Parameters: (gfxw_visual_t *) visual: The visual the port is to be retrieved from
**             (int) ID: The port's ID
** Returns   : (gfxw_port_t *) The requested port, or NULL if it didn't exist
** This function is O(1).
*/

gfxw_port_t *gfxw_find_default_port(gfxw_visual_t *visual);
/* Retreives the default port from a visual
** Parameters: (gfxw_visual_t *) visual: The visual the port should be retrieved from
** Returns   : (gfxw_port_t *) The default port, or NULL if no port is present
** The 'default port' is the last port to be instantiated; usually the topmost
** or highest-ranking port.
*/

void gfxw_port_set_auto_restore(gfxw_visual_t *visual, gfxw_port_t *window, rect_t auto_rect);
/* Sets rectangle to be restored upon port removal
** Parameters: (state_t *) s: The state to operate on
**             (gfxw_port_t *) window: The affected window
**             (rect_t) auto_rect: The area to restore
** Returns   : (void)
*/

gfxw_port_t *gfxw_remove_port(gfxw_visual_t *visual, gfxw_port_t *port);
/* Removes a port from a visual
** Parameters: (gfxw_visual_t *) visual: The visual the port should be removed from
**             (gfxw_port_t *) port: The port to remove
** Returns   : (gfxw_port_t *) port's parent port, or NULL if it had none
*/

void gfxw_remove_widget_from_container(gfxw_container_t *container, gfxw_widget_t *widget);
/* Removes the widget from the specified port
** Parameters: (gfxw_container_t *) container: The container it should be removed from
**             (gfxw_widget_t *) widget: The widget to remove
** Returns   : (void)
*/

gfxw_snapshot_t *gfxw_make_snapshot(gfxw_visual_t *visual, rect_t area);
/* Makes a "snapshot" of a visual
** Parameters: (gfxw_visual_t *) visual: The visual a snapshot is to be taken of
**             (rect_t) area: The area a snapshot should be taken of
** Returns   : (gfxw_snapshot_t *) The resulting, newly allocated snapshot
** It's not really a full qualified snaphot, though. See gfxw_restore_snapshot
** for a full discussion.
** This operation also increases the global serial number counter by one.
*/

int gfxw_widget_matches_snapshot(gfxw_snapshot_t *snapshot, gfxw_widget_t *widget);
/* Predicate to test whether a widget would be destroyed by applying a snapshot
** Parameters: (gfxw_snapshot_t *) snapshot: The snapshot to test against
**             (gfxw_widget_t *) widget: The widget to test
** Retunrrs  : (int) An appropriate boolean value
*/

gfxw_snapshot_t *gfxw_restore_snapshot(gfxw_visual_t *visual, gfxw_snapshot_t *snapshot);
/* Restores a snapshot to a visual
** Parameters: (gfxw_visual_t *) visual: The visual to operate on
**             (gfxw_snapshot_t *) snapshot: The snapshot to restore
** Returns   : (gfxw_snapshot_t *) snapshot (still needs to be freed)
** The snapshot is not really restored; only more recent widgets touching
** the snapshotted area are destroyed.
*/

void gfxw_annihilate(gfxw_widget_t *widget);
/* As widget->widfree(widget), but destroys all overlapping widgets
** Parameters: (gfxw_widget_t *) widget: The widget to use
** Returns   : (void)
** This operation calls widget->widfree(widget), but it also destroys
** all widgets with a higher or equal priority drawn after this widget.
*/

gfxw_dyn_view_t *gfxw_picviewize_dynview(gfxw_dyn_view_t *dynview);
/* Turns a dynview into a picview
** Parameters: (gfxw_dyn_view_t *) dynview: The victim
** Returns   : (gfxw_dyn_view_t *) The victim, after his transformation
** The only changes are in function and type variables, actually.
*/

void gfxw_port_auto_restore_background(gfxw_visual_t *visual, gfxw_port_t *window, rect_t auto_rect);
/* Tags a window widget as automatically restoring the visual background upon removal
** Parameters: (gfx_visual_t *) visual: The base visual
**             (gfxw_port_t *) window: The window to tag
**             (rect_t) auto_rect: The background to remember
** Also records the specified background rectangle, for later recovery
*/

} // End of namespace Sci

#endif // SCI_GFX_GFX_WIDGETS_H
