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
#include "titanic/pet_control/pet_text.h"

namespace Titanic {

class CPetSaveGlyphs : public CPetGlyphs {
};

class CPetRealLife : public CPetSection {
private:
	CPetGlyphs _glyphs;
	CPetText _text;
private:
	/**
	 * Does setup
	 */
	bool setupControl(CPetControl *petControl);
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
	virtual Rect getBounds() { return Rect(); }
	
	virtual void proc5(int val) {}
	
	/**
	 * Following are handlers for the various messages that the PET can
	 * pass onto the currently active section/area
	 */
	virtual bool MouseButtonDownMsg(CMouseButtonDownMsg *msg) { return false; }
	virtual bool MouseDragStartMsg(CMouseDragStartMsg *msg) { return false; }
	virtual bool MouseDragMoveMsg(CMouseDragMoveMsg *msg) { return false; }
	virtual bool MouseDragEndMsg(CMouseDragEndMsg *msg) { return false; }
	virtual bool MouseButtonUpMsg(CMouseButtonUpMsg *msg) { return false; }
	virtual bool MouseDoubleClickMsg(CMouseDoubleClickMsg *msg) { return false; }
	virtual bool KeyCharMsg(CKeyCharMsg *msg) { return false; }
	virtual bool VirtualKeyCharMsg(CVirtualKeyCharMsg *msg) { return false; }

	virtual int proc14() { return 0; }
	
	/**
	 * Returns item a drag-drop operation has dropped on, if any
	 */
	virtual CGameObject *dragEnd(const Point &pt) const { return nullptr; }
	
	/**
	 * Display a message
	 */
	virtual void displayMessage(const CString &msg) {}

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
	virtual void postLoad() {}

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const {}

	/**
	 * Called when a section is switched to
	 */
	virtual void enter(PetArea oldArea) {}
	
	/**
	 * Called when a section is being left, to switch to another area
	 */
	virtual void leave() {}
	
	virtual void proc23() {}

	/**
	 * Called when a new room is entered
	 */
	virtual void enterRoom(CRoomItem *room) {}

};

} // End of namespace Titanic

#endif /* TITANIC_PET_REAL_LIFE_H */
