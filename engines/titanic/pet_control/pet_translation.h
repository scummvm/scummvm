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
	bool setup(CPetControl *petControl) override;

	/**
	 * Reset the section
	 */
	bool reset() override { return true; }

	/**
	 * Draw the section
	 */
	void draw(CScreenManager *screenManager) override;

	/**
	 * Following are handlers for the various messages that the PET can
	 * pass onto the currently active section/area
	 */
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg) override { return false; }
	bool MouseButtonUpMsg(CMouseButtonUpMsg *msg) override { return false; }

	/**
	 * Returns true if the object is in a valid state
	 */
	bool isValid(CPetControl *petControl) override {
		return setupControl(petControl);
	}

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file, int param) override {}

	/**
	 * Called after a game has been loaded
	 */
	void postLoad() override { reset(); }

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override {}

	/**
	 * Get a reference to the tooltip text associated with the section
	 */
	CTextControl *getText() override { return &_tooltip; }

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
