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

#include "kyra/engine/darkmoon.h"
#include "kyra/resource/resource.h"
#include "kyra/sound/sound.h"

namespace Kyra {

DarkMoonEngine::DarkMoonEngine(OSystem *system, const GameFlags &flags) : EoBCoreEngine(system, flags) {
	_dscDoorType5Offs = 0;
	_numSpells = 70;
	_menuChoiceInit = 4;

	_kheldranStrings = _npcStrings[0] = _npcStrings[1] = _hornStrings = 0;
	_utilMenuStrings = _ascii2SjisTables = _ascii2SjisTables2 = 0;
	_npcShpData = _dscDoorType5Offs = _hornSounds = 0;
	_dreamSteps = 0;

	_amigaSoundMapExtra = _amigaSoundFiles2 = 0;
	_amigaSoundIndex1 = 0;
	_amigaSoundIndex2 = 0;
	_amigaCurSoundIndex = 0;
	_amigaSoundPatch = 0;
	_amigaSoundPatchSize = 0;
}

DarkMoonEngine::~DarkMoonEngine() {
}

Common::Error DarkMoonEngine::init() {
	Common::Error err = EoBCoreEngine::init();
	if (err.getCode() != Common::kNoError)
		return err;

	initStaticResource();

	_monsterProps = new EoBMonsterProperty[10];

	if (_configRenderMode == Common::kRenderEGA) {
		Palette pal(16);
		_screen->loadPalette(_egaDefaultPalette, pal, 16);
		_screen->setScreenPalette(pal);
	}

	_screen->loadPalette(_flags.platform == Common::kPlatformFMTowns ? "MENU.PAL" : "PALETTE.COL", _screen->getPalette(0));
	_screen->setScreenPalette(_screen->getPalette(0));

	if (_flags.platform == Common::kPlatformFMTowns) {
		// adjust menu settings for EOB II FM-Towns
		_screen->modifyScreenDim(6, 10, 100, 21, 40);
		_screen->modifyScreenDim(27, 0, 0, 21, 2);
		_vcnFilePattern = "%s.VCC";
	}

	return Common::kNoError;
}


void DarkMoonEngine::loadItemsAndDecorationsShapes() {
	if (_flags.platform != Common::kPlatformFMTowns) {
		EoBCoreEngine::loadItemsAndDecorationsShapes();
		return;
	}

	releaseItemsAndDecorationsShapes();
	int size = 0;

	_largeItemShapes = new const uint8 *[_numLargeItemShapes];
	for (int i = 0; i < _numLargeItemShapes; i++)
		_largeItemShapes[i] = _staticres->loadRawData(kEoB2LargeItemsShapeData00 + i, size);
	_smallItemShapes = new const uint8 *[_numSmallItemShapes];
	for (int i = 0; i < _numSmallItemShapes; i++)
		_smallItemShapes[i] = _staticres->loadRawData(kEoB2SmallItemsShapeData00 + i, size);
	_thrownItemShapes = new const uint8 *[_numThrownItemShapes];
	for (int i = 0; i < _numThrownItemShapes; i++)
		_thrownItemShapes[i] = _staticres->loadRawData(kEoB2ThrownShapeData00 + i, size);
	_spellShapes = new const uint8 *[4];
	for (int i = 0; i < 4; i++)
		_spellShapes[i] = _staticres->loadRawData(kEoB2SpellShapeData00 + i, size);
	_firebeamShapes = new const uint8 *[3];
	for (int i = 0; i < 3; i++)
		_firebeamShapes[i] = _staticres->loadRawData(kEoB2FirebeamShapeData00 + i, size);
	_redSplatShape = _staticres->loadRawData(kEoB2RedSplatShapeData, size);
	_greenSplatShape = _staticres->loadRawData(kEoB2GreenSplatShapeData, size);
	_itemIconShapes = new const uint8 *[_numItemIconShapes];
	for (int i = 0; i < _numItemIconShapes; i++)
		_itemIconShapes[i] = _staticres->loadRawData(kEoB2ItemIconShapeData00 + i, size);

	_teleporterShapes = new const uint8 *[6];
	_sparkShapes = new const uint8 *[3];
	_compassShapes = new const uint8 *[12];
	if (_flags.gameID == GI_EOB2)
		_wallOfForceShapes = new const uint8 *[6];

	_lightningColumnShape = _staticres->loadRawData(kEoB2LightningColumnShapeData, size);
	for (int i = 0; i < 6; i++)
		_wallOfForceShapes[i] = _staticres->loadRawData(kEoB2WallOfForceShapeData00 + i, size);
	for (int i = 0; i < 6; i++)
		_teleporterShapes[i] = _staticres->loadRawData(kEoB2TeleporterShapeData00 + i, size);
	for (int i = 0; i < 3; i++)
		_sparkShapes[i] = _staticres->loadRawData(kEoB2SparkShapeData00 + i, size);
	for (int i = 0; i < 12; i++)
		_compassShapes[i] = _staticres->loadRawData(kEoB2CompassShapeData00 + i, size);

	_deadCharShape = _staticres->loadRawData(kEoB2DeadCharShapeData, size);
	_disabledCharGrid = _staticres->loadRawData(kEoB2DisabledCharGridShapeData, size);
	_blackBoxSmallGrid = _staticres->loadRawData(kEoB2SmallGridShapeData, size);
	_weaponSlotGrid = _staticres->loadRawData(kEoB2WeaponSlotGridShapeData, size);
	_blackBoxWideGrid = _staticres->loadRawData(kEoB2WideGridShapeData, size);
}

void DarkMoonEngine::startupNew() {
	_sound->selectAudioResourceSet(kMusicIngame);
	_currentLevel = 4;
	_currentSub = 0;
	loadLevel(4, 0);
	_currentBlock = 171;
	_currentDirection = 2;
	setHandItem(0);

	EoBCoreEngine::startupNew();
}

void DarkMoonEngine::startupLoad() {
	_sound->selectAudioResourceSet(kMusicIngame);
}

void DarkMoonEngine::drawNpcScene(int npcIndex) {
	const uint8 *shpDef = &_npcShpData[npcIndex << 3];
	for (int i = npcIndex; i != 255; i = shpDef[7]) {
		shpDef = &_npcShpData[i << 3];
		_screen->_curPage = 2;
		const uint8 *shp = _screen->encodeShape(READ_LE_UINT16(shpDef), shpDef[2], shpDef[3], shpDef[4]);
		_screen->_curPage = 0;
		_screen->drawShape(0, shp, 88 + shpDef[5] - (shp[2] << 2), 104 + shpDef[6] - shp[1], 5);
		delete[] shp;
	}
}

void DarkMoonEngine::runNpcDialogue(int npcIndex) {
	if (npcIndex == 0) {
		snd_playSoundEffect(57);
		if (npcJoinDialogue(0, 1, 3, 2))
			setScriptFlags(0x40);
	} else if (npcIndex == 1) {
		snd_playSoundEffect(53);
		gui_drawDialogueBox();

		_txt->printDialogueText(4, 0);
		int r = runDialogue(-1, 2, -1, _npcStrings[0][0], _npcStrings[0][1]) - 1;

		if (r == 0) {
			snd_stopSound();
			delay(3 * _tickLength);
			snd_playSoundEffect(91);
			npcJoinDialogue(1, 5, 6, 7);
		} else if (r == 1) {
			setScriptFlags(0x20);
		}

	} else if (npcIndex == 2) {
		snd_playSoundEffect(55);
		gui_drawDialogueBox();

		_txt->printDialogueText(8, 0);
		int r = runDialogue(-1, 2, -1, _npcStrings[1][0], _npcStrings[1][1]) - 1;

		if (r == 0) {
			if (rollDice(1, 2, -1))
				_txt->printDialogueText(9, _okStrings[0]);
			else
				npcJoinDialogue(2, 102, 103, 104);
			setScriptFlags(8);
		} else if (r == 1) {
			_currentDirection = 0;
		}
	}
}

void DarkMoonEngine::updateUsedCharacterHandItem(int charIndex, int slot) {
	EoBItem *itm = &_items[_characters[charIndex].inventory[slot]];
	if (itm->type == 48 || itm->type == 62) {
		if (itm->value == 5)
			return;
		int charges = itm->flags & 0x3F;
		if (--charges)
			--itm->flags;
		else
			deleteInventoryItem(charIndex, slot);
	} else if (itm->type == 26 || itm->type == 34 || itm->type == 35) {
		deleteInventoryItem(charIndex, slot);
	}
}

void DarkMoonEngine::loadMonsterShapes(const char *filename, int monsterIndex, bool hasDecorations, int encodeTableIndex) {
	if (_flags.platform != Common::kPlatformFMTowns) {
		EoBCoreEngine::loadMonsterShapes(filename, monsterIndex, hasDecorations, encodeTableIndex);
		return;
	}

	Common::String tmp = Common::String::format("%s.MNT", filename);
	Common::SeekableReadStream *s = _res->createReadStream(tmp);
	if (!s)
		error("Screen_EoB::loadMonsterShapes(): Failed to load file '%s'", tmp.c_str());

	for (int i = 0; i < 6; i++)
		_monsterShapes[monsterIndex + i] = loadFMTownsShape(s);

	for (int i = 0; i < 6; i++) {
		for (int ii = 0; ii < 2; ii++)
			s->read(_monsterPalettes[(monsterIndex >= 18 ? i + 6 : i) * 2 + ii], 16);
	}

	if (hasDecorations)
		loadMonsterDecoration(s, monsterIndex);

	delete s;
}

uint8 *DarkMoonEngine::loadFMTownsShape(Common::SeekableReadStream *stream) {
	uint32 size = stream->readUint32LE();
	uint8 *shape = new uint8[size];
	stream->read(shape, size);
	if (shape[0] == 1)
		shape[0]++;
	return shape;
}

void DarkMoonEngine::generateMonsterPalettes(const char *file, int16 monsterIndex) {
	if (_flags.platform == Common::kPlatformAmiga)
		return;

	int cp = _screen->setCurPage(2);
	_screen->loadShapeSetBitmap(file, 3, 3);
	uint8 tmpPal[16];
	uint8 newPal[16];

	for (int i = 0; i < 6; i++) {
		int dci = monsterIndex + i;
		memcpy(tmpPal, _monsterShapes[dci] + 4, 16);
		int colx = 302 + 3 * i;

		for (int ii = 0; ii < 16; ii++) {
			uint8 col = _screen->getPagePixel(_screen->_curPage, colx, 184 + ii);
			int iii = 0;
			for (; iii < 16; iii++) {
				if (tmpPal[iii] == col) {
					newPal[ii] = iii;
					break;
				}
			}

			if (iii == 16)
				newPal[ii] = 0;
		}

		for (int ii = 1; ii < 3; ii++) {
			memcpy(tmpPal, _monsterShapes[dci] + 4, 16);

			for (int iii = 0; iii < 16; iii++) {
				uint8 col = _screen->getPagePixel(_screen->_curPage, colx + ii, 184 + iii);
				if (newPal[iii])
					tmpPal[newPal[iii]] = col;
			}

			int c = i;
			if (monsterIndex >= 18)
				c += 6;

			c = (c << 1) + (ii - 1);
			assert(c < 24);
			memcpy(_monsterPalettes[c], tmpPal, 16);
		}
	}

	_screen->setCurPage(cp);
}

void DarkMoonEngine::loadMonsterDecoration(Common::SeekableReadStream *stream, int16 monsterIndex) {
	int len = stream->readUint16LE();
	Common::List<SpriteDecoration*> activeDecorations;

	for (int i = 0; i < len; i++) {
		for (int ii = 0; ii < 6; ii++) {
			uint8 dc[6];
			stream->read(dc, 6);
			if (!dc[2] || !dc[3])
				continue;

			SpriteDecoration *m = &_monsterDecorations[i * 6 + ii + monsterIndex];
			if (_flags.platform != Common::kPlatformFMTowns)
				m->shp = _screen->encodeShape(dc[0], dc[1], dc[2], dc[3]);
			m->x = (int8)dc[4];
			m->y = (int8)dc[5];
			activeDecorations.push_back(m);
		}
	}

	if (_flags.platform == Common::kPlatformFMTowns) {
		while (!activeDecorations.empty()) {
			activeDecorations.front()->shp = loadFMTownsShape(stream);
			activeDecorations.pop_front();
		}
	}
}

const uint8 *DarkMoonEngine::loadMonsterProperties(const uint8 *data) {
	uint8 cmd = *data++;
	while (cmd != 0xFF) {
		EoBMonsterProperty *d = &_monsterProps[cmd];
		d->armorClass = (int8)*data++;
		d->hitChance = (int8)*data++;
		d->level = (int8)*data++;
		d->hpDcTimes = *data++;
		d->hpDcPips = *data++;
		d->hpDcBase = *data++;
		d->attacksPerRound = *data++;
		d->dmgDc[0].times = *data++;
		d->dmgDc[0].pips = *data++;
		d->dmgDc[0].base = (int8)*data++;
		d->dmgDc[1].times = *data++;
		d->dmgDc[1].pips = *data++;
		d->dmgDc[1].base = (int8)*data++;
		d->dmgDc[2].times = *data++;
		d->dmgDc[2].pips = *data++;
		d->dmgDc[2].base = (int8)*data++;
		d->immunityFlags = READ_LE_UINT16(data);
		data += 2;
		d->capsFlags = READ_LE_UINT16(data);
		data += 2;
		d->typeFlags = READ_LE_UINT16(data);
		data += 2;
		d->experience = READ_LE_UINT16(data);
		data += 2;

		d->u30 = *data++;
		d->sound1 = (int8)*data++;
		d->sound2 = (int8)*data++;

		// I have confirmed with WinUAE that the monster sounds in EOB II Amiga German are broken. Some
		// monsters do at least have walking sounds. Attack sounds seem to be completely dysfunctional
		// for all monsters. Maybe the devs who did the localization used the DOS resources without doing
		// the necessary modifications. A quick debug run and comparison leaves the impression that the
		// German Amiga version indeed has the track numbers from the DOS version.
		// WORKAROUND: I've made an autogenerated sound patch file from the English version for all levels,
		// sub levels and monster types. It became clear from the patch file that 95% of all sound entries
		// are sound1 = 38 and sound2 = 36. So I eliminated these entries from the patch file and set
		// the 38/36 combo as the default. This remaining patch file is rather small...
		if (_flags.platform == Common::kPlatformAmiga && _flags.lang == Common::DE_DEU) {
			d->sound1 = 38;
			d->sound2 = 36;
			uint8 id = (_currentLevel - 1) | (_currentSub << 4) | (cmd << 5);
			for (int i = 0; i < _amigaSoundPatchSize; i += 3) {
				if (_amigaSoundPatch[i] == id) {
					d->sound1 = _amigaSoundPatch[i + 1];
					d->sound2 = _amigaSoundPatch[i + 2];
					break;
				}
			}
		}

		d->numRemoteAttacks = *data++;

		if (*data++ != 0xFF) {
			d->remoteWeaponChangeMode = *data++;
			d->numRemoteWeapons = *data++;

			for (int i = 0; i < d->numRemoteWeapons; i++) {
				d->remoteWeapons[i] = (int8)*data;
				data += 2;
			}
		}

		d->tuResist = (int8)*data++;
		d->dmgModifierEvade = *data++;

		for (int i = 0; i < 3; i++)
			d->decorations[i] = *data++;

		cmd = *data++;
	}

	return data;
}

void DarkMoonEngine::replaceMonster(int unit, uint16 block, int pos, int dir, int type, int shpIndex, int mode, int h2, int randItem, int fixedItem) {
	uint8 flg = _levelBlockProperties[block].flags & 7;

	if (flg == 7 || _currentBlock == block || (flg && (_monsterProps[type].u30 || pos == 4)))
		return;

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].block != block)
			continue;
		if (_monsters[i].pos == 4 || _monsterProps[_monsters[i].type].u30)
			return;
	}

	int index = -1;
	int maxDist = 0;

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].hitPointsCur <= 0) {
			index = i;
			break;
		}

		if (_monsters[i].flags & 0x40)
			continue;

		// WORKAROUND for bug #3611077 (Dran's dragon transformation sequence triggered prematurely):
		// The boss level and the mindflayer level share the same monster data. If you hang around
		// long enough in the mindflayer level all 30 monster slots will be used up. When this
		// happens it will trigger the dragon transformation sequence when Dran is moved around by script.
		// We avoid removing Dran here by prefering monster slots occupied by monsters from another
		// sub level.
		if (_monsters[i].sub != _currentSub) {
			index = i;
			break;
		}

		int dist = getBlockDistance(_monsters[i].block, _currentBlock);

		if (dist > maxDist) {
			maxDist = dist;
			index = i;
		}
	}

	if (index == -1)
		return;

	if (_monsters[index].hitPointsCur > 0)
		killMonster(&_monsters[index], false);

	initMonster(index, unit, block, pos, dir, type, shpIndex, mode, h2, randItem, fixedItem);
}

bool DarkMoonEngine::killMonsterExtra(EoBMonsterInPlay *m) {
	// WORKAROUND for bug #3611077 (see DarkMoonEngine::replaceMonster())
	// The mindflayers have monster type 0, just like Dran. Using a monster slot occupied by a mindflayer would trigger the dragon transformation
	// sequence when all 30 monster slots are used up. We avoid this by checking for m->sub == 1.
	if (_currentLevel == 16 && _currentSub == 1 && m->sub == 1 && (_monsterProps[m->type].capsFlags & 4)) {
		if (m->type) {
			_playFinale = true;
			_runFlag = false;
			delay(850);
		} else {
			m->hitPointsCur = 150;
			m->curRemoteWeapon = 0;
			m->numRemoteAttacks = 255;
			m->shpIndex++;
			m->type++;
			seq_dranDragonTransformation();
		}
		return false;
	}
	return true;
}

void DarkMoonEngine::loadVcnData(const char *file, const uint8 *cgaMapping) {
	if (file)
		strcpy(_lastBlockDataFile, file);
	delete[] _vcnBlocks;

	if (_flags.platform == Common::kPlatformFMTowns) {
		Common::String fn = Common::String::format(_vcnFilePattern.c_str(), _lastBlockDataFile);
		_vcnBlocks = _res->fileData(fn.c_str(), 0);
	} else {
		EoBCoreEngine::loadVcnData(file, cgaMapping);
	}
}

const uint8 *DarkMoonEngine::loadDoorShapes(const char *filename, int doorIndex, const uint8 *shapeDefs) {
	_screen->loadShapeSetBitmap(filename, 3, 3);
	for (int i = 0; i < 3; i++) {
		_doorShapes[doorIndex * 3 + i] = _screen->encodeShape(READ_LE_UINT16(shapeDefs), READ_LE_UINT16(shapeDefs + 2), READ_LE_UINT16(shapeDefs + 4), READ_LE_UINT16(shapeDefs + 6));
		shapeDefs += 8;
	}

	for (int i = 0; i < 2; i++) {
		_doorSwitches[doorIndex * 3 + i].shp = _screen->encodeShape(READ_LE_UINT16(shapeDefs), READ_LE_UINT16(shapeDefs + 2), READ_LE_UINT16(shapeDefs + 4), READ_LE_UINT16(shapeDefs + 6));
		shapeDefs += 8;
		_doorSwitches[doorIndex * 3 + i].x = *shapeDefs;
		shapeDefs += 2;
		_doorSwitches[doorIndex * 3 + i].y = *shapeDefs;
		shapeDefs += 2;
	}
	_screen->_curPage = 0;
	return shapeDefs;
}

void DarkMoonEngine::drawDoorIntern(int type, int, int x, int y, int w, int wall, int mDim, int16, int16) {
	int shapeIndex = type * 3 + 2 - mDim;
	uint8 *shp = _doorShapes[shapeIndex];
	if (!shp)
		return;

	if ((_doorType[type] == 0) || (_doorType[type] == 1)) {
		y = _dscDoorY1[mDim] - shp[1];
		x -= (shp[2] << 2);

		if (_doorType[type] == 1) {
			drawBlockObject(0, 2, shp, x, y, 5);
			shp = _doorShapes[3 + shapeIndex];
		}

		y -= ((wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult1[mDim]);

		if (_specialWallTypes[wall] == 5)
			y -= _dscDoorType5Offs[shapeIndex];

	} else if (_doorType[type] == 2) {
		x -= (shp[2] << 2);
		y = _dscDoorY2[mDim] - ((wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult3[mDim]);
	}

	drawBlockObject(0, 2, shp, x, y, 5);

	if (_doorType[type] == 2) {
		shp = _doorShapes[shapeIndex + 3];
		y = _dscDoorFrameY2[mDim] - shp[1] + (((wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult3[mDim]) >> 1) - 1;
		drawBlockObject(0, 2, shp, x, y, 5);
	}

	if (_wllShapeMap[wall] == -1 && !_noDoorSwitch[type])
		drawBlockObject(0, 2, _doorSwitches[shapeIndex].shp, _doorSwitches[shapeIndex].x + w, _doorSwitches[shapeIndex].y, 5);
}

void DarkMoonEngine::turnUndeadAutoHit() {
	snd_playSoundEffect(95);
}

void DarkMoonEngine::restParty_npc() {
	int insalId = -1;
	int numChar = 0;

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 1))
			continue;
		if (testCharacter(i, 2) && _characters[i].portrait == -1)
			insalId = i;
		numChar++;
	}

	if (insalId == -1 || numChar < 5)
		return;

	removeCharacterFromParty(insalId);
	if (insalId < 4)
		exchangeCharacters(insalId, testCharacter(5, 1) ? 5 : 4);

	clearScriptFlags(6);

	if (!stripPartyItems(1, 1, 1, 1))
		stripPartyItems(2, 1, 1, 1);
	stripPartyItems(31, 0, 1, 3);
	stripPartyItems(39, 1, 0, 3);
	stripPartyItems(47, 0, 1, 2);

	_items[createItemOnCurrentBlock(28)].value = 26;

	gui_drawPlayField(false);
	gui_drawAllCharPortraitsWithStats();

	_screen->setClearScreenDim(10);
	_screen->set16bitShadingLevel(4);
	gui_drawBox(_screen->_curDim->sx << 3, _screen->_curDim->sy, _screen->_curDim->w << 3, _screen->_curDim->h, guiSettings()->colors.frame1, guiSettings()->colors.frame2, -1);
	gui_drawBox((_screen->_curDim->sx << 3) + 1, _screen->_curDim->sy + 1, (_screen->_curDim->w << 3) - 2, _screen->_curDim->h - 2, guiSettings()->colors.frame1, guiSettings()->colors.frame2, guiSettings()->colors.fill);
	_screen->set16bitShadingLevel(0);
	_gui->messageDialogue2(11, 63, guiSettings()->colors.guiColorLightRed);
	_gui->messageDialogue2(11, 64, guiSettings()->colors.guiColorLightRed);
}

bool DarkMoonEngine::restParty_extraAbortCondition() {
	if (_currentLevel != 3)
		return false;

	seq_nightmare();

	return true;
}

void DarkMoonEngine::snd_loadAmigaSounds(int level, int sub) {
	if (_flags.platform != Common::kPlatformAmiga)
		return;

	int fileNum = _amigaSoundIndex2[level];
	if (fileNum != _amigaCurSoundFile) {
		for (int i = 52; i < 68; ++i) {
			if (_amigaSoundMap[i]) {
				_sound->unloadSoundFile(_amigaSoundMap[i]);
				_amigaSoundMap[i] = 0;
			}
		}

		_sound->loadSoundFile(_amigaSoundFiles2[fileNum]);
		_amigaCurSoundFile = fileNum;

		int mapCnt = 0;
		for (int i = 0; i < fileNum + 1; ) {
			if (!_amigaSoundMapExtra[mapCnt++][0])
				i++;
		}

		for (int i = 52; i < 68; ++i) {
			if (!_amigaSoundMapExtra[mapCnt][0]) {
				_amigaSoundMap[i] = 0;
				break;
			}
			_amigaSoundMap[i] = _amigaSoundMapExtra[mapCnt++];
		}
	}

	if (level == 10 || (level == 8 && sub))
		return;

	uint16 sndIndex = 0;

	for (int i = 0; i != level; ) {
		int8 val = _amigaSoundIndex1[sndIndex++];
		if (val == -1)
			i++;
	}

	if (sub)
		sndIndex += 4;

	if (_amigaCurSoundIndex) {
		for (int i = 0; i < 4; ++i) {
			int8 valCur = _amigaSoundIndex1[_amigaCurSoundIndex + i];
			int8 valNew = _amigaSoundIndex1[sndIndex + i];
			if (valCur < 0)
				continue;

			if (i < 2) {
				for (int ii = 1; ii < 5; ++ii)
					_sound->unloadSoundFile(Common::String::format("%s%d", _amigaLevelSoundList2[valCur], ii));
			} else {
				if (valCur != valNew)
					_sound->unloadSoundFile(Common::String::format("%s.SAM", _amigaLevelSoundList1[valCur]));
				_sound->unloadSoundFile(Common::String::format("%s1", _amigaLevelSoundList2[valCur]));
			}
		}
	}

	for (int i = 0; i < 4; ++i) {
		int8 valCur = _amigaCurSoundIndex ? _amigaSoundIndex1[_amigaCurSoundIndex + i] : -5;
		int8 valNew = _amigaSoundIndex1[sndIndex + i];

		if (valNew >= 0 && valNew != valCur) {
			if (i < 2 && valCur >= 0 && _amigaCurSoundIndex)
				_sound->unloadSoundFile(Common::String::format("%s.SAM", _amigaLevelSoundList1[_amigaSoundIndex1[_amigaCurSoundIndex]]));
			_sound->loadSoundFile(Common::String::format("%s.CPS", _amigaLevelSoundList1[valNew]));
			assert(_amigaLevelSoundList2[valNew]);
			_amigaSoundMap[36 + i] = _amigaLevelSoundList2[valNew][0] ? _amigaLevelSoundList2[valNew] : 0;
		} else if (valNew == -2) {
			_amigaSoundMap[36 + i] = 0;
		} else if (valNew == -3) {
			_amigaSoundMap[36 + i] = _amigaSoundMap[35 + i];
		}
	}

	_sound->loadSoundFile(Common::String::format(sub ? "LEVEL%da.SAM" : "LEVEL%d.SAM", level));

	_amigaCurSoundIndex = sndIndex;
}

void DarkMoonEngine::useHorn(int charIndex, int weaponSlot) {
	int v = _items[_characters[charIndex].inventory[weaponSlot]].value - 1;
	_txt->printMessage(_hornStrings[v]);
	snd_playSoundEffect(_hornSounds[v]);
}

bool DarkMoonEngine::checkPartyStatusExtra() {
	if (checkScriptFlags(0x100000))
		seq_kheldran();
	return _gui->confirmDialogue2(14, 67, 1);
}

void DarkMoonEngine::drawLightningColumn() {
	int f = rollDice(1, 2, -1);
	int y = 0;

	for (int i = 0; i < 6; i++) {
		f ^= 1;
		drawBlockObject(f, 2, _lightningColumnShape, 72, y, 5);
		y += 64;
	}
}

int DarkMoonEngine::resurrectionSelectDialogue() {
	countResurrectionCandidates();

	_rrNames[_rrCount] = _abortStrings[0];
	_rrId[_rrCount++] = 99;

	int r = _rrId[runDialogue(-1, 9, -1, _rrNames[0], _rrNames[1], _rrNames[2], _rrNames[3], _rrNames[4], _rrNames[5], _rrNames[6], _rrNames[7], _rrNames[8]) - 1];
	if (r == 99)
		return 0;

	if (r < 0) {
		r = -r;
		if (prepareForNewPartyMember(33, r))
			initNpc(r - 1);
	} else {
		_characters[r].hitPointsCur = 1;
	}

	return 1;
}

int DarkMoonEngine::charSelectDialogue() {
	int cnt = 0;
	const char *namesList[7];
	memset(namesList, 0, 7 * sizeof(const char *));

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 3))
			continue;
		namesList[cnt++] = _characters[i].name;
	}

	namesList[cnt++] = _abortStrings[0];

	int r = runDialogue(-1, 7, -1, namesList[0], namesList[1], namesList[2], namesList[3], namesList[4], namesList[5], namesList[6]) - 1;
	if (r == cnt - 1)
		return 99;

	for (cnt = 0; cnt < 6; cnt++) {
		if (!testCharacter(cnt, 3))
			continue;
		if (--r < 0)
			break;
	}
	return cnt;
}

void DarkMoonEngine::characterLevelGain(int charIndex) {
	EoBCharacter *c = &_characters[charIndex];
	int s = _numLevelsPerClass[c->cClass];
	for (int i = 0; i < s; i++) {
		uint32 er = getRequiredExperience(c->cClass, i, c->level[i] + 1);
		if (er == 0xFFFFFFFF)
			continue;

		increaseCharacterExperience(charIndex, er - c->experience[i] + 1);
	}
}

const KyraRpgGUISettings *DarkMoonEngine::guiSettings() const {
	if (_flags.platform == Common::kPlatformAmiga)
		return &_guiSettingsAmiga;
	else if (_flags.platform == Common::kPlatformFMTowns)
		return &_guiSettingsFMTowns;
	else
		return &_guiSettingsDOS;
}

} // End of namespace Kyra

#endif // ENABLE_EOB
