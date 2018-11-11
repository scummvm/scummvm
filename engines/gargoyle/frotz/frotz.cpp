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

#include "gargoyle/frotz/frotz.h"
#include "gargoyle/frotz/frotz_types.h"

namespace Gargoyle {
namespace Frotz {

Frotz *g_vm;

Frotz::Frotz(OSystem *syst, const GargoyleGameDescription *gameDesc) : Glk(syst, gameDesc),
		_storyId(UNKNOWN), _storySize(0), _sp(nullptr), _fp(nullptr), _frameCount(0),
		_ostream_screen(true), _ostream_script(false), _ostream_memory(false),
		_ostream_record(false), _istream_replay(false), _message(false),
		_cwin(0), _mwin(0), _mouse_x(0), _mouse_y(0), _menu_selected(0),
		_enableWrapping(false), _enableScripting(true), _enableScrolling(false),
		_enableBuffering(false), _reserveMem(0) {
	g_vm = this;
	Common::fill(&_stack[0], &_stack[STACK_SIZE], 0);
}

void Frotz::runGame(Common::SeekableReadStream *gameFile) {
	initialize();

	// TODO
}

void Frotz::initialize() {
	_mem.initialize();
}

Common::Error Frotz::loadGameState(int slot) {
	return Common::kNoError;
}

Common::Error Frotz::saveGameState(int slot, const Common::String &desc) {
	return Common::kNoError;
}

uint Frotz::getPC() const {
	// TODO
	return 0;
}

} // End of namespace Scott
} // End of namespace Gargoyle
