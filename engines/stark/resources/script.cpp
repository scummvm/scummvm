/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/resources/script.h"

#include "engines/stark/formats/xrc.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/command.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/scroll.h"
#include "engines/stark/resources/sound.h"
#include "engines/stark/resources/speech.h"

#include "engines/stark/services/dialogplayer.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"
#include "engines/stark/services/userinterface.h"

#include "engines/stark/tools/decompiler.h"

namespace Stark {
namespace Resources {

Script::~Script() {
}

Script::Script(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_scriptType(0),
		_runEvent(0),
		_minChapter(0),
		_maxChapter(999),
		_shouldResetGameSpeed(false),
		_enabled(false),
		_nextCommand(nullptr),
		_pauseTimeLeft(-1),
		_suspendingResource(nullptr),
		_resumeStatus(kResumeSuspend) {
	_type = TYPE;
}

void Script::readData(Formats::XRCReadStream *stream) {
	uint32 type = stream->readUint32LE();
	_runEvent = stream->readUint32LE();
	_minChapter = stream->readUint32LE();
	_maxChapter = stream->readUint32LE();
	_shouldResetGameSpeed = stream->readBool();

	_enabled = type == 0;

	switch (_subType) {
	case kSubTypeGameEvent:
		_scriptType = type == 2 ? kScriptTypePassiveDialog : kScriptTypeOnGameEvent;
		break;
	case kSubTypePlayerAction:
		_scriptType = kScriptTypeOnPlayerAction;
		break;
	case kSubTypeDialog:
		_scriptType = kScriptType4;
		break;
	default:
		error("Unknown script subtype %d for script %s", _subType, getName().c_str());
	}
}

void Script::onAllLoaded() {
	Object::onAllLoaded();
	reset();
}

void Script::onGameLoop() {
	Object::onGameLoop();
	execute(kCallModeGameLoop);
}

void Script::reset() {
	if (_suspendingResource && _suspendingResource->getType() == Type::kItem) {
		Item *item = _suspendingResource->cast<Item>(_suspendingResource);
		item->setMovement(nullptr);
	}

	_suspendingResource = nullptr;
	_resumeStatus = kResumeSuspend;
	_pauseTimeLeft = -1;

	_nextCommand = getBeginCommand();
}

bool Script::isOnBegin() {
	return _nextCommand && _nextCommand->getSubType() == Command::kCommandBegin;
}

bool Script::isOnEnd() {
	return _nextCommand && _nextCommand->getSubType() == Command::kCommandEnd;
}

Command *Script::getBeginCommand() {
	return findChildWithSubtype<Command>(Command::kCommandBegin, false);
}

bool Script::isEnabled() {
	switch (_scriptType) {
	case kScriptTypeOnGameEvent:
	case kScriptTypeOnPlayerAction:
		return _enabled;
	case kScriptType3:
		return false;
	case kScriptTypePassiveDialog:
	case kScriptType4:
		return true;
	default:
		error("Unknown script type %d for script %s", _scriptType, getName().c_str());
	}
}

void Script::enable(bool value) {
	if (_scriptType == kScriptTypeOnGameEvent || _scriptType == kScriptTypeOnPlayerAction) {
		_enabled = value;
	}
}

bool Script::shouldExecute(uint32 callMode) {
	if ((!isEnabled() && isOnBegin()) || !_nextCommand) {
		return false; // Don't execute disabled scripts
	}

	if (callMode == kCallModeGameLoop && !isOnBegin()) {
		return true; // Continue previously running script
	}

	if (_scriptType == kScriptTypeOnGameEvent) {
		if (_runEvent == kGameEventOnGameLoop && callMode != kCallModeGameLoop) {
			return false; // Wrong call mode for this script
		}
		if (_runEvent == kGameEventOnEnterLocation && callMode != kCallModeEnterLocation) {
			return false; // Wrong call mode for this script
		}
		if (_runEvent == kGameEventOnExitLocation && callMode != kCallModeExitLocation) {
			return false; // Wrong call mode for this script
		}

		Item *parentItem = findParent<Item>();
		if (parentItem && !parentItem->isEnabled()) {
			return false; // Disabled parent
		}
	} else if (_scriptType == kScriptTypePassiveDialog) {
		if (callMode != kCallModeDialogCreateSelections && callMode != kCallModeDialogAnswer) {
			return false; // Wrong call mode for this script
		}
	} else if (_scriptType == kScriptTypeOnPlayerAction) {
		if (callMode != kCallModePlayerAction) {
			return false; // Wrong call mode for this script
		}
	} else {
		return false; // Wrong script type
	}

	uint32 currentChapter = StarkGlobal->getCurrentChapter();
	if (currentChapter < _minChapter || currentChapter >= _maxChapter) {
		return false; // Wrong chapter
	}

	return true;
}

bool Script::isSuspended() {
	return _pauseTimeLeft >= 0 || _suspendingResource;
}

Object *Script::getSuspendingResource() const {
	return _suspendingResource;
}

void Script::updateSuspended() {
	if (_pauseTimeLeft >= 0) {
		// Decrease the remaining pause time
		_pauseTimeLeft -= StarkGlobal->getMillisecondsPerGameloop();
	} else {
		_pauseTimeLeft = -1;
	}

	if (_nextCommand->getSubType() == Command::kScriptPauseSkippable
			&& (StarkUserInterface->wasInteractionDenied() || _pauseTimeLeft < 0)) {
		StarkUserInterface->setInteractive(true);
		_pauseTimeLeft = -1;
	}

	bool commandChanged = false;

	if (_suspendingResource) {
		// Check if the suspending resource is still active
		switch (_suspendingResource->getType().get()) {
		case Type::kDialog: {
			if (!StarkDialogPlayer->isRunning()) {
				// Resume the script execution if the dialog is complete
				_suspendingResource = nullptr;
			}
			break;
		}
		case Type::kFMV: {
			// Scripts are not running during an FMV, if we are here, then it has stopped playing
			_suspendingResource = nullptr;
			break;
		}
		case Type::kSoundItem: {
			Sound *soundItem = Object::cast<Sound>(_suspendingResource);
			if (!soundItem->isPlaying()) {
				// Resume the script execution once the sound has stopped playing
				_suspendingResource = nullptr;
			}
			break;
		}
		case Type::kSpeech: {
			Speech *speech = Object::cast<Speech>(_suspendingResource);
			if (!StarkDialogPlayer->isSpeechReady(speech) && !speech->isPlaying()) {
				// Resume the script execution once the speech has stopped playing
				_suspendingResource = nullptr;
			}
			break;
		}
		case Type::kScroll: {
			Scroll *scroll = Object::cast<Scroll>(_suspendingResource);
			if (!scroll->isActive()) {
				// Resume the script execution once the scroll target position is reached
				_suspendingResource = nullptr;
			}
			break;
		}
		case Type::kItem: {
			if (_nextCommand->getSubType() == Command::kWalkTo) {
				if (_resumeStatus == kResumeComplete) {
					// Resume the script execution once the item has stopped its movement
					_suspendingResource = nullptr;
					_nextCommand = _nextCommand->nextCommandIf(false);
					commandChanged = true;
				} else if (_resumeStatus == kResumeAbort) {
					// Resume the script execution once the item has stopped its movement
					_suspendingResource = nullptr;
					_nextCommand = _nextCommand->nextCommandIf(true);
					commandChanged = true;
				}
			} else {
				if (_resumeStatus != kResumeSuspend) {
					// Resume the script execution once the item has stopped its movement
					_suspendingResource = nullptr;
				}
			}
			break;
		}
		case Type::kAnim: {
			Anim *anim = Object::cast<Anim>(_suspendingResource);
			if (anim->isDone()) {
				anim->resetItem();
				// Resume the script execution once the animation is complete
				_suspendingResource = nullptr;
			}
			break;
		}
		default:
			error("Unhandled suspending resource type %s", _suspendingResource->getType().getName());
		}
	}

	if (_nextCommand->getSubType() == Command::kItemSetActivity && !_suspendingResource) {
		_nextCommand->resumeItemSetActivity();
	}

	if (!isSuspended() && _shouldResetGameSpeed) {
		StarkGlobal->setNormalSpeed();
	}

	if (!isSuspended() && !commandChanged) {
		// Resume to the next command
		goToNextCommand();
	}
}

void Script::stop() {
	reset();
	_enabled = false;
	_returnObjects.clear();
}

void Script::pause(int32 msecs) {
	_pauseTimeLeft = msecs;
}

void Script::suspend(Object *cause) {
	_suspendingResource = cause;
	_resumeStatus = kResumeSuspend;
}

void Script::setResumeStatus(ResumeStatus status) {
	_resumeStatus = status;
}

void Script::goToNextCommand() {
	_nextCommand = _nextCommand->nextCommand();
}

void Script::execute(uint32 callMode) {
	if (!shouldExecute(callMode)) {
		return;
	}

	if (isSuspended()) {
		// If the script is suspended, check if it can be resumed
		updateSuspended();
	}

	uint32 executedCommands = 0;
	while (1) {
		if (isSuspended()) {
			break;
		}

		if (!_nextCommand) {
			break; // No next command, stop here
		}

		if (isOnEnd()) {
			break; // Reached the end of the script
		}

		_nextCommand = _nextCommand->execute(callMode, this);

		executedCommands++;

		if (executedCommands > 50) {
			break; // Too many consecutive commands
		}
	}

	if (isOnEnd() || !_nextCommand) {
		// Reset ended scripts so they can be started again
		reset();

		// Check if we should return to some caller script
		if (!_returnObjects.empty()) {
			Object *callerObject = _returnObjects.back();
			_returnObjects.pop_back();

			// Resume execution of the caller object
			resumeCallerExecution(callerObject);
		}
	}
}

void Script::resumeCallerExecution(Object *callerObject) {
	switch (callerObject->getType().get()) {
		case Type::kCommand: {
			Command *callerCommand = Object::cast<Command>(callerObject);
			_nextCommand = callerCommand->nextCommand();
			break;
		}
		case Type::kDialog: {
			Dialog *callerDialog = Object::cast<Dialog>(callerObject);
			StarkDialogPlayer->resume(callerDialog);
			break;
		}
		default:
			error("Unhandled caller object type %s", callerObject->getType().getName());
	}
}

void Script::addReturnObject(Object *object) {
	_returnObjects.push_back(object);
}

void Script::print(uint depth) {
	printDescription(depth);
	printData();

	// Print anything that is not a command
	for (uint i = 0; i < _children.size(); i++) {
		if (_children[i]->getType() != Type::kCommand) {
			_children[i]->print(depth + 1);
		}
	}

	Tools::Decompiler *decompiler = new Tools::Decompiler(this);

	// Print the decompiled output
	printWithDepth(depth + 1, "Decompiled output");
	if (decompiler->getError() == "") {
		decompiler->printDecompiled();
	} else {
		debug("Decompilation failure: %s", decompiler->getError().c_str());
	}

	delete decompiler;
}

void Script::printData() {
	debug("scriptType: %d", _scriptType);
	debug("runEvent: %d", _runEvent);
	debug("minChapter: %d", _minChapter);
	debug("maxChapter: %d", _maxChapter);
	debug("shouldResetGameSpeed: %d", _shouldResetGameSpeed);
}

void Script::saveLoad(ResourceSerializer *serializer) {
	serializer->syncAsSint32LE(_enabled);
}

void Script::saveLoadCurrent(ResourceSerializer *serializer) {
	bool isStarted = !isOnBegin();
	serializer->syncAsUint32LE(isStarted);

	if (isStarted) {
		serializer->syncAsResourceReference(&_nextCommand);

		serializer->syncArraySize(_returnObjects);
		for (uint i = 0; i < _returnObjects.size(); i++) {
			serializer->syncAsResourceReference(&_returnObjects[i]);
		}

		serializer->syncAsSint32LE(_pauseTimeLeft);
		serializer->syncAsResourceReference(&_suspendingResource);
		serializer->syncAsSint32LE(_resumeStatus);
	}
}

} // End of namespace Resources
} // End of namespace Stark
