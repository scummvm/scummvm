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


#include "parallaction/parallaction.h"
#include "parallaction/sound.h"

namespace Parallaction {

#define CMD_SET				1
#define CMD_CLEAR			2
#define CMD_START			3
#define CMD_SPEAK			4
#define CMD_GET				5
#define CMD_LOCATION		6
#define CMD_OPEN			7
#define CMD_CLOSE			8
#define CMD_ON				9
#define CMD_OFF				10
#define CMD_CALL			11
#define CMD_TOGGLE			12
#define CMD_DROP			13
#define CMD_QUIT			14
#define CMD_MOVE			15
#define CMD_STOP			16

#define INST_ON				1
#define INST_OFF			2
#define INST_X				3
#define INST_Y				4
#define INST_Z				5
#define INST_F				6
#define INST_LOOP			7
#define INST_ENDLOOP		8
#define INST_SHOW			9
#define INST_INC			10
#define INST_DEC			11
#define INST_SET			12
#define INST_PUT			13
#define INST_CALL			14
#define INST_WAIT			15
#define INST_START			16
#define INST_SOUND			17
#define INST_MOVE			18
#define INST_END			19


const char *_zoneFlagNamesRes_ns[] = {
	"closed",
	"active",
	"remove",
	"acting",
	"locked",
	"fixed",
	"noname",
	"nomasked",
	"looping",
	"added",
	"character",
	"nowalk"
};

const char *_zoneTypeNamesRes_ns[] = {
	"examine",
	"door",
	"get",
	"merge",
	"taste",
	"hear",
	"feel",
	"speak",
	"none",
	"trap",
	"yourself",
	"Command"
};

const char *_commandsNamesRes_ns[] = {
	"set",
	"clear",
	"start",
	"speak",
	"get",
	"location",
	"open",
	"close",
	"on",
	"off",
	"call",
	"toggle",
	"drop",
	"quit",
	"move",
	"stop",
	"endcommands",
	"endzone"
};

const char *_locationStmtRes_ns[] = {
	"endlocation",
	"location",
	"disk",
	"nodes",
	"zone",
	"animation",
	"localflags",
	"commands",
	"acommands",
	"flags",
	"comment",
	"endcomment",
	"sound",
	"music"
};

const char *_locationZoneStmtRes_ns[] = {
	"limits",
	"moveto",
	"type",
	"commands",
	"label",
	"flags",
	"endzone"
};

const char *_locationAnimStmtRes_ns[] = {
	"script",
	"commands",
	"type",
	"label",
	"flags",
	"file",
	"position",
	"moveto",
	"endanimation"
};

const char *_instructionNamesRes_ns[] = {
	"on",
	"off",
	"x",
	"y",
	"z",
	"f",
	"loop",
	"endloop",
	"show",
	"inc",
	"dec",
	"set",
	"put",
	"call",
	"wait",
	"start",
	"sound",
	"move",
	"endscript"
};


#define DECLARE_ZONE_PARSER(sig) void LocationParser_ns::locZoneParse_##sig()
#define DECLARE_ANIM_PARSER(sig) void LocationParser_ns::locAnimParse_##sig()
#define DECLARE_COMMAND_PARSER(sig) void LocationParser_ns::cmdParse_##sig()
#define DECLARE_LOCATION_PARSER(sig) void LocationParser_ns::locParse_##sig()

#define DECLARE_INSTRUCTION_PARSER(sig) void ProgramParser_ns::instParse_##sig()


void LocationParser_ns::warning_unexpected() {
	debugC(1, kDebugParser, "unexpected keyword '%s' in line %i", _tokens[0], _script->getLine());
}


DECLARE_ANIM_PARSER(script)  {
	debugC(7, kDebugParser, "ANIM_PARSER(script) ");

	ctxt.a->_scriptName = strdup(_tokens[1]);
}


DECLARE_ANIM_PARSER(commands)  {
	debugC(7, kDebugParser, "ANIM_PARSER(commands) ");

	 parseCommands(ctxt.a->_commands);
}


DECLARE_ANIM_PARSER(type)  {
	debugC(7, kDebugParser, "ANIM_PARSER(type) ");

	if (_tokens[2][0] != '\0') {
		ctxt.a->_type = ((4 + _vm->_objectsNames->lookup(_tokens[2])) << 16) & 0xFFFF0000;
	}
	int16 _si = _zoneTypeNames->lookup(_tokens[1]);
	if (_si != Table::notFound) {
		ctxt.a->_type |= 1 << (_si-1);
		if (((ctxt.a->_type & 0xFFFF) != kZoneNone) && ((ctxt.a->_type & 0xFFFF) != kZoneCommand)) {
			parseZoneTypeBlock(ctxt.a);
		}
	}

	ctxt.a->_oldPos.x = -1000;
	ctxt.a->_oldPos.y = -1000;

	ctxt.a->_flags |= 0x1000000;

	_parser->popTables();
}


DECLARE_ANIM_PARSER(label)  {
	debugC(7, kDebugParser, "ANIM_PARSER(label) ");

	ctxt.a->_label = _vm->_gfx->renderFloatingLabel(_vm->_labelFont, _tokens[1]);
}


DECLARE_ANIM_PARSER(flags)  {
	debugC(7, kDebugParser, "ANIM_PARSER(flags) ");

	uint16 _si = 1;

	do {
		byte _al = _zoneFlagNames->lookup(_tokens[_si]);
		_si++;
		ctxt.a->_flags |= 1 << (_al - 1);
	} while (!scumm_stricmp(_tokens[_si++], "|"));
}


DECLARE_ANIM_PARSER(file)  {
	debugC(7, kDebugParser, "ANIM_PARSER(file) ");

	char vC8[200];
	strcpy(vC8, _tokens[1]);
	if (_engineFlags & kEngineTransformedDonna) {
		if (!scumm_stricmp(_tokens[1], "donnap") || !scumm_stricmp(_tokens[1], "donnapa")) {
			strcat(vC8, "tras");
		}
	}
	ctxt.a->gfxobj = _vm->_gfx->loadAnim(vC8);
}


DECLARE_ANIM_PARSER(position)  {
	debugC(7, kDebugParser, "ANIM_PARSER(position) ");

	ctxt.a->_left = atoi(_tokens[1]);
	ctxt.a->_top = atoi(_tokens[2]);
	ctxt.a->_z = atoi(_tokens[3]);
}


DECLARE_ANIM_PARSER(moveto)  {
	debugC(7, kDebugParser, "ANIM_PARSER(moveto) ");

	ctxt.a->_moveTo.x = atoi(_tokens[1]);
	ctxt.a->_moveTo.y = atoi(_tokens[2]);
}


DECLARE_ANIM_PARSER(endanimation)  {
	debugC(7, kDebugParser, "ANIM_PARSER(endanimation) ");


	ctxt.a->_oldPos.x = -1000;
	ctxt.a->_oldPos.y = -1000;

	ctxt.a->_flags |= 0x1000000;

	_parser->popTables();
}

void LocationParser_ns::parseAnimation(AnimationList &list, char *name) {
	debugC(5, kDebugParser, "parseAnimation(name: %s)", name);

	AnimationPtr a(new Animation);

	strncpy(a->_name, name, ZONENAME_LENGTH);

	list.push_front(AnimationPtr(a));

	ctxt.a = a;
	_parser->pushTables(&_locationAnimParsers, _locationAnimStmt);
}

void ProgramParser_ns::parseInstruction() {

	_script->readLineToken(true);

	if (_tokens[0][1] == '.') {
		_tokens[0][1] = '\0';
		ctxt.a = _vm->findAnimation(&_tokens[0][2]);
	} else
	if (_tokens[1][1] == '.') {
		_tokens[1][1] = '\0';
		ctxt.a = _vm->findAnimation(&_tokens[1][2]);
	} else
		ctxt.a = _program->_anim;

	if (!ctxt.a) {
		return;
	}

	InstructionPtr inst(new Instruction);
	ctxt.inst = inst;
	_parser->parseStatement();
	_program->_instructions.push_back(inst);

	return;
}

void ProgramParser_ns::parse(Script *script, ProgramPtr program) {

	_script = script;
	_program = program;

	ctxt.openIf = nullInstructionPtr;
	ctxt.end = false;
	ctxt.locals = program->_locals;

	_parser->reset();
	_parser->pushTables(&_instructionParsers, _instructionNames);
	do {
		parseInstruction();
	} while (!ctxt.end);
	_parser->popTables();

	program->_ip = program->_instructions.begin();
}

void Parallaction_ns::loadProgram(AnimationPtr a, const char *filename) {
	debugC(1, kDebugParser, "loadProgram(Animation: %s, script: %s)", a->_name, filename);

	Script *script = _disk->loadScript(filename);
	ProgramPtr program(new Program);
	program->_anim = a;

	_programParser->parse(script, program);

	delete script;

	_vm->_location._programs.push_back(program);

	debugC(1, kDebugParser, "loadProgram() done");

	return;
}

DECLARE_INSTRUCTION_PARSER(animation)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(animation) ");

	if (!scumm_stricmp(_tokens[1], ctxt.a->_name)) {
		ctxt.inst->_a = ctxt.a;
	} else {
		ctxt.inst->_a = _vm->findAnimation(_tokens[1]);
	}

	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(loop)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(loop) ");

	parseRValue(ctxt.inst->_opB, _tokens[1]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(x)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(x) ");

	parseLValue(ctxt.inst->_opA, "X");
	parseRValue(ctxt.inst->_opB, _tokens[1]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(y)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(y) ");

	parseLValue(ctxt.inst->_opA, "Y");
	parseRValue(ctxt.inst->_opB, _tokens[1]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(z)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(z) ");

	parseLValue(ctxt.inst->_opA, "Z");
	parseRValue(ctxt.inst->_opB, _tokens[1]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(f)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(f) ");

	parseLValue(ctxt.inst->_opA, "F");
	parseRValue(ctxt.inst->_opB, _tokens[1]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(inc)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(inc) ");

	parseLValue(ctxt.inst->_opA, _tokens[1]);
	parseRValue(ctxt.inst->_opB, _tokens[2]);

	if (!scumm_stricmp(_tokens[3], "mod")) {
		ctxt.inst->_flags |= kInstMod;
	}
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(set)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(set) ");

	// WORKAROUND: At least one script (balzo.script) in Amiga versions didn't declare
	//	local variables before using them, thus leading to crashes. The line launching the
	// script was commented out on Dos version. This workaround enables the engine
	// to dynamically add a local variable when it is encountered the first time in
	// the script, so should fix any other occurrence as well.
	if (_program->findLocal(_tokens[1]) == -1) {
		_program->addLocal(_tokens[1]);
	}

	parseLValue(ctxt.inst->_opA, _tokens[1]);
	parseRValue(ctxt.inst->_opB, _tokens[2]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(move)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(move) ");

	parseRValue(ctxt.inst->_opA, _tokens[1]);
	parseRValue(ctxt.inst->_opB, _tokens[2]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(put)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(put) ");

	if (!scumm_stricmp(_tokens[1], ctxt.a->_name)) {
		ctxt.inst->_a = ctxt.a;
	} else {
		ctxt.inst->_a = _vm->findAnimation(_tokens[1]);
	}

	parseRValue(ctxt.inst->_opA, _tokens[2]);
	parseRValue(ctxt.inst->_opB, _tokens[3]);
	if (!scumm_stricmp(_tokens[4], "masked")) {
		ctxt.inst->_flags |= kInstMaskedPut;
	}
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(call)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(call) ");

	int index = _vm->_callableNames->lookup(_tokens[1]);
	if (index == Table::notFound)
		error("unknown callable '%s'", _tokens[1]);
	ctxt.inst->_immediate = index - 1;
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(sound)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(sound) ");

	ctxt.inst->_z = _vm->findZone(_tokens[1]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(null)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(null) ");
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(defLocal)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(defLocal) ");

	int16 val = atoi(_tokens[2]);
	int16 index;

	if (_tokens[3][0] != '\0') {
		index = _program->addLocal(_tokens[0], val, atoi(_tokens[3]), atoi(_tokens[4]));
	} else {
		index = _program->addLocal(_tokens[0], val);
	}

	ctxt.inst->_opA.setLocal(&ctxt.locals[index]);
	ctxt.inst->_opB.setImmediate(ctxt.locals[index]._value);

	ctxt.inst->_index = INST_SET;
}

DECLARE_INSTRUCTION_PARSER(endscript)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(endscript) ");

	ctxt.end = true;
	ctxt.inst->_index = _parser->_lookup;
}



void ProgramParser_ns::parseRValue(ScriptVar &v, const char *str) {

	if (isdigit(str[0]) || str[0] == '-') {
		v.setImmediate(atoi(str));
		return;
	}

	int index = _program->findLocal(str);
	if (index != -1) {
		v.setLocal(&ctxt.locals[index]);
		return;
	}

	AnimationPtr a;
	if (str[1] == '.') {
		a = _vm->findAnimation(&str[2]);
	} else {
		a = ctxt.a;
	}

	if (str[0] == 'X') {
		v.setField(&a->_left);
	} else
	if (str[0] == 'Y') {
		v.setField(&a->_top);
	} else
	if (str[0] == 'Z') {
		v.setField(&a->_z);
	} else
	if (str[0] == 'F') {
		v.setField(&a->_frame);
	}

}

void ProgramParser_ns::parseLValue(ScriptVar &v, const char *str) {

	int index = _program->findLocal(str);
	if (index != -1) {
		v.setLocal(&ctxt.locals[index]);
		return;
	}

	AnimationPtr a;
	if (str[1] == '.') {
		a = _vm->findAnimation(&str[2]);
	} else {
		a = ctxt.a;
	}

	if (str[0] == 'X') {
		v.setField(&a->_left);
	} else
	if (str[0] == 'Y') {
		v.setField(&a->_top);
	} else
	if (str[0] == 'Z') {
		v.setField(&a->_z);
	} else
	if (str[0] == 'F') {
		v.setField(&a->_frame);
	}

}


DECLARE_COMMAND_PARSER(flags)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(flags) ");

	createCommand(_parser->_lookup);

	if (_vm->_globalTable->lookup(_tokens[1]) == Table::notFound) {
		do {
			char _al = _vm->_localFlagNames->lookup(_tokens[ctxt.nextToken]);
			ctxt.nextToken++;
			ctxt.cmd->u._flags |= 1 << (_al - 1);
		} while (!scumm_stricmp(_tokens[ctxt.nextToken++], "|"));
		ctxt.nextToken--;
	} else {
		ctxt.cmd->u._flags |= kFlagsGlobal;
		do {
			char _al = _vm->_globalTable->lookup(_tokens[1]);
			ctxt.nextToken++;
			ctxt.cmd->u._flags |= 1 << (_al - 1);
		} while (!scumm_stricmp(_tokens[ctxt.nextToken++], "|"));
		ctxt.nextToken--;
	}

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(zone)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(zone) ");

	createCommand(_parser->_lookup);

	ctxt.cmd->u._zone = _vm->findZone(_tokens[ctxt.nextToken]);
	if (!ctxt.cmd->u._zone) {
		saveCommandForward(_tokens[ctxt.nextToken], ctxt.cmd);
	}
	ctxt.nextToken++;

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(location)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(location) ");

	createCommand(_parser->_lookup);

	ctxt.cmd->u._string = strdup(_tokens[ctxt.nextToken]);
	ctxt.nextToken++;

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(drop)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(drop) ");

	createCommand(_parser->_lookup);

	ctxt.cmd->u._object = 4 + _vm->_objectsNames->lookup(_tokens[ctxt.nextToken]);
	ctxt.nextToken++;

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(call)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(call) ");

	createCommand(_parser->_lookup);

	ctxt.cmd->u._callable = _vm->_callableNames->lookup(_tokens[ctxt.nextToken]) - 1;
	ctxt.nextToken++;

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(simple)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(simple) ");

	createCommand(_parser->_lookup);
	addCommand();
}


DECLARE_COMMAND_PARSER(move)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(move) ");

	createCommand(_parser->_lookup);

	ctxt.cmd->u._move.x = atoi(_tokens[ctxt.nextToken]);
	ctxt.nextToken++;
	ctxt.cmd->u._move.y = atoi(_tokens[ctxt.nextToken]);
	ctxt.nextToken++;

	parseCommandFlags();
	addCommand();
}

DECLARE_COMMAND_PARSER(endcommands)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(endcommands) ");

	_parser->popTables();

	// temporary trick to handle dialogue commands
	ctxt.endcommands = true;
}

void LocationParser_ns::parseCommandFlags() {

	int _si = ctxt.nextToken;
	CommandPtr cmd = ctxt.cmd;

	if (!scumm_stricmp(_tokens[_si], "flags")) {
		_si++;

		do {
			if (!scumm_stricmp(_tokens[_si], "exit") || !scumm_stricmp(_tokens[_si], "exittrap")) {
				cmd->_flagsOn |= kFlagsExit;
			} else
			if (!scumm_stricmp(_tokens[_si], "enter") || !scumm_stricmp(_tokens[_si], "entertrap")) {
				cmd->_flagsOn |= kFlagsEnter;
			} else
			if (!scumm_strnicmp(_tokens[_si], "no", 2)) {
				byte _al = _vm->_localFlagNames->lookup(&_tokens[_si][2]);
				assert(_al != Table::notFound);
				cmd->_flagsOff |= 1 << (_al - 1);
			} else {
				byte _al = _vm->_localFlagNames->lookup(_tokens[_si]);
				assert(_al != Table::notFound);
				cmd->_flagsOn |= 1 << (_al - 1);
			}

			_si++;

		} while (!scumm_stricmp(_tokens[_si++], "|"));

	}

	if (!scumm_stricmp(_tokens[_si], "gflags")) {
		_si++;
		cmd->_flagsOn |= kFlagsGlobal;

		do {
			if (!scumm_stricmp(_tokens[_si], "exit")) {
				cmd->_flagsOn |= kFlagsExit;
			} else
			if (!scumm_stricmp(_tokens[_si], "enter")) {
				cmd->_flagsOn |= kFlagsEnter;
			} else
			if (!scumm_strnicmp(_tokens[_si], "no", 2)) {
				byte _al = _vm->_globalTable->lookup(&_tokens[_si][2]);
				assert(_al != Table::notFound);
				cmd->_flagsOff |= 1 << (_al - 1);
			} else {
				byte _al = _vm->_globalTable->lookup(_tokens[_si]);
				assert(_al != Table::notFound);
				cmd->_flagsOn |= 1 << (_al - 1);
			}

			_si++;

		} while (!scumm_stricmp(_tokens[_si++], "|"));

	}

	_si = ctxt.nextToken;

}

void LocationParser_ns::addCommand() {
	ctxt.list->push_front(ctxt.cmd);	// NOTE: command lists are written backwards in scripts
}

void LocationParser_ns::createCommand(uint id) {

	ctxt.nextToken = 1;
	ctxt.cmd = CommandPtr(new Command);
	ctxt.cmd->_id = id;

}

void LocationParser_ns::saveCommandForward(const char *name, CommandPtr cmd) {
	assert(_numForwardedCommands < MAX_FORWARDS);

	strcpy(_forwardedCommands[_numForwardedCommands].name, name);
	_forwardedCommands[_numForwardedCommands].cmd = cmd;

	_numForwardedCommands++;
}

void LocationParser_ns::resolveCommandForwards() {
	for (uint i = 0; i < _numForwardedCommands; i++) {
		_forwardedCommands[i].cmd->u._zone = _vm->findZone(_forwardedCommands[i].name);
		if (_forwardedCommands[i].cmd->u._zone == 0) {
			warning("Cannot find zone '%s' into current location script. This may be a bug in the original scripts.\n", _forwardedCommands[i].name);
		}
	}
	_numForwardedCommands = 0;
}

void LocationParser_ns::parseCommands(CommandList& list) {
	debugC(5, kDebugParser, "parseCommands()");

	ctxt.list = &list;
	ctxt.endcommands = false;

	_parser->pushTables(&_commandParsers, _commandsNames);
}

Dialogue *LocationParser_ns::parseDialogue() {
	debugC(7, kDebugParser, "parseDialogue()");

	uint16 numQuestions = 0;

	Dialogue *dialogue = new Dialogue;
	assert(dialogue);

	Table forwards(20);

	_script->readLineToken(true);

	while (scumm_stricmp(_tokens[0], "enddialogue")) {
		if (scumm_stricmp(_tokens[0], "Question")) continue;

		forwards.addData(_tokens[1]);

		dialogue->_questions[numQuestions++] = parseQuestion();

		_script->readLineToken(true);
	}

	resolveDialogueForwards(dialogue, numQuestions, forwards);

	debugC(7, kDebugParser, "parseDialogue() done");

	return dialogue;
}

Question *LocationParser_ns::parseQuestion() {

	Question *question = new Question;
	assert(question);

	question->_text = parseDialogueString();

	_script->readLineToken(true);
	question->_mood = atoi(_tokens[0]);

	uint16 numAnswers = 0;

	_script->readLineToken(true);
	while (scumm_stricmp(_tokens[0], "endquestion")) {	// parse answers
		question->_answers[numAnswers] = parseAnswer();
		numAnswers++;
	}

	return question;
}

Answer *LocationParser_ns::parseAnswer() {

	Answer *answer = new Answer;
	assert(answer);

	if (_tokens[1][0]) {

		Table* flagNames;
		uint16 token;

		if (!scumm_stricmp(_tokens[1], "global")) {
			token = 2;
			flagNames = _vm->_globalTable;
			answer->_yesFlags |= kFlagsGlobal;
		} else {
			token = 1;
			flagNames = _vm->_localFlagNames;
		}

		do {

			if (!scumm_strnicmp(_tokens[token], "no", 2)) {
				byte _al = flagNames->lookup(_tokens[token]+2);
				answer->_noFlags |= 1 << (_al - 1);
			} else {
				byte _al = flagNames->lookup(_tokens[token]);
				answer->_yesFlags |= 1 << (_al - 1);
			}

			token++;

		} while (!scumm_stricmp(_tokens[token++], "|"));

	}

	answer->_text = parseDialogueString();

	_script->readLineToken(true);
	answer->_mood = atoi(_tokens[0]);
	answer->_following._name = parseDialogueString();

	_script->readLineToken(true);
	if (!scumm_stricmp(_tokens[0], "commands")) {

		parseCommands(answer->_commands);
		ctxt.endcommands = false;
		do {
			_script->readLineToken(true);
			_parser->parseStatement();
		} while (!ctxt.endcommands);

		_script->readLineToken(true);
	}

	return answer;
}

void LocationParser_ns::resolveDialogueForwards(Dialogue *dialogue, uint numQuestions, Table &forwards) {

	for (uint16 i = 0; i < numQuestions; i++) {
		Question *question = dialogue->_questions[i];

		for (uint16 j = 0; j < NUM_ANSWERS; j++) {
			Answer *answer = question->_answers[j];
			if (answer == 0) continue;

			int16 index = forwards.lookup(answer->_following._name);
			free(answer->_following._name);
			answer->_following._name = 0;

			if (index == Table::notFound)
				answer->_following._question = 0;
			else
				answer->_following._question = dialogue->_questions[index - 1];

		}
	}

}

char *LocationParser_ns::parseDialogueString() {

	char vC8[200];
	char *vD0 = NULL;
	do {

		vD0 = _script->readLine(vC8, 200);
		if (vD0 == 0) return NULL;

		vD0 = Common::ltrim(vD0);

	} while (strlen(vD0) == 0);

	vD0[strlen(vD0)-1] = '\0';	// deletes the trailing '0xA'
								// this is critical for Gfx::displayWrappedString to work properly
	return strdup(vD0);
}


DECLARE_LOCATION_PARSER(endlocation)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(endlocation) ");

	ctxt.end = true;
}


DECLARE_LOCATION_PARSER(location)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(location) ");

	// The parameter for location is 'location.mask'.
	// If mask is not present, then it is assumed
	// that path & mask are encoded in the background
	// bitmap, otherwise a separate .msk file exists.

	char *mask = strchr(_tokens[1], '.');
	if (mask) {
		mask[0] = '\0';
		mask++;
	}

	strcpy(_vm->_location._name, _tokens[1]);
	_vm->switchBackground(_vm->_location._name, mask);

	if (_tokens[2][0] != '\0') {
		_vm->_char._ani->_left = atoi(_tokens[2]);
		_vm->_char._ani->_top = atoi(_tokens[3]);
	}

	if (_tokens[4][0] != '\0') {
		_vm->_char._ani->_frame = atoi(_tokens[4]);
	}
}


DECLARE_LOCATION_PARSER(disk)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(disk) ");

	_vm->_disk->selectArchive(_tokens[1]);
}


DECLARE_LOCATION_PARSER(nodes)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(nodes) ");

	parseWalkNodes(_vm->_location._walkNodes);
}


DECLARE_LOCATION_PARSER(zone)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(zone) ");

	parseZone(_vm->_location._zones, _tokens[1]);
}


DECLARE_LOCATION_PARSER(animation)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(animation) ");

	parseAnimation(_vm->_location._animations, _tokens[1]);
}


DECLARE_LOCATION_PARSER(localflags)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(localflags) ");

	int _si = 1;
	while (_tokens[_si][0] != '\0') {
		_vm->_localFlagNames->addData(_tokens[_si]);
		_si++;
	}
}


DECLARE_LOCATION_PARSER(commands)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(commands) ");

	parseCommands(_vm->_location._commands);
}


DECLARE_LOCATION_PARSER(acommands)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(acommands) ");

	parseCommands(_vm->_location._aCommands);
}


DECLARE_LOCATION_PARSER(flags)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(flags) ");

	if ((_vm->getLocationFlags() & kFlagsVisited) == 0) {
		// only for 1st visit
		_vm->clearLocationFlags((uint32)kFlagsAll);
		int _si = 1;

		do {
			byte _al = _vm->_localFlagNames->lookup(_tokens[_si]);
			_vm->setLocationFlags(1 << (_al - 1));

			_si++;
			if (scumm_stricmp(_tokens[_si], "|")) break;
			_si++;
		} while (true);
	}
}


DECLARE_LOCATION_PARSER(comment)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(comment) ");

	_vm->_location._comment = parseComment();
}


DECLARE_LOCATION_PARSER(endcomment)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(endcomment) ");

	_vm->_location._endComment = parseComment();
}


DECLARE_LOCATION_PARSER(sound)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(sound) ");

	if (_vm->getPlatform() == Common::kPlatformAmiga) {
		strcpy(_vm->_location._soundFile, _tokens[1]);
		_vm->_location._hasSound = true;
	}
}


DECLARE_LOCATION_PARSER(music)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(music) ");

	if (_vm->getPlatform() == Common::kPlatformAmiga)
		_vm->_soundMan->setMusicFile(_tokens[1]);
}

void LocationParser_ns::parse(Script *script) {

	_numForwardedCommands = 0;

	ctxt.end = false;
	_script = script;
	ctxt.filename = 0;//filename;

	_parser->reset();
	_parser->pushTables(&_locationParsers, _locationStmt);
	do {
		_script->readLineToken(true);
		_parser->parseStatement();
	} while (!ctxt.end);
	_parser->popTables();

	resolveCommandForwards();
}

void LocationParser_ns::parseWalkNodes(WalkNodeList &list) {
	debugC(5, kDebugParser, "parseWalkNodes()");

	_script->readLineToken(true);
	while (scumm_stricmp(_tokens[0], "ENDNODES")) {

		if (!scumm_stricmp(_tokens[0], "COORD")) {

			WalkNodePtr v4(new WalkNode(
				atoi(_tokens[1]),
				atoi(_tokens[2])
			));

			list.push_front(v4);
		}

		_script->readLineToken(true);
	}

	debugC(5, kDebugParser, "parseWalkNodes() done");

	return;
}
/*
typedef OpcodeImpl<ProgramParser_ns> OpcodeV1;
#define INSTRUCTION_PARSER(sig) OpcodeV1(this, &ProgramParser_ns::instParse_##sig)

typedef OpcodeImpl<LocationParser_ns> OpcodeV2;
#define ZONE_PARSER(sig)		OpcodeV2(this, &LocationParser_ns::locZoneParse_##sig)
#define ANIM_PARSER(sig)		OpcodeV2(this, &LocationParser_ns::locAnimParse_##sig)
#define LOCATION_PARSER(sig)	OpcodeV2(this, &LocationParser_ns::locParse_##sig)
#define COMMAND_PARSER(sig)		OpcodeV2(this, &LocationParser_ns::cmdParse_##sig)

#define WARNING_PARSER(sig)		OpcodeV2(this, &LocationParser_br::warning_##sig)
*/

#define SetOpcodeTable(x) table = &x;


typedef Common::Functor0Mem<void, ProgramParser_ns> OpcodeV1;
#define SetOpcodeTable(x) table = &x;
#define INSTRUCTION_PARSER(sig) table->push_back(new OpcodeV1(this, &ProgramParser_ns::instParse_##sig))

typedef Common::Functor0Mem<void, LocationParser_ns> OpcodeV2;
#define ZONE_PARSER(sig)		table->push_back(new OpcodeV2(this, &LocationParser_ns::locZoneParse_##sig))
#define ANIM_PARSER(sig)		table->push_back(new OpcodeV2(this, &LocationParser_ns::locAnimParse_##sig))
#define LOCATION_PARSER(sig)	table->push_back(new OpcodeV2(this, &LocationParser_ns::locParse_##sig))
#define COMMAND_PARSER(sig)		table->push_back(new OpcodeV2(this, &LocationParser_ns::cmdParse_##sig))

#define WARNING_PARSER(sig)		table->push_back(new OpcodeV2(this, &LocationParser_br::warning_##sig))


void LocationParser_ns::init() {

	_parser = new Parser;

	_zoneFlagNames = new Table(ARRAYSIZE(_zoneFlagNamesRes_ns), _zoneFlagNamesRes_ns);
	_zoneTypeNames = new Table(ARRAYSIZE(_zoneTypeNamesRes_ns), _zoneTypeNamesRes_ns);
	_commandsNames = new Table(ARRAYSIZE(_commandsNamesRes_ns), _commandsNamesRes_ns);
	_locationStmt = new Table(ARRAYSIZE(_locationStmtRes_ns), _locationStmtRes_ns);
	_locationZoneStmt = new Table(ARRAYSIZE(_locationZoneStmtRes_ns), _locationZoneStmtRes_ns);
	_locationAnimStmt = new Table(ARRAYSIZE(_locationAnimStmtRes_ns), _locationAnimStmtRes_ns);

	Common::Array<const Opcode*> *table = 0;

	SetOpcodeTable(_commandParsers);
	WARNING_PARSER(unexpected);
	COMMAND_PARSER(flags);			// set
	COMMAND_PARSER(flags);			// clear
	COMMAND_PARSER(zone);		// start
	COMMAND_PARSER(zone);			// speak
	COMMAND_PARSER(zone);			// get
	COMMAND_PARSER(location);		// location
	COMMAND_PARSER(zone);			// open
	COMMAND_PARSER(zone);			// close
	COMMAND_PARSER(zone);			// on
	COMMAND_PARSER(zone);			// off
	COMMAND_PARSER(call);			// call
	COMMAND_PARSER(flags);			// toggle
	COMMAND_PARSER(drop);			// drop
	COMMAND_PARSER(simple);			// quit
	COMMAND_PARSER(move);			// move
	COMMAND_PARSER(zone);		// stop
	COMMAND_PARSER(endcommands);	// endcommands
	COMMAND_PARSER(endcommands);		// endzone

	SetOpcodeTable(_locationParsers);
	WARNING_PARSER(unexpected);
	LOCATION_PARSER(endlocation);
	LOCATION_PARSER(location);
	LOCATION_PARSER(disk);
	LOCATION_PARSER(nodes);
	LOCATION_PARSER(zone);
	LOCATION_PARSER(animation);
	LOCATION_PARSER(localflags);
	LOCATION_PARSER(commands);
	LOCATION_PARSER(acommands);
	LOCATION_PARSER(flags);
	LOCATION_PARSER(comment);
	LOCATION_PARSER(endcomment);
	LOCATION_PARSER(sound);
	LOCATION_PARSER(music);

	SetOpcodeTable(_locationZoneParsers);
	WARNING_PARSER(unexpected);
	ZONE_PARSER(limits);
	ZONE_PARSER(moveto);
	ZONE_PARSER(type);
	ZONE_PARSER(commands);
	ZONE_PARSER(label);
	ZONE_PARSER(flags);
	ZONE_PARSER(endzone);

	SetOpcodeTable(_locationAnimParsers);
	WARNING_PARSER(unexpected);
	ANIM_PARSER(script);
	ANIM_PARSER(commands);
	ANIM_PARSER(type);
	ANIM_PARSER(label);
	ANIM_PARSER(flags);
	ANIM_PARSER(file);
	ANIM_PARSER(position);
	ANIM_PARSER(moveto);
	ANIM_PARSER(endanimation);

}

void ProgramParser_ns::init() {

	_parser = new Parser;

	_instructionNames = new Table(ARRAYSIZE(_instructionNamesRes_ns), _instructionNamesRes_ns);

	Common::Array<const Opcode*> *table = 0;
	SetOpcodeTable(_instructionParsers);
	INSTRUCTION_PARSER(defLocal);	// invalid opcode -> local definition
	INSTRUCTION_PARSER(animation);	// on
	INSTRUCTION_PARSER(animation);	// off
	INSTRUCTION_PARSER(x);
	INSTRUCTION_PARSER(y);
	INSTRUCTION_PARSER(z);
	INSTRUCTION_PARSER(f);
	INSTRUCTION_PARSER(loop);
	INSTRUCTION_PARSER(null);		// endloop
	INSTRUCTION_PARSER(null);		// show
	INSTRUCTION_PARSER(inc);
	INSTRUCTION_PARSER(inc);		// dec
	INSTRUCTION_PARSER(set);
	INSTRUCTION_PARSER(put);
	INSTRUCTION_PARSER(call);
	INSTRUCTION_PARSER(null);		// wait
	INSTRUCTION_PARSER(animation);	// start
	INSTRUCTION_PARSER(sound);
	INSTRUCTION_PARSER(move);
	INSTRUCTION_PARSER(endscript);
}

//
//	a comment can appear both at location and Zone levels
//	comments are displayed into rectangles on the screen
//
char *LocationParser_ns::parseComment() {

	char			_tmp_comment[1000] = "\0";
	char *v194;

	do {
		char v190[400];
		v194 = _script->readLine(v190, 400);

		v194[strlen(v194)-1] = '\0';
		if (!scumm_stricmp(v194, "endtext"))
			break;

		strcat(_tmp_comment, v194);
		strcat(_tmp_comment, " ");
	} while (true);

	v194 = strdup(_tmp_comment);
	_tmp_comment[0] = '\0';

	return v194;
}

DECLARE_ZONE_PARSER(null) {
	debugC(7, kDebugParser, "ZONE_PARSER(null) ");
}


DECLARE_ZONE_PARSER(endzone)  {
	debugC(7, kDebugParser, "ZONE_PARSER(endzone) ");

	_parser->popTables();
}

DECLARE_ZONE_PARSER(limits)  {
	debugC(7, kDebugParser, "ZONE_PARSER(limits) ");

	ctxt.z->_left = atoi(_tokens[1]);
	ctxt.z->_top = atoi(_tokens[2]);
	ctxt.z->_right = atoi(_tokens[3]);
	ctxt.z->_bottom = atoi(_tokens[4]);
}


DECLARE_ZONE_PARSER(moveto)  {
	debugC(7, kDebugParser, "ZONE_PARSER(moveto) ");

	ctxt.z->_moveTo.x = atoi(_tokens[1]);
	ctxt.z->_moveTo.y = atoi(_tokens[2]);
}


DECLARE_ZONE_PARSER(type)  {
	debugC(7, kDebugParser, "ZONE_PARSER(type) ");

	if (_tokens[2][0] != '\0') {
		ctxt.z->_type = (4 + _vm->_objectsNames->lookup(_tokens[2])) << 16;
	}
	int16 _si = _zoneTypeNames->lookup(_tokens[1]);
	if (_si != Table::notFound) {
		ctxt.z->_type |= 1 << (_si - 1);
		parseZoneTypeBlock(ctxt.z);
	}

	_parser->popTables();
}


DECLARE_ZONE_PARSER(commands)  {
	debugC(7, kDebugParser, "ZONE_PARSER(commands) ");

	 parseCommands(ctxt.z->_commands);
}


DECLARE_ZONE_PARSER(label)  {
	debugC(7, kDebugParser, "ZONE_PARSER(label) ");

//			printf("label: %s", _tokens[1]);
	ctxt.z->_label = _vm->_gfx->renderFloatingLabel(_vm->_labelFont, _tokens[1]);
}


DECLARE_ZONE_PARSER(flags)  {
	debugC(7, kDebugParser, "ZONE_PARSER(flags) ");

	uint16 _si = 1;

	do {
		char _al = _zoneFlagNames->lookup(_tokens[_si]);
		_si++;
		ctxt.z->_flags |= 1 << (_al - 1);
	} while (!scumm_stricmp(_tokens[_si++], "|"));
}

void LocationParser_ns::parseZone(ZoneList &list, char *name) {
	debugC(5, kDebugParser, "parseZone(name: %s)", name);

	if (_vm->findZone(name)) {
		_script->skip("endzone");
		return;
	}

	ZonePtr z(new Zone);

	strncpy(z->_name, name, ZONENAME_LENGTH);

	ctxt.z = z;

	list.push_front(z);

	_parser->pushTables(&_locationZoneParsers, _locationZoneStmt);
	
	return;
}




void LocationParser_ns::parseGetData(ZonePtr z) {

	GetData *data = new GetData;

	do {

		if (!scumm_stricmp(_tokens[0], "file")) {

			bool visible = (z->_flags & kFlagsRemove) == 0;

			GfxObj *obj = _vm->_gfx->loadGet(_tokens[1]);
			obj->frame = 0;
			obj->x = z->_left;
			obj->y = z->_top;
			_vm->_gfx->showGfxObj(obj, visible);

			data->gfxobj = obj;
		}

		if (!scumm_stricmp(_tokens[0], "icon")) {
			data->_icon = 4 + _vm->_objectsNames->lookup(_tokens[1]);
		}

		_script->readLineToken(true);
	} while (scumm_stricmp(_tokens[0], "endzone"));

	z->u.get = data;

}


void LocationParser_ns::parseExamineData(ZonePtr z) {

	ExamineData *data = new ExamineData;

	do {

		if (!scumm_stricmp(_tokens[0], "file")) {
			data->_filename = strdup(_tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "desc")) {
			data->_description = parseComment();
		}

		_script->readLineToken(true);
	} while (scumm_stricmp(_tokens[0], "endzone"));

	z->u.examine = data;

}


void LocationParser_ns::parseDoorData(ZonePtr z) {

	DoorData *data = new DoorData;

	do {

		if (!scumm_stricmp(_tokens[0], "slidetext")) {
			strcpy(_vm->_location._slideText[0], _tokens[1]);
//				printf("%s\t", _slideText[0]);
			strcpy(_vm->_location._slideText[1], _tokens[2]);
		}

		if (!scumm_stricmp(_tokens[0], "location")) {
			data->_location = strdup(_tokens[1]);
		}

		if (!scumm_stricmp(_tokens[0], "file")) {
//				printf("file: '%s'", _tokens[0]);

			uint16 frame = (z->_flags & kFlagsClosed ? 0 : 1);

			GfxObj *obj = _vm->_gfx->loadDoor(_tokens[1]);
			obj->frame = frame;
			obj->x = z->_left;
			obj->y = z->_top;
			_vm->_gfx->showGfxObj(obj, true);

			data->gfxobj = obj;
		}

		if (!scumm_stricmp(_tokens[0],	"startpos")) {
			data->_startPos.x = atoi(_tokens[1]);
			data->_startPos.y = atoi(_tokens[2]);
			data->_startFrame = atoi(_tokens[3]);
		}

		_script->readLineToken(true);
	} while (scumm_stricmp(_tokens[0], "endzone"));

	z->u.door = data;

}


void LocationParser_ns::parseMergeData(ZonePtr z) {

	MergeData *data = new MergeData;

	do {

		if (!scumm_stricmp(_tokens[0], "obj1")) {
			data->_obj1 = 4 + _vm->_objectsNames->lookup(_tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "obj2")) {
			data->_obj2 = 4 + _vm->_objectsNames->lookup(_tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "newobj")) {
			data->_obj3 = 4 + _vm->_objectsNames->lookup(_tokens[1]);
		}

		_script->readLineToken(true);
	} while (scumm_stricmp(_tokens[0], "endzone"));

	z->u.merge = data;

}

void LocationParser_ns::parseHearData(ZonePtr z) {

	HearData *data = new HearData;

	do {

		if (!scumm_stricmp(_tokens[0], "sound")) {
			strcpy(data->_name, _tokens[1]);
			data->_channel = atoi(_tokens[2]);
		}
		if (!scumm_stricmp(_tokens[0], "freq")) {
			data->_freq = atoi(_tokens[1]);
		}

		_script->readLineToken(true);
	} while (scumm_stricmp(_tokens[0], "endzone"));

	z->u.hear = data;

}

void LocationParser_ns::parseSpeakData(ZonePtr z) {

	SpeakData *data = new SpeakData;

	do {

		if (!scumm_stricmp(_tokens[0], "file")) {
			strcpy(data->_name, _tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "Dialogue")) {
			data->_dialogue = parseDialogue();
		}

		_script->readLineToken(true);
	} while (scumm_stricmp(_tokens[0], "endzone"));

	z->u.speak = data;

}


void LocationParser_ns::parseZoneTypeBlock(ZonePtr z) {
	debugC(7, kDebugParser, "parseZoneTypeBlock(name: %s, type: %x)", z->_name, z->_type);

	switch (z->_type & 0xFFFF) {
	case kZoneExamine:	// examine Zone alloc
		parseExamineData(z);
		break;

	case kZoneDoor: // door Zone alloc
		parseDoorData(z);
		break;

	case kZoneGet:	// get Zone alloc
		parseGetData(z);
		break;

	case kZoneMerge:	// merge Zone alloc
		parseMergeData(z);
		break;

	case kZoneHear: // hear Zone alloc
		parseHearData(z);
		break;

	case kZoneSpeak:	// speak Zone alloc
		parseSpeakData(z);
		break;

	default:
		// eats up 'ENDZONE' line for unprocessed zone types
		_script->readLineToken(true);
		break;
	}

	debugC(7, kDebugParser, "parseZoneTypeBlock() done");

	return;
}


} // namespace Parallaction
