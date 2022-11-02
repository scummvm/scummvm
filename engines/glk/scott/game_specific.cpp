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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "common/str.h"
#include "glk/scott/scott.h"
#include "glk/scott/globals.h"
#include "glk/scott/game_specific.h"

namespace Glk {
namespace Scott {

void mysterious64Sysmess() {
	SysMessageType messageKey[] = {
		NORTH,
		SOUTH,
		EAST,
		WEST,
		UP,
		DOWN,
		EXITS,
		YOU_SEE,
		YOU_ARE,
		TOO_DARK_TO_SEE,
		LIGHT_HAS_RUN_OUT,
		LIGHT_RUNS_OUT_IN,
		TURNS,
		I_DONT_KNOW_HOW_TO,
		SOMETHING,
		I_DONT_KNOW_WHAT_A,
		IS,
		YOU_CANT_GO_THAT_WAY,
		OK,
		WHAT_NOW,
		HUH,
		YOU_HAVENT_GOT_IT,
		INVENTORY,
		YOU_DONT_SEE_IT,
		THATS_BEYOND_MY_POWER,
		DANGEROUS_TO_MOVE_IN_DARK,
		DIRECTION,
		YOU_FELL_AND_BROKE_YOUR_NECK,
		YOURE_CARRYING_TOO_MUCH,
		IM_DEAD,
		PLAY_AGAIN,
		RESUME_A_SAVED_GAME,
		IVE_STORED,
		TREASURES,
		ON_A_SCALE_THAT_RATES,
		YOU_CANT_DO_THAT_YET,
		I_DONT_UNDERSTAND,
		NOTHING,
		YOUVE_SOLVED_IT,
		YOUVE_SOLVED_IT};

	for (int i = 0; i < 40; i++) {
		_G(_sys)[messageKey[i]] = _G(_systemMessages)[i];
	}

	_G(_sys)[ITEM_DELIMITER] = " - ";
	_G(_sys)[MESSAGE_DELIMITER] = "\n";

	_G(_sys)[YOU_SEE] = "\nThings I can see:\n";

	_G(_sys)[I_DONT_KNOW_HOW_TO] = "\"";
	_G(_sys)[PLAY_AGAIN] = "The game is over, thanks for playing\nWant to play again ? ";

	char *dictword = nullptr;
	for (int i = 1; i <= 6; i++) {
		dictword = new char[_G(_gameHeader)->_wordLength];
		Common::strlcpy(dictword, _G(_sys)[i - 1].c_str(), _G(_gameHeader)->_wordLength);
		_G(_nouns)[i] = dictword;
	}

	_G(_nouns)[0] = "ANY\0";

	switch (CURRENT_GAME) {
	case BATON_C64:
		_G(_nouns)[79] = "CAST\0";
		_G(_verbs)[79] = ".\0";
		_G(_gameHeader)->_numWords = 79;
		break;
	case TIME_MACHINE_C64:
		_G(_verbs)[86] = ".\0";
		break;
	case ARROW1_C64:
		_G(_nouns)[82] = ".\0";
		break;
	case ARROW2_C64:
		_G(_verbs)[80] = ".\0";
		break;
	case PULSAR7_C64:
		_G(_nouns)[102] = ".\0";
		break;
	case CIRCUS_C64:
		_G(_nouns)[96] = ".\0";
		break;
	case FEASIBILITY_C64:
		_G(_nouns)[80] = ".\0";
		break;
	case PERSEUS_C64:
		_G(_nouns)[82] = ".\0";
		break;
	default:
		break;
	}
}

void claymorgue64Sysmess() {
	SysMessageType messagekey[] = {
		NORTH,
		SOUTH,
		EAST,
		WEST,
		UP,
		DOWN,
		EXITS,
		YOU_SEE,
		YOU_ARE,
		TOO_DARK_TO_SEE,
		LIGHT_HAS_RUN_OUT,
		LIGHT_RUNS_OUT_IN,
		TURNS,
		I_DONT_KNOW_HOW_TO,
		SOMETHING,
		I_DONT_KNOW_WHAT_A,
		IS,
		YOU_CANT_GO_THAT_WAY,
		OK,
		WHAT_NOW,
		HUH,
		YOU_HAVE_IT,
		YOU_HAVENT_GOT_IT,
		INVENTORY,
		YOU_DONT_SEE_IT,
		THATS_BEYOND_MY_POWER,
		DANGEROUS_TO_MOVE_IN_DARK,
		DIRECTION,
		YOU_FELL_AND_BROKE_YOUR_NECK,
		YOURE_CARRYING_TOO_MUCH,
		IM_DEAD,
		PLAY_AGAIN,
		RESUME_A_SAVED_GAME,
		IVE_STORED,
		TREASURES,
		ON_A_SCALE_THAT_RATES,
		YOU_CANT_DO_THAT_YET,
		I_DONT_UNDERSTAND,
		NOTHING,
		YOUVE_SOLVED_IT};

	for (int i = 0; i < 40; i++) {
		_G(_sys)[messagekey[i]] = _G(_systemMessages)[i];
	}

	_G(_sys)[I_DONT_KNOW_HOW_TO] = "I don't know how to \"";
	_G(_sys)[SOMETHING] = "\" something. ";
}

void adventurelandAction(int p) {
	int image = 0;
	switch (p) {
	case 1:
		image = 36;
		break;
	case 2:
		image = 34;
		break;
	case 3:
		image = 33;
		break;
	case 4:
		image = 35;
		break;
	default:
		error("Adventureland: Unsupported action parameter %d", p);
		return;
	}
	g_scott->drawImage(image);
	g_scott->output("\n");
	g_scott->output(_G(_sys)[HIT_ENTER]);
	g_scott->hitEnter();
	return;
}

void adventurelandDarkness() {
	if ((_G(_rooms)[MY_LOC]._image & 128) == 128)
		_G(_bitFlags) |= 1 << DARKBIT;
	else
		_G(_bitFlags) &= ~(1 << DARKBIT);
}

void adventureland64Sysmess() {
	SysMessageType messagekey[] = {
		NORTH,
		SOUTH,
		EAST,
		WEST,
		UP,
		DOWN,
		EXITS,
		YOU_SEE,
		YOU_ARE,
		TOO_DARK_TO_SEE,
		LIGHT_HAS_RUN_OUT,
		LIGHT_RUNS_OUT_IN,
		TURNS,
		I_DONT_KNOW_HOW_TO,
		SOMETHING,
		I_DONT_KNOW_WHAT_A,
		IS,
		YOU_CANT_GO_THAT_WAY,
		OK,
		WHAT_NOW,
		HUH,
		YOU_HAVENT_GOT_IT,
		INVENTORY,
		YOU_DONT_SEE_IT,
		THATS_BEYOND_MY_POWER,
		DANGEROUS_TO_MOVE_IN_DARK,
		DIRECTION,
		YOU_FELL_AND_BROKE_YOUR_NECK,
		YOURE_CARRYING_TOO_MUCH,
		IM_DEAD,
		PLAY_AGAIN,
		RESUME_A_SAVED_GAME,
		IVE_STORED,
		TREASURES,
		ON_A_SCALE_THAT_RATES,
		YOU_CANT_DO_THAT_YET,
		I_DONT_UNDERSTAND,
		NOTHING,
		YOUVE_SOLVED_IT};

	for (int i = 0; i < 39; i++) {
		_G(_sys)[messagekey[i]] = _G(_systemMessages)[i];
	}

	_G(_sys)[I_DONT_KNOW_HOW_TO] = "I don't know how to \"";
	_G(_sys)[SOMETHING] = "\" something. ";
}

void spiderman64Sysmess() {
	SysMessageType messagekey[] = {
		NORTH,
		SOUTH,
		EAST,
		WEST,
		UP,
		DOWN,
		EXITS,
		YOU_SEE,
		YOU_ARE,
		TOO_DARK_TO_SEE,
		LIGHT_HAS_RUN_OUT,
		LIGHT_RUNS_OUT_IN,
		TURNS,
		I_DONT_KNOW_HOW_TO,
		SOMETHING,
		I_DONT_KNOW_WHAT_A,
		IS,
		YOU_CANT_GO_THAT_WAY,
		OK,
		WHAT_NOW,
		HUH,
		YOU_HAVE_IT,
		YOU_HAVENT_GOT_IT,
		INVENTORY,
		YOU_DONT_SEE_IT,
		THATS_BEYOND_MY_POWER,
		DANGEROUS_TO_MOVE_IN_DARK,
		DIRECTION,
		YOU_FELL_AND_BROKE_YOUR_NECK,
		YOURE_CARRYING_TOO_MUCH,
		IM_DEAD,
		DROPPED,
		TAKEN,
		PLAY_AGAIN,
		RESUME_A_SAVED_GAME,
		IVE_STORED,
		TREASURES,
		ON_A_SCALE_THAT_RATES,
		YOU_CANT_DO_THAT_YET,
		I_DONT_UNDERSTAND,
		NOTHING,
		YOUVE_SOLVED_IT};

	for (int i = 0; i < 42; i++) {
		_G(_sys)[messagekey[i]] = _G(_systemMessages)[i];
	}

	_G(_sys)[I_DONT_KNOW_HOW_TO] = "I don't know how to \"";
	_G(_sys)[SOMETHING] = "\" something. ";
}

void supergran64Sysmess(void) {
	SysMessageType messagekey[] = {
		NORTH,
		SOUTH,
		EAST,
		WEST,
		UP,
		DOWN,
		EXITS,
		YOU_SEE,
		YOU_ARE,
		I_DONT_KNOW_WHAT_A,
		IS,
		YOU_CANT_GO_THAT_WAY,
		OK,
		WHAT_NOW,
		HUH,
		YOU_HAVE_IT,
		TAKEN,
		DROPPED,
		YOU_HAVENT_GOT_IT,
		INVENTORY,
		YOU_DONT_SEE_IT,
		THATS_BEYOND_MY_POWER,
		DIRECTION,
		YOURE_CARRYING_TOO_MUCH,
		IM_DEAD,
		PLAY_AGAIN,
		RESUME_A_SAVED_GAME,
		YOU_CANT_DO_THAT_YET,
		I_DONT_UNDERSTAND,
		NOTHING};

	for (int i = 0; i < 30; i++) {
		_G(_sys)[messagekey[i]] = _G(_systemMessages)[i];
	}

	_G(_sys)[I_DONT_KNOW_WHAT_A] = "\"";
	_G(_sys)[IS] = "\" is a word I don't know. ";
}

} // End of namespace Scott
} // End of namespace Glk
