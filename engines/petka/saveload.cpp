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

#include "common/system.h"
#include "common/savefile.h"

#include "engines/savestate.h"

#include "graphics/thumbnail.h"

#include "petka/petka.h"
#include "petka/objects/object_star.h"
#include "petka/q_system.h"
#include "petka/interfaces/panel.h"
#include "petka/interfaces/save_load.h"
#include "petka/interfaces/main.h"

namespace Petka {

Common::Error PetkaEngine::loadGameState(int slot) {
	Common::ScopedPtr<Common::SeekableReadStream> in(_saveFileMan->openForLoading(generateSaveName(slot, _targetName.c_str())));
	if (!in)
		return Common::kNoGameDataFoundError;

	SaveStateDescriptor desc;
	if (!readSaveHeader(*in, desc))
		return Common::kUnknownError;

	setTotalPlayTime(desc.getPlayTimeMSecs());

	_nextPart = in->readUint32LE();
	_chapter = in->readUint32LE();
	if (_nextPart == _part) {
		loadChapter(_chapter);
		_qsystem->load(in.get());
	} else {
		_shouldChangePart = true;
		_saveSlot = slot;
	}

	return Common::kNoError;
}

Common::Error PetkaEngine::saveGameState(int slot, const Common::String &desci, bool isAutosave) {
	Common::ScopedPtr<Common::OutSaveFile> out(_saveFileMan->openForSaving(generateSaveName(slot, _targetName.c_str())));
	if (!out)
		return Common::kUnknownError;

	out->writeUint32BE(MKTAG('p', 'e', 't', 'k'));
	out->writeByte(desci.size());
	out->writeString(desci);

	TimeDate curTime;
	_system->getTimeAndDate(curTime);

	out->writeUint32LE(((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF));
	out->writeUint16LE(((curTime.tm_hour & 0xFF) << 8) | ((curTime.tm_min) & 0xFF));

	out->writeUint32LE(getTotalPlayTime() / 1000);

	if (!_thumbnail)
		return Common::kUnknownError;

	out->writeStream(_thumbnail.get());

	out->writeUint32LE(_part);
	out->writeUint32LE(_chapter);
	_qsystem->save(out.get());

	return Common::kNoError;
}

bool PetkaEngine::canSaveGameStateCurrently() {
	if (isDemo() || !_qsystem)
		return false;

	Interface *panel = _qsystem->_panelInterface.get();
	InterfaceSaveLoad *saveLoad = _qsystem->_saveLoadInterface.get();

	Interface *curr = _qsystem->_currInterface;
	Interface *prev = _qsystem->_prevInterface;

	return prev == _qsystem->_mainInterface.get() && (curr == saveLoad || curr == panel);
}

bool PetkaEngine::canLoadGameStateCurrently() {
	return !isDemo() && _qsystem;
}

int PetkaEngine::getSaveSlot() {
	return _saveSlot;
}

bool readSaveHeader(Common::InSaveFile &in, SaveStateDescriptor &desc, bool skipThumbnail) {
	if (in.readUint32BE() != MKTAG('p', 'e', 't', 'k'))
		return false;

	const Common::String description = in.readPascalString();
	uint32 date = in.readUint32LE();
	uint16 time = in.readUint16LE();
	uint32 playTime = in.readUint32LE();

	Graphics::Surface *thumbnail = nullptr;
	if (!Graphics::loadThumbnail(in, thumbnail, skipThumbnail))
		return false;

	int day = (date >> 24) & 0xFF;
	int month = (date >> 16) & 0xFF;
	int year = date & 0xFFFF;

	int hour = (time >> 8) & 0xFF;
	int minutes = time & 0xFF;

	desc.setSaveDate(year, month, day);
	desc.setSaveTime(hour, minutes);
	desc.setPlayTime(playTime * 1000);
	desc.setDescription(description);
	desc.setThumbnail(thumbnail);

	return true;
}

Common::String generateSaveName(int slot, const char *gameId) {
	return Common::String::format("%s.s%02d", gameId, slot);
}

}

