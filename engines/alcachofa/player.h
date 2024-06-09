/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "rooms.h"

namespace Alcachofa {

class Player {
public:
	Player();

	inline Room *currentRoom() const { return _currentRoom; }
	inline MainCharacter *activeCharacter() const { return _activeCharacter; }
    inline ShapeObject *&selectedObject() { return _selectedObject; }
	inline ShapeObject *&pressedObject() { return _pressedObject; }
	inline Item *&heldItem() { return _heldItem; }
	inline FakeSemaphore &semaphore() { return _semaphore; }
	MainCharacter *inactiveCharacter() const;
	FakeSemaphore &semaphoreFor(MainCharacterKind kind);

	inline bool &isOptionsMenuOpen() { return _isOptionsMenuOpen; }
	inline bool &isGameLoaded() { return _isGameLoaded; }

	inline MainCharacterKind activeCharacterKind() const {
		return _activeCharacter == nullptr ? MainCharacterKind::None : _activeCharacter->kind();
	}

	void preUpdate();
	void postUpdate();
	void updateCursor();
	void drawCursor(bool forceDefaultCursor = false);
	void changeRoom(const Common::String &targetRoomName, bool resetCamera);
	void changeRoomToBeforeInventory();
	void triggerObject(ObjectBase *object, const char *action);
	void triggerDoor(const Door *door);

private:
	Common::ScopedPtr<Animation> _cursorAnimation;
	FakeSemaphore _semaphore;
	Room *_currentRoom = nullptr,
		*_roomBeforeInventory = nullptr;
	MainCharacter *_activeCharacter;
    ShapeObject *_selectedObject = nullptr;
    ShapeObject *_pressedObject = nullptr;
	Item *_heldItem = nullptr;
	int32 _cursorFrameI = 0;
	bool
		_isOptionsMenuOpen = false,
		_isGameLoaded = true,
		_didLoadGlobalRooms = false;
};

}

#endif // PLAYER_H
