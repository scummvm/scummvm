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

class CPetGlyphs;

class CPetGlyph : public ListItem {
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
	 * Set the glyph
	 */
	virtual void setOwner(CPetControl *petControl, CPetGlyphs *owner);

	virtual int proc9() { return 0; }

	virtual void proc10() {}
	virtual void proc11() {}

	/**
	 * Draw the glyph at a translated position without permanently
	 * changing the position
	 */
	virtual void translateDraw(CScreenManager *screenManager, int deltaX, int deltaY);

	virtual void proc13() {}

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
	virtual int proc38() { return 1; }
};
		
class CPetGlyphs : public List<CPetGlyph> {
protected:
	int _field10;
	int _field14;
	int _field18;
	int _field1C;
	int _field20;
	int _field24;
	CPetGfxElement _selection;
	CPetGfxElement _scrollLeft;
	CPetGfxElement _scrollRight;
protected:
	/**
	 * Change the currently selected glyph
	 */
	void changeHighlight(int index);
public:
	CPetGlyphs::CPetGlyphs() : _field10(0), _field14(7),
		_field18(-1), _field1C(-1), _field20(0), _field24(0) {
	}

	/**
	 * Clears the glyph list
	 */
	void clear();


	virtual void proc8();

	/**
	 * Set up the control
	 */
	virtual void setup();

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
};

} // End of namespace Titanic

#endif /* TITANIC_PET_GLYPHS_H */
