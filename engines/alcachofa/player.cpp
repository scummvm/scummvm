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

#include "alcachofa/player.h"
#include "alcachofa/script.h"
#include "alcachofa/alcachofa.h"
#include "alcachofa/menu.h"

using namespace Common;

namespace Alcachofa {

Player::Player()
	: _activeCharacter(&g_engine->world().mortadelo())
	, _semaphore("player") {
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

void Player::resetCursor() {
	_cursorFrameI = 0;
}

void Player::updateCursor() {
	if (g_engine->menu().isOpen() || !_isGameLoaded)
		_cursorFrameI = 0;
	else if (_selectedObject == nullptr)
		_cursorFrameI = !g_engine->input().isMouseLeftDown() || _pressedObject != nullptr ? 6 : 7;
	else {
		auto type = _selectedObject->cursorType();
		switch (type) {
		case CursorType::LeaveUp:
			_cursorFrameI = 8;
			break;
		case CursorType::LeaveRight:
			_cursorFrameI = 10;
			break;
		case CursorType::LeaveDown:
			_cursorFrameI = 12;
			break;
		case CursorType::LeaveLeft:
			_cursorFrameI = 14;
			break;
		case CursorType::WalkTo:
			_cursorFrameI = 6;
			break;
		case CursorType::Point:
		default:
			_cursorFrameI = 0;
			break;
		}

		if (_cursorFrameI != 0) {
			if (g_engine->input().isAnyMouseDown() && _pressedObject == _selectedObject)
				_cursorFrameI++;
		} else if (g_engine->input().isMouseLeftDown())
			_cursorFrameI = 2;
		else if (g_engine->input().isMouseRightDown())
			_cursorFrameI = 4;
	}
}

void Player::drawCursor(bool forceDefaultCursor) {
	Point cursorPos = g_engine->input().mousePos2D();
	if (_heldItem == nullptr || forceDefaultCursor) {
		if (forceDefaultCursor)
			_cursorFrameI = 0;
		g_engine->drawQueue().add<AnimationDrawRequest>(_cursorAnimation.get(), _cursorFrameI, as2D(cursorPos), -10);
	} else {
		auto itemGraphic = _heldItem->graphic();
		assert(itemGraphic != nullptr);
		auto &animation = itemGraphic->animation();
		auto frameOffset = animation.totalFrameOffset(0);
		auto imageSize = animation.imageSize(animation.imageIndex(0, 0));
		cursorPos -= frameOffset + imageSize / 2;
		g_engine->drawQueue().add<AnimationDrawRequest>(&animation, 0, as2D(cursorPos), -kForegroundOrderCount);
	}
}

void Player::changeRoom(const Common::String &targetRoomName, bool resetCamera, bool isTemporary) {
	debugC(1, kDebugGameplay, "Change room to %s", targetRoomName.c_str());

	// original would be to always free all resources from globalRoom, inventory, GlobalUI
	// We don't do that, it is unnecessary, all resources would be loaded right after
	// Instead we just keep resources loaded for all global rooms and during inventory/room transitions

	if (targetRoomName.equalsIgnoreCase("SALIR")) {
		_currentRoom = nullptr;
		return; // exiting game entirely
	}

	if (_currentRoom != nullptr) {
		g_engine->scheduler().killProcessByName("ACTUALIZAR_" + _currentRoom->name());

		bool keepResources =
			_currentRoom->name().equalsIgnoreCase(targetRoomName) ||
			_currentRoom->name().equalsIgnoreCase("inventario");
		if (targetRoomName.equalsIgnoreCase("inventario")) {
			keepResources = true;
			if (!_isInTemporaryRoom)
				_roomBeforeInventory = _currentRoom;
		}
		if (!keepResources)
			_currentRoom->freeResources();
	}

	// this fixes a bug with all original games where changing the room in the inventory (e.g. iFOTO in aventura de cine)
	// would overwrite the actual game room thus returning from the inventory one would be stuck in the temporary room
	// If we know that a transition is temporary we prevent that and only remember the real game room
	_isInTemporaryRoom = isTemporary;

	_currentRoom = g_engine->world().getRoomByName(targetRoomName.c_str());
	if (_currentRoom == nullptr) // no good way to recover, leaving-the-room actions might already prevent further progress
		error("Invalid room name: %s", targetRoomName.c_str());

	if (!_didLoadGlobalRooms) {
		_didLoadGlobalRooms = true;
		g_engine->world().inventory().loadResources();
		g_engine->world().globalRoom().loadResources();
	}
	_currentRoom->loadResources(); // if we kept resources we loop over a couple noops, that is fine.

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
	static FakeSemaphore dummySemaphore("dummy");
	switch (kind) {
	case MainCharacterKind::None:
		return _semaphore;
	case MainCharacterKind::Mortadelo:
		return g_engine->world().mortadelo().semaphore();
	case MainCharacterKind::Filemon:
		return g_engine->world().filemon().semaphore();
	default:
		assert(false && "Invalid main character kind");
		return dummySemaphore;
	}
}

void Player::triggerObject(ObjectBase *object, const char *action) {
	assert(object != nullptr && action != nullptr);
	if (_activeCharacter->isBusy() || _activeCharacter->currentlyUsing() != nullptr)
		return;
	debugC(1, kDebugGameplay, "Trigger object %s %s with %s", object->typeName(), object->name().c_str(), action);

	if (strcmp(action, "MIRAR") == 0 || inactiveCharacter()->currentlyUsing() == object) {
		action = "MIRAR";
		_activeCharacter->currentlyUsing() = nullptr;
	} else
		_activeCharacter->currentlyUsing() = object;

	auto &script = g_engine->script();
	if (script.createProcess(activeCharacterKind(), object->name(), action, ScriptFlags::AllowMissing) != nullptr)
		return;

	_activeCharacter->currentlyUsing() = nullptr;
	if (scumm_stricmp(action, "MIRAR") == 0)
		script.createProcess(activeCharacterKind(), "DefectoMirar");
	//else if (action[0] == 'i' && object->name()[0] == 'i')
	// This case can happen if you combine two objects without procedure, the original engine
	// would attempt to start the procedure "DefectoObjeto" which does not exist
	// (this should be revised when working on further games)
	else
		script.createProcess(activeCharacterKind(), "DefectoUsar");
}

struct DoorTask final : public Task {
	DoorTask(Process &process, const Door *door, FakeLock &&lock)
		: Task(process)
		, _lock(move(lock))
		, _sourceDoor(door)
		, _character(g_engine->player().activeCharacter())
		, _player(g_engine->player())
		, _targetObject(nullptr)
		, _targetDirection(Direction::Invalid) {
		findTarget();
		process.name() = String::format("Door to %s %s",
			_targetRoom == nullptr ? "<null>" : _targetRoom->name().c_str(),
			_targetObject == nullptr ? "<null>" : _targetObject->name().c_str());
	}

	DoorTask(Process &process, Serializer &s)
		: Task(process)
		, _player(g_engine->player()) {
		DoorTask::syncGame(s);
	}

	TaskReturn run() override {
		TASK_BEGIN;
		if (_targetRoom == nullptr || _targetObject == nullptr)
			return TaskReturn::finish(1);

		_musicLock = FakeLock("door-music", g_engine->sounds().musicSemaphore());
		if (g_engine->sounds().musicID() != _targetRoom->musicID())
			g_engine->sounds().fadeMusic();
		TASK_WAIT(1, fade(process(), FadeType::ToBlack, 0, 1, 500, EasingType::Out, -5));
		_player.changeRoom(_targetRoom->name(), true); //-V779

		if (_targetRoom->fixedCameraOnEntering())
			g_engine->camera().setPosition(as2D(_targetObject->interactionPoint()));
		else {
			_character->room() = _targetRoom;
			_character->setPosition(_targetObject->interactionPoint());
			_character->stopWalking(_targetDirection);
			g_engine->camera().setFollow(_character, true);
		}

		g_engine->sounds().setMusicToRoom(_targetRoom->musicID());
		_musicLock.release();

		if (g_engine->script().createProcess(_character->kind(), "ENTRAR_" + _targetRoom->name(), ScriptFlags::AllowMissing))
			TASK_YIELD(2);
		else
			TASK_WAIT(3, fade(process(), FadeType::ToBlack, 1, 0, 500, EasingType::Out, -5));
		TASK_END;
	}

	void debugPrint() override {
		g_engine->console().debugPrintf("%s\n", process().name().c_str());
	}

	void syncGame(Serializer &s) override {
		assert(s.isSaving() || (_lock.isReleased() && _musicLock.isReleased()));

		Task::syncGame(s);
		syncObjectAsString(s, _sourceDoor);
		syncObjectAsString(s, _character);
		bool hasMusicLock = !_musicLock.isReleased();
		s.syncAsByte(hasMusicLock);
		if (s.isLoading() && hasMusicLock)
			_musicLock = FakeLock("door-music", g_engine->sounds().musicSemaphore());

		_lock = FakeLock("door", _character->semaphore());
		findTarget();
	}

	const char *taskName() const override;

private:
	void findTarget() {
		_targetRoom = g_engine->world().getRoomByName(_sourceDoor->targetRoom().c_str());
		if (_targetRoom == nullptr) {
			g_engine->game().unknownDoorTargetRoom(_sourceDoor->targetRoom());
			return;
		}

		_targetObject = dynamic_cast<InteractableObject *>(_targetRoom->getObjectByName(_sourceDoor->targetObject().c_str()));
		if (_targetObject == nullptr) {
			g_engine->game().unknownDoorTargetDoor(_sourceDoor->targetRoom(), _sourceDoor->targetObject());
			return;
		}

		_targetDirection = _sourceDoor->characterDirection();
	}

	FakeLock _lock, _musicLock;
	const Door *_sourceDoor = nullptr;
	const InteractableObject *_targetObject = nullptr;
	Direction _targetDirection = {};
	Room *_targetRoom = nullptr;
	MainCharacter *_character = nullptr;
	Player &_player;
};
DECLARE_TASK(DoorTask)

void Player::triggerDoor(const Door *door) {
	_heldItem = nullptr;

	if (g_engine->game().shouldTriggerDoor(door)) {
		FakeLock lock("door", _activeCharacter->semaphore());
		g_engine->scheduler().createProcess<DoorTask>(activeCharacterKind(), door, move(lock));
	}
}

// the last dialog character mechanic seems like a hack in the original engine
// all talking characters (see SayText kernel call) are added to a fixed-size
// rolling queue and stopped upon killProcesses

void Player::addLastDialogCharacter(Character *character) {
	auto lastDialogCharactersEnd = _lastDialogCharacters + kMaxLastDialogCharacters;
	if (Common::find(_lastDialogCharacters, lastDialogCharactersEnd, character) != lastDialogCharactersEnd)
		return;
	_lastDialogCharacters[_nextLastDialogCharacter++] = character;
	_nextLastDialogCharacter %= kMaxLastDialogCharacters;
}

void Player::stopLastDialogCharacters() {
	// originally only the isTalking flag is reset, but this seems a bit safer so unless we find a bug
	for (int i = 0; i < kMaxLastDialogCharacters; i++) {
		auto character = _lastDialogCharacters[i];
		if (character != nullptr)
			character->resetTalking();
	}
}

void Player::setActiveCharacter(MainCharacterKind kind) {
	scumm_assert(kind == MainCharacterKind::Mortadelo || kind == MainCharacterKind::Filemon);
	_activeCharacter = &g_engine->world().getMainCharacterByKind(kind);
}

bool Player::isAllowedToOpenMenu() {
	return
		isGameLoaded() &&
		!g_engine->menu().isOpen() &&
		g_engine->sounds().musicSemaphore().isReleased() &&
		!g_engine->script().variable("prohibirESC") &&
		!_isInTemporaryRoom; // we cannot reliably store this state across multiple room changes
}

void Player::syncGame(Serializer &s) {
	auto characterKind = activeCharacterKind();
	syncEnum(s, characterKind);
	switch (characterKind) {
	case MainCharacterKind::None:
		_activeCharacter = nullptr;
		break;
	case MainCharacterKind::Mortadelo:
	case MainCharacterKind::Filemon:
		_activeCharacter = &g_engine->world().getMainCharacterByKind(characterKind);
		break;
	default:
		error("Invalid character kind in savestate: %d", (int)characterKind);
	}

	FakeSemaphore::sync(s, _semaphore);

	String roomName;
	if (s.isSaving()) {
		roomName =
			g_engine->menu().isOpen() ? g_engine->menu().previousRoom()->name() // save from in-game menu
			: _roomBeforeInventory != nullptr ? _roomBeforeInventory->name() // save from ScummVM while in inventory
			: currentRoom()->name(); // save from ScumnmVM global menu or autosave in normal gameplay
	}
	s.syncString(roomName);
	if (s.isLoading()) {
		_selectedObject = nullptr;
		_pressedObject = nullptr;
		_heldItem = nullptr;
		_nextLastDialogCharacter = 0;
		_isGameLoaded = true;
		_roomBeforeInventory = nullptr;
		_isInTemporaryRoom = false;
		fill(_lastDialogCharacters, _lastDialogCharacters + kMaxLastDialogCharacters, nullptr);
		changeRoom(roomName, true);
	}
}

}
