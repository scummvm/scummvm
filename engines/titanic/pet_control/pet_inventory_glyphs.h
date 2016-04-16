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

#ifndef TITANIC_PET_INVENTORY_GLYPHS_H
#define TITANIC_PET_INVENTORY_GLYPHS_H

#include "titanic/pet_control/pet_glyphs.h"
#include "titanic/support/screen_manager.h"

namespace Titanic {

class CPetInventoryGlyph : public CPetGlyph {
public:
	CCarry *_item;
	int _field34;
	int _field38;
	int _field3C;
	int _field40;
public:
	CPetInventoryGlyph(CCarry *item, int val) : _item(item),
		_field34(val), _field38(0), _field3C(0), _field40(0) {}
};

class CPetCarry {
public:
	int _val;
	CCarry *_item;
public:
	CPetCarry(CCarry *item, int val) : _item(item), _val(val) {}
};

class CPetInventoryGlyphs : public CPetGlyphs<CPetInventoryGlyph> {
private:
	void fn1(int val);
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
public:
	CPetInventoryGlyphs();

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
	 * Add a new item to the list
	 */
	void addItem(CPetCarry *item);

	/**
	 * Clears the glyph list
	 */
	void clear();
};

} // End of namespace Titanic

#endif /* TITANIC_PET_INVENTORY_GLYPHS_H */
