/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "queen/state.h"

namespace Queen {


Direction State::findDirection(uint16 state) {
	// queen.c l.4014-4021
	static const Direction sd[] = {
		DIR_BACK,
		DIR_RIGHT,
		DIR_LEFT,
		DIR_FRONT
	};
	return sd[(state >> 2) & 3];
}


StateTalk State::findTalk(uint16 state) {
	return (state & (1 << 9)) ? STATE_TALK_TALK : STATE_TALK_MUTE;
}


StateGrab State::findGrab(uint16 state) {
	// queen.c l.4022-4029
	static const StateGrab gd[] = {
		STATE_GRAB_NONE,
		STATE_GRAB_DOWN,
		STATE_GRAB_UP,
		STATE_GRAB_MID
	};
	return gd[state & 3];
}


StateOn State::findOn(uint16 state) {
	return (state & (1 << 8)) ? STATE_ON_ON : STATE_ON_OFF;
}


Verb State::findDefaultVerb(uint16 state) {
	Verb v;
	switch((state >> 4) & 0xF) {
	case 1:
		v = Verb(VERB_OPEN);
		break;
	case 3:
		v = Verb(VERB_CLOSE);
		break;
	case 7:
		v = Verb(VERB_MOVE);
		break;
	case 8:
		v = Verb(VERB_GIVE);
		break;
	case 12:
		v = Verb(VERB_USE);
		break;
	case 14:
		v = Verb(VERB_PICK_UP);
		break;
	case 9:
		v = Verb(VERB_TALK_TO);
		break;
	case 6:
		v = Verb(VERB_LOOK_AT);
		break;
	default:
		v = Verb(VERB_NONE);
		break;
	}
	return v;
}


StateUse State::findUse(uint16 state) {
	return (state & (1 << 10)) ? STATE_USE : STATE_USE_ON;
}


void State::alterOn(uint16 *objState, StateOn state) {
	switch (state) {
	case STATE_ON_ON:
		*objState |= (1 << 8);
		break;
	case STATE_ON_OFF:
		*objState &= ~(1 << 8);
		break;
	}
}


void State::alterDefaultVerb(uint16 *objState, Verb v) {
	uint16 val;
	switch (v.value()) {
	case VERB_OPEN:
		val = 1;
		break;
	case VERB_CLOSE:
		val = 3;
		break;
	case VERB_MOVE:
		val = 7;
		break;
	case VERB_GIVE:
		val = 8;
		break;
	case VERB_USE:
		val = 12;
		break;
	case VERB_PICK_UP:
		val = 14;
		break;
	case VERB_TALK_TO:
		val = 9;
		break;
	case VERB_LOOK_AT:
		val = 6;
		break;
	default:
		val = 0;
		break;
	}
	*objState = (*objState & ~0xF0) | (val << 4);
}


} // End of namespace Queen
