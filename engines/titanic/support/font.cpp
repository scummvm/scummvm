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

#include "common/textconsole.h"
#include "titanic/support/font.h"
#include "titanic/support/files_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

STFont::STFont() {
	_dataPtr = nullptr;
	_dataSize = 0;
	_fontHeight = 0;
	_dataWidth = 0;
	_fontR = _fontG = _fontB = 0;
}

STFont::~STFont() {
	delete[] _dataPtr;
}

void STFont::load(int fontNumber) {
	assert(!_dataPtr);
	Common::SeekableReadStream *stream = g_vm->_filesManager.getResource(
		Common::WinResourceID("STFONT"), fontNumber);
	if (!stream)
		error("Could not locate the specified font");

	_fontHeight = stream->readUint32LE();
	_dataWidth = stream->readUint32LE();
	for (uint idx = 0; idx < 256; ++idx)
		_chars[idx]._width = stream->readUint32LE();
	for (uint idx = 0; idx < 256; ++idx)
		_chars[idx]._offset = stream->readUint32LE();

	_dataSize = stream->readUint32LE();
	_dataPtr = new byte[_dataSize];
	stream->read(_dataPtr, _dataSize);

	delete stream;
}

void STFont::setColor(byte r, byte g, byte b) {
	_fontR = r;
	_fontG = g;
	_fontB = b;
}

uint16 STFont::getColor() const {
	return g_system->getScreenFormat().RGBToColor(_fontR, _fontG, _fontB);
}

void STFont::writeString(int maxWidth, const CString &text, int *v1, int *v2) {
	warning("TODO: STFont::writeString");
}

int STFont::stringWidth(const CString &text) const {
	if (text.empty())
		return 0;

	const char *srcP = text.c_str();
	int total = 0;
	char c;
	while (c = *srcP++) {
		if (c == 26) {
			// Skip over command parameter bytes
			srcP += 3;
		} else if (c == 27) {
			// Skip over command parameter bytes
			srcP += 4;
		} else if (c != '\n') {
			total += _chars[c]._width;
		}
	}

	return total;
}

int STFont::writeChar(CVideoSurface *surface, unsigned char c, const Common::Point &pt, Rect *destRect, Rect *srcRect) {
	if (c == 233)
		c = '$';

	Rect tempRect;
	tempRect.left = _chars[c]._offset;
	tempRect.right = _chars[c]._offset + _chars[c]._width;
	tempRect.top = 0;
	tempRect.bottom = _fontHeight;
	Point destPos(pt.x + destRect->left, pt.y + destRect->top);

	if (srcRect->isEmpty())
		srcRect = destRect;
	if (destPos.y > srcRect->bottom)
		return -2;

	if ((destPos.y + tempRect.height()) > srcRect->bottom) {
		tempRect.bottom += tempRect.top - destPos.y;
	}

	if (destPos.y < srcRect->top) {
		if ((tempRect.height() + destPos.y) < srcRect->top)
			return -1;

		tempRect.top += srcRect->top - destPos.y;
		destPos.y = srcRect->top;
	}

	if (destPos.x < srcRect->left) {
		if ((tempRect.width() + destPos.x) < srcRect->left)
			return -3;

		tempRect.left += srcRect->left - destPos.x;
		destPos.x = srcRect->left;
	} else {
		if ((tempRect.width() + destPos.x) > srcRect->right) {
			if (destPos.x > srcRect->right)
				return -4;

			tempRect.right += srcRect->left - destPos.x;
		}
	}

	copyRect(surface, destPos, tempRect);
	return 0;
}

void STFont::copyRect(CVideoSurface *surface, const Common::Point &pt, Rect &rect) {
	if (surface->lock()) {
		uint16 *lineP = surface->getBasePtr(pt.x, pt.y);
		uint16 color = getColor();

		for (int yp = rect.top; yp < rect.bottom; ++yp, lineP += surface->getPitch()) {
			uint16 *destP = lineP;
			for (int xp = rect.left; xp < rect.right; ++xp, ++destP) {
				const byte *srcP = _dataPtr + yp * _dataWidth + xp;
				//surface->changePixel(destP, color, *srcP >> 3, 1);
			}
		}

		surface->unlock();
	}
}

} // End of namespace Titanic
