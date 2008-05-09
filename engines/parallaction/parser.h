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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef PARALLACTION_PARSER_H
#define PARALLACTION_PARSER_H

#include "common/stream.h"
#include "parallaction/objects.h"
#include "parallaction/walk.h"

namespace Parallaction {

char   *parseNextToken(char *s, char *tok, uint16 count, const char *brk, bool ignoreQuotes = false);

#define MAX_TOKEN_LEN	50
extern char _tokens[][MAX_TOKEN_LEN];

class Script {

	Common::ReadStream *_input;
	bool	_disposeSource;
	uint	_line;				// for debug messages

	void clearTokens();
	uint16 fillTokens(char* line);

public:
	Script(Common::ReadStream *, bool _disposeSource = false);
	~Script();

	char *readLine(char *buf, size_t bufSize);
	uint16 readLineToken(bool errorOnEOF = false);

	void skip(const char* endToken);

	uint	getLine() { return _line; }
};



class Opcode {

public:
	virtual void operator()() const = 0;
	virtual ~Opcode() { }
};

template <class T>
class OpcodeImpl : public Opcode {

	typedef void (T::*Fn)();

	T*	_instance;
	Fn	_fn;

public:
	OpcodeImpl(T* instance, const Fn &fn) : _instance(instance), _fn(fn) { }

	void operator()() const {
		(_instance->*_fn)();
	}

};

typedef Common::Array<const Opcode*>	OpcodeSet;


class Parser {

public:
	Parser() { reset(); }
	~Parser() {}

	uint	_lookup;

	Common::Stack<OpcodeSet*>	_opcodes;
	Common::Stack<Table*>		_statements;

	OpcodeSet	*_currentOpcodes;
	Table		*_currentStatements;

	void	bind(Script *script);
	void	unbind();
	void	pushTables(OpcodeSet *opcodes, Table* statements);
	void	popTables();
	void	parseStatement();

protected:
	void	reset();

	Script	*_script;
};

#define DECLARE_UNQUALIFIED_ZONE_PARSER(sig) void locZoneParse_##sig()
#define DECLARE_UNQUALIFIED_ANIM_PARSER(sig) void locAnimParse_##sig()
#define DECLARE_UNQUALIFIED_COMMAND_PARSER(sig) void cmdParse_##sig()
#define DECLARE_UNQUALIFIED_LOCATION_PARSER(sig) void locParse_##sig()

#define MAX_FORWARDS	50

class Parallaction_ns;
class Parallaction_br;

class LocationParser_ns {

protected:
	Parallaction_ns*	_vm;
	Parser	*parser;

	Table		*_zoneTypeNames;
	Table		*_zoneFlagNames;

	// location parser
	OpcodeSet	_locationParsers;
	OpcodeSet	_locationZoneParsers;
	OpcodeSet	_locationAnimParsers;
	OpcodeSet	_commandParsers;
	Table		*_commandsNames;
	Table		*_locationStmt;
	Table		*_locationZoneStmt;
	Table		*_locationAnimStmt;

	struct LocationParserContext {
		bool		end;

		const char	*filename;
		Script		*script;
		ZonePtr		z;
		AnimationPtr	a;
		int			nextToken;
		CommandList *list;
		bool		endcommands;
		CommandPtr	cmd;

		// BRA specific
		int numZones;
		char *bgName;
		char *maskName;
		char *pathName;
	} _locParseCtxt;

	void warning_unexpected();

	DECLARE_UNQUALIFIED_LOCATION_PARSER(endlocation);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(location);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(disk);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(nodes);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(zone);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(animation);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(localflags);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(commands);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(acommands);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(flags);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(comment);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(endcomment);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(sound);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(music);
	DECLARE_UNQUALIFIED_ZONE_PARSER(limits);
	DECLARE_UNQUALIFIED_ZONE_PARSER(moveto);
	DECLARE_UNQUALIFIED_ZONE_PARSER(type);
	DECLARE_UNQUALIFIED_ZONE_PARSER(commands);
	DECLARE_UNQUALIFIED_ZONE_PARSER(label);
	DECLARE_UNQUALIFIED_ZONE_PARSER(flags);
	DECLARE_UNQUALIFIED_ZONE_PARSER(endzone);
	DECLARE_UNQUALIFIED_ZONE_PARSER(null);
	DECLARE_UNQUALIFIED_ANIM_PARSER(script);
	DECLARE_UNQUALIFIED_ANIM_PARSER(commands);
	DECLARE_UNQUALIFIED_ANIM_PARSER(type);
	DECLARE_UNQUALIFIED_ANIM_PARSER(label);
	DECLARE_UNQUALIFIED_ANIM_PARSER(flags);
	DECLARE_UNQUALIFIED_ANIM_PARSER(file);
	DECLARE_UNQUALIFIED_ANIM_PARSER(position);
	DECLARE_UNQUALIFIED_ANIM_PARSER(moveto);
	DECLARE_UNQUALIFIED_ANIM_PARSER(endanimation);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(flags);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(animation);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(zone);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(location);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(drop);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(call);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(simple);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(move);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(endcommands);

	virtual void parseGetData(Script &script, ZonePtr z);
	virtual void parseExamineData(Script &script, ZonePtr z);
	virtual void parseDoorData(Script &script, ZonePtr z);
	virtual void parseMergeData(Script &script, ZonePtr z);
	virtual void parseHearData(Script &script, ZonePtr z);
	virtual void parseSpeakData(Script &script, ZonePtr z);

	char		*parseComment(Script &script);
	char		*parseDialogueString(Script &script);
	Dialogue	*parseDialogue(Script &script);
	void		resolveDialogueForwards(Dialogue *dialogue, uint numQuestions, Table &forwards);
	Answer		*parseAnswer(Script &script);
	Question	*parseQuestion(Script &script);

	void		parseZone(Script &script, ZoneList &list, char *name);
	void		parseZoneTypeBlock(Script &script, ZonePtr z);
	void		parseWalkNodes(Script& script, WalkNodeList &list);
	void		parseAnimation(Script &script, AnimationList &list, char *name);
	void		parseCommands(Script &script, CommandList&);
	void		parseCommandFlags();
	void 		saveCommandForward(const char *name, CommandPtr cmd);
	void 		resolveCommandForwards();
	void		createCommand(uint id);
	void		addCommand();
	void		initParsers();

	struct CommandForwardReference {
		char		name[20];
		CommandPtr	cmd;
	} _forwardedCommands[MAX_FORWARDS];
	uint		_numForwardedCommands;

	void init();

public:
	LocationParser_ns(Parallaction_ns *vm) : _vm(vm) {
		init();
	}

	virtual ~LocationParser_ns() {
		delete _commandsNames;
		delete _locationStmt;
		delete _zoneTypeNames;
		delete _zoneFlagNames;
	}

	void parse(Script *script);

};

class LocationParser_br : public LocationParser_ns {

protected:
	Table		*_audioCommandsNames;

	Parallaction_br*	_vm;

	DECLARE_UNQUALIFIED_LOCATION_PARSER(location);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(zone);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(animation);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(localflags);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(flags);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(comment);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(endcomment);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(sound);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(music);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(redundant);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(ifchar);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(character);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(mask);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(path);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(escape);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(zeta);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(null);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(ifchar);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(endif);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(location);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(toggle);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(string);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(math);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(test);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(music);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(zeta);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(swap);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(give);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(text);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(unary);
	DECLARE_UNQUALIFIED_ZONE_PARSER(limits);
	DECLARE_UNQUALIFIED_ZONE_PARSER(moveto);
	DECLARE_UNQUALIFIED_ZONE_PARSER(type);
	DECLARE_UNQUALIFIED_ANIM_PARSER(file);
	DECLARE_UNQUALIFIED_ANIM_PARSER(position);
	DECLARE_UNQUALIFIED_ANIM_PARSER(moveto);
	DECLARE_UNQUALIFIED_ANIM_PARSER(endanimation);

	void init();

public:
	LocationParser_br(Parallaction_br *vm) : LocationParser_ns((Parallaction_ns*)vm), _vm(vm) {
		init();
	}

	virtual ~LocationParser_br() {
		delete _commandsNames;
		delete _locationStmt;
	}

	void parse(Script *script);

};


} // namespace Parallaction

#endif





