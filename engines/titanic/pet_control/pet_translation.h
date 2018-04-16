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

#ifndef TITANIC_PET_TRANSLATION_H
#define TITANIC_PET_TRANSLATION_H

#include "titanic/pet_control/pet_section.h"
#include "titanic/gfx/text_control.h"

namespace Titanic {

class CPetTranslation : public CPetSection {
private:
	CTextControl _message;
	CTextControl _tooltip;
private:
	/**
	 * Setup the control
	 */
	bool setupControl(CPetControl *petControl);
public:
	CPetTranslation();

	/**
	 * Sets up the section
	 */
	virtual bool setup(CPetControl *petControl);

	/**
	 * Reset the section
	 */
	virtual bool reset() { return true; }

	/**
	 * Draw the section
	 */
	virtual void draw(CScreenManager *screenManager);

	/**
	 * Following are handlers for the various messages that the PET can
	 * pass onto the currently active section/area
	 */
	virtual bool MouseButtonDownMsg(CMouseButtonDownMsg *msg) { return false; }
	virtual bool MouseButtonUpMsg(CMouseButtonUpMsg *msg) { return false; }

	/**
	 * Returns true if the object is in a valid state
	 */
	virtual bool isValid(CPetControl *petControl) {
		return setupControl(petControl);
	}

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file, int param) {}

	/**
	 * Called after a game has been loaded
	 */
	virtual void postLoad() { reset(); }

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) {}

	/**
	 * Get a reference to the tooltip text associated with the section
	 */
	virtual CTextControl *getText() { return &_tooltip; }

	/**
	 * Clear any current translation text
	 */
	void clearTranslation();

	/**
	 * Adds a line to the translation display
	 */
	void addTranslation(const CString &str1, const CString &str2);
};

} // End of namespace Titanic

#endif /* TITANIC_PET_TRANSLATION_H */
