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

#ifndef TITANIC_PET_GFX_ELEMENT_H
#define TITANIC_PET_GFX_ELEMENT_H

#include "titanic/pet_control/pet_element.h"

namespace Titanic {

class CPetGfxElement: public CPetElement {
public:
	CGameObject *_object0;
	CGameObject *_object1;
	CGameObject *_object2;
public:
	CPetGfxElement() : CPetElement(), _object0(nullptr), _object1(nullptr),
		_object2(nullptr) {}

	/**
	 * Setup the element
	 */
	void setup(PetElementMode mode, const CString &name,
		CPetControl *petControl) override;

	/**
	 * Reset the element
	 */
	void reset(const CString &name, CPetControl *petControl,
		PetElementMode mode = MODE_UNSELECTED) override;

	/**
	 * Draw the item
	 */
	void draw(CScreenManager *screenManager) override;

	/**
	 * Draw the item
	 */
	void draw(CScreenManager *screenManager, const Common::Point &destPos) override;

	/**
	 * Get the bounds for the element
	 */
	Rect getBounds() const override;

	/**
	 * Get the game object associated with this item
	 */
	CGameObject *getObject() const override;

	/**
	 * Gets the explicit bounds set for the graphic element,
	 * ignoring any associated sub-object bounds
	 */
	const Rect &getRawBounds() const { return _bounds; }
};

} // End of namespace Titanic

#endif /* TITANIC_PET_GFX_ELEMENT_H */
