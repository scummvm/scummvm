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

#include "sword1/console.h"
#include "sword1/sword1.h"
#include "sword1/sound.h"
#include "common/config-manager.h"
#include "common/str.h"

namespace Sword1 {

SwordConsole::SwordConsole(SwordEngine *vm) : GUI::Debugger(), _vm(vm) {
	assert(_vm);
	if (_vm->isMac())
		registerCmd("speechEndianness",    WRAP_METHOD(SwordConsole, Cmd_SpeechEndianness));
}

SwordConsole::~SwordConsole() {
}

bool SwordConsole::Cmd_SpeechEndianness(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Using %s speech\n", _vm->_sound->_bigEndianSpeech ? "be" : "le");
		return true;
	}
	if (argc == 2) {
		if (scumm_stricmp(argv[1], "le") == 0) {
			_vm->_sound->_bigEndianSpeech = false;
			return false;
		} else if (scumm_stricmp(argv[1], "be") == 0) {
			_vm->_sound->_bigEndianSpeech = true;
			return false;
		}
	}
	debugPrintf("Usage: %s [le | be]\n", argv[0]);
	return true;
}

} // End of namespace Sword
