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

#include "darkseed/darkseed.h"
#include "darkseed/big5font.h"

namespace Darkseed {

Big5Font::Big5Font() {
	Common::File tosFile;
	if (!tosFile.open("tos.exe")) { // TODO handle packed tos.exe
		error("Error: failed to open tos.exe");
	}
	tosFile.seek(0x2701);
	_big5.loadPrefixedRaw(tosFile, 15);
	tosFile.close();
}

int Big5Font::getFontHeight() const {
	return 15;
}

int Big5Font::getMaxCharWidth() const {
	return 17;
}

int Big5Font::getCharWidth(uint32 chr) const {
	return getMaxCharWidth();
}

void Big5Font::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	Common::Point charPos = {(int16)x, (int16)y};
	if (_big5.drawBig5Char(g_engine->_screen->surfacePtr(), chr, charPos, 0xf)) {
		charPos.x++;
		_big5.drawBig5Char(g_engine->_screen->surfacePtr(), chr, charPos, 0xc);
		charPos.x += Graphics::Big5Font::kChineseTraditionalWidth + 1;
	}
}

} // namespace Darkseed
