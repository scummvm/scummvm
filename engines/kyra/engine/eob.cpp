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
#include "kyra/graphics/screen_eob_segacd.h"
#include "kyra/resource/resource.h"
#include "kyra/resource/resource_segacd.h"
#include "kyra/sequence/seqplayer_eob_segacd.h"
#include "kyra/sound/sound.h"
#include "kyra/text/text_eob_segacd.h"

#include "common/system.h"

namespace Kyra {

EoBEngine::EoBEngine(OSystem *system, const GameFlags &flags)
	: EoBCoreEngine(system, flags) {
	_numSpells = 53;
	_menuChoiceInit = 4;

	_turnUndeadString = 0;
	_itemNamesStatic = 0;
	_numItemNamesStatic = 0;
	_finBonusStrings = _npcStrings[1] = _npcStrings[2] = 0;
	_npcStrings[3] = _npcStrings[4] = _npcStrings[5] = _npcStrings[6] = 0;
	_npcStrings[7] = _npcStrings[8] = _npcStrings[9] = _npcStrings[10] = 0;
	_npcShpData = _npcSubShpIndex1 = _npcSubShpIndex2 = _npcSubShpY = 0;
	_dscDoorScaleMult4 = _dscDoorScaleMult5 = _dscDoorScaleMult6 = _dscDoorY3 = 0;
	_dscDoorY4 = _dscDoorY5 = _dscDoorY6 = _dscDoorY7 = _doorShapeEncodeDefs = 0;
	_doorSwitchShapeEncodeDefs = _doorSwitchCoords = 0;
	_doorShapesSrc = _doorSwitchShapesSrc = 0;
	_dscDoorCoordsExt = 0;
	_invSmallDigits = _weaponSlotShapes = 0;
	_useMainMenuGUISettings = false;
	_addrTbl1 = _textFieldPattern = 0;
	_playFldPattern1 = _invPattern = _statsPattern;
	_playFldPattern2 = _statsPattern2 = 0;
	_ttlCfg = 0;
	_xdth = false;

	memset(_strikeAnimShapes, 0, sizeof(_strikeAnimShapes));
	_sceneShakeOffsetX = _sceneShakeOffsetY = 0;
	_shakeBackBuffer1 = _shakeBackBuffer2 = 0;
	_redGrid = 0;
	_charTilesTable = 0;

	_seqPlayer = 0;
	_sres = 0;
	_levelCurTrack = 0;
	_dcrResCur = -1;
}

EoBEngine::~EoBEngine() {
	for (int i = 0; i < 7; ++i) {
		releaseShpArr(_strikeAnimShapes[i], 4);
	}

	releaseShpArr(_weaponSlotShapes, 6);
	releaseShpArr(_invSmallDigits, 32);
	delete[] _redGrid;

	delete[] _doorShapesSrc;
	delete[] _doorSwitchShapesSrc;
	delete[] _itemsOverlay;
	delete[] _playFldPattern2;
	delete[] _statsPattern2;
	delete[] _shakeBackBuffer1;
	delete[] _shakeBackBuffer2;

	delete _seqPlayer;
	delete _sres;
}

Common::Error EoBEngine::init() {
	Common::Error err = EoBCoreEngine::init();
	if (err.getCode() != Common::kNoError)
		return err;

	initStaticResource();

	for (int i = 0; i < ARRAYSIZE(_titleConfig); ++i) {
		if (_flags.platform == _titleConfig[i].platform && _flags.lang == _titleConfig[i].lang)
			_ttlCfg = &_titleConfig[i];
	}
	if (!_ttlCfg) {
		for (int i = 0; i < ARRAYSIZE(_titleConfig); ++i) {
			if (_flags.platform == _titleConfig[i].platform && _titleConfig[i].lang == Common::UNK_LANG)
				_ttlCfg = &_titleConfig[i];
		}
	}
	assert(_ttlCfg);

	if (_configRenderMode != Common::kRenderCGA)
		_itemsOverlay = _res->fileData((_configRenderMode == Common::kRenderEGA) ? "ITEMRMP.EGA" : "ITEMRMP.VGA", 0);

	_screen->modifyScreenDim(7, 0x01, 0xB3, 0x22, 0x12);
	_screen->modifyScreenDim(9, 0x01, 0x7D, 0x26, 0x3F);

	if (_flags.platform == Common::kPlatformPC98) {
		_screen->modifyScreenDim(28, 0x0A, 0xA4, 0x15, 0x18);
		_screen->modifyScreenDim(12, 0x01, 0x04, 0x14, 0x9A);
	} else if (_flags.platform == Common::kPlatformSegaCD) {
		_screen->modifyScreenDim(27, 0x00, 0x02, 0x11, 0x03);
		_screen->modifyScreenDim(28, 0x07, 0xA0, 0x17, 0x24);
	} else {
		_screen->modifyScreenDim(12, 0x01, 0x04, 0x14, 0xA0);
	}

	_scriptTimersCount = 1;

	if (_configRenderMode == Common::kRenderEGA) {
		Palette pal(16);
		_screen->loadPalette(_egaDefaultPalette, pal, 16);
		_screen->setScreenPalette(pal);
	} else {
		_screen->loadPalette("PALETTE.COL", _screen->getPalette(0));
	}

	if (_flags.platform == Common::kPlatformPC98) {
		_vcnFilePattern = "%s.ECB";
		_vmpFilePattern = "%s.EMP";
	} else if (_configRenderMode == Common::kRenderEGA || _configRenderMode == Common::kRenderCGA) {
		_vcnFilePattern = "%s.ECN";
		_vmpFilePattern = "%s.EMP";
	} else if (_flags.platform == Common::kPlatformSegaCD) {
		_sres = new SegaCDResource(_res);
		assert(_sres);
		_seqPlayer = new SegaSequencePlayer(this, _screen, _sres);
		assert(_seqPlayer);
		_txt = new TextDisplayer_SegaCD(this, _screen);
		assert(_txt);
		_playFldPattern2 = new uint16[1040];
		_statsPattern2 = new uint16[792];
		_shakeBackBuffer1 = new uint8[120 * 6];
		_shakeBackBuffer2 = new uint8[179 * 6];
	}

	return Common::kNoError;
}

#define loadSpritesAndEncodeToShapes(resID, resOffset, shapeBuffer, numShapes, width, height) \
	shapeBuffer = new const uint8 *[numShapes]; \
	memset(shapeBuffer, 0, numShapes * sizeof(uint8*)); \
	in = _sres->resData(resID); \
	_screen->sega_encodeShapesFromSprites(shapeBuffer, in + (resOffset), numShapes, width, height, 3); \
	delete[] in

#define loadSpritesAndMergeToSingleShape(resID, resOffset, singleShape, numSprites, spriteWidth, spriteHeight) \
	in = _sres->resData(resID); \
	{ \
	const uint8 **shapeBuffer = new const uint8 *[numSprites]; \
	_screen->sega_encodeShapesFromSprites(shapeBuffer, in + (resOffset), numSprites, spriteWidth, spriteHeight, 3, false); \
	releaseShpArr(shapeBuffer, numSprites); \
	_screen->sega_getRenderer()->render(Screen_EoB::kSegaInitShapesPage, true); \
	_screen->sega_getAnimator()->clearSprites(); \
	int cp = _screen->setCurPage(Screen_EoB::kSegaInitShapesPage); \
	singleShape = _screen->encodeShape(0, 0, numSprites  * (spriteWidth >> 3), spriteHeight); \
	_screen->setCurPage(cp); \
	_screen->clearPage(Screen_EoB::kSegaInitShapesPage); \
	} \
	delete[] in

#define loadAndConvertShapes(resID, resOffset, shapeBuffer, numShapes, width, height, size) \
	shapeBuffer = new const uint8 *[numShapes]; \
	memset(shapeBuffer, 0, numShapes * sizeof(uint8*)); \
	in = _sres->resData(resID); \
	for (int ii = 0; ii < numShapes; ++ii) \
		shapeBuffer[ii] = _screen->sega_convertShape(in + (resOffset) + ii * size, width, height, 3); \
	delete[] in

void EoBEngine::loadItemsAndDecorationsShapes() {
	if (_flags.platform != Common::kPlatformSegaCD) {
		EoBCoreEngine::loadItemsAndDecorationsShapes();
		return;
	}

	releaseItemsAndDecorationsShapes();
	_sres->loadContainer("ITEM");
	uint8 *in = 0;

	loadSpritesAndEncodeToShapes(0, 0, _itemIconShapes, _numItemIconShapes, 16, 16);
	loadSpritesAndEncodeToShapes(14, 0, _blueItemIconShapes, _numItemIconShapes, 16, 16);
	loadSpritesAndEncodeToShapes(13, 0, _xtraItemIconShapes, 3, 16, 16);

	for (int i = 0; i < 7; ++i) {
		loadAndConvertShapes(15, i << 11, _strikeAnimShapes[i], 4, 32, 32, 512);
	}

	loadAndConvertShapes(1, 0, _smallItemShapes, _numSmallItemShapes, 32, 24, 768);
	loadAndConvertShapes(2, 0, _largeItemShapes, _numLargeItemShapes, 64, 24, 1472);
	loadAndConvertShapes(11, 0, _thrownItemShapes, _numThrownItemShapes, 32, 24, 768);
	int offset1 = 0, offset2 = 0;
	for (int i = 0; i < 3; ++i) {
		offset1 += (0x180 / (i + 1));
		offset2 += (0x300 / (i + 1));
		loadAndConvertShapes(1, offset1, _smallItemShapesScl[i], _numSmallItemShapes, (3 - i) << 3, 16 - ((i >> 1) << 3), 768);
		loadAndConvertShapes(2, offset2, _largeItemShapesScl[i], _numLargeItemShapes, (6 - 2 * i) << 3, 16 - ((i >> 1) << 3), 1472);
		loadAndConvertShapes(11, offset1, _thrownItemShapesScl[i], _numThrownItemShapes, (3 - i) << 3, 16 - ((i >> 1) << 3), 768);
	}

	loadSpritesAndMergeToSingleShape(5, 0, _redSplatShape, 5, 8, 24);
	loadSpritesAndMergeToSingleShape(5, 2016, _swapShape, 7, 8, 8);
	loadSpritesAndMergeToSingleShape(12, 0, _deadCharShape, 1, 32, 32);
	loadSpritesAndEncodeToShapes(5, 480, _weaponSlotShapes, 6, 32, 16);
	loadSpritesAndEncodeToShapes(6, 0, _invSmallDigits, 32, 16, 8);

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

	int cp = _screen->setCurPage(Screen_EoB::kSegaInitShapesPage);
	for (int i = 0; i < 4; ++i)
		_screen->sega_getRenderer()->loadToVRAM(_redGridTile, 8, 0x52A0 + i * 8);
	_screen->sega_getRenderer()->fillRectWithTiles(1, 0, 4, 4, 4, 0x6295);
	_screen->sega_getRenderer()->render(Screen_EoB::kSegaInitShapesPage);
	_screen->drawShape(Screen_EoB::kSegaInitShapesPage, _weaponSlotShapes[1], 0, 0, 0);
	_screen->drawShape(Screen_EoB::kSegaInitShapesPage, _weaponSlotShapes[1], 0, 16, 0);
	_weaponSlotGrid = _screen->encodeShape(0, 0, 4, 16);
	_disabledCharGrid = _screen->encodeShape(0, 0, 4, 32);
	_blackBoxSmallGrid = _screen->encodeShape(0, 0, 2, 8);
	_blackBoxWideGrid = _screen->encodeShape(0, 0, 4, 8);
	_redGrid = _screen->encodeShape(0, 32, 4, 32);
	_screen->clearPage(Screen_EoB::kSegaInitShapesPage);
	_screen->setCurPage(cp);
}

#undef loadAndConvertShapes
#undef loadSpritesAndEncodeToShapes
#undef loadSpritesAndMergeToSingleShape

Common::SeekableReadStreamEndian *EoBEngine::getItemDefinitionFile(int index) {
	assert(index == 0 || index == 1);
	if (_flags.platform != Common::kPlatformSegaCD)
		return EoBCoreEngine::getItemDefinitionFile(index);

	_sres->loadContainer("ITEMDAT");
	return _sres->resStreamEndian(index);
}

void EoBEngine::startupNew() {
	_sound->selectAudioResourceSet(kMusicIngame);
	_sound->loadSoundFile(0);
	_screen->selectPC98Palette(0, _screen->getPalette(0));
	if (_flags.platform == Common::kPlatformSegaCD) {
		_screen->sega_selectPalette(4, 0);
		_screen->sega_selectPalette(6, 1);
		_screen->sega_selectPalette(8, 2);
		_screen->sega_selectPalette(7, 3);
		makeNameShapes();
		_screen->sega_getRenderer()->fillRectWithTiles(0, 0, 0, 40, 28, 0x2000);
		_screen->sega_getRenderer()->fillRectWithTiles(1, 0, 0, 40, 28, 0x2000);
		_txt->clearDim(0);
	}
	_currentLevel = 1;
	_currentSub = 0;
	loadLevel(1, 0);
	_currentBlock = 490;
	_currentDirection = 0;
	setHandItem(0);

	EoBCoreEngine::startupNew();
}

void EoBEngine::startupLoad() {
	_sound->selectAudioResourceSet(kMusicIngame);
	_sound->loadSoundFile(0);
	_screen->selectPC98Palette(0, _screen->getPalette(0));
}

void EoBEngine::updateSpecialGfx() {
	bool updScreen = false;

	// Red grid effect
	for (int i = 0; i < 6; ++i) {
		if (!_characters[i].specialGfxCountdown)
			continue;
		_characters[i].specialGfxCountdown--;
		int cp = _screen->setCurPage(0);

		if (!_currentControlMode && (_characters[i].specialGfxCountdown & 1))
			_screen->drawShape(0, _redGrid, 176 + guiSettings()->charBoxCoords.facePosX_1[i & 1], guiSettings()->charBoxCoords.facePosY_1[i >> 1], 0);
		else if (_currentControlMode && _updateCharNum == i && (_characters[i].specialGfxCountdown & 1))
			_screen->drawShape(0, _redGrid, guiSettings()->charBoxCoords.facePosX_2[0], guiSettings()->charBoxCoords.facePosY_2[0], 0);
		else
			gui_drawFaceShape(i);

		_screen->setCurPage(cp);
		updScreen = true;
	}

	// Scene shake
	if (_specialGfxCountdown) {
		--_specialGfxCountdown;
		_sceneShakeOffsetX = _sceneShakeOffsets[_specialGfxCountdown << 1];
		_sceneShakeOffsetY = _sceneShakeOffsets[(_specialGfxCountdown << 1) + 1];
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
	_sceneDrawPage1 = 2;
}

void EoBEngine::drawNpcScene(int npcIndex) {
	_screen->copyRegion(0, 0, 0, 0, 176, 120, 6, 0, Screen::CR_NO_P_CHECK);
	switch (npcIndex) {
	case 0:
		encodeDrawNpcSeqShape(2, 88, 104);
		break;

	case 1:
		if (_npcSequenceSub == -1) {
			encodeDrawNpcSeqShape(0, 88, 104);
		} else {
			encodeDrawNpcSeqShape(0, 60, 104);
			encodeDrawNpcSeqShape(5, 116, 104);
		}
		break;

	case 2:
		if (_npcSequenceSub == -1) {
			encodeDrawNpcSeqShape(3, 88, 104);
		} else {
			encodeDrawNpcSeqShape(3, 60, 104);
			encodeDrawNpcSeqShape(_npcSubShpIndex1[_npcSequenceSub], 116, 104);
			encodeDrawNpcSeqShape(_npcSubShpIndex2[_npcSequenceSub], 116, _npcSubShpY[_npcSequenceSub]);
		}
		break;

	case 3:
		encodeDrawNpcSeqShape(7, 88, 104);
		break;

	case 4:
		encodeDrawNpcSeqShape(6, 88, 104);
		break;

	case 5:
		encodeDrawNpcSeqShape(18, 88, 88);
		break;

	case 6:
		encodeDrawNpcSeqShape(17, 88, 104);
		break;

	case 7:
		encodeDrawNpcSeqShape(4, 88, 104);
		break;

	default:
		break;
	}
}

void EoBEngine::encodeDrawNpcSeqShape(int npcIndex, int drawX, int drawY) {
	const uint8 *shpDef = &_npcShpData[npcIndex << 2];
	_screen->_curPage = 2;
	const uint8 *shp = _screen->encodeShape(shpDef[0], shpDef[1], shpDef[2], shpDef[3], false, _cgaMappingDefault);
	_screen->_curPage = 0;
	_screen->drawShape(0, shp, drawX - (shp[2] << 2), drawY - shp[1], 5);
	delete[] shp;
}

#define DLG2(txt, buttonstr) (runDialogue(txt, 2, _npcStrings[buttonstr][0], _npcStrings[buttonstr][1]) - 1)
#define DLG3(txt, buttonstr) (runDialogue(txt, 3, _npcStrings[buttonstr][0], _npcStrings[buttonstr][1], _npcStrings[buttonstr][2]) - 1)
#define DLG2A3(cond, txt, buttonstr1, buttonstr2) ((cond) ? (DLG2(txt, buttonstr1) ? 2 : 0) : DLG3(txt, buttonstr2))
#define TXT(txt) _txt->printDialogueText(txt, _moreStrings[0])

void EoBEngine::runNpcDialogue(int npcIndex) {
	int r = 0;
	int a = 0;
	Item itm = 0;

	switch (npcIndex) {
	case 0:
		for (r = 1; r == 1;) {
			gui_drawDialogueBox();
			r = DLG2A3(checkScriptFlags(0x2000), 8, 2, 1);
			if (r == 1) {
				TXT(1);
				setScriptFlags(0x2000);
			} else if (r == 0) {
				npcJoinDialogue(6, 12, 23, 2);
				setScriptFlags(0x4000);
			}
		}
		break;

	case 1:
		if (!checkScriptFlags(0x10000)) {
			if (checkScriptFlags(0x8000)) {
				a = 13;
			} else {
				setScriptFlags(0x8000);
				r = DLG2(3, 3);
				a = 4;
			}
			if (!r)
				r = DLG2(a, 4);

			if (!r) {
				for (a = 0; a < 6; a++)
					createItemOnCurrentBlock(55);
				createItemOnCurrentBlock(62);
				setScriptFlags(0x10000);
				TXT(6);
				npcJoinDialogue(7, 7, 29, 30);
			} else {
				TXT(5);
			}
			r = 1;
		}

		if (!checkScriptFlags(0x80000)) {
			for (a = 0; a < 6; a++) {
				if (testCharacter(a, 1) && _characters[a].portrait == -9)
					break;
			}
			if (a != 6) {
				TXT(25);
				TXT(26);
				setScriptFlags(0x80000);
				r = 1;
			}
		}

		if (!checkScriptFlags(0x100000)) {
			if (deletePartyItems(6, -1)) {
				_npcSequenceSub = 0;
				drawNpcScene(npcIndex);
				TXT(28);
				createItemOnCurrentBlock(32);
				setScriptFlags(0x100000);
				r = 1;
			}
		}

		if (!r)
			_txt->printDialogueText(_npcStrings[0][0], true);

		break;

	case 2:
		if (checkScriptFlags(0x10000)) {
			if (checkScriptFlags(0x20000)) {
				TXT(11);
			} else {
				r = DLG2A3(!countResurrectionCandidates(), 9, 5, 6);
				if (r < 2) {
					if (r == 0)
						healParty();
					else
						resurrectionSelectDialogue();
					setScriptFlags(0x20000);
				}
			}
		} else {
			TXT(24);
		}
		break;

	case 3:
		if (!DLG2(18, 7)) {
			setScriptFlags(0x8400000);
			for (a = 0; a < 30; a++) {
				if (_monsters[a].mode == 8)
					_monsters[a].mode = 5;
			}
		} else if (deletePartyItems(49, -1)) {
			TXT(20);
			setScriptFlags(0x400000);
		} else {
			TXT(19);
		}
		break;

	case 4:
		r = DLG3(14, 8);
		if (r == 0)
			setScriptFlags(0x200000);
		else if (r == 1)
			TXT(15);
		setScriptFlags(0x800000);
		break;

	case 5:
		if (!DLG2(16, 9)) {
			TXT(17);
			for (a = 0; a < 6; a++) {
				for (r = 0; r < 2; r++) {
					itm = _characters[a].inventory[r];
					if (itm && (_items[itm].type < 51 || _items[itm].type > 56)) {
						_characters[a].inventory[r] = 0;
						setItemPosition((Item *)&_levelBlockProperties[_currentBlock].drawObjects, _currentBlock, itm, _dropItemDirIndex[(_currentDirection << 2) + rollDice(1, 2, -1)]);
					}
				}
			}
		}
		setScriptFlags(0x2000000);
		break;

	case 6:
		TXT(21);
		setScriptFlags(0x1000000);
		break;

	case 7:
		r = DLG3(22, 10);
		if (r  < 2) {
			if (r == 0)
				npcJoinDialogue(8, 27, 44, 45);
			else
				TXT(31);
			setScriptFlags(0x4000000);
		}
		break;

	default:
		break;
	}
}

#undef TXT
#undef DLG2
#undef DLG3
#undef DLG2A3

void EoBEngine::updateUsedCharacterHandItem(int charIndex, int slot) {
	EoBItem *itm = &_items[_characters[charIndex].inventory[slot]];
	if (itm->type == 48) {
		int charges = itm->flags & 0x3F;
		if (--charges)
			--itm->flags;
		else
			deleteInventoryItem(charIndex, slot);
	} else if (itm->type == 34 || itm->type == 35) {
		deleteInventoryItem(charIndex, slot);
	}
}

void EoBEngine::loadMonsterShapes(const char *filename, int monsterIndex, bool hasDecorations, int encodeTableIndex) {
	if (_flags.platform != Common::kPlatformSegaCD) {
		EoBCoreEngine::loadMonsterShapes(filename, monsterIndex, hasDecorations, encodeTableIndex);
		return;
	}

	static const uint8 lvlEncodeTableIndex[] = {
		0x00, 0x00, 0x00, 0x01, 0x03, 0x02, 0x04, 0x05, 0x06, 0x06, 0x07, 0x06, 0x10,
		0x0e, 0x0c, 0x08, 0x0f, 0x14, 0x12, 0x09, 0x0b, 0x0a, 0x13, 0x11, 0x15, 0x0d
	};

	_sres->loadContainer(Common::String::format("L%d", _currentLevel));
	uint8 *data = _sres->resData(monsterIndex >> 4, 0);
	const uint8 *pos = data;

	int size = 0;
	const uint8 *enc = _staticres->loadRawData(kEoBBaseEncodeMonsterDefs00 + lvlEncodeTableIndex[(_currentLevel << 1) + (monsterIndex >> 4)], size);
	size >>= 1;
	assert(size <= 18);

	for (int i = 0; i < size; i++) {
		_monsterShapes[monsterIndex + i] = _screen->sega_convertShape(pos, enc[0], enc[1], 2);
		pos += ((enc[0] * enc[1]) >> 1);
		enc += 2;
	}

	delete[] data;

#if 0
	// DEBUG: display all the just encoded monster shapes on screen
	setLevelPalettes(_currentLevel);
	_screen->sega_fadeToNeutral(0);
	_screen->clearPage(0);
	uint16 x = 0;
	uint8 y = 0;
	uint8 hmax = 0;
	for (int i = 0; i < size; i++) {
		if (x + (_monsterShapes[monsterIndex + i][2] << 3) > 320) {
			y += hmax;
			x = hmax = 0;
		}
		hmax = MAX(_monsterShapes[monsterIndex + i][1], hmax);
		_screen->drawShape(0, _monsterShapes[monsterIndex + i], x, y);
		_screen->updateScreen();
		x += _monsterShapes[monsterIndex + i][2] << 3;
	}
	_screen->updateScreen();
#endif
}

void EoBEngine::replaceMonster(int unit, uint16 block, int pos, int dir, int type, int shpIndex, int mode, int h2, int randItem, int fixedItem) {
	if (_levelBlockProperties[block].flags & 7)
		return;

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].hitPointsCur <= 0) {
			initMonster(i, unit, block, pos, dir, type, shpIndex, mode, h2, randItem, fixedItem);
			break;
		}
	}
}

bool EoBEngine::killMonsterExtra(EoBMonsterInPlay *m) {
	if (m->type == 21) {
		_playFinale = true;
		_runFlag = false;
	}
	return true;
}

void EoBEngine::updateScriptTimersExtra() {
	int cnt = 0;
	for (int i = 1; i < 30; i++) {
		if (_monsters[i].hitPointsCur <= 0)
			cnt++;
	}

	if (!cnt) {
		for (int i = 1; i < 30; i++) {
			if (getBlockDistance(_monsters[i].block, _currentBlock) > 3) {
				killMonster(&_monsters[i], true);
				break;
			}
		}
	}
}

void EoBEngine::readLevelFileData(int level) {
	if (_flags.platform != Common::kPlatformSegaCD) {
		EoBCoreEngine::readLevelFileData(level);
		return;
	}
	_sres->loadContainer(Common::String::format("L%d", level));
	Common::SeekableReadStream *s = _sres->resStream(7);
	_screen->loadFileDataToPage(s, 5, 15000);
	delete s;
}

void EoBEngine::loadVcnData(const char *file, const uint8 *cgaMapping) {
	if (file)
		strcpy(_lastBlockDataFile, file);
	delete[] _vcnBlocks;

	Common::String fn = Common::String::format(_vcnFilePattern.c_str(), _lastBlockDataFile);
	if (_flags.platform == Common::kPlatformAmiga) {
		Common::SeekableReadStream *in = _res->createReadStream(fn);
		uint32 vcnSize = in->readUint16LE() * (_vcnSrcBitsPerPixel << 3);
		_vcnBlocks = new uint8[vcnSize];
		_screen->getPalette(1).loadAmigaPalette(*in, 1, 5);
		in->seek(22, SEEK_CUR);
		in->read(_vcnBlocks, vcnSize);
		delete in;
	} else if (_flags.platform == Common::kPlatformPC98) {
		_vcnBlocks = _res->fileData(fn.c_str(), 0);
	} else if (_flags.platform == Common::kPlatformSegaCD) {
		_sres->loadContainer(Common::String::format("L%d", _currentLevel));
		_vcnBlocks = _sres->resData(5, 0);
	} else {
		EoBCoreEngine::loadVcnData(file, cgaMapping);
	}
}

Common::SeekableReadStreamEndian *EoBEngine::getVmpData(const char *file) {
	if (_flags.platform != Common::kPlatformSegaCD)
		return EoBCoreEngine::getVmpData(file);
	_sres->loadContainer(Common::String::format("L%d", _currentLevel));
	return _sres->resStreamEndian(3);
}

const uint8 *EoBEngine::getBlockFileData(int level) {
	if (_flags.platform != Common::kPlatformSegaCD)
		return EoBCoreEngine::getBlockFileData(level);
	_sres->loadContainer(Common::String::format("L%d", level));
	Common::SeekableReadStream *s = _sres->resStream(6);
	_screen->loadFileDataToPage(s, 15, s->size());
	delete s;
	_dcrResCur = -1;
	return _screen->getCPagePtr(15);
}

Common::SeekableReadStreamEndian *EoBEngine::getDecDefinitions(const char *decFile) {
	if (_flags.platform != Common::kPlatformSegaCD)
		return EoBCoreEngine::getDecDefinitions(decFile);
	_sres->loadContainer(Common::String::format("L%d", _currentLevel));
	return _sres->resStreamEndian(4);
}

void EoBEngine::loadDecShapesToPage3(const char *shpFile) {
	if (_flags.platform != Common::kPlatformSegaCD) {
		EoBCoreEngine::loadDecShapesToPage3(shpFile);
		return;
	}
	if (_dcrResCur != _currentLevel) {
		_sres->loadContainer(Common::String::format("L%d", _currentLevel));
		Common::SeekableReadStream *s = _sres->resStream(2);
		_screen->loadFileDataToPage(s, 3, s->size());
		_dcrShpDataPos = _screen->getCPagePtr(3);
		_dcrResCur = _currentLevel;
		delete s;
	}
}

void EoBEngine::loadDoorShapes(int doorType1, int shapeId1, int doorType2, int shapeId2) {
	static const uint8 lvlIndex[13] = { 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x04 };
	const int doorType[2] = { doorType1, doorType2 };
	const int shapeId[2] = { shapeId1, shapeId2 };

	if (_flags.platform == Common::kPlatformSegaCD) {
		_sres->loadContainer(Common::String::format("L%d", _currentLevel));
		Common::SeekableReadStreamEndian *in = _sres->resStreamEndian(8);
		_screen->loadFileDataToPage(in, 2, in->size());
		delete in;
	} else {
		_screen->loadShapeSetBitmap("DOOR", 5, 3);
		_screen->_curPage = 2;
	}

	for (int a = 0; a < 2; ++a) {
		if (doorType[a] == 0xFF)
			continue;

		for (int i = 0; i < 3; i++) {
			if (_flags.platform == Common::kPlatformSegaCD) {
				int offs = lvlIndex[_currentLevel] * 6 + shapeId[a] + i;
				const uint8 *enc = &_doorShapeEncodeDefs[offs << 2];
				_doorShapes[shapeId[a] + i] = _screen->sega_convertShape(_doorShapesSrc[offs], enc[0] << 3, enc[1] << 3, 0, enc[2] - enc[3]);
				enc = &_doorSwitchShapeEncodeDefs[(offs << 2) - shapeId[a]];
				_doorSwitches[shapeId[a] + i].shp = _screen->sega_convertShape(_doorSwitchShapesSrc[offs], enc[0] << 3, enc[1] << 3, 0, enc[2] - enc[3]);
			} else {
				const uint8 *enc = &_doorShapeEncodeDefs[(doorType[a] * 3 + i) << 2];
				_doorShapes[shapeId[a] + i] = _screen->encodeShape(enc[0], enc[1], enc[2], enc[3], false, _cgaLevelMappingIndex ? _cgaMappingLevel[_cgaLevelMappingIndex[_currentLevel - 1]] : 0);
				enc = &_doorSwitchShapeEncodeDefs[(doorType[a] * 3 + i) << 2];
				_doorSwitches[shapeId[a] + i].shp = _screen->encodeShape(enc[0], enc[1], enc[2], enc[3], false, _cgaLevelMappingIndex ? _cgaMappingLevel[_cgaLevelMappingIndex[_currentLevel - 1]] : 0);
			}
			_doorSwitches[shapeId[a] + i].x = _doorSwitchCoords[doorType[a] * 6 + i * 2];
			_doorSwitches[shapeId[a] + i].y = _doorSwitchCoords[doorType[a] * 6 + i * 2 + 1];
		}
	}
	_screen->_curPage = 0;
}

void EoBEngine::drawDoorIntern(int type, int index, int x, int y, int w, int wall, int mDim, int16 y1, int16 y2) {
	int shapeIndex = type + 2 - mDim;
	uint8 *shp = _doorShapes[shapeIndex];
	if (!shp)
		return;

	int d1 = 0;
	int d2 = 0;
	int v = 0;
	const ScreenDim *td = _screen->getScreenDim(5);

	switch (_currentLevel) {
	case 4:
	case 5:
	case 6:
		y = _dscDoorY7[mDim] - shp[3];
		d1 = _dscDoorCoordsExt[index << 1] >> 3;
		d2 = _dscDoorCoordsExt[(index << 1) + 1] >> 3;
		if (_shpDmX1 > d1)
			d1 = _shpDmX1;
		if (_shpDmX2 < d2)
			d2 = _shpDmX2;
		_screen->modifyScreenDim(5, d1, td->sy, d2 - d1, td->h);
		v = ((wall < 30) ? (wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult3[mDim] : _dscDoorScaleMult4[mDim]) * -1;
		v -= (shp[2] << 3);
		drawBlockObject(0, 2, shp, x + v, y, 5);
		v += (shp[2] << 3);
		drawBlockObject(1, 2, shp, x - v, y, 5);
		if (_wllShapeMap[wall] == -1)
			drawBlockObject(0, 2, _doorSwitches[shapeIndex].shp, _doorSwitches[shapeIndex].x + w - v, _doorSwitches[shapeIndex].y, 5);
		break;

	case 7:
	case 8:
	case 9:
		y = _dscDoorY3[mDim] - _doorShapes[shapeIndex + 3][3];
		d1 = x - (_doorShapes[shapeIndex + 3][2] << 2);
		x -= (shp[2] << 2);
		drawBlockObject(0, 2, _doorShapes[shapeIndex + 3], d1, y, 5);
		setDoorShapeDim(index, y1, y2, 5);
		y = _dscDoorY3[mDim] - ((wall < 30) ? (wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult1[mDim] : _dscDoorScaleMult2[mDim]);
		drawBlockObject(0, 2, shp, x, y, 5);
		if (_wllShapeMap[wall] == -1)
			drawBlockObject(0, 2, _doorSwitches[shapeIndex].shp, _doorSwitches[shapeIndex].x + w, _doorSwitches[shapeIndex].y, 5);
		break;

	case 10:
	case 11:
		v = ((wall < 30) ? (wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult5[mDim] : _dscDoorScaleMult6[mDim]) * -1;
		x -= (shp[2] << 2);
		y = _dscDoorY4[mDim] + v;
		drawBlockObject(0, 2, shp, x, y + v, 5);
		v = (v >> 3) + (v >> 2);
		y = _dscDoorY5[mDim];
		drawBlockObject(0, 2, _doorShapes[shapeIndex + 3], x, y - v, 5);
		if (_wllShapeMap[wall] == -1)
			drawBlockObject(0, 2, _doorSwitches[shapeIndex].shp, _doorSwitches[shapeIndex].x + w, _doorSwitches[shapeIndex].y, 5);
		break;

	default:
		y = (_currentLevel == 12 ? _dscDoorY6[mDim] : _dscDoorY1[mDim]) - shp[3];
		x -= (shp[2] << 2);
		y -= (wall >= 30 ? _dscDoorScaleMult2[mDim] : (wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult1[mDim]);
		drawBlockObject(0, 2, shp, x, y, 5);

		if (_wllShapeMap[wall] == -1)
			drawBlockObject(0, 2, _doorSwitches[shapeIndex].shp, _doorSwitches[shapeIndex].x + w, _doorSwitches[shapeIndex].y, 5);
		break;
	}
}

void EoBEngine::setLevelPalettes(int level) {
	if (_flags.platform != Common::kPlatformSegaCD)
		return;

	static const uint8 palette0[13] = { 0x04, 0x10, 0x11, 0x12, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31 };
	static const uint8 palette2[13] = { 0x08, 0x08, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d };

	_screen->sega_selectPalette(palette0[level], 0);
	_screen->sega_selectPalette(palette2[level], 2);
}

void EoBEngine::playStrikeAnimation(uint8 pos, Item itm) {
	static const uint8 aX[5] = { 28, 116, 28, 116, 72 };
	static const uint8 aY[5] = { 60, 60, 40, 40, 50 };
	static const uint8 aTypes[5][5] = {
		{ 1,  3, 27,  48, 255 },
		{ 2, 24, 26,  54,  77 },
		{ 4,  6, 86, 255, 255 },
		{ 5, 73, 90, 255, 255 },
		{ 6, 35, 81, 255, 255 }
	};

	if (!_strikeAnimShapes[0])
		return;

	int aType = -1;
	for (int i = 0; i < 5 && aType == -1; ++i) {
		const uint8 *p = aTypes[i];
		uint8 t = *p++;
		for (int ii = 1; ii < 5 && aType == -1; ++ii) {
			if (_items[itm].nameUnid == *p++)
				aType = t;
		}
	}

	if (aType < 0)
		return;

	int16 x = aX[pos];
	int16 y = aY[pos];
	for (int i = 0; i < 5; ++i) {
		uint32 del = _system->getMillis() + _tickLength;
		_screen->copyRegionToBuffer(0, x, y, 32, 32, _spellAnimBuffer);
		_screen->drawShape(0, _strikeAnimShapes[aType][MIN(i, 3)], x, y);
		_screen->updateScreen();
		_screen->copyBlockToPage(0, x, y, 32, 32, _spellAnimBuffer);
		delayUntil(del);
	}
}

void EoBEngine::turnUndeadAuto() {
	if (_currentLevel != 2 && _currentLevel != 7)
		return;

	int oc = _openBookChar;

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 0x0D))
			continue;

		EoBCharacter *c = &_characters[i];

		if (_itemTypes[_items[c->inventory[0]].type].extraProperties != 6 && _itemTypes[_items[c->inventory[1]].type].extraProperties != 6)
			continue;

		int l = getCharacterLevelIndex(2, c->cClass);
		if (l > -1) {
			if (c->level[l] > _openBookCasterLevel) {
				_openBookCasterLevel = c->level[l];
				_openBookChar = i;
			}
		} else {
			l = getCharacterLevelIndex(4, c->cClass);
			if (l > -1) {
				if ((c->level[l] - 2) > _openBookCasterLevel) {
					_openBookCasterLevel = (c->level[l] - 2);
					_openBookChar = i;
				}
			}
		}
	}

	if (_openBookCasterLevel)
		spellCallback_start_turnUndead();

	_openBookChar = oc;
	_openBookCasterLevel = 0;
}

void EoBEngine::turnUndeadAutoHit() {
	_txt->printMessage(_turnUndeadString[0], -1, _characters[_openBookChar].name);
	snd_playSoundEffect(_flags.platform == Common::kPlatformAmiga ? 16 : 95);
	sparkEffectOffensive();
}

void EoBEngine::snd_loadAmigaSounds(int level, int) {
	if (_flags.platform != Common::kPlatformAmiga || level == _amigaCurSoundFile)
		return;

	if (_amigaCurSoundFile != -1) {
		_sound->unloadSoundFile(Common::String::format("L%dM1A1", _amigaCurSoundFile));
		_sound->unloadSoundFile(Common::String::format("L%dM2A1", _amigaCurSoundFile));

		for (int i = 1; i < 5; ++i) {
			_sound->unloadSoundFile(Common::String::format("L%dM1M%d", _amigaCurSoundFile, i));
			_sound->unloadSoundFile(Common::String::format("L%dM2M%d", _amigaCurSoundFile, i));
		}

		for (int i = 0; i < 2; ++i) {
			if (_amigaLevelSoundList1[_amigaCurSoundFile * 2 + i][0])
				_sound->unloadSoundFile(_amigaLevelSoundList1[_amigaCurSoundFile * 2 + i]);
			if (_amigaLevelSoundList2[_amigaCurSoundFile * 2 + i][0])
				_sound->unloadSoundFile(_amigaLevelSoundList2[_amigaCurSoundFile * 2 + i]);
		}
	}

	for (int i = 0; i < 2; ++i) {
		if (_amigaLevelSoundList1[level * 2 + i][0])
			_sound->loadSoundFile(Common::String::format("%s.CPS", _amigaLevelSoundList1[level * 2 + i]));
		if (_amigaLevelSoundList2[level * 2 + i][0])
			_sound->loadSoundFile(Common::String::format("%s.CPS", _amigaLevelSoundList2[level * 2 + i]));
	}

	_sound->loadSoundFile(Common::String::format("LEVELSAM%d.CPS", level));

	_amigaCurSoundFile = level;
}

void EoBEngine::snd_updateLevelScore() {
	if (_flags.platform != Common::kPlatformSegaCD || _currentLevel != 5)
		return;

	int x = _currentBlock & 0x1F;
	int y = (_currentBlock >> 5) & 0x1F;

	int track = (x >= 14 && x < 20 && y >= 7 && y < 15) ? (x == 14 && y == 14 ? 6 : 12) : (x == 17 && y == 6 ? 12 : 6);
	if (track == _levelCurTrack)
		return;

	_levelCurTrack = track;
	snd_playSong(track);
}

bool EoBEngine::checkPartyStatusExtra() {
	_screen->copyPage(0, Screen_EoB::kDefeatMsgBackupPage);
	int cd = _screen->curDimIndex();
	gui_drawBox(0, 121, 320, 80, guiSettings()->colors.frame1, guiSettings()->colors.frame2, guiSettings()->colors.fill);
	_txt->setupField(9, false);
	_txt->printMessage(_menuStringsDefeat[0]);
	while (!shouldQuit()) {
		removeInputTop();
		if (checkInput(0, false, 0) & 0xFF)
			break;
	}
	_screen->copyPage(Screen_EoB::kDefeatMsgBackupPage, 0);
	_eventList.clear();
	_screen->setScreenDim(cd);
	_txt->removePageBreakFlag();
	return true;
}

int EoBEngine::resurrectionSelectDialogue() {
	gui_drawDialogueBox();
	_txt->printDialogueText(_npcStrings[0][1]);

	int r = _rrId[runDialogue(-1, 9, _rrNames[0], _rrNames[1], _rrNames[2], _rrNames[3], _rrNames[4], _rrNames[5], _rrNames[6], _rrNames[7], _rrNames[8]) - 1];

	if (r < 0) {
		r = -r;
		deletePartyItems(33, r);
		_npcSequenceSub = r - 1;
		drawNpcScene(2);
		npcJoinDialogue(_npcSequenceSub, 32 + (_npcSequenceSub << 1), -1, 33 + (_npcSequenceSub << 1));
	} else {
		_characters[r].hitPointsCur = _characters[r].hitPointsMax;
	}

	return 1;
}

void EoBEngine::healParty() {
	int cnt = rollDice(1, 3, 2);
	for (int i = 0; i < 6 && cnt; i++) {
		if (testCharacter(i, 3))
			continue;

		_characters[i].flags &= ~4;
		neutralizePoison(i);

		if (_characters[i].hitPointsCur >= _characters[i].hitPointsMax)
			continue;

		cnt--;
		_characters[i].hitPointsCur += rollDice(1, 8, 9);
		if (_characters[i].hitPointsCur > _characters[i].hitPointsMax)
			_characters[i].hitPointsCur = _characters[i].hitPointsMax;
	}
}

const KyraRpgGUISettings *EoBEngine::guiSettings() const {
	if (_flags.platform == Common::kPlatformAmiga)
		return _useMainMenuGUISettings ? &_guiSettingsAmigaMainMenu : &_guiSettingsAmiga;
	else if (_configRenderMode == Common::kRenderCGA || _configRenderMode == Common::kRenderEGA)
		return &_guiSettingsEGA;
	else if (_flags.platform == Common::kPlatformPC98)
		return &_guiSettingsPC98;
	else if (_flags.platform == Common::kPlatformSegaCD)
		return &_guiSettingsSegaCD;
	else
		return &_guiSettingsVGA;
}

} // End of namespace Kyra

#endif // ENABLE_EOB
