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

#include "common/rect.h"

#include "scumm/scumm_v7.h"
#include "scumm/smush/rebel/smush_multi_font.h"
#include "scumm/smush/smush_player.h"

namespace Scumm {

void SmushPlayer::ensureMultiFont() {
	if (!_multiFont)
		_multiFont = new SmushMultiFont(_vm, this, true);
}

void SmushPlayer::addMaskedRegion(const Common::Rect &rect) {
	for (Common::List<Common::Rect>::iterator it = _maskedRegions.begin(); it != _maskedRegions.end(); ++it) {
		if (*it == rect)
			return;
	}
	_maskedRegions.push_back(rect);
}

void SmushPlayer::removeMaskedRegion(const Common::Rect &rect) {
	for (Common::List<Common::Rect>::iterator it = _maskedRegions.begin(); it != _maskedRegions.end(); ++it) {
		if (*it == rect) {
			_maskedRegions.erase(it);
			return;
		}
	}
}

void SmushPlayer::clearMaskedRegions() {
	_maskedRegions.clear();
}

void SmushPlayer::setScrollOffset(int x, int y) {
	_scrollX = MAX(0, x);
	_scrollY = MAX(0, y);
}

void SmushPlayer::adjustFrameCoords(int &left, int &top, int &width, int &height, int pitch, int *srcSkipY) {
	left += _fobjOffsetX;
	top += _fobjOffsetY;

	int bufHeight = (_dst == _specialBuffer) ? _height : _vm->_screenHeight;
	if (top < 0) {
		if (srcSkipY)
			*srcSkipY = -top;
		height += top;
		top = 0;
	}
	if (left < 0) {
		width += left;
		left = 0;
	}
	if (top + height > bufHeight)
		height = bufHeight - top;
	if (left + width > pitch)
		width = pitch - left;
}

void SmushPlayer::rememberLastFobj(int codec, const byte *data, int32 dataSize,
		int left, int top, int width, int height) {
	if (dataSize <= 0) {
		_hasFrameFobjForGost = false;
		return;
	}

	byte *newData = (byte *)realloc(_lastFobjData, dataSize);
	if (newData == nullptr) {
		warning("SmushPlayer::rememberLastFobj: Failed to allocate %d bytes", dataSize);
		free(_lastFobjData);
		_lastFobjData = nullptr;
		_lastFobjDataSize = 0;
		_hasFrameFobjForGost = false;
		return;
	}

	_lastFobjData = newData;
	memcpy(_lastFobjData, data, dataSize);
	_lastFobjDataSize = dataSize;
	_lastFobjCodec = codec;
	_lastFobjLeft = left;
	_lastFobjTop = top;
	_lastFobjWidth = width;
	_lastFobjHeight = height;
	_hasFrameFobjForGost = true;
}

} // End of namespace Scumm
