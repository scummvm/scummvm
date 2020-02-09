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
#include "titanic/gfx/text_control.h"
#include "titanic/pet_control/pet_gfx_element.h"

namespace Titanic {

enum MarkerState { MS_BLANK = 0, MS_FLICKERING = 1, MS_HIGHLIGHTED = 2};

class CPetStarfield : public CPetSection {
private:
	CPetGfxElement _imgStarfield;
	CPetGfxElement _imgPhoto;
	CPetGfxElement _imgStarCtrl;
	CPetGfxElement _btnSetDest;
	MarkerState _markerStates[3];
	CPetGfxElement _leds[6];
	Rect _rect1;
	int _flickerCtr;
	CTextControl _text;
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
	void drawButton(MarkerState state, int index, CScreenManager *screenManager);

	/**
	 * Handles clicking on any of the three locked star LED markers
	 */
	bool markersMouseDown(CMouseButtonDownMsg *msg);

	/**
	 * Handles clicking on a specific locked star LED marker
	 */
	bool markerMouseDown(int index, CMouseButtonDownMsg *msg, const Rect &rect);
public:
	CPetStarfield();

	/**
	 * Sets up the section
	 */
	bool setup(CPetControl *petControl) override;

	/**
	 * Reset the section
	 */
	bool reset() override;

	/**
	 * Draw the section
	 */
	void draw(CScreenManager *screenManager) override;

	/**
	* Following are handlers for the various messages that the PET can
	* pass onto the currently active section/area
	*/
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg) override;
	bool MouseButtonUpMsg(CMouseButtonUpMsg *msg) override;

	/**
	 * Returns true if the object is in a valid state
	 */
	bool isValid(CPetControl *petControl) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file, int param) override;

	/**
	 * Called after a game has been loaded
	 */
	void postLoad() override;

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Sets the display for the marker buttons
	 */
	void setButtons(int matchIndex, bool isMarkerClose);

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
