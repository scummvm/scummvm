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

#include "eem/console.h"
#include "eem/eem.h"
#include "eem/resource.h"

namespace EEM {

Console::Console(EEMEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("pic", WRAP_METHOD(Console, cmdPic));
}

bool Console::cmdPic(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: pic <number>\n");
		debugPrintf("Loads picture <number> from PICS.DBD and reports its dimensions.\n");
		debugPrintf("PICS.DBX has %u entries.\n", _vm->getPics().size());
		return true;
	}

	const uint num = (uint)atoi(argv[1]);
	Picture pic;
	if (!_vm->getPics().getPicture(num, pic)) {
		debugPrintf("pic %u: load failed\n", num);
		return true;
	}

	debugPrintf("pic %u (idx %u): %dx%d, compsize=%u, flags=0x%04x, miscflags=0x%04x, rowoff=%u\n",
				num, num - 1, pic.surface.w, pic.surface.h,
				pic.compsize, pic.flags, pic.miscflags, pic.rowoff);
	return true;
}

} // End of namespace EEM
