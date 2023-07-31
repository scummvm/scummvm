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
#include "m4/burger/burger.h"
#include "m4/burger/vars.h"
#include "m4/burger/wilbur.h"

namespace M4 {
namespace Burger {

BurgerEngine::BurgerEngine(OSystem *syst, const ADGameDescription *gameDesc) :
		M4Engine(syst, gameDesc) {
	_sections.push_back(&_section1);
	_sections.push_back(&_section1);	// TODO: Replace me
	_sections.push_back(&_section1);	// TODO: Replace me
	_sections.push_back(&_section1);	// TODO: Replace me
	_sections.push_back(&_section1);	// TODO: Replace me
	_sections.push_back(&_section1);	// TODO: Replace me
	_sections.push_back(&_section1);	// TODO: Replace me
	_sections.push_back(&_section1);	// TODO: Replace me
	_sections.push_back(&_section9);
}

M4::Vars *BurgerEngine::createVars() {
	return new Burger::Vars();
}

void BurgerEngine::showEngineInfo() {
	debug("Orion Burger\n");
	debug("Game Version %s -- %s\n", "Giraffe", "September 27, 1996");
	debug("%s\n", "M4 Runtime Engine by NickP, MikeE, AndrasK, Tinman, XiH, PaulL, MattP");
	debug("M4 Library Version %s -- %s\n", "v1.400 OB", "January 21, 1996");
	debug("%s.\n\n", "Copyright (c) 1996 by Sanctuary Woods Multimedia Corporation");
	debug("Orion Burger tastes great!\n");
}

void BurgerEngine::syncFlags(Common::Serializer &s) {
	g_globals->_flags.sync(s);
}

void BurgerEngine::global_daemon() {
	switch (_G(kernel).trigger) {
	case gSERIES_STREAM_BREAK:
		handle_series_stream_break();
		break;

	case gRELEASE_TRIGGER_DIGI_CHECK:
		release_trigger_digi_check();
		break;

	case gWILBURS_SPEECH_START:
		wilbur_say();
		break;

	case gWILBURS_SPEECH_FINISHED:
		wilburs_speech_finished();
		break;

	// TODO: Other cases

	default:
		break;
	}
}

} // namespace Burger
} // namespace M4
