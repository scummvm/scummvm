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
#include "common/rect.h"
#include "chewy/font.h"

namespace Chewy {

ChewyFont::ChewyFont(Common::String filename) {
	const uint32 headerFont = MKTAG('T', 'F', 'F', '\0');
	Common::File stream;

	stream.open(filename);

	uint32 header = stream.readUint32BE();

	if (header != headerFont)
		error("Invalid resource - %s", filename.c_str());

	stream.skip(4);	// total memory
	_count = stream.readUint16LE();
	_first = stream.readUint16LE();
	_last = stream.readUint16LE();
	_deltaX = _dataWidth = stream.readUint16LE();
	_dataHeight = stream.readUint16LE();

	_displayWidth = _dataWidth;
	_displayHeight = _dataHeight;

	_fontSurface.create(_dataWidth * _count, _dataHeight, Graphics::PixelFormat::createFormatCLUT8());

	int bitIndex = 7;

	byte curr = stream.readByte();

	for (uint n = 0; n < _count; n++) {
		for (uint y = 0; y < _dataHeight; y++) {
			byte *p = (byte *)_fontSurface.getBasePtr(n * _dataWidth, y);

			for (uint x = n * _dataWidth; x < n * _dataWidth + _dataWidth; x++) {
				*p++ = (curr & (1 << bitIndex)) ? 0 : 0xFF;

				bitIndex--;
				if (bitIndex < 0) {
					bitIndex = 7;
					curr = stream.readByte();
				}
			}
		}
	}
}

ChewyFont::~ChewyFont() {
	_fontSurface.free();
}

void ChewyFont::setDisplaySize(uint16 width, uint16 height) {
	_displayWidth = width;
	_displayHeight = height;
}

void ChewyFont::setDeltaX(uint16 deltaX) {
	_deltaX = deltaX;
}

Graphics::Surface *ChewyFont::getLine(const Common::String &texts) {
	Graphics::Surface *line = new Graphics::Surface();
	if (texts.size() == 0)
		return line;

	Common::Rect subrect(0, 0, _dataWidth, _dataHeight);
	line->create(texts.size() * _deltaX, _dataHeight, Graphics::PixelFormat::createFormatCLUT8());
	line->fillRect(Common::Rect(line->w, line->h), 0xFF);

	for (uint i = 0; i < texts.size(); i++) {
		subrect.moveTo(((byte)texts[i] - _first) * _dataWidth, 0);
		line->copyRectToSurface(_fontSurface, i * (_deltaX - 2), 0, subrect);
	}

	return line;
}

Graphics::Surface *FontMgr::getLine(const Common::String &texts) {
	return _font->getLine(texts);
}

} // namespace Chewy
