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

#include "common/config-manager.h"
#include "common/file.h"

#include "freescape/freescape.h"

namespace Freescape {

DarkEngine::DarkEngine(OSystem *syst) : FreescapeEngine(syst) {
	_playerHeight = 64;
	_playerWidth = 12;
	_playerDepth = 32;
}

void DarkEngine::loadAssets() {
	Common::SeekableReadStream *file = nullptr;
	Common::String path = ConfMan.get("path");
	Common::FSDirectory gameDir(path);

    Common::File exe;
    if (_renderMode == "ega") {
        file = gameDir.createReadStreamForMember("DSIDEE.EXE");

        if (file == nullptr)
            error("Failed to open DSIDEE.EXE");

        load8bitBinary(file, 0xa280, 16);
		_gfx->_missingColor = 0;
    } else if (_renderMode == "cga") {
        file = gameDir.createReadStreamForMember("DSIDEC.EXE");

        if (file == nullptr)
            error("Failed to open DSIDE.EXE");
        load8bitBinary(file, 0x7bb0, 4); // TODO
    } else
        error("Invalid render mode %s for Dark Side", _renderMode.c_str());
}

} // End of namespace Freescape