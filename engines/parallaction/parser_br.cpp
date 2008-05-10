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

#define CMD_SET			1
#define CMD_CLEAR		2
#define CMD_START		3
#define CMD_SPEAK		4
#define CMD_GET			5
#define CMD_LOCATION	6
#define CMD_OPEN		7
#define CMD_CLOSE		8
#define CMD_ON			9
#define CMD_OFF			10
#define CMD_CALL		11
#define CMD_TOGGLE		12
#define CMD_DROP		13
#define CMD_QUIT		14
#define CMD_MOVE		15
#define CMD_STOP		16
#define CMD_CHARACTER	17
#define CMD_FOLLOWME	18
#define CMD_ONMOUSE		19
#define CMD_OFFMOUSE	20
#define CMD_ADD			21
#define CMD_LEAVE		22
#define CMD_INC			23
#define CMD_DEC			24
#define CMD_TEST		25
#define CMD_TEST_GT		26
#define CMD_TEST_LT		27
#define CMD_LET			28
#define CMD_MUSIC		29
#define CMD_FIX			30
#define CMD_UNFIX		31
#define CMD_ZETA		32
#define CMD_SCROLL		33
#define CMD_SWAP		34
#define CMD_GIVE		35
#define CMD_TEXT		36
#define CMD_PART		37
#define CMD_TEST_SFX	38
#define CMD_RETURN		39
#define CMD_ONSAVE		40
#define CMD_OFFSAVE		41


#define INST_ON			1
#define INST_OFF		2
#define INST_X			3
#define INST_Y			4
#define INST_Z			5
#define INST_F			6
#define INST_LOOP		7
#define INST_ENDLOOP	8
#define INST_SHOW		9
#define INST_INC		10
#define INST_DEC		11
#define INST_SET		12
#define INST_PUT		13
#define INST_CALL		14
#define INST_WAIT		15
#define INST_START		16
#define INST_PROCESS	17
#define INST_MOVE		18
#define INST_COLOR		19
#define INST_SOUND		20
#define INST_MASK		21
#define INST_PRINT		22
#define INST_TEXT		23
#define INST_MUL		24
#define INST_DIV		25
#define INST_IF			26
#define INST_IFEQ		27
#define INST_IFLT		28
#define INST_IFGT		29
#define INST_ENDIF		30
#define INST_STOP		31

const char *_zoneTypeNamesRes_br[] = {
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
	"you",
	"command",
	"path",
	"box"
};

const char *_zoneFlagNamesRes_br[] = {
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
	"nowalk",
	"yourself",
	"scaled",
	"selfuse"
};

const char *_commandsNamesRes_br[] = {
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
	"character",
	"followme",
	"onmouse",
	"offmouse",
	"add",
	"leave",
	"inc",
	"dec",
	"test",
	"dummy",
	"dummy",
	"let",
	"music",
	"fix",
	"unfix",
	"zeta",
	"scroll",
	"swap",
	"give",
	"text",
	"part",
	"dummy",
	"return",
	"onsave",
	"offsave",
	"endcommands",
	"ifchar",
	"endif"
};


const char *_audioCommandsNamesRes_br[] = {
	"play",
	"stop",
	"pause",
	"channel_level",
	"fadein",
	"fadeout",
	"volume",
	" ",
	"faderate",
	" ",
	" ",
	" ",
	" ",
	" ",
	" ",
	" ",
	"loop"
};

const char *_locationStmtRes_br[] = {
	"character",
	"endlocation",
	"ifchar",
	"endif",
	"location",
	"mask",
	"path",
	"disk",
	"localflags",
	"commands",
	"escape",
	"acommands",
	"flags",
	"comment",
	"endcomment",
	"zone",
	"animation",
	"zeta",
	"music",
	"sound"
};

const char *_locationZoneStmtRes_br[] = {
	"endzone",
	"limits",
	"moveto",
	"type",
	"commands",
	"label",
	"flags"
};

const char *_locationAnimStmtRes_br[] = {
	"endanimation",
	"endzone",
	"script",
	"commands",
	"type",
	"label",
	"flags",
	"file",
	"position",
	"moveto"
};

const char *_instructionNamesRes_br[] = {
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
	"process",
	"move",
	"color",
	"sound",
	"mask",
	"print",
	"text",
	"mul",
	"div",
	"if",
	"dummy",
	"dummy",
	"endif",
	"stop",
	"endscript"
};


#define DECLARE_ZONE_PARSER(sig) void LocationParser_br::locZoneParse_##sig()
#define DECLARE_ANIM_PARSER(sig) void LocationParser_br::locAnimParse_##sig()
#define DECLARE_COMMAND_PARSER(sig) void LocationParser_br::cmdParse_##sig()
#define DECLARE_LOCATION_PARSER(sig) void LocationParser_br::locParse_##sig()

#define DECLARE_INSTRUCTION_PARSER(sig) void ProgramParser_br::instParse_##sig()


DECLARE_LOCATION_PARSER(location)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(location) ");

	strcpy(_vm->_location._name, _tokens[1]);
	_locParseCtxt.bgName = strdup(_tokens[1]);

	bool flip = false;
	int nextToken;

	if (!scumm_stricmp("flip", _tokens[2])) {
		flip = true;
		nextToken = 3;
	} else {
		nextToken = 2;
	}

	// TODO: handle background horizontal flip (via a context parameter)

	if (_tokens[nextToken][0] != '\0') {
		_vm->_char._ani->_left = atoi(_tokens[nextToken]);
		nextToken++;
		_vm->_char._ani->_top = atoi(_tokens[nextToken]);
		nextToken++;
	}

	if (_tokens[nextToken][0] != '\0') {
		_vm->_char._ani->_frame = atoi(_tokens[nextToken]);
	}
}



DECLARE_LOCATION_PARSER(zone)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(zone) ");

	parseZone(*_locParseCtxt.script, _vm->_location._zones, _tokens[1]);

	_locParseCtxt.z->_index = _locParseCtxt.numZones++;

	if (_vm->getLocationFlags() & kFlagsVisited) {
		_locParseCtxt.z->_flags = _vm->_zoneFlags[_vm->_currentLocationIndex][_locParseCtxt.z->_index];
	} else {
		_vm->_zoneFlags[_vm->_currentLocationIndex][_locParseCtxt.z->_index] = _locParseCtxt.z->_flags;
	}

}


DECLARE_LOCATION_PARSER(animation)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(animation) ");

	parseAnimation(*_locParseCtxt.script, _vm->_location._animations, _tokens[1]);

	_locParseCtxt.a->_index = _locParseCtxt.numZones++;

	if (_vm->getLocationFlags() & kFlagsVisited) {
		_locParseCtxt.a->_flags = _vm->_zoneFlags[_vm->_currentLocationIndex][_locParseCtxt.a->_index];
	} else {
		_vm->_zoneFlags[_vm->_currentLocationIndex][_locParseCtxt.a->_index] = _locParseCtxt.a->_flags;
	}

}


DECLARE_LOCATION_PARSER(localflags)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(localflags) ");

	int _si = 1;
	while (_tokens[_si][0] != '\0') {
		_vm->_localFlagNames->addData(_tokens[_si]);
		_si++;
	}
}


DECLARE_LOCATION_PARSER(flags)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(flags) ");

	if ((_vm->getLocationFlags() & kFlagsVisited) == 0) {
		// only for 1st visit
		_vm->clearLocationFlags(kFlagsAll);
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

	_vm->_location._comment = parseComment(*_locParseCtxt.script);
}


DECLARE_LOCATION_PARSER(endcomment)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(endcomment) ");

	_vm->_location._endComment = parseComment(*_locParseCtxt.script);
}


DECLARE_LOCATION_PARSER(sound)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(sound) ");

//	_soundMan->loadSound(_tokens[1]);
}


DECLARE_LOCATION_PARSER(music)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(music) ");

//	_soundMan->loadMusic(_tokens[1]);
}

DECLARE_LOCATION_PARSER(redundant)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(redundant) ");

	warning("redundant '%s' line found in script '%s'", _tokens[0], _locParseCtxt.filename);
}


DECLARE_LOCATION_PARSER(character)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(character) ");

//	changeCharacter(character);
}


DECLARE_LOCATION_PARSER(ifchar)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(ifchar) ");

	_locParseCtxt.script->skip("ENDIF");
}


DECLARE_LOCATION_PARSER(null)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(null) ");


}


DECLARE_LOCATION_PARSER(mask)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(mask) ");

	_locParseCtxt.maskName = strdup(_tokens[1]);
	_vm->_gfx->_backgroundInfo.layers[0] = atoi(_tokens[2]);
	_vm->_gfx->_backgroundInfo.layers[1] = atoi(_tokens[3]);
	_vm->_gfx->_backgroundInfo.layers[2] = atoi(_tokens[4]);
}


DECLARE_LOCATION_PARSER(path)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(path) ");

	_locParseCtxt.pathName = strdup(_tokens[1]);
}


DECLARE_LOCATION_PARSER(escape)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(escape) ");

	parseCommands(*_locParseCtxt.script, _vm->_location._escapeCommands);
}


DECLARE_LOCATION_PARSER(zeta)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(zeta) ");

	_vm->_location._zeta0 = atoi(_tokens[1]);
	_vm->_location._zeta1 = atoi(_tokens[2]);

	if (_tokens[3][0] != '\0') {
		_vm->_location._zeta2 = atoi(_tokens[1]);
	} else {
		_vm->_location._zeta2 = 50;
	}
}

DECLARE_COMMAND_PARSER(ifchar)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(ifchar) ");

	if (!scumm_stricmp(_vm->_char.getName(), _tokens[1]))
		_locParseCtxt.script->skip("endif");
}


DECLARE_COMMAND_PARSER(endif)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(endif) ");


}


DECLARE_COMMAND_PARSER(location)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(location) ");

	createCommand(parser->_lookup);

	_locParseCtxt.cmd->u._string = strdup(_tokens[1]);
	_locParseCtxt.nextToken++;

	if (_tokens[_locParseCtxt.nextToken][0] != '\0') {
		if (isdigit(_tokens[_locParseCtxt.nextToken][0]) || _tokens[_locParseCtxt.nextToken][0] == '-') {
			_locParseCtxt.cmd->u._startPos.x = atoi(_tokens[_locParseCtxt.nextToken]);
			_locParseCtxt.nextToken++;
			_locParseCtxt.cmd->u._startPos.y = atoi(_tokens[_locParseCtxt.nextToken]);
			_locParseCtxt.nextToken++;
		}

		if (isdigit(_tokens[_locParseCtxt.nextToken][0]) || _tokens[_locParseCtxt.nextToken][0] == '-') {
			_locParseCtxt.cmd->u._startPos2.x = atoi(_tokens[_locParseCtxt.nextToken]);
			_locParseCtxt.nextToken++;
			_locParseCtxt.cmd->u._startPos2.y = atoi(_tokens[_locParseCtxt.nextToken]);
			_locParseCtxt.nextToken++;
		}
	}

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(string)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(string) ");

	createCommand(parser->_lookup);

	_locParseCtxt.cmd->u._string = strdup(_tokens[1]);
	_locParseCtxt.nextToken++;

	parseCommandFlags();
	addCommand();
}

DECLARE_COMMAND_PARSER(math)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(math) ");

	createCommand(parser->_lookup);

	_locParseCtxt.cmd->u._lvalue = _vm->_countersNames->lookup(_tokens[1]);
	_locParseCtxt.nextToken++;
	_locParseCtxt.cmd->u._rvalue = atoi(_tokens[2]);
	_locParseCtxt.nextToken++;

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(test)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(test) ");

	createCommand(parser->_lookup);

	uint counter = _vm->_countersNames->lookup(_tokens[1]);
	_locParseCtxt.nextToken++;

	if (counter == Table::notFound) {
		if (!scumm_stricmp("SFX", _tokens[1])) {
			_locParseCtxt.cmd->_id = CMD_TEST_SFX;
		} else {
			error("unknown counter '%s' in test opcode", _tokens[1]);
		}
	} else {
		_locParseCtxt.cmd->u._lvalue = counter;
		_locParseCtxt.cmd->u._rvalue = atoi(_tokens[3]);
		_locParseCtxt.nextToken++;

		if (_tokens[2][0] == '>') {
			_locParseCtxt.cmd->_id = CMD_TEST_GT;
		} else
		if (_tokens[2][0] == '<') {
			_locParseCtxt.cmd->_id = CMD_TEST_LT;
		}
		_locParseCtxt.nextToken++;
	}

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(music)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(music) ");

	createCommand(parser->_lookup);

	_locParseCtxt.cmd->u._musicCommand = _audioCommandsNames->lookup(_tokens[1]);
	_locParseCtxt.nextToken++;

	if (_tokens[2][0] != '\0' && scumm_stricmp("flags", _tokens[2]) && scumm_stricmp("gflags", _tokens[2])) {
		_locParseCtxt.cmd->u._musicParm = atoi(_tokens[2]);
		_locParseCtxt.nextToken++;
	}

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(zeta)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(zeta) ");

	createCommand(parser->_lookup);

	_locParseCtxt.cmd->u._zeta0 = atoi(_tokens[1]);
	_locParseCtxt.nextToken++;
	_locParseCtxt.cmd->u._zeta1 = atoi(_tokens[2]);
	_locParseCtxt.nextToken++;

	if (_tokens[3][0] != '\0') {
		_locParseCtxt.cmd->u._zeta2 = atoi(_tokens[3]);
		_locParseCtxt.nextToken++;
	} else {
		_locParseCtxt.cmd->u._zeta2 = 50;
	}

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(give)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(give) ");

	createCommand(parser->_lookup);

	_locParseCtxt.cmd->u._object = 4 + atoi(_tokens[1]);
	_locParseCtxt.nextToken++;

	if (!scumm_stricmp("dino", _tokens[2])) {
		_locParseCtxt.cmd->u._characterId = 1;
	} else
	if (!scumm_stricmp("doug", _tokens[2])) {
		_locParseCtxt.cmd->u._characterId = 2;
	} else
	if (!scumm_stricmp("donna", _tokens[2])) {
		_locParseCtxt.cmd->u._characterId = 3;
	} else
		error("unknown recipient '%s' in give command", _tokens[2]);

	_locParseCtxt.nextToken++;

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(text)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(text) ");

	createCommand(parser->_lookup);

	if (isdigit(_tokens[1][1])) {
		_locParseCtxt.cmd->u._zeta0 = atoi(_tokens[1]);
		_locParseCtxt.nextToken++;
	} else {
		_locParseCtxt.cmd->u._zeta0 = -1;
	}

	_locParseCtxt.cmd->u._string = strdup(_tokens[_locParseCtxt.nextToken]);
	_locParseCtxt.nextToken++;

	if (_tokens[_locParseCtxt.nextToken][0] != '\0' && scumm_stricmp("flags", _tokens[_locParseCtxt.nextToken])) {
		_locParseCtxt.cmd->u._string2 = strdup(_tokens[_locParseCtxt.nextToken]);
		_locParseCtxt.nextToken++;
	}


	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(unary)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(unary) ");

	createCommand(parser->_lookup);

	_locParseCtxt.cmd->u._rvalue = atoi(_tokens[1]);
	_locParseCtxt.nextToken++;

	parseCommandFlags();
	addCommand();
}


DECLARE_ZONE_PARSER(limits)  {
	debugC(7, kDebugParser, "ZONE_PARSER(limits) ");

	if (isalpha(_tokens[1][1])) {
		_locParseCtxt.z->_flags |= kFlagsAnimLinked;
		_locParseCtxt.z->_linkedAnim = _vm->findAnimation(_tokens[1]);
		_locParseCtxt.z->_linkedName = strdup(_tokens[1]);
	} else {
		_locParseCtxt.z->_left = atoi(_tokens[1]);
		_locParseCtxt.z->_top = atoi(_tokens[2]);
		_locParseCtxt.z->_right = atoi(_tokens[3]);
		_locParseCtxt.z->_bottom = atoi(_tokens[4]);
	}
}


DECLARE_ZONE_PARSER(moveto)  {
	debugC(7, kDebugParser, "ZONE_PARSER(moveto) ");

	_locParseCtxt.z->_moveTo.x = atoi(_tokens[1]);
	_locParseCtxt.z->_moveTo.y = atoi(_tokens[2]);
//	_locParseCtxt.z->_moveTo.z = atoi(_tokens[3]);
}


DECLARE_ZONE_PARSER(type)  {
	debugC(7, kDebugParser, "ZONE_PARSER(type) ");

	if (_tokens[2][0] != '\0') {
		_locParseCtxt.z->_type = (4 + _vm->_objectsNames->lookup(_tokens[2])) << 16;
	}
	int16 _si = _zoneTypeNames->lookup(_tokens[1]);
	if (_si != Table::notFound) {
		_locParseCtxt.z->_type |= 1 << (_si - 1);
		parseZoneTypeBlock(*_locParseCtxt.script, _locParseCtxt.z);

//		if (_locParseCtxt.z->_type & kZoneHear) {
//			_soundMan->sfxCommand(START...);
//		}
	}

	parser->popTables();
}


DECLARE_ANIM_PARSER(file)  {
	debugC(7, kDebugParser, "ANIM_PARSER(file) ");

	_locParseCtxt.a->gfxobj = _vm->_gfx->loadAnim(_tokens[1]);
}


DECLARE_ANIM_PARSER(position)  {
	debugC(7, kDebugParser, "ANIM_PARSER(position) ");

	_locParseCtxt.a->_left = atoi(_tokens[1]);
	_locParseCtxt.a->_top = atoi(_tokens[2]);
	_locParseCtxt.a->_z = atoi(_tokens[3]);
	_locParseCtxt.a->_frame = atoi(_tokens[4]);
}


DECLARE_ANIM_PARSER(moveto)  {
	debugC(7, kDebugParser, "ANIM_PARSER(moveto) ");

	_locParseCtxt.a->_moveTo.x = atoi(_tokens[1]);
	_locParseCtxt.a->_moveTo.y = atoi(_tokens[2]);
//	_locParseCtxt.a->_moveTo.z = atoi(_tokens[3]);
}


DECLARE_ANIM_PARSER(endanimation)  {
	debugC(7, kDebugParser, "ANIM_PARSER(endanimation) ");


	if (_locParseCtxt.a->gfxobj) {
		_locParseCtxt.a->_right = _locParseCtxt.a->width();
		_locParseCtxt.a->_bottom = _locParseCtxt.a->height();
	}

	_locParseCtxt.a->_oldPos.x = -1000;
	_locParseCtxt.a->_oldPos.y = -1000;

	_locParseCtxt.a->_flags |= 0x1000000;

	parser->popTables();
}










DECLARE_INSTRUCTION_PARSER(zone)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(zone) ");

	_instParseCtxt.inst->_z = _vm->findZone(_tokens[1]);
	_instParseCtxt.inst->_index = parser->_lookup;
}



DECLARE_INSTRUCTION_PARSER(color)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(color) ");


	parseRValue(_instParseCtxt.inst->_opB, _tokens[1]);

	_instParseCtxt.inst->_colors[0] = atoi(_tokens[2]);
	_instParseCtxt.inst->_colors[1] = atoi(_tokens[3]);
	_instParseCtxt.inst->_colors[2] = atoi(_tokens[4]);
	_instParseCtxt.inst->_index = parser->_lookup;

}


DECLARE_INSTRUCTION_PARSER(mask)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(mask) ");


	parseRValue(_instParseCtxt.inst->_opA, _tokens[1]);
	parseRValue(_instParseCtxt.inst->_opB, _tokens[2]);
	parseRValue(_instParseCtxt.inst->_opC, _tokens[3]);
	_instParseCtxt.inst->_index = parser->_lookup;

}


DECLARE_INSTRUCTION_PARSER(print)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(print) ");

	parseRValue(_instParseCtxt.inst->_opB, _tokens[1]);
	_instParseCtxt.inst->_index = parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(text)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(text) ");


	int _si = 1;

	if (isdigit(_tokens[1][1])) {
		_instParseCtxt.inst->_y = atoi(_tokens[1]);
		_si = 2;
	} else {
		_instParseCtxt.inst->_y = -1;
	}

	_instParseCtxt.inst->_text = strdup(_tokens[_si]);
	_si++;

	if (_tokens[_si][0] != '\0' && scumm_stricmp("flags", _tokens[_si])) {
		_instParseCtxt.inst->_text2 = strdup(_tokens[_si]);
	}
	_instParseCtxt.inst->_index = parser->_lookup;

}


DECLARE_INSTRUCTION_PARSER(if_op)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(if_op) ");


	if (_instParseCtxt.openIf)
		error("cannot nest 'if' blocks");

	parseLValue(_instParseCtxt.inst->_opA, _tokens[1]);
	parseRValue(_instParseCtxt.inst->_opB, _tokens[3]);

	if (_tokens[2][0] == '=') {
		_instParseCtxt.inst->_index = INST_IFEQ;
	} else
	if (_tokens[2][0] == '>') {
		_instParseCtxt.inst->_index = INST_IFGT;
	} else
	if (_tokens[2][0] == '<') {
		_instParseCtxt.inst->_index = INST_IFLT;
	} else
		error("unknown test operator '%s' in if-clause", _tokens[2]);

	_instParseCtxt.openIf = _instParseCtxt.inst;

}


DECLARE_INSTRUCTION_PARSER(endif)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(endif) ");

	if (!_instParseCtxt.openIf)
		error("unexpected 'endif'");

//	_instParseCtxt.openIf->_endif = _instParseCtxt.inst;
	_instParseCtxt.openIf = nullInstructionPtr;
	_instParseCtxt.inst->_index = parser->_lookup;
}


void ProgramParser_br::parseRValue(ScriptVar &v, const char *str) {

	if (isdigit(str[0]) || str[0] == '-') {
		v.setImmediate(atoi(str));
		return;
	}

	int index = _instParseCtxt.program->findLocal(str);
	if (index != -1) {
		v.setLocal(&_instParseCtxt.locals[index]);
		return;
	}

	AnimationPtr a;
	if (str[1] == '.') {
		a = _vm->findAnimation(&str[2]);
		if (!a) {
			error("unknown animation '%s' in script", &str[2]);
		}
	} else
		a = AnimationPtr(_instParseCtxt.a);

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
	}	else
	if (str[0] == 'N') {
		v.setImmediate(a->getFrameNum());
	} else
	if (str[0] == 'R') {
		v.setRandom(atoi(&str[1]));
	} else
	if (str[0] == 'L') {
		v.setField(&_vm->_lipSyncVal);
	}

}

typedef OpcodeImpl<LocationParser_br> OpcodeV2;
#define ZONE_PARSER(sig)		OpcodeV2(this, &LocationParser_br::locZoneParse_##sig)
#define ANIM_PARSER(sig)		OpcodeV2(this, &LocationParser_br::locAnimParse_##sig)
#define LOCATION_PARSER(sig)	OpcodeV2(this, &LocationParser_br::locParse_##sig)
#define COMMAND_PARSER(sig)		OpcodeV2(this, &LocationParser_br::cmdParse_##sig)
#define WARNING_PARSER(sig)		OpcodeV2(this, &LocationParser_br::warning_##sig)

typedef OpcodeImpl<ProgramParser_br> OpcodeV3;
#define INSTRUCTION_PARSER(sig) OpcodeV3(this, &ProgramParser_br::instParse_##sig)


void LocationParser_br::init() {

	parser = new Parser;

	_zoneFlagNames = new Table(ARRAYSIZE(_zoneFlagNamesRes_br), _zoneFlagNamesRes_br);
	_zoneTypeNames = new Table(ARRAYSIZE(_zoneTypeNamesRes_br), _zoneTypeNamesRes_br);
	_commandsNames = new Table(ARRAYSIZE(_commandsNamesRes_br), _commandsNamesRes_br);
	_audioCommandsNames = new Table(ARRAYSIZE(_audioCommandsNamesRes_br), _audioCommandsNamesRes_br);
	_locationStmt = new Table(ARRAYSIZE(_locationStmtRes_br), _locationStmtRes_br);
	_locationZoneStmt = new Table(ARRAYSIZE(_locationZoneStmtRes_br), _locationZoneStmtRes_br);
	_locationAnimStmt = new Table(ARRAYSIZE(_locationAnimStmtRes_br), _locationAnimStmtRes_br);


	int i;

	static const OpcodeV2 op2[] = {
		WARNING_PARSER(unexpected),
		COMMAND_PARSER(flags),		// set
		COMMAND_PARSER(flags),		// clear
		COMMAND_PARSER(zone),	// start
		COMMAND_PARSER(zone),		// speak
		COMMAND_PARSER(zone),		// get
		COMMAND_PARSER(location),
		COMMAND_PARSER(zone),		// open
		COMMAND_PARSER(zone),		// close
		COMMAND_PARSER(zone),		// on
		COMMAND_PARSER(zone),		// off
		COMMAND_PARSER(call),
		COMMAND_PARSER(flags),		// toggle
		COMMAND_PARSER(drop),
		COMMAND_PARSER(simple),		// quit
		COMMAND_PARSER(move),
		COMMAND_PARSER(zone),	// stop
		COMMAND_PARSER(string),		// character
		COMMAND_PARSER(string),		// followme
		COMMAND_PARSER(simple),		// onmouse
		COMMAND_PARSER(simple),		// offmouse
		COMMAND_PARSER(drop),		// add
		COMMAND_PARSER(zone),		// leave
		COMMAND_PARSER(math),		// inc
		COMMAND_PARSER(math),		// dec
		COMMAND_PARSER(test),		// test
		WARNING_PARSER(unexpected),
		WARNING_PARSER(unexpected),
		COMMAND_PARSER(math),		// let
		COMMAND_PARSER(music),
		COMMAND_PARSER(zone),		// fix
		COMMAND_PARSER(zone),		// unfix
		COMMAND_PARSER(zeta),
		COMMAND_PARSER(unary),		// scroll
		COMMAND_PARSER(string),		// swap
		COMMAND_PARSER(give),
		COMMAND_PARSER(text),
		COMMAND_PARSER(unary),		// part
		WARNING_PARSER(unexpected),
		COMMAND_PARSER(simple),		// return
		COMMAND_PARSER(simple),		// onsave
		COMMAND_PARSER(simple),		// offsave
		COMMAND_PARSER(endcommands),	// endcommands
		COMMAND_PARSER(ifchar),
		COMMAND_PARSER(endif)
	};

	for (i = 0; i < ARRAYSIZE(op2); i++)
		_commandParsers.push_back(&op2[i]);

	static const OpcodeV2 op4[] = {
		WARNING_PARSER(unexpected),
		LOCATION_PARSER(character),
		LOCATION_PARSER(endlocation),
		LOCATION_PARSER(ifchar),
		LOCATION_PARSER(null),			// endif
		LOCATION_PARSER(location),
		LOCATION_PARSER(mask),
		LOCATION_PARSER(path),
		LOCATION_PARSER(null),			// disk
		LOCATION_PARSER(localflags),
		LOCATION_PARSER(commands),
		LOCATION_PARSER(escape),
		LOCATION_PARSER(acommands),
		LOCATION_PARSER(flags),
		LOCATION_PARSER(comment),
		LOCATION_PARSER(endcomment),
		LOCATION_PARSER(zone),
		LOCATION_PARSER(animation),
		LOCATION_PARSER(zeta),
		LOCATION_PARSER(music),
		LOCATION_PARSER(sound)
	};

	for (i = 0; i < ARRAYSIZE(op4); i++)
		_locationParsers.push_back(&op4[i]);

	static const OpcodeV2 op5[] = {
		WARNING_PARSER(unexpected),
		ZONE_PARSER(endzone),
		ZONE_PARSER(limits),
		ZONE_PARSER(moveto),
		ZONE_PARSER(type),
		ZONE_PARSER(commands),
		ZONE_PARSER(label),
		ZONE_PARSER(flags)
	};

	for (i = 0; i < ARRAYSIZE(op5); i++)
		_locationZoneParsers.push_back(&op5[i]);

	static const OpcodeV2 op6[] = {
		WARNING_PARSER(unexpected),
		ANIM_PARSER(endanimation),
		ANIM_PARSER(endanimation),		// endzone
		ANIM_PARSER(script),
		ANIM_PARSER(commands),
		ANIM_PARSER(type),
		ANIM_PARSER(label),
		ANIM_PARSER(flags),
		ANIM_PARSER(file),
		ANIM_PARSER(position),
		ANIM_PARSER(moveto)
	};

	for (i = 0; i < ARRAYSIZE(op6); i++)
		_locationAnimParsers.push_back(&op6[i]);

}

void ProgramParser_br::init() {

	parser = new Parser;

	_instructionNames = new Table(ARRAYSIZE(_instructionNamesRes_br), _instructionNamesRes_br);

	static const OpcodeV3 op0[] = {
		INSTRUCTION_PARSER(defLocal),	// invalid opcode -> local definition
		INSTRUCTION_PARSER(zone),		// on
		INSTRUCTION_PARSER(zone),		// off
		INSTRUCTION_PARSER(x),
		INSTRUCTION_PARSER(y),
		INSTRUCTION_PARSER(z),
		INSTRUCTION_PARSER(f),
		INSTRUCTION_PARSER(loop),
		INSTRUCTION_PARSER(null),		// endloop
		INSTRUCTION_PARSER(null),		// show
		INSTRUCTION_PARSER(inc),
		INSTRUCTION_PARSER(inc),		// dec
		INSTRUCTION_PARSER(set),
		INSTRUCTION_PARSER(put),
		INSTRUCTION_PARSER(call),
		INSTRUCTION_PARSER(null),		// wait
		INSTRUCTION_PARSER(zone),		// start
		INSTRUCTION_PARSER(zone),		// process
		INSTRUCTION_PARSER(move),
		INSTRUCTION_PARSER(color),
		INSTRUCTION_PARSER(zone),		// sound
		INSTRUCTION_PARSER(mask),
		INSTRUCTION_PARSER(print),
		INSTRUCTION_PARSER(text),
		INSTRUCTION_PARSER(inc),		// mul
		INSTRUCTION_PARSER(inc),		// div
		INSTRUCTION_PARSER(if_op),
		INSTRUCTION_PARSER(null),
		INSTRUCTION_PARSER(null),
		INSTRUCTION_PARSER(endif),
		INSTRUCTION_PARSER(zone),		// stop
		INSTRUCTION_PARSER(endscript)
	};

	uint i;
	for (i = 0; i < ARRAYSIZE(op0); i++)
		_instructionParsers.push_back(&op0[i]);

}

void LocationParser_br::parse(Script *script) {

	_locParseCtxt.numZones = 0;
	_locParseCtxt.bgName = 0;
	_locParseCtxt.maskName = 0;
	_locParseCtxt.pathName = 0;

	LocationParser_ns::parse(script);

	_vm->_gfx->setBackground(kBackgroundLocation, _locParseCtxt.bgName, _locParseCtxt.maskName, _locParseCtxt.pathName);
	_vm->_pathBuffer = &_vm->_gfx->_backgroundInfo.path;

	free(_locParseCtxt.bgName);
	free(_locParseCtxt.maskName);
	free(_locParseCtxt.pathName);

}

} // namespace Parallaction
