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
class CTextControl;

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
	virtual void enter() {}

	/**
	 * Called when the PET area is left
	 */
	virtual void leave() {}

	/**
	 * Draw the glyph at a specified position
	 */
	virtual void drawAt(CScreenManager *screenManager, const Point &pt, bool isHighlighted);

	/**
	 * Handles any secondary drawing of the glyph
	 */
	virtual void draw2(CScreenManager *screenManager) {}

	/**
	 * Updates the tooltip being shown for the glyph
	 */
	virtual void updateTooltip();

	/**
	 * Get the bounds for the glyph
	 */
	virtual Rect getBounds() const { return Rect(); }

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

	/**
	 * Handles mouse double-click messages
	 */
	virtual bool MouseDoubleClickMsg(const CMouseDoubleClickMsg *msg) { return false; }

	/**
	 * Handles keypresses
	 */
	virtual bool KeyCharMsg(int key) { return false; }

	/**
	 * Handles keypresses
	 */
	virtual bool VirtualKeyCharMsg(CVirtualKeyCharMsg *msg) { return false; }

	/**
	 * Unhighlight any currently highlighted element
	 */
	virtual void unhighlightCurrent() {}

	/**
	 * Highlight any currently highlighted element
	 */
	virtual void highlightCurrent(const Point &pt) {}

	/**
	 * Glyph has been shifted to be first visible one
	 */
	virtual void glyphFocused(const Point &topLeft, bool flag) {}

	/**
	 * Selects a glyph
	 */
	virtual void selectGlyph(const Point &topLeft, const Point &pt) {}

	/**
	 * Called when a glyph drag starts
	 */
	virtual bool dragGlyph(const Point &topLeft, CMouseDragStartMsg *msg) { return false; }

	/**
	 * Returns true if the glyph's bounds, shifted to a given position,
	 * will contain the specified point
	 */
	virtual bool contains(const Point &delta, const Point &pt);

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	virtual void getTooltip(CTextControl *text) {}

	/**
	 * Saves the data for the glyph
	 */
	virtual void saveGlyph(SimpleFile *file, int indent) {}

	virtual bool proc33(CPetGlyph *glyph) { return true; }

	/**
	 * Return whether the glyph is currently valid
	 */
	virtual bool isValid() const { return true; }

	/**
	 * Called on a highlighted item when PET area is entered
	 */
	virtual bool enterHighlighted() { return false; }

	/**
	 * Called on a highlighted item when PET area is left
	 */
	virtual void leaveHighlighted() {}

	/**
	 * Returns the object associated with the glyph
	 */
	virtual CGameObject *getObjectAt() { return nullptr; }

	/**
	 * Does a processing action on the glyph
	 */
	virtual bool doAction(CGlyphAction *action) { return true; }

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
	 * Returns true if the specified glyph is the currently highlighted one
	 */
	bool isHighlighted() const;
};

class CPetGlyphs : public List<CPetGlyph> {
private:
	/**
	 * Get a position for the glyph
	 */
	Point getPosition(int index) const;

	/**
	 * Get a rect for the glyph
	 */
	Rect getRect(int index) const;

	/**
	 * Returns the on-screen index for the highlight to be shown at
	 */
	int getHighlightedIndex(int index) const;

	/**
	 * Returns the index of a glyph given the visible on-screen glyph number
	 */
	int getItemIndex(int index) const;

	/**
	 * Set the item index
	 */
	void setSelectedIndex(int index);

	/**
	 * Return a specified glyph
	 */
	CPetGlyph *getGlyph(int index) const;

	/**
	 * Set the first visible glyph index
	 */
	void setFirstVisible(int index);

	/**
	 * Make the PET dirty
	 */
	void makePetDirty();

	/**
	 * Returns true if all the glyphs are in a valid state
	 */
	bool areItemsValid() const;
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
	virtual void enter();

	/**
	 * Called when PET area is left
	 */
	virtual void leave();

	void setFlags(int flags) { _flags = flags; }

	/**
	 * Draw the control
	 */
	void draw(CScreenManager *screenManager);

	/**
	 * Highlight a specific glyph by indexe
	 */
	void highlight(int index);

	/**
	 * Highlight a specific glyph
	 */
	void highlight(const CPetGlyph *glyph);

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
	 * Mouse double click message
	 */
	bool MouseDoubleClickMsg(const Point &pt) { return true; }

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
	bool VirtualKeyCharMsg(CVirtualKeyCharMsg *msg);

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

	/**
	 * Returns the index of the specified glyph in the lsit
	 */
	int indexOf(const CPetGlyph *glyph) const;

	/**
	 * Resets the scrolling of the glyphs list back to the start
	 */
	void scrollToStart() { _firstVisibleIndex = 0; }

	/**
	 * Scrolls the glyphs to the left
	 */
	void scrollLeft();

	/**
	 * Scrolls the glyphs to the right
	 */
	void scrollRight();

	/**
	 * Increment the currently selected index
	 */
	void incSelection();

	/**
	 * Decrement the currently selected index
	 */
	void decSelection();

	/**
	 * Returns the object associated the glyph under the specified position
	 */
	CGameObject *getObjectAt(const Point &pt) const;

	/**
	 * Returns true if the specified glyph is the currently highlighted one
	 */
	bool isGlyphHighlighted(const CPetGlyph *glyph) const;

	/**
	 * Returns the highlighted index, if any
	 */
	int getHighlightIndex() const { return _highlightIndex; }

	/**
	 * Get the top-left position of the currently highlighted glyph
	 */
	Point getHighlightedGlyphPos() const;

	/**
	 * Removes any glyphs from the list that no longer have any images
	 * associated with them
	 */
	void removeInvalid();
};

} // End of namespace Titanic

#endif /* TITANIC_PET_GLYPHS_H */
