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

#ifndef TITANIC_PET_INVENTORY_H
#define TITANIC_PET_INVENTORY_H

#include "titanic/support/simple_file.h"
#include "titanic/pet_control/pet_section.h"
#include "titanic/pet_control/pet_control_sub10.h"
#include "titanic/pet_control/pet_control_sub12.h"

namespace Titanic {

/**
 * Handles displaying the player's inventory in the PET
 */
class CPetInventory : public CPetSection {
private:
	CPetControlSub12 _sub12;
	CPetControlSub10 _sub10;
	CGameObject *_itemBackgrounds[46];
	CGameObject *_itemGlyphs[46];
	int _field28C;
	int _field290;
	int _field294;
	int _field298;
private:
	/**
	 * Handles initial setup
	 */
	bool setPetControl(CPetControl *petControl);
public:
	CPetInventory();

	/**
	 * Sets up the section
	 */
	virtual bool setup(CPetControl *petControl);
	
	/**
	 * Sets up the section
	 */
	virtual bool setup();

	/**
	 * Draw the section
	 */
	virtual void draw(CScreenManager *screenManager);
	
	/**
	 * Get the bounds for the section
	 */
	virtual Rect getBounds();
	
	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file, int param);

	/**
	 * Returns true if the object is in a valid state
	 */
	virtual bool isValid(CPetControl *petControl);
};

} // End of namespace Titanic

#endif /* TITANIC_PET_INVENTORY_H */
