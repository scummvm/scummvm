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

#ifndef TITANIC_PET_SECTION_H
#define TITANIC_PET_SECTION_H

#include "titanic/messages/mouse_messages.h"
#include "titanic/support/simple_file.h"

namespace Titanic {

enum PetArea {
	PET_INVENTORY = 0, PET_CONVERSATION = 1, PET_REMOTE = 2,
	PET_ROOMS = 3, PET_REAL_LIFE = 4, PET_STARFIELD = 5, PET_TRANSLATION = 6
};

class CPetControl;
class CPetElement;
class CTextControl;
class CScreenManager;
class CRoomItem;

struct CPetSectionSubData {
	int _field0;
	int _field4;
	int _field8;
	int _fieldC;

	CPetSectionSubData() : _field0(0), _field4(0), _field8(0),
		_fieldC(0) {}
};

class CPetSection {
public:
	CPetControl *_petControl;
protected:
	/**
	 * Called when the current area is changed
	 */
	void areaChanged(PetArea area);

	/**
	 * Returns the name of the currently active NPC, if any
	 */
	CString getActiveNPCName() const;

	/**
	 * Create a color table
	 */
	void copyColors(uint tableNum, uint colors[5]);
public:
	CPetSection() : _petControl(nullptr) {}
	virtual ~CPetSection() {}

	/**
	 * Sets up the section
	 */
	virtual bool setup(CPetControl *petControl) { return false; }

	/**
	 * Reset the section
	 */
	virtual bool reset() { return false; }

	/**
	 * Draw the section
	 */
	virtual void draw(CScreenManager *screenManager) {}

	/**
	 * Get the bounds for the section
	 */
	virtual Rect getBounds() const { return Rect(); }

	/**
	 * Called when a general change occurs
	 */
	virtual void changed(int changeType) {}

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
	virtual bool MouseWheelMsg(CMouseWheelMsg *msg) { return false; }
	virtual bool KeyCharMsg(CKeyCharMsg *msg) { return false; }
	virtual bool VirtualKeyCharMsg(CVirtualKeyCharMsg *msg) { return false; }

	/**
	 * Check whether a drag drop can occur
	 */
	virtual bool checkDragEnd(CGameObject *item) { return false; }

	/**
	 * Returns item a drag-drop operation has dropped on, if any
	 */
	virtual CGameObject *dragEnd(const Point &pt) const { return nullptr; }

	/**
	 * Display a message
	 */
	virtual void displayMessage(const CString &msg);

	/**
	 * Returns true if the object is in a valid state
	 */
	virtual bool isValid(CPetControl *petControl) { return false; }

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
	virtual void save(SimpleFile *file, int indent) {}

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

	/**
	 * Called when a previously set up PET timer expires
	 */
	virtual void timerExpired(int val);

	/**
	 * Get a reference to the tooltip text associated with the section
	 */
	virtual CTextControl *getText() { return nullptr; }

	/**
	 * Removes text after a given duration
	 */
	virtual void removeText(int duration);

	/**
	 * Removes text after a given duration
	 */
	virtual void removeText();

	/**
	 * Stops the text removal timer
	 */
	virtual void stopTextTimer();

	/**
	 * Get an element from the section by a designated Id
	 */
	virtual CPetElement *getElement(uint id) { return nullptr; }

	/**
	 * Special retrieval of glyph background image
	 */
	virtual CGameObject *getBackground(int index) const { return nullptr; }

	/**
	 * Display a title for an NPC
	 */
	virtual void displayNPCName(CGameObject *npc) {}

	virtual void proc33() {}

	/**
	 * Sets the NPC to use
	 */
	virtual void setNPC(const CString &name) {}

	/**
	 * Resets the active NPC
	 */
	virtual void resetNPC() {}

	/**
	 * Show the text cursor
	 */
	virtual void showCursor() {}

	/**
	 * Hide the text cursor
	 */
	virtual void hideCursor() {}

	/**
	 * Highlights a glyph item in the section, if applicable
	 */
	virtual void highlight(int id) {}

	/**
	 * Get the PET control
	 */
	CPetControl *getPetControl() const { return _petControl; }

	/**
	 * Get a specified color in the currently active UI color table
	 */
	uint getColor(uint index);

	/**
	 * Get one of the game's three UI color tables. If the default
	 * tableNum of -1 is used, the table is taken from the game state
	 */
	const uint *getColorTable(int tableNum = -1);
};

} // End of namespace Titanic

#endif /* TITANIC_PET_SECTION_H */
