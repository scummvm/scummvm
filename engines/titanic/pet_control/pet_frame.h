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

#ifndef TITANIC_PET_FRAME_H
#define TITANIC_PET_FRAME_H

#include "titanic/pet_control/pet_section.h"
#include "titanic/pet_control/pet_graphic.h"

namespace Titanic {

class CPetFrame : public CPetSection {
private:
	static int _indexes[6];

	CPetGraphic _modeButtons[6];
	CPetGraphic _titles[6];
	CPetGraphic _modeBackground;
	CPetGraphic _val2;
	CPetGraphic _val3;
	CPetGraphic _background;
	CPetGraphic _indent[7];
private:
	/**
	 * Called to set the owning PET instance and set some initial state
	 */
	bool setPetControl(CPetControl *petControl);
public:
	CPetFrame();

	/**
	 * Sets up the section
	 */
	virtual bool setup(CPetControl *petControl);
	
	/**
	 * Sets up the section
	 */
	virtual bool setup();

	/**
	 * Returns true if the object is in a valid state
	 */
	virtual bool isValid(CPetControl *petControl);

	/**
	 * Called after a game has been loaded
	 */
	virtual void postLoad();

	/**
	 * Called when the current PET area changes
	 */
	void setArea(PetArea newArea);

	/**
	 * Draws the PET frame
	 */
	void drawFrame(CScreenManager *screenManager);
};

} // End of namespace Titanic

#endif /* TITANIC_PET_FRAME_H */
