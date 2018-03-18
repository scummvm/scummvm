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

#include "common/array.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace Common {
class SeekableReadStream;
class String;
}

namespace MutationOfJB {

class Command;
class LabelCommand;
class GotoCommand;
class ConditionalCommand;
typedef Common::Array<Command *> Commands;


struct ActionInfo {
	enum Action {
		Walk,
		Talk,
		Look,
		Use
	};

	Action _action;
	Common::String _object1Name;
	Common::String _object2Name;
	bool _walkTo;
	Command *_command;
};

typedef Common::Array<ActionInfo> ActionInfos;
typedef Common::Array<GotoCommand *> GotoCommands;

class ScriptParseContext
{
public:
	ScriptParseContext(Common::SeekableReadStream &stream);
	bool readLine(Common::String &line);
	void addConditionalCommand(ConditionalCommand *command, char tag);
	void addLookSection(const Common::String & item, bool walkTo);

	Common::SeekableReadStream &_stream;
	Command *_currentCommand;
	Command *_lastCommand;

	struct ConditionalCommandInfo {
		ConditionalCommand *_command;
		char _tag;
	};
	typedef Common::Array<ConditionalCommandInfo> ConditionalCommandInfos;
	ConditionalCommandInfos _pendingCondCommands;

	typedef Common::HashMap<Common::String, LabelCommand *> LabelMap;
	LabelMap _labels;

	typedef Common::HashMap<Common::String, GotoCommands> PendingGotoMap;
	PendingGotoMap _pendingGotos;

	ActionInfos _actionInfos;

private:
};

class Script {
public:
	bool loadFromStream(Common::SeekableReadStream &stream);
	~Script();

	const ActionInfos &getLookActionInfos() const;
	const ActionInfos &getWalkActionInfos() const;
	const ActionInfos &getTalkActionInfos() const;
	const ActionInfos &getUseActionInfos() const;

private:
	void destroy();
	Commands _allCommands;
	ActionInfos _lookActionInfos;
	ActionInfos _walkActionInfos;
	ActionInfos _talkActionInfos;
	ActionInfos _useActionInfos;
};

}

#endif
