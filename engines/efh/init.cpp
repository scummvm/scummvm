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
#include "efh/efh.h"

namespace Efh {
EfhGraphicsStruct::EfhGraphicsStruct() {
	_vgaLineBuffer = nullptr;
	_shiftValue = 0;
	_width = 0;
	_height = 0;
	_area = Common::Rect(0, 0, 0, 0);
}
EfhGraphicsStruct::EfhGraphicsStruct(int8 **lineBuf, int16 x, int16 y, int16 width, int16 height) {
	_vgaLineBuffer = lineBuf;
	_shiftValue = 0;
	_width = width;
	_height = height;
	_area = Common::Rect(x, y, x + width - 1, y + height - 1);
}

void InvObject::init() {
	_ref = 0;
	_stat1 = 0;
	_stat2 = 0;
}

void UnkMapStruct::init() {
	_placeId = _posX = _posY = _field3 = _field4_NpcId = 0;
	_field5_textId = _field7_textId = 0;
}

void UnkAnimStruct::init() {
	memset(_field, 0, 4);
}

void AnimInfo::init() {
	for (int i = 0; i < 15; ++i)
		_unkAnimArray[i].init();

	for (int i = 0; i < 10; ++i) {
		_field3C_startY[i] = 0;
		_field46_startX[i] = 0;
	}
}

void ItemStruct::init() {
	for (uint idx = 0; idx < 15; ++idx)
		_name[idx] = 0;

	_damage = 0;
	_defense = 0;
	_attacks = 0;
	_uses = 0;
	field_13 = 0;
	_range = 0;
	_attackType = 0;
	_specialEffect = 0;
	_field17_attackTypeDefense = 0;
	field_18 = 0;
	_field19_mapPosX_or_maxDeltaPoints = 0;
	_mapPosY = 0;
}

void NPCStruct::init() {
	for (int i = 0; i < 11; ++i)
		_name[i] = 0;
	fieldB_textId = 0;
	field_C = 0;
	field_D = 0;
	fieldE_textId = 0;
	field_F = 0;
	field_10 = 0;
	field11_NpcId = 0;
	field12_textId = 0;
	field14_textId = 0;
	_xp = 0;

	for (int i = 0; i < 15; ++i)
		_activeScore[i] = 0;

	for (int i = 0; i < 11; ++i) {
		_passiveScore[i] = 0;
		_infoScore[i] = 0;
	}

	field_3F = 0;
	field_40 = 0;

	for (int i = 0; i < 10; ++i)
		_inventory[i].init();

	_possessivePronounSHL6 = 0;
	_speed = 0;
	field_6B = 0;
	field_6C = 0;
	field_6D = 0;
	_unkItemId = 0;
	field_6F = 0;
	field_70 = 0;
	field_71 = 0;
	field_72 = 0;
	field_73 = 0;
	_hitPoints = 0;
	_maxHP = 0;
	field_78 = 0;
	field_79 = 0;
	field_7B = 0;
	field_7D = 0;
	field_7E = 0;
	field_7F = 0;
	field_80 = 0;
	field_81 = 0;
	field_82 = 0;
	field_83 = 0;
	field_84 = 0;
	field_85 = 0;
}

uint8 NPCStruct::getPronoun() {
	return _possessivePronounSHL6 >> 6;
}

uint8 MapMonster::getPronoun() {
	return _possessivePronounSHL6 >> 6;
}

void Stru32686::init() {
	for (int i = 0; i < 9; ++i) {
		_effect[i] = 0;
		_duration[i] = 0;
	}
}

void InitiativeStruct::init() {
	_id = _initiative = 0;
}

void TileFactStruct::init() {
	_field0 = _tileId = 0;
}

EfhEngine::EfhEngine(OSystem *syst, const ADGameDescription *gd) : Engine(syst), _gameDescription(gd) {
	const Common::FSNode gameDataDir(ConfMan.get("path"));

	SearchMan.addSubDirectoryMatching(gameDataDir, "gendata");
	SearchMan.addSubDirectoryMatching(gameDataDir, "images");
	SearchMan.addSubDirectoryMatching(gameDataDir, "imp");
	SearchMan.addSubDirectoryMatching(gameDataDir, "maps");

	_system = syst;
	_rnd = nullptr;

	_shouldQuit = false;
	_eventMan = nullptr;
	_lastTime = 0;
	_platform = Common::kPlatformUnknown;
	_mainSurface = nullptr;

	_vgaGraphicsStruct1 = new EfhGraphicsStruct(_vgaLineBuffer, 0, 0, 320, 200);
	_vgaGraphicsStruct2 = new EfhGraphicsStruct();

	_videoMode = 0;
	_graphicsStruct = nullptr;

	for (int i = 0; i < 19; ++i)
		_mapBitmapRefArr[i] = nullptr;

	_defaultBoxColor = 0;

	_fontDescr._widthArray = nullptr;
	_fontDescr._extraLines = nullptr;
	_fontDescr._fontData = nullptr;
	_fontDescr._charHeight = 0;
	_fontDescr._extraHorizontalSpace = _fontDescr._extraVerticalSpace = 0;

	_introDoneFl = false;
	_oldAnimImageSetId = -1;
	_animImageSetId = 0xFE;
	_paletteTransformationConstant = 10;

	for (int i = 0; i < 12; ++i)
		_circleImageSubFileArray[i] = nullptr;

	_imageDataPtr._dataPtr = nullptr;
	_imageDataPtr._width = 0;
	_imageDataPtr._startX = _imageDataPtr._startY = 0;
	_imageDataPtr._height = 0;
	_imageDataPtr._lineDataSize = 0;
	_imageDataPtr._paletteTransformation = 0;
	_imageDataPtr._fieldD = 0;

	for (int i = 0; i < 3; ++i)
		_currentTileBankImageSetId[i] = -1;

	_unkRelatedToAnimImageSetId = 0;
	_techId = 0;
	_currentAnimImageSetId = 0xFF;

	for (int i = 0; i < 20; ++i) {
		_portraitSubFilesArray[i] = nullptr;
	}

	_characterNamePt1 = "";
	_characterNamePt2 = "";
	_enemyNamePt1 = "";
	_enemyNamePt2 = "";
	_nameBuffer = "";
	_attackBuffer = "";

	for (int i = 0; i < 100; ++i) {
		_imp1PtrArray[i] = nullptr;
		_mapUnknown[i].init();
	}

	for (int i = 0; i < 432; ++i)
		_imp2PtrArray[i] = nullptr;

	_unkAnimRelatedIndex = -1;

	_initRect = Common::Rect(0, 0, 0, 0);
	_engineInitPending = true;
	_textColor = 0x0E; // Yellow
	_protectionPassed = false;
	_fullPlaceId = 0xFF;
	_guessAnimationAmount = 9;
	_largeMapFlag = 0xFFFF;
	_unk2C8AA = 0;
	_teamCharId[0] = 0;
	_teamCharId[1] = _teamCharId[2] = -1;

	for (int i = 0; i < 3; ++i) {
		_teamCharStatus[i]._status = 0;
		_teamCharStatus[i]._duration = 0;
		_teamPctVisible[i] = 0;
		_word32482[i] = 0;
		_teamNextAttack[i] = -1;
		_word31780[i] = 0;
		_teamLastAction[i] = 0;
	}

	for (int i = 0; i < 5; ++i) {
		_teamMonsterIdArray[i] = -1;
		_teamMonsterEffects[i].init();
	}

	_teamSize = 1;
	_word2C872 = 0;
	_imageSetSubFilesIdx = 144;
	_oldImageSetSubFilesIdx = 143;

	_mapPosX = _mapPosY = 31;
	_oldMapPosX = _oldMapPosY = 31;
	_techDataId_MapPosX = _techDataId_MapPosY = 31;

	_textPosX = 0;
	_textPosY = 0;

	_lastMainPlaceId = 0;
	_tempTextDelay = 0;
	_tempTextPtr = nullptr;
	_word2C880 = false;
	_redrawNeededFl = false;
	_drawHeroOnMapFl = true;
	_drawMonstersOnMapFl = true;
	_word2C87A = false;
	_dbgForceMonsterBlock = false;
	_ongoingFightFl = false;
	_statusMenuActive = false;
	_menuDepth = 0;
	_menuItemCounter = 0;

	for (int i = 0; i < 15; ++i) {
		_menuStatItemArr[i] = 0;
	}

	_messageToBePrinted = "";
	for (int i = 0; i < 8; ++i)
		_initiatives[i].init();

	memset(_bufferCharBM, 0, ARRAYSIZE(_bufferCharBM));
	for (int i = 0; i < 3; ++i)
		memset(_tileBank[i], 0, ARRAYSIZE(_tileBank[i]));
	memset(_circleImageBuf, 0, ARRAYSIZE(_circleImageBuf));
	memset(_portraitBuf, 0, ARRAYSIZE(_portraitBuf));
	memset(_hiResImageBuf, 0, ARRAYSIZE(_hiResImageBuf));
	memset(_loResImageBuf, 0, ARRAYSIZE(_loResImageBuf));
	memset(_menuBuf, 0, ARRAYSIZE(_menuBuf));
	memset(_windowWithBorderBuf, 0, ARRAYSIZE(_windowWithBorderBuf));
	memset(_places, 0, ARRAYSIZE(_places));
	for (int i = 0; i < 24; ++i)
		memset(_curPlace[i], 0, ARRAYSIZE(_curPlace[i]));
	memset(_npcBuf, 0, ARRAYSIZE(_npcBuf));
	memset(_imp1, 0, ARRAYSIZE(_imp1));
	memset(_imp2, 0, ARRAYSIZE(_imp2));
	memset(_titleSong, 0, ARRAYSIZE(_titleSong));
	memset(_items, 0, ARRAYSIZE(_items));
	memset(_tileFact, 0, ARRAYSIZE(_tileFact));
	memset(_animInfo, 0, ARRAYSIZE(_animInfo));
	memset(_history, 0, ARRAYSIZE(_history));
	for (int i = 0; i < 19; ++i) {
		memset(_techDataArr[i], 0, ARRAYSIZE(_techDataArr[i]));
		memset(_mapArr[i], 0, ARRAYSIZE(_mapArr[i]));
	}
	memset(_mapMonsters, 0, ARRAYSIZE(_mapMonsters));
	memset(_mapGameMap, 0, ARRAYSIZE(_mapGameMap));
	memset(_imageSetSubFilesArray, 0, ARRAYSIZE(_imageSetSubFilesArray));
	_regenCounter = 0;

	// If requested, load a savegame instead of showing the intro
	_loadSaveSlot = -1;
	_saveAuthorized = false;

	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 999)
			_loadSaveSlot = saveSlot;
	}
}

} // End of namespace Efh

