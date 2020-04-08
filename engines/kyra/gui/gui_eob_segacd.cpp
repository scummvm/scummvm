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


#ifdef ENABLE_EOB

#include "kyra/engine/eob.h"
#include "kyra/graphics/screen_eob.h"
#include "kyra/graphics/screen_eob_segacd.h"
#include "kyra/resource/resource.h"
#include "kyra/resource/resource_segacd.h"

namespace Kyra {

void EoBEngine::gui_drawPlayField(bool refresh) {
	if (_flags.platform != Common::kPlatformSegaCD) {
		EoBCoreEngine::gui_drawPlayField(refresh);
		return;
	}

	//if (!_loading)
		_screen->sega_fadeToBlack(1);

	// transposeScreenOutputY(8);
	_txt->clearDim(0);
	_screen->sega_getAnimator()->clearSprites();
	SegaRenderer *r = _screen->sega_getRenderer();

	uint8 *data = _res->fileData("PLAYFLD", 0);
	for (int i = 0; i < 256; ++i)
		r->loadToVRAM(&data[i << 5], 32, _addrTbl1[i] << 5);
	memcpy(_compassData, data + 0x2000, 0x5000);
	delete[] data;

	const uint16 *pattern = _playFldPattern1;
	uint16 *dst = _playFldPattern2;

	for (int i = 0; i < 1040; ++i) {
		int ix = (*pattern++) - 11;
		*dst++ = (ix < 0) ? 0 : _addrTbl1[ix];
	}

	const uint16 ps[] = { 0xCE, 0xE0, 0x2FE, 0x310, 0x52E, 0x540 };

	for (int i = 0; i < 4; ++i) {
		dst = &_playFldPattern2[ps[i] >> 1];
		memset(dst, 0, 8);
		memset(&dst[40], 0, 8);
		memset(&dst[80], 0, 8);
		memset(&dst[120], 0, 8);
	}

	r->fillRectWithTiles(1, 0, 0, 40, 26, 0x2000, true, false, _playFldPattern2);
	r->fillRectWithTiles(0, 0, 21, 40, 5, 0x2000, true, false, _textFieldPattern);

	// Name tables for scene window vcn block tiles. We don't need that. We draw the blocks with our "normal" graphics code.
	// r->fillRectWithTiles(1, 0, 0, 22, 15, 0xC14B, true, true);
	// Name tables for scene window shapes tiles. We don't need that, since we're not going to draw any shapes with the renderer.
	// r->fillRectWithTiles(0, 0, 1, 22, 14, 0xE295, true, true);

	// Text field tiles
	r->fillRectWithTiles(0, 1, 22, 35, 3, 0x2597, true);
	r->render(0);

	_sres->loadContainer("ITEM");
	Common::SeekableReadStreamEndian *str = _sres->resStreamEndian(7);
	r->loadStreamToVRAM(str, 0x8880, true);
	delete str;
	str = _sres->resStreamEndian(9);
	r->loadStreamToVRAM(str, 0xA4A0, false);
	delete str;

	r->fillRectWithTiles(1, 22, 0, 18, 21, 0x6444, true, true, _invPattern);
	r->render(2);
	r->fillRectWithTiles(1, 22, 0, 18, 21, 0x6444, true, true, _statsPattern);
	r->render(Screen_EoB::kSegaRenderPage);

	if (refresh && !_sceneDrawPage2)
		drawScene(0);

	_screen->copyRegion(184, 1, 176, 168, guiSettings()->charBoxCoords.boxWidth, 24, 0, 2, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(184, 25, 240, 168, guiSettings()->charBoxCoords.boxWidth, guiSettings()->charBoxCoords.boxHeight - 24, 0, 2, Screen::CR_NO_P_CHECK);
	_screen->copyRegionToBuffer(0, 173, 0, 6, 120, _shakeBackBuffer1);
	_screen->copyRegionToBuffer(0, 0, 117, 179, 6, _shakeBackBuffer2);

	// Since we're no going to draw the character portrait boxes with the SegaRenderer but rather with our "normal" code, we have to backup
	// some parts of the background between the character portraits. Unlike in the other versions the red splat shapes overlaps with that space.
	for (int i = 0; i < 6; ++i) {
		delete[] _redSplatBG[i];
		_redSplatBG[i] = new uint8[_redSplatShape[2] << 5];
		_screen->copyRegionToBuffer(0, guiSettings()->charBoxCoords.boxX[i & 1] + guiSettings()->charBoxCoords.redSplatOffsetX, guiSettings()->charBoxCoords.boxY[i >> 1] + guiSettings()->charBoxCoords.boxHeight - 1, _redSplatShape[2] << 3, 4, _redSplatBG[i]);
	}
	for (int i = 2; i < 4; ++i) {
		if (_characters[i + 2].flags & 1)
			memcpy(_redSplatBG[i] + _redSplatShape[2] * 24, _redSplatBG[0] + _redSplatShape[2] * 24, _redSplatShape[2] << 3);
	}

	_compassDirection2 = -1;
	gui_drawCompass(false);;

	//if (!_loading)
		_screen->sega_fadeToNeutral(1);
}

void EoBEngine::gui_drawWeaponSlotStatus(int x, int y, int status) {
	if (_flags.platform != Common::kPlatformSegaCD) {
		EoBCoreEngine::gui_drawWeaponSlotStatus(x, y, status);
		return;
	}

	if (status < 0) {
		_screen->drawShape(_screen->_curPage, _weaponSlotShapes[status < -2 ? -status - 1 : 3 - status], x - 1, y, 0);
	} else {
		_screen->drawShape(_screen->_curPage, _weaponSlotShapes[0], x - 1, y, 0);
		gui_printInventoryDigits(x + 8, y + 6, status);
	}
}

void EoBEngine::gui_printInventoryDigits(int x, int y, int val) {
	if (_flags.platform != Common::kPlatformSegaCD) {
		EoBCoreEngine::gui_printInventoryDigits(x, y, val);
		return;
	}
	_screen->drawShape(_screen->_curPage, _invSmallDigits[(val < 10) ? 22 + val : (val >= 100 ? 1 : 2 + val / 10)], x, y);
	_screen->drawShape(_screen->_curPage, (val >= 10 && val < 100) ? _invSmallDigits[12 + (val % 10)] : 0, x, y);
}

void EoBEngine::gui_drawCharacterStatsPage() {
	SegaRenderer *r = _screen->sega_getRenderer();
	EoBCharacter *c = &_characters[_updateCharNum];

	memset(_statsPattern2, 0, 792);
	for (int i = 0; i < 11; ++i) {
		_statsPattern2[5 * 18 + i + 1] = 0x6555 + i;
		_statsPattern2[6 * 18 + i + 1] = 0x6565 + i;
	}

	for (int i = 0; i < 4; i++)
		printStatsString(_chargenStatStrings[6 + i], 1, 11 + i);
	printStatsString(_chargenStatStrings[10], 7, 16);

	printStatsString(_chargenClassStrings[c->cClass == 9 ? 27 : (c->cClass == 12 ? 28 : c->cClass)], 2, 7);
	printStatsString(_chargenAlignmentStrings[c->alignment], 2, 8);
	printStatsString(_chargenRaceSexStrings[c->raceSex], 2, 9);

	// FIXME? For now I have kept the exact original layout. If the character has a strengthExt stat there will be
	// no space left between the digits and the right stats row ("DEX" etc.). Maybe I should move that row one tile
	// further to the right? There is space enough left over there.
	printStatsString(getCharStrength(c->strengthCur, c->strengthExtCur).c_str(), c->strengthExtCur ? 4 : 5, 11);
	printStatsString(Common::String::format("%2d", c->intelligenceCur).c_str(), 5, 12);
	printStatsString(Common::String::format("%2d", c->wisdomCur).c_str(), 5, 13);
	printStatsString(Common::String::format("%2d", c->dexterityCur).c_str(), 13, 11);
	printStatsString(Common::String::format("%2d", c->constitutionCur).c_str(), 13, 12);
	printStatsString(Common::String::format("%2d", c->charismaCur).c_str(), 13, 13);
	printStatsString(Common::String::format("%2d", c->armorClass).c_str(), 5, 14);

	for (int i = 0; i < 3; i++) {
		int t = getCharacterClassType(c->cClass, i);
		if (t == -1)
			continue;
		printStatsString(_chargenClassStrings[t + 21], 1, 17 + i);
		printStatsString(Common::String::format("%2d", c->level[i]).c_str(), 14, 17 + i);
		printStatsString(Common::String::format("%6d", c->experience[i]).c_str(), 7, 17 + i);
	}

	r->fillRectWithTiles(0, 22, 0, 18, 21, 0, true, true, _statsPattern2);
	r->render(Screen_EoB::kSegaRenderPage);

	_screen->copyRegion(176, 40, 176, 40, 144, 128, Screen_EoB::kSegaRenderPage, 2, Screen::CR_NO_P_CHECK);
}

void EoBEngine::gui_displayMap() {

}

void EoBEngine::makeNameShapes() {
	if (_flags.platform != Common::kPlatformSegaCD)
		return;

	int cd = _txt->clearDim(4);
	int cp = _screen->setCurPage(2);
	_screen->sega_getRenderer()->fillRectWithTiles(1, 0, 0, 40, 28, 0x2000);
	_screen->sega_getRenderer()->fillRectWithTiles(0, 0, 0, 30, 28, 0x600A, true);
	_screen->sega_clearTextBuffer(0);

	for (int i = 0; i < 6; ++i) {
		if (!_characters[i].flags)
			continue;
		_txt->printShadowedText(_characters[i].name, 0, i << 4, 0xFF, 0xCC);
	}

	_screen->sega_getRenderer()->render(_screen->_curPage);

	for (int i = 0; i < 6; ++i) {
		if (!_characters[i].flags)
			continue;
		delete[] _characters[i].nameShape;
		_characters[i].nameShape = _screen->encodeShape(0, i << 4, (_screen->getTextWidth(_characters[i].name) + 8) >> 3, 13);
	}

	_screen->clearPage(2);
	_screen->setCurPage(cp);
	_screen->sega_clearTextBuffer(0);

	_txt->clearDim(4);
	_txt->clearDim(cd);
}

void EoBEngine::makeFaceShapes() {
	uint8 *in = _res->fileData("FACE", 0);
	for (int i = 0; i < 6; i++) {
		EoBCharacter *c = &_characters[i];
		if (!c->flags)
			continue;
		_screen->sega_encodeShapesFromSprites(&c->faceShape, &in[(c->portrait < 0 ? -c->portrait + 43 : c->portrait) << 9], 1, 32, 32, 3);
	}	
	delete[] in;
}

void EoBEngine::printStatsString(const char *str, int x, int y) {
	uint16 *dst = &_statsPattern2[y * 18 + x];
	for (const uint8 *pos = (const uint8*)str; *pos; ++pos)
		*dst++ = 0x6525 + _charTilesTable[*pos];
}

void EoBEngine::gui_updateAnimations() {
	if (_flags.platform != Common::kPlatformSegaCD)
		return;

	bool updScreen = false;
	bool redrawCompass = false;

	// Compass
	if (_compassDirection != _compassDirection2) {
		_compassAnimDest = _compassDirection << 2;
		int diff = _compassAnimDest - _compassAnimPhase;
		if (diff < 0)
			diff += 16;
		if (diff) {
			_compassAnimStep = (diff < 8) ? 1 : -1;
			_compassAnimDone = false;
		}
		_compassDirection2 = _compassDirection;
		redrawCompass = true;
	}
	if (_compassAnimDelayCounter) {
		--_compassAnimDelayCounter;
	} else if (!redrawCompass) {
		if (_compassAnimDest != _compassAnimPhase) {
			_compassAnimPhase = (_compassAnimPhase + _compassAnimStep) & 0x0F;
			_compassAnimDelayCounter = 6;
			redrawCompass = true;
		} else if (!_compassAnimDone) {
			if (_compassAnimSwitch) {
				_compassAnimPhase = (_compassAnimPhase + _compassAnimStep) & 0x0F;
				_compassAnimDelayCounter = 6;
				redrawCompass = true;
				_compassAnimStep = -_compassAnimStep;
				_compassAnimSwitch = false;
			} else {
				_compassAnimDone = _compassAnimSwitch = true;
			}
		}
	}
	if (redrawCompass) {
		_screen->sega_getRenderer()->loadToVRAM(_compassData + (_compassAnimPhase & 0x0F) * 0x500, 0x500, 0xEE00);
		_screen->sega_getRenderer()->render(Screen_EoB::kSegaRenderPage);
		_screen->copyRegion(88, 120, 88, 120, 80, 48, Screen_EoB::kSegaRenderPage, 0, Screen::CR_NO_P_CHECK);
		updScreen = true;
	}

	// Red grid effect
	for (int i = 0; i < 6; ++i) {
		if (!_characters[i].gfxUpdateCountdown)
			continue;
		_characters[i].gfxUpdateCountdown--;
		int cp = _screen->setCurPage(0);

		if (!_currentControlMode && (_characters[i].gfxUpdateCountdown & 1))
			_screen->drawShape(0, _redGrid, 176 + guiSettings()->charBoxCoords.facePosX_1[i & 1], guiSettings()->charBoxCoords.facePosY_1[i >> 1], 0);
		else if (_currentControlMode && _updateCharNum == i && (_characters[i].gfxUpdateCountdown & 1))
			_screen->drawShape(0, _redGrid, guiSettings()->charBoxCoords.facePosX_2[0], guiSettings()->charBoxCoords.facePosY_2[0], 0);
		else
			gui_drawFaceShape(i);

		_screen->setCurPage(cp);
		updScreen = true;
	}

	// Scene shake
	if (_sceneShakeCountdown) {
		--_sceneShakeCountdown;
		_sceneShakeOffsetX = _sceneShakeOffsets[_sceneShakeCountdown << 1];
		_sceneShakeOffsetY = _sceneShakeOffsets[(_sceneShakeCountdown << 1) + 1];
		_screen->fillRect(0, 0, 2, 119, 0, _sceneDrawPage1);
		_screen->fillRect(0, 0, 175, 2, 0, _sceneDrawPage1);
		_screen->copyBlockToPage(_sceneDrawPage1, 173, 0, 6, 120, _shakeBackBuffer1);
		_screen->copyBlockToPage(_sceneDrawPage1, 0, 117, 179, 6, _shakeBackBuffer2);
		_screen->copyBlockToPage(_sceneDrawPage1, _sceneXoffset + _sceneShakeOffsetX, _sceneShakeOffsetY, 176, 120, _sceneWindowBuffer);

		// For whatever reason the original shakes all types of shapes (decorations, doors, etc.) except the monsters and
		// the items lying on the floor. So we do the same. I've added drawing flags to drawSceneShapes() which allow
		// separate drawing passes for the different shape types.
		_shapeShakeOffsetX = _sceneShakeOffsetX;
		_shapeShakeOffsetY = _sceneShakeOffsetY;
		// All shapes except monsters and items
		drawSceneShapes(0, 0xFF & ~0x2A);
		_shapeShakeOffsetX = _shapeShakeOffsetY = 0;
		// Monsters and items
		drawSceneShapes(0, 0x2A);

		_screen->copyRegion(0, 0, 0, 0, 179, 123, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		updScreen = true;
	}

	if (updScreen)
		_screen->updateScreen();
}

} // End of namespace Kyra

#endif // ENABLE_EOB
