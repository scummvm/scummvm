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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA8_GUMPS_GUMP_H
#define ULTIMA8_GUMPS_GUMP_H

#include "ultima/ultima8/kernel/object.h"
#include "ultima/ultima8/misc/rect.h"
#include "ultima/ultima8/graphics/frame_id.h"
#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class RenderSurface;
class Shape;
class Item;
class GumpNotifyProcess;

class Gump;
typedef bool (*FindGumpPredicate)(const Gump *g);
template<class T> inline bool IsOfType(const Gump *g) { return dynamic_cast<const T*>(g) != nullptr; }

/**
 * A Gump is a single GUI element within the game, like the backpack window, menu,
 * conversation text, etc.  Like most windowing systems, gumps nest.
 */
class Gump : public Object {
protected:
	uint16 _owner;        // Owner item
	Gump *_parent;        // Parent gump
	int32 _x, _y;         // Gump's position in parent.
	// Always the upper left corner!

	Rect _dims;           // The dimensions/coord space of the gump
	uint32 _flags;        // Gump flags
	int32 _layer;         // gump ordering layer

	int32 _index;         // 'Index'

	const Shape *_shape;  // The gumps shape (always painted at 0,0)
	uint32 _frameNum;

	//! The Gump list for this gump. This will contain all child gumps,
	//! as well as all gump widgets.
	Std::list<Gump *> _children;      // List of all gumps
	Gump               *_focusChild;  // The child that has focus

	uint16              _notifier;      // Process to notify when we're closing
	uint32              _processResult; // Result for the notifier process

public:
	ENABLE_RUNTIME_CLASSTYPE()
	Gump();
	Gump(int x, int y, int width, int height, uint16 owner = 0,
	     uint32 flags = 0, int32 layer = LAYER_NORMAL);
	~Gump() override;

public:

	virtual void                CreateNotifier();
	void                        SetNotifyProcess(GumpNotifyProcess *proc);
	GumpNotifyProcess          *GetNotifyProcess();
	inline uint32               GetResult() {
		return _processResult;
	}
	void                        SetResult(uint32 res) {
		_processResult = res;
	}

	//! Set the Gump's shape/frame
	inline void SetShape(const Shape *shape, uint32 frameNum) {
		_shape = shape;
		_frameNum = frameNum;
	}

	void                        SetShape(FrameID frame, bool adjustsize = false);

	//! Update the width/height to match the gump's current shape frame
	void 						UpdateDimsFromShape();

	//! Set the Gump's frame
	inline void                 Set_frameNum(uint32 frameNum) {
		_frameNum = frameNum;
	}

	//! Init the gump and add it to parent; call after construction
	//! When newparent is 0, this will call Ultima8Engine::addGump().
	//! \param newparent The Gump's new parent or 0.
	//! \param takefocus If true, set parent's _focusChild to this
	virtual void                InitGump(Gump *newparent, bool take_focus = true);

	//! Find a gump of that matches a predicate function (this or child)
	//! \param predicate Function to check if a gump is a match
	//! \param recursive Recursively search through children?
	//! \return the desired Gump, or NULL if not found
	virtual Gump               *FindGump(FindGumpPredicate predicate, bool recursive = true);

	//! Find a gump of the specified type (this or child)
	//! \param T Type of gump to look for
	//! \param recursive Recursively search through children?
	//! \return the desired Gump, or NULL if not found
	template<class T> Gump     *FindGump(bool recursive = true) {
		return FindGump(&IsOfType<T>, recursive);
	}

	//! A predicate to find a ui element by its index
	template<int T> static bool FindByIndex(const Gump *g) { return g->GetIndex() == T; }

	//! Find gump (this, child or NULL) at parent coordinates (mx,my)
	//! \return the Gump at these coordinates, or NULL if none
	virtual Gump       *FindGump(int mx, int my);

	//! Get the mouse cursor for position mx, my relative to parents position.
	//! If this gump doesn't want to set the cursor, the gump list will
	//! attempt to get the cursor shape from the next lower gump.
	//! \return true if this gump wants to set the cursor, false otherwise
	virtual bool GetMouseCursor(int32 mx, int32 my, Shape &shape, int32 &frame);

	// Notify gumps the render surface changed.
	virtual void        RenderSurfaceChanged();

	//! Run the gump
	virtual void        run();

	//! Close item-dependent gumps (recursively).
	//! Called when there is a map change (so the gumps can self terminate
	//! among other things), or when backspace is pressed by the user.
	virtual void        CloseItemDependents();

	//! Paint the Gump (RenderSurface is relative to parent).
	//! Calls PaintThis and PaintChildren
	// \param surf The RenderSurface to paint to
	// \param lerp_factor The lerp_factor to paint at (0-256)
	// \param scaled Set if the gump is being drawn scaled.
	virtual void        Paint(RenderSurface *surf, int32 lerp_factor, bool scaled);

	//! Paint the unscaled compontents of the Gump with compositing (RenderSurface is relative to parent).
	//! Calls PaintComposited on self and PaintCompositing on children
	// \param surf The RenderSurface to paint to
	// \param lerp_factor The lerp_factor to paint at (0-256)
	// \param scalex Fixed point scaling factor for x coord
	// \param scaley Fixed point scaling factor for y coord
	virtual void        PaintCompositing(RenderSurface *surf, int32 lerp_factor, int32 scalex, int32 scaley);

protected:

	//! Overloadable method to Paint just this Gump (RenderSurface is relative to this)
	// \param surf The RenderSurface to paint to
	// \param lerp_factor The lerp_factor to paint at (0-256)
	// \param scaled Set if the gump is being drawn scaled.
	virtual void        PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled);

	//! Paint the Gumps Children (RenderSurface is relative to this)
	// \param surf The RenderSurface to paint to
	// \param lerp_factor The lerp_factor to paint at (0-256)
	// \param scaled Set if the gump is being drawn scaled.
	virtual void        PaintChildren(RenderSurface *surf, int32 lerp_factor, bool scaled);

	//! Overloadable method to Paint just this gumps unscaled components that require compositing (RenderSurface is relative to parent).
	// \param surf The RenderSurface to paint to
	// \param lerp_factor The lerp_factor to paint at (0-256)
	// \param scalex Fixed point scaling factor for x coord
	// \param scaley Fixed point scaling factor for y coord
	virtual void        PaintComposited(RenderSurface *surf, int32 lerp_factor, int32 scalex, int32 scaley);

	static inline int32 ScaleCoord(int32 c, int32 factor) {
		return ((c * factor) + (1 << 15)) >> 16;
	}
	static inline int32 UnscaleCoord(int32 c, int32 factor) {
		return (c << 16) / factor;
	}

public:

	//! Close the gump
	//! \param no_del If true, do not delete after closing
	virtual void        Close(bool no_del = false);

	//! Check to see if a Gump is Closing
	bool                IsClosing() const {
		return (_flags & FLAG_CLOSING) != 0;
	}

	//! Move this gump
	virtual void        Move(int32 x, int32 y) {
		_x = x;
		_y = y;
	}

	//! Move this gump relative to its current position
	virtual void        MoveRelative(int x, int y) {
		_x += x;
		_y += y;
	}

	void getLocation(int32 &x, int32 &y) const {
		x = _x;
		y = _y;
	}

	enum Position {
		CENTER = 1,
		TOP_LEFT = 2,
		TOP_RIGHT = 3,
		BOTTOM_LEFT = 4,
		BOTTOM_RIGHT = 5,
		TOP_CENTER = 6,
		BOTTOM_CENTER = 7
	};

	//! Moves this gump to a relative location on the parent gump
	// \param pos the postition on the parent gump
	// \param xoffset an offset from the position on the x-axis
	// \param yoffset an offset from the position on the y-axis
	virtual void        setRelativePosition(Position pos, int xoffset = 0, int yoffset = 0);

	//
	// Points and Coords
	//

	//! Get the _dims
	virtual void GetDims(Rect &d) const {
		d = _dims;
	}

	//! Set the _dims
	virtual void SetDims(const Rect &d) {
		_dims = d;
	}

	//! Detect if a point is on the gump
	virtual bool PointOnGump(int mx, int my);

	enum PointRoundDir {
		ROUND_TOPLEFT = 0,
		ROUND_BOTTOMRIGHT = 1
	};
	enum RectRoundDir {
		ROUND_INSIDE,
		ROUND_OUTSIDE
	};

	//! Convert a screen space point to a gump point
	virtual void ScreenSpaceToGump(int32 &sx, int32 &sy,
		PointRoundDir r = ROUND_TOPLEFT);

	//! Convert a gump point to a screen space point
	virtual void GumpToScreenSpace(int32 &gx, int32 &gy,
		PointRoundDir r = ROUND_TOPLEFT);

	//! Convert a parent relative point to a gump point
	virtual void ParentToGump(int32 &px, int32 &py,
		PointRoundDir r = ROUND_TOPLEFT);

	//! Convert a gump point to parent relative point
	virtual void GumpToParent(int32 &gx, int32 &gy,
		PointRoundDir r = ROUND_TOPLEFT);

	//! Transform a rectangle to screenspace from gumpspace
	virtual void GumpRectToScreenSpace(Rect &gr, RectRoundDir r = ROUND_OUTSIDE);

	//! Transform a rectangle to gumpspace from screenspace
	virtual void ScreenSpaceToGumpRect(Rect &sr, RectRoundDir r = ROUND_OUTSIDE);

	//! Trace a click, and return ObjId
	virtual uint16 TraceObjId(int32 mx, int32 my);

	//! Get the location of an item in the gump (coords relative to this).
	//! \return false on failure
	virtual bool GetLocationOfItem(uint16 itemid, int32 &gx, int32 &gy,
		int32 lerp_factor = 256);


	//
	// Some event handlers. In theory they 'should' be able to be mapped to
	// Usecode classes.
	//
	// mx and my are relative to parents position
	//
	// onMouseDown returns the Gump that handled the Input, if it was handled.
	// The MouseUp,MouseDouble events will be sent to the same gump.
	//
	// onMouseMotion works like onMouseDown,
	// but independently of the other methods.
	//
	// Unhandled input will be passed down to the next lower gump.
	//
	// A mouse click on a gump will make it focus, IF it wants it.
	//

	// Return Gump that handled event
	virtual Gump       *onMouseDown(int button, int32 mx, int32 my);
	virtual void        onMouseUp(int button, int32 mx, int32 my) { }
	virtual void        onMouseClick(int button, int32 mx, int32 my) { }
	virtual void        onMouseDouble(int button, int32 mx, int32 my) { }
	virtual Gump       *onMouseMotion(int32 mx, int32 my);

	// onMouseOver is only call when the mouse first passes over the gump
	// onMouseLeft is call as the mouse leaves the gump.
	virtual void        onMouseOver() { };
	virtual void        onMouseLeft() { };

	// Keyboard input gets sent to the FocusGump. Or if there isn't one, it
	// will instead get sent to the default key handler. TextInput requires
	// that text mode be enabled. Return true if handled, false if not.
	// Default, returns false, unless handled by focus child
	virtual bool        OnKeyDown(int key, int mod);
	virtual bool        OnKeyUp(int key);
	virtual bool        OnTextInput(int unicode);

	// This is for detecting focus changes for keyboard input. Gets called true
	// when the this gump is being set as the focus focus gump. It is called
	// false when focus is being taken away.
	virtual void        OnFocus(bool /*gain*/) { }

	// Makes this gump the focus
	virtual void        MakeFocus();

	// Is this gump the focus?
	inline bool         IsFocus() {
		return _parent ? _parent->_focusChild == this : false;
	}

	// Get the child in focus
	inline Gump        *GetFocusChild() {
		return _focusChild;
	}

	// Find a new Child to be the focus
	void                FindNewFocusChild();


	//
	// Child gump related
	//

	//! Add a gump to the child list.
	virtual void AddChild(Gump *, bool take_focus = true);

	//! Remove a gump from the child list
	virtual void RemoveChild(Gump *);

	//! Move child to front (within its layer)
	virtual void MoveChildToFront(Gump *);

	//! Get the parent
	inline Gump *GetParent() {
		return _parent;
	}

	//! Get the root gump (or self)
	Gump *GetRootGump();

	//! This function is used by our children to notifty us of 'something'
	//! Think of it as a generic call back function
	virtual void ChildNotify(Gump *child, uint32 message) { }
	void SetIndex(int32 i) {
		_index = i;
	}
	int32 GetIndex() const {
		return _index;
	}

	// Dragging
	//! Called when a child gump starts to be dragged.
	//! \return false if the child isn't allowed to be dragged.
	virtual bool StartDraggingChild(Gump *gump, int32 mx, int32 my);
	virtual void DraggingChild(Gump *gump, int mx, int my);
	virtual void StopDraggingChild(Gump *gump);

	//! This will be called when an item in this gump starts to be dragged.
	//! \return false if the item isn't allowed to be dragged.
	virtual bool StartDraggingItem(Item *item, int mx, int my) {
		return false;
	}

	//! Called when an item is being dragged over the gump.
	//! Note: this may be called on a different gump than StartDraggingItem.
	//! \return false if the item can't be dragged to this location.
	virtual bool DraggingItem(Item *item, int mx, int my) {
		return false;
	}

	//! Called when an item that was being dragged over the gump left the gump
	virtual void DraggingItemLeftGump(Item *item) { }

	//! Called when a drag operation finished.
	//! This is called on the same gump that received StartDraggingItem
	//! \param moved If true, the item was actually dragged somewhere else.
	//!              If false, the drag was cancelled.
	virtual void StopDraggingItem(Item *item, bool moved) { }

	//! Called when an item has been dropped on a gump.
	//! This is called after StopDraggingItem has been called, but possibly
	//! on a different gump.
	//! It's guaranteed that a gump will only receive a DropItem at a location
	//! if a DraggingItem there returned true.
	virtual void DropItem(Item *item, int mx, int my) { }

public:

	//
	// Gump Flags
	//
	enum GumpFlags {
		FLAG_DRAGGABLE      = 0x01,     // When set, the gump can be dragged
		FLAG_HIDDEN         = 0x02,     // When set, the gump will not be drawn
		FLAG_CLOSING        = 0x04,     // When set, the gump is closing
		FLAG_CLOSE_AND_DEL  = 0x08,     // When set, the gump is closing and will be deleted
		FLAG_ITEM_DEPENDENT = 0x10,     // When set, the gump will be deleted on MapChange
		FLAG_DONT_SAVE      = 0x20,     // When set, don't save this gump.
		// Be very careful with this one!
		FLAG_CORE_GUMP      = 0x40,     // core gump (only children are saved)
		FLAG_KEEP_VISIBLE   = 0x80      // Keep this gump on-screen.
		                      // (only for ItemRelativeGumps)
	};

	inline bool IsHidden() const {
		return (_flags & FLAG_HIDDEN) || (_parent && _parent->IsHidden());
	}
	bool IsDraggable() const {
		return _flags & FLAG_DRAGGABLE;
	}
	virtual void HideGump() {
		_flags |= FLAG_HIDDEN;
	}
	virtual void UnhideGump() {
		_flags &= ~FLAG_HIDDEN;
	}
	void SetVisibility(bool visible) {
		if (visible)
			UnhideGump();
		else
			HideGump();
	}

	bool mustSave(bool toplevel) const;

	//
	// Gump Layers
	//
	enum GumpLayers {
		LAYER_DESKTOP       = -16,      // Layer for Desktop 'bottom most'
		LAYER_GAMEMAP       = -8,       // Layer for the World Gump
		LAYER_NORMAL        = 0,        // Layer for Normal gumps
		LAYER_ABOVE_NORMAL  = 8,        // Layer for Always on top Gumps
		LAYER_MODAL         = 12,       // Layer for Modal Gumps
		LAYER_CONSOLE       = 16        // Layer for the console
	};

	enum Message {
		GUMP_CLOSING = 0x100
	};

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
