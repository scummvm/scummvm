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

#include "common/savefile.h"

#include "graphics/thumbnail.h"

#include "illusions/illusions.h"
#include "illusions/gamestate.h"

namespace Illusions {

#define ILLUSIONS_SAVEGAME_VERSION 0

IllusionsEngine::kReadSaveHeaderError IllusionsEngine::readSaveHeader(Common::SeekableReadStream *in, SaveHeader &header, bool skipThumbnail) {

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
	header.gameID = in->readByte();
	header.flags = in->readUint32LE();

	header.saveDate = in->readUint32LE();
	header.saveTime = in->readUint32LE();
	header.playTime = in->readUint32LE();

	return ((in->eos() || in->err()) ? kRSHEIoError : kRSHENoError);
}

bool IllusionsEngine::savegame(const char *filename, const char *description) {

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
	out->writeByte(0);
	out->writeUint32LE(0);
	uint32 saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	uint32 saveTime = ((curTime.tm_hour & 0xFF) << 16) | (((curTime.tm_min) & 0xFF) << 8) | ((curTime.tm_sec) & 0xFF);
	uint32 playTime = g_engine->getTotalPlayTime() / 1000;
	out->writeUint32LE(saveDate);
	out->writeUint32LE(saveTime);
	out->writeUint32LE(playTime);
	// Header end

	_gameState->write(out);

	out->finalize();
	delete out;
	return true;
}

bool IllusionsEngine::loadgame(const char *filename) {
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

	_gameState->read(in);

	delete in;
	return true;
}

Common::Error IllusionsEngine::loadGameState(int slot) {
	_resumeFromSavegameRequested = false;
	const char *fileName = getSavegameFilename(slot);
	if (!loadgame(fileName))
		return Common::kReadingFailed;
	_resumeFromSavegameRequested = true;
	_savegameSlotNum = slot;
	return Common::kNoError;
}
 
Common::Error IllusionsEngine::saveGameState(int slot, const Common::String &description, bool isAutosave) {
	const char *fileName = getSavegameFilename(slot);
	if (!savegame(fileName, description.c_str()))
		return Common::kWritingFailed;
	return Common::kNoError;
}

Common::Error IllusionsEngine::removeGameState(int slot) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String filename = Illusions::IllusionsEngine::getSavegameFilename(_targetName, slot);
	saveFileMan->removeSavefile(filename.c_str());
	return Common::kNoError;
}

const char *IllusionsEngine::getSavegameFilename(int num) {
	static Common::String filename;
	filename = getSavegameFilename(_targetName, num);
	return filename.c_str();
}

Common::String IllusionsEngine::getSavegameFilename(const Common::String &target, int num) {
	assert(num >= 0 && num <= 999);
	return Common::String::format("%s.%03d", target.c_str(), num);
}

} // End of namespace Illusions
