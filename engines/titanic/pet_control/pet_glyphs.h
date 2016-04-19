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

#include "titanic/core/list.h"
#include "titanic/pet_control/pet_gfx_element.h"
#include "titanic/support/rect.h"

namespace Titanic {

#define TOTAL_GLYPHS 7

class CPetGlyphs;
class CPetSection;

enum GlyphActionMode { ACTION_REMOVE = 0, ACTION_REMOVED = 1, ACTION_CHANGE = 2 };

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
	virtual void setup(CPetControl *petControl, CPetGlyphs *owner);

	/**
	 * Reset the glyph
	 */
	virtual bool reset() { return false; }

	virtual void proc10() {}
	virtual void proc11() {}

	/**
	 * Draw the glyph at a translated position without permanently
	 * changing the position
	 */
	virtual void drawAt(CScreenManager *screenManager, int x, int y);

	/**
	 * Handles any secondary drawing of a glyph as highlighted
	 */
	virtual void drawHighlight() {}

	virtual void proc14();

	/**
	 * Get the bounds for the glyph
	 */
	virtual Rect getBounds() { return Rect(); }

	virtual int proc16() { return 0; }
	virtual int proc17() { return 0; }
	virtual int proc18() { return 0; }
	virtual int proc19() { return 0; }
	virtual int proc20() { return 0; }
	virtual int proc21() { return 0; }
	virtual int proc22() { return 0; }
	virtual int proc23() { return 0; }
	virtual int proc24() { return 0; }
	virtual void proc25() {}
	virtual void proc26() {}
	virtual void proc27() {}
	virtual void proc28() {}
	virtual int proc29() { return 0; }

	/**
	 * Returns true if the glyph's bounds, shifted by a given delta,
	 * will contain the specified point
	 */
	virtual bool translateContains(const Point &delta, const Point &pt);

	virtual void proc31() {}
	virtual void proc32() {}

	virtual int proc33() { return 1; }
	virtual int proc34() { return 1; }
	virtual int proc35() { return 0; }
	virtual void proc36() {}
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
protected:
	int _firstVisibleIndex;
	int _totalGlyphs;
	int _numVisibleGlyphs;
	int _highlightIndex;
	int _field1C;
	int _field20;
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

	virtual void proc10();
	virtual void proc11();

	void set20(int val) { _field20 = val; }

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
};

} // End of namespace Titanic

#endif /* TITANIC_PET_GLYPHS_H */
