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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/resources/script.h"

#include "asylum/puzzles/puzzles.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/encounters.h"
#include "asylum/resources/object.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/speech.h"

#include "asylum/views/menu.h"
#include "asylum/views/scene.h"

#include "asylum/asylum.h"
#include "asylum/staticres.h"

namespace Asylum {

//////////////////////////////////////////////////////////////////////////
// ActionArea
//////////////////////////////////////////////////////////////////////////
void ActionArea::load(Common::SeekableReadStream *stream) {
	stream->read(&name, sizeof(name));
	id             = stream->readSint32LE();
	field01        = stream->readSint32LE();
	field02        = stream->readSint32LE();
	field_40       = stream->readSint32LE();
	field_44       = stream->readSint32LE();
	flags          = stream->readSint32LE();
	scriptIndex    = stream->readSint32LE();
	scriptIndex2   = stream->readSint32LE();
	actionType     = stream->readSint32LE();

	for (int32 i = 0; i < ARRAYSIZE(flagNums); i++)
		flagNums[i] = stream->readSint32LE();

	field_7C             = stream->readSint32LE();
	polygonIndex         = stream->readUint32LE();
	soundResourceIdFrame = (ResourceId)stream->readSint32LE();
	field_88             = stream->readSint32LE();
	soundResourceId      = (ResourceId)stream->readSint32LE();
	field_90             = stream->readSint32LE();
	paletteResourceId    = (ResourceId)stream->readSint32LE();

	for (int32 i = 0; i < 5; i++)
		paths[i] = stream->readSint32LE();

	volume = stream->readSint32LE();
}

void ActionArea::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncBytes((byte *)&name, sizeof(name));
	s.syncAsSint32LE(id);
	s.syncAsSint32LE(field01);
	s.syncAsSint32LE(field02);
	s.syncAsSint32LE(field_40);
	s.syncAsSint32LE(field_44);
	s.syncAsSint32LE(flags);
	s.syncAsSint32LE(scriptIndex);
	s.syncAsSint32LE(scriptIndex2);
	s.syncAsSint32LE(actionType);

	for (int32 i = 0; i < ARRAYSIZE(flagNums); i++)
		s.syncAsSint32LE(flagNums[i]);

	s.syncAsSint32LE(field_7C);
	s.syncAsUint32LE(polygonIndex);
	s.syncAsSint32LE(soundResourceIdFrame);
	s.syncAsSint32LE(field_88);
	s.syncAsSint32LE(soundResourceId);
	s.syncAsSint32LE(field_90);
	s.syncAsSint32LE(paletteResourceId);

	for (int32 i = 0; i < 5; i++)
		s.syncAsSint32LE(paths[i]);

	s.syncAsSint32LE(volume);
}

//////////////////////////////////////////////////////////////////////////
// ScriptManager
//////////////////////////////////////////////////////////////////////////
ScriptManager::ScriptManager(AsylumEngine *engine) : _vm(engine) {
	// Build list of opcodes
	ADD_OPCODE(Return);
	ADD_OPCODE(SetGameFlag);
	ADD_OPCODE(ClearGameFlag);
	ADD_OPCODE(ToggleGameFlag);
	ADD_OPCODE(JumpIfGameFlag);
	ADD_OPCODE(HideCursor);
	ADD_OPCODE(ShowCursor);
	ADD_OPCODE(PlayAnimation);
	ADD_OPCODE(MoveScenePosition);
	ADD_OPCODE(HideActor);
	ADD_OPCODE(ShowActor);
	ADD_OPCODE(SetActorPosition);
	ADD_OPCODE(SetSceneMotionStatus);
	ADD_OPCODE(DisableActor);
	ADD_OPCODE(EnableActor);
	ADD_OPCODE(EnableObjects);
	ADD_OPCODE(Return);
	ADD_OPCODE(RemoveObject);
	ADD_OPCODE(JumpActorSpeech);
	ADD_OPCODE(JumpAndSetDirection);
	ADD_OPCODE(JumpIfActorCoordinates);
	ADD_OPCODE(Nop);
	ADD_OPCODE(ResetAnimation);
	ADD_OPCODE(DisableObject);
	ADD_OPCODE(JumpIfSoundPlayingAndPlaySound);
	ADD_OPCODE(JumpIfActionFind);
	ADD_OPCODE(SetActionFind);
	ADD_OPCODE(ClearActionFind);
	ADD_OPCODE(JumpIfActionGrab);
	ADD_OPCODE(SetActionGrab);
	ADD_OPCODE(ClearActionGrab);
	ADD_OPCODE(JumpIfActionTalk);
	ADD_OPCODE(SetActionTalk);
	ADD_OPCODE(ClearActionTalk);
	ADD_OPCODE(AddToInventory);
	ADD_OPCODE(RemoveFromInventory);
	ADD_OPCODE(JumpIfInventoryOmits);
	ADD_OPCODE(RunEncounter);
	ADD_OPCODE(JumpIfAction16);
	ADD_OPCODE(SetAction16);
	ADD_OPCODE(ClearAction16);
	ADD_OPCODE(SelectInventoryItem);
	ADD_OPCODE(JumpIfInventoryItemNotSelected);
	ADD_OPCODE(ChangeScene);
	ADD_OPCODE(Interact);
	ADD_OPCODE(PlayMovie);
	ADD_OPCODE(StopAllObjectsSounds);
	ADD_OPCODE(StopProcessing);
	ADD_OPCODE(ResumeProcessing);
	ADD_OPCODE(ResetSceneRect);
	ADD_OPCODE(ChangeMusicById);
	ADD_OPCODE(StopMusic);
	ADD_OPCODE(IncrementParam1);
	ADD_OPCODE(SetVolume);
	ADD_OPCODE(Jump);
	ADD_OPCODE(RunPuzzle);
	ADD_OPCODE(JumpIfAction8);
	ADD_OPCODE(SetAction8);
	ADD_OPCODE(ClearAction8);
	ADD_OPCODE(CreatePalette);
	ADD_OPCODE(IncrementParam2);
	ADD_OPCODE(WaitUntilFramePlayed);
	ADD_OPCODE(UpdateWideScreen);
	ADD_OPCODE(JumpIfActor);
	ADD_OPCODE(PlaySpeechScene);
	ADD_OPCODE(PlaySpeech);
	ADD_OPCODE(PlaySpeechScene2);
	ADD_OPCODE(MoveScenePositionFromActor);
	ADD_OPCODE(PaletteFade);
	ADD_OPCODE(QueuePaletteFade);
	ADD_OPCODE(PlaySoundUpdateObject);
	ADD_OPCODE(ActorFaceTarget);
	ADD_OPCODE(HideMatteBars);
	ADD_OPCODE(ShowMatteBars);
	ADD_OPCODE(JumpIfSoundPlaying);
	ADD_OPCODE(ChangePlayer);
	ADD_OPCODE(ChangeActorStatus);
	ADD_OPCODE(StopSound);
	ADD_OPCODE(JumpRandom);
	ADD_OPCODE(ClearScreen);
	ADD_OPCODE(Quit);
	ADD_OPCODE(JumpObjectFrame);
	ADD_OPCODE(DeleteGraphics);
	ADD_OPCODE(SetActorField944);
	ADD_OPCODE(SetScriptField1BB0);
	ADD_OPCODE(OnScriptField1BB0);
	ADD_OPCODE(WalkToActor);
	ADD_OPCODE(SetResourcePalette);
	ADD_OPCODE(SetObjectFrameIndexAndFlags);
	ADD_OPCODE(SetObjectFlags);
	ADD_OPCODE(SetActorActionIndex2);
	ADD_OPCODE(UpdateTransparency);
	ADD_OPCODE(QueueScript);
	ADD_OPCODE(ProcessActor);
	ADD_OPCODE(ClearActorFields);
	ADD_OPCODE(SetObjectLastFrameIndex);
	ADD_OPCODE(SetActionAreaFlags);
	ADD_OPCODE(MorphActor);
	ADD_OPCODE(ShowMenu);
	ADD_OPCODE(UpdateGlobalFlags);

	reset();
}

ScriptManager::~ScriptManager() {
	for (int i = 0; i < (int)_opcodes.size(); i++)
		delete _opcodes[i];

	_scripts.clear();
	_queue.reset();
}

void ScriptManager::load(Common::SeekableReadStream *stream) {
	stream->readSint32LE();  // size
	int32 numEntries = stream->readSint32LE();

	for (int32 a = 0; a < numEntries; a++) {
		Script script;

		for (int32 c = 0; c < MAX_ACTION_COMMANDS; c++) {
			ScriptEntry command;

			command.numLines = stream->readSint32LE();
			command.opcode   = (OpcodeType)stream->readSint32LE();
			command.param1   = stream->readSint32LE();
			command.param2   = stream->readSint32LE();
			command.param3   = stream->readSint32LE();
			command.param4   = stream->readSint32LE();
			command.param5   = stream->readSint32LE();
			command.param6   = stream->readSint32LE();
			command.param7   = stream->readSint32LE();
			command.param8   = stream->readSint32LE();
			command.param9   = stream->readSint32LE();

			script.commands[c] = command;

#ifdef DEBUG
			// Output encounter info
			if (command.opcode == kOpcodeRunEncounter)
				debugC(kDebugLevelEncounter, "%d, %d, %d, %d", command.param1, command.param2, command.param3, command.param4);
#endif
		}

		if (_vm->checkGameVersion("Demo")) {
			stream->seek(2 * 4, SEEK_CUR);
		} else {
			script.field_1BAC = stream->readSint32LE();
			script.field_1BB0 = stream->readSint32LE();
			script.counter    = stream->readSint32LE();
		}

		_scripts.push_back(script);
	}

	// Patch for Chapter 2 Lockout bug
	if (_vm->checkGameVersion("Unpatched") && getWorld()->chapter == kChapter2) {
		_scripts[ 3].commands[ 2].param1 = 1506;
		_scripts[34].commands[13].param1 =  453;
		_scripts[43].commands[ 9].param1 =  455;
	}

	// Patch for Demo lockup bug
	if (_vm->checkGameVersion("Demo")) {
		_scripts[1].commands[6].param2 = 151;
		_scripts[1].commands[6].param3 = 332;
	}
}

void ScriptManager::saveLoadWithSerializer(Common::Serializer &s) {
	for (uint i = 0; i < _scripts.size(); i++)
		_scripts[i].saveLoadWithSerializer(s);
}

// Save the script queue (in the original, it is part of the shared data)
void ScriptManager::saveQueue(Common::Serializer &s) {
	_queue.saveLoadWithSerializer(s);
}

void ScriptManager::resetAll() {
	_scripts.clear();
	resetQueue();
	reset();
}

void ScriptManager::reset(uint32 count) {
	// Create a set of empty scripts
	_scripts.clear();
	for (uint32 i = 0; i < count; i++) {
		Script script;
		_scripts.push_back(script);
	}

	_done = false;
	_exit = false;
	_processNextEntry = false;

	_lastProcessedCmd = nullptr;
	_currentScript = nullptr;
	_currentQueueEntry = nullptr;
}

void ScriptManager::resetQueue() {
	_queue.reset();
}

void ScriptManager::queueScript(int32 scriptIndex, ActorIndex actorIndex) {
	// When the skipProcessing flag is set, do not queue any more scripts
	if (getSharedData()->getFlag(kFlagSkipScriptProcessing))
		return;

	// Look for a empty queue slot
	uint32 index;
	for (index = 1; index < ARRAYSIZE(_queue.entries); index++) {
		if (_queue.entries[index].scriptIndex == -1)
			break;
	}

	// No empty queue slot, return
	if (index == 0 || index == ARRAYSIZE(_queue.entries))
		return;

	// Setup script and queue
	_scripts[scriptIndex].counter = 0;
	_queue.entries[index].prev = 0;
	_queue.entries[index].next = 0;

	if (_queue.first) {
		_queue.entries[_queue.last].next = index;
		_queue.entries[index].prev = _queue.last;
	} else {
		_queue.first = index;
	}

	_queue.last = index;

	_queue.entries[index].scriptIndex = scriptIndex;
	_queue.entries[index].currentLine = 0;
	_queue.entries[index].actorIndex = actorIndex;
}

bool ScriptManager::isInQueue(int32 scriptIndex) const {
	for (int32 i = 0; i < ARRAYSIZE(_queue.entries); i++) {
		if (_queue.entries[i].scriptIndex == scriptIndex)
			return true;
	}

	return false;
}

void ScriptManager::removeFromQueue(uint32 entryIndex) {
	if (_queue.first == _queue.last) {
		_queue.last = 0;
		_queue.first = 0;
		_queue.entries[entryIndex].scriptIndex = -1;
	} else if (_queue.first == entryIndex) {
		_queue.first = _queue.entries[entryIndex].next;
		_queue.entries[_queue.first].prev = 0;
		_queue.entries[entryIndex].scriptIndex = -1;
	} else if (_queue.last == entryIndex) {
		_queue.last = _queue.entries[entryIndex].prev;
		_queue.entries[_queue.last].next = 0;
		_queue.entries[entryIndex].scriptIndex = -1;
	} else {
		_queue.entries[_queue.entries[entryIndex].prev].next = _queue.entries[entryIndex].next;
		_queue.entries[_queue.entries[entryIndex].next].prev = _queue.entries[entryIndex].prev;
		_queue.entries[entryIndex].scriptIndex = -1;
	}
}

bool ScriptManager::process() {
	_exit = false;

	_vm->setGameFlag(kGameFlagScriptProcessing);

	// Setup queue entry
	if (_queue.first) {
		uint32 entryIndex  = _queue.first;
		uint32 nextIndex   = _queue.entries[entryIndex].next;
		int32  scriptIndex = _queue.entries[entryIndex].scriptIndex;

		if (scriptIndex != -1) {

			// Setup script
			for (;;) {
				_processNextEntry = false;
				_done             = false;
				_currentScript    = &_scripts[scriptIndex];

				// Run script
				for (;;) {
					ScriptEntry *cmd = nullptr;
					uint32 cmdIndex = 0;

					if (_processNextEntry)
						goto label_processNextEntry;

					// Get the script command
					cmdIndex = (uint32)_queue.entries[entryIndex].currentLine;
					if (cmdIndex >= MAX_ACTION_COMMANDS)
						error("[ScriptManager::process] Invalid command index (was: %d, max: %d)", cmdIndex, MAX_ACTION_COMMANDS);

					cmd = &_currentScript->commands[cmdIndex];

					// Check script opcode
					if (cmd->opcode >= (int32)_opcodes.size())
						error("[ScriptManager::process] Invalid opcode index (was: %d, max: %d)", cmd->opcode, _opcodes.size() - 1);

					if (_lastProcessedCmd != cmd)
						debugC(kDebugLevelCommands, "[Script idx: %d] %2d: %s (%d, %d, %d, %d, %d, %d, %d, %d, %d)",
							scriptIndex, cmdIndex, _opcodes[cmd->opcode]->name,
							cmd->param1, cmd->param2, cmd->param3, cmd->param4, cmd->param5,
							cmd->param6, cmd->param7, cmd->param8, cmd->param9);

					_lastProcessedCmd = cmd; // DEBUGGING

					// Execute opcode
					_currentQueueEntry = &_queue.entries[entryIndex];
					(*_opcodes[cmd->opcode]->func)(cmd);

					// When we are asked to exit, return directly without setting the processing flag
					if (_exit)
						return true;

					if (!_processNextEntry)
						++_queue.entries[entryIndex].currentLine;

					if (_done)
						removeFromQueue(entryIndex);

					if (!_done)
						continue;

label_processNextEntry:
					break;
				}

				entryIndex = nextIndex;

				if (!nextIndex)
					goto label_exit_processing;

				scriptIndex = _queue.entries[nextIndex].scriptIndex;
				nextIndex = _queue.entries[nextIndex].next;

				if (scriptIndex == -1)
						goto label_exit_processing;
			}
		}
	}

label_exit_processing:
	_vm->clearGameFlag(kGameFlagScriptProcessing);

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Opcode Functions
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Opcode 0x00
IMPLEMENT_OPCODE(Return)
	_done          = true;
	_processNextEntry = false;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x01
IMPLEMENT_OPCODE(SetGameFlag)
	int32 flagNum = cmd->param1;

	if (flagNum >= 0)
		_vm->setGameFlag((GameFlag)flagNum);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x02
IMPLEMENT_OPCODE(ClearGameFlag)
	int32 flagNum = cmd->param1;

	if (flagNum >= 0)
		_vm->clearGameFlag((GameFlag)flagNum);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x03
IMPLEMENT_OPCODE(ToggleGameFlag)
	int32 flagNum = cmd->param1;

	if (flagNum >= 0)
		_vm->toggleGameFlag((GameFlag)flagNum);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x04
IMPLEMENT_OPCODE(JumpIfGameFlag)
	if (cmd->param1 < 0)
		return;

	bool doJump = (cmd->param2) ? _vm->isGameFlagNotSet((GameFlag)cmd->param1) : _vm->isGameFlagSet((GameFlag)cmd->param1);
	if (!doJump)
		return;

	setNextLine(cmd->param3);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x05
IMPLEMENT_OPCODE(HideCursor)
	getCursor()->hide();
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x06
IMPLEMENT_OPCODE(ShowCursor)
	getCursor()->show();

	getSharedData()->setFlag(kFlag1, false);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x07
IMPLEMENT_OPCODE(PlayAnimation)
	Object *object = getWorld()->getObjectById((ObjectId)cmd->param1);
	if (!object)
		error("[ScriptManager::opcodePlayAnimation] Cannot find specified object (id: %d)", cmd->param1);

	if (cmd->param2 == 2) {
		if (object->checkFlags()) {
			_processNextEntry = true;
		} else
			cmd->param2 = 1;

		return;
	}

	// Update flags
	if (cmd->param4) {
		object->flags &= ~kObjectFlag10E38;
		object->flags |= kObjectFlag20;
	} else if (cmd->param3) {
		object->flags &= ~kObjectFlag10E38;
		object->flags |= kObjectFlag10000;
	} else if (object->flags & kObjectFlag10000) {
		object->flags |= kObjectFlag8;
		object->flags &= ~kObjectFlag10000;
	} else if (!(object->flags & kObjectFlag10E38)) {
		object->flags |= kObjectFlag8;
	}

	object->setNextFrame(object->flags);

	if (object->getField688() == 1) {
		if (object->flags & kObjectFlag4) {
			getSharedData()->setGlobalPoint(Common::Point(object->x, object->y));
		} else {
			Common::Rect frameRect = GraphicResource::getFrameRect(_vm, object->getResourceId(), object->getFrameIndex());

			getSharedData()->setGlobalPoint(Common::Point(frameRect.left + (int16)Common::Rational(frameRect.width(), 2).toInt()  + object->x,
			                                              frameRect.top  + (int16)Common::Rational(frameRect.height(), 2).toInt() + object->y));
		}
	}

	if (cmd->param2) {
		cmd->param2 = 2;
		_processNextEntry = true;
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x08
IMPLEMENT_OPCODE(MoveScenePosition)
	if (cmd->param3 < 1) {
		getWorld()->xLeft = (int16)cmd->param1;
		getWorld()->yTop  = (int16)cmd->param2;
		getWorld()->motionStatus = 3;

	} else if (!cmd->param4) {
		getWorld()->motionStatus = 5;

		getScene()->updateSceneCoordinates(cmd->param1,
			                               cmd->param2,
		                                   cmd->param3);

	} else if (cmd->param5) {
		if (getWorld()->motionStatus == 2)
			_processNextEntry = true;
		else
			cmd->param5 = 0;
	} else {
		cmd->param5 = 1;
		getWorld()->motionStatus = 2;

		getScene()->updateSceneCoordinates(cmd->param1,
		                                   cmd->param2,
		                                   cmd->param3,
		                                   true);

		_processNextEntry = true;
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x09
IMPLEMENT_OPCODE(HideActor)
	Actor *actor = getScene()->getActor(cmd->param1);

	actor->hide();
	actor->updateReflectionData();
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x0A
IMPLEMENT_OPCODE(ShowActor)
	Actor *actor = getScene()->getActor(cmd->param1);

	actor->show();
	actor->updateReflectionData();
	actor->setLastScreenUpdate(_vm->getTick());
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x0B
IMPLEMENT_OPCODE(SetActorPosition)
	Actor *actor = getScene()->getActor(cmd->param1);

	actor->setPosition((int16)cmd->param2, (int16)cmd->param3, (ActorDirection)cmd->param4, (uint32)cmd->param5);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x0C
IMPLEMENT_OPCODE(SetSceneMotionStatus)
	getWorld()->motionStatus = cmd->param1;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x0D
IMPLEMENT_OPCODE(DisableActor)
	Actor *actor = getScene()->getActor(cmd->param1);

	actor->changeStatus(kActorStatusDisabled);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x0E
IMPLEMENT_OPCODE(EnableActor)
	Actor *actor = getScene()->getActor(cmd->param1);

	if (actor->getStatus() == kActorStatusDisabled)
		actor->enable();
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x0F
IMPLEMENT_OPCODE(EnableObjects)
	Object *object = getWorld()->getObjectById((ObjectId)cmd->param1);
	if (!object)
		error("[ScriptManager::opcodeEnableObjects] Cannot find specified object (id: %d)", cmd->param1);

	if (!_currentScript->counter && getWorld()->chapter != 13)
		_vm->sound()->playSound(cmd->param3 ? MAKE_RESOURCE(kResourcePackSound, 6) : MAKE_RESOURCE(kResourcePackSound, 1));

	if (_currentScript->counter >= (3 * cmd->param2 - 1)) {
		_currentScript->counter = 0;
		object->setTransparency(0);
		enableObject(cmd, kObjectTransparencyOpaque);
	} else {
		++_currentScript->counter;
		if (cmd->param3) {
			object->setTransparency(3 - _currentScript->counter / cmd->param2);
			enableObject(cmd, kObjectEnableType1);
		} else {
			object->setTransparency(_currentScript->counter / cmd->param2 + 1);
			enableObject(cmd, kObjectEnableType0);
		}

		_processNextEntry = true;
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x10 : Identical to opcode 0x00

//////////////////////////////////////////////////////////////////////////
// Opcode 0x11
IMPLEMENT_OPCODE(RemoveObject)
	if (!cmd->param1)
		return;

	Object *object = getWorld()->getObjectById((ObjectId)cmd->param1);
	if (!object)
		error("[ScriptManager::opcodeRemoveObject] Cannot find specified object (id: %d)", cmd->param1);

	object->disableAndRemoveFromQueue();
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x12
IMPLEMENT_OPCODE(JumpActorSpeech)
	Actor *actor = getScene()->getActor(cmd->param1);

	// Actor goes to position (param2, param3)
	if (actor->canReach(Common::Point((int16)cmd->param2, (int16)cmd->param3)))
		return;

	_currentQueueEntry->currentLine = cmd->param4;

	if (cmd->param5)
		getSpeech()->playIndexed(1);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x13
IMPLEMENT_OPCODE(JumpAndSetDirection)
	Actor *actor = getScene()->getActor(cmd->param1);
	ActorDirection newDirection = (ActorDirection)(cmd->param4 & 7);

	if (actor->getStatus() != kActorStatusWalkingTo && actor->getStatus() != kActorStatusWalkingTo2) {
		if (cmd->param5 != 2) {

			if (cmd->param2 == -1 || cmd->param3 == -1) {
				actor->changeDirection(newDirection);
			} else if ((actor->getPoint1()->x + actor->getPoint2()->x) == cmd->param2 && (actor->getPoint1()->y + actor->getPoint2()->y) == cmd->param3) {
				actor->changeDirection(newDirection);
			} else {
				actor->forceTo((int16)cmd->param2, (int16)cmd->param3, (bool)cmd->param6);

				if (cmd->param5 == 1) {
					cmd->param5 = 2;
					_processNextEntry = true;
				}
			}
		} else {
			cmd->param5 = 1;
			_processNextEntry = false;

			if ((actor->getPoint1()->x + actor->getPoint2()->x) == cmd->param2 && (actor->getPoint1()->y + actor->getPoint2()->y) == cmd->param3)
				actor->changeDirection(newDirection);
		}
	} else {
		if (cmd->param5 == 2)
			_processNextEntry = true;
	}
	
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x14
IMPLEMENT_OPCODE(JumpIfActorCoordinates)
	Actor *actor = getScene()->getActor(cmd->param1);

	if ((actor->getPoint1()->x + actor->getPoint2()->x) != cmd->param2 ||
		(actor->getPoint1()->y + actor->getPoint2()->y) != cmd->param3)
		_currentQueueEntry->currentLine = cmd->param4;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x15
IMPLEMENT_OPCODE(Nop)
	// Nothing to do
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x16
IMPLEMENT_OPCODE(ResetAnimation)
	Object *object = getWorld()->getObjectById((ObjectId)cmd->param1);
	if (!object)
		error("[ScriptManager::opcodeResetAnimation] Cannot find specified object (id: %d)", cmd->param1);

	if (object->flags & kObjectFlag10000)
		object->setFrameIndex(object->getFrameCount() - 1);
	else
		object->setFrameIndex(0);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x17
IMPLEMENT_OPCODE(DisableObject)
	Object *object = getWorld()->getObjectById((ObjectId)cmd->param1);
	if (!object)
		error("[ScriptManager::opcodeDisableObject] Cannot find specified object (id: %d)", cmd->param1);

	object->disable();
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x18
IMPLEMENT_OPCODE(JumpIfSoundPlayingAndPlaySound)
	ResourceId resource = (ResourceId)cmd->param1;

	if (cmd->param2 == 2) {
		if (getSound()->isPlaying(resource))
			_processNextEntry = true;
		else
			cmd->param2 = 1;
	} else if (!_vm->sound()->isPlaying(resource)) {
		int32 vol = getSound()->getAdjustedVolume(abs(Config.sfxVolume));
		getSound()->playSound(resource, (bool)cmd->param4, -((abs(cmd->param3) + vol) * (abs(cmd->param3) + vol)), 0);

		if (cmd->param2 == 1) {
			cmd->param2 = 2;
			_processNextEntry = true;
		}
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x19
IMPLEMENT_OPCODE(JumpIfActionFind)
	jumpIfActionFlag(cmd, kActionTypeFind);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x1A
IMPLEMENT_OPCODE(SetActionFind)
	setActionFlag(cmd, kActionTypeFind);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x1B
IMPLEMENT_OPCODE(ClearActionFind)
	clearActionFlag(cmd, kActionTypeFind);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x1C
IMPLEMENT_OPCODE(JumpIfActionGrab)
	jumpIfActionFlag(cmd, kActionTypeGrab);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x1D
IMPLEMENT_OPCODE(SetActionGrab)
	setActionFlag(cmd, kActionTypeGrab);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x1E
IMPLEMENT_OPCODE(ClearActionGrab)
	clearActionFlag(cmd, kActionTypeGrab);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x1F
IMPLEMENT_OPCODE(JumpIfActionTalk)
	jumpIfActionFlag(cmd, kActionTypeTalk);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x20
IMPLEMENT_OPCODE(SetActionTalk)
	setActionFlag(cmd, kActionTypeTalk);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x21
IMPLEMENT_OPCODE(ClearActionTalk)
	clearActionFlag(cmd, kActionTypeTalk);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x22
IMPLEMENT_OPCODE(AddToInventory)
	Actor *actor = getScene()->getActor(cmd->param3 ? cmd->param3 : _currentQueueEntry->actorIndex);

	actor->inventory.add(cmd->param1, cmd->param2);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x23
IMPLEMENT_OPCODE(RemoveFromInventory)
	Actor *actor = getScene()->getActor(cmd->param3 ? cmd->param3 : _currentQueueEntry->actorIndex);

	actor->inventory.remove(cmd->param1, cmd->param2);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x24
IMPLEMENT_OPCODE(JumpIfInventoryOmits)
	Actor *actor = getScene()->getActor(cmd->param4 ? cmd->param4 : _currentQueueEntry->actorIndex);

	if (!actor->inventory.contains(cmd->param1, cmd->param3))
		_currentQueueEntry->currentLine = cmd->param2;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x25
IMPLEMENT_OPCODE(RunEncounter)
	Encounter *encounter = _vm->encounter();

	getSharedData()->setFlag(kFlagEncounterDisablePlayerOnExit, (bool)cmd->param5);

	if (cmd->param6) {
		if (getSharedData()->getFlag(kFlagIsEncounterRunning))
			_processNextEntry = true;
		else
			cmd->param6 = 0;
	} else {
		encounter->run(cmd->param1, (ObjectId)cmd->param2, (ObjectId)cmd->param3, cmd->param4);

		cmd->param6 = 2;
		_processNextEntry = true;
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x26
IMPLEMENT_OPCODE(JumpIfAction16)
	jumpIfActionFlag(cmd, kActionType16);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x27
IMPLEMENT_OPCODE(SetAction16)
	setActionFlag(cmd, kActionType16);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x28
IMPLEMENT_OPCODE(ClearAction16)
	clearActionFlag(cmd, kActionType16);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x29
IMPLEMENT_OPCODE(SelectInventoryItem)
	Actor *actor = getScene()->getActor(cmd->param1);

	actor->inventory.selectItem(cmd->param2);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x2A
IMPLEMENT_OPCODE(JumpIfInventoryItemNotSelected)
	Actor *actor = getScene()->getActor(cmd->param1);

	if ((int32)actor->inventory.getSelectedItem() != cmd->param2)
		_currentQueueEntry->currentLine = cmd->param3;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x2B
IMPLEMENT_OPCODE(ChangeScene)
	if (_vm->isAltDemo()) {
		Engine::quitGame();
		_done = true;
		return;
	}

	uint32 tick = _vm->getTick();
	getScene()->getActor(0)->changeStatus(kActorStatusDisabled);
	resetQueue();

	// Fade screen to black
	getScreen()->paletteFade(0, 75, 8);
	getScreen()->clear();

	// Stop all sounds & music
	_vm->setTick(tick);
	getSound()->stopAll();
	getSound()->stopMusic();

	_vm->unlockAchievement(Common::String::format("ASYLUM_LEVEL_%d", getWorld()->chapter));

	// Switch the scene
	_vm->switchScene((ResourcePackId)(cmd->param1 + 4));

	_exit = true;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x2C
IMPLEMENT_OPCODE(Interact)
	Actor *player = getScene()->getActor();
	Actor *actor = getScene()->getActor(_currentQueueEntry->actorIndex);
	Common::Point playerPoint((int16)(player->getPoint1()->x + player->getPoint2()->x), (int16)(player->getPoint1()->y + player->getPoint2()->y));
	ActorDirection direction = (cmd->param2 == 8) ? player->getDirection() : (ActorDirection)cmd->param2;

	ActorDirection newDirection = (ActorDirection)((player->getDirection() + 4) % 8);

	if (cmd->param2 == 8)
		cmd->param2 = player->getDirection();

	if (cmd->param3 == 2) {
		switch (actor->getStatus()) {
		default:
			_processNextEntry = true;
			return;

		case kActorStatusEnabled:
		case kActorStatusEnabled2:
			return;

		case kActorStatusStoppedInteracting:
			actor->enable();
			break;

		case kActorStatusGettingHurt:
			// We want to continue processing and not go into the default case
			break;

		case kActorStatusStoppedHitting:
			actor->changeStatus(kActorStatusEnabled2);
		}

		cmd->param3 = 0;

		if (cmd->param1 == 2) {
			Common::Point point(playerPoint);

			if (player->canMove(&point, newDirection, 3, false)) {

				point.x += (int16)(3 * deltaPointsArray[newDirection][0]);
				point.y += (int16)(3 * deltaPointsArray[newDirection][1]);

				player->setPosition(point.x, point.y, actor->getDirection(), 0);
			}
		}

	} else if (cmd->param1 != 2 || player->canMove(&playerPoint, newDirection, 3, false)) {
		ResourceId id = kResourceNone;
		if (direction >= 5)
			id = actor->getResourcesId((uint32)(5 * cmd->param1 - (direction - 38)));
		else
			id = actor->getResourcesId((uint32)(5 * cmd->param1 + direction + 30));

		actor->setResourceId(id);
		actor->setFrameCount(GraphicResource::getFrameCount(_vm, id));
		actor->setFrameIndex(0);
		actor->setDirection(direction);
		actor->changeStatus(actor->getStatus() <= kActorStatus11 ? kActorStatusInteracting : kActorStatusHittingPumpkin);

		cmd->param3 = 2;
		_processNextEntry = true;
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x2D
IMPLEMENT_OPCODE(PlayMovie)
	if (_vm->checkGameVersion("Demo") && cmd->param1 == 4) {
		Engine::quitGame();
		_done = true;
		return;
	}

	if (getSharedData()->getMatteBarHeight() < 170) {
		_processNextEntry = true;

		if (!getSharedData()->getMatteBarHeight()) {
			getCursor()->hide();
			getScreen()->loadGrayPalette();
			getSharedData()->setMatteVar1(1);
			getSharedData()->setMatteBarHeight(1);
			getSharedData()->setMatteVar2(0);
			getSharedData()->setMattePlaySound(cmd->param3 == 0);
			getSharedData()->setMatteInitialized(cmd->param2 == 0);
			getSharedData()->movieIndex = (uint32)cmd->param1;
		}

		return;
	}

	bool check = false;
	ActionArea *area = getWorld()->actions[getScene()->getActor()->getActionIndex3()];
	if (area->paletteResourceId) {
		getScreen()->setPalette(area->paletteResourceId);
		getScreen()->setGammaLevel(area->paletteResourceId);
	} else {
		getScreen()->setPalette(getWorld()->currentPaletteId);
		getScreen()->setGammaLevel(getWorld()->currentPaletteId);
	}

	getSharedData()->setMatteBarHeight(0);
	_processNextEntry = false;

	if (!getSharedData()->getMattePlaySound() && _currentScript->commands[0].numLines != 0) {
		bool found = true;
		int index = 0;


		while (_currentScript->commands[index].opcode != 0x2B) { // ChangeScene
			++index;

			if (index >= _currentScript->commands[0].numLines) {
				found = false;
				break;
			}
		}

		if (found)
			check = true;
	}

	if (!check && getSharedData()->getMatteVar2() == 0 && getWorld()->musicCurrentResourceIndex != kMusicStopped)
		_vm->sound()->playMusic(MAKE_RESOURCE(kResourcePackMusic, getWorld()->musicCurrentResourceIndex));

	getCursor()->show();
	getSharedData()->setMatteVar2(0);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x2E
IMPLEMENT_OPCODE(StopAllObjectsSounds)
	Object *object = getWorld()->getObjectById((ObjectId)cmd->param1);

	object->stopAllSounds();
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x2F
IMPLEMENT_OPCODE(StopProcessing)
	getSharedData()->setFlag(kFlagSkipScriptProcessing, true);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x30
IMPLEMENT_OPCODE(ResumeProcessing)
	getSharedData()->setFlag(kFlagSkipScriptProcessing, false);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x31
IMPLEMENT_OPCODE(ResetSceneRect)
	getWorld()->sceneRectIdx = (uint8)LOBYTE(cmd->param1);
	getScreen()->paletteFade(0, 25, 10);
	getSharedData()->setFlag(kFlagScene1, true);

	getWorld()->xLeft = getWorld()->sceneRects[getWorld()->sceneRectIdx].left;
	getWorld()->yTop  = getWorld()->sceneRects[getWorld()->sceneRectIdx].top;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x32
IMPLEMENT_OPCODE(ChangeMusicById)
	_vm->sound()->changeMusic((ResourceId)cmd->param1, cmd->param2 ? 2 : 1);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x33
IMPLEMENT_OPCODE(StopMusic)
	_vm->sound()->changeMusic(kMusicStopped, 0);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x34
IMPLEMENT_OPCODE(IncrementParam1)
	if (cmd->param1 >= 2) {
		cmd->param1 = 0;
	} else {
		cmd->param1++;
		_processNextEntry = true;
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x35
IMPLEMENT_OPCODE(SetVolume)
	AmbientSoundItem item = getWorld()->ambientSounds[cmd->param1];
	int var = cmd->param2 + item.delta;

	int32 volume = -((Config.sfxVolume + var) * (Config.ambientVolume + var));

	if (volume < 0) {
		if (volume < -10000)
			volume = -10000;
	} else {
		volume = 0;
	}

	_vm->sound()->setVolume(item.resourceId, volume);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x36
IMPLEMENT_OPCODE(Jump)
	_currentQueueEntry->currentLine = cmd->param1 - 1;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x37
IMPLEMENT_OPCODE(RunPuzzle)
	getScreen()->clear();
	getScreen()->clearGraphicsInQueue();

	_vm->switchEventHandler(getPuzzles()->getPuzzle((uint32)cmd->param1));

	_currentQueueEntry->currentLine++;

	_exit = true;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x38
IMPLEMENT_OPCODE(JumpIfAction8)
	jumpIfActionFlag(cmd, kActionType8);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x39
IMPLEMENT_OPCODE(SetAction8)
	setActionFlag(cmd, kActionType8);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x3A
IMPLEMENT_OPCODE(ClearAction8)
	clearActionFlag(cmd, kActionType8);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x3B
IMPLEMENT_OPCODE(CreatePalette)
	if (!cmd->param2) {
		getScreen()->loadGrayPalette();
		cmd->param2 = 1;
	}

	if (cmd->param1 >= 22) {
		getScreen()->clear();

		cmd->param1 = 0;
		cmd->param2 = 0;
		_currentQueueEntry->currentLine++;

		getScreen()->clearGraphicsInQueue();

		_exit = true;
		return;
	}

	getScreen()->updatePalette(cmd->param1);

	_processNextEntry = true;
	++cmd->param1;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x3C
IMPLEMENT_OPCODE(IncrementParam2)
	if (cmd->param1) {
		if (cmd->param2 >= cmd->param1) {
			cmd->param2 = 0;
		} else {
			++cmd->param2;
			_processNextEntry = true;
		}
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x3D
IMPLEMENT_OPCODE(WaitUntilFramePlayed)
	Object *object = getWorld()->getObjectById((ObjectId)cmd->param1);

	int32 frameNum = cmd->param2;
	if (frameNum == -1)
		frameNum = object->getFrameCount() - 1;

	if ((int32)object->getFrameIndex() != frameNum)
		_processNextEntry = true;
END_OPCODE


//////////////////////////////////////////////////////////////////////////
// Opcode 0x3E
IMPLEMENT_OPCODE(UpdateWideScreen)
	int barSize = cmd->param1;

	if (barSize >= 22) {
		cmd->param1 = 0;
		_processNextEntry = false;

		getSharedData()->setMatteBarHeight(0);
	} else {
		getScreen()->drawWideScreenBars((int16)(4 * barSize));

		_processNextEntry = true;
		++cmd->param1;
	}
END_OPCODE


//////////////////////////////////////////////////////////////////////////
// Opcode 0x3F
IMPLEMENT_OPCODE(JumpIfActor)
	ActorIndex index = (cmd->param1 == kActorInvalid) ? getSharedData()->getPlayerIndex() : cmd->param1;

	if (_currentQueueEntry->actorIndex != index)
		_currentQueueEntry->currentLine = cmd->param2 - 1;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x40
IMPLEMENT_OPCODE(PlaySpeechScene)
	if (cmd->param1 < 0 || cmd->param1 >= 5)
		return;

	if (cmd->param4 != 2) {
		cmd->param5 = (int32)getSpeech()->playIndexed((ResourceId)cmd->param1);

		if (cmd->param2) {
			_vm->setGameFlag(kGameFlagScriptProcessing);
			cmd->param4 = 2;
			if (cmd->param6) {
				getSharedData()->setFlag(kFlag1, true);
				getSharedData()->setFlag(kFlag2, true);
			}
			_processNextEntry = true;
		}

		if (cmd->param3 && !cmd->param6)
			_vm->setGameFlag(kGameFlag219);

		return;
	}

	if (_vm->sound()->isPlaying((ResourceId)cmd->param5)) {
		_processNextEntry = true;
		return;
	}

	_vm->clearGameFlag(kGameFlagScriptProcessing);
	cmd->param4 = 0;

	if (cmd->param3) {
		if (cmd->param6) {
			getSharedData()->setFlag(kFlag1, false);
			getSharedData()->setFlag(kFlag2, false);

			return;
		} else {
			_vm->clearGameFlag(kGameFlag219);
		}
	}

	if (!cmd->param6) {
		cmd->param6 = 1;
	} else {
		getSharedData()->setFlag(kFlag1, false);
		getSharedData()->setFlag(kFlag2, false);
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x41
IMPLEMENT_OPCODE(PlaySpeech)
	if (cmd->param1 < 0)
		return;

	if (cmd->param4 != 2) {
		if (cmd->param1 == 153 && getWorld()->chapter == kChapter2)
			_vm->unlockAchievement("ASYLUM_FIND_CHILDREN");

		cmd->param5 = (int32)getSpeech()->playPlayer((ResourceId)cmd->param1);

		if (cmd->param2) {
			_vm->setGameFlag(kGameFlagScriptProcessing);
			cmd->param4 = 2;
			if (cmd->param6) {
				getSharedData()->setFlag(kFlag1, true);
				getSharedData()->setFlag(kFlag2, true);
			}
			_processNextEntry = true;
		}

		if (cmd->param3 && !cmd->param6)
			_vm->setGameFlag(kGameFlag219);

		return;
	}

	if (_vm->sound()->isPlaying((ResourceId)cmd->param5)) {
		_processNextEntry = true;
		return;
	}

	_vm->clearGameFlag(kGameFlagScriptProcessing);
	cmd->param4 = 0;

	if (cmd->param3) {
		if (cmd->param6) {
			getSharedData()->setFlag(kFlag1, false);
			getSharedData()->setFlag(kFlag2, false);

			return;
		} else {
			_vm->clearGameFlag(kGameFlag219);
		}
	}

	if (!cmd->param6) {
		cmd->param6 = 1;
	} else {
		getSharedData()->setFlag(kFlag1, false);
		getSharedData()->setFlag(kFlag2, false);
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x42
IMPLEMENT_OPCODE(PlaySpeechScene2)
	if (cmd->param1 < 0 || cmd->param1 >= 20 || cmd->param2 < 0)
		return;

	if (cmd->param5 == 2) {
		if (getSound()->isPlaying((ResourceId)cmd->param6)) {
			_processNextEntry = true;
			return;
		}

		_vm->clearGameFlag(kGameFlagScriptProcessing);

		cmd->param5 = 0;

		if (!cmd->param4) {
			if (!cmd->param7) {
				cmd->param7 = 1;
				return;
			}
		}

		if (!cmd->param7) {
			_vm->clearGameFlag(kGameFlag219);
			cmd->param7 = 1;
			return;
		}

		getSharedData()->setFlag(kFlag1, false);
		getSharedData()->setFlag(kFlag2, false);
		return;
	}

	cmd->param6 = (int32)getSpeech()->playScene(cmd->param1, cmd->param2);

	if (cmd->param3) {
		_vm->setGameFlag(kGameFlagScriptProcessing);

		cmd->param5 = 2;

		if (cmd->param7) {
			getSharedData()->setFlag(kFlag1, true);
			getSharedData()->setFlag(kFlag2, true);
		}

		_processNextEntry = true;
	}

	if (cmd->param4 && !cmd->param7)
		_vm->setGameFlag(kGameFlag219);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x43
IMPLEMENT_OPCODE(MoveScenePositionFromActor)
	Actor *actor = getScene()->getActor(cmd->param1);

	if (!cmd->param3) {
		getWorld()->motionStatus = 5;

		getScene()->updateSceneCoordinates(actor->getPoint1()->x + Common::Rational(actor->getPoint2()->x, 2).toInt() - 320,
		                                   actor->getPoint1()->y + Common::Rational(actor->getPoint2()->y, 2).toInt() - 240,
		                                   cmd->param2);
	} else if (cmd->param6) {
		if (getWorld()->motionStatus == 2) {
			_processNextEntry = true;
		} else {
			cmd->param6 = 0;
			getWorld()->coordinates[0] = -1;
		}
	} else {
		cmd->param6 = 1;
		getWorld()->motionStatus = 2;

		if (getScene()->updateSceneCoordinates(actor->getPoint1()->x + Common::Rational(actor->getPoint2()->x, 2).toInt() - 320,
		                                       actor->getPoint1()->y + Common::Rational(actor->getPoint2()->y, 2).toInt() - 240,
		                                       cmd->param2,
		                                       true,
		                                       &cmd->param6))
			_processNextEntry = false;
		else
			_processNextEntry = true;
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x44
IMPLEMENT_OPCODE(PaletteFade)
	getScreen()->paletteFade(0, cmd->param1, cmd->param2);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x45
IMPLEMENT_OPCODE(QueuePaletteFade)
	getScreen()->queuePaletteFade(getWorld()->currentPaletteId, cmd->param1, cmd->param2);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x46
IMPLEMENT_OPCODE(PlaySoundUpdateObject)
	if (cmd->param6) {
		if (getSound()->isPlaying(getSpeech()->getSoundResourceId())) {
			_processNextEntry = true;
		} else {
			cmd->param6 = 0;
			if (cmd->param5) {
				getScene()->getActor(cmd->param5)->enable();
			} else if (cmd->param4 != cmd->param3 && cmd->param4) {
				getWorld()->getObjectById((ObjectId)cmd->param3)->disable();

				Object *object = getWorld()->getObjectById((ObjectId)cmd->param4);
				object->setNextFrame(object->flags);
			}

			_vm->clearGameFlag(kGameFlagScriptProcessing);

			getSpeech()->resetResourceIds();
		}
	} else {
		_vm->setGameFlag(kGameFlagScriptProcessing);
		getSpeech()->play(MAKE_RESOURCE(kResourcePackSpeech, 515 + cmd->param1), MAKE_RESOURCE(kResourcePackText, 1290 + cmd->param1));

		if (cmd->param2) {
			getScene()->getActor(cmd->param5)->changeStatus(kActorStatus8);
			cmd->param6 = 1;
			_processNextEntry = true;
		} else {
			if (cmd->param4 != cmd->param3) {
				if (cmd->param4)
					getWorld()->getObjectById((ObjectId)cmd->param4)->disable();

				if (cmd->param3) {
					Object *object = getWorld()->getObjectById((ObjectId)cmd->param3);
					object->setNextFrame(object->flags);
				}
			}

			cmd->param6 = 1;
			_processNextEntry = true;
		}
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x47
IMPLEMENT_OPCODE(ActorFaceTarget)
	getScene()->getActor(cmd->param1)->faceTarget((uint32)cmd->param2, (DirectionFrom)cmd->param3);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x48
IMPLEMENT_OPCODE(HideMatteBars)
	getSharedData()->setMatteVar1(0);
	getSharedData()->setMatteInitialized(true);

	if (getSharedData()->getMatteBarHeight() >= 170) {
		getSharedData()->setMatteBarHeight(0);
		_processNextEntry = false;
		getCursor()->show();
	} else {
		_processNextEntry = true;

		if (!getSharedData()->getMatteBarHeight()) {
			getCursor()->hide();
			getSharedData()->setMatteBarHeight(1);
		}
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x49
IMPLEMENT_OPCODE(ShowMatteBars)
	getSharedData()->setMatteVar1(0);
	getSharedData()->setMatteInitialized(true);
	getSharedData()->setMattePlaySound(true);

	if (getSharedData()->getMatteBarHeight() >= 170) {
		getSharedData()->setMatteBarHeight(0);
		_processNextEntry = false;
		getCursor()->show();
	} else {
		_processNextEntry = true;

		if (!getSharedData()->getMatteBarHeight()) {
			getCursor()->hide();
			getSharedData()->setMatteBarHeight(90);
		}
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x4A
IMPLEMENT_OPCODE(JumpIfSoundPlaying)
	if (cmd->param3 == 1) {
		if (_vm->sound()->isPlaying((ResourceId)cmd->param1)) {
			_currentQueueEntry->currentLine = cmd->param2;
		}
	} else if (!_vm->sound()->isPlaying((ResourceId)cmd->param1)) {
		_currentQueueEntry->currentLine = cmd->param2;
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x4B
IMPLEMENT_OPCODE(ChangePlayer)
	getScene()->changePlayer(cmd->param1);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x4C
IMPLEMENT_OPCODE(ChangeActorStatus)
	Actor *actor = getScene()->getActor(cmd->param1);

	if (cmd->param2) {
		if (actor->getStatus() < kActorStatus11)
			actor->setStatus(kActorStatusEnabled2);
	} else {
		actor->setStatus(kActorStatusEnabled);
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x4D
IMPLEMENT_OPCODE(StopSound)
	if (_vm->sound()->isPlaying((ResourceId)cmd->param1))
		_vm->sound()->stop((ResourceId)cmd->param1);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x4E
IMPLEMENT_OPCODE(JumpRandom)
	if (_vm->getRandom((uint32)cmd->param1) < (uint32)cmd->param2)
		return;

	setNextLine(cmd->param3);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x4F
IMPLEMENT_OPCODE(ClearScreen)
	getSharedData()->setFlag(kFlagSkipDrawScene, (bool)cmd->param1);

	if (cmd->param1)
		getScreen()->clear();
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x50
IMPLEMENT_OPCODE(Quit)
	getScreen()->clear();
	Engine::quitGame();

	// We need to exit the interpreter loop so we get back to the event loop
	// and get the quit message
	_exit = true;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x51
IMPLEMENT_OPCODE(JumpObjectFrame)
	Object *object = getWorld()->getObjectById((ObjectId)cmd->param1);

	if (cmd->param2 == -1)
		cmd->param2 = object->getFrameCount() - 1;

	if (cmd->param3) {
		if (object->getFrameIndex() == (uint32)cmd->param2)
			return;
	} else if (cmd->param4) {
		if (object->getFrameIndex() < (uint32)cmd->param2)
			return;
	} else if (cmd->param5) {
		if (object->getFrameIndex() > (uint32)cmd->param2)
			return;
	} else if (cmd->param6) {
		if (object->getFrameIndex() <= (uint32)cmd->param2)
			return;
	} else if (cmd->param7) {
		if (object->getFrameIndex() >= (uint32)cmd->param2)
			return;
	} else if (!cmd->param8 || object->getFrameIndex() != (uint32)cmd->param2) {
		return;
	}

	setNextLine(cmd->param9);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x52
IMPLEMENT_OPCODE(DeleteGraphics)
	for (uint i = 0; i < 55; i++)
		getScreen()->deleteGraphicFromQueue(getScene()->getActor((ActorIndex)(cmd->param1))->getResourcesId((uint32)cmd->param1));
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x53
IMPLEMENT_OPCODE(SetActorField944)
	Actor *actor = getScene()->getActor(cmd->param1);

	actor->setField944(cmd->param2);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x54
IMPLEMENT_OPCODE(SetScriptField1BB0)
	if (cmd->param2)
		_currentScript->field_1BB0 = _vm->getRandom((uint32)cmd->param1);
	else
		_currentScript->field_1BB0 = cmd->param1;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x55
IMPLEMENT_OPCODE(OnScriptField1BB0)

	if (cmd->param2) {
		if (_currentScript->field_1BB0 == cmd->param1)
			return;
	} else {

		if (cmd->param3) {
			if (_currentScript->field_1BB0 < cmd->param1)
				return;
		} else if (cmd->param4) {
			if (_currentScript->field_1BB0 > cmd->param1)
				return;
		} else if (!cmd->param5) {
			if (cmd->param6) {
				if (_currentScript->field_1BB0 >= cmd->param1)
					return;
			} else {
				if (!cmd->param7 || _currentScript->field_1BB0 != cmd->param1)
					return;
			}
		} else {
			if (_currentScript->field_1BB0 <= cmd->param1)
				return;
		}
	}

	setNextLine(cmd->param8);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x56
IMPLEMENT_OPCODE(WalkToActor)
	Actor *player = getScene()->getActor(), *actor = getScene()->getActor((ActorIndex)cmd->param1);

	if (cmd->param2 == 2) {
		if (player->getStatus() == kActorStatusWalkingTo || player->getStatus() == kActorStatusWalkingTo2) {
			_processNextEntry = true;

			return;
		}

		cmd->param2 = 1;
		_processNextEntry = false;

		if ((player->getPoint1()->x + player->getPoint2()->x == cmd->param6)
		 && (player->getPoint1()->y + player->getPoint2()->y == cmd->param7)) {
			player->faceTarget((uint32)cmd->param1, kDirectionFromActor);
			actor->changeDirection((ActorDirection)((player->getDirection() + 4) & 7));
		} else {
			_currentQueueEntry->currentLine = cmd->param3;
		}
	} else {
		Common::Point point;

		if (actor->getStatus() == kActorStatusWalkingTo || actor->getStatus() == kActorStatusWalkingTo2)
			return;

		if (actor->aNicePlaceToTalk(&point, &cmd->param4)) {
			player->forceTo(point.x, point.y, (bool)cmd->param4);
			cmd->param6 = point.x;
			cmd->param7 = point.y;

			if (cmd->param2 == 1) {
				cmd->param2 = 2;
				_processNextEntry = true;
			}
		} else {
			if (cmd->param4)
				getSpeech()->playIndexed(1);

			_currentQueueEntry->currentLine = cmd->param3;
		}
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x57
IMPLEMENT_OPCODE(SetResourcePalette)
	getWorld()->currentPaletteId = getWorld()->graphicResourceIds[cmd->param1];
	getScreen()->setPalette(getWorld()->currentPaletteId);
	getScreen()->setGammaLevel(getWorld()->currentPaletteId);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x58
IMPLEMENT_OPCODE(SetObjectFrameIndexAndFlags)
	Object *object = getWorld()->getObjectById((ObjectId)cmd->param1);

	if (cmd->param3)
		object->flags = 1 | object->flags;
	else
		object->flags &= ~kObjectFlagEnabled;

	object->setFrameIndex(cmd->param2);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x59
IMPLEMENT_OPCODE(SetObjectFlags)
	Object *object = getWorld()->getObjectById((ObjectId)cmd->param1);

	if (cmd->param2) {
		object->flags |= kObjectFlag40000;
	} else {
		object->flags &= ~kObjectFlag10E38;
	}

	if (cmd->param3 && (object->flags & kObjectFlag10E38))
		_processNextEntry = true;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x5A
IMPLEMENT_OPCODE(SetActorActionIndex2)
	getScene()->getActor(cmd->param1)->setActionIndex2(cmd->param2);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x5B
IMPLEMENT_OPCODE(UpdateTransparency)
	if (cmd->param2 >= 0 && cmd->param2 <= 3) {
		if (cmd->param1) {
			Object *object = getWorld()->getObjectById((ObjectId)cmd->param1);
			if (!object) {
				debugC(kDebugLevelObjects, "Object with id %d doesn't exist", cmd->param1);
				return;
			}

			if (!cmd->param2)
				object->setTransparency(0);
			else
				object->setTransparency(cmd->param2 + 3);
		} else {
			getScene()->getActor(cmd->param3)->setTransparency(cmd->param2);
		}
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x5C
IMPLEMENT_OPCODE(QueueScript)
	queueScript(getWorld()->getActionAreaById(cmd->param1)->scriptIndex, cmd->param2);
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x5D
IMPLEMENT_OPCODE(ProcessActor)
	Actor *actor = getScene()->getActor(cmd->param1);

	actor->setupReflectionData(cmd->param2, cmd->param3, (ActorDirection)cmd->param4, Common::Point((int16)cmd->param5, (int16)cmd->param6), (bool)cmd->param7, Common::Point((int16)cmd->param8, (int16)cmd->param9));
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x5E
IMPLEMENT_OPCODE(ClearActorFields)
	Actor *actor = getScene()->getActor(cmd->param1);

	// Clear fields starting from field_970
	actor->clearFields();
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x5F
IMPLEMENT_OPCODE(SetObjectLastFrameIndex)
	Object *object = getWorld()->getObjectById((ObjectId)cmd->param1);

	if (object->getFrameIndex() == object->getFrameCount() - 1) {
		_processNextEntry = false;
		object->flags &= ~kObjectFlag10E38;
	} else {
		_processNextEntry = true;
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x60
IMPLEMENT_OPCODE(SetActionAreaFlags)
	ActionArea *area = getWorld()->getActionAreaById(cmd->param1);

	if (cmd->param2)
		area->flags |= 1;
	else
		area->flags &= ~1;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x61
IMPLEMENT_OPCODE(MorphActor)
	if (cmd->param2) {
		if (getWorld()->nextPlayer == kActorInvalid) {
			_processNextEntry = false;
			cmd->param2 = 0;
		} else {
			_processNextEntry = true;
		}
	} else {
		Actor::morphInto(_vm, cmd->param1);
		cmd->param2 = 1;
		_processNextEntry = true;
	}
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x62
IMPLEMENT_OPCODE(ShowMenu)
	if (!_vm->isGameFlagSet(kGameFlag3931)) {
		_vm->unlockAchievement("ASYLUM_LEVEL_13");
		_vm->setGameFlag(kGameFlag3931);
	}
	_vm->menu()->show();
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode 0x63
IMPLEMENT_OPCODE(UpdateGlobalFlags)
	if (!_vm->sound()->isPlaying(getSpeech()->getSoundResourceId())) {
		if (cmd->param1) {
			getSharedData()->setFlag(kFlag1, false);
			getSharedData()->setFlag(kFlag2, false);
		} else {
			cmd->param1 = 1;
		}

		return;
	}

	if (cmd->param1) {
		getSharedData()->setFlag(kFlag1, true);
		getSharedData()->setFlag(kFlag2, true);
	}
	_processNextEntry = true;
END_OPCODE

//////////////////////////////////////////////////////////////////////////
// Opcode Helper functions
//////////////////////////////////////////////////////////////////////////

void ScriptManager::enableObject(ScriptEntry *cmd, ObjectTransparency type) {
	if (!_currentScript)
		error("[ScriptManager::enableObject] No current script");

	int32 transparency = 0;

	// Setup transparency
	switch (type) {
	default:
		break;

	case kObjectEnableType0:
		transparency = 4 + _currentScript->counter / cmd->param2;
		break;

	case kObjectEnableType1:
		transparency = 6 - _currentScript->counter / cmd->param2;
		break;

	case kObjectTransparencyOpaque:
		transparency = 0;
		break;
	}

	// Update first set of objects
	int32 *param = &cmd->param4;
	for (int i = 0; i < 6; i++) {
		Object *object = getWorld()->getObjectById((ObjectId)*param);
		if (object != nullptr)
			object->setTransparency(transparency);

		++param;
	}

	// Update per-chapter objects
	switch (getWorld()->chapter) {
	default:
		break;

	case kChapter3:
		if (cmd->param1 != 810)
			break;

		getWorld()->getObjectById(kObjectTableRecordRoom)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectOrangeRecord)->setTransparency(transparency);
		break;

	case kChapter4:
		if (cmd->param1 != 1232)
			break;

		getWorld()->getObjectById(kObjectInfernoStatusQuo)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectJugglerWithPin)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectJuggler)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectClownStatusQuo)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectTrixieStatusQuo)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectSimonStatusQuo)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectBigTopBarrel)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectStandBehindJuggler)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectStrongmanLeft)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectStrongmanRight)->setTransparency(transparency);
		break;

	case kChapter6:
		if (cmd->param1 == 1998) {
			getWorld()->getObjectById(kObjectGlow4)->setTransparency(transparency);
			getWorld()->getObjectById(kObjectBugOnTable)->setTransparency(transparency);
			getWorld()->getObjectById(kObjectInsidePipeCyberPod)->setTransparency(transparency);
			getWorld()->getObjectById(kObjectDiscardedBugPincer)->setTransparency(transparency);
			getWorld()->getObjectById(kObjectLitLimbScanner)->setTransparency(transparency);
			getWorld()->getObjectById(kObjectCyberTable)->setTransparency(transparency);
		}

		if (cmd->param1 == 2003) {
			getWorld()->getObjectById(kObjectNPC066StatusQuo)->setTransparency(transparency);
			/* getWorld()->getObjectById(kObject2507)->setTransparency(transparency); // XXX no such object */
			getWorld()->getObjectById(kObjectBrokenPipe)->setTransparency(transparency);
			getWorld()->getObjectById(kObjectEmberPopsOut)->setTransparency(transparency);
			getWorld()->getObjectById(kObjectBugCarriesEmber)->setTransparency(transparency);
			getWorld()->getObjectById(kObjectFurnaceHole)->setTransparency(transparency);
			getWorld()->getObjectById(kObjectTopOfFurnace)->setTransparency(transparency);
			getWorld()->getObjectById(kObjectElderBugLimb)->setTransparency(transparency);
		}
		break;

	case kChapter7:
		if (cmd->param1 != 1273)
			break;

		getWorld()->getObjectById(kObjectHeadOnTable)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectOfficeWallNew)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectChalice)->setTransparency(transparency);
		break;

	case kChapter8:
		if (cmd->param1 != 1795)
			break;

		getWorld()->getObjectById(kObjectHook1B)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectHook2B)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectHook3B)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectHook4B)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectHook5B)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectHook6B)->setTransparency(transparency);
		//getWorld()->getObjectById(kObjectHook0Down)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectHook2Down)->setTransparency(transparency);
		//getWorld()->getObjectById(kObjectHook3Down)->setTransparency(transparency);
		getWorld()->getObjectById(kObjectHook4Down)->setTransparency(transparency);
		//getWorld()->getObjectById(kObjectHook5Down)->setTransparency(transparency);
		//getWorld()->getObjectById(kObjectHook6Down)->setTransparency(transparency);
		//getWorld()->getObjectById(kObject2230)->setTransparency(transparency);
		break;
	}
}

void ScriptManager::setActionFlag(ScriptEntry *cmd, ActionType flag) {
	switch (cmd->param2) {
	default:
		if (!getWorld()->getObjectById((ObjectId)cmd->param1))
			return;

		getWorld()->getObjectById((ObjectId)cmd->param1)->actionType |= flag;
		break;

	case 1:
		getWorld()->getActionAreaById(cmd->param1)->actionType |= flag;
		break;

	case 2:
		getWorld()->actors[cmd->param1]->actionType |= flag;
		break;
	}
}

void ScriptManager::clearActionFlag(ScriptEntry *cmd, ActionType flag) {
	switch (cmd->param2) {
	default:
		if (!getWorld()->getObjectById((ObjectId)cmd->param1))
			return;

		getWorld()->getObjectById((ObjectId)cmd->param1)->actionType &= ~flag;
		break;

	case 1:
		getWorld()->getActionAreaById(cmd->param1)->actionType &= ~flag;
		break;

	case 2:
		getWorld()->actors[cmd->param1]->actionType &= ~flag;
		break;
	}
}

void ScriptManager::jumpIfActionFlag(ScriptEntry *cmd, ActionType flag) {
	bool done = false;

	switch (cmd->param3) {
	default:
		done = (getWorld()->actors[cmd->param1]->actionType & flag) == 0;
		break;

	case 0:
		done = (getWorld()->getObjectById((ObjectId)cmd->param1)->actionType & flag) == 0;
		break;

	case 1:
		done = (getWorld()->getActionAreaById(cmd->param1)->actionType & flag) == 0;
		break;
	}

	if (!done)
		return;

	setNextLine(cmd->param2);
}

void ScriptManager::setNextLine(int32 line) {
	if (!_currentScript)
		error("[ScriptManager::setNextLine] No current script");

	if (!_currentQueueEntry)
		error("[ScriptManager::setNextLine] No current queue entry");

	OpcodeType opcode = _currentScript->commands[line].opcode;
	if (opcode == kOpcodeReturn1 || opcode == kOpcodeReturn) {
		_currentQueueEntry->currentLine = line;
	} else {
		_done = true;
	}
}

} // end of namespace Asylum
