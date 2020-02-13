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

#ifndef TITANIC_PET_REAL_LIFE_H
#define TITANIC_PET_REAL_LIFE_H

#include "titanic/pet_control/pet_section.h"
#include "titanic/pet_control/pet_glyphs.h"
#include "titanic/gfx/text_control.h"

namespace Titanic {

class CPetSaveGlyphs : public CPetGlyphs {
};

class CPetRealLife : public CPetSection {
private:
	CPetGlyphs _glyphs;
	CTextControl _text;
private:
	/**
	 * Does setup
	 */
	bool setupControl(CPetControl *petControl);

	/**
	 * Adds one of the four button glyphs for display
	 */
	void addButton(CPetGlyph *glyph);
public:
	~CPetRealLife() override {}

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
	 * Get the bounds for the section
	 */
	Rect getBounds() const override { return Rect(); }

	/**
	 * Following are handlers for the various messages that the PET can
	 * pass onto the currently active section/area
	 */
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg) override;
	bool MouseDragStartMsg(CMouseDragStartMsg *msg) override;
	bool MouseDragMoveMsg(CMouseDragMoveMsg *msg) override;
	bool MouseDragEndMsg(CMouseDragEndMsg *msg) override;
	bool MouseButtonUpMsg(CMouseButtonUpMsg *msg) override;
	bool KeyCharMsg(CKeyCharMsg *msg) override;
	bool VirtualKeyCharMsg(CVirtualKeyCharMsg *msg) override;

	/**
	 * Returns item a drag-drop operation has dropped on, if any
	 */
	CGameObject *dragEnd(const Point &pt) const override { return nullptr; }

	/**
	 * Returns true if the object is in a valid state
	 */
	bool isValid(CPetControl *petControl) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file, int param) override {}

	/**
	 * Called after a game has been loaded
	 */
	void postLoad() override;

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override {}

	/**
	 * Called when a section is switched to
	 */
	void enter(PetArea oldArea) override;

	/**
	 * Called when a section is being left, to switch to another area
	 */
	void leave() override;

	/**
	 * Called when a new room is entered
	 */
	void enterRoom(CRoomItem *room) override {}

	/**
	 * Get a reference to the tooltip text associated with the section
	 */
	CTextControl *getText() override { return &_text; }

	/**
	 * Handles updates to the sound levels
	 */
	void syncSoundSettings();
};

} // End of namespace Titanic

#endif /* TITANIC_PET_REAL_LIFE_H */
