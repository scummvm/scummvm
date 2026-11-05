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

#ifndef NANCY_SCRIPT_V5_SCRIPTINPUT_H
#define NANCY_SCRIPT_V5_SCRIPTINPUT_H

#include "engines/nancy/script/types.h"

namespace Nancy {

struct NancyInput;

namespace Script {

class Record;

// Nancy26 hands the player's input to the records of all the streams at once,
// the frontmost first, as far down as the first stream that keeps the input to
// itself. The mouse is over the frontmost hotspot it is on, which gets a click;
// while a button is down, the mouse stays with the hotspot it went down on.
// The functions Overrides have for the mouse and the keys get called in the
// same order, until one of them returns true; the keys go by keyZ, though.
void handleNancy26Input(NancyInput &input);

// Where the mouse is, in screen coordinates
const ScriptPoint &getScriptMousePoint();

// Whether a button was just clicked twice in a row; 0 is the left one
bool isDoubleClick(int button);

// A record that goes away can't keep the mouse
void forgetInputRecord(Record *record);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_SCRIPTINPUT_H
