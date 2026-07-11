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

#include "comfy/comfy.h"

namespace Comfy {

bool ComfyEngine::sceneOpen() {
	if (_sceneOpen)
		sceneClose();

	if (_comfyObjData.empty() || _picFileData.empty() || _midiFileData.empty() || !midiPlyrStart())
		return false;

	paletteLoadWithFade(0, 0);
	framebufClear(0);
	int16 y = 0;
	for (uint spriteId = 1; spriteId < _spriteHeaders.size() && y < int16(_logicalScreenHeight); spriteId++) {
		SpriteObjectHeader &header = _spriteHeaders[spriteId];
		if (header.width != _logicalScreenWidth || !header.height || y + header.height > _logicalScreenHeight)
			continue;

		spriteBlitClipped(spriteId, 0, y);
		y += header.height;
	}

	_sceneOpen = true;
	renderSetDirty();
	return true;
}

void ComfyEngine::sceneClose() {
	if (!_sceneOpen)
		return;

	midiPlyrStop();
	for (uint i = 0; i < _spriteResources.size(); i++) {
		_spriteResources[i].pixels.clear();
		_spriteResources[i].loaded = false;
	}

	_sceneOpen = false;
}


} // End of namespace Comfy
