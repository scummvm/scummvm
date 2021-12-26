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

#include "glk/tads/tads2/tads2.h"
#include "glk/tads/tads2/appctx.h"
#include "glk/tads/tads2/runtime_app.h"
#include "glk/tads/tads2/os.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

TADS2::TADS2(OSystem *syst, const GlkGameDescription &gameDesc) : TADS(syst, gameDesc) {
}

void TADS2::runGame() {
	// Initialize the OS layer
	os_init(nullptr, nullptr, nullptr, nullptr, 0);
	os_instbrk(true);

	char name[255];
	strcpy(name, getFilename().c_str());
	char *argv[2] = { nullptr, name };

	trdmain(2, argv, nullptr, ".sav");

	os_instbrk(false);
	os_uninit();
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
