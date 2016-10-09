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

#ifndef TITANIC_PET_STARFIELD_H
#define TITANIC_PET_STARFIELD_H

#include "titanic/pet_control/pet_section.h"
#include "titanic/pet_control/pet_text.h"
#include "titanic/pet_control/pet_gfx_element.h"

namespace Titanic {

class CPetStarfield : public CPetSection {
private:
	CPetGfxElement _imgStarfield;
	CPetGfxElement _imgPhoto;
	CPetGfxElement _imgStarCtrl;
	CPetGfxElement _btnSetDest;
	int _btnOffsets[3];
	CPetGfxElement _leds[6];
	Rect _rect1;
	int _field18C;
	CPetText _text;
	bool _photoOn;
	bool _hasReference;
private:
	/**
	 * Setup the control
	 */
	bool setupControl(CPetControl *petControl);

	/**
	 * Draw a button
	 */
	void drawButton(int offset, int index, CScreenManager *screenManager);

	/**
	 * Mouse down handling for Nav elements
	 */
	bool elementsMouseDown(CMouseButtonDownMsg *msg);

	bool elementMouseButton(int index, CMouseButtonDownMsg *msg, const Rect &rect);
public:
	CPetStarfield();

	/**
	 * Sets up the section
	 */
	virtual bool setup(CPetControl *petControl);

	/**
	 * Reset the section
	 */
	virtual bool reset();

	/**
	 * Draw the section
	 */
	virtual void draw(CScreenManager *screenManager);

	/**
	* Following are handlers for the various messages that the PET can
	* pass onto the currently active section/area
	*/
	virtual bool MouseButtonDownMsg(CMouseButtonDownMsg *msg);
	virtual bool MouseButtonUpMsg(CMouseButtonUpMsg *msg);

	/**
	 * Returns true if the object is in a valid state
	 */
	virtual bool isValid(CPetControl *petControl);

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file, int param);

	/**
	 * Called after a game has been loaded
	 */
	virtual void postLoad();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent);

	/**
	 * Sets the offsets for each of the buttons
	 */
	void setButtons(int val1, int val2);

	/**
	 * Sets whether the player has the galactic reference material
	 */
	void setHasReference(bool hasRef) { _hasReference = hasRef; }

	/**
	 * Make the PET as dirty, requiring a redraw
	 */
	void makePetDirty();
};

} // End of namespace Titanic

#endif /* TITANIC_PET_STARFIELD_H */
