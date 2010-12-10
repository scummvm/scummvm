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
 * $URL$
 * $Id$
 *
 */

#include "asylum/system/savegame.h"

#include "asylum/puzzles/data.h"

#include "asylum/resources/encounters.h"
#include "asylum/resources/script.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/screen.h"
#include "asylum/system/sound.h"
#include "asylum/system/text.h"

#include "asylum/views/menu.h"
#include "asylum/views/scene.h"

#include "asylum/asylum.h"

#include "common/file.h"

namespace Asylum {

#define SAVEGAME_BUILD 851
#define SAVEGAME_VERSION_SIZE 11
#define SAVEGAME_NAME_SIZE 45

#define SAVEGAME_NAME "asylum"

#define SAVEGAME_QUICKSLOT 25

#define SAVEGAME_MOVIES "asylum.movies"

const Common::String savegame_version = "v1.01 FINAL";

Savegame::Savegame(AsylumEngine *engine) : _vm(engine), _index(0), _valid(false) {
	memset(&_moviesViewed, 0, sizeof(_moviesViewed));
	memset(&_savegames, 0, sizeof(_savegames));
	memset(&_savegameToScene, 0, sizeof(_savegameToScene));
}

Savegame::~Savegame() {
	// Zero passed pointers
	_vm = NULL;
}

bool Savegame::hasSavegames() {
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

			_savegameToScene[i] = read(file, "Level");
			_names[i] = read(file, 45, "Game Name");
			_savegames[i] = true;

			delete file;
		} else {
			_names[i] = getText()->get(MAKE_RESOURCE(kResourcePackText, 1324));
			_savegames[i] = false;
		}
	}
}

bool Savegame::load() {
	if (!check()) {
		getMenu()->setDword455C78(true);
		getMenu()->setDword455C80(false);

		return false;
	}

	getCursor()->hide();
	// Original clears the graphic cache
	getScript()->resetQueue();
	getSound()->playMusic(kResourceNone, 0);
	getScene()->load((ResourcePackId)(_savegameToScene[_index] + 4));
	_vm->reset();
	// Original loads encounter data

	if (!loadData(getFilename(_index))) {
		// FIXME convert to GUI dialog
		if (_valid)
			error("[Savegame::load] Could not load game!");
		else
			error("[Savegame::load] Trying to load a game for a different version of Sanitarium!");
	}

	loadMoviesViewed();

	getMenu()->setDword455C80(false);
	getScreen()->clear();

	return true;
}

bool Savegame::quickLoad() {
	if (!isSavegamePresent(getFilename(SAVEGAME_QUICKSLOT)))
		return false;

	_index = SAVEGAME_QUICKSLOT;
	_vm->startGame((ResourcePackId)(getSharedData()->cdNumber + 4), AsylumEngine::kStartGameLoad);

	return true;
}

void Savegame::save() {
	// Original creates a folder to hold saved games and checks for disk space, we can skip that
	getCursor()->hide();

	if (saveData(getFilename(_index), _names[_index], getWorld()->chapter)) {
		_savegames[_index] = true;

		getMenu()->setDword455C78(true);
	} else {
		warning("[Savegame::save] Could not save game: %s", getFilename(_index).c_str());

		_savegames[_index] = false;
		_names[_index] = getText()->get(MAKE_RESOURCE(kResourcePackText, 1344));
	}

	getMenu()->setDword455C80(false);
	getCursor()->show();
}

bool Savegame::quickSave() {
	_index = 24;

	// Check if there is a quick save already
	if (isSavegamePresent(getFilename(SAVEGAME_QUICKSLOT))) {
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
bool Savegame::check() {
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(getFilename(_index));
	if (!file)
		return false;

	seek(file, 2, "Level and Name");

	bool valid = false;
	if (readHeader(file))
		valid = true;

	delete file;

	return valid;
}

Common::String Savegame::getFilename(uint32 index) {
	if (index > SAVEGAME_COUNT - 1)
		error("[Savegame::getFilename] Invalid savegame index (was:%d, valid: [0-25])", index);

	return Common::String::format("%s%02d.sav", SAVEGAME_NAME, index);
}

bool Savegame::isSavegamePresent(Common::String filename) {
	if (g_system->getSavefileManager()->listSavefiles(filename).size() == 0)
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Reading & writing
//////////////////////////////////////////////////////////////////////////
bool Savegame::readHeader(Common::InSaveFile *file) {
	uint32 versionLength = read(file, "Version Length");
	Common::String version = read(file, versionLength, "Version");
	/*uint32 build = */read(file, "Build");

	// Original does not do any version check
	// TODO check version to make sure we can read the data
	_valid = true;

	return true;
}

void Savegame::writeHeader(Common::OutSaveFile *file) {
	// We write saved games with a 1.01 final version (build 851)

	write(file, SAVEGAME_VERSION_SIZE, "Version Length");
	write(file, savegame_version, SAVEGAME_VERSION_SIZE, "Version");
	write(file, SAVEGAME_BUILD, "Build");
}

bool Savegame::loadData(Common::String filename) {
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(getFilename(_index));
	if (!file) {
		getWorld()->chapter = kChapterInvalid;
		return false;
	}

	seek(file, 1, "Level");
	seek(file, 1, "Game Name");

	if (!readHeader(file)) {
		getWorld()->chapter = kChapterInvalid;
		return false;
	}

	read(file, _vm, 1512, 1, "Game Stats");
	read(file, getWorld(), 951928, 1, "World Stats");
	read(file, getPuzzleData(), 752, 1, "Blowup Puzzle Data");
	read(file, getEncounter()->items(), 109, getEncounter()->items()->size(), "Encounter Data");
	read(file, getEncounter()->variables(), 2, getEncounter()->variables()->size(), "Encounter Variables");

	getScript()->reset();

	if (getWorld()->numScripts)
		read(file, getScript(), 7096, getWorld()->numScripts, "Action Lists");

	uint32 tick = read(file, "Time");

	_vm->setTick(tick);

	delete file;

	return true;
}

bool Savegame::saveData(Common::String filename, Common::String name, ChapterIndex chapter) {
	Common::OutSaveFile *file = g_system->getSavefileManager()->openForSaving(getFilename(_index));
	if (!file)
		return false;

	write(file, chapter, "Level");
	write(file, name, SAVEGAME_NAME_SIZE, "Game Name");
	writeHeader(file);
	write(file, _vm, 1512, 1, "Game Stats");
	write(file, getWorld(), 951928, 1, "World Stats");
	write(file, getPuzzleData(), 752, 1, "Blowup Puzzle Data");
	write(file, getEncounter()->items(), 109, getEncounter()->items()->size(), "Encounter Data");
	write(file, getEncounter()->variables(), 2, getEncounter()->variables()->size(), "Encounter Variables");

	if (getWorld()->numScripts)
		write(file, getScript(), 7096, getWorld()->numScripts, "Action Lists");

	write(file, _vm->getTick(), "Time");

	delete file;

	return true;
}

void Savegame::seek(Common::InSaveFile *file, uint32 offset, Common::String description) {
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

uint32 Savegame::read(Common::InSaveFile *file, Common::String description) {
	uint32 size = file->readUint32LE();
	uint32 count = file->readUint32LE();

	if (size * count == 0)
		return 0;

	return file->readUint32LE();
}

Common::String Savegame::read(Common::InSaveFile *file, uint32 strLength, Common::String description) {
	/*uint32 size =*/ file->readUint32LE();
	uint32 count = file->readUint32LE();

	if (strLength > count)
		error("[Savegame::read] Count too large (asked: %d, present: %d)", strLength, count);

	char *str = new char[strLength + 1];
	memset(str, 0, strLength + 1);
	file->read(str, strLength);

	Common::String ret(str);

	delete str;

	return ret;
}

void Savegame::read(Common::InSaveFile *file, Common::Serializable *data, uint32 size, uint32 count, Common::String description) {
	uint32 fileSize = file->readUint32LE();
	if (size > fileSize)
		error("[Savegame::read] Size too large (asked: %d, present: %d)", size, fileSize);

	uint32 fileCount = file->readUint32LE();
	if (count > fileCount)
		error("[Savegame::read] Count too large (asked: %d, present: %d)", count, fileCount);

	if (fileCount * fileSize == 0)
		return;

	Common::Serializer ser(file, NULL);
	data->saveLoadWithSerializer(ser);
}

void Savegame::write(Common::OutSaveFile *file, uint32 val, Common::String description) {
	file->writeUint32LE(4);
	file->writeUint32LE(1);

	// Write data
	// TODO check for errors
	file->writeUint32LE(val);
}

void Savegame::write(Common::OutSaveFile *file, Common::String val, uint32 count, Common::String description) {
	file->writeUint32LE(1);
	file->writeUint32LE(count);

	// Write data
	// TODO check for errors
	file->writeString(val);
}

void Savegame::write(Common::OutSaveFile *file, Common::Serializable *data, uint32 size, uint32 count, Common::String description) {
	file->writeUint32LE(size);
	file->writeUint32LE(count);

	if (size * count == 0)
		return;

	Common::Serializer ser(NULL, file);
	data->saveLoadWithSerializer(ser);
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
		Common::OutSaveFile *movies = g_system->getSavefileManager()->openForSaving(SAVEGAME_MOVIES);
		if (!movies)
			error("[Savegame::setMovieViewed] Could not open viewed movie list!");

		movies->write((byte *)&_moviesViewed, sizeof(_moviesViewed));

		delete movies;
	}
}

uint32 Savegame::getMoviesViewed(int32 *movieList) {
	memset(movieList, -1, 196 * sizeof(int32));

	uint32 count = 0;

	for (uint32 i = 0; i < ARRAYSIZE(_moviesViewed); i++) {
		if (_moviesViewed[i]) {
			movieList[i] = i;
			++count;
		}
	}

	return count;
}

void Savegame::loadMoviesViewed() {
	if (!isSavegamePresent(SAVEGAME_MOVIES))
		return;

	// Load data from disk
	Common::InSaveFile *movies = g_system->getSavefileManager()->openForLoading(SAVEGAME_MOVIES);
	if (!movies)
		error("[Savegame::setMovieViewed] Could not open viewed movie list!");

	movies->read((byte *)&_moviesViewed, sizeof(_moviesViewed));

	delete movies;
}

//////////////////////////////////////////////////////////////////////////
// Accessors
//////////////////////////////////////////////////////////////////////////
void Savegame::setName(uint32 index, Common::String name) {
	if (index > ARRAYSIZE(_names) - 1)
		error("[Savegame::setName] Invalid index (was: %d, max: %d)", index, ARRAYSIZE(_names) - 1);

	_names[index] = name;
}

Common::String Savegame::getName(uint32 index) {
	if (index > ARRAYSIZE(_names) - 1)
		error("[Savegame::getName] Invalid index (was: %d, max: %d)", index, ARRAYSIZE(_names) - 1);

	return _names[index];
}

bool Savegame::hasSavegame(uint32 index) {
	if (index > ARRAYSIZE(_savegames) - 1)
		error("[Savegame::hasSavegame] Invalid index (was: %d, max: %d)", index, ARRAYSIZE(_savegames) - 1);

	return _savegames[index];
}

} // End of namespace Asylum
