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
#include "kyra/gui/gui_eob_segacd.h"
#include "kyra/resource/resource.h"
#include "kyra/resource/resource_segacd.h"

#include "common/system.h"

namespace Kyra {

int EoBEngine::clickedCamp(Button *button) {
	uint32 startTime = _system->getMillis();
	gui_resetAnimations();

	if (_flags.platform == Common::kPlatformSegaCD)
		snd_playSong(11);

	EoBCoreEngine::clickedCamp(button);

	if (_flags.platform != Common::kPlatformSegaCD)
		return button->arg;

	gui_resetAnimations();
	_totalPlaySecs += ((_system->getMillis() - startTime) / 1000);

	return button->arg;
}

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

	static const uint16 ps[6] = { 0xCE, 0xE0, 0x2FE, 0x310, 0x52E, 0x540 };

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
	str = _sres->resStreamEndian(10);
	r->loadStreamToVRAM(str, 0x7920, false);
	delete str;

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

void EoBEngine::gui_setupPlayFieldHelperPages(bool keepText) {
	if (_flags.platform != Common::kPlatformSegaCD) {
		EoBCoreEngine::gui_setupPlayFieldHelperPages();
		return;
	}

	if (!keepText)
		_txt->clearDim(0);

	SegaRenderer *r = _screen->sega_getRenderer();
	r->loadToVRAM(_scrYellow, 4992, 0x3CE0);
	r->fillRectWithTiles(0, 0, 0, 22, 21, 0);
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
	_screen->drawShape(_screen->_curPage, _invSmallDigits[(val < 10) ? 22 + val : (val >= 100 ? 1 : 2 + val / 10)], x, y, 0);
	_screen->drawShape(_screen->_curPage, (val >= 10 && val < 100) ? _invSmallDigits[12 + (val % 10)] : 0, x, y, 0);
}

void EoBEngine::gui_drawCharacterStatsPage() {
	if (_flags.platform != Common::kPlatformSegaCD) {
		EoBCoreEngine::gui_drawCharacterStatsPage();
		return;
	}

	SegaRenderer *r = _screen->sega_getRenderer();
	EoBCharacter *c = &_characters[_updateCharNum];

	memset(_tempPattern, 0, 792);
	for (int i = 0; i < 11; ++i) {
		_tempPattern[5 * 18 + i + 1] = 0x6555 + i;
		_tempPattern[6 * 18 + i + 1] = 0x6565 + i;
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
	printStatsString(Common::String::format("%2d", c->armorClass).c_str(), _flags.lang == Common::JA_JPN ? 9 : 5, 14);

	for (int i = 0; i < 3; i++) {
		int t = getCharacterClassType(c->cClass, i);
		if (t == -1)
			continue;
		printStatsString(_chargenClassStrings[t + 21], 1, 17 + i);
		printStatsString(Common::String::format("%2d", c->level[i]).c_str(), 14, 17 + i);
		printStatsString(Common::String::format("%6d", c->experience[i]).c_str(), 7, 17 + i);
	}

	r->fillRectWithTiles(0, 22, 0, 18, 21, 0, true, true, _tempPattern);
	r->render(2, 22, 5, 18, 16);
}

void EoBEngine::gui_displayMap() {
	uint32 startTime = _system->getMillis();
	disableSysTimer(2);

	_screen->sega_fadeToBlack(2);
	Button b;
	clickedSpellbookAbort(&b);
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


	int cs = _screen->setFontStyles(_screen->_currentFont, _flags.lang == Common::JA_JPN ? Font::kStyleNone : Font::kStyleFullWidth);

	_screen->sega_clearTextBuffer(0);
	for (int i = 0; i < 3; ++i)
		drawMapButton(_mapStrings1[i], 0, i << 4);
	_screen->sega_loadTextBufferToVRAM(0, 0x7E20, 1536);
	r->fillRectWithTiles(0, 31, 19, 8, 6, 0x63F1, true);
	_screen->sega_clearTextBuffer(0);

	_screen->sega_clearTextBuffer(0);
	_txt->printShadedText(_mapStrings2[_currentLevel - 1], 0, 0, 0xFF, 0, 64, 16, 0, false);
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

void EoBEngine::gui_drawSpellbook() {
	if (_flags.platform != Common::kPlatformSegaCD) {
		EoBCoreEngine::gui_drawSpellbook();
		return;
	}

	SegaRenderer *r = _screen->sega_getRenderer();
	r->fillRectWithTiles(0, 10, 15, 12, 7, 0);
	r->fillRectWithTiles(1, 10, 15, 12, 7, 0x6429);
	memset(_tempPattern, 0, 168);
	uint16 *dst = _tempPattern;

	for (int i = 0; i < 6; ++i) {
		dst[0] = 0x642B + 2 * i + (i == _openBookSpellLevel ? 0 : 12);
		dst[1] = dst[0] + 1;
		dst += 2;
	}

	for (int i = 0; i < 6; ++i) {
		int d = _openBookAvailableSpells[_openBookSpellLevel * 10 + i];
		if (d < 0)
			continue;
		printSpellbookString(&_tempPattern[(i + 1) * 12], _openBookSpellList[d], (i == _openBookSpellSelectedItem) ? 0x6223 : 0x63C9);
	}

	r->fillRectWithTiles(0, 10, 15, 12, 6, 0, true, false, _tempPattern);
	r->render(Screen_EoB::kSegaRenderPage, 10, 15, 12, 7);

	// The original SegaCD version actually doesn't disable the spell book after use but closes it instead.
	if (!_closeSpellbookAfterUse) {
		if (_characters[_openBookChar].disabledSlots & 4) {
			static const uint8 xpos[] = { 0x44, 0x62, 0x80, 0x90 };
			static const uint8 ypos[] = { 0x80, 0x90, 0xA0 };
			for (int yc = 0; yc < 3; yc++) {
				for (int xc = 0; xc < 4; xc++)
					_screen->drawShape(Screen_EoB::kSegaRenderPage, _weaponSlotGrid, xpos[xc], ypos[yc], 0);
			}
		}
	}

	_screen->copyRegion(80, 120, 80, 120, 96, 56, Screen_EoB::kSegaRenderPage, 0, Screen::CR_NO_P_CHECK);
	if (!_loading)
		_screen->updateScreen();
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
	if (_updateFlags)
		_compassTilesRestore = true;
	else if (_compassTilesRestore) {
		_screen->sega_getRenderer()->fillRectWithTiles(0, 10, 15, 12, 7, 0);
		for (int i = 15; i < 22; ++i)
			_screen->sega_getRenderer()->fillRectWithTiles(1, 10, i, 12, 1, 0x2000, true, true, &_playFldPattern2[i * 40 + 10]);
		_compassTilesRestore = false;
	}
	if (redrawCompass) {
		_screen->sega_getRenderer()->loadToVRAM(_compassData + (_compassAnimPhase & 0x0F) * 0x500, 0x500, 0xEE00);
		_screen->sega_getRenderer()->render(0, 11, 15, 10, 6);
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
		if (_updateFlags)
			_screen->copyRegion(64, 120, 64, 120, 112, 3, Screen_EoB::kSegaRenderPage, _sceneDrawPage1);
		_screen->copyBlockToPage(_sceneDrawPage1, _sceneXoffset + _sceneShakeOffsetX, _sceneShakeOffsetY, 176, 120, _sceneWindowBuffer);

		// For whatever reason the original shakes all types of shapes (decorations, doors, etc.) except the monsters and
		// the items lying on the floor. So we do the same. I've added drawing flags to drawSceneShapes() which allow
		// separate drawing passes for the different shape types.
		for (int i = 0; i < 18; i++) {
			_shapeShakeOffsetX = _sceneShakeOffsetX;
			_shapeShakeOffsetY = _sceneShakeOffsetY;
			// All shapes except monsters and items
			drawSceneShapes(i, i + 1, 0xFF & ~0x2A);
			_shapeShakeOffsetX = _shapeShakeOffsetY = 0;
			// Monsters and items
			drawSceneShapes(i, i + 1, 0x2A);
		}
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
	int cs = _screen->setFontStyles(_screen->_currentFont, Font::kStyleForceOneByte);

	uint8 *in = _res->fileData("FACE", 0);
	for (int i = first; i <= last; ++i) {
		if (!_characters[i].flags)
			continue;
		if (_characters[i].portrait < 0) {
			_screen->sega_getRenderer()->loadToVRAM(in + 27424 - _characters[i].portrait * 224, 224, 0x3F00 + i * 0xE0);
			_screen->sega_getRenderer()->fillRectWithTiles(0, 0, i << 1, 7, 1, 0x61F8 + i * 7, true);
		} else {
			_txt->printShadedText(_characters[i].name, 0, i << 4, 0xFF, 0xCC);
		}
	}
	delete[] in;

	_screen->sega_getRenderer()->render(_screen->_curPage, 0, 0, 8, 12);
	for (int i = first; i <= last; ++i) {
		if (!_characters[i].flags)
			continue;
		delete[] _characters[i].nameShape;
		_characters[i].nameShape = _screen->encodeShape(0, i << 4, 8, 13);
	}

	_screen->clearPage(2);
	_screen->setCurPage(cp);
	_screen->sega_clearTextBuffer(0);
	_screen->setFontStyles(_screen->_currentFont, cs);

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
	uint16 *dst = &_tempPattern[y * 18 + x];
	for (const uint8 *pos = (const uint8*)str; *pos; ++pos)
		*dst++ = 0x6525 + _charTilesTable[*pos];
}

void EoBEngine::printSpellbookString(uint16 *dst, const char *str, uint16 ntbl) {
	assert(str);
	const uint8 *in = (const uint8*)str;
	for (uint8 c = *in++; c; c = *in++) {
		if (_flags.lang == Common::JA_JPN) {
			if (c > 165 && c < 222)
				*dst = ntbl + c - 166;
			else if (c == 32)
				*dst = ntbl + 82;
			else if (c > 47 && c < 58)
				*dst = ntbl + c + 35;
			else if (c == 47)
				*dst = ntbl + 93;
			else if (c == 165)
				*dst = ntbl + 94;
			else if (c == 43)
				*dst = ntbl + 95;

			if (*in == 222) {
				if (c > 181 && c < 197) {
					*dst = ntbl + c - 121;
					++in;
				} else if (c > 201 && c < 207) {
					*dst = ntbl + c - 126;
					++in;
				} else if (c == 179) {
					*dst = ntbl + c + 81;
					++in;
				}
			} else if (*in == 223 && c > 201 && c < 207) {
				*dst = ntbl + c - 146;
				++in;
			}

		} else {
			if (c > 31 && c < 128)
				*dst = ntbl + c - 32;
		}
		dst++;
	}
}

void EoBEngine::drawMapButton(const char *str, int x, int y) {
	_screen->sega_drawClippedLine(8, 9, x, y, 64, 14, 0x99);
	_screen->sega_drawClippedLine(8, 9, x, y + 1, 63, 13, 0xBB);
	_screen->sega_drawClippedLine(8, 9, x + 1, y + 1, 62, 12, 0xAA);
	_txt->printShadedText(str, x + 14, y + 1, 0xFF, 0xCC, 64, 72, 0, false);
}

void EoBEngine::drawMapPage(int level) {
	_screen->sega_clearTextBuffer(0);
	int cs = _screen->setFontStyles(_screen->_currentFont, (_flags.lang == Common::JA_JPN ? Font::kStyleNone : Font::kStyleFullWidth) | Font::kStyleNarrow1);
	_txt->printShadedText(_mapStrings3[level - 1], 0, 0, 0xCC, 0, 48, 16, 0, false);
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
		int cs = _screen->setFontStyles(_screen->_currentFont, (_flags.lang == Common::JA_JPN ? Font::kStyleNone : Font::kStyleFullWidth) | Font::kStyleNarrow2);
		if (_screen->getTextWidth(_dialogueButtonString[i]) > 90)
			_screen->setFontStyles(_screen->_currentFont, (_flags.lang == Common::JA_JPN ? Font::kStyleNone : Font::kStyleFullWidth) | Font::kStyleNarrow1);
		_screen->sega_drawClippedLine(38, 6, _dialogueButtonPosX[i], _dialogueButtonPosY[i], 90, 14, 0x99);
		_screen->sega_drawClippedLine(38, 6, _dialogueButtonPosX[i], _dialogueButtonPosY[i] + 1, 89, 13, 0xBB);
		_screen->sega_drawClippedLine(38, 6, _dialogueButtonPosX[i] + 1, _dialogueButtonPosY[i] + 1, 88, 12, 0xAA);
		_txt->printShadedText(_dialogueButtonString[i], _dialogueButtonPosX[i] + (_dialogueButtonWidth >> 1) - MIN<int>(_dialogueButtonWidth, _screen->getTextWidth(_dialogueButtonString[i])) / 2,
			_dialogueButtonPosY[i] + 1, _dialogueHighlightedButton == i ? _dialogueButtonLabelColor1 : _dialogueButtonLabelColor2, 0xEE, 304, 48, 0, false);
		_screen->setFontStyles(_screen->_currentFont, cs);
	}

	_screen->sega_loadTextBufferToVRAM(0, 0xA380, 7296);
	_screen->sega_getRenderer()->render(0);
}

GUI_EoB_SegaCD::GUI_EoB_SegaCD(EoBEngine *vm) : GUI_EoB(vm), _vm(vm), _clickableCharactersNumPages(vm->_textInputCharacterLinesSize) {
	_vm->_sres->loadContainer("ITEM");
	uint8 *cm = _vm->_sres->resData(8, 0);
	uint8 *cmdec = new uint8[47925];
	uint16 decodeSize = READ_LE_UINT16(cm + 2);
	_screen->decodeBIN(cm + 4, cmdec, decodeSize);
	_campMenu = cmdec;
	delete[] cm;

	const EoBMenuButtonDef* df = &_vm->_menuButtonDefs[6];
	_saveLoadCancelButton = new Button();
	_saveLoadCancelButton->index = 7;
	_saveLoadCancelButton->width = df->width;
	_saveLoadCancelButton->height = df->height;
	_saveLoadCancelButton->flags = df->flags;
	_saveLoadCancelButton->extButtonDef = df;
}

GUI_EoB_SegaCD::~GUI_EoB_SegaCD() {
	delete[] _campMenu;
	delete _saveLoadCancelButton;
}

void GUI_EoB_SegaCD::drawCampMenu() {
	_screen->sega_getRenderer()->loadToVRAM(_campMenu, 14784, 0x20);
	_screen->sega_getRenderer()->fillRectWithTiles(0, 0, 0, 22, 21, 0);
	_screen->sega_getRenderer()->fillRectWithTiles(1, 0, 0, 22, 21, 0x4001, true);
	_screen->sega_selectPalette(40, 2, true);
}

void GUI_EoB_SegaCD::initMemorizePrayMenu(int spellType) {
	_screen->sega_getRenderer()->fillRectWithTiles(0, 0, 0, 22, 21, 0);
	_screen->sega_getRenderer()->fillRectWithTiles(0, 1, 8, 20, 2, 0x62AB, true);
	_screen->sega_getRenderer()->fillRectWithTiles(0, 1, 4, 20, 4, 0x6283, true);
	_screen->sega_getRenderer()->memsetVRAM(0x5060, 0, 2560);
	_screen->sega_getRenderer()->memsetVRAM(0x5560, 0, 1280);
	_screen->sega_getRenderer()->loadToVRAM(&_campMenu[0x87C0], 4992, 0x3CE0);
	_screen->sega_clearTextBuffer(0);
	_vm->_txt->printShadedText(_vm->_menuStringsSpells[spellType ? 17 : 14], 0, 2, 0xFF, 0xCC, 160, 16, 0, false);
	_screen->sega_loadTextBufferToVRAM(0, 0x5060, 2560);
	_screen->sega_getRenderer()->render(0, 1, 4, 20, 2);
}

void GUI_EoB_SegaCD::initScribeScrollMenu() {
	_screen->sega_getRenderer()->fillRectWithTiles(0, 0, 0, 22, 21, 0);
	_screen->sega_getRenderer()->fillRectWithTiles(0, 1, 4, 20, 4, 0x6283, true);
	_screen->sega_getRenderer()->loadToVRAM(&_campMenu[0x87C0], 4992, 0x3CE0);
	_screen->sega_clearTextBuffer(0);
	_vm->_txt->printShadedText(getMenuString(48), 0, 3, 0xFF, 0xCC, 160, 16, 0, false);
	_screen->sega_loadTextBufferToVRAM(0, 0x5060, 2560);
	_screen->sega_getRenderer()->render(0, 1, 4, 20, 2);
}

void GUI_EoB_SegaCD::printScribeScrollSpellString(const int16 *menuItems, int id, bool highlight) {
	assert(menuItems);
	uint16 buf[22];
	memset(buf, 0, sizeof(buf));
	_vm->printSpellbookString(&buf[1], _vm->_mageSpellList[menuItems[id]], highlight ? 0x6223 : 0x63C9);
	_screen->sega_getRenderer()->fillRectWithTiles(0, 1, 10 + id, 20, 1, 0,  true, false, buf);
	_screen->sega_getRenderer()->render(0, 1, 10 + id, 20, 1);
}

void GUI_EoB_SegaCD::drawSaveSlotDialog(int x, int y, int id) {
	_screen->sega_getRenderer()->fillRectWithTiles(0, 0, 0, 22, 21, 0);
	_screen->sega_getRenderer()->fillRectWithTiles(0, (x >> 3) + 1, (y >> 3) + (y ? 3 : 4), 20, 2, 0x6283, true);
	_screen->sega_getRenderer()->fillRectWithTiles(0, (x >> 3) + (x ? 5 : 6), (y >> 3) + (y ? 6 : 7), 15, 10, 0x62AB, true);
	_screen->sega_getRenderer()->fillRectWithTiles(0, (x >> 3) + 1, (y >> 3) + 19, 7, 1, 0x6002, true);
	_screen->sega_getRenderer()->loadToVRAM(&_campMenu[0x87C0], 4992, 0x3CE0);
	_screen->sega_getRenderer()->memsetVRAM(0x5560, 0, 4480);
	_screen->sega_clearTextBuffer(0);
	_saveLoadCancelButton->x = ((const EoBMenuButtonDef*)_saveLoadCancelButton->extButtonDef)->x + x - (x ? 8 : 0);
	_saveLoadCancelButton->y = ((const EoBMenuButtonDef*)_saveLoadCancelButton->extButtonDef)->y + y;
	int cs = _screen->setFontStyles(_screen->_currentFont, _vm->gameFlags().lang == Common::JA_JPN ? Font::kStyleNone : Font::kStyleFullWidth);
	_vm->_txt->printShadedText(_vm->_saveLoadStrings[2 + id], 0, 3, 0xFF, 0xCC, 160, 16, 0, false);
	_screen->setFontStyles(_screen->_currentFont, cs);
	_screen->sega_loadTextBufferToVRAM(0, 0x5060, 1280);
	_screen->sega_getRenderer()->render(0, x >> 3, (y >> 3) + 1, 22, 21);
}

bool GUI_EoB_SegaCD::confirmDialogue(int id) {
	_screen->sega_clearTextBuffer(0);

	int cs = _vm->gameFlags().lang == Common::JA_JPN ? Font::kStyleNone : Font::kStyleFullWidth;
	if (id == 47) {
		cs |= Font::kStyleNarrow2;
		_screen->_charSpacing = 1;
	}
	cs = _screen->setFontStyles(_screen->_currentFont, cs);
	_vm->_txt->printShadedText(getMenuString(id), 0, 3, 0xFF, 0xCC, 160, 40, 0, false);
	_screen->_charSpacing = 0;
	_screen->setFontStyles(_screen->_currentFont, cs);

	_screen->sega_loadTextBufferToVRAM(0, 0x5060, 10240);
	_screen->sega_getRenderer()->fillRectWithTiles(0, 1, 0, 20, 20, 0);
	_screen->sega_getRenderer()->fillRectWithTiles(0, 1, 5, 20, 8, 0x6283, true);
	_screen->sega_getRenderer()->render(0, 1, 0, 22, 20);
	_screen->updateScreen();

	Button *buttonList = initMenu(5);

	int newHighlight = 0;
	int lastHighlight = -1;
	bool result = false;

	for (bool runLoop = true; runLoop && !_vm->shouldQuit();) {
		if (newHighlight != lastHighlight) {
			if (lastHighlight != -1)
				drawMenuButton(_vm->gui_getButton(buttonList, lastHighlight + 33), false, false, true);
			drawMenuButton(_vm->gui_getButton(buttonList, newHighlight + 33), false, true, true);
			_screen->updateScreen();
			lastHighlight = newHighlight;
		}

		int inputFlag = _vm->checkInput(buttonList, false, 0) & 0x80FF;
		_vm->removeInputTop();

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP5] || inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN]) {
			result = lastHighlight == 0;
			inputFlag = 0x8012 + lastHighlight;
			runLoop = false;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP4] || inputFlag == _vm->_keyMap[Common::KEYCODE_LEFT] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP6] || inputFlag == _vm->_keyMap[Common::KEYCODE_RIGHT]) {
			newHighlight ^= 1;
		} else if (inputFlag == 0x8012) {
			result = true;
			runLoop = false;
		} else if (inputFlag == 0x8013) {
			result = false;
			runLoop = false;
		} else {
			Common::Point p = _vm->getMousePos();
			for (Button *b = buttonList; b; b = b->nextButton) {
				if ((b->arg & 2) && _vm->posWithinRect(p.x, p.y, b->x, b->y, b->x + b->width, b->y + b->height))
					newHighlight = b->index - 18;
			}
		}

		if (!runLoop) {
			Button *b = _vm->gui_getButton(buttonList, lastHighlight + 18);
			drawMenuButton(b, true, true, true);
			_screen->updateScreen();
			_vm->_system->delayMillis(80);
			drawMenuButton(b, false, true, true);
			_screen->updateScreen();
		}
	}

	releaseButtons(buttonList);

	return result;
}

void GUI_EoB_SegaCD::displayTextBox(int id, int textColor, bool wait) {
	_screen->sega_getRenderer()->fillRectWithTiles(0, 0, 0, 22, 20, 0);
	_screen->sega_clearTextBuffer(0);
	int cs = _vm->gameFlags().lang == Common::JA_JPN ? Font::kStyleNone : Font::kStyleFullWidth;
	if (id == 23 || id == 26 || id == 49)
		cs |= Font::kStyleNarrow2;
	cs = _screen->setFontStyles(_screen->_currentFont, cs);
	_vm->_txt->printShadedText(getMenuString(id), 0, 0, textColor, 0xCC, 160, 40, 0, false);
	_screen->sega_loadTextBufferToVRAM(0, 0x5060, 3200);
	_screen->setFontStyles(_screen->_currentFont, cs);
	_screen->sega_getRenderer()->fillRectWithTiles(0, 1, 6, 20, 5, 0x6283, true);
	_screen->sega_getRenderer()->render(0, 0, 1, 22, 19);
	_screen->updateScreen();
	if (!wait)
		return;

	_vm->resetSkipFlag();
	while (!(_vm->shouldQuit() || _vm->skipFlag()))
		_vm->delay(20);
	_vm->resetSkipFlag();
}

void GUI_EoB_SegaCD::drawMenuButton(Button *b, bool clicked, bool highlight, bool noFill) {
	if (!b)
		return;

	const MenuButtonTiles &t = _menuButtonTiles[b->index - 1];
	if (!t.nameTbl)
		return;

	_screen->sega_getRenderer()->loadToVRAM(&_campMenu[(0x1CE + t.srcOffs + (clicked ? 1 : 0) * ((b->width * b->height) >> 6)) << 5], (b->width * b->height) >> 1, t.nameTbl << 5);
	_screen->sega_getRenderer()->fillRectWithTiles(0, b->x >> 3, b->y >> 3, b->width >> 3, b->height >> 3, 0x4000 + t.nameTbl, true);
	_screen->sega_getRenderer()->render(0, b->x >> 3, b->y >> 3, b->width >> 3, b->height >> 3);
}

void GUI_EoB_SegaCD::drawSaveSlotButton(int slot, int redrawBox, bool highlight) {
	if (slot < 0)
		return;

	if (slot == 5) {
		drawMenuButton(_saveLoadCancelButton, redrawBox == 2, false, false);
		return;
	}

	_screen->sega_getRenderer()->fillRectWithTiles(0, (_saveSlotX >> 3) + (_saveSlotX ? 1 : 2), (_saveSlotY >> 3) + (_saveSlotY ? 6 : 7) + (slot << 1), 3, 2, 0x41E7 + slot * 12 + (redrawBox == 2 ? 6 : 0), true);
	_screen->sega_clearTextBuffer(0);
	_vm->_txt->printShadedText(slot < 5 ? _saveSlotStringsTemp[slot] : _vm->_saveLoadStrings[0], 0, (slot << 4) + (slot < 5 ? 0 : 2), highlight ? 0x55 : 0xFF, 0xCC, 121, 80, 0, false);
	_screen->sega_loadTextBufferToVRAM(0, 0x5560, 4800);
	_screen->sega_getRenderer()->render(0, (_saveSlotX >> 3) + (_saveSlotX ? 1 : 2), (_saveSlotY >> 3) + (_saveSlotY ? 6 : 7) + (slot << 1), 21, 2);
}

int GUI_EoB_SegaCD::getHighlightSlot() {
	int res = -1;
	Common::Point p = _vm->getMousePos();

	for (int i = 0; i < 5; i++) {
		int y = _saveSlotY + i * 16 + (_saveSlotY ? 48 : 56);
		if (_vm->posWithinRect(p.x, p.y, _saveSlotX + (_saveSlotX ? 8 : 16), y, _saveSlotX + 167, y + 15)) {
			res = i;
			break;
		}
	}

	if (_vm->posWithinRect(p.x, p.y, _saveLoadCancelButton->x, _saveLoadCancelButton->y, _saveLoadCancelButton->x + _saveLoadCancelButton->width - 1, _saveLoadCancelButton->y + _saveLoadCancelButton->height - 1))
		res = 5;

	return res;
}

void GUI_EoB_SegaCD::memorizePrayMenuPrintString(int spellId, int bookPageIndex, int spellType, bool noFill, bool highLight) {
	if (bookPageIndex < 0)
		return;

	if (spellId) {
		memset(_vm->_tempPattern, 0, 924);
		Common::String s = Common::String::format(_vm->_menuStringsMgc[0], spellType ? _vm->_clericSpellList[spellId] : _vm->_mageSpellList[spellId], _numAssignedSpellsOfType[spellId * 2 - 2]);
		if (_vm->gameFlags().lang == Common::JA_JPN) {
			for (int i = 0; i < 19; ++i) {
				if (s[i] == -34 || s[i] == -33)
					s.insertChar(' ', 18);
			}
		}
		_vm->printSpellbookString(_vm->_tempPattern, s.c_str(), highLight ? 0x6223 : 0x63C9);
		_screen->sega_getRenderer()->fillRectWithTiles(0, 1, 10 + bookPageIndex, 20, 1, 0, true, true, _vm->_tempPattern);
	} else {
		_screen->sega_getRenderer()->fillRectWithTiles(0, 1, 10 + bookPageIndex, 20, 1, 0);
	}
	_screen->sega_getRenderer()->render(0, 1, 10 + bookPageIndex, 20, 1);
}

void GUI_EoB_SegaCD::updateOptionsStrings() {
	uint16 ntblInputMode[3] = { 0x34C, 0x360, 0x30C };
	int speed = _vm->_configMouse ? _vm->_mouseSpeed : _vm->_padSpeed;

	SegaRenderer *r = _screen->sega_getRenderer();
	r->loadToVRAM(&_campMenu[(0x1CE + (_vm->_configMouse ? 0x240 : 0x24C)) << 5], 0x180, 0x42E0);
	r->loadToVRAM(&_campMenu[(0x1CE + (_vm->_configMusic ? 0x258 : 0x264)) << 5], 0x180, 0x4460);
	r->loadToVRAM(&_campMenu[(0x1CE + (_vm->_configSounds ? 0x258 : 0x264)) << 5], 0x180, 0x45E0);
	r->loadToVRAM(&_campMenu[(0x1CE + ntblInputMode[_vm->_inputMode]) << 5], 0x280, 0x49A0);
	r->loadToVRAM(&_campMenu[(0x444 + speed * 12) << 5], 0xC0, 0x48E0);

	r->fillRectWithTiles(0, 15, 5, 3, 2, 0x4247, true);
	r->fillRectWithTiles(0, 8, 5, 6, 2, 0x4217, true);
	r->fillRectWithTiles(0, 8, 8, 6, 2, 0x4223, true);
	r->fillRectWithTiles(0, 8, 11, 6, 2, 0x422F, true);
	r->fillRectWithTiles(0, 8, 14, 10, 2, 0x424D, true);
}

void GUI_EoB_SegaCD::restParty_updateRestTime(int hours, bool init) {
	SegaRenderer *r = _screen->sega_getRenderer();
	if (init)
		r->fillRectWithTiles(0, 1, 4, 20, 17, 0);
	_screen->sega_clearTextBuffer(0);

	int cs = _screen->setFontStyles(_screen->_currentFont, Font::kStyleFullWidth);
	_vm->_txt->printShadedText(getMenuString(42), 0, 0, 0xFF, 0xCC, 160, 48, 0, false);
	_vm->_txt->printShadedText(_vm->_menuStringsRest2[3], 0, 16, 0xFF, 0xCC, 160, 48, 0, false);
	_vm->_txt->printShadedText(Common::String::format("%3d", hours).c_str(), _vm->gameFlags().lang == Common::JA_JPN ? 60 : 117, 16, 0xFF, 0xCC, 160, 48, 0, false);
	_screen->setFontStyles(_screen->_currentFont, cs);

	_screen->sega_loadTextBufferToVRAM(0, 0x5060, 5120);
	r->fillRectWithTiles(0, 1, 4, 20, 2, 0x6000);
	r->fillRectWithTiles(0, 1, 6, 20, 6, 0x6283, true);
	r->render(0, 0, 0, 22, 16);
	_screen->updateScreen();
	_vm->delay(160);
}

uint16 GUI_EoB_SegaCD::checkClickableCharactersSelection() {
	Common::Point mousePos = _vm->getMousePos();
	int highlight = -1;

	for (int i = 0; i < 60; ++i) {
		int x = (i % 12) * 12 + 152;
		int y = (i / 12) * 12 + 96;
		if (!_vm->posWithinRect(mousePos.x, mousePos.y, x, y, x + 11, y + 7))
			continue;
		highlight = i;
		break;
	}

	if (highlight == -1) {
		for (int i = 0; i < 3; ++i) {
			int x = 200 + i * 36;
			if (!_vm->posWithinRect(mousePos.x, mousePos.y, x, 164, x + _screen->getTextWidth(_vm->_textInputSelectStrings[i ? i + 2 : _clickableCharactersPage]) - 1, 171))
				continue;
			highlight = 200 + i;
			break;
		}
	}

	if (highlight != _menuCur) {
		printClickableCharacters(_clickableCharactersPage);
		if (highlight != -1)
			printClickableCharacter(highlight, 0x55);
		_screen->sega_getRenderer()->render(0, 18, 10, 20, 14);
		_menuCur = highlight;
	}

	_csjis[0] = _csjis[1] = _csjis[2] = 0;
	int in = 0;
	for (Common::List<KyraEngine_v1::Event>::const_iterator evt = _vm->_eventList.begin(); evt != _vm->_eventList.end(); ++evt) {
		if (evt->event.type == Common::EVENT_LBUTTONDOWN)
			in = 1;
	}

	if (in && highlight != -1) {
		_menuCur = -1;
		switch (highlight) {
		case 200:
			printClickableCharacters((_clickableCharactersPage + 1) % _clickableCharactersNumPages);
			break;
		case 201:
			_keyPressed.keycode = Common::KEYCODE_BACKSPACE;
			break;
		case 202:
			_keyPressed.keycode = Common::KEYCODE_RETURN;
			break;
		default:
			_csjis[0] = fetchClickableCharacter(highlight);
			_csjis[1] = '\x1';
			return 0x89;
		}
	}

	return in;
}

void GUI_EoB_SegaCD::printClickableCharacters(int page) {
	if (_clickableCharactersPage != page) {
		_clickableCharactersPage = page;
		assert(_vm->_wndBackgrnd);
		_screen->sega_loadTextBackground(_vm->_wndBackgrnd, 10240);
	}
	for (int i = 0; i < 60; ++i)
		printClickableCharacter(i, 0xFF);
	for (int i = 200; i < 203; ++i)
		printClickableCharacter(i, 0xFF);
	_screen->sega_getRenderer()->render(0, 18, 10, 20, 14);
}

void GUI_EoB_SegaCD::printClickableCharacter(int id, int col) {
	char ch[3] = "\0\0";

	if (id < 60) {
		ch[0] = fetchClickableCharacter(id);
		_vm->_txt->printShadedText(ch, (id % 12) * 12 + 12, (id / 12) * 12 + 32, col);
	} else if (id >= 200) {
		id -= 200;
		_vm->_txt->printShadedText(_vm->_textInputSelectStrings[id ? id + 2 : _clickableCharactersPage], 60 + id * 36, 100, col);
	}
}

char GUI_EoB_SegaCD::fetchClickableCharacter(int id) const {
	if (id >= 200)
		return (char)id;
	if (id >= 60)
		return 0;

	uint8 c = (uint8)_vm->_textInputCharacterLines[_clickableCharactersPage][id];
	if (_clickableCharactersPage == 1) {
		if (c > 159 && c < 192)
			c -= 32;
		else if (c > 191 && c < 224)
			c += 32;
	}
	return (char)c;
}

const GUI_EoB_SegaCD::MenuButtonTiles GUI_EoB_SegaCD::_menuButtonTiles[40] = {
	{ 0x01e7, 0x0000 },	{ 0x01fb, 0x0028 },	{ 0x020f, 0x0050 }, { 0x0223, 0x0078 },
	{ 0x0237, 0x00a0 }, { 0x0000, 0x0000 },	{ 0x01cf, 0x01c8 },	{ 0x025f, 0x0118 },
	{ 0x024b, 0x00c8 }, { 0x0273, 0x0140 },	{ 0x020b, 0x0198 },	{ 0x01cf, 0x01c8 },
	{ 0x01f3, 0x01e0 }, { 0x01ff, 0x01f8 },	{ 0x0000, 0x0000 },	{ 0x01e7, 0x0210 },
	{ 0x0000, 0x0000 },	{ 0x01CF, 0x0168 }, { 0x01db, 0x0180 }, { 0x01cf, 0x01c8 },
	{ 0x0000, 0x0000 }, { 0x0000, 0x0000 }, { 0x0000, 0x0000 }, { 0x0000, 0x0000 },
	{ 0x0000, 0x0000 },	{ 0x0000, 0x0000 },	{ 0x01db, 0x01b0 }, { 0x01cf, 0x01c8 },
	{ 0x0000, 0x0000 }, { 0x01e7, 0x0270 },	{ 0x01f3, 0x027C }, { 0x01ff, 0x0288 },
	{ 0x020b, 0x0294 },	{ 0x0217, 0x02A0 },	{ 0x024d, 0x030c },	{ 0x0000, 0x0000 },
	{ 0x0000, 0x0000 }, { 0x0000, 0x0000 }, { 0x01CF, 0x01C8 }, { 0x0000, 0x0000 }
};

} // End of namespace Kyra

#endif // ENABLE_EOB
