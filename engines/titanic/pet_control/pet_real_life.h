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
	virtual ~CPetRealLife() {}

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
	 * Get the bounds for the section
	 */
	virtual Rect getBounds() const { return Rect(); }

	/**
	 * Following are handlers for the various messages that the PET can
	 * pass onto the currently active section/area
	 */
	virtual bool MouseButtonDownMsg(CMouseButtonDownMsg *msg);
	virtual bool MouseDragStartMsg(CMouseDragStartMsg *msg);
	virtual bool MouseDragMoveMsg(CMouseDragMoveMsg *msg);
	virtual bool MouseDragEndMsg(CMouseDragEndMsg *msg);
	virtual bool MouseButtonUpMsg(CMouseButtonUpMsg *msg);
	virtual bool KeyCharMsg(CKeyCharMsg *msg);
	virtual bool VirtualKeyCharMsg(CVirtualKeyCharMsg *msg);

	/**
	 * Returns item a drag-drop operation has dropped on, if any
	 */
	virtual CGameObject *dragEnd(const Point &pt) const { return nullptr; }

	/**
	 * Returns true if the object is in a valid state
	 */
	virtual bool isValid(CPetControl *petControl);

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file, int param) {}

	/**
	 * Called after a game has been loaded
	 */
	virtual void postLoad();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) {}

	/**
	 * Called when a section is switched to
	 */
	virtual void enter(PetArea oldArea);

	/**
	 * Called when a section is being left, to switch to another area
	 */
	virtual void leave();

	/**
	 * Called when a new room is entered
	 */
	virtual void enterRoom(CRoomItem *room) {}

	/**
	 * Get a reference to the tooltip text associated with the section
	 */
	virtual CTextControl *getText() { return &_text; }

	/**
	 * Handles updates to the sound levels
	 */
	void syncSoundSettings();
};

} // End of namespace Titanic

#endif /* TITANIC_PET_REAL_LIFE_H */
