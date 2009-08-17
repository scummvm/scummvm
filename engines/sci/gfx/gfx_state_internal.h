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

#ifndef SCI_GFX_GFX_STATE_INTERNAL_H
#define SCI_GFX_GFX_STATE_INTERNAL_H

#include "sci/engine/vm.h"
#include "sci/gfx/gfx_tools.h"
#include "sci/gfx/gfx_options.h"
#include "sci/gfx/operations.h"
#include "sci/gfx/gfx_resmgr.h"
#include "sci/gfx/gfx_system.h"

namespace Sci {

enum gfxw_flag_t {
	GFXW_FLAG_VISIBLE				= (1<<0),
	GFXW_FLAG_OPAQUE				= (1<<1),
	GFXW_FLAG_CONTAINER				= (1<<2),
	GFXW_FLAG_DIRTY					= (1<<3),
	GFXW_FLAG_TAGGED				= (1<<4),
	GFXW_FLAG_MULTI_ID				= (1<<5), /**< Means that the ID used herein may be used more than once, i.e. is not unique */
	GFXW_FLAG_IMMUNE_TO_SNAPSHOTS	= (1<<6), /**< Snapshot restoring doesn't kill this widget, and +5 bonus to saving throws vs. Death Magic */
	GFXW_FLAG_NO_IMPLICIT_SWITCH 	= (1<<7) /**< Ports: Don't implicitly switch to this port when disposing windows */
};

struct gfxw_snapshot_t {
	int serial; /**< The first serial number to kill */
	rect_t area;
};

enum gfxw_widget_type_t {
	GFXW_, /**< Base widget */

	GFXW_BOX,
	GFXW_RECT,
	GFXW_LINE, /* For lines, the bounding rectangle's xl, yl determine the line's expansion:
		   ** (x2, y2) = (x+xl, y+yl) */
	GFXW_VIEW,
	GFXW_STATIC_VIEW,
	GFXW_DYN_VIEW,
	GFXW_PIC_VIEW,
	GFXW_TEXT,

	GFXW_CONTAINER,

	GFXW_LIST,
	GFXW_SORTED_LIST,
	GFXW_VISUAL,
	GFXW_PORT
};


#define GFXW_MAGIC_VALID 0xC001
#define GFXW_MAGIC_INVALID 0xbad

#define GFXW_NO_ID -1

struct GfxWidget;
struct GfxContainer;
struct GfxVisual;
struct GfxPort;

typedef int gfxw_bin_op(GfxWidget *, GfxWidget *);

/** SCI graphics widget */
struct GfxWidget {
public:
	int _magic; /**< Extra check after typecasting */
	int _serial; /**< Serial number */
	int _flags; /**< Widget flags */
	gfxw_widget_type_t _type;
	rect_t _bounds; /**< Boundaries */
	GfxWidget *_next; /**< Next widget in widget list */
	int _ID; /**< Unique ID or GFXW_NO_ID */
	int _subID; /**< A 'sub-ID', or GFXW_NO_ID */
	GfxContainer *_parent; /**< The parent widget, or NULL if not owned */
	GfxVisual *_visual; /**< The owner visual */
	int _widgetPriority; /**< Drawing priority, or -1 */

public:
	GfxWidget(gfxw_widget_type_t type);

	/**
	 * The widget automatically removes itself from its owner, if it has one.
	 * Deleting a container will recursively free all of its contents.
	 */
	virtual ~GfxWidget();

	/**
	 * Draws the widget.
	 *
	 * The widget is drawn iff it is flagged as dirty. Invoking this operation
	 * on a container widget will recursively draw all of its contents.
	 *
	 * @param[in] pos	The position to draw to (added to the widget's
	 * 					internal position)
	 */
	virtual int draw(const Common::Point &pos) = 0;

	/**
	 * Tags the specified widget.
	 *
	 * If invoked on a container widget, this will also tag all of the
	 * container's contents (but not the contents' contents!)
	 * FIXME: Actually, the code in GfxContainer::tag contradicts the last
	 * claim!
	 */
	virtual void tag() {
		_flags |= GFXW_FLAG_TAGGED;
	}

	/**
	 * Prints a string representation of the widget with printf.
	 *
	 * Will recursively print all of the widget's contents if the widget
	 * contains further sub-widgets
	 *
	 * @param[in] indentation	Number of double spaces to indent
	 */
	virtual void print(int indentation) const;

	/**
	 * Compares two comparable widgets by their screen position.
	 *
	 * This comparison only applies to some widgets; compare_to(a,a)=0 is not
	 * guaranteed. It may be used for sorting for all widgets.
	 *
	 * @param other	The other widget
	 * @return		<0, 0, or >0 if other is, respectively, less than, equal
	 * 				to, or greater than self
	 */
	gfxw_bin_op *compare_to;

	/**
	 * Compares two compareable widgets for equality.
	 *
	 * This operation checks whether two widgets describe the same graphical
	 * data. It is used to determine whether a new widget should be discarded
	 * because it describes the same graphical data as an old widget that has
	 * already been drawn. For lists, it also checks whether all contents are
	 * in an identical order.
	 *
	 * @param[in] other	The other widget
	 * @return			false if the widgets are not equal, true if they match
	 */
	gfxw_bin_op *equals;

	/**
	 * Determine whether other should replace this even though they are
	 * equivalent.
	 *
	 * When 'equals' returns true, this means that no new widget will be added.
	 * However, in some cases newer widgets may contain information that should
	 * cause the older widget to be removed nonetheless; this is indicated by
	 * this function.
	 *
	 * @param[in] other	The other widget
	 * @return			false if this should be kept, true if this should be
	 * 					replaced by the 'other'
	 */
	gfxw_bin_op *should_replace;

	/**
	 * Tests whether drawing this after other would reduce all traces of
	 * other.
	 *
	 * /a superarea_of b <=> for each pixel of b there exists an opaque pixel
	 * in a at the same location
	 *
	 * @param[in] other	The widget to compare for containment
	 * @return			true if this is superarea_of other, false otherwise
	 */
	gfxw_bin_op *superarea_of;

	/**
	 * Sets the visual for the widget
	 *
	 * This function is called by container->add() and need not be invoked
	 * explicitly. It also makes sure that dirty rectangles are passed to
	 * parent containers.
	 *
	 * @param[in] visual	GfxVisual to set for the widget
	 */
	virtual int setVisual(GfxVisual *visual);

//protected:
	void printIntern(int indentation) const;

};


#define GFXW_IS_BOX(widget) ((widget)->_type == GFXW_BOX)
/** SCI box widget */
struct GfxBox : public GfxWidget {
	gfx_color_t _color1, _color2;
	gfx_box_shade_t _shadeType;

public:
	GfxBox(GfxState *state, rect_t area, gfx_color_t color1, gfx_color_t color2, gfx_box_shade_t shade_type);

	virtual int draw(const Common::Point &pos);
	virtual void print(int indentation) const;
};


#define GFXW_IS_PRIMITIVE(widget) ((widget)->_type == GFXW_RECT || (widget)->_type == GFXW_LINE)
/** SCI graphics primitive */
struct GfxPrimitive : public GfxWidget {
	gfx_color_t _color;
	gfx_line_mode_t _lineMode;
	gfx_line_style_t _lineStyle;

public:
	GfxPrimitive(rect_t area, gfx_color_t color, gfx_line_mode_t mode,
						gfx_line_style_t style, gfxw_widget_type_t type);
};



#define GFXW_IS_VIEW(widget) ((widget)->_type == GFXW_VIEW || (widget)->_type == GFXW_STATIC_VIEW \
			      || (widget)->_type == GFXW_DYN_VIEW || (widget)->_type == GFXW_PIC_VIEW)
/** SCI graphics view */
struct GfxView  : public GfxWidget {
	Common::Point _pos; /**< Implies the value of 'bounds' in GfxWidget */
	gfx_color_t _color;
	int _view, _loop, _cel;
	int _palette;

public:
	GfxView(GfxState *state, Common::Point pos, int view_nr, int loop, int cel, int palette, int priority, int control,
		gfx_alignment_t halign, gfx_alignment_t valign, int flags);

	virtual int draw(const Common::Point &pos);
	virtual void print(int indentation) const;
};

#define GFXW_IS_DYN_VIEW(widget) ((widget)->_type == GFXW_DYN_VIEW || (widget)->_type == GFXW_PIC_VIEW)
/** SCI dynamic view */
struct GfxDynView : public GfxView {
	/* FIXME: This code is specific to SCI */
	rect_t draw_bounds; /* The correct position to draw to */
	ObjVarRef under_bitsp;
	ObjVarRef signalp;
	int under_bits, signal;
	int _z; /**< The z coordinate: Added to y, but used for sorting */
	int sequence; /**< Sequence number: For sorting */
	int force_precedence; /**< Precedence enforcement variable for sorting- defaults to 0 */

	bool _isDrawn;	// FIXME: This is specific to GFXW_PIC_VIEW

public:
	GfxDynView(GfxState *state, Common::Point pos, int z, int view, int loop, int cel, int palette, int priority, int control,
		gfx_alignment_t halign, gfx_alignment_t valign, int sequence);

	virtual int draw(const Common::Point &pos);
	virtual void print(int indentation) const;
};



#define GFXW_IS_TEXT(widget) ((widget)->_type == GFXW_TEXT)
/** SCI text widget */
struct GfxText : public GfxWidget {
	int _font;
	int lines_nr, lineheight, lastline_width;
	Common::String _text;
	gfx_alignment_t halign, valign;
	gfx_color_t _color1, _color2, _bgcolor;
	int _textFlags;
	int width; /**< Real text width */
	int height; /**< Real text height */
	TextHandle *_textHandle;

public:
	GfxText(GfxState *state, rect_t area, int font, const char *text, gfx_alignment_t halign,
		gfx_alignment_t valign, gfx_color_t color1, gfx_color_t color2, gfx_color_t bgcolor, int text_flags);

	~GfxText();

	virtual int draw(const Common::Point &pos);
	virtual void print(int indentation) const;
};


/* Container widgets */

typedef int gfxw_unary_container_op(GfxContainer *);
typedef int gfxw_container_op(GfxContainer *, GfxWidget *);
typedef int gfxw_rect_op(GfxContainer *, rect_t, int);

/** SCI container widget */
struct GfxContainer : public GfxWidget {
	rect_t zone; /**< The writeable zone (absolute) for contained objects */
	DirtyRectList _dirtyRects; /**< List of dirty rectangles */
	GfxWidget *_contents;
	GfxWidget **_nextpp; /**< Pointer to the 'next' pointer in the last entry in contents */

public:
	// TODO: Replace the following with virtual methods
	gfxw_unary_container_op *free_tagged; /**< Free all tagged contained widgets */
	gfxw_unary_container_op *free_contents; /**< Free all contained widgets */
	gfxw_rect_op *add_dirty_abs; /**< Add an absolute dirty rectangle */
	gfxw_rect_op *add_dirty_rel; /**< Add a relative dirty rectangle */
	gfxw_container_op *add;  /**< Append widget to an appropriate position (for view and control lists) */

public:
	// FIXME: This should be a virtual base class, mark it so somehow?
	GfxContainer(rect_t area, gfxw_widget_type_t type);
	~GfxContainer();

	virtual void tag();
	virtual void print(int indentation) const;
	virtual int setVisual(GfxVisual *);
};


#define GFXW_IS_CONTAINER(widget) ((widget)->_type == GFXW_PORT || (widget)->_type == GFXW_VISUAL || \
				   (widget)->_type == GFXW_SORTED_LIST || (widget)->_type == GFXW_LIST)

#define GFXW_IS_LIST(widget) ((widget)->_type == GFXW_LIST || (widget)->_type == GFXW_SORTED_LIST)
#define GFXW_IS_SORTED_LIST(widget) ((widget)->_type == GFXW_SORTED_LIST)
/** SCI graphics list */
struct GfxList : public GfxContainer {
public:
	GfxList(rect_t area, bool sorted);

	virtual int draw(const Common::Point &pos);
	virtual void print(int indentation) const;
};

#define GFXW_IS_VISUAL(widget) ((widget)->_type == GFXW_VISUAL)
/** SCI graphic visual */
struct GfxVisual : public GfxContainer {
	Common::Array<GfxPort *> _portRefs; /**< References to ports */
	int _font; /**< Default font */
	GfxState *_gfxState;

public:
	GfxVisual(GfxState *state, int font);
	~GfxVisual();

	virtual int draw(const Common::Point &pos);
	virtual void print(int indentation) const;
	virtual int setVisual(GfxVisual *);

	GfxPort *getPort(int portId) {
		return (portId < 0 || portId >= (int)_portRefs.size()) ? NULL : _portRefs[portId];
	}
};

#define GFXW_IS_PORT(widget) ((widget)->_type == GFXW_PORT)
/** SCI graphics port */
struct GfxPort : public GfxContainer {
	GfxList *_decorations; /**< optional window decorations - drawn before the contents */
	GfxWidget *port_bg; /**< Port background widget or NULL */
	gfx_color_t _color, _bgcolor;
	int _font;
	Common::Point draw_pos; /**< Drawing position */
	gfxw_snapshot_t *restore_snap; /**< Snapshot to be restored automagically,
					  experimental feature used in the PQ3 interpreter */
	int port_flags; /**< interpreter-dependant flags */
	const char *title_text;
	byte gray_text; /**< Whether text is 'grayed out' (dithered) */

public:
	/**
	 * Creates a new port widget with the default settings
	 *
	 * A port differentiates itself from a list in that it contains additional
	 * information, and an optional title (stored in a display list).
	 * Ports are assigned implicit IDs identifying their position within the
	 * port stack.
	 *
	 * @param[in] visual		The visual the port is added to
	 * @param[in] area			The screen area covered by the port (absolute
	 * 							position)
	 * @param[in] fgcolor		Foreground drawing color
	 * @param[in] bgcolor		Background color
	 */
	GfxPort(GfxVisual *visual, rect_t area, gfx_color_t fgcolor, gfx_color_t bgcolor);
	~GfxPort();

	virtual int draw(const Common::Point &pos);
	virtual void print(int indentation) const;
	virtual int setVisual(GfxVisual *);
};

} // End of namespace Sci

#endif // SCI_GFX_GFX_STATE_INTERNAL_H
