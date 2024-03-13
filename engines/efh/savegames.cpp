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
#include "graphics/paletteman.h"

#include "efh/efh.h"

namespace Efh {

Common::String EfhEngine::getSavegameFilename(int slot) {
	return _targetName + Common::String::format("-%03d.SAV", slot);
}

bool EfhEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return true;
}

bool EfhEngine::canSaveGameStateCurrently(Common::U32String *msg) {
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
	if (!Graphics::loadThumbnail(*saveFile, thumbnail))
		return Common::kReadingFailed;

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
	s.syncAsSint16LE(_teamChar[0]._id);
	s.syncAsSint16LE(_teamChar[1]._id);
	s.syncAsSint16LE(_teamChar[2]._id);

	for (int i = 0; i < 3; ++i) {
		s.syncAsSint16LE(_teamChar[i]._status._type);
		s.syncAsSint16LE(_teamChar[i]._status._duration);
	}

	s.syncAsSint16LE(_teamSize);
	s.syncAsSint16LE(_alertDelay);
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

		s.syncAsByte(_mapBitmapRefArr[i]._setId1);
		s.syncAsByte(_mapBitmapRefArr[i]._setId2);
		for (int idx = 0; idx < 100; ++idx) {
			s.syncAsByte(_mapSpecialTiles[i][idx]._placeId);
			s.syncAsByte(_mapSpecialTiles[i][idx]._posX);
			s.syncAsByte(_mapSpecialTiles[i][idx]._posY);
			s.syncAsByte(_mapSpecialTiles[i][idx]._triggerType);
			s.syncAsByte(_mapSpecialTiles[i][idx]._triggerValue);
			s.syncAsUint16LE(_mapSpecialTiles[i][idx]._field5_textId);
			s.syncAsUint16LE(_mapSpecialTiles[i][idx]._field7_textId);
		}

		for (int idx = 0; idx < 64; ++idx) {
			s.syncAsByte(_mapMonsters[i][idx]._possessivePronounSHL6);
			s.syncAsByte(_mapMonsters[i][idx]._npcId);
			s.syncAsByte(_mapMonsters[i][idx]._fullPlaceId);
			s.syncAsByte(_mapMonsters[i][idx]._posX);
			s.syncAsByte(_mapMonsters[i][idx]._posY);
			s.syncAsByte(_mapMonsters[i][idx]._weaponItemId);
			s.syncAsByte(_mapMonsters[i][idx]._maxDamageAbsorption);
			s.syncAsByte(_mapMonsters[i][idx]._monsterRef);
			s.syncAsByte(_mapMonsters[i][idx]._additionalInfo);
			s.syncAsByte(_mapMonsters[i][idx]._talkTextId);
			s.syncAsByte(_mapMonsters[i][idx]._groupSize);
			for (int j = 0; j < 9; ++j)
				s.syncAsSint16LE(_mapMonsters[i][idx]._hitPoints[j]);
		}

		for (int x = 0; x < 64; ++x) {
			for (int y = 0; y < 64; ++y)
				s.syncAsByte(_mapGameMaps[i][x][y]);
		}
	}

	// Dialog flags
	for (int i = 0; i < 256; ++i)
		s.syncAsByte(_history[i]);

	// NPCs
	for (int i = 0; i < 99; ++i) {
		_npcBuf[i].synchronize(s);
	}

	s.syncAsByte(_saveAuthorized);
}

} // End of namespace Efh

