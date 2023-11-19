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
#include "freescape/games/eclipse/eclipse.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void EclipseEngine::initCPC() {
	_viewArea = Common::Rect(36 + 3, 24 + 8, 284, 130 + 3);
}

extern byte kCPCPaletteTitleData[4][3];
extern byte kCPCPaletteBorderData[4][3];

extern Graphics::ManagedSurface *readCPCImage(Common::SeekableReadStream *file, bool mode0);

void EclipseEngine::loadAssetsCPCDemo() {
	Common::File file;

	file.open("TECON.BIN");
	if (!file.isOpen())
		error("Failed to open TECON.BIN");

	_border = readCPCImage(&file, true);
	_border->setPalette((byte*)&kCPCPaletteTitleData, 0, 4);

	file.close();
	file.open("TEPROG.BIN");

	if (!file.isOpen())
		error("Failed to open TEPROG.BIN");

	loadFonts(&file, 0x63ce);
	loadMessagesFixedSize(&file, 0x362, 16, 23);
	loadMessagesFixedSize(&file, 0x570b, 264, 5);
	load8bitBinary(&file, 0x65c6, 16);
	for (auto &it : _areaMap) {
		it._value->_name = "  NOW TRAINING  ";
		it._value->addStructure(_areaMap[255]);
		for (int16 id = 183; id < 207; id++)
			it._value->addObjectFromArea(id, _areaMap[255]);
	}
	loadColorPalette();
	swapPalette(1);
	//_indicators.push_back(loadBundledImage("dark_fallen_indicator"));
	//_indicators.push_back(loadBundledImage("dark_crouch_indicator"));
	//_indicators.push_back(loadBundledImage("dark_walk_indicator"));
	//_indicators.push_back(loadBundledImage("dark_jet_indicator"));

	//for (auto &it : _indicators)
	//	it->convertToInPlace(_gfx->_texturePixelFormat);
}

void EclipseEngine::drawCPCUI(Graphics::Surface *surface) {
	uint32 color = _currentArea->_underFireBackgroundColor;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_inkColor;

	_gfx->readFromPalette(color, r, g, b);
	uint32 other = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];

	if (!_currentAreaMessages.empty())
		drawStringInSurface(_currentAreaMessages[0], 102, 135, back, front, surface);
	drawStringInSurface(Common::String::format("%08d", score), 136, 6, back, other, surface);
}

} // End of namespace Freescape
