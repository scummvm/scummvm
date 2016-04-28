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

#ifndef TITANIC_PET_GLYPHS_H
#define TITANIC_PET_GLYPHS_H

#include "common/keyboard.h"
#include "titanic/core/list.h"
#include "titanic/messages/mouse_messages.h"
#include "titanic/pet_control/pet_gfx_element.h"
#include "titanic/support/rect.h"

namespace Titanic {

#define TOTAL_GLYPHS 7

class CPetGlyphs;
class CPetSection;
class CPetText;

enum GlyphActionMode { ACTION_REMOVE = 0, ACTION_REMOVED = 1, ACTION_CHANGE = 2 };

enum GlyphFlag { GFLAG_1 = 1, GFLAG_2 = 2, GFLAG_4 = 4, GFLAG_8 = 8, GFLAG_16 = 16 };

class CGlyphAction {
protected:
	GlyphActionMode _mode;
public:
	CGlyphAction() : _mode(ACTION_REMOVED) {}
	CGlyphAction(GlyphActionMode mode) : _mode(mode) {}

	GlyphActionMode getMode() const { return _mode; }
};

class CPetGlyph : public ListItem {
protected:
	/**
	 * Get the overall pet section owner
	 */
	CPetSection *getPetSection() const;
public:
	CPetGfxElement _element;
	CPetGlyphs *_owner;
public:
	CPetGlyph() : ListItem(), _owner(nullptr) {}

	/**
	 * Translate the glyph's position
	 */
	void translate(const Point &pt) { _element.translate(pt.x, pt.y); }

	/**
	 * Translate the glyph's position back
	 */
	void translateBack(const Point &pt) { _element.translate(-pt.x, -pt.y); }

	/**
	 * Get the parent RealLife area
	 */
	CPetGlyphs *getOwner() { return _owner; }

	/**
	 * Get the PET control
	 */
	CPetControl *getPetControl() const;

	/**
	 * Sets new name and default bounds for glyph
	 */
	void setName(const CString &name, CPetControl *petControl);

	/**
	 * Setup the glyph
	 */
	virtual bool setup(CPetControl *petControl, CPetGlyphs *owner);

	/**
	 * Reset the glyph
	 */
	virtual bool reset() { return false; }

	/**
	 * Called when the PET area is entered
	 */
	virtual bool enter() { return false; }
	
	/**
	 * Called when the PET area is left
	 */
	virtual bool leave() { return false; }

	/**
	 * Draw the glyph at a specified position
	 */
	virtual void drawAt(CScreenManager *screenManager, const Point &pt, bool isHighlighted);

	/**
	 * Handles any secondary drawing of the glyph
	 */
	virtual void draw2(CScreenManager *screenManager) {}

	virtual void proc14();

	/**
	 * Get the bounds for the glyph
	 */
	virtual Rect getBounds() { return Rect(); }

	/**
	 * Called for mouse button down messages
	 */
	virtual bool MouseButtonDownMsg(const Point &pt) { return false; }

	/**
	 * Called when mouse drag starts
	 */
	virtual bool MouseDragStartMsg(CMouseDragStartMsg *msg) { return false; }

	/**
	 * Called during mouse drags
	 */
	virtual bool MouseDragMoveMsg(CMouseDragMoveMsg *msg) { return false; }

	/**
	 * Called when mouse drag ends
	 */
	virtual bool MouseDragEndMsg(CMouseDragEndMsg *msg) { return false; }

	/**
	 * Handles mouse button up messages
	 */
	virtual bool MouseButtonUpMsg(const Point &pt) { return false; }
	
	virtual int proc21() { return 0; }
	virtual int proc22() { return 0; }

	/**
	 * Handles keypresses when the glyph is focused
	 */
	virtual bool KeyCharMsg(int key) { return false; }
	
	virtual bool VirtualKeyCharMsg(int key) { return false; }

	/**
	 * Unhighlight any currently highlighted element
	 */
	virtual void unhighlightCurrent() {}

	/**
	 * Highlight any currently highlighted element
	 */
	virtual void highlightCurrent(const Point &pt) {}

	virtual void proc27(const Point &pt, bool flag) {}

	/**
	 *
	 */
	virtual void proc28(const Point &pt) {}
	
	virtual int proc29(const Point &pt) { return 0; }

	/**
	 * Returns true if the glyph's bounds, shifted to a given position,
	 * will contain the specified point
	 */
	virtual bool contains(const Point &delta, const Point &pt);

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	virtual void getTooltip(CPetText *text) {}

	virtual void proc32() {}

	virtual int proc33() { return 1; }
	virtual int proc34() { return 1; }

	/**
	 * Called on a highlighted item when PET area is entered
	 */
	virtual bool enterHighlighted() { return false; }
	
	/**
	 * Called on a highlighted item when PET area is left
	 */
	virtual void leaveHighlighted() {}

	virtual int proc37() { return 0; }

	/**
	 * Does a processing action on the glyph
	 */
	virtual bool doAction(CGlyphAction *action) { return true; }
};
		
class CPetGlyphs : public List<CPetGlyph> {
private:
	/**
	 * Get a position for the glyph
	 */
	Point getPosition(int index);

	/**
	 * Get a rect for the glyph
	 */
	Rect getRect(int index);

	/**
	 * Returns the on-screen index for the highlight to be shown at
	 */
	int getHighlightedIndex(int index);

	/**
	 * Returns the index of a glyph given the visible on-screen glyph number
	 */
	int getItemIndex(int index);

	/**
	 * Return a specified glyph
	 */
	CPetGlyph *getGlyph(int index);

	/**
	 * Scrolls the glyphs to the left
	 */
	void scrollLeft();

	/**
	 * Scrolls the glyphs to the right
	 */
	void scrollRight();

	/**
	 * Set the first visible glyph index
	 */
	void setFirstVisible(int index);

	/**
	 * Make the PET dirty
	 */
	void makePetDirty();
protected:
	int _firstVisibleIndex;
	int _totalGlyphs;
	int _numVisibleGlyphs;
	int _highlightIndex;
	int _field1C;
	int _flags;
	CPetGlyph *_dragGlyph;
	CPetSection *_owner;
	CPetGfxElement _selection;
	CPetGfxElement _scrollLeft;
	CPetGfxElement _scrollRight;
protected:
	/**
	 * Change the currently selected glyph
	 */
	void changeHighlight(int index);
public:
	CPetGlyphs();

	/**
	 * Set the number of visible glyphs
	 */
	void setNumVisible(int total);

	/**
	 * Clears the glyph list
	 */
	void clear();


	/**
	 * The visual dimensions for the control and it's components
	 */
	virtual void setup(int numVisible, CPetSection *owner);

	/**
	 * Set up the control
	 */
	virtual void reset();

	/**
	 * Called when PET area is entered
	 */
	virtual bool enter();

	/**
	 * Called when PET area is left
	 */
	virtual bool leave();

	void setFlags(int flags) { _flags = flags; }

	/**
	 * Draw the control
	 */
	void draw(CScreenManager *screenManager);

	/**
	 * Highlight a specific glyph
	 */
	void highlight(int index);

	/**
	 * Get the owning section for the glyphs
	 */
	CPetSection *getOwner() const { return _owner; }

	/**
	 * Get the PET control
	 */
	CPetControl *getPetControl() const;

	/**
	 * Mouse button down message
	 */
	bool MouseButtonDownMsg(const Point &pt);

	/**
	 * Mouse button up message
	 */
	bool MouseButtonUpMsg(const Point &pt);

	/**
	 * Mouse drag start messagge
	 */
	bool MouseDragStartMsg(CMouseDragStartMsg *msg);

	/**
	 * Mouse drag move message
	 */
	bool MouseDragMoveMsg(CMouseDragMoveMsg *msg);

	/**
	 * Mouse drag end message
	 */
	bool MouseDragEndMsg(CMouseDragEndMsg *msg);

	/**
	 * Key character message
	 */
	bool KeyCharMsg(int key);

	/**
	 * Virtual key message
	 */
	bool VirtualKeyCharMsg(int key);

	/**
	 * When the PET section is entered, passes onto the highlighted
	 * glyph, if any
	 */
	bool enterHighlighted();

	/**
	 * When the PET section is left, passes onto the highlighted
	 * glyph, if any
	 */
	void leaveHighlighted();

	/**
	 * Called when a dragging operation starts
	 */
	void startDragging(CPetGlyph *glyph, CMouseDragStartMsg *msg);

	/**
	 * Called when a dragging operation ends
	 */
	void endDragging();

	/**
	 * Reset the highlight
	 */
	void resetHighlight() { changeHighlight(-1); }

	bool highlighted14();
};

} // End of namespace Titanic

#endif /* TITANIC_PET_GLYPHS_H */
