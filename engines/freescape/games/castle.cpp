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

#include "common/file.h"
#include "common/memstream.h"

#include "freescape/freescape.h"

namespace Freescape {

CastleEngine::CastleEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {
	_playerHeightNumber = 1;
	_playerHeights.push_back(16);
	_playerHeights.push_back(48);
	_playerHeight = _playerHeights[_playerHeightNumber];

	_playerWidth = 8;
	_playerDepth = 8;
}

Common::SeekableReadStream *CastleEngine::decryptFile(const Common::String filename) {
	Common::File file;
	file.open(filename);
	if (!file.isOpen())
		error("Failed to open %s", filename.c_str());

	int size = file.size();
	byte *encryptedBuffer = (byte *)malloc(size);
	file.read(encryptedBuffer, size);
	file.close();

	int seed = 24;
	for (int i = 0; i < size; i++) {
		encryptedBuffer[i] ^= seed;
		seed = (seed + 1) & 0xff;
	}

	return (new Common::MemoryReadStream(encryptedBuffer, size));
}

void CastleEngine::loadAssetsFullGame() {
	Common::SeekableReadStream *stream = nullptr;

	stream = decryptFile("CMLE");
	loadMessagesVariableSize(stream, 0x11, 164);
	delete stream;

	stream = decryptFile("CMEDF");
	load8bitBinary(stream, 0, 16);
	for (auto &it : _areaMap)
		it._value->addStructure(_areaMap[255]);
	delete stream;

	// CPC
	// file = gameDir.createReadStreamForMember("cm.bin");
	// if (file == nullptr)
	//	error("Failed to open cm.bin");
	// load8bitBinary(file, 0x791a, 16);
}

void CastleEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);
	if (!_gameStateBits.contains(areaID))
		_gameStateBits[areaID] = 0;

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	if (entranceID > 0)
		traverseEntrance(entranceID);

	playSound(5, false);
	_lastPosition = _position;

	if (_currentArea->_skyColor > 0 && _currentArea->_skyColor != 255) {
		_gfx->_keyColor = 0;
		_gfx->clear(_currentArea->_skyColor);
	} else
		_gfx->_keyColor = 255;
}

Common::Error CastleEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	return Common::kNoError;
}

Common::Error CastleEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	return Common::kNoError;
}

} // End of namespace Freescape
