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

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

static const entrancesTableEntry rawEntranceTable[] = {
	{183, {36, 137, 13}}, // Correct?
	{184, {36, 137, 13}}, // TODO
	{185, {204, 68, 66}},
	{186, {204, 88, 66}},
	{187, {36, 137, 13}}, // TODO
	{188, {352, 105, 204}},
	{190, {36, 137, 13}}, // TODO
	{191, {49, 7, 23}}, // TODO
	{192, {36, 137, 13}}, // TODO
	{193, {36, 137, 13}}, // TODO
	{194, {36, 137, 13}}, // TODO
	{195, {36, 137, 13}}, // TODO
	{196, {36, 137, 13}}, // <-
	{197, {203, 0, 31}},  // TODO
	{198, {36, 137, 13}}, // TODO
	{199, {36, 137, 13}}, // TODO
	{200, {36, 137, 13}}, // TODO
	{201, {36, 137, 13}}, // TODO
	{202, {360, 25, 373}},
	{203, {207, 25, 384}},
	{204, {33, 48, 366}},
	{206, {25, 8, 200}},
	{0, {0, 0, 0}},        // NULL
};

static const char* rawMessagesTable[] = {
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
	NULL
};

EclipseEngine::EclipseEngine(OSystem *syst) : FreescapeEngine(syst) {
	_viewArea = Common::Rect(40, 32, 280, 132);
	_playerHeightNumber = 1;
	_playerHeights.push_back(16);
	_playerHeights.push_back(48);
	_playerHeight = _playerHeights[_playerHeightNumber];

	_playerWidth = 8;
	_playerDepth = 8;

	const entrancesTableEntry *entry = rawEntranceTable;
	while (entry->id) {
		_entranceTable[entry->id] = entry;
		entry++;
	}

	const char **messagePtr = rawMessagesTable;
	debugC(1, kFreescapeDebugParser, "String table:");
	while (*messagePtr) {
		Common::String message(*messagePtr);
		_messagesList.push_back(message);
		debugC(1, kFreescapeDebugParser, "%s", message.c_str());
		messagePtr++;
	}
}

void EclipseEngine::loadAssets() {
	Common::SeekableReadStream *file = nullptr;
	Common::String path = ConfMan.get("path");
	Common::FSDirectory gameDir(path);

	Common::File exe;
	if (_renderMode == "ega") {
		file = gameDir.createReadStreamForMember("TOTEE.EXE");

		if (file == nullptr)
			error("Failed to open TOTEE.EXE");

		load8bitBinary(file, 0x3ce0, 16);
	} else if (_renderMode == "cga") {
		file = gameDir.createReadStreamForMember("TOTEC.EXE");

		if (file == nullptr)
			error("Failed to open TOTEC.EXE");
		load8bitBinary(file, 0x7bb0, 4); // TODO
	} else
		error("Invalid render mode %s for Total Eclipse", _renderMode.c_str());

}

void EclipseEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);
	if (!_gameStateBits.contains(areaID))
		_gameStateBits[areaID] = 0;

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	_currentAreaMessages.clear();
	_currentAreaMessages.push_back(_currentArea->name);

	int scale = _currentArea->getScale();
	assert(scale > 0);

	Entrance *entrance = nullptr;
	if (entranceID == -1)
		return;

	assert(entranceID > 0);

	entrance = (Entrance*) _currentArea->entranceWithID(entranceID);

	if (!entrance) {
		assert(_entranceTable.contains(entranceID));
		const entrancesTableEntry *entry = _entranceTable[entranceID];
		_position = scale * Math::Vector3d(entry->position[0], entry->position[1], entry->position[2]);
		_position.setValue(1, _position.y() + scale * _playerHeight);
		debugC(1, kFreescapeDebugMove, "entrace position: %f %f %f", _position.x(), _position.y(), _position.z());
		debugC(1, kFreescapeDebugMove, "player height: %d", scale * _playerHeight);
	} else
		traverseEntrance(entranceID);

	_lastPosition = _position;

	if (_currentArea->skyColor > 0 && _currentArea->skyColor != 255) {
		_gfx->_keyColor = 0;
		_gfx->setSkyColor(_currentArea->skyColor);
	} else
		_gfx->_keyColor = 255;
}


void EclipseEngine::drawUI() {
	_gfx->renderCrossair(0);
	_gfx->setViewport(_fullscreenViewArea);

	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_currentPixelFormat);
	surface->fillRect(_fullscreenViewArea, 0xA0A0A0FF);

	int score = _gameStateVars[k8bitVariableScore];
	uint32 yellow = 0xFFFF55FF;
	uint32 black = 0x000000FF;
	uint32 white = 0xFFFFFFFF;

	if (!_currentAreaMessages.empty())
		drawStringInSurface(_currentAreaMessages[0], 102, 135, black, yellow, surface);
	drawStringInSurface(Common::String::format("%07d", score), 136, 6, black, white, surface);

	Texture *texture = _gfx->createTexture(surface);
	_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, texture);
	surface->free();
	delete surface;

	_gfx->setViewport(_viewArea);
}


} // End of namespace Freescape