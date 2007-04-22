/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "common/stdafx.h"

#include "parallaction/parallaction.h"
#include "parallaction/graphics.h"


namespace Parallaction {

byte Gfx::_mouseArrow[256] = {
	0x12, 0x11, 0x11, 0x11, 0x13, 0x12, 0x12, 0x12, 0x13, 0x12, 0x12, 0x11, 0x13, 0x12, 0x12, 0x00,
	0x13, 0x12, 0x12, 0x11, 0x13, 0x12, 0x12, 0x12, 0x13, 0x12, 0x12, 0x12, 0x13, 0x12, 0x12, 0x12,
	0x13, 0x12, 0x12, 0x12, 0x13, 0x12, 0x12, 0x12, 0x13, 0x12, 0x12, 0x13, 0x13, 0x13, 0x12, 0x00,
	0x13, 0x00, 0x12, 0x00, 0x13, 0x00, 0x12, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x11, 0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x11, 0x12, 0x12, 0x12, 0x00, 0x12, 0x12, 0x12, 0x00,
	0x12, 0x12, 0x12, 0x00, 0x12, 0x12, 0x12, 0x11, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
	0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x13, 0x12, 0x12, 0x12, 0x00, 0x12, 0x12, 0x12, 0x00,
	0x13, 0x13, 0x12, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x11, 0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x00, 0x12, 0x12, 0x12, 0x00, 0x12, 0x12, 0x12, 0x00,
	0x12, 0x12, 0x12, 0x00, 0x12, 0x12, 0x12, 0x00, 0x12, 0x12, 0x12, 0x11, 0x12, 0x12, 0x12, 0x12,
	0x12, 0x12, 0x12, 0x13, 0x12, 0x12, 0x12, 0x00, 0x12, 0x12, 0x12, 0x00, 0x13, 0x12, 0x12, 0x00,
	0x00, 0x12, 0x13, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x11, 0x11, 0x11, 0x00, 0x12, 0x12, 0x12, 0x00, 0x12, 0x12, 0x12, 0x00, 0x12, 0x12, 0x11, 0x00,
	0x12, 0x12, 0x12, 0x00, 0x12, 0x12, 0x12, 0x00, 0x12, 0x12, 0x12, 0x00, 0x12, 0x12, 0x12, 0x11,
	0x12, 0x12, 0x12, 0x00, 0x12, 0x12, 0x12, 0x00, 0x13, 0x12, 0x12, 0x00, 0x00, 0x12, 0x13, 0x00,
	0x00, 0x12, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};



const char *_zoneFlagNamesRes[] = {
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

const char *_zoneTypeNamesRes[] = {
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

const char _gameNames[10][20] = {
	"GAME1",
	"GAME2",
	"GAME3",
	"GAME4",
	"GAME5",
	"GAME6",
	"GAME7",
	"GAME8",
	"GAME9",
	"GAME10"
};

const char *_commandsNamesRes[] = {
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
	"stop"
};

const char *_instructionNamesRes[] = {
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
	"move"
};

const char *_callableNamesRes[] = {
	"HBOff",
	"Projector",
	"StartIntro",
	"EndIntro",
	"MoveSheet",
	"Sketch",
	"Shade",
	"Score",
	"OffSound",
	"StartMusic",
	"CloseMusic",
	"Fade",
	"HBOn",
	"MoveSarc",
	"ContaFoglie",
	"ZeroFoglie",
	"Trasformata",
	"OffMouse",
	"OnMouse",
	"SetMask",
	"EndComment",
	"Frankenstain",
	"Finito",
	"Ridux",
	"TestResult"
};

typedef void (*callable)(void*);


void _c_play_boogie(void*);
void _c_startIntro(void*);
void _c_endIntro(void*);
void _c_moveSheet(void*);
void _c_sketch(void*);
void _c_shade(void*);
void _c_score(void*);
void _c_fade(void*);
void _c_moveSarc(void*);
void _c_contaFoglie(void*);
void _c_zeroFoglie(void*);
void _c_trasformata(void*);
void _c_offMouse(void*);
void _c_onMouse(void*);
void _c_setMask(void*);
void _c_endComment(void*);
void _c_frankenstein(void*);
void _c_finito(void*);
void _c_ridux(void*);
void _c_testResult(void*);
void _c_null(void*);

void _c_projector(void*);
void _c_HBOff(void*);
void _c_offSound(void*);
void _c_startMusic(void*);
void _c_closeMusic(void*);
void _c_HBOn(void*);

callable _callables[25];


Credit _credits[] = {
	{"Music and Sound Effects", "MARCO CAPRELLI"},
	{"PC Version", "RICCARDO BALLARINO"},
	{"Project Manager", "LOVRANO CANEPA"},
	{"Production", "BRUNO BOZ"},
	{"Special Thanks to", "LUIGI BENEDICENTI - GILDA and DANILO"},
	{"Copyright 1992 Euclidea s.r.l ITALY", "All rights reserved"},
	{"CLICK MOUSE BUTTON TO START", 0}
};

const char *_dinoName = "dino";
const char *_donnaName = "donna";
const char *_doughName = "dough";
const char *_drkiName = "drki";

const char *_minidinoName = "minidino";
const char *_minidonnaName = "minidonna";
const char *_minidoughName = "minidough";
const char *_minidrkiName = "minidrki";


void Parallaction::initResources() {

	_callableNames = new Table(ARRAYSIZE(_callableNamesRes), _callableNamesRes);
	_instructionNames = new Table(ARRAYSIZE(_instructionNamesRes), _instructionNamesRes);
	_zoneFlagNames = new Table(ARRAYSIZE(_zoneFlagNamesRes), _zoneFlagNamesRes);
	_zoneTypeNames = new Table(ARRAYSIZE(_zoneTypeNamesRes), _zoneTypeNamesRes);
	_commandsNames = new Table(ARRAYSIZE(_commandsNamesRes), _commandsNamesRes);

	_localFlagNames = new Table(120);
	_localFlagNames->addData("visited");

	if (getPlatform() == Common::kPlatformPC) {
		_callables[0] = 	_c_play_boogie;
		_callables[1] = 	_c_play_boogie;
		_callables[2] = 	_c_startIntro;
		_callables[3] = 	_c_endIntro;
		_callables[4] = 	_c_moveSheet;
		_callables[5] = 	_c_sketch;
		_callables[6] = 	_c_shade;
		_callables[7] = 	_c_score;
		_callables[8] = 	_c_null;
		_callables[9] = 	_c_null;
		_callables[10] = 	_c_null;
		_callables[11] = 	_c_fade;
		_callables[12] = 	_c_play_boogie;
		_callables[13] = 	_c_moveSarc;
		_callables[14] = 	_c_contaFoglie;
		_callables[15] = 	_c_zeroFoglie;
		_callables[16] = 	_c_trasformata;
		_callables[17] = 	_c_offMouse;
		_callables[18] = 	_c_onMouse;
		_callables[19] = 	_c_setMask;
		_callables[20] = 	_c_endComment;
		_callables[21] = 	_c_frankenstein;
		_callables[22] = 	_c_finito;
		_callables[23] = 	_c_ridux;
		_callables[24] = 	_c_testResult;
	} else {
		_callables[0] = 	_c_projector;
		_callables[1] = 	_c_HBOff;
		_callables[2] = 	_c_startIntro;
		_callables[3] = 	_c_endIntro;
		_callables[4] = 	_c_moveSheet;
		_callables[5] = 	_c_sketch;
		_callables[6] = 	_c_shade;
		_callables[7] = 	_c_score;
		_callables[8] = 	_c_offSound;
		_callables[9] = 	_c_startMusic;
		_callables[10] = 	_c_closeMusic;
		_callables[11] = 	_c_fade;
		_callables[12] = 	_c_HBOn;
		_callables[13] = 	_c_moveSarc;
		_callables[14] = 	_c_contaFoglie;
		_callables[15] = 	_c_zeroFoglie;
		_callables[16] = 	_c_trasformata;
		_callables[17] = 	_c_offMouse;
		_callables[18] = 	_c_onMouse;
		_callables[19] = 	_c_setMask;
		_callables[20] = 	_c_endComment;
		_callables[21] = 	_c_frankenstein;
		_callables[22] = 	_c_finito;
		_callables[23] = 	_c_ridux;
		_callables[24] = 	_c_testResult;
	}

}


} // namespace Parallaction
