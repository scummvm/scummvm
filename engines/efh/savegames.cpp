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

#include "common/savefile.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "common/system.h"
#include "graphics/palette.h"

#include "efh/efh.h"

namespace Efh {

Common::String EfhEngine::getSavegameFilename(int slot) {
	return _targetName + Common::String::format("-%03d.SAV", slot);
}

bool EfhEngine::canLoadGameStateCurrently() {
	return true;
}

bool EfhEngine::canSaveGameStateCurrently() {
	return _saveAuthorized;
}

Common::Error EfhEngine::loadGameState(int slot) {
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(getSaveStateName(slot));
	if (!saveFile)
		return Common::kReadingFailed;

	uint32 signature = saveFile->readUint32LE();
	byte version = saveFile->readByte();

	if (signature != EFH_SAVE_HEADER || version > kSavegameVersion)
		error("Invalid savegame");

	// Skip savegame name
	uint16 size = saveFile->readUint16LE();
	saveFile->skip(size);

	// Skip the thumbnail
	Graphics::Surface *thumbnail;
	Graphics::loadThumbnail(*saveFile, thumbnail);
	delete (thumbnail);

	// Skip the savegame date
	saveFile->skip(10); // year, month, day, hours, minutes (all int16)

	Common::Serializer s(saveFile, nullptr);
	synchronize(s);

	delete saveFile;

	_oldMapPosX = _mapPosX;
	_oldMapPosY = _mapPosY;
	_unkRelatedToAnimImageSetId = 0;

	loadTechMapImp(_techId);
	_lastMainPlaceId = 0xFFFF;
	loadPlacesFile(_fullPlaceId, true);

	return Common::kNoError;
}

Common::Error EfhEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::OutSaveFile *out = _system->getSavefileManager()->openForSaving(getSaveStateName(slot));
	if (!out)
		return Common::kCreatingFileFailed;

	out->writeUint32LE(EFH_SAVE_HEADER);
	out->writeByte(kSavegameVersion);

	// Write savegame name
	uint16 size = desc.size();
	out->writeUint16LE(size);
	for (int i = 0; i < size; ++i)
		out->writeByte(desc.c_str()[i]);

	// Get the active palette
	uint8 thumbPalette[16 * 3];
	_system->getPaletteManager()->grabPalette(thumbPalette, 0, 16);
	// Create a thumbnail and save it
	Graphics::Surface *thumb = new Graphics::Surface();
	Graphics::Surface *sf = _mainSurface;
	::createThumbnail(thumb, (const byte *)sf->getPixels(), 320, 200, thumbPalette);
	Graphics::saveThumbnail(*out, *thumb);
	thumb->free();
	delete thumb;

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);

	Common::Serializer s(nullptr, out);
	synchronize(s);

	out->finalize();
	delete out;

	return Common::kNoError;
}

void EfhEngine::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(_techId);
	s.syncAsUint16LE(_fullPlaceId);
	s.syncAsSint16LE(_guessAnimationAmount);
	s.syncAsUint16LE(_largeMapFlag);
	s.syncAsSint16LE(_teamCharId[0]);
	s.syncAsSint16LE(_teamCharId[1]);
	s.syncAsSint16LE(_teamCharId[2]);

	for (int i = 0; i < 3; ++i) {
		s.syncAsSint16LE(_teamCharStatus[i]._status);
		s.syncAsSint16LE(_teamCharStatus[i]._duration);
	}

	s.syncAsSint16LE(_teamSize);
	s.syncAsSint16LE(_unk2C8AA);
	s.syncAsSint16LE(_word2C872);
	s.syncAsSint16LE(_imageSetSubFilesIdx);
	s.syncAsSint16LE(_mapPosX);
	s.syncAsSint16LE(_mapPosY);
	s.syncAsSint16LE(_techDataId_MapPosX);
	s.syncAsSint16LE(_techDataId_MapPosY);

	for (int i = 0; i < 19; ++i) {
		int size = ARRAYSIZE(_techDataArr[i]);
		for (int j = 0; j < size; ++j)
			s.syncAsByte(_techDataArr[i][j]);

		size = ARRAYSIZE(_mapArr[i]);
		for (int j = 0; j < size; ++j)
			s.syncAsByte(_mapArr[i][j]);

		_mapBitmapRefArr[i] = &_mapArr[i][0];
	}

	// Dialog flags
	for (int i = 0; i < 256; ++i)
		s.syncAsByte(_history[i]);

	// NPCs
	for (int i = 0; i < 99; ++i) {
		for (int idx = 0; idx < 11; ++idx)
			s.syncAsByte(_npcBuf[i]._name[idx]);

		s.syncAsByte(_npcBuf[i].fieldB_textId);
		s.syncAsByte(_npcBuf[i].field_C);
		s.syncAsByte(_npcBuf[i].field_D);
		s.syncAsByte(_npcBuf[i].fieldE_textId);
		s.syncAsByte(_npcBuf[i].field_F);
		s.syncAsByte(_npcBuf[i].field_10);
		s.syncAsByte(_npcBuf[i].field_11);
		s.syncAsSint16LE(_npcBuf[i].field12_textId);
		s.syncAsSint16LE(_npcBuf[i].field14_textId);
		s.syncAsSint32LE(_npcBuf[i]._xp);
		for (int idx = 0; idx < 15; ++idx)
			s.syncAsByte(_npcBuf[i]._activeScore[idx]);

		for (int idx = 0; idx < 11; ++idx)
			s.syncAsByte(_npcBuf[i]._passiveScore[idx]);

		for (int idx = 0; idx < 11; ++idx)
			s.syncAsByte(_npcBuf[i]._infoScore[idx]);

		s.syncAsByte(_npcBuf[i].field_3F);
		s.syncAsByte(_npcBuf[i].field_40);
		for (int idx = 0; idx < 10; ++idx) {
			s.syncAsSint16LE(_npcBuf[i]._inventory[idx]._ref);
			s.syncAsByte(_npcBuf[i]._inventory[idx]._stat1);
			s.syncAsByte(_npcBuf[i]._inventory[idx]._stat2);
		}
		s.syncAsByte(_npcBuf[i]._possessivePronounSHL6);
		s.syncAsByte(_npcBuf[i]._speed);
		s.syncAsByte(_npcBuf[i].field_6B);
		s.syncAsByte(_npcBuf[i].field_6C);
		s.syncAsByte(_npcBuf[i].field_6D);
		s.syncAsByte(_npcBuf[i]._unkItemId);
		s.syncAsByte(_npcBuf[i].field_6F);
		s.syncAsByte(_npcBuf[i].field_70);
		s.syncAsByte(_npcBuf[i].field_71);
		s.syncAsByte(_npcBuf[i].field_72);
		s.syncAsByte(_npcBuf[i].field_73);
		s.syncAsSint16LE(_npcBuf[i]._hitPoints);
		s.syncAsSint16LE(_npcBuf[i]._maxHP);
		s.syncAsByte(_npcBuf[i].field_78);
		s.syncAsSint16LE(_npcBuf[i].field_79);
		s.syncAsSint16LE(_npcBuf[i].field_7B);
		s.syncAsByte(_npcBuf[i].field_7D);
		s.syncAsByte(_npcBuf[i].field_7E);
		s.syncAsByte(_npcBuf[i].field_7F);
		s.syncAsByte(_npcBuf[i].field_80);
		s.syncAsByte(_npcBuf[i].field_81);
		s.syncAsByte(_npcBuf[i].field_82);
		s.syncAsByte(_npcBuf[i].field_83);
		s.syncAsByte(_npcBuf[i].field_84);
		s.syncAsByte(_npcBuf[i].field_85);
	}

	s.syncAsByte(_saveAuthorized);
}

} // End of namespace Efh

