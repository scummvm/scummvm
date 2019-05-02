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

#include "mutationofjb/script.h"

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stream.h"
#include "common/debug.h"
#include "mutationofjb/commands/command.h"
#include "mutationofjb/commands/ifcommand.h"
#include "mutationofjb/commands/ifitemcommand.h"
#include "mutationofjb/commands/ifpiggycommand.h"
#include "mutationofjb/commands/endblockcommand.h"
#include "mutationofjb/commands/changecommand.h"
#include "mutationofjb/commands/saycommand.h"
#include "mutationofjb/commands/additemcommand.h"
#include "mutationofjb/commands/removeitemcommand.h"
#include "mutationofjb/commands/removeallitemscommand.h"
#include "mutationofjb/commands/labelcommand.h"
#include "mutationofjb/commands/gotocommand.h"
#include "mutationofjb/commands/camefromcommand.h"
#include "mutationofjb/commands/callmacrocommand.h"
#include "mutationofjb/commands/newroomcommand.h"
#include "mutationofjb/commands/renamecommand.h"
#include "mutationofjb/commands/definestructcommand.h"
#include "mutationofjb/commands/talkcommand.h"
#include "mutationofjb/commands/randomcommand.h"
#include "mutationofjb/commands/setcolorcommand.h"
#include "mutationofjb/commands/specialshowcommand.h"
#include "mutationofjb/commands/switchpartcommand.h"
#include "mutationofjb/commands/loadplayercommand.h"
#include "mutationofjb/commands/bitmapvisibilitycommand.h"
#include "mutationofjb/commands/playanimationcommand.h"
#include "mutationofjb/commands/setobjectframecommand.h"
#include "mutationofjb/game.h"

namespace MutationOfJB {

static CommandParser **getParsers() {
	static CommandParser *parsers[] = {
		new IfPiggyCommandParser,
		new IfItemCommandParser,
		new IfCommandParser,
		new CameFromCommandParser,
		new CallMacroCommandParser,
		new EndBlockCommandParser,
		new ChangeDoorCommandParser,
		new ChangeObjectCommandParser,
		new ChangeStaticCommandParser,
		new ChangeSceneCommandParser,
		new DefineStructCommandParser,
		new SayCommandParser,
		new TalkCommandParser,
		new AddItemCommandParser,
		new RemoveItemCommandParser,
		new RemoveAllItemsCommandParser,
		new RenameCommandParser,
		new NewRoomCommandParser,
		new GotoCommandParser,
		new LabelCommandParser,
		new RandomCommandParser,
		new RandomBlockStartParser,
		new SetColorCommandParser,
		new SpecialShowCommandParser,
		new SwitchPartCommandParser,
		new LoadPlayerCommandParser,
		new BitmapVisibilityCommandParser,
		new PlayAnimationCommandParser,
		new SetObjectFrameCommandParser,
		nullptr
	};

	return parsers;
}


ScriptParseContext::ScriptParseContext(Common::SeekableReadStream &stream) :
	_stream(stream),
	_currentCommand(nullptr),
	_lastCommand(nullptr),
	_pendingRandomCommand(nullptr) {}

bool ScriptParseContext::readLine(Common::String &line) {
	do {
		Common::String str = _stream.readLine();
		if (str.empty())
			continue;

		if (str[0] != '.') {
			line = str;
			if (line[0] == '*') {
				line.deleteChar(0);
			}
			return true;
		}
	} while (!_stream.eos());

	return false;
}

void ScriptParseContext::addConditionalCommand(ConditionalCommand *command, char tag, bool firstHash) {
	ConditionalCommandInfo cmi = {command, tag, firstHash};
	_pendingCondCommands.push_back(cmi);
}


void ScriptExecutionContext::pushReturnCommand(Command *cmd) {
	_callStack.push(cmd);
}

Command *ScriptExecutionContext::popReturnCommand() {
	if (_callStack.empty()) {
		return nullptr;
	}

	return _callStack.pop();
}

Game &ScriptExecutionContext::getGame() {
	return _game;
}

GameData &ScriptExecutionContext::getGameData() {
	return _game.getGameData();
}

void ScriptExecutionContext::clear() {
	_callStack.clear();
}

Command::ExecuteResult ScriptExecutionContext::runActiveCommand() {
	while (_activeCommand) {
		const Command::ExecuteResult result = _activeCommand->execute(*this);
		if (result == Command::Finished) {
			_activeCommand = _activeCommand->next();
		} else {
			return result;
		}
	}

	return Command::Finished;
}

Command::ExecuteResult ScriptExecutionContext::startCommand(Command *cmd) {
	if (_activeCommand) {
		warning("Trying to start command while another one is running.");
		return Command::Finished;
	}
	getGameData()._color = WHITE; // The original game resets the color to WHITE beforing running script sections.
	clear();
	_activeCommand = cmd;
	return runActiveCommand();
}

Command::ExecuteResult ScriptExecutionContext::startStartupSection() {
	Script *localScript = _localScriptOverride ? _localScriptOverride : _game.getLocalScript();

	if (localScript) {
		Command *const startupCmd = localScript->getStartup(_game.getGameData().getCurrentScene()->_startup);
		if (startupCmd) {
			return startCommand(startupCmd);
		}
	}

	return Command::Finished;
}

Command *ScriptExecutionContext::getMacro(const Common::String &name) const {
	Command *cmd = nullptr;

	Script *const localScript = _localScriptOverride ? _localScriptOverride : _game.getLocalScript();
	Script *const globalScript = _game.getGlobalScript();

	if (localScript) {
		cmd = localScript->getMacro(name);
	}

	if (!cmd && globalScript) {
		cmd = globalScript->getMacro(name);
	}

	return cmd;
}

Command *ScriptExecutionContext::getExtra(const Common::String &name) const {
	Command *cmd = nullptr;

	Script *const localScript = _localScriptOverride ? _localScriptOverride : _game.getLocalScript();
	Script *const globalScript = _game.getGlobalScript();

	if (localScript) {
		cmd = localScript->getExtra(name);
	}

	if (!cmd && globalScript) {
		cmd = globalScript->getExtra(name);
	}

	return cmd;
}

bool ScriptExecutionContext::isCommandRunning() const {
	return _activeCommand;
}

bool Script::loadFromStream(Common::SeekableReadStream &stream) {
	destroy();

	CommandParser **parsers = getParsers();

	ScriptParseContext parseCtx(stream);

	Common::String line;

	Command *lastCmd = nullptr;
	CommandParser *lastParser = nullptr;
	while (parseCtx.readLine(line)) {
		Command *currentCmd = nullptr;
		CommandParser *currentParser = nullptr;

		for (CommandParser **parser = parsers; *parser; ++parser) {
			if ((*parser)->parse(line, parseCtx, currentCmd)) {
				currentParser = *parser;
				break;
			}
		}
		if (!currentParser) {
			continue;
		}

		if (lastParser) {
			lastParser->transition(parseCtx, lastCmd, currentCmd, currentParser);
		}

		if (currentCmd) {
			_allCommands.push_back(currentCmd);
		}

		lastCmd = currentCmd;
		lastParser = currentParser;
	}

	for (CommandParser **parser = parsers; *parser; ++parser) {
		(*parser)->finish(parseCtx);
	}

	for (ActionInfos::iterator it = parseCtx._actionInfos.begin(); it != parseCtx._actionInfos.end(); ++it) {
		_actionInfos[it->_action].push_back(*it);
	}

	_macros = parseCtx._macros;
	_startups = parseCtx._startups;
	_extras = parseCtx._extras;

	return true;
}

void Script::destroy() {
	for (Commands::iterator it = _allCommands.begin(); it != _allCommands.end(); ++it) {
		delete *it;
	}
	_allCommands.clear();
}

Script::~Script() {
	destroy();
}

const ActionInfos &Script::getActionInfos(ActionInfo::Action action) {
	return _actionInfos[action];
}

const Commands &Script::getAllCommands() const {
	return _allCommands;
}

const Macros &Script::getMacros() const {
	return _macros;
}

Command *Script::getMacro(const Common::String &name) const {
	Macros::const_iterator it = _macros.find(name);
	if (it == _macros.end()) {
		return nullptr;
	}

	return it->_value;
}

const Startups &Script::getStartups() const {
	return _startups;
}

Command *Script::getStartup(uint8 startupId) const {
	Startups::const_iterator it = _startups.find(startupId);
	if (it == _startups.end()) {
		return nullptr;
	}

	return it->_value;
}

Command *Script::getExtra(const Common::String &name) const {
	Extras::const_iterator it = _extras.find(name);
	if (it == _extras.end()) {
		return nullptr;
	}

	return it->_value;
}

}
