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

#ifndef TITANIC_PET_ROOMS_H
#define TITANIC_PET_ROOMS_H

#include "titanic/pet_control/pet_section.h"
#include "titanic/gfx/text_control.h"
#include "titanic/pet_control/pet_rooms_glyphs.h"
#include "titanic/game_location.h"

namespace Titanic {

class CPetRooms : public CPetSection {
private:
	CPetRoomsGlyphs _glyphs;
	CPetRoomsGlyph _glyphItem;
	CGameObject *_chevLeftOnDim;
	CGameObject *_chevLeftOffDim;
	CGameObject *_chevRightOnDim;
	CGameObject *_chevRightOffDim;
	CGameObject *_chevLeftOnLit;
	CGameObject *_chevLeftOffLit;
	CGameObject *_chevRightOnLit;
	CGameObject *_chevRightOffLit;
	CPetGfxElement _plinth;
	CTextControl _text;
	int _floorNum;
	int _elevatorNum;
	int _roomNum;
	int _sublevel;
	int _wellEntry;
	bool _elevatorBroken;
private:
	/**
	 * Setup the control
	 */
	bool setupControl(CPetControl *petControl);

	/**
	 * Returns the glyth position
	 */
	Point getGlyphPos() const { return Point(509, 388); }

	/**
	 * Adds a glyph to the list
	 */
	CPetRoomsGlyph *addRoom(uint roomFlags, bool highlight);

	/**
	 * Adds a glyph to the list
	 */
	CPetRoomsGlyph *addGlyph(uint roomFlags, bool highlight);
public:
	CPetRooms();

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
	virtual bool MouseDragStartMsg(CMouseDragStartMsg *msg);
	virtual bool MouseButtonUpMsg(CMouseButtonUpMsg *msg);
	virtual bool MouseDoubleClickMsg(CMouseDoubleClickMsg *msg);
	virtual bool VirtualKeyCharMsg(CVirtualKeyCharMsg *msg);

	/**
	 * Check whether a drag drop can occur
	 */
	virtual bool checkDragEnd(CGameObject *item);

	/**
	 * Display a message
	 */
	virtual void displayMessage(const CString &msg);

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
	 * Called when a section is switched to
	 */
	virtual void enter(PetArea oldArea);

	/**
	 * Called when a new room is entered
	 */
	virtual void enterRoom(CRoomItem *room);

	/**
	 * Get a reference to the tooltip text associated with the section
	 */
	virtual CTextControl *getText();

	/**
	 * Special retrieval of glyph background image
	 */
	virtual CGameObject *getBackground(int index) const;

	/**
	 * Reset the highlight
	 */
	void resetHighlight();

	/**
	 * Gives the player a new assigned room in the specified passenger class
	 */
	void reassignRoom(PassengerClass passClassNum);

	/**
	 * Change the current location passenger class
	 */
	bool changeLocationClass(PassengerClass newClassNum);

	/**
	 * Returns true if the specified location is the current or
	 * previously assigned room
	 */
	bool isAssignedRoom(uint roomFlags) const;

	/**
	 * Returns the room flags for the player's currently assigned room
	 */
	uint getAssignedRoomFlags() const;

	/**
	 * Returns the room number for the player's currently assigned room
	 */
	int getAssignedRoomNum() const;

	/**
	 * Returns the floor number for the player's currently assigned room
	 */
	int getAssignedFloorNum() const;

	/**
	 * Returns the elevator number for the player's currently assigned room
	 */
	int getAssignedElevatorNum() const;

	/**
	 * Gets room flags to use for glyphs
	 */
	uint getRoomFlags() const;

	void setFloorNum(int floorNum) { _floorNum = floorNum; }
	int getFloorNum() const { return _floorNum; }
	void setElevatorNum(int elevNum) { _elevatorNum = elevNum; }
	int getElevatorNum() const { return _elevatorNum; }
	void setRoomNum(int roomNum) { _roomNum = roomNum; }
	int getRoomNum() const { return _roomNum; }
	void setSublevel(int level) { _sublevel = level; }
	int getSublevel() const { return _sublevel; }

	/**
	 * Sets the entry number for arriving at the well
	 */
	void setWellEntry(int val) { _wellEntry = val; }

	/**
	 * Gets the entry number used when last arriving at the well
	 */
	int getWellEntry() const { return _wellEntry; }

	/**
	 * Sets the broken elevator flag
	 */
	void setElevatorBroken(bool flag) { _elevatorBroken = flag; }
};

} // End of namespace Titanic

#endif /* TITANIC_PET_ROOMS_H */
