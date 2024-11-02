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
#include "freescape/games/castle/castle.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

Graphics::ManagedSurface *CastleEngine::loadFrameFromPlanesVertical(Common::SeekableReadStream *file, int widthInBytes, int height) {
	Graphics::ManagedSurface *surface;
	surface = new Graphics::ManagedSurface();
	surface->create(widthInBytes * 8 / 4, height, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, widthInBytes * 8 / 4, height), 0);
	loadFrameFromPlanesInternalVertical(file, surface, widthInBytes / 4, height, 0);
	loadFrameFromPlanesInternalVertical(file, surface, widthInBytes / 4, height, 1);
	loadFrameFromPlanesInternalVertical(file, surface, widthInBytes / 4, height, 2);
	loadFrameFromPlanesInternalVertical(file, surface, widthInBytes / 4, height, 3);
	return surface;
}

Graphics::ManagedSurface *CastleEngine::loadFrameFromPlanesInternalVertical(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int width, int height, int plane) {
	byte *colors = (byte *)malloc(sizeof(byte) * height * width);
	file->read(colors, height * width);

	for (int i = 0; i < height * width; i++) {
		byte color = colors[i];
		for (int n = 0; n < 8; n++) {
			int y = i / width;
			int x = (i % width) * 8 + (7 - n);

			int bit = ((color >> n) & 0x01) << plane;
			int sample = surface->getPixel(x, y) | bit;
			assert(sample < 16);
			surface->setPixel(x, y, sample);
		}
	}
	free(colors);
	return surface;
}

void CastleEngine::loadAssetsAmigaDemo() {
	Common::File file;
	file.open("x");
	if (!file.isOpen())
		error("Failed to open 'x' file");

	_viewArea = Common::Rect(40, 29, 280, 154);
	loadMessagesVariableSize(&file, 0x8bb2, 178);
	loadRiddles(&file, 0x96c8 - 2 - 19 * 2, 19);

	file.seek(0x3c6d0);
	byte *borderPalete = loadPalette(&file);

	file.seek(0x11eec);
	Common::Array<Graphics::ManagedSurface *> chars;
	for (int i = 0; i < 90; i++) {
		Graphics::ManagedSurface *img = loadFrameFromPlanes(&file, 8, 8);
		//Graphics::ManagedSurface *imgRiddle = new Graphics::ManagedSurface();
		//imgRiddle->copyFrom(*img);

		chars.push_back(img);
		chars[i]->convertToInPlace(_gfx->_texturePixelFormat, borderPalete, 16);

		//charsRiddle.push_back(imgRiddle);
		//charsRiddle[i]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGARiddleFontPalette, 16);
	}
	_font = Font(chars);
	_font.setCharWidth(9);

	load8bitBinary(&file, 0x162a6, 16);
	loadPalettes(&file, 0x151a6);

	file.seek(0x2be96); // Area 255
	_areaMap[255] = load8bitArea(&file, 16);

	file.seek(0x2cf28 + 0x28 - 0x2 + 0x28);
	_border = loadFrameFromPlanesVertical(&file, 160, 200);
	_border->convertToInPlace(_gfx->_texturePixelFormat, borderPalete, 16);
	delete[] borderPalete;
	file.close();

	_areaMap[2]->_groundColor = 1;
	for (auto &it : _areaMap)
		it._value->addStructure(_areaMap[255]);
}

void CastleEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
	drawStringInSurface(_currentArea->_name, 97, 182, 0, 0, surface);
}

} // End of namespace Freescape