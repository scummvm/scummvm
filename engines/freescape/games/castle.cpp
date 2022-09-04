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
#include "common/memstream.h"

#include "freescape/freescape.h"

namespace Freescape {

CastleEngine::CastleEngine(OSystem *syst) : FreescapeEngine(syst) {
	_playerHeight = 48;
	_playerWidth = 8;
	_playerDepth = 8;
}

void CastleEngine::loadAssets() {
	Common::SeekableReadStream *file = nullptr;
	Common::String path = ConfMan.get("path");
	Common::FSDirectory gameDir(path);

	_renderMode = "ega";

	file = gameDir.createReadStreamForMember("CMEDF");
	int size = file->size();
	byte *encryptedBuffer = (byte*) malloc(size);
	file->read(encryptedBuffer, size);

	int seed = 24;
	for (int i = 0; i < size; i++) {
		encryptedBuffer[i] ^= seed;
		seed = (seed + 1) & 0xff;
    }

	file = new Common::MemoryReadStream(encryptedBuffer, size);
	load8bitBinary(file, 0, 16);

	// CPC
	//file = gameDir.createReadStreamForMember("cm.bin");
	//if (file == nullptr)
	//	error("Failed to open cm.bin");
	//load8bitBinary(file, 0x791a, 16);
}

} // End of namespace Freescape