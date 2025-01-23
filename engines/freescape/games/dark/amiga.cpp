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
#include "freescape/games/dark/dark.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

extern Common::String centerAndPadString(const Common::String &str, int size);

void DarkEngine::loadAssetsAmigaFullGame() {
	Common::File file;
	file.open("0.drk");
	_title = loadAndConvertNeoImage(&file, 0x9930);
	file.close();

	Common::SeekableReadStream *stream = decryptFileAmigaAtari("1.drk", "0.drk", 798);
	parseAmigaAtariHeader(stream);

	_border = loadAndConvertNeoImage(stream, 0x1b762);
	load8bitBinary(stream, 0x2e96a, 16);
	loadPalettes(stream, 0x2e528);
	loadGlobalObjects(stream, 0x30f0 - 50, 24);
	loadMessagesVariableSize(stream, 0x3d37, 66);
	loadSoundsFx(stream, 0x34738 + 2, 11);

	Common::Array<Graphics::ManagedSurface *> chars;
	chars = getCharsAmigaAtariInternal(8, 8, - 7 - 8, 16, 16, stream, 0x1b0bc, 85);
	_fontBig = Font(chars);

	chars = getCharsAmigaAtariInternal(8, 8, 0, 10, 8, stream, 0x1b0bc + 0x430, 85);
	_fontMedium = Font(chars);

	chars = getCharsAmigaAtariInternal(8, 5, - 7 - 8, 10, 16, stream, 0x1b0bc + 0x430, 85);
	_fontSmall = Font(chars);
	_fontSmall.setCharWidth(4);

	_fontLoaded = true;

	GeometricObject *obj = nullptr;
	obj = (GeometricObject *)_areaMap[15]->objectWithID(18);
	assert(obj);
	obj->_cyclingColors = true;

	obj = (GeometricObject *)_areaMap[15]->objectWithID(26);
	assert(obj);
	obj->_cyclingColors = true;

	for (int i = 0; i < 3; i++) {
		int16 id = 227 + i * 6 - 2;
		for (int j = 0; j < 2; j++) {
			//debugC(1, kFreescapeDebugParser, "Restoring object %d to from ECD %d", id, index);
			obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
			assert(obj);
			obj->_cyclingColors = true;
			id--;
		}
	}

	for (auto &area : _areaMap) {
		// Center and pad each area name so we do not have to do it at each frame
		area._value->_name = centerAndPadString(area._value->_name, 26);
	}
}

void DarkEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xEE, 0xCC, 0x00);
	uint32 orange = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xEE, 0x88, 0x00);
	uint32 red = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xEE, 0x00, 0x00);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
	uint32 grey = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x60, 0x60, 0x60);

	uint32 grey8 = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x88, 0x88, 0x88);
	uint32 greyA = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xAA, 0xAA, 0xAA);

	int score = _gameStateVars[k8bitVariableScore];
	int ecds = _gameStateVars[kVariableActiveECDs];
	drawString(kDarkFontSmall, Common::String::format("%04d", int(2 * _position.x())), 19, 178, red, red, black, surface);
	drawString(kDarkFontSmall, Common::String::format("%04d", int(2 * _position.z())), 19, 184, red, red, black, surface);
	drawString(kDarkFontSmall, Common::String::format("%04d", int(2 * _position.y())), 19, 190, red, red, black, surface);

	drawString(kDarkFontBig, Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 73, 178, red, red, black, surface);
	drawString(kDarkFontBig, Common::String::format("%3d", _playerSteps[_playerStepIndex]), 73, 186, red, red, black, surface);
	drawString(kDarkFontBig, Common::String::format("%07d", score), 93, 16, orange, yellow, black, surface);
	drawString(kDarkFontBig, Common::String::format("%3d%%", ecds), 181, 16, orange, yellow, black, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawString(kDarkFontSmall, message, 32, 157, grey8, greyA, transparent, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	}

	drawString(kDarkFontSmall, _currentArea->_name, 32, 151, grey8, greyA, transparent, surface);
	drawBinaryClock(surface, 6, 110, white, grey);

	int x = 229;
	int y = 180;
	for (int i = 0; i < _maxShield / 2; i++) {
		if (i < _gameStateVars[k8bitVariableShield] / 2) {
			surface->drawLine(x, y, x, y + 3, orange);
			surface->drawLine(x, y + 1, x, y + 2, yellow);
		} else
			surface->drawLine(x, y, x, y + 3, red);
		x += 2;
	}

	x = 229;
	y = 188;
	for (int i = 0; i < _maxEnergy / 2; i++) {
		if (i < _gameStateVars[k8bitVariableEnergy] / 2) {
			surface->drawLine(x, y, x, y + 3, orange);
			surface->drawLine(x, y + 1, x, y + 2, yellow);
		} else
			surface->drawLine(x, y, x, y + 3, red);
		x += 2;
	}
}

void DarkEngine::initAmigaAtari() {
	_viewArea = Common::Rect(32, 33, 287, 130);
}

void DarkEngine::drawString(const DarkFontSize size, const Common::String &str, int x, int y, uint32 primaryColor, uint32 secondaryColor, uint32 backColor, Graphics::Surface *surface) {
	if (!_fontLoaded)
		return;

	Font *font = nullptr;

	if (size == kDarkFontBig) {
		font = &_fontBig;
	} else if (size == kDarkFontMedium) {
		font = &_fontMedium;
	} else if (size == kDarkFontSmall) {
		font = &_fontSmall;
	} else {
		error("Invalid font size %d", size);
		return;
	}

	Common::String ustr = str;
	ustr.toUppercase();
	font->setBackground(backColor);
	font->setSecondaryColor(secondaryColor);
	font->drawString(surface, ustr, x, y, _screenW, primaryColor);
}

} // End of namespace Freescape
