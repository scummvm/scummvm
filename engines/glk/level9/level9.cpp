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

#include "glk/level9/level9.h"
#include "glk/level9/level9_main.h"
#include "glk/level9/os_glk.h"

namespace Glk {
namespace Level9 {

Level9 *g_vm = nullptr;

Level9::Level9(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		_detection(startdata, FileSize) {
	g_vm = this;
}

void Level9::runGame() {
	initialize();

	_gameFile.close();
	gln_main(getFilename().c_str());

	deinitialize();
}

bool Level9::initialize() {
	gln_initialize();
	return gln_startup_code(0, nullptr);
}

void Level9::deinitialize() {
}

Common::Error Level9::readSaveData(Common::SeekableReadStream *rs) {
	// TODO
	return Common::kNoError;
}

Common::Error Level9::writeGameData(Common::WriteStream *ws) {
	// TODO
	return Common::kNoError;
}

} // End of namespace Level9
} // End of namespace Glk
