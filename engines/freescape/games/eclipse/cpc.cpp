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
#include "common/memstream.h"

#include "freescape/freescape.h"
#include "freescape/games/eclipse/ay.music.h"
#include "freescape/games/eclipse/eclipse.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void EclipseEngine::initCPC() {
	_viewArea = Common::Rect(36 + 3, 24 + 8, 284, 130 + 3);
	// Sound mappings from TEPROG.BIN disassembly (sub_6D19h call sites)
	_soundIndexShoot = 5;            // 0x5D80: LD A,05h; CALL 6D19h (type 0x16 destroy)
	_soundIndexCollide = 12;         // 0x5192/0x5239: deferred via (0CFD9h)
	_soundIndexStepDown = 12;        // 0x5194/0x5239: small height drop within threshold
	_soundIndexStepUp = 12;          // same sound for step up (matches ZX version)
	_soundIndexStart = 3;            // 0x770F/7726/776A: game start transition
	_soundIndexAreaChange = 7;       // 0x63E0: deferred via (0CFD9h), type 0x12
	_soundIndexStartFalling = 6;     // 0x797E: falling handler, first phase
	_soundIndexEndFalling = 8;       // 0x79AC: falling handler, landing
	_soundIndexFall = 5;             // 0x7D25: death/game-over animation
	_soundIndexNoShield = 5;         // game-over conditions reuse death sound
	_soundIndexFallen = 5;
	_soundIndexTimeout = 5;
	_soundIndexForceEndGame = 5;
	_soundIndexCrushed = 5;
}

byte kCPCPaletteEclipseTitleData[4][3] = {
	{0x00, 0x00, 0x00},
	{0xff, 0xff, 0x00},
	{0xff, 0x00, 0xff},
	{0xff, 0x80, 0x00},
};

byte kCPCPaletteEclipseBorderData[4][3] = {
	{0x00, 0x00, 0x00},
	{0xff, 0x80, 0x00},
	{0x80, 0xff, 0xff},
	{0x00, 0x80, 0x00},
};



void EclipseEngine::loadHeartFramesCPC(Common::SeekableReadStream *file, int restOffset, int beatOffset) {
	// Decode heart frames as indexed (CLUT8) pixel data.
	// The actual palette is applied at draw time from the current area's
	// ink/paper colors, since CPC pen assignments change per area.
	int offsets[2] = { beatOffset, restOffset };

	for (int f = 0; f < 2; f++) {
		file->seek(offsets[f]);
		int height = file->readByte();
		int widthBytes = file->readByte();

		auto *indexed = new Graphics::ManagedSurface();
		indexed->create(widthBytes * 4, height, Graphics::PixelFormat::createFormatCLUT8());
		loadFrameCPCIndexed(file, indexed, widthBytes, height);
		_heartFramesCPCIndexed.push_back(indexed);
	}
}

void EclipseEngine::loadAssetsCPCFullGame() {
	Common::File file;

	if (isEclipse2())
		file.open("TE2.BI1");
	else
		file.open("TESCR.SCR");

	if (!file.isOpen())
		error("Failed to open TESCR.SCR/TE2.BI1");

	_title = readCPCImage(&file, true);
	_title->setPalette((byte*)&kCPCPaletteEclipseTitleData, 0, 4);

	file.close();
	if (isEclipse2())
		file.open("TE2.BI3");
	else
		file.open("TECON.SCR");

	if (!file.isOpen())
		error("Failed to open TECON.SCR/TE2.BI3");

	_border = readCPCImage(&file, true);
	_border->setPalette((byte*)&kCPCPaletteEclipseBorderData, 0, 4);

	file.close();
	if (isEclipse2())
		file.open("TE2.BI2");
	else
		file.open("TECODE.BIN");

	if (!file.isOpen())
		error("Failed to open TECODE.BIN/TE2.BI2");

	if (isEclipse2()) {
		loadFonts(&file, 0x60bc);
		loadMessagesFixedSize(&file, 0x326, 16, 34);
		load8bitBinary(&file, 0x62b4, 16);
		loadSoundsCPC(&file, 0x0879, 104, 0x08E1, 165, 0x07E6, 147);
	} else {
		loadFonts(&file, 0x6076);
		loadMessagesFixedSize(&file, 0x326, 16, 30);
		load8bitBinary(&file, 0x626e, 16);
		loadSoundsCPC(&file, 0x07C9, 104, 0x0831, 165, 0x0736, 147);
	}

	loadColorPalette();
	swapPalette(1);

	if (isEclipse2()) {
		loadHeartFramesCPC(&file, 0x0D8B, 0x0DBD);
	} else {
		loadHeartFramesCPC(&file, 0x0CDB, 0x0D0D);
	}
	updateHeartFramesCPC();

	_indicators.push_back(loadBundledImage("eclipse_ankh_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);

	if (ConfMan.getBool("ay_music"))
		_playerMusic = new EclipseAYMusicPlayer(_mixer);
}

void EclipseEngine::loadAssetsCPCDemo() {
	Common::File file;

	file.open("TECON.BIN");
	if (!file.isOpen())
		error("Failed to open TECON.BIN");

	_border = readCPCImage(&file, true);
	_border->setPalette((byte*)&kCPCPaletteEclipseTitleData, 0, 4);

	file.close();
	file.open("TEPROG.BIN");

	if (!file.isOpen())
		error("Failed to open TEPROG.BIN");

	loadFonts(&file, 0x63ce);
	loadMessagesFixedSize(&file, 0x362, 16, 23);
	loadMessagesFixedSize(&file, 0x570b, 264, 5);
	load8bitBinary(&file, 0x65c6, 16);
	loadSoundsCPC(&file, 0x0805, 104, 0x086D, 165, 0x0772, 147);
	loadColorPalette();
	swapPalette(1);
	loadHeartFramesCPC(&file, 0x0D17, 0x0D49);
	updateHeartFramesCPC();

	// This patch forces a solid color to the bottom of the chest in the area 5
	// It was transparent in the original game
	GeometricObject *obj = (GeometricObject *)_areaMap[5]->objectWithID(12);
	assert(obj);
	obj->setColor(2, 4);

	_indicators.push_back(loadBundledImage("eclipse_ankh_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);

	if (ConfMan.getBool("ay_music"))
		_playerMusic = new EclipseAYMusicPlayer(_mixer);
}

void EclipseEngine::updateHeartFramesCPC() {
	if (_heartFramesCPCIndexed.empty())
		return;

	uint8 r, g, b;
	byte palette[4 * 3];
	for (int c = 0; c < 4; c++) {
		_gfx->selectColorFromFourColorPalette(c, r, g, b);
		palette[c * 3 + 0] = r;
		palette[c * 3 + 1] = g;
		palette[c * 3 + 2] = b;
	}

	for (auto &sprite : _eclipseSprites) {
		sprite->free();
		delete sprite;
	}
	_eclipseSprites.clear();

	for (uint i = 0; i < _heartFramesCPCIndexed.size(); i++) {
		Graphics::ManagedSurface clut8;
		clut8.copyFrom(*_heartFramesCPCIndexed[i]);
		clut8.setPalette(palette, 0, 4);

		Graphics::Surface *converted = _gfx->convertImageFormatIfNecessary(&clut8);
		auto *surf = new Graphics::ManagedSurface();
		surf->copyFrom(*converted);
		converted->free();
		delete converted;

		_eclipseSprites.push_back(surf);
	}
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
	int shield = _gameStateVars[k8bitVariableShield] * 100 / _maxShield;
	shield = shield < 0 ? 0 : shield;

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 102, 135, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else if (!_currentAreaMessages.empty())
		drawStringInSurface(_currentArea->_name, 102, 135, back, front, surface);

	drawScoreString(score, 136, 6, back, other, surface);

	int x = 171;
	if (shield < 10)
		x = 179;
	else if (shield < 100)
		x = 175;

	Common::String shieldStr = Common::String::format("%d", shield);
	drawStringInSurface(shieldStr, x, 162, back, other, surface);

	drawStringInSurface(shiftStr("0", 'Z' - '$' + 1 - _angleRotationIndex), 79, 135, back, front, surface);
	drawStringInSurface(shiftStr("3", 'Z' - '$' + 1 - _playerStepIndex), 63, 135, back, front, surface);
	drawStringInSurface(shiftStr("7", 'Z' - '$' + 1 - _playerHeightNumber), 240, 135, back, front, surface);

	if (_shootingFrames > 0) {
		drawStringInSurface(shiftStr("4", 'Z' - '$' + 1), 232, 135, back, front, surface);
		drawStringInSurface(shiftStr("<", 'Z' - '$' + 1), 240, 135, back, front, surface);
	}
	drawAnalogClock(surface, 90, 172, back, other, front);
	drawIndicator(surface, 45, 4, 12);
	drawEclipseIndicator(surface, 228, 0, front, other);

	int energy = _gameStateVars[k8bitVariableEnergy];
	if (energy < 0)
		energy = 0;

	_gfx->readFromPalette(_currentArea->_paperColor, r, g, b);
	uint32 waterColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Common::Rect jarBackground(124, 165, 148, 192);
	surface->fillRect(jarBackground, back);

	Common::Rect jarWater(124, 192 - energy, 148, 192);
	surface->fillRect(jarWater, waterColor);

	drawHeartIndicator(surface, 176, 168);

	surface->fillRect(Common::Rect(225, 168, 235, 187), front);
	drawCompass(surface, 229, 177, _yaw, 10, back);

}

} // End of namespace Freescape
