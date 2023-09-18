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

#include "common/debug.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"
#include "m4/core/errors.h"

namespace M4 {
namespace Riddle {

RiddleEngine::RiddleEngine(OSystem *syst, const ADGameDescription *gameDesc) :
		M4Engine(syst, gameDesc) {
}

M4::Vars *RiddleEngine::createVars() {
	return new Riddle::Vars();
}

void RiddleEngine::showEngineInfo() {
	debug("The Riddle of Master Lu\n");
	debug("Game Version %s -- %s\n", "2.05", "Dec 14, 1995");
	debug("%s\n", "M4 Runtime by Nick, Tinman, Mike, Xi, Andras, Paul (the fifth Beatle)");
	debug("M4 Library Version %s -- %s\n", "v1.213", "September 8, 1995");
	debug("%s.\n\n", "Copyright (c) 1995 by Sanctuary Woods Multimedia Corporation");
}

void RiddleEngine::syncFlags(Common::Serializer &s) {
	//g_vars->_flags.sync(s);
}

void RiddleEngine::global_daemon() {
	// TODO
}

void RiddleEngine::global_pre_parser() {
	// TODO
}

void RiddleEngine::global_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;
	// TODO
}

} // namespace Riddle
} // namespace M4
