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
#include "common/savefile.h"

#include "graphics/thumbnail.h"

#include "dragons/dragons.h"
#include "dragons/dragonflg.h"
#include "dragons/dragonini.h"
#include "dragons/dragonvar.h"
#include "dragons/scene.h"
#include "dragons/cursor.h"


namespace Dragons {

#define ILLUSIONS_SAVEGAME_VERSION 0

kReadSaveHeaderError DragonsEngine::readSaveHeader(Common::SeekableReadStream *in, SaveHeader &header, bool skipThumbnail) {
	header.version = in->readUint32LE();
	if (header.version > ILLUSIONS_SAVEGAME_VERSION)
		return kRSHEInvalidVersion;

	byte descriptionLen = in->readByte();
	header.description = "";
	while (descriptionLen--) {
		header.description += (char)in->readByte();
	}

	if (!Graphics::loadThumbnail(*in, header.thumbnail, skipThumbnail)) {
		return kRSHEIoError;
	}

	// Not used yet, reserved for future usage
//	header.gameID = in->readByte();
	header.flags = in->readUint32LE();

	header.saveDate = in->readUint32LE();
	header.saveTime = in->readUint32LE();
	header.playTime = in->readUint32LE();

	return ((in->eos() || in->err()) ? kRSHEIoError : kRSHENoError);
}

bool DragonsEngine::savegame(const char *filename, const char *description) {

	Common::OutSaveFile *out;
	if (!(out = g_system->getSavefileManager()->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename);
		return false;
	}

	TimeDate curTime;
	g_system->getTimeAndDate(curTime);

	// Header start
	out->writeUint32LE(ILLUSIONS_SAVEGAME_VERSION);

	byte descriptionLen = strlen(description);
	out->writeByte(descriptionLen);
	out->write(description, descriptionLen);

	// TODO Probably pre-generate the thumbnail before the internal menu system is
	// called to have a thumbnail without the menu system itself on it.
	// Use the automatic thumbnail generation only when the ScummVM save dialog is used.
	Graphics::saveThumbnail(*out);

	// Not used yet, reserved for future usage
	//out->writeByte(0);
	out->writeUint32LE(0);
	uint32 saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	uint32 saveTime = ((curTime.tm_hour & 0xFF) << 16) | (((curTime.tm_min) & 0xFF) << 8) | ((curTime.tm_sec) & 0xFF);
	uint32 playTime = g_engine->getTotalPlayTime() / 1000;
	out->writeUint32LE(saveDate);
	out->writeUint32LE(saveTime);
	out->writeUint32LE(playTime);
	// Header end

	//TODO save gamestate here. _gameState->write(out);
	out->writeByte((int8)getCurrentSceneId());
	_dragonFLG->saveState(out);

	out->finalize();
	delete out;
	return true;
}

bool DragonsEngine::loadgame(const char *filename) {
	Common::InSaveFile *in;
	if (!(in = g_system->getSavefileManager()->openForLoading(filename))) {
		warning("Can't open file '%s', game not loaded", filename);
		return false;
	}

	SaveHeader header;

	kReadSaveHeaderError errorCode = readSaveHeader(in, header);

	if (errorCode != kRSHENoError) {
		warning("Error loading savegame '%s'", filename);
		delete in;
		return false;
	}

	g_engine->setTotalPlayTime(header.playTime * 1000);

	reset();
	// TODO load game state here. _gameState->read(in);
	uint16 newSceneId = (uint16)in->readByte();
	_dragonFLG->loadState(in);
	//_dragonFLG->set(165, true); //TODO check why this is needed to load save games properly.
	_dragonFLG->set(125, false);  //TODO check why this is needed to load save games properly.
	_dragonVAR->reset();

	_dragonINIResource->reset();

	init();
	loadScene(newSceneId);
	setFlags(ENGINE_FLAG_8); //Re-enable cursor TODO should we need to do this?

	delete in;
	return true;
}

Common::Error DragonsEngine::loadGameState(int slot) {
	if (!loadgame(getSavegameFilename(slot).c_str()))
		return Common::kReadingFailed;
	return Common::kNoError;
}

Common::Error DragonsEngine::saveGameState(int slot, const Common::String &description, bool isAutoSave) {
	if (!savegame(getSavegameFilename(slot).c_str(), description.c_str()))
		return Common::kWritingFailed;
	return Common::kNoError;
}

Common::String DragonsEngine::getSavegameFilename(int num) {
	return getSavegameFilename(_targetName, num);
}

Common::String DragonsEngine::getSavegameFilename(const Common::String &target, int num) {
	assert(num >= 0 && num <= 999);
	return Common::String::format("%s.%03d", target.c_str(), num);
}

} // End of namespace Illusions
