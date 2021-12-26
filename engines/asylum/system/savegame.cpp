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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"

#include "asylum/system/savegame.h"

#include "asylum/puzzles/puzzles.h"

#include "asylum/resources/encounters.h"
#include "asylum/resources/script.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/screen.h"
#include "asylum/system/text.h"

#include "asylum/views/menu.h"
#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

#define SAVEGAME_BUILD 851
#define SAVEGAME_VERSION_SIZE 11
#define SAVEGAME_NAME_SIZE 45

#define SAVEGAME_QUICKSLOT 24

static const char *savegame_version = "v1.01 FINAL";

Savegame::Savegame(AsylumEngine *engine) : _vm(engine), _index(0) {
	memset(&_moviesViewed, 0, sizeof(_moviesViewed));
	memset(&_savegames, 0, sizeof(_savegames));
	memset(&_savegameToScene, 0, sizeof(_savegameToScene));
	_build = 0;
}

bool Savegame::hasSavegames() const {
	for (uint i = 0; i < SAVEGAME_COUNT; i++)
		if (isSavegamePresent(getFilename(i)))
			return true;

	return false;
}

void Savegame::loadList() {
	for (uint32 i = 0; i < SAVEGAME_COUNT; i++) {
		if (isSavegamePresent(getFilename(i))) {
			Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(getFilename(i));
			if (!file)
				error("[Savegame::loadList] Cannot open savegame: %s", getFilename(i).c_str());

			// Check file size (we handle empty files, but not invalid ones)
			if (file->size() == 0) {
				_names[i] = getText()->get(MAKE_RESOURCE(kResourcePackText, 1324));
				_savegames[i] = false;
			} else {
				_savegameToScene[i] = read(file, "Level");
				_names[i] = read(file, 45, "Game Name");
				_savegames[i] = true;
			}

			delete file;
		} else {
			_names[i] = getText()->get(MAKE_RESOURCE(kResourcePackText, 1324));
			_savegames[i] = false;
		}
	}
}

void Savegame::load() {
	getCursor()->hide();
	// Original clears the graphic cache
	getScript()->resetQueue();
	getSound()->playMusic(kResourceNone, 0);
	getScene()->load((ResourcePackId)(_savegameToScene[_index] + 4));
	_vm->reset();
	// Original loads encounter data

	loadData(getFilename(_index));

	loadMoviesViewed();

	getMenu()->setDword455C80(false);
	getScreen()->clear();
}

bool Savegame::quickLoad() {
	if (!isSavegamePresent(getFilename(SAVEGAME_QUICKSLOT)))
		return false;

	_index = SAVEGAME_QUICKSLOT;
	(void)_vm->startGame(getScenePack(), AsylumEngine::kStartGameLoad);

	return true;
}

void Savegame::save() {
	// Original creates a folder to hold saved games and checks for disk space, we can skip that
	getCursor()->hide();

	saveData(getFilename(_index), _names[_index], getWorld()->chapter);
	_savegames[_index] = true;
	getMenu()->setDword455C78(true);
	getMenu()->setDword455C80(false);
	getCursor()->show();
}

bool Savegame::quickSave() {
	_index = 24;

	// Check if there is a quick save already
	if (!isSavegamePresent(getFilename(SAVEGAME_QUICKSLOT))) {
		_names[_index] = getText()->get(MAKE_RESOURCE(kResourcePackText, 1342));

		save();
	} else {
		Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(getFilename(SAVEGAME_QUICKSLOT));
		if (!file)
			return false;

		// Read game name
		seek(file, 1, "Level");
		_names[_index] = read(file, 45, "Game Name");

		delete file;

		save();
	}

	return true;
}

void Savegame::remove() {
	if (_index >= ARRAYSIZE(_savegames))
		error("[Savegame::remove] Invalid savegame index");

	getCursor()->hide();
	g_system->getSavefileManager()->removeSavefile(getFilename(_index));

	// Update status and name
	_savegames[_index] = false;
	_names[_index] = getText()->get(MAKE_RESOURCE(kResourcePackText, 1344));

	getMenu()->setDword455C80(false);
	getCursor()->show();
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
bool Savegame::isCompatible() {
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(getFilename(_index));

	assert(file);
	seek(file, 2, "Level and Name");
	bool result = readHeader(file);

	delete file;

	return result;
}

Common::String Savegame::getFilename(uint32 index) const {
	if (index > SAVEGAME_COUNT - 1)
		error("[Savegame::getFilename] Invalid savegame index (was:%d, valid: [0-24])", index);

	return _vm->getSaveStateName(index);
}

bool Savegame::isSavegamePresent(const Common::String &filename) const {
	if (g_system->getSavefileManager()->listSavefiles(filename).size() == 0)
		return false;

	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(filename);
	if (!file)
		return false;

	bool isSaveValid = (file->size() == 0) ? false : true;

	delete file;

	return isSaveValid;
}

//////////////////////////////////////////////////////////////////////////
// Reading & writing
//////////////////////////////////////////////////////////////////////////
bool Savegame::readHeader(Common::InSaveFile *file) {
	uint32 versionLength = read(file, "Version Length");
	_version = read(file, versionLength, "Version");
	_build = read(file, "Build");

	// Original does not do any version check
	return !strcmp(_version.c_str(), savegame_version) && _build == SAVEGAME_BUILD;
}

void Savegame::writeHeader(Common::OutSaveFile *file) const {
	// We write saved games with a 1.01 final version (build 851)

	write(file, SAVEGAME_VERSION_SIZE, "Version Length");
	write(file, Common::String(savegame_version), SAVEGAME_VERSION_SIZE, "Version");
	write(file, SAVEGAME_BUILD, "Build");
}

void Savegame::loadData(const Common::String &filename) {
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(filename);
	assert(file);

	seek(file, 1, "Level");
	seek(file, 1, "Game Name");

	(void)readHeader(file);

	read(file, _vm, 1512, 1, "Game Stats");
	read(file, getWorld(), 951928, 1, "World Stats");
	read(file, getPuzzles(), 752, 1, "Blowup Puzzle Data");
	read(file, getEncounter()->items(), 109, getEncounter()->items()->size(), "Encounter Data");
	read(file, getEncounter()->variables(), 2, getEncounter()->variables()->size(), "Encounter Variables");

	getScript()->reset(getWorld()->numScripts);

	if (getWorld()->numScripts)
		read(file, getScript(), 7096, (uint32)getWorld()->numScripts, "Action Lists");

	uint32 tick = read(file, "Time");

	_vm->setTick(tick);

	delete file;
}

void Savegame::saveData(const Common::String &filename, const Common::String &name, ChapterIndex chapter) {
	Common::OutSaveFile *file = g_system->getSavefileManager()->openForSaving(filename);
	assert(file);

	write(file, (unsigned) (int32)chapter, "Level");
	write(file, name, SAVEGAME_NAME_SIZE, "Game Name");
	writeHeader(file);
	write(file, _vm, 1512, 1, "Game Stats");
	write(file, getWorld(), 951928, 1, "World Stats");
	write(file, getPuzzles(), 752, 1, "Blowup Puzzle Data");
	write(file, getEncounter()->items(), 109, getEncounter()->items()->size(), "Encounter Data");
	write(file, getEncounter()->variables(), 2, getEncounter()->variables()->size(), "Encounter Variables");

	if (getWorld()->numScripts)
		write(file, getScript(), 7096, (uint32)getWorld()->numScripts, "Action Lists");

	write(file, _vm->getTick(), "Time");

	_vm->getMetaEngine()->appendExtendedSaveToStream(file, _vm->getTotalPlayTime() / 1000, name, false);

	delete file;
}

void Savegame::seek(Common::InSaveFile *file, uint32 offset, const Common::String &description) {
	debugC(kDebugLevelSavegame, "[Savegame] Seeking to offset: %s", description.c_str());

	if (offset == 0)
		return;

	uint32 size = 0;
	uint32 count = 0;

	for (uint i = 0; i < offset; i++) {
		size = file->readUint32LE();
		count = file->readUint32LE();

		file->seek(size * count, SEEK_CUR);
	}
}

uint32 Savegame::read(Common::InSaveFile *file, const Common::String &description) {
	debugC(kDebugLevelSavegame, "[Savegame] Reading %s", description.c_str());

	uint32 size = file->readUint32LE();
	uint32 count = file->readUint32LE();

	if (size * count == 0)
		return 0;

	return file->readUint32LE();
}

Common::String Savegame::read(Common::InSaveFile *file, uint32 strLength, const Common::String &description) {
	debugC(kDebugLevelSavegame, "[Savegame] Reading %s (of length %d)", description.c_str(), strLength);

	/*uint32 size =*/ file->readUint32LE();
	uint32 count = file->readUint32LE();

	if (strLength > count)
		error("[Savegame::read] Count too large (asked: %d, present: %d)", strLength, count);

	char *str = new char[strLength + 1]();
	file->read(str, strLength);

	Common::String ret(str);

	delete[] str;

	return ret;
}

void Savegame::read(Common::InSaveFile *file, Common::Serializable *data, uint32 size, uint32 count, const Common::String &description) {
	debugC(kDebugLevelSavegame, "[Savegame] Reading %s (%d block(s) of size %d)", description.c_str(), size, count);

	uint32 fileSize = file->readUint32LE();
	if (size > fileSize)
		error("[Savegame::read] Size too large (asked: %d, present: %d)", size, fileSize);

	uint32 fileCount = file->readUint32LE();
	if (count > fileCount)
		error("[Savegame::read] Count too large (asked: %d, present: %d)", count, fileCount);

	if (fileCount * fileSize == 0)
		return;

	Common::Serializer ser(file, nullptr);
	data->saveLoadWithSerializer(ser);
}

void Savegame::write(Common::OutSaveFile *file, uint32 val, const Common::String &description) {
	debugC(kDebugLevelSavegame, "[Savegame] Writing %s: %d", description.c_str(), val);

	file->writeUint32LE(4);
	file->writeUint32LE(1);

	file->writeUint32LE(val);
}

void Savegame::write(Common::OutSaveFile *file, const Common::String &val, uint32 strLength, const Common::String &description) {
	debugC(kDebugLevelSavegame, "[Savegame] Writing %s (of length %d): %s", description.c_str(), strLength, val.c_str());

	if (val.size() > strLength)
		error("[Savegame::write] Trying to save a string that is longer than the specified size (string size: %d, size: %d)", val.size(), strLength);

	file->writeUint32LE(1);
	file->writeUint32LE(strLength);

	file->writeString(val);

	// Add padding
	if (val.size() < strLength) {
		for (uint32 i = 0; i < (strLength - val.size()); i++)
			file->writeByte(0);
	}
}

void Savegame::write(Common::OutSaveFile *file, Common::Serializable *data, uint32 size, uint32 count, const Common::String &description) {
	debugC(kDebugLevelSavegame, "[Savegame] Writing %s (%d block(s) of size %d)", description.c_str(), size, count);

	file->writeUint32LE(size);
	file->writeUint32LE(count);

	if (size * count == 0)
		return;

	Common::Serializer ser(nullptr, file);
	uint before = ser.bytesSynced();

	// Save the data
	data->saveLoadWithSerializer(ser);

	// Check we wrote the correct amount of data
	uint after = ser.bytesSynced();

	if ((after - before) != (size * count))
		error("[Savegame::write] Invalid number of bytes written to file (was: %d, expected: %d)", after - before, size * count);
}

//////////////////////////////////////////////////////////////////////////
// Movies
//////////////////////////////////////////////////////////////////////////
void Savegame::setMovieViewed(uint32 index) {
	if (index >= ARRAYSIZE(_moviesViewed))
		error("[Savegame::setMovieViewed] Invalid movie index!");

	if (!_moviesViewed[index]) {
		_moviesViewed[index] = 1;

		// Write data to disk
		Common::OutSaveFile *movies = g_system->getSavefileManager()->openForSaving(_vm->getMoviesFileName());
		if (!movies)
			error("[Savegame::setMovieViewed] Could not open viewed movie list!");

		movies->write((byte *)&_moviesViewed, sizeof(_moviesViewed));

		delete movies;
	}
}

uint32 Savegame::getMoviesViewed(int32 *movieList) const {
	memset(movieList, -1, 196 * sizeof(int32));

	uint32 count = 0;

	for (uint32 i = 0; i < ARRAYSIZE(_moviesViewed); i++) {
		if (_moviesViewed[i]) {
			movieList[count] = i;
			++count;
		}
	}

	return count;
}

void Savegame::loadMoviesViewed() {
	if (!isSavegamePresent(_vm->getMoviesFileName()))
		return;

	// Load data from disk
	Common::InSaveFile *movies = g_system->getSavefileManager()->openForLoading(_vm->getMoviesFileName());
	if (!movies)
		error("[Savegame::setMovieViewed] Could not open viewed movie list!");

	movies->read((byte *)&_moviesViewed, sizeof(_moviesViewed));

	delete movies;
}

//////////////////////////////////////////////////////////////////////////
// Accessors
//////////////////////////////////////////////////////////////////////////
void Savegame::setName(uint32 index, const Common::String &name) {
	if (index >= ARRAYSIZE(_names))
		error("[Savegame::setName] Invalid index (was: %d, max: %d)", index, ARRAYSIZE(_names) - 1);

	_names[index] = name;
}

Common::String Savegame::getName(uint32 index) const {
	if (index >= ARRAYSIZE(_names))
		error("[Savegame::getName] Invalid index (was: %d, max: %d)", index, ARRAYSIZE(_names) - 1);

	return _names[index];
}

bool Savegame::hasSavegame(uint32 index) const {
	if (index >= ARRAYSIZE(_savegames))
		error("[Savegame::hasSavegame] Invalid index (was: %d, max: %d)", index, ARRAYSIZE(_savegames) - 1);

	return _savegames[index];
}

} // End of namespace Asylum
