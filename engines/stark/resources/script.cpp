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

#include "engines/stark/resources/command.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/services/resourceprovider.h"
#include "engines/stark/stark.h"
#include "engines/stark/xrcreader.h"

namespace Stark {

Script::~Script() {
}

Script::Script(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Resource(parent, subType, index, name),
				_scriptType(0),
				_runEvent(0),
				_minChapter(0),
				_maxChapter(999),
				_shouldResetGameSpeed(false),
				_enabled(false),
				_nextCommand(nullptr),
				_pauseTimeLeft(-1) {
	_type = TYPE;
}

void Script::readData(XRCReadStream *stream) {
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
	Resource::onAllLoaded();
	reset();
}

void Script::onGameLoop() {
	Resource::onGameLoop();
	execute(kCallModeGameLoop);
}

void Script::reset() {
	_nextCommand = findChildWithSubtype<Command>(Command::kCommandBegin);
}

bool Script::isOnBegin() {
	return _nextCommand && _nextCommand->getSubType() == Command::kCommandBegin;
}

bool Script::isOnEnd() {
	return _nextCommand && _nextCommand->getSubType() == Command::kCommandEnd;
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
		if (callMode != kCallModeDialogCreateSelections || callMode != kCallModeDialogAnswer) {
			return false; // Wrong call mode for this script
		}
	} else if (_scriptType == kScriptTypeOnPlayerAction) {
		if (callMode != kCallModePlayerAction) {
			return false; // Wrong call mode for this script
		}
	} else {
		return false; // Wrong script type
	}

	uint32 currentChapter = 0; // TODO: Implement
	if (currentChapter < _minChapter || currentChapter > _maxChapter) {
		return false; // Wrong chapter
	}

	return true;
}

bool Script::isSuspended() {
	return _pauseTimeLeft >= 0;
}

void Script::updateSuspended() {
	if (_pauseTimeLeft >= 0) {
		// Decrease the remaining pause time
		Global *global = StarkServices::instance().global;
		_pauseTimeLeft -= global->getMillisecondsPerGameloop();
	}

	if (_pauseTimeLeft < 0) {
		// Resume to the next command
		_pauseTimeLeft = -1;
		_nextCommand = _nextCommand->nextCommand();
	}
}

void Script::pause(int32 msecs) {
	_pauseTimeLeft = msecs;
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

		_nextCommand = _nextCommand->execute(callMode, this);

		executedCommands++;

		if (!_nextCommand) {
			break; // No next command, stop here
		}

		if (isOnEnd()) {
			break; // Reached the end of the script
		}

		if (executedCommands > 50) {
			break; // Too many consecutive commands
		}
	}

	if (isOnEnd()) {
		// Reset ended scripts so they can be started again
		reset();
	}
}

void Script::printData() {
	debug("scriptType: %d", _scriptType);
	debug("runEvent: %d", _runEvent);
	debug("minChapter: %d", _minChapter);
	debug("maxChapter: %d", _maxChapter);
	debug("shouldResetGameSpeed: %d", _shouldResetGameSpeed);
}

} // End of namespace Stark
