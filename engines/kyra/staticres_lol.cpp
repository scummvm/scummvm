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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "kyra/resource.h"
#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/gui_lol.h"

#ifdef ENABLE_LOL

namespace Kyra {

const LoLCharacter *StaticResource::loadCharData(int id, int &entries) {
	return (const LoLCharacter *)getData(id, kLolCharData, entries);
}

const SpellProperty *StaticResource::loadSpellData(int id, int &entries) {
	return (const SpellProperty *)getData(id, kLolSpellData, entries);
}

const CompassDef *StaticResource::loadCompassData(int id, int &entries) {
	return (const CompassDef *)getData(id, kLolCompassData, entries);
}

const FlyingObjectShape *StaticResource::loadFlyingObjectData(int id, int &entries) {
	return (const FlyingObjectShape *)getData(id, kLolFlightShpData, entries);
}

const uint16 *StaticResource::loadRawDataBe16(int id, int &entries) {
	return (const uint16 *)getData(id, kLolRawDataBe16, entries);
}

const uint32 *StaticResource::loadRawDataBe32(int id, int &entries) {
	return (const uint32 *)getData(id, kLolRawDataBe32, entries);
}

const ButtonDef *StaticResource::loadButtonDefs(int id, int &entries) {
	return (const ButtonDef *)getData(id, kLolButtonData, entries);
}

bool StaticResource::loadCharData(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	size = stream.size() / 130;
	LoLCharacter *charData = new LoLCharacter[size];

	for (int i = 0; i < size; i++) {
		LoLCharacter *t = &charData[i];

		t->flags = stream.readUint16LE();
		stream.read(t->name, 11);
		t->raceClassSex = stream.readByte();
		t->id = stream.readSint16LE();
		t->curFaceFrame = stream.readByte();
		t->tempFaceFrame = stream.readByte();
		t->screamSfx = stream.readByte();
		stream.readUint32LE();
		for (int ii = 0; ii < 8; ii++)
			t->itemsMight[ii] = stream.readUint16LE();
		for (int ii = 0; ii < 8; ii++)
			t->protectionAgainstItems[ii] = stream.readUint16LE();
		t->itemProtection = stream.readUint16LE();
		t->hitPointsCur = stream.readSint16LE();
		t->hitPointsMax = stream.readUint16LE();
		t->magicPointsCur = stream.readSint16LE();
		t->magicPointsMax = stream.readUint16LE();
		t->field_41 = stream.readByte();
		t->damageSuffered = stream.readUint16LE();
		t->weaponHit = stream.readUint16LE();
		t->totalMightModifier = stream.readUint16LE();
		t->totalProtectionModifier = stream.readUint16LE();
		t->might = stream.readUint16LE();
		t->protection = stream.readUint16LE();
		t->nextAnimUpdateCountdown = stream.readSint16LE();
		for (int ii = 0; ii < 11; ii++)
			t->items[ii] = stream.readUint16LE();
		for (int ii = 0; ii < 3; ii++)
			t->skillLevels[ii] = stream.readByte();
		for (int ii = 0; ii < 3; ii++)
			t->skillModifiers[ii] = stream.readByte();
		for (int ii = 0; ii < 3; ii++)
			t->experiencePts[ii] = stream.readUint32LE();
		for (int ii = 0; ii < 5; ii++)
			t->characterUpdateEvents[ii] = stream.readByte();
		for (int ii = 0; ii < 5; ii++)
			t->characterUpdateDelay[ii] = stream.readByte();
	};

	ptr = charData;
	return true;
}

bool StaticResource::loadSpellData(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	size = stream.size() / 28;
	SpellProperty *spellData = new SpellProperty[size];

	for (int i = 0; i < size; i++) {
		SpellProperty *t = &spellData[i];

		t->spellNameCode = stream.readUint16LE();
		for (int ii = 0; ii < 4; ii++)
			t->mpRequired[ii] = stream.readUint16LE();
		t->field_a = stream.readUint16LE();
		t->field_c = stream.readUint16LE();
		for (int ii = 0; ii < 4; ii++)
			t->hpRequired[ii] = stream.readUint16LE();
		t->field_16 = stream.readUint16LE();
		t->field_18 = stream.readUint16LE();
		t->flags = stream.readUint16LE();
	};

	ptr = spellData;
	return true;
}

bool StaticResource::loadCompassData(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	size = stream.size() / 4;
	CompassDef *defs = new CompassDef[size];

	for (int i = 0; i < size; i++) {
		CompassDef *t = &defs[i];
		t->shapeIndex = stream.readByte();
		t->x = stream.readByte();
		t->y = stream.readByte();
		t->flags = stream.readByte();
	};


	ptr = defs;
	return true;
}

bool StaticResource::loadFlyingObjectData(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	size = stream.size() / 5;
	FlyingObjectShape *defs = new FlyingObjectShape[size];

	for (int i = 0; i < size; i++) {
		FlyingObjectShape *t = &defs[i];
		t->shapeFront = stream.readByte();
		t->shapeBack = stream.readByte();
		t->shapeLeft = stream.readByte();
		t->drawFlags = stream.readByte();
		t->flipFlags = stream.readByte();
	};

	ptr = defs;
	return true;
}

bool StaticResource::loadRawDataBe16(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	size = stream.size() >> 1;

	uint16 *r = new uint16[size];

	for (int i = 0; i < size; i++)
		r[i] = stream.readUint16BE();

	ptr = r;
	return true;
}

bool StaticResource::loadRawDataBe32(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	size = stream.size() >> 2;

	uint32 *r = new uint32[size];

	for (int i = 0; i < size; i++)
		r[i] = stream.readUint32BE();

	ptr = r;
	return true;
}

bool StaticResource::loadButtonDefs(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	size = stream.size() / 18;

	ButtonDef *r = new ButtonDef[size];

	for (int i = 0; i < size; i++) {
		r[i].buttonflags = stream.readUint16BE();
		r[i].keyCode = stream.readUint16BE();
		r[i].keyCode2 = stream.readUint16BE();
		r[i].x = stream.readSint16BE();
		r[i].y = stream.readSint16BE();
		r[i].w = stream.readUint16BE();
		r[i].h = stream.readUint16BE();
		r[i].index = stream.readUint16BE();
		r[i].screenDim = stream.readUint16BE();
	}

	ptr = r;
	return true;
}

void StaticResource::freeCharData(void *&ptr, int &size) {
	LoLCharacter *d = (LoLCharacter *)ptr;
	delete[] d;
	ptr = 0;
	size = 0;
}

void StaticResource::freeSpellData(void *&ptr, int &size) {
	SpellProperty *d = (SpellProperty *)ptr;
	delete[] d;
	ptr = 0;
	size = 0;
}

void StaticResource::freeCompassData(void *&ptr, int &size) {
	CompassDef *d = (CompassDef *)ptr;
	delete[] d;
	ptr = 0;
	size = 0;
}

void StaticResource::freeFlyingObjectData(void *&ptr, int &size) {
	FlyingObjectShape *d = (FlyingObjectShape *)ptr;
	delete[] d;
	ptr = 0;
	size = 0;
}


void StaticResource::freeRawDataBe16(void *&ptr, int &size) {
	uint16 *data = (uint16 *)ptr;
	delete[] data;
	ptr = 0;
	size = 0;
}

void StaticResource::freeRawDataBe32(void *&ptr, int &size) {
	uint32 *data = (uint32 *)ptr;
	delete[] data;
	ptr = 0;
	size = 0;
}

void StaticResource::freeButtonDefs(void *&ptr, int &size) {
	ButtonDef *d = (ButtonDef *)ptr;
	delete[] d;
	ptr = 0;
	size = 0;
}

void LoLEngine::initStaticResource() {
	// assign music data
	static const char *pcMusicFileListIntro[] = { "LOREINTR" };
	static const char *pcMusicFileListFinale[] = { "LOREFINL" };
	static const char *pcMusicFileListIngame[] = { "LORE%02d%c" };

	static const char *pc98MusicFileListIntro[] = { 0, "lore84.86", "lore82.86", 0, 0, 0, "lore83.86", "lore81.86" };
	static const char *pc98MusicFileListFinale[] = { 0, 0, "lore85.86", "lore86.86", "lore87.86" };
	static const char *pc98MusicFileListIngame[] = { "lore%02d.86" };

	memset(_soundData, 0, sizeof(_soundData));
	if (_flags.platform == Common::kPlatformPC) {
		_soundData[0].fileList = pcMusicFileListIntro;
		_soundData[0].fileListLen = ARRAYSIZE(pcMusicFileListIntro);
		_soundData[1].fileList = pcMusicFileListIngame;
		_soundData[1].fileListLen = ARRAYSIZE(pcMusicFileListIngame);
		_soundData[2].fileList = pcMusicFileListFinale;
		_soundData[2].fileListLen = ARRAYSIZE(pcMusicFileListFinale);
	} else if (_flags.platform == Common::kPlatformPC98) {
		_soundData[0].fileList = pc98MusicFileListIntro;
		_soundData[0].fileListLen = ARRAYSIZE(pc98MusicFileListIntro);
		_soundData[1].fileList = pc98MusicFileListIngame;
		_soundData[1].fileListLen = ARRAYSIZE(pc98MusicFileListIngame);
		_soundData[2].fileList = pc98MusicFileListFinale;
		_soundData[2].fileListLen = ARRAYSIZE(pc98MusicFileListFinale);
	}

	if (_flags.isDemo)
		return;

	_pakFileList = _staticres->loadStrings(kLolIngamePakFiles, _pakFileListSize);
	_charDefaults = _staticres->loadCharData(kLolCharacterDefs, _charDefaultsSize);
	_ingameSoundIndex = (const uint16 *)_staticres->loadRawData(kLolIngameSfxIndex, _ingameSoundIndexSize);
	_musicTrackMap = _staticres->loadRawData(kLolMusicTrackMap, _musicTrackMapSize);
	_ingameGMSoundIndex = _staticres->loadRawData(kLolIngameGMSfxIndex, _ingameGMSoundIndexSize);
	_ingameMT32SoundIndex = _staticres->loadRawData(kLolIngameMT32SfxIndex, _ingameMT32SoundIndexSize);
	_ingamePCSpeakerSoundIndex = _staticres->loadRawData(kLolIngamePcSpkSfxIndex, _ingamePCSpeakerSoundIndexSize);
	_spellProperties = _staticres->loadSpellData(kLolSpellProperties, _spellPropertiesSize);
	_gameShapeMap = (const int8 *)_staticres->loadRawData(kLolGameShapeMap, _gameShapeMapSize);
	_sceneItemOffs = (const int8 *)_staticres->loadRawData(kLolSceneItemOffs, _sceneItemOffsSize);
	_charInvIndex = _staticres->loadRawData(kLolCharInvIndex, _charInvIndexSize);
	_charInvDefs = _staticres->loadRawData(kLolCharInvDefs, _charInvDefsSize);
	_charDefsMan = _staticres->loadRawDataBe16(kLolCharDefsMan, _charDefsManSize);
	_charDefsWoman = _staticres->loadRawDataBe16(kLolCharDefsWoman, _charDefsWomanSize);
	_charDefsKieran = _staticres->loadRawDataBe16(kLolCharDefsKieran, _charDefsKieranSize);
	_charDefsAkshel = _staticres->loadRawDataBe16(kLolCharDefsAkshel, _charDefsAkshelSize);
	_expRequirements = (const int32 *)_staticres->loadRawDataBe32(kLolExpRequirements, _expRequirementsSize);
	_monsterModifiers = _staticres->loadRawDataBe16(kLolMonsterModifiers, _monsterModifiersSize);
	_monsterShiftOffs = (const int8 *)_staticres->loadRawData(kLolMonsterShiftOffsets, _monsterShiftOffsSize);
	_monsterDirFlags = _staticres->loadRawData(kLolMonsterDirFlags, _monsterDirFlagsSize);
	_monsterScaleX = _staticres->loadRawData(kLolMonsterScaleX, _monsterScaleXSize);
	_monsterScaleY = _staticres->loadRawData(kLolMonsterScaleY, _monsterScaleYSize);
	_monsterScaleWH = _staticres->loadRawDataBe16(kLolMonsterScaleWH, _monsterScaleWHSize);
	_inventorySlotDesc = _staticres->loadRawDataBe16(kLolInventoryDesc, _inventorySlotDescSize);
	_levelShpList = _staticres->loadStrings(kLolLevelShpList, _levelShpListSize);
	_levelDatList = _staticres->loadStrings(kLolLevelDatList, _levelDatListSize);
	_compassDefs = _staticres->loadCompassData(kLolCompassDefs, _compassDefsSize);
	_flyingItemShapes = _staticres->loadFlyingObjectData(kLolFlyingObjectShp, _flyingItemShapesSize);
	_itemCost = _staticres->loadRawDataBe16(kLolItemPrices, _itemCostSize);
	_stashSetupData = _staticres->loadRawData(kLolStashSetup, _stashSetupDataSize);

	_dscUnk1 = (const int8 *)_staticres->loadRawData(kLolDscUnk1, _dscUnk1Size);
	_dscShapeIndex = (const int8 *)_staticres->loadRawData(kLolDscShapeIndex, _dscShapeIndexSize);
	_dscOvlMap = _staticres->loadRawData(kLolDscOvlMap, _dscOvlMapSize);
	_dscShapeScaleW = _staticres->loadRawDataBe16(kLolDscScaleWidthData, _dscShapeScaleWSize);
	_dscShapeScaleH = _staticres->loadRawDataBe16(kLolDscScaleHeightData, _dscShapeScaleHSize);
	_dscShapeX = (const int16 *)_staticres->loadRawDataBe16(kLolDscX, _dscShapeXSize);
	_dscShapeY = (const int8 *)_staticres->loadRawData(kLolDscY, _dscShapeYSize);
	_dscTileIndex = _staticres->loadRawData(kLolDscTileIndex, _dscTileIndexSize);
	_dscUnk2 = _staticres->loadRawData(kLolDscUnk2, _dscUnk2Size);
	_dscDoorShpIndex = _staticres->loadRawData(kLolDscDoorShapeIndex, _dscDoorShpIndexSize);
	_dscDim1 = (const int8 *)_staticres->loadRawData(kLolDscDimData1, _dscDim1Size);
	_dscDim2 = (const int8 *)_staticres->loadRawData(kLolDscDimData2, _dscDim2Size);
	_dscBlockMap = _staticres->loadRawData(kLolDscBlockMap, _dscBlockMapSize);
	_dscDimMap = _staticres->loadRawData(kLolDscDimMap, _dscDimMapSize);
	_dscDoorMonsterScaleTable = _staticres->loadRawDataBe16(kLolDscDoorScale, _dscDoorMonsterScaleTableSize);
	_dscShapeOvlIndex = _staticres->loadRawData(kLolDscOvlIndex, _dscShapeOvlIndexSize);
	_dscDoor4 = _staticres->loadRawDataBe16(kLolDscDoor4, _dscDoor4Size);
	_dscBlockIndex = (const int8 *)_staticres->loadRawData(kLolDscBlockIndex, _dscBlockIndexSize);
	_dscDoor1 = _staticres->loadRawData(kLolDscDoor1, _dscDoor1Size);
	_dscDoorMonsterX = (const int16 *)_staticres->loadRawDataBe16(kLolDscDoorX, _dscDoorMonsterXSize);
	_dscDoorMonsterY = (const int16 *)_staticres->loadRawDataBe16(kLolDscDoorY, _dscDoorMonsterYSize);

	_scrollXTop = _staticres->loadRawData(kLolScrollXTop, _scrollXTopSize);
	_scrollYTop = _staticres->loadRawData(kLolScrollYTop, _scrollYTopSize);
	_scrollXBottom = _staticres->loadRawData(kLolScrollXBottom, _scrollXBottomSize);
	_scrollYBottom = _staticres->loadRawData(kLolScrollYBottom, _scrollYBottomSize);

	const char *const *tmpSndList = _staticres->loadStrings(kLolIngameSfxFiles, _ingameSoundListSize);
	if (tmpSndList) {
		_ingameSoundList = new char *[_ingameSoundListSize];
		for (int i = 0; i < _ingameSoundListSize; i++) {
			_ingameSoundList[i] = new char[strlen(tmpSndList[i]) + 1];
			strcpy(_ingameSoundList[i], tmpSndList[i]);
		}
		_staticres->unloadId(kLolIngameSfxFiles);
	}

	_buttonData = _staticres->loadButtonDefs(kLolButtonDefs, _buttonDataSize);
	_buttonList1 = (const int16 *)_staticres->loadRawDataBe16(kLolButtonList1, _buttonList1Size);
	_buttonList2 = (const int16 *)_staticres->loadRawDataBe16(kLolButtonList2, _buttonList2Size);
	_buttonList3 = (const int16 *)_staticres->loadRawDataBe16(kLolButtonList3, _buttonList3Size);
	_buttonList4 = (const int16 *)_staticres->loadRawDataBe16(kLolButtonList4, _buttonList4Size);
	_buttonList5 = (const int16 *)_staticres->loadRawDataBe16(kLolButtonList5, _buttonList5Size);
	_buttonList6 = (const int16 *)_staticres->loadRawDataBe16(kLolButtonList6, _buttonList6Size);
	_buttonList7 = (const int16 *)_staticres->loadRawDataBe16(kLolButtonList7, _buttonList7Size);
	_buttonList8 = (const int16 *)_staticres->loadRawDataBe16(kLolButtonList8, _buttonList8Size);

	_autoMapStrings = _staticres->loadRawDataBe16(kLolMapStringId, _autoMapStringsSize);

	int tempSize;
	const uint8 *tmp = _staticres->loadRawData(kLolLegendData, tempSize);
	uint8 entrySize = tempSize / 12;
	tempSize /= entrySize;
	if (tempSize) {
		_defaultLegendData = new MapLegendData[tempSize];
		for (int i = 0; i < tempSize; i++) {
			_defaultLegendData[i].shapeIndex = *tmp++;
			_defaultLegendData[i].enable = *tmp++ ? true : false;
			_defaultLegendData[i].y = (entrySize == 5) ? (int8)*tmp++ : (i == 10 ? -5 : 0);
			_defaultLegendData[i].stringId = READ_LE_UINT16(tmp);
			tmp += 2;
		}
		_staticres->unloadId(kLolLegendData);
	}

	tmp = _staticres->loadRawData(kLolMapCursorOvl, tempSize);
	_mapCursorOverlay = new uint8[tempSize];
	memcpy(_mapCursorOverlay, tmp, tempSize);
	_staticres->unloadId(kLolMapCursorOvl);

	_updateSpellBookCoords = _staticres->loadRawData(kLolSpellbookCoords, _updateSpellBookCoordsSize);
	_updateSpellBookAnimData = _staticres->loadRawData(kLolSpellbookAnim, _updateSpellBookAnimDataSize);
	_healShapeFrames = _staticres->loadRawData(kLolHealShapeFrames, _healShapeFramesSize);

	tmp = _staticres->loadRawData(kLolLightningDefs, tempSize);
	if (tmp) {
		_lightningProps = new LightningProperty[5];
		for (int i = 0; i < 5; i++) {
			_lightningProps[i].lastFrame = tmp[i << 2];
			_lightningProps[i].frameDiv = tmp[(i << 2) + 1];
			_lightningProps[i].sfxId = READ_LE_UINT16(&tmp[(i << 2) + 2]);
		}
		_staticres->unloadId(kLolLightningDefs);
	}

	_fireBallCoords = (const int16 *)_staticres->loadRawDataBe16(kLolFireballCoords, _fireBallCoordsSize);

	_buttonCallbacks.clear();
	_buttonCallbacks.reserve(95);
#define cb(x) _buttonCallbacks.push_back(BUTTON_FUNCTOR(LoLEngine, this, &LoLEngine::x))
	// 0x00
	cb(clickedUpArrow);
	cb(clickedDownArrow);
	_buttonCallbacks.push_back(_buttonCallbacks[1]);
	cb(clickedLeftArrow);

	// 0x04
	cb(clickedRightArrow);
	cb(clickedTurnLeftArrow);
	cb(clickedTurnRightArrow);
	cb(clickedAttackButton);

	// 0x08
	for (int i = 0; i < 3; ++i)
		_buttonCallbacks.push_back(_buttonCallbacks[7]);
	cb(clickedMagicButton);

	// 0x0C
	for (int i = 0; i < 3; ++i)
		_buttonCallbacks.push_back(_buttonCallbacks[11]);
	cb(clickedMagicSubmenu);

	// 0x10
	cb(clickedScreen);
	cb(clickedPortraitLeft);
	for (int i = 0; i < 7; ++i)
		_buttonCallbacks.push_back(_buttonCallbacks[17]);

	// 0x19
	cb(clickedLiveMagicBarsLeft);
	for (int i = 0; i < 3; ++i)
		_buttonCallbacks.push_back(_buttonCallbacks[25]);

	// 0x1D
	cb(clickedPortraitEtcRight);
	for (int i = 0; i < 3; ++i)
		_buttonCallbacks.push_back(_buttonCallbacks[29]);

	// 0x21
	cb(clickedCharInventorySlot);
	for (int i = 0; i < 10; ++i)
		_buttonCallbacks.push_back(_buttonCallbacks[33]);

	// 0x2C
	cb(clickedExitCharInventory);
	cb(clickedSceneDropItem);
	for (int i = 0; i < 3; ++i)
		_buttonCallbacks.push_back(_buttonCallbacks[45]);

	// 0x31
	cb(clickedScenePickupItem);
	cb(clickedInventorySlot);
	for (int i = 0; i < 9; ++i)
		_buttonCallbacks.push_back(_buttonCallbacks[50]);

	// 0x3C
	cb(clickedInventoryScroll);
	cb(clickedInventoryScroll);
	cb(clickedWall);
	_buttonCallbacks.push_back(_buttonCallbacks[62]);

	// 0x40
	cb(clickedSequenceWindow);
	_buttonCallbacks.push_back(_buttonCallbacks[0]);
	_buttonCallbacks.push_back(_buttonCallbacks[1]);
	_buttonCallbacks.push_back(_buttonCallbacks[3]);

	// 0x44
	_buttonCallbacks.push_back(_buttonCallbacks[4]);
	_buttonCallbacks.push_back(_buttonCallbacks[5]);
	_buttonCallbacks.push_back(_buttonCallbacks[6]);
	cb(clickedScroll);

	// 0x48
	for (int i = 0; i < 9; ++i)
		_buttonCallbacks.push_back(_buttonCallbacks[71]);

	// 0x51
	cb(clickedSpellTargetCharacter);
	for (int i = 0; i < 3; ++i)
		_buttonCallbacks.push_back(_buttonCallbacks[81]);

	// 0x55
	cb(clickedSpellTargetScene);
	cb(clickedSceneThrowItem);
	_buttonCallbacks.push_back(_buttonCallbacks[86]);

	// 0x58
	cb(clickedOptions);
	cb(clickedRestParty);
	cb(clickedMoneyBox);
	cb(clickedCompass);

	// 0x5C
	cb(clickedAutomap);
	cb(clickedLamp);
	cb(clickedStatusIcon);
#undef cb
}

void GUI_LoL::initStaticData() {
	GUI_V2_BUTTON(_scrollUpButton, 20, 96, 0, 1, 1, 1, 0x4487, 0, 0, 0, 25, 16, 0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01, 0);
	GUI_V2_BUTTON(_scrollDownButton, 21, 98, 0, 1, 1, 1, 0x4487, 0, 0, 0, 25, 16, 0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01, 0);

	for (uint i = 0; i < ARRAYSIZE(_menuButtons); ++i)
		GUI_V2_BUTTON(_menuButtons[i], i, 0, 0, 0, 0, 0, 0x4487, 0, 0, 0, 0, 0, 0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01, 0);

	if (_vm->gameFlags().isTalkie)
		GUI_LOL_MENU(_mainMenu, 9, 0x4000, 0, 7, -1, -1, -1, -1);
	 else
		GUI_LOL_MENU(_mainMenu, 17, 0x4000, 0, 6, -1, -1, -1, -1);

	GUI_LOL_MENU_ITEM(_mainMenu.item[0], 0x4001, 16, 23, 176, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_mainMenu.item[1], 0x4002, 16, 40, 176, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_mainMenu.item[2], 0x4003, 16, 57, 176, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_mainMenu.item[3], 0x4004, 16, 74, 176, 15, 0, 0);

	if (_vm->gameFlags().isTalkie) {
		GUI_LOL_MENU_ITEM(_mainMenu.item[4], 0x42D9, 16, 91, 176, 15, 0, 0);
		GUI_LOL_MENU_ITEM(_mainMenu.item[5], 0x4006, 16, 108, 176, 15, 0, 0);
		GUI_LOL_MENU_ITEM(_mainMenu.item[6], 0x4005, 88, 127, 104, 15, 0, _vm->_keyMap[Common::KEYCODE_ESCAPE]);
	} else {
		GUI_LOL_MENU_ITEM(_mainMenu.item[4], 0x4006, 16, 91, 176, 15, 0, 0);
		GUI_LOL_MENU_ITEM(_mainMenu.item[5], 0x4005, 88, 110, 104, 15, 0, _vm->_keyMap[Common::KEYCODE_ESCAPE]);
	}

	Button::Callback mainMenuFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::clickedMainMenu);
	for (int i = 0; i < _mainMenu.numberOfItems; ++i)
		_mainMenu.item[i].callback = mainMenuFunctor;

	GUI_LOL_MENU(_loadMenu, 10, 0x400e, 1, 5, 128, 20, 128, 118);
	GUI_LOL_MENU_ITEM(_loadMenu.item[0], 0xfffe, 8, 39, 256, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_loadMenu.item[1], 0xfffd, 8, 56, 256, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_loadMenu.item[2], 0xfffc, 8, 73, 256, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_loadMenu.item[3], 0xfffb, 8, 90, 256, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_loadMenu.item[4], 0x4011, 168, 118, 96, 15, 0, _vm->_keyMap[Common::KEYCODE_ESCAPE]);
	Button::Callback loadMenuFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::clickedLoadMenu);
	for (int i = 0; i < 5; ++i)
		_loadMenu.item[i].callback = loadMenuFunctor;

	GUI_LOL_MENU(_saveMenu, 10, 0x400d, 1, 5, 128, 20, 128, 118);
	GUI_LOL_MENU_ITEM(_saveMenu.item[0], 0xfffe, 8, 39, 256, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_saveMenu.item[1], 0xfffd, 8, 56, 256, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_saveMenu.item[2], 0xfffc, 8, 73, 256, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_saveMenu.item[3], 0xfffb, 8, 90, 256, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_saveMenu.item[4], 0x4011, 168, 118, 96, 15, 0, _vm->_keyMap[Common::KEYCODE_ESCAPE]);
	Button::Callback saveMenuFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::clickedSaveMenu);
	for (int i = 0; i < 5; ++i)
		_saveMenu.item[i].callback = saveMenuFunctor;

	GUI_LOL_MENU(_deleteMenu, 10, 0x400f, 1, 5, 128, 20, 128, 118);
	GUI_LOL_MENU_ITEM(_deleteMenu.item[0], 0xfffe, 8, 39, 256, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_deleteMenu.item[1], 0xfffd, 8, 56, 256, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_deleteMenu.item[2], 0xfffc, 8, 73, 256, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_deleteMenu.item[3], 0xfffb, 8, 90, 256, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_deleteMenu.item[4], 0x4011, 168, 118, 96, 15, 0, _vm->_keyMap[Common::KEYCODE_ESCAPE]);
	Button::Callback deleteMenuFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::clickedDeleteMenu);
	for (int i = 0; i < 5; ++i)
		_deleteMenu.item[i].callback = deleteMenuFunctor;

	GUI_LOL_MENU(_gameOptions, 17, 0x400c, 0, 6, -1, -1, -1, -1);
	if (_vm->gameFlags().isTalkie) {
		GUI_LOL_MENU_ITEM(_gameOptions.item[0], 0xfff7, 120, 22, 80, 15, 0x406e, 0);
		GUI_LOL_MENU_ITEM(_gameOptions.item[1], 0xfff6, 120, 39, 80, 15, 0x406c, 0);
		GUI_LOL_MENU_ITEM(_gameOptions.item[2], 0xfff5, 120, 56, 80, 15, 0x406d, 0);
		GUI_LOL_MENU_ITEM(_gameOptions.item[3], 0xfff4, 120, 73, 80, 15, 0x42d5, 0);
		GUI_LOL_MENU_ITEM(_gameOptions.item[4], 0xfff3, 120, 90, 80, 15, 0x42d2, 0);
		GUI_LOL_MENU_ITEM(_gameOptions.item[5], 0x4072, 104, 110, 96, 15, 0, _vm->_keyMap[Common::KEYCODE_ESCAPE]);
	} else {
		GUI_LOL_MENU_ITEM(_gameOptions.item[0], 0xfff9, 120, 22, 80, 15, 0x406a, 0);
		GUI_LOL_MENU_ITEM(_gameOptions.item[1], 0xfff8, 120, 39, 80, 15, 0x406b, 0);
		GUI_LOL_MENU_ITEM(_gameOptions.item[2], 0xfff7, 120, 56, 80, 15, 0x406e, 0);
		GUI_LOL_MENU_ITEM(_gameOptions.item[3], 0xfff6, 120, 73, 80, 15, 0x406c, 0);
		GUI_LOL_MENU_ITEM(_gameOptions.item[4], 0xfff5, 120, 90, 80, 15, 0x406d, 0);
		GUI_LOL_MENU_ITEM(_gameOptions.item[5], 0x4072, 104, 110, 96, 15, 0, _vm->_keyMap[Common::KEYCODE_ESCAPE]);
	}
	Button::Callback optionsMenuFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::clickedOptionsMenu);
	for (int i = 0; i < _gameOptions.numberOfItems; ++i)
		_gameOptions.item[i].callback = optionsMenuFunctor;

	GUI_LOL_MENU(_audioOptions, 18, 0x42d9, 2, 1, -1, -1, -1, -1);
	GUI_LOL_MENU_ITEM(_audioOptions.item[0], 0x4072, 152, 76, 96, 15, 0, _vm->_keyMap[Common::KEYCODE_ESCAPE]);
	GUI_LOL_MENU_ITEM(_audioOptions.item[1], 3, 128, 22, 114, 14, 0x42db, 0);
	GUI_LOL_MENU_ITEM(_audioOptions.item[2], 4, 128, 39, 114, 14, 0x42da, 0);
	GUI_LOL_MENU_ITEM(_audioOptions.item[3], 5, 128, 56, 114, 14, 0x42dc, 0);
	Button::Callback audioMenuFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::clickedAudioMenu);
	for (int i = 0; i < 4; ++i)
		_audioOptions.item[i].callback = audioMenuFunctor;

	GUI_LOL_MENU(_deathMenu, 11, 0x4013, 0, 2, -1, -1, -1, -1);
	GUI_LOL_MENU_ITEM(_deathMenu.item[0], 0x4006, 8, 30, 104, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_deathMenu.item[1], 0x4001, 176, 30, 104, 15, 0, 0);
	Button::Callback deathMenuFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::clickedDeathMenu);
	for (int i = 0; i < 2; ++i)
		_deathMenu.item[i].callback = deathMenuFunctor;

	GUI_LOL_MENU(_savenameMenu, 7, 0x4053, 0, 2, -1, -1, -1, -1);
	GUI_LOL_MENU_ITEM(_savenameMenu.item[0], 0x4012, 8, 38, 72, 15, 0, _vm->_keyMap[Common::KEYCODE_RETURN]);
	GUI_LOL_MENU_ITEM(_savenameMenu.item[1], 0x4011, 176, 38, 72, 15, 0, _vm->_keyMap[Common::KEYCODE_ESCAPE]);
	Button::Callback savenameMenuFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::clickedSavenameMenu);
	for (int i = 0; i < 2; ++i)
		_savenameMenu.item[i].callback = savenameMenuFunctor;

	GUI_LOL_MENU(_choiceMenu, 11, 0, 0, 2, -1, -1, -1, -1);
	GUI_LOL_MENU_ITEM(_choiceMenu.item[0], 0x4007, 8, 30, 72, 15, 0, 0);
	GUI_LOL_MENU_ITEM(_choiceMenu.item[1], 0x4008, 208, 30, 72, 15, 0, 0);
	Button::Callback choiceMenuFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::clickedChoiceMenu);
	for (int i = 0; i < 2; ++i)
		_choiceMenu.item[i].callback = choiceMenuFunctor;
}

const ScreenDim Screen_LoL::_screenDimTable256C[] = {
	{ 0x00, 0x00, 0x28, 0xC8, 0xC7, 0xCF, 0x00, 0x00 }, // Taken from Intro
	{ 0x08, 0x48, 0x18, 0x38, 0xFE, 0x01, 0x00, 0x00 },
	{ 0x0E, 0x00, 0x16, 0x78, 0xFE, 0x01, 0x00, 0x00 },
	{ 0x0B, 0x7B, 0x1C, 0x12, 0xFE, 0xFC, 0x00, 0x00 },
	{ 0x0B, 0x7B, 0x1C, 0x2D, 0xFE, 0xFC, 0x00, 0x00 },
	{ 0x55, 0x7B, 0xE9, 0x37, 0xFE, 0xFC, 0x00, 0x00 },
	{ 0x0B, 0x8C, 0x10, 0x2B, 0x3D, 0x01, 0x00, 0x00 }, // Main menu box (4 entries)
	{ 0x04, 0x59, 0x20, 0x3C, 0x00, 0x00, 0x00, 0x00 },
	{ 0x05, 0x6E, 0x1E, 0x0C, 0xFE, 0x01, 0x00, 0x00 },
	{ 0x07, 0x19, 0x1A, 0x97, 0x00, 0x00, 0x00, 0x00 }, // Ingame main menu box CD version
	{ 0x03, 0x1E, 0x22, 0x8C, 0x00, 0x00, 0x00, 0x00 },
	{ 0x02, 0x48, 0x24, 0x34, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x0E, 0x00, 0x16, 0x78, 0xFE, 0x01, 0x00, 0x00 },
	{ 0x0D, 0xA2, 0x18, 0x0C, 0xFE, 0x01, 0x00, 0x00 },
	{ 0x0F, 0x06, 0x14, 0x6E, 0x01, 0x00, 0x00, 0x00 },
	{ 0x1A, 0xBE, 0x0A, 0x07, 0xFE, 0x01, 0x00, 0x00 },
	{ 0x07, 0x21, 0x1A, 0x85, 0x00, 0x00, 0x00, 0x00 },
	{ 0x03, 0x32, 0x22, 0x62, 0x00, 0x00, 0x00, 0x00 },
	{ 0x0B, 0x8C, 0x10, 0x33, 0x3D, 0x01, 0x00, 0x00 }, // Main menu box (5 entries, CD version only)
	{ 0x0B, 0x8C, 0x10, 0x23, 0x3D, 0x01, 0x00, 0x00 }, // Main menu box (3 entries, floppy version only)

	{ 0x01, 0x20, 0x26, 0x80, 0xDC, 0xFD, 0x00, 0x00 }, // Credits
	{ 0x09, 0x29, 0x08, 0x2C, 0x00, 0x00, 0x00, 0x00 },
	{ 0x19, 0x29, 0x08, 0x2C, 0x00, 0x00, 0x00, 0x00 },
	{ 0x01, 0x02, 0x26, 0x14, 0x00, 0x0F, 0x0E, 0x00 },
};

const ScreenDim Screen_LoL::_screenDimTable16C[] = {
	{ 0x00, 0x00, 0x28, 0xC8, 0x33, 0x44, 0x00, 0x00 }, // Taken from Intro
	{ 0x08, 0x48, 0x18, 0x38, 0x33, 0x44, 0x00, 0x00 },
	{ 0x0E, 0x00, 0x16, 0x78, 0x33, 0x44, 0x00, 0x00 },
	{ 0x0B, 0x7B, 0x1C, 0x11, 0x33, 0x11, 0x00, 0x00 },
	{ 0x0B, 0x7B, 0x1C, 0x2D, 0x33, 0x11, 0x00, 0x00 },
	{ 0x55, 0x7B, 0xE9, 0x37, 0x33, 0x11, 0x00, 0x00 },
	{ 0x0B, 0x92, 0x10, 0x2A, 0x33, 0x44, 0x00, 0x00 }, // Main menu box (4 entries)
	{ 0x04, 0x58, 0x20, 0x3C, 0x00, 0x00, 0x00, 0x00 },
	{ 0x05, 0x6C, 0x1E, 0x0D, 0x33, 0x44, 0x00, 0x00 },
	{ 0x07, 0x20, 0x1A, 0x86, 0x00, 0x00, 0x00, 0x00 },
	{ 0x03, 0x20, 0x22, 0x8C, 0x00, 0x00, 0x00, 0x00 },
	{ 0x02, 0x48, 0x24, 0x34, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x0E, 0x00, 0x16, 0x78, 0x33, 0x44, 0x00, 0x00 },
	{ 0x0D, 0xA2, 0x18, 0x0C, 0x33, 0x44, 0x00, 0x00 },
	{ 0x0F, 0x06, 0x14, 0x6E, 0x44, 0x00, 0x00, 0x00 },
	{ 0x1A, 0xBE, 0x0A, 0x07, 0x33, 0x44, 0x00, 0x00 },
	{ 0x07, 0x21, 0x1A, 0x85, 0x00, 0x00, 0x00, 0x00 },
	{ 0x03, 0x32, 0x22, 0x62, 0x00, 0x00, 0x00, 0x00 },
	{ 0x0B, 0x8C, 0x10, 0x33, 0x33, 0x44, 0x00, 0x00 }, // Main menu box (5 entries, not used here)
	{ 0x0B, 0x8C, 0x10, 0x23, 0x33, 0x44, 0x00, 0x00 }, // Main menu box (3 entries)

	{ 0x01, 0x20, 0x26, 0x80, 0xDC, 0xFD, 0x00, 0x00 }, // Credits (TODO: Check this!)
	{ 0x09, 0x29, 0x08, 0x2C, 0x00, 0x00, 0x00, 0x00 },
	{ 0x19, 0x29, 0x08, 0x2C, 0x00, 0x00, 0x00, 0x00 },
	{ 0x01, 0x02, 0x26, 0x14, 0x00, 0x0F, 0x0E, 0x00 },
};

const int Screen_LoL::_screenDimTableCount = ARRAYSIZE(Screen_LoL::_screenDimTable256C);

const char * const LoLEngine::_languageExt[] = {
	"ENG",
	"FRE",
	"GER"
};

const char *const LoLEngine::_charPreviewNamesDefault[] = {
	"Ak\'shel",
	"Michael",
	"Kieran",
	"Conrad"
};

const char *const LoLEngine::_charPreviewNamesRussianFloppy[] = {
	"\x80\xAA\xE8\xA5\xAB\0",
	"\x8C\xA0\xA9\xAA\xAB\0",
	"\x8A\xA8\xE0\xA0\xAD\0",
	"\x8A\xAE\xAD\xE0\xA0\xA4\0"
};

const LoLEngine::CharacterPrev LoLEngine::_charPreviews[] = {
	{ 0x060, 0x7F, { 0x0F, 0x08, 0x05 } },
	{ 0x09A, 0x7F, { 0x06, 0x0A, 0x0F } },
	{ 0x0D4, 0x7F, { 0x08, 0x06, 0x08 } },
	{ 0x10F, 0x7F, { 0x0A, 0x0C, 0x0A } }
};

const uint16 LoLEngine::_charPosXPC98[] = {
	92, 152, 212, 268
};

const uint8 LoLEngine::_charNamesPC98[][11] = {
	{ 0x83, 0x41, 0x83, 0x4E, 0x83, 0x56, 0x83, 0x46, 0x83, 0x8B, 0x00 },
	{ 0x83, 0x7D, 0x83, 0x43, 0x83, 0x50, 0x83, 0x8B, 0x00, 0x00, 0x00 },
	{ 0x83, 0x4C, 0x81, 0x5B, 0x83, 0x89, 0x83, 0x93, 0x00, 0x00, 0x00 },
	{ 0x83, 0x52, 0x83, 0x93, 0x83, 0x89, 0x83, 0x62, 0x83, 0x68, 0x00 }
};

const uint8 LoLEngine::_chargenFrameTableTalkie[] = {
	0x00, 0x01, 0x02, 0x03, 0x04,
	0x05, 0x04, 0x03, 0x02, 0x01,
	0x00, 0x00, 0x01, 0x02, 0x03,
	0x04, 0x05, 0x06, 0x07, 0x08,
	0x09, 0x0A, 0x0B, 0x0C, 0x0D,
	0x0E, 0x0F, 0x10, 0x11, 0x12
};

const uint8 LoLEngine::_chargenFrameTableFloppy[] = {
	0,  1,  2,  3,  4,  5,  4,  3,  2,
	1,  0,  0,  1,  2,  3,  4,  5,  6,
	7,  8,  9, 10, 11, 12, 13, 14, 15
};

const uint16 LoLEngine::_selectionPosTable[] = {
	0x6F, 0x00, 0x8F, 0x00, 0xAF, 0x00,  0xCF, 0x00,
	0xEF, 0x00, 0x6F, 0x20, 0x8F, 0x20,  0xAF, 0x20,
	0xCF, 0x20, 0xEF, 0x20, 0x6F, 0x40,  0x8F, 0x40,
	0xAF, 0x40, 0xCF, 0x40, 0xEF, 0x40, 0x10F, 0x00
};

const uint8 LoLEngine::_selectionChar1IdxTable[] = {
	0, 0, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 0, 0, 5, 5, 5,
	5, 5, 5, 5, 0, 0, 5, 5,
	5, 5, 5
};

const uint8 LoLEngine::_selectionChar2IdxTable[] = {
	1, 1, 6, 6, 1, 1, 6, 6,
	6, 6, 6, 6, 6, 1, 1, 6,
	6, 6, 1, 1, 6, 6, 6, 6,
	6, 6, 6
};

const uint8 LoLEngine::_selectionChar3IdxTable[] = {
	2, 2, 7, 7, 7, 7, 2, 2,
	7, 7, 7, 7, 7, 7, 7, 2,
	2, 7, 7, 7, 7, 2, 2, 7,
	7, 7, 7
};

const uint8 LoLEngine::_selectionChar4IdxTable[] = {
	3, 3, 8, 8, 8, 8, 3, 3,
	8, 8, 3, 3, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 3, 3, 8,
	8, 8, 8
};

const uint8 LoLEngine::_reminderChar1IdxTable[] = {
	4, 4, 4, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	5
};

const uint8 LoLEngine::_reminderChar2IdxTable[] = {
	9, 9, 9, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6
};

const uint8 LoLEngine::_reminderChar3IdxTable[] = {
	0xE, 0xE, 0xE, 0x7, 0x7, 0x7, 0x7, 0x7,
	0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7,
	0x7
};

const uint8 LoLEngine::_reminderChar4IdxTable[] = {
	0xF, 0xF, 0xF, 0x8, 0x8, 0x8, 0x8, 0x8,
	0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
	0x8
};

const uint8 LoLEngine::_selectionAnimIndexTable[] = {
	0, 5, 1, 6, 2, 7, 3, 8
};

const uint8 LoLEngine::_charInfoFrameTable[] = {
	0x0, 0x7, 0x8, 0x9, 0xA, 0xB, 0xA, 0x9,
	0x8, 0x7, 0x0, 0x0, 0x7, 0x8, 0x9, 0xA,
	0xB, 0xA, 0x9, 0x8, 0x7, 0x0, 0x0, 0x7,
	0x8, 0x9, 0xA, 0xB, 0xA, 0x9, 0x8, 0x7
};

const uint8 LoLEngine::_clock2Timers[] = {
	0x00, 0x10, 0x11, 0x03, 0x04, 0x50,
	0x51, 0x52, 0x08, 0x09, 0x0A
};

const uint8 LoLEngine::_numClock2Timers = ARRAYSIZE(LoLEngine::_clock2Timers);

const int8 LoLEngine::_mapCoords[12][4] = {
	{  0,  7,  0, -5 }, { -5,  0,  6,  0 }, {  7,  5,  7,  1 },
	{  5,  6,  4,  6 }, {  0,  7,  0, -1 }, { -3,  0,  6,  0 },
	{  6,  7,  6, -3 }, { -3,  5,  6,  5 }, {  1,  5,  1,  1 },
	{  3,  1,  3,  1 }, { -1,  6, -1, -8 }, { -7, -1,  5, -1 }
};

const MistOfDoomAnimData LoLEngine::_mistAnimData[] = {
	{  0,   7,   7,  13, 155 },
	{  0,  16,  16,  17, 155 },
	{  0,  24,  24,  24, 174 },
	{  0,  19,  19,  19, 174 },
	{  0,  16,  16,  17, 175 },
};

const char * const LoLEngine::_outroShapeFileTable[] = {
	"AMAZON.SHP", "ARCHRSLG.SHP", "AVIANWRM.SHP", "BANDIT.SHP", "BOAR.SHP", "CABAL.SHP",
	"GUARD.SHP", "HAG.SHP", "HORNET.SHP", "HURZELL.SHP", "IRONGRZR.SHP", "KNOWLES.SHP",
	"LIZARD.SHP", "MANTHA.SHP", "MINOTAUR.SHP", "MORIBUND.SHP", "ORC.SHP", "ORCLDR.SHP",
	"PENTROG.SHP", "RATMAN.SHP", "ROCKLING.SHP", "SCAVNGR.SHP", "STARK.SHP",
	"SWAMPCIT.SHP", "SWAMPMON.SHP", "THUG.SHP", "VIPER.SHP", "XEOB.SHP"
};

const uint8 LoLEngine::_outroFrameTable[] = {
	 0,  0,  0,  0,  0,  1,  2,  3,
	 0,  1,  2,  3,  8,  9, 10, 11,
	 8,  9, 10, 11,  4,  5,  6,  7
};

const int16 LoLEngine::_outroRightMonsterPos[] = {
	205,  55, 205,  55, 205,  55, 205,  55,
	205,  56, 207,  57, 208,  58, 210,  59,
	213,  60, 216,  61, 220,  61, 225,  61,
	230,  61, 235,  61, 240,  61, 240,  61,
	240,  61, 240,  61, 240,  61, 240,  61,
	240,  61, 265,  61, 290,  61, 315,  61
};

const int16 LoLEngine::_outroLeftMonsterPos[] = {
	 92,  55,  92,  55,  92,  55,  92,  55,
	 92,  56,  90,  57,  85,  58,  77,  59,
	 67,  60,  57,  61,  47,  61,  35,  61,
	 35,  61,  35,  61,  35,  61,  35,  61,
	 35,  61,  35,  61,  35,  61,  35,  61,
	 35,  61,  10,  61, -20,  61, -45,  61
};

const int16 LoLEngine::_outroRightDoorPos[] = {
	200,  41, 200,  29, 200,  17, 200,   5,
	200,  -7, 200,  -7, 200,  -7, 200,  -7,
	200,   5, 200,  17, 200,  29, 200,  41,
	200,  41, 200,  41, 200,  41, 200,  41,
	200,  41, 200,  41, 200,  41, 200,  41,
	200,  41, 200,  41, 200,  41, 200,  41
};

const int16 LoLEngine::_outroLeftDoorPos[] = {
	 72,  41,  72,  29,  72,  17,  72,   5,
	 72,  -7,  72,  -7,  72,  -7,  72,  -7,
	 72,   5,  72,  17,  72,  29,  72,  41,
	 72,  41,  72,  41,  72,  41,  72,  41,
	 72,  41,  72,  41,  72,  41,  72,  41,
	 72,  41,  72,  41,  72,  41,  72,  41
};

const int LoLEngine::_outroMonsterScaleTableX[] = {
	0x050, 0x050, 0x050, 0x050, 0x050, 0x05D, 0x070, 0x085,
	0x0A0, 0x0C0, 0x0E2, 0x100, 0x100, 0x100, 0x100, 0x100,
	0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100
};

const int LoLEngine::_outroMonsterScaleTableY[] = {
	0x04C, 0x04C, 0x04C, 0x04C, 0x04C, 0x059, 0x06B, 0x080,
	0x099, 0x0B8, 0x0D9, 0x100, 0x100, 0x100, 0x100, 0x100,
	0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100
};

} // End of namespace Kyra

#endif

