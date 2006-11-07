/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/savefile.h"

#include "touche/graphics.h"
#include "touche/touche.h"

namespace Touche {

enum {
	kCurrentGameStateVersion = 5,
	kGameStateDescriptionLen = 32
};

template <class S, class T>
static void saveOrLoad(S &s, T &t);

template <>
static void saveOrLoad(Common::WriteStream &stream, uint16 &i) {
	stream.writeUint16LE(i);
}

template <>
static void saveOrLoad(Common::ReadStream &stream, uint16 &i) {
	i = stream.readUint16LE();
}

template <>
static void saveOrLoad(Common::WriteStream &stream, int16 &i) {
	stream.writeSint16LE(i);
}

template <>
static void saveOrLoad(Common::ReadStream &stream, int16 &i) {
	i = stream.readSint16LE();
}

template <class S, class T>
static void saveOrLoadPtr(S &s, T *&t, T *base);

template <>
static void saveOrLoadPtr(Common::WriteStream &stream, int16 *&p, int16 *base) {
	int32 offset = (int32)(p - base);
	stream.writeSint32LE(offset);
}

template <>
static void saveOrLoadPtr(Common::ReadStream &stream, int16 *&p, int16 *base) {
	int32 offset = stream.readSint32LE();
	p = base + offset;
}

template <class S>
static void saveOrLoad(S &s, Common::Rect &r) {
	saveOrLoad(s, r.left);
	saveOrLoad(s, r.top);
	saveOrLoad(s, r.right);
	saveOrLoad(s, r.bottom);
}

template <class S>
static void saveOrLoad(S &s, SequenceEntry &seq) {
	saveOrLoad(s, seq.sprNum);
	saveOrLoad(s, seq.seqNum);
}

template <class S>
static void saveOrLoad(S &s, KeyChar &key) {
	saveOrLoad(s, key.num);
	saveOrLoad(s, key.flags);
	saveOrLoad(s, key.currentAnimCounter);
	saveOrLoad(s, key.strNum);
	saveOrLoad(s, key.walkDataNum);
	saveOrLoad(s, key.spriteNum);
	saveOrLoad(s, key.prevBoundingRect);
	saveOrLoad(s, key.boundingRect);
	saveOrLoad(s, key.xPos);
	saveOrLoad(s, key.yPos);
	saveOrLoad(s, key.zPos);
	saveOrLoad(s, key.xPosPrev);
	saveOrLoad(s, key.yPosPrev);
	saveOrLoad(s, key.zPosPrev);
	saveOrLoad(s, key.prevWalkDataNum);
	saveOrLoad(s, key.textColor);
	for (uint i = 0; i < 4; ++i) {
		saveOrLoad(s, key.inventoryItems[i]);
	}
	saveOrLoad(s, key.money);
	saveOrLoad(s, key.pointsDataNum);
	saveOrLoad(s, key.currentWalkBox);
	saveOrLoad(s, key.prevPointsDataNum);
	saveOrLoad(s, key.currentAnim);
	saveOrLoad(s, key.facingDirection);
	saveOrLoad(s, key.currentAnimSpeed);
	for (uint i = 0; i < 16; ++i) {
		saveOrLoad(s, key.framesList[i]);
	}
	saveOrLoad(s, key.framesListCount);
	saveOrLoad(s, key.currentFrame);
	saveOrLoad(s, key.anim1Start);
	saveOrLoad(s, key.anim1Count);
	saveOrLoad(s, key.anim2Start);
	saveOrLoad(s, key.anim2Count);
	saveOrLoad(s, key.anim3Start);
	saveOrLoad(s, key.anim3Count);
	saveOrLoad(s, key.followingKeyCharNum);
	saveOrLoad(s, key.followingKeyCharPos);
	saveOrLoad(s, key.sequenceDataIndex);
	saveOrLoad(s, key.sequenceDataOffset);
	saveOrLoad(s, key.walkPointsListCount);
	for (uint i = 0; i < 40; ++i) {
		saveOrLoad(s, key.walkPointsList[i]);
	}
	saveOrLoad(s, key.scriptDataStartOffset);
	saveOrLoad(s, key.scriptDataOffset);
	saveOrLoadPtr(s, key.scriptStackPtr, &key.scriptStackTable[39]);
	saveOrLoad(s, key.delay);
	saveOrLoad(s, key.waitingKeyChar);
	for (uint i = 0; i < 3; ++i) {
		saveOrLoad(s, key.waitingKeyCharPosTable[i]);
	}
	for (uint i = 0; i < 40; ++i) {
		saveOrLoad(s, key.scriptStackTable[i]);
	}
}

template <class S>
static void saveOrLoad(S &s, TalkEntry &entry) {
	saveOrLoad(s, entry.otherKeyChar);
	saveOrLoad(s, entry.talkingKeyChar);
	saveOrLoad(s, entry.num);
}

template <class S>
static void saveOrLoad(S &s, ProgramHitBoxData &data) {
	saveOrLoad(s, data.item);
	saveOrLoad(s, data.talk);
	saveOrLoad(s, data.state);
	saveOrLoad(s, data.str);
	saveOrLoad(s, data.defaultStr);
	for (uint i = 0; i < 8; ++i) {
		saveOrLoad(s, data.actions[i]);
	}
	for (uint i = 0; i < 2; ++i) {
		saveOrLoad(s, data.hitBoxes[i]);
	}
}

template <class S>
static void saveOrLoad(S &s, Area &area) {
	saveOrLoad(s, area.r);
	saveOrLoad(s, area.srcX);
	saveOrLoad(s, area.srcY);
}

template <class S>
static void saveOrLoad(S &s, ProgramBackgroundData &data) {
	saveOrLoad(s, data.area);
	saveOrLoad(s, data.type);
	saveOrLoad(s, data.offset);
	saveOrLoad(s, data.scaleMul);
	saveOrLoad(s, data.scaleDiv);
}

template <class S>
static void saveOrLoad(S &s, ProgramAreaData &data) {
	saveOrLoad(s, data.area);
	saveOrLoad(s, data.id);
	saveOrLoad(s, data.state);
	saveOrLoad(s, data.animCount);
	saveOrLoad(s, data.animNext);
}

template <class S>
static void saveOrLoad(S &s, ProgramWalkData &data) {
	saveOrLoad(s, data.point1);
	saveOrLoad(s, data.point2);
	saveOrLoad(s, data.clippingRect);
	saveOrLoad(s, data.area1);
	saveOrLoad(s, data.area2);
}

template <class S>
static void saveOrLoad(S &s, ProgramPointData &data) {
	saveOrLoad(s, data.x);
	saveOrLoad(s, data.y);
	saveOrLoad(s, data.z);
	saveOrLoad(s, data.priority);
}

void ToucheEngine::saveGameStateData(Common::WriteStream *stream) {
	setKeyCharMoney();
	stream->writeUint16LE(_currentEpisodeNum);
	stream->writeUint16LE(_currentMusicNum);
	stream->writeUint16LE(_currentRoomNum);
	stream->writeUint16LE(_flagsTable[614]);
	stream->writeUint16LE(_flagsTable[615]);
	stream->writeUint16LE(_disabledInputCounter);
	for (uint i = 0; i < _programHitBoxTable.size(); ++i) {
		saveOrLoad(*stream, _programHitBoxTable[i]);
	}
	for (uint i = 0; i < _programBackgroundTable.size(); ++i) {
		saveOrLoad(*stream, _programBackgroundTable[i]);
	}
	for (uint i = 0; i < _programAreaTable.size(); ++i) {
		saveOrLoad(*stream, _programAreaTable[i]);
	}
	for (uint i = 0; i < _programWalkTable.size(); ++i) {
		saveOrLoad(*stream, _programWalkTable[i]);
	}
	for (uint i = 0; i < _programPointsTable.size(); ++i) {
		saveOrLoad(*stream, _programPointsTable[i]);
	}
	stream->write(_updatedRoomAreasTable, 200);
	for (uint i = 0; i < NUM_SEQUENCES; ++i) {
		saveOrLoad(*stream, _sequenceEntryTable[i]);
	}
	for (uint i = 0; i < 1024; ++i) {
		saveOrLoad(*stream, _flagsTable[i]);
	}
	for (uint i = 0; i < 100; ++i) {
		saveOrLoad(*stream, _inventoryList1[i]);
	}
	for (uint i = 0; i < 100; ++i) {
		saveOrLoad(*stream, _inventoryList2[i]);
	}
	for (uint i = 0; i < 6; ++i) {
		saveOrLoad(*stream, _inventoryList3[i]);
	}
	for (uint i = 0; i < NUM_KEYCHARS; ++i) {
		saveOrLoad(*stream, _keyCharsTable[i]);
	}
	for (uint i = 0; i < NUM_INVENTORY_ITEMS; ++i) {
		saveOrLoad(*stream, _inventoryItemsInfoTable[i]);
	}
	for (uint i = 0; i < NUM_TALK_ENTRIES; ++i) {
		saveOrLoad(*stream, _talkTable[i]);
	}
	stream->writeUint16LE(_talkListEnd);
	stream->writeUint16LE(_talkListCurrent);
}

void ToucheEngine::loadGameStateData(Common::ReadStream *stream) {
	setKeyCharMoney();
	clearDirtyRects();
	clearAreaTable();
	_flagsTable[115] = 0;
	clearRoomArea();
	int16 room_offs_x, room_offs_y;
	_currentEpisodeNum = stream->readUint16LE();
	_newMusicNum = stream->readUint16LE();
	_currentRoomNum = stream->readUint16LE();
	res_loadRoom(_currentRoomNum);
	room_offs_x = stream->readUint16LE();
	room_offs_y = stream->readUint16LE();
	_disabledInputCounter = stream->readUint16LE();
	res_loadProgram(_currentEpisodeNum);
	setupEpisode(-1);
	for (uint i = 0; i < _programHitBoxTable.size(); ++i) {
		saveOrLoad(*stream, _programHitBoxTable[i]);
	}
	for (uint i = 0; i < _programBackgroundTable.size(); ++i) {
		saveOrLoad(*stream, _programBackgroundTable[i]);
	}
	for (uint i = 0; i < _programAreaTable.size(); ++i) {
		saveOrLoad(*stream, _programAreaTable[i]);
	}
	for (uint i = 0; i < _programWalkTable.size(); ++i) {
		saveOrLoad(*stream, _programWalkTable[i]);
	}
	for (uint i = 0; i < _programPointsTable.size(); ++i) {
		saveOrLoad(*stream, _programPointsTable[i]);
	}
	stream->read(_updatedRoomAreasTable, 200);
	for (uint i = 1; i <= _updatedRoomAreasTable[0]; ++i) {
		updateRoomAreas(_updatedRoomAreasTable[i], -1);
	}
	for (uint i = 0; i < NUM_SEQUENCES; ++i) {
		saveOrLoad(*stream, _sequenceEntryTable[i]);
	}
	for (uint i = 0; i < 1024; ++i) {
		saveOrLoad(*stream, _flagsTable[i]);
	}
	for (uint i = 0; i < 100; ++i) {
		saveOrLoad(*stream, _inventoryList1[i]);
	}
	for (uint i = 0; i < 100; ++i) {
		saveOrLoad(*stream, _inventoryList2[i]);
	}
	for (uint i = 0; i < 6; ++i) {
		saveOrLoad(*stream, _inventoryList3[i]);
	}
	for (uint i = 0; i < NUM_KEYCHARS; ++i) {
		saveOrLoad(*stream, _keyCharsTable[i]);
	}
	for (uint i = 0; i < NUM_INVENTORY_ITEMS; ++i) {
		saveOrLoad(*stream, _inventoryItemsInfoTable[i]);
	}
	for (uint i = 0; i < NUM_TALK_ENTRIES; ++i) {
		saveOrLoad(*stream, _talkTable[i]);
	}
	_talkListEnd = stream->readUint16LE();
	_talkListCurrent = stream->readUint16LE();
	_flagsTable[614] = room_offs_x;
	_flagsTable[615] = room_offs_y;
	for (uint i = 0; i < 6; ++i) {
		if (_sequenceEntryTable[i].seqNum != -1) {
			res_loadSequence(_sequenceEntryTable[i].seqNum, i);
		}
		if (_sequenceEntryTable[i].sprNum != -1) {
			res_loadSprite(_sequenceEntryTable[i].sprNum, i);
		}
	}
	_currentKeyCharNum = _flagsTable[104];
	_inventoryStateTable[0].displayOffset = 0;
	_inventoryStateTable[1].displayOffset = 0;
	_inventoryStateTable[2].displayOffset = 0;
	drawInventory(_currentKeyCharNum, 1);
	Graphics::copyRect(_offscreenBuffer, 640, 0, 0, _backdropBuffer, _currentBitmapWidth, _flagsTable[614], _flagsTable[615], 640, 352);
	updateEntireScreen();
	if (_flagsTable[617] != 0) {
		res_loadSpeech(_flagsTable[617]);
	}
	debug(0, "Loaded state, current episode %d", _currentEpisodeNum);
}

bool ToucheEngine::saveGameState(int num, const char *description) {
	bool saveOk = false;
	char gameStateFileName[16];
	generateGameStateFileName(num, gameStateFileName, 15);
	Common::OutSaveFile *f = _saveFileMan->openForSaving(gameStateFileName);
	if (f) {
		f->writeUint16LE(kCurrentGameStateVersion);
		f->writeUint16LE(0);
		char headerDescription[kGameStateDescriptionLen];
		memset(headerDescription, 0, kGameStateDescriptionLen);
		strncpy(headerDescription, description, kGameStateDescriptionLen - 1);
		f->write(headerDescription, kGameStateDescriptionLen);
		saveGameStateData(f);
		f->flush();
		if (!f->ioFailed()) {
			saveOk = true;
		} else {
			warning("Can't write file '%s'", gameStateFileName);
		}
		delete f;
	}
	return saveOk;
}

bool ToucheEngine::loadGameState(int num) {
	bool loadOk = false;
	char gameStateFileName[16];
	generateGameStateFileName(num, gameStateFileName, 15);
	Common::InSaveFile *f = _saveFileMan->openForLoading(gameStateFileName);
	if (f) {
		uint16 version = f->readUint16LE();
		if (version < kCurrentGameStateVersion) {
			warning("Unsupported gamestate version %d\n", version);
		} else {
			f->skip(2 + kGameStateDescriptionLen);
			loadGameStateData(f);
			if (!f->ioFailed()) {
				loadOk = true;
			} else {
				warning("Can't read file '%s'", gameStateFileName);
			}
		}
		delete f;
	}
	return loadOk;
}

void ToucheEngine::readGameStateDescription(int num, char *description, int len) {
	char gameStateFileName[16];
	generateGameStateFileName(num, gameStateFileName, 15);
	Common::InSaveFile *f = _saveFileMan->openForLoading(gameStateFileName);
	if (f) {
		uint16 version = f->readUint16LE();
		if (version >= kCurrentGameStateVersion) {
			f->readUint16LE();
			f->read(description, MIN<int>(len, kGameStateDescriptionLen));
			description[len] = 0;
		}
		delete f;
	}
}

void ToucheEngine::generateGameStateFileName(int num, char *dst, int len, bool prefixOnly) const {
	if (prefixOnly) {
		snprintf(dst, len, "%s.", _targetName.c_str());
	} else {
		snprintf(dst, len, "%s.%d", _targetName.c_str(), num);
	}
	dst[len] = 0;
}

} // namespace Touche
