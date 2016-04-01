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

#ifndef TITANIC_PET_ELEMENT_H
#define TITANIC_PET_ELEMENT_H

#include "titanic/simple_file.h"
#include "titanic/string.h"
#include "titanic/core/link_item.h"

namespace Titanic {

enum PetElementMode { MODE_UNSELECTED = 0, MODE_SELECTED = 1, MODE_2 = 2 };

class CGameObject;
class CPetControl;

class CPetElement {
protected:
	Rect _bounds;
	PetElementMode _mode;
public:
	CPetElement();
	virtual ~CPetElement() {}

	/**
	 * Sets up the element
	 */
	virtual void setup(PetElementMode mode, const CString &name,
		CPetControl *petControl) {}

	/**
	 * Sets up the element
	 */
	virtual void setup(int val, const CString &name, CPetControl *petControl) {}
	
	/**
	 * Draw the item
	 */
	virtual void draw(CScreenManager *screenManager) {}

	/**
	 * Draw the item
	 */
	virtual void draw(CScreenManager *screenManager, const Common::Point &destPos) {}

	/**
	 * Get the bounds for the element
	 */
	virtual void getBounds(Rect *rect);

	virtual bool proc6(const Common::Point &pt);
	virtual bool proc7(const Common::Point &pt);

	/**
	 * Returns whether the passed point falls inside the item
	 */
	virtual bool contains1(const Common::Point &pt) const;

	virtual int proc9(const Common::Point &pt);

	/**
	 * Returns whether the passed point falls inside the item
	 */
	virtual bool contains2(const Common::Point &pt) const;

	virtual void proc11(int val1, int val2) const;

	/**
	 * Change the status of the associated object
	 */
	virtual void changeStatus(int newStatus) const;

	/**
	 * Returns true if the object associated with the item has an active movie
	 */
	virtual bool hasActiveMovie() const;

	/**
	 * Loads a frame
	 */
	virtual void loadFrame(int frameNumber);

	virtual int proc15();

	/**
	 * Get the game object associated with this item
	 */
	virtual CGameObject *getObject() const { return nullptr; }

	virtual void changeMode(PetElementMode newMode) { _mode = newMode; }

	void setMode(PetElementMode mode);

	/**
	 * Set the bounds for the element
	 */
	void setBounds(const Rect &r) { _bounds = r; }

	/**
	 * Translate the position of the element
	 */
	void translate(int deltaX, int deltaY) { _bounds.translate(deltaX, deltaY); }
};

} // End of namespace Titanic

#endif /* TITANIC_PET_ELEMENT_H */
