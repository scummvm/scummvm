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

#ifndef TITANIC_PET_CONTROL_H
#define TITANIC_PET_CONTROL_H

#include "titanic/core/game_object.h"
#include "titanic/core/node_item.h"
#include "titanic/core/room_item.h"
#include "titanic/messages/messages.h"
#include "titanic/messages/mouse_messages.h"
#include "titanic/pet_control/pet_conversations.h"
#include "titanic/pet_control/pet_frame.h"
#include "titanic/pet_control/pet_inventory.h"
#include "titanic/pet_control/pet_message.h"
#include "titanic/pet_control/pet_nav_helmet.h"
#include "titanic/pet_control/pet_real_life.h"
#include "titanic/pet_control/pet_remote.h"
#include "titanic/pet_control/pet_rooms.h"
#include "titanic/room_flags.h"

namespace Titanic {

enum SummonResult { SUMMON_CANT = 0, SUMMON_PRESENT = 1, SUMMON_CAN = 2 };

class CPetControl : public CGameObject {
	DECLARE_MESSAGE_MAP
	struct PetEventInfo {
		int _id;
		void *_target;
		PetEventInfo() : _id(0), _target(nullptr) {}
	};
private:
	int _fieldC0;
	int _locked;
	int _fieldC8;
	CPetSection *_sections[7];
	CPetConversations _conversations;
	CPetInventory _inventory;
	CPetNavHelmet _navHelmet;
	CPetRemote _remote;
	CPetRooms _rooms;
	CPetRealLife _realLife;
	CPetMessage _message;
	CPetFrame _frame;
	CString _activeNPCName;
	CString _remoteTargetName;
	CRoomItem *_hiddenRoom;
	Rect _drawBounds;
	PetEventInfo _timers[2];
private:
	/**
	 * Returns true if the control is in a valid state
	 */
	bool isValid();

	/**
	 * Loads data for the individual areas
	 */
	void loadAreas(SimpleFile *file, int param);

	/**
	 * Saves data for the individual areas
	 */
	void saveAreas(SimpleFile *file, int indent) const;

	/**
	 * Called at the end of the post game-load handling
	 */
	void loaded();

	/**
	 * Returns true if the draw bounds contains the specified point
	 */
	bool containsPt(const Common::Point &pt) const;

	/**
	 * Checks whether a designated NPC in present in the current view
	 */
	bool isNPCInView(const CString &name) const;

	void setTimer44(int id, int val);
protected:
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg);
	bool MouseDragStartMsg(CMouseDragStartMsg *msg);
	bool MouseDragMoveMsg(CMouseDragMoveMsg *msg);
	bool MouseDragEndMsg(CMouseDragEndMsg *msg);
	bool MouseButtonUpMsg(CMouseButtonUpMsg *msg);
	bool MouseDoubleClickMsg(CMouseDoubleClickMsg *msg);
	bool KeyCharMsg(CKeyCharMsg *msg);
	bool VirtualKeyCharMsg(CVirtualKeyCharMsg *msg);
	bool TimerMsg(CTimerMsg *msg);
public:
	PetArea _currentArea;
	CTreeItem *_activeNPC;
	CGameObject *_remoteTarget;
public:
	CLASSDEF
	CPetControl();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);

	/**
	 * Allows the item to draw itself
	 */
	virtual void draw(CScreenManager *screenManager);

	/**
	 * Gets the bounds occupied by the item
	 */
	virtual Rect getBounds();

	/**
	 * Setups the sections within the PET
	 */
	void setup();

	/**
	 * Called after loading a game has finished
	 */
	void postLoad();

	/**
	 * Called when a new node is entered
	 */
	void enterNode(CNodeItem *node);

	/**
	 * Called when a new room is entered
	 */
	void enterRoom(CRoomItem *room);

	/**
	 * Called to reset the remote target
	 */
	void resetRemoteTarget();

	/**
	 * Resets the Active NPC
	 */
	void resetActiveNPC();

	bool fn1(int val);

	/**
	 * Set the remote target
	 */
	void setRemoteTarget(CGameObject *item);

	/**
	 * Sets the currently viewed area within the PET
	 */
	PetArea setArea(PetArea newSection);

	/**
	 * Hides the text cursor in the current section, if applicable
	 */
	void hideCursor();

	/**
	 * Shows the text cursor in the current section, if applicable
	 */
	void showCursor();

	/**
	 * Highlights a glyph item in the currently active section, if applicable
	 */
	void highlightGlyph(int id);

	/**
	 * Returns true if the PET is currently unlocked
	 */
	bool isUnlocked() const { return _locked == 0; }

	/**
	 * Returns a game object used by the PET by name from within the
	 * special hidden room container
	 */
	CGameObject *getHiddenObject(const CString &name);

	/**
	 * Returns a reference to the special hidden room container
	 */
	CRoomItem *getHiddenRoom();

	/**
	 * Draws squares for showing glyphs inside
	 */
	void drawSquares(CScreenManager *screenManager, int count);

	/**
	 * Returns true if the point is within the PET's draw bounds
	 */
	bool contains(const Point &pt) const {
		return _drawBounds.contains(pt);
	}

	/**
	 * Handles drag ends within the PET
	 */
	CGameObject *dragEnd(const Point &pt) const;

	/**
	 * Handles checking when a drag-drop operation ends
	 */
	bool checkDragEnd(CGameObject *item) const;

	/**
	 * Display a message
	 */
	void displayMessage(const CString &msg) const;

	/**
	 * Get the first game object stored in the PET
	 */
	CGameObject *getFirstObject() const;

	/**
	 * Get the next game object stored in the PET following
	 * the passed game object
	 */
	CGameObject *getNextObject(CGameObject *prior) const;

	/**
	 * Adds an item to the PET inventory
	 */
	void addToInventory(CCarry *item);

	/**
	 * Remove an item from the inventory
	 */
	void removeFromInventory(CCarry *item, CTreeItem *newParent,
		bool refreshUI = true, bool sendMsg = true);

	/**
	 * Remove an item from the inventory
	 */
	void removeFromInventory(CCarry *item, bool refreshUI = true, bool sendMsg = true);

	/**
	 * Called when the status of an item in the inventory has changed
	 */
	void invChange(CCarry *item);

	/**
	 * Moves a tree item from it's original position to be under the hidden room
	 */
	void moveToHiddenRoom(CTreeItem *item);

	void setC8(int val) { _fieldC8 = val; }

	/**
	 * Play a sound
	 */
	void playSound(int soundNum);

	/**
	 * Get the room name
	 */
	CString getRoomName() const;

	/**
	 * Check whether an NPC can be summoned
	 */
	int canSummonNPC(const CString &name);

	/**
	 * Summon an NPC to the player
	 */
	void summonNPC(const CString &name, int val);

	/**
	 * Start a timer
	 */
	void startPetTimer(uint timerIndex, uint firstDuration, uint duration, void *target);

	/**
	 * Stop a timer
	 */
	void stopPetTimer(uint timerIndex);

	/**
	 * Return the full Id of the current view in a
	 * room.node.view tuplet form
	 */
	CString getFullViewName();

	/**
	 * Resets the dial display to reflect new values
	 */
	void resetDials(int flag = 1);

	bool getC0() const { return _fieldC0 > 0; }
	void incC0() { ++_fieldC0; }
	void decC0() { --_fieldC0; }

	/* CPetRooms methods */

	/**
	 * Adds a random room to the room list
	 */
	void addRandomRoom(int passClassNum) {
		_rooms.addRandomRoom(passClassNum);
	}

	/**
	 * Change the current location passenger class
	 */
	bool changeLocationClass(int newClassNum) {
		return _rooms.changeLocationClass(newClassNum);
	}

	/**
	 * Returns true if the Rooms list has a room with the given flags
	 */
	bool hasRoomFlags() const {
		return _rooms.hasRoomFlags(getRoomFlags());
	}

	uint getRoomFlags() const {
		return _rooms.getRoomFlags();
	}

	/**
	 * Set the current elevator number to use for room glyphs
	 */
	void setRoomsElevatorNum(int elevNum) {
		_rooms.setElevatorNum(elevNum);
	}

	/**
	 * Get the current elevator number used by room glyphs
	 */
	int getRoomsElevatorNum() const {
		return _rooms.getElevatorNum();
	}

	/**
	 * Set the current floor number to use for room glyphs
	 */
	void setRoomsFloorNum(int floorNum) {
		_rooms.setFloorNum(floorNum);
	}

	/**
	 * Get the current floor number used by room glyphs
	 */
	int getRoomsFloorNum() const {
		return _rooms.getFloorNum();
	}

	/**
	 * Set the current room number to use for room glyphs
	 */
	void setRoomsRoomNum(int roomNum) {
		_rooms.setRoomNum(roomNum);
	}

	/**
	 * Get the current floor number used by room glyphs
	 */
	int getRoomsRoomNum() const {
		return _rooms.getRoomNum();
	}

	void setRooms1D0(int v) {
		_rooms.set1D0(v);
	}
	int getRooms1D0() const {
		return _rooms.get1D0();
	}
	void setRooms1CC(int v) {
		_rooms.set1CC(v);
	}
	int getRooms1CC() const {
		return _rooms.get1CC();
	}

	/**
	 * Reset the highlight
	 */
	void resetRoomsHighlight() {
		_rooms.resetHighlight();
	}

	int getRoomsMode1Flags() const {
		return _rooms.mode1Flags();
	}

	uint getSpecialRoomFlags(const CString &name) {
		return CRoomFlags::getSpecialRoomFlags(name);
	}

	/**
	 * Get mail destination given the specified flags
	 */
	int getMailDest(const CRoomFlags &roomFlags) const;

	bool testRooms5(uint roomFlags) {
		return CRoomFlags(roomFlags).not5();
	}

	int getRoomsRoomNum1() const {
		return _rooms.getMode1RoomNum();
	}
	int getRoomsFloorNum1() const {
		return _rooms.getMode1FloorNum();
	}
	int getRoomsElevatorNum1() const {
		return _rooms.getMode1ElevatorNum();
	}

	void setRooms1D4(int val) {
		_rooms.set1D4(val);
	}

	bool isRoom59706() const {
		return CRoomFlags(getRoomFlags()).is59706();
	}

	/**
	 * Returns true if the passed room flags indicate the room has a succubus
	 */
	bool isSuccUBusRoom(const CRoomFlags &roomFlags) {
		return roomFlags.isSuccUBusRoomFlags();
	}
};

} // End of namespace Titanic

#endif /* TITANIC_PET_CONTROL_H */
