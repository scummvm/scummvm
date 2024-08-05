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

#include "player.h"
#include "script.h"
#include "alcachofa.h"

using namespace Common;

namespace Alcachofa {

Player::Player()
	: _activeCharacter(&g_engine->world().mortadelo()) {
	const auto &cursorPath = g_engine->world().getGlobalAnimationName(GlobalAnimationKind::Cursor);
	_cursorAnimation.reset(new Animation(cursorPath));
	_cursorAnimation->load();
}

void Player::preUpdate() {
	_selectedObject = nullptr;
	_cursorFrameI = 0;
}

void Player::postUpdate() {
	if (g_engine->input().wasAnyMouseReleased())
		_pressedObject = nullptr;
}

void Player::drawScreenStates() {
	if (_isPermanentFaded && !_isOptionsMenuOpen)
		g_engine->drawQueue().add<FadeDrawRequest>(FadeType::ToBlack, 1.0f, -9);
}

void Player::updateCursor() {
	if (_isOptionsMenuOpen || !_isGameLoaded)
		_cursorFrameI = 0;
	else if (_selectedObject == nullptr)
		_cursorFrameI = !g_engine->input().isMouseLeftDown() || _pressedObject != nullptr ? 6 : 7;
	else {
		auto type = _selectedObject->cursorType();
		switch (type) {
		case CursorType::Point: _cursorFrameI = 0; break;
		case CursorType::LeaveUp: _cursorFrameI = 8; break;
		case CursorType::LeaveRight: _cursorFrameI = 10; break;
		case CursorType::LeaveDown: _cursorFrameI = 12; break;
		case CursorType::LeaveLeft: _cursorFrameI = 14; break;
		case CursorType::WalkTo: _cursorFrameI = 6; break;
		default: error("Invalid cursor type %u", (uint)type); break;
		}

		if (_cursorFrameI != 0) {
			if (g_engine->input().isAnyMouseDown() && _pressedObject == _selectedObject)
				_cursorFrameI++;
		}
		else if (g_engine->input().isMouseLeftDown())
			_cursorFrameI = 2;
		else if (g_engine->input().isMouseRightDown())
			_cursorFrameI = 4;
	}

	drawCursor();
}

void Player::drawCursor(bool forceDefaultCursor) {
	Point cursorPos = g_engine->input().mousePos2D();
	if (_heldItem == nullptr || forceDefaultCursor) {
		if (forceDefaultCursor)
			_cursorFrameI = 0;
		g_engine->drawQueue().add<AnimationDrawRequest>(_cursorAnimation.get(), _cursorFrameI, as2D(cursorPos), -10);
	}
	else {
		auto itemGraphic = _heldItem->graphic();
		assert(itemGraphic != nullptr);
		auto &animation = itemGraphic->animation();
		auto frameOffset = animation.totalFrameOffset(0);
		auto imageSize = animation.imageSize(animation.imageIndex(0, 0));
		cursorPos -= frameOffset + imageSize / 2;
		g_engine->drawQueue().add<AnimationDrawRequest>(&animation, 0, as2D(cursorPos), -kForegroundOrderCount);
	}
}

void Player::changeRoom(const Common::String &targetRoomName, bool resetCamera) {
	debug("Change room to %s", targetRoomName.c_str());

	// original would be to always free all resources from globalRoom, inventory, GlobalUI
	if (targetRoomName.equalsIgnoreCase("SALIR")) {
		_currentRoom = nullptr;
		return;
	}

	Room &inventory = g_engine->world().inventory();
	bool keepResources;
	if (_currentRoom == &inventory)
		keepResources = _roomBeforeInventory != nullptr && _roomBeforeInventory->name().equalsIgnoreCase(targetRoomName);
	else {
		keepResources = _currentRoom != nullptr && _currentRoom->name().equalsIgnoreCase(targetRoomName);
	}
	_roomBeforeInventory = nullptr;
	if (targetRoomName.equalsIgnoreCase("inventario")) {
		keepResources = true;
		_roomBeforeInventory = _currentRoom;
	}

	if (!keepResources && _currentRoom != nullptr) {
		g_engine->scheduler().killProcessByName("ACTUALIZAR_" + _currentRoom->name());
		_currentRoom->freeResources();
	}
	_currentRoom = g_engine->world().getRoomByName(targetRoomName);
	if (_currentRoom == nullptr)
		error("Invalid room name: %s", targetRoomName.c_str());

	if (!_didLoadGlobalRooms) {
		_didLoadGlobalRooms = true;
		inventory.loadResources();
		g_engine->world().globalRoom().loadResources();
	}
	if (!keepResources)
		_currentRoom->loadResources();

	if (resetCamera)
		g_engine->camera().resetRotationAndScale();
	WalkingCharacter *followTarget = g_engine->camera().followTarget();
	if (followTarget != nullptr)
		g_engine->camera().setFollow(followTarget, true);
	_pressedObject = _selectedObject = nullptr;
}

void Player::changeRoomToBeforeInventory() {
	assert(_roomBeforeInventory != nullptr);
	changeRoom(_roomBeforeInventory->name(), true);
}

MainCharacter *Player::inactiveCharacter() const {
	if (_activeCharacter == nullptr)
		return nullptr;
	return &g_engine->world().getOtherMainCharacterByKind(activeCharacterKind());
}

FakeSemaphore &Player::semaphoreFor(MainCharacterKind kind) {
	static FakeSemaphore dummySemaphore;
	switch (kind) {
	case MainCharacterKind::None: return _semaphore;
	case MainCharacterKind::Mortadelo: return g_engine->world().mortadelo().semaphore();
	case MainCharacterKind::Filemon: return g_engine->world().filemon().semaphore();
	default: assert(false && "Invalid main character kind"); return dummySemaphore;
	}
}

void Player::triggerObject(ObjectBase *object, const char *action) {
	assert(object != nullptr && action != nullptr);
	if (_activeCharacter->isBusy() || _activeCharacter->currentlyUsing() != nullptr)
		return;
	debug("Trigger object %s %s with %s", object->typeName(), object->name().c_str(), action);

	if (strcmp(action, "MIRAR") == 0 || inactiveCharacter()->currentlyUsing() == object) {
		action = "MIRAR";
		_activeCharacter->currentlyUsing() = nullptr;
	}
	else
		_activeCharacter->currentlyUsing() = object;

	auto &script = g_engine->script();
	if (script.createProcess(activeCharacterKind(), object->name(), action, ScriptFlags::AllowMissing) != nullptr)
		return;
	else if (scumm_stricmp(action, "MIRAR") == 0)
		script.createProcess(activeCharacterKind(), "DefectoMirar");
	//else if (action[0] == 'i' && object->name()[0] == 'i')
	// This case can happen if you combine two objects without procedure, the original engine
	// would attempt to start the procedure "DefectoObjeto" which does not exist
	else
		script.createProcess(activeCharacterKind(), "DefectoUsar");
}

struct DoorTask : public Task {
	DoorTask(Process &process, const Door *door, FakeLock &&lock)
		: Task(process)
		, _lock(move(lock))
		, _sourceDoor(door)
		, _character(g_engine->player().activeCharacter())
		, _player(g_engine->player()) {
		_targetRoom = g_engine->world().getRoomByName(door->targetRoom());
		if (_targetRoom == nullptr)
			error("Invalid door target room: %s", door->targetRoom().c_str());

		_targetDoor = dynamic_cast<Door *>(_targetRoom->getObjectByName(door->targetObject()));
		if (_targetDoor == nullptr)
			error("Invalid door target door: %s", door->targetObject().c_str());

		process.name() = String::format("Door to %s %s", _targetRoom->name().c_str(), _targetDoor->name().c_str());
	}

	virtual TaskReturn run() {
		TASK_BEGIN;
		// TODO: Fade out music on room change
		// TODO: Fade out/in on room change instead of delay
		TASK_WAIT(fade(process(), FadeType::ToBlack, 0, 1, 500, EasingType::Out, -5));
		_player.changeRoom(_targetRoom->name(), true);

		if (_targetRoom->fixedCameraOnEntering())
			g_engine->camera().setPosition(as2D(_targetDoor->interactionPoint()));
		else {
			_character->room() = _targetRoom;
			_character->setPosition(_targetDoor->interactionPoint());
			_character->stopWalking(_targetDoor->characterDirection());
			g_engine->camera().setFollow(_character, true);
		}

		// TODO: Start music on room change
		if (g_engine->script().createProcess(_character->kind(), "ENTRAR_" + _targetRoom->name(), ScriptFlags::AllowMissing))
			TASK_YIELD;
		else
			TASK_WAIT(fade(process(), FadeType::ToBlack, 1, 0, 500, EasingType::Out, -5));
		TASK_END;
	}

	virtual void debugPrint() {
		g_engine->console().debugPrintf("%s\n", process().name().c_str());
	}

private:
	FakeLock _lock;
	const Door *_sourceDoor, *_targetDoor;
	Room *_targetRoom;
	MainCharacter *_character;
	Player &_player;
};

void Player::triggerDoor(const Door *door) {
	_heldItem = nullptr;
	
	FakeLock lock(_activeCharacter->semaphore());
	g_engine->scheduler().createProcess<DoorTask>(activeCharacterKind(), door, move(lock));
}

void Player::setPermanentFade(bool isFaded) {
	_isPermanentFaded = isFaded;
}

}
