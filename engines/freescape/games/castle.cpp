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
	_playerHeightNumber = 1;
	_playerHeights.push_back(16);
	_playerHeights.push_back(48);
	_playerHeight = _playerHeights[_playerHeightNumber];

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

void CastleEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);
	if (!_gameStateBits.contains(areaID))
		_gameStateBits[areaID] = 0;

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	int scale = _currentArea->getScale();
	assert(scale > 0);

	Entrance *entrance = nullptr;
	if (entranceID > 0) {
		entrance = (Entrance*) _currentArea->entranceWithID(entranceID);

		assert(entrance);
		_position = entrance->getOrigin();

		if (_rotation == Math::Vector3d(0, 0, 0)) {
			_rotation = entrance->getRotation();
			_pitch = _rotation.x();
			_yaw = _rotation.y() - 260;
		}
		debugC(1, kFreescapeDebugMove, "entrace position: %f %f %f", _position.x(), _position.y(), _position.z());
		debugC(1, kFreescapeDebugMove, "player height: %d", scale * _playerHeight);
		_position.setValue(1, _position.y() + scale * _playerHeight);
	}

	debugC(1, kFreescapeDebugMove, "starting player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	playSound(5);

	if (_currentArea->skyColor > 0) {
		_gfx->_keyColor = 0;
		_gfx->setSkyColor(_currentArea->skyColor);
	} else
		_gfx->_keyColor = 255;
}


} // End of namespace Freescape