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

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/wolfcommands.h"

namespace Nancy {
namespace Action {

static const char *const kEngineTargetName = "Engine_WolfCommand";

WolfCommands::~WolfCommands() {
	UI::NDUI::Manager *ndui = NancySceneState.getNDUI();
	if (!_isRegistered || !ndui) {
		return;
	}

	ndui->removeEngineTarget(kEngineTargetName);
	ndui->takeAction(UI::NDUI::kActionHide, _dialogName, 1.0);
	ndui->takeAction(UI::NDUI::kActionSetText, _displayName, Common::String());
	for (const Command &command : _commands) {
		ndui->destroyObject(command.buttonName);
	}
}

void WolfCommands::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _playlistName);
	readFilename(stream, _dialogName);
	readFilename(stream, _buttonTemplate);
	readFilename(stream, _displayName);
	_maxCommands = stream.readUint32LE();
	readFilename(stream, _separator);
	_doneFlag.label = stream.readSint16LE();
	_doneFlag.flag = stream.readByte();

	_commands.resize(MAX<int16>(stream.readSint16LE(), 0));
	for (Command &command : _commands) {
		readFilename(stream, command.buttonName);
		readFilename(stream, command.label);
		command.clickSound.readData(stream);
		command.callSound.readData(stream);
	}

	_nameSound.readData(stream);
	_goSound.readData(stream);
	_fullSound.readData(stream);
}

const WolfCommands::Command *WolfCommands::findCommand(const Common::String &buttonName) const {
	for (const Command &command : _commands) {
		if (command.buttonName.equalsIgnoreCase(buttonName)) {
			return &command;
		}
	}

	return nullptr;
}

void WolfCommands::addCommand(const Common::String &buttonName) {
	const Command *command = findCommand(buttonName);
	if (!command) {
		return;
	}

	_queue.push_back(command->buttonName);
	command->clickSound.play();
	NancySceneState.getNDUI()->takeAction(UI::NDUI::kActionAddText, _displayName, command->label + _separator);
}

void WolfCommands::clearCommands() {
	NancySceneState.getNDUI()->takeAction(UI::NDUI::kActionSetText, _displayName, Common::String());
	_queue.clear();
}

// Nancy calls the wolf by her name, then each of the queued commands, then says "go";
// after that, the wolf gets the commands
void WolfCommands::callNextCommand() {
	++_callIndex;
	if (_callIndex < (int)_queue.size()) {
		_callSound = findCommand(_queue[_callIndex])->callSound.play();
		return;
	}

	if (_callIndex == (int)_queue.size()) {
		_callSound = _goSound.play();
		return;
	}

	NancySceneState.setMoviePlaylist(_playlistName, _queue);
	NancySceneState.setEventFlag(_doneFlag);
	NancySceneState.getNDUI()->takeAction(UI::NDUI::kActionEnable, _dialogName, 1.0);

	_isCalling = false;
	_clearOnNextCommand = true;
}

void WolfCommands::execute() {
	UI::NDUI::Manager *ndui = NancySceneState.getNDUI();
	if (!ndui) {
		return;
	}

	switch (_state) {
	case kBegin:
		for (const Command &command : _commands) {
			ndui->createObject(command.buttonName, _buttonTemplate, _dialogName);
			ndui->takeAction(UI::NDUI::kActionSetText, command.buttonName, command.label);
		}

		ndui->addEngineTarget(kEngineTargetName, this);
		_isRegistered = true;
		ndui->takeAction(UI::NDUI::kActionShow, _dialogName, 1.0);
		ndui->takeAction(UI::NDUI::kActionEnable, _dialogName, 1.0);
		_state = kRun;
		break;
	case kRun:
		if (_isCalling && !g_nancy->_sound->isSoundPlaying(_callSound)) {
			callNextCommand();
		}

		break;
	default:
		break;
	}
}

// The dialog's buttons: a command button adds its command, "Clear" empties the list and
// "Go" has Nancy call the commands out
void WolfCommands::onEngineAction(const UI::NDUI::Action &action) {
	switch (action.type) {
	case UI::NDUI::kActionAddText:
		if (_queue.size() < _maxCommands) {
			addCommand(action.text);
		} else if (_clearOnNextCommand) {
			clearCommands();
			addCommand(action.text);
		} else {
			_fullSound.play();
			return;
		}

		break;
	case UI::NDUI::kActionSetText:
		clearCommands();
		break;
	case UI::NDUI::kActionSetValue:
		NancySceneState.getNDUI()->takeAction(UI::NDUI::kActionDisable, _dialogName, 1.0);
		_callSound = _nameSound.play();
		_callIndex = -1;
		_isCalling = true;
		break;
	default:
		return;
	}

	_clearOnNextCommand = false;
}

} // End of namespace Action
} // End of namespace Nancy
