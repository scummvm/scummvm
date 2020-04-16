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

#include "common/system.h"

namespace Kyra {

void EoBEngine::gui_drawPlayField(bool refresh) {
	if (_flags.platform != Common::kPlatformSegaCD) {
		EoBCoreEngine::gui_drawPlayField(refresh);
		return;
	}

	_screen->sega_fadeToBlack(_loading ? 0 : 1);
	_screen->sega_selectPalette(6, 1);
	_screen->sega_selectPalette(7, 3);

	// transposeScreenOutputY(8);
	_txt->clearDim(0);
	_screen->sega_getAnimator()->clearSprites();
	_screen->sega_getAnimator()->update();
	SegaRenderer *r = _screen->sega_getRenderer();
	r->fillRectWithTiles(0, 0, 0, 40, 28, 0);
	r->fillRectWithTiles(1, 0, 0, 40, 28, 0);

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

	_sres->loadContainer("ITEM");
	Common::SeekableReadStreamEndian *str = _sres->resStreamEndian(7);
	r->loadStreamToVRAM(str, 0x8880, true);
	delete str;
	str = _sres->resStreamEndian(9);
	r->loadStreamToVRAM(str, 0xA4A0, false);
	delete str;
	/*
// CAMP MENU
str = _sres->resStreamEndian(8);
_screen->sega_getRenderer()->loadStreamToVRAM(str, 0x20, true);
delete str;
_screen->sega_getRenderer()->fillRectWithTiles(0, 0, 0, 22, 15, 0);
_screen->sega_getRenderer()->fillRectWithTiles(1, 0, 0, 22, 21, 0x4001, true);
_screen->sega_getRenderer()->render(0);
_screen->sega_selectPalette(40, 2, true);
*/

	gui_setupPlayFieldHelperPages();

	if (refresh && !_sceneDrawPage2)
		drawScene(1);

	_screen->copyRegionToBuffer(0, 173, 0, 6, 120, _shakeBackBuffer1);
	_screen->copyRegionToBuffer(0, 0, 117, 179, 6, _shakeBackBuffer2);

	// Since we're not going to draw the character portrait boxes with the SegaRenderer but rather with our "normal" code, we have to backup
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
	gui_drawCompass(false);

	_screen->sega_fadeToNeutral(1);
}

void EoBEngine::gui_setupPlayFieldHelperPages() {
	if (_flags.platform != Common::kPlatformSegaCD) {
		EoBCoreEngine::gui_setupPlayFieldHelperPages();
		return;
	}

	_txt->clearDim(0);
	SegaRenderer *r = _screen->sega_getRenderer();
	r->fillRectWithTiles(0, 22, 0, 18, 21, 0);
	r->fillRectWithTiles(1, 0, 0, 40, 26, 0x2000, true, false, _playFldPattern2);
	r->fillRectWithTiles(0, 0, 21, 40, 5, 0x2000, true, false, _textFieldPattern);
	// Nametables for scene window vcn block tiles. We don't need that. We draw the blocks with our "normal" graphics code.
	// r->fillRectWithTiles(1, 0, 0, 22, 15, 0xC14B, true, true);
	// Nametables for scene window shapes tiles. We don't need that, since we're not going to draw any scene shapes with the renderer.
	// r->fillRectWithTiles(0, 0, 1, 22, 14, 0xE295, true, true);
	// Text field tiles
	r->fillRectWithTiles(0, 1, 22, 35, 3, 0x2597, true);
	r->render(0);
	r->fillRectWithTiles(1, 22, 0, 18, 21, 0x6444, true, true, _invPattern);
	r->render(2);
	r->fillRectWithTiles(1, 22, 0, 18, 21, 0x6444, true, true, _statsPattern);
	r->render(Screen_EoB::kSegaRenderPage);
	_screen->copyRegion(184, 1, 176, 168, guiSettings()->charBoxCoords.boxWidth, 24, 0, 2, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(184, 25, 240, 168, guiSettings()->charBoxCoords.boxWidth, guiSettings()->charBoxCoords.boxHeight - 24, 0, 2, Screen::CR_NO_P_CHECK);
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
	uint32 startTime = _system->getMillis();
	disableSysTimer(2);

	_screen->sega_fadeToBlack(2);

	gui_resetAnimations();
	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 1))
			continue;
		_characters[i].damageTaken = 0;
		_characters[i].slotStatus[0] = _characters[i].slotStatus[1] = 0;
		gui_drawCharPortraitWithStats(i);
	}

	SegaRenderer *r = _screen->sega_getRenderer();
	r->fillRectWithTiles(0, 0, 0, 40, 28, 0);
	r->fillRectWithTiles(1, 0, 0, 40, 28, 0);
	_screen->sega_getAnimator()->clearSprites();
	_screen->sega_getAnimator()->update();
	_screen->sega_selectPalette(55, 1);
	_screen->sega_selectPalette(56, 2);

	snd_stopSound();
	_sres->loadContainer("MAP");
	Common::SeekableReadStreamEndian *in = _sres->resStreamEndian(0);
	r->loadStreamToVRAM(in, 0x20);
	delete in;
	in = _sres->resStreamEndian(1);
	r->loadStreamToVRAM(in, 0x80);
	delete in;

	int cs = _screen->setFontStyles(_screen->_currentFont, _flags.lang == Common::JA_JPN ? Font::kStyleFixedWidth : Font::kStyleForceTwoByte | Font::kStyleFat);

	_screen->sega_clearTextBuffer(0);
	for (int i = 0; i < 3; ++i)
		drawMapButton(_mapStrings1[i], 0, i << 4);
	_screen->sega_loadTextBufferToVRAM(0, 0x7E20, 1536);
	r->fillRectWithTiles(0, 31, 19, 8, 6, 0x63F1, true);
	_screen->sega_clearTextBuffer(0);

	_screen->sega_clearTextBuffer(0);
	_txt->printShadowedText(_mapStrings2[_currentLevel - 1], 0, 0, 0xFF, 0, 64, 16, 0, false);
	_screen->sega_loadTextBufferToVRAM(0, 0x7C20, 512);
	r->fillRectWithTiles(0, 31, 16, 8, 2, 0x63E1, true);

	drawMapPage(_currentLevel);
	r->render(0);
	_screen->sega_fadeToNeutral(3);

	gui_resetButtonList();
	for (int i = 96; i < 99; ++i)
		gui_initButton(i);

	int animState = 0;
	for (int level = _currentLevel; level && !shouldQuit(); ) {
		uint32 del = _system->getMillis() + 16;
		int inputFlag = checkInput(_activeButtons, false, 0);
		removeInputTop();

		drawMapSpots(level, animState < 20 ? 0 : 1);
		bool update = (animState == 0 || animState == 20);
		if (++animState == 40)
			animState = 0;

		int op = (inputFlag & 0x8000) ? (int16)gui_getButton(_activeButtons, inputFlag & 0xFF)->arg : 0;
		if (op) {
			snd_playSoundEffect(0x81);
			level = (op == 2) ? 0 : CLIP(level + op, 1, 12);
			if (level)
				drawMapPage(level);
		}

		if (update) {
			r->render(0);
			_screen->updateScreen();
		}
		delayUntil(del);
	}

	_screen->setFontStyles(_screen->_currentFont, cs);
	_screen->sega_fadeToBlack(3);

	setLevelPalettes(_currentLevel);
	gui_drawPlayField(true);
	gui_drawAllCharPortraitsWithStats();
	gui_setInventoryButtons();
	snd_playLevelScore();

	enableSysTimer(2);
	_totalPlaySecs += ((_system->getMillis() - startTime) / 1000);
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
				_compassAnimStep = -_compassAnimStep;
				_compassAnimSwitch = false;
			} else {
				_compassAnimDone = _compassAnimSwitch = true;
			}
			redrawCompass = true;
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

void EoBEngine::gui_resetAnimations() {
	if (_flags.platform != Common::kPlatformSegaCD)
		return;

	for (int i = 0; i < 6; ++i)
		_characters[i].gfxUpdateCountdown = 1;
	_sceneShakeCountdown = 1;
	_compassAnimDelayCounter = _compassAnimSwitch = 0;
	_compassAnimPhase = _compassAnimDest;
}

void EoBEngine::makeNameShapes(int charId) {
	if (_flags.platform != Common::kPlatformSegaCD)
		return;

	int first = 0;
	int last = 5;
	if (charId != -1)
		first = last = charId;

	int cd = _txt->clearDim(4);
	int cp = _screen->setCurPage(2);
	_screen->sega_getRenderer()->fillRectWithTiles(1, 0, 0, 40, 28, 0x2000);
	_screen->sega_getRenderer()->fillRectWithTiles(0, 0, 0, 30, 28, 0x600A, true);
	_screen->sega_clearTextBuffer(0);

	uint8 *in = _res->fileData("FACE", 0);
	for (int i = first; i <= last; ++i) {
		if (!_characters[i].flags)
			continue;
		if (_characters[i].portrait < 0) {
			_screen->sega_getRenderer()->loadToVRAM(in + 27648 + (-_characters[i].portrait - 1) * 224, 224, 0x3F00 + i * 0xE0);
			_screen->sega_getRenderer()->fillRectWithTiles(0, 0, i << 1, 7, 1, 0x61F8 + i * 7, true);
		} else {
			_txt->printShadowedText(_characters[i].name, 0, i << 4, 0xFF, 0xCC);
		}
	}
	delete[] in;

	_screen->sega_getRenderer()->render(_screen->_curPage);
	_screen->sega_getRenderer()->render(0);
	_screen->updateScreen();
	_screen->sega_fadeToNeutral(0);

	for (int i = first; i <= last; ++i) {
		if (!_characters[i].flags)
			continue;
		delete[] _characters[i].nameShape;
		_characters[i].nameShape = _screen->encodeShape(0, i << 4, 8, 13);
	}

	_screen->clearPage(2);
	_screen->setCurPage(cp);
	_screen->sega_clearTextBuffer(0);

	_txt->clearDim(4);
	_txt->clearDim(cd);
}

void EoBEngine::makeFaceShapes(int charId) {
	if (_flags.platform != Common::kPlatformSegaCD) {
		EoBCoreEngine::makeFaceShapes();
		return;
	}

	int first = 0;
	int last = 5;
	if (charId != -1)
		first = last = charId;

	uint8 *in = _res->fileData("FACE", 0);
	for (int i = first; i <= last; i++) {
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

void EoBEngine::drawMapButton(const char *str, int x, int y) {
	_screen->sega_drawClippedLine(8, 9, x, y, 64, 14, 0x99);
	_screen->sega_drawClippedLine(8, 9, x, y + 1, 63, 13, 0xBB);
	_screen->sega_drawClippedLine(8, 9, x + 1, y + 1, 62, 12, 0xAA);
	_txt->printShadowedText(str, x + 14, y + 1, 0xFF, 0xCC, 64, 72, 0, false);
}

void EoBEngine::drawMapPage(int level) {
	int temp = 0;
	_screen->sega_clearTextBuffer(0);
	int cs = _screen->setFontStyles(_screen->_currentFont, (_flags.lang == Common::JA_JPN ? Font::kStyleFixedWidth : Font::kStyleForceTwoByte | Font::kStyleFat) | Font::kStyleNarrow1);
	_txt->printShadowedText(_mapStrings3[level - 1], 0, 0, 0xCC, 0, 48, 16, 0, false);
	_screen->setFontStyles(_screen->_currentFont, cs);
	_screen->sega_loadTextBufferToVRAM(0, 0x7920, 384);
	SegaRenderer *r = _screen->sega_getRenderer();
	r->fillRectWithTiles(0, 23, 8, 6, 2, 0x63C9, true);

	Common::SeekableReadStreamEndian *in = _sres->resStreamEndian(hasLevelMap(level) ? 2 + level : 2);
	r->loadStreamToVRAM(in, 0x5500, true);
	delete in;
	r->fillRectWithTiles(1, 3, 0, 26, 26, 0x2004, true);
	r->fillRectWithTiles(0, 5, 6, 17, 17, 0x42A8, true);
}

void EoBEngine::drawMapSpots(int level, int animState) {
	SegaAnimator *a = _screen->sega_getAnimator();
	const EoBItem &m = _items[447 + level];
	int curX = _currentBlock & 0x1F;
	int curY = _currentBlock >> 5;
	int mX = m.block & 0x1F;
	int mY = m.block >> 5;

	if (hasLevelMap(level)) {
		if (!animState && level == _currentLevel)
			a->initSprite(0, (curX << 2) + 48, (curY << 2) + 56, 0x6001, 0);
		else
			a->initSprite(0, 0x4000, 0, 0, 0);
		a->initSprite(1, 0x4000, 0, 0, 0);
	} else {
		a->initSprite(0, 0x4000, 0, 0, 0);
		if (level == _currentLevel)
			a->initSprite(0, (curX << 2) + 48, (curY << 2) + 56, animState ? 0x2002 : 0x2001, 0);
		a->initSprite(1, (mX << 2) + 48, (mY << 2) + 56, animState ? 0x2002 : 0x2003, 0);
	}
	a->update();
}

void EoBEngine::drawDialogueButtons() {
	if (_flags.platform != Common::kPlatformSegaCD) {
		KyraRpgEngine::drawDialogueButtons();
		return;
	}

	_screen->sega_clearTextBuffer(0);

	for (int i = 0; i < _dialogueNumButtons; i++) {
		int cs = _screen->setFontStyles(_screen->_currentFont, (_flags.lang == Common::JA_JPN ? Font::kStyleFixedWidth : Font::kStyleForceTwoByte | Font::kStyleFat) | Font::kStyleNarrow2);
		if (_screen->getTextWidth(_dialogueButtonString[i]) > 90)
			_screen->setFontStyles(_screen->_currentFont, (_flags.lang == Common::JA_JPN ? Font::kStyleFixedWidth : Font::kStyleForceTwoByte | Font::kStyleFat) | Font::kStyleNarrow1);
		_screen->sega_drawClippedLine(38, 6, _dialogueButtonPosX[i], _dialogueButtonPosY[i], 90, 14, 0x99);
		_screen->sega_drawClippedLine(38, 6, _dialogueButtonPosX[i], _dialogueButtonPosY[i] + 1, 89, 13, 0xBB);
		_screen->sega_drawClippedLine(38, 6, _dialogueButtonPosX[i] + 1, _dialogueButtonPosY[i] + 1, 88, 12, 0xAA);
		_txt->printShadowedText(_dialogueButtonString[i], _dialogueButtonPosX[i] + (_dialogueButtonWidth >> 1) - MIN<int>(_dialogueButtonWidth, _screen->getTextWidth(_dialogueButtonString[i])) / 2,
			_dialogueButtonPosY[i] + 1, _dialogueHighlightedButton == i ? _dialogueButtonLabelColor1 : _dialogueButtonLabelColor2, 0xEE, 304, 48, 0, false);
		_screen->setFontStyles(_screen->_currentFont, cs);
	}

	_screen->sega_loadTextBufferToVRAM(0, 0xA380, 7296);
	_screen->sega_getRenderer()->render(0);
}

} // End of namespace Kyra

#endif // ENABLE_EOB
