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

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

static const char *rawMessagesTable[] = {
	"HEART  FAILURE",
	"SUN ECLIPSED",
	"CRUSHED TO DEATH",
	"FATAL FALL",
	"CURSE OVERCOME",
	"TOTAL ECLIPSE",
	"TOO HOT TO REST!",
	"RESTING...",
	" ANKH FOUND ",
	"WAY  BLOCKED",
	"5 ANKHS REQUIRED",
	"$2M REWARD",
	"MAKE THE MATCH",
	"TOUCH TO COLLECT",
	"NO ENTRY",
	"REMOVE LID",
	"POISON AIR",
	"MATCH MADE",
	NULL};

EclipseEngine::EclipseEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {
	_viewArea = Common::Rect(40, 32, 280, 132);
	_playerHeightNumber = 1;
	_playerHeights.push_back(16);
	_playerHeights.push_back(48);
	_playerHeight = _playerHeights[_playerHeightNumber];

	_playerWidth = 8;
	_playerDepth = 8;

	const char **messagePtr = rawMessagesTable;
	debugC(1, kFreescapeDebugParser, "String table:");
	while (*messagePtr) {
		Common::String message(*messagePtr);
		_messagesList.push_back(message);
		debugC(1, kFreescapeDebugParser, "%s", message.c_str());
		messagePtr++;
	}
}

extern byte kEGADefaultPaletteData[16][3];

void EclipseEngine::loadAssetsDOSFullGame() {
	Common::File file;
	if (_renderMode == Common::kRenderEGA) {
		file.open("SCN1E.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte *)&kEGADefaultPaletteData, 0, 16);
		}
		file.close();
		file.open("TOTEE.EXE");

		if (!file.isOpen())
			error("Failed to open TOTEE.EXE");

		loadFonts(&file, 0xd403);
		load8bitBinary(&file, 0x3ce0, 16);
		for (auto &it : _areaMap) {
			it._value->addStructure(_areaMap[255]);
			for (int16 id = 183; id < 207; id++)
				it._value->addObjectFromArea(id, _areaMap[255]);
		}
		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte *)&kEGADefaultPaletteData, 0, 16);
	} else if (_renderMode == Common::kRenderCGA) {
		file.open("TOTEC.EXE");

		if (!file.isOpen())
			error("Failed to open TOTEC.EXE");
		load8bitBinary(&file, 0x7bb0, 4); // TODO
	} else
		error("Invalid or unsupported render mode %s for Total Eclipse", Common::getRenderModeDescription(_renderMode));
}

void EclipseEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);
	if (!_gameStateBits.contains(areaID))
		_gameStateBits[areaID] = 0;

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	_currentAreaMessages.clear();
	_currentAreaMessages.push_back(_currentArea->_name);

	if (entranceID == -1)
		return;

	assert(entranceID > 0);
	traverseEntrance(entranceID);

	_lastPosition = _position;

	if (_currentArea->_skyColor > 0 && _currentArea->_skyColor != 255) {
		_gfx->_keyColor = 0;
	} else
		_gfx->_keyColor = 255;
}

void EclipseEngine::drawUI() {
	_gfx->setViewport(_fullscreenViewArea);

	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
	uint32 gray = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0xA0, 0xA0, 0xA0);
	surface->fillRect(_fullscreenViewArea, gray);
	drawCrossair(surface);

	int score = _gameStateVars[k8bitVariableScore];
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0x55);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);

	if (!_currentAreaMessages.empty())
		drawStringInSurface(_currentAreaMessages[0], 102, 135, black, yellow, surface);
	drawStringInSurface(Common::String::format("%07d", score), 136, 6, black, white, surface);

	drawFullscreenSurface(surface);
	surface->free();
	delete surface;

	_gfx->setViewport(_viewArea);
}

Common::Error EclipseEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	return Common::kNoError;
}

Common::Error EclipseEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	return Common::kNoError;
}

} // End of namespace Freescape
