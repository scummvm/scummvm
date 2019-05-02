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

#ifndef MUTATIONOFJB_SCRIPT_H
#define MUTATIONOFJB_SCRIPT_H

#include "mutationofjb/commands/command.h"
#include "common/array.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stack.h"

namespace Common {
class SeekableReadStream;
class String;
}

namespace MutationOfJB {

class Command;
class LabelCommand;
class Game;
class GotoCommand;
class ConditionalCommand;
class Script;
class RandomCommand;
struct GameData;

typedef Common::Array<Command *> Commands;

struct ActionInfo {
	enum Action {
		Look,
		Walk,
		Talk,
		Use,
		PickUp
	};

	Action _action;
	Common::String _entity1Name;
	Common::String _entity2Name;
	bool _walkTo;
	Command *_command;
};

typedef Common::Array<ActionInfo> ActionInfos;
typedef Common::Array<GotoCommand *> GotoCommands;
typedef Common::HashMap<Common::String, Command *> Macros;
typedef Common::HashMap<uint8, Command *> Startups;
typedef Common::HashMap<Common::String, Command *> Extras;

class ScriptParseContext {
public:
	ScriptParseContext(Common::SeekableReadStream &stream);
	bool readLine(Common::String &line);
	void addConditionalCommand(ConditionalCommand *command, char tag, bool firstHash);
	void addLookSection(const Common::String &item, bool walkTo);

	Common::SeekableReadStream &_stream;
	Command *_currentCommand;
	Command *_lastCommand;

	struct ConditionalCommandInfo {
		ConditionalCommand *_command;
		char _tag;
		bool _firstHash;
	};
	typedef Common::Array<ConditionalCommandInfo> ConditionalCommandInfos;
	ConditionalCommandInfos _pendingCondCommands;

	typedef Common::HashMap<Common::String, LabelCommand *> LabelMap;
	LabelMap _labels;

	typedef Common::HashMap<Common::String, GotoCommands> PendingGotoMap;
	PendingGotoMap _pendingGotos;

	RandomCommand *_pendingRandomCommand;

	ActionInfos _actionInfos;
	Macros _macros;
	Startups _startups;
	Extras _extras;

private:
};

class ScriptExecutionContext {
public:
	ScriptExecutionContext(Game &game, Script *localScriptOverride = nullptr) : _game(game), _activeCommand(nullptr), _localScriptOverride(localScriptOverride) {}
	void clear();

	Command::ExecuteResult runActiveCommand();
	Command::ExecuteResult startCommand(Command *cmd);
	Command::ExecuteResult startStartupSection();

	void pushReturnCommand(Command *);
	Command *popReturnCommand();
	Game &getGame();
	GameData &getGameData();
	Command *getMacro(const Common::String &name) const;
	Command *getExtra(const Common::String &name) const;
	bool isCommandRunning() const;

private:
	Game &_game;
	Command *_activeCommand;
	Common::Stack<Command *> _callStack;
	Script *_localScriptOverride;
};

class Script {
public:
	bool loadFromStream(Common::SeekableReadStream &stream);
	~Script();

	const ActionInfos &getActionInfos(ActionInfo::Action action);
	const Commands &getAllCommands() const;
	const Macros &getMacros() const;
	const Startups &getStartups() const;
	Command *getMacro(const Common::String &name) const;
	Command *getStartup(uint8 startupId) const;
	Command *getExtra(const Common::String &name) const;

private:
	void destroy();
	Commands _allCommands;
	ActionInfos _actionInfos[5];
	Macros _macros;
	Startups _startups;
	Extras _extras;
};

}

#endif
