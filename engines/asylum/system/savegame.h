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

#ifndef ASYLUM_SYSTEM_SAVEGAME_H
#define ASYLUM_SYSTEM_SAVEGAME_H

#include "common/savefile.h"
#include "common/serializer.h"
#include "common/util.h"

#include "asylum/shared.h"

namespace Asylum {

#define SAVEGAME_COUNT 25

class AsylumEngine;

class Savegame {
public:
	Savegame(AsylumEngine *engine);
	~Savegame() {};

	/**
	 * Checks if saved games are present
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool hasSavegames() const;

	/**
	 * Loads the list of saved games.
	 */
	void loadList();

	/**
	 * Loads a game
	 */
	void load();

	/**
	 * Quick loads a game
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool quickLoad();

	/**
	 * Saves a game
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	void save();

	/**
	 * Quick saves a game
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool quickSave();

	/**
	 * Removes a savegame
	 */
	void remove();

	/**
	 * Checks if a savegame is compatible
	 *
	 * @return true if it is, false otherwise.
	 */
	bool isCompatible();

	//////////////////////////////////////////////////////////////////////////
	// Static methods
	//////////////////////////////////////////////////////////////////////////
	/**
	 * Seeks to a specific place in the file
	 *
	 * @param [in,out] file If non-null, the file.
	 * @param offset 		Offset index of the info into the file
	 * @param description   The description.
	 */
	static void seek(Common::InSaveFile *file, uint32 offset, const Common::String &description);

	/**
	 * Reads data from a file.
	 *
	 * @param [in,out] file If non-null, the file.
	 * @param description   The description.
	 *
	 * @return the value
	 */
	static uint32 read(Common::InSaveFile *file, const Common::String &description);

	/**
	 * Reads data from a file.
	 *
	 * @param [in,out] file If non-null, the file.
	 * @param strLength 	Length of the string.
	 * @param description   The description.
	 *
	 * @return the string
	 */
	static Common::String read(Common::InSaveFile *file, uint32 strLength, const Common::String &description);

	/**
	 * Reads data from a file.
	 *
	 * @param [in,out] file If non-null, the file.
	 * @param [in,out] data If non-null, the data.
	 * @param size 			The size.
	 * @param count 		Number of.
	 * @param description   The description.
	 */
	static void read(Common::InSaveFile *file, Common::Serializable *data, uint32 size, uint32 count, const Common::String &description);

	/**
	 * Writes data to a file.
	 *
	 * @param [in,out] file If non-null, the file.
	 * @param val 			The value
	 * @param description   The description.
	 */
	static void write(Common::OutSaveFile *file, uint32 val, const Common::String &description);

	/**
	 * Writes data to a file.
	 *
	 * @param [in,out] file If non-null, the file.
	 * @param val 			The string
	 * @param strLength		The size of the string.
	 * @param description   The description.
	 */
	static void write(Common::OutSaveFile *file, const Common::String &val, uint32 strLength, const Common::String &description);

	/**
	 * Writes data to a file.
	 *
	 * @param [in,out] file If non-null, the file.
	 * @param [in,out] data If non-null, the data.
	 * @param size 			The size.
	 * @param count 		Number of.
	 * @param description   The description.
	 */
	static void write(Common::OutSaveFile *file, Common::Serializable *data, uint32 size, uint32 count, const Common::String &description);

	//////////////////////////////////////////////////////////////////////////
	// Movies
	//////////////////////////////////////////////////////////////////////////
	void setMovieViewed(uint32 index);
	uint32 getMoviesViewed(int32 *movieList) const;
	void loadMoviesViewed();

	//////////////////////////////////////////////////////////////////////////
	// Accessors
	//////////////////////////////////////////////////////////////////////////
	void setName(uint32 index, const Common::String &name);
	Common::String getName(uint32 index) const;

	Common::String *getName() { return &_names[_index]; }
	void setIndex(uint32 index) { _index = index; }
	uint32 getIndex() { return _index; }

	bool hasSavegame(uint32 index) const;
	ResourcePackId getScenePack() { return (ResourcePackId)(_savegameToScene[_index] + 4); }

	void resetVersion();
	const char *getVersion() { return _version.c_str(); }
	uint32 getBuild() { return _build; }

private:
	AsylumEngine *_vm;

	uint32 _index;
	byte _moviesViewed[196];
	uint32 _savegameToScene[SAVEGAME_COUNT];
	bool _savegames[SAVEGAME_COUNT];
	Common::String _names[SAVEGAME_COUNT];
	Common::String _version;
	uint32 _build;

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////


	/**
	 * Gets a filename for a given save index
	 *
	 * @param index Zero-based index of the savegame
	 *
	 * @return The filename.
	 */
	Common::String getFilename(uint32 index) const;

	/**
	 * Check if a specific savegame exists
	 *
	 * @param filename Filename of the file.
	 *
	 * @return true if savegame present, false if not.
	 */
	bool isSavegamePresent(const Common::String &filename) const;

	//////////////////////////////////////////////////////////////////////////
	// Reading & writing
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Reads a savegame header.
	 *
	 * @param [in,out] file If non-null, the file.
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool readHeader(Common::InSaveFile *file);

	/**
	 * Writes a savegame header.
	 *
	 * @param [in,out] file If non-null, the file.
	 */
	void writeHeader(Common::OutSaveFile *file) const;

	/**
	 * Loads savegame data
	 *
	 * @param filename Filename of the file.
	 */
	void loadData(const Common::String &filename);

	/**
	 * Save savegame data.
	 *
	 * @param filename       Filename of the file.
	 * @param name 	         The name.
	 * @param chapter        The chapter.
	 */
	void saveData(const Common::String &filename, const Common::String &name, ChapterIndex chapter);
};

} // End of namespace Asylum

#endif // ASYLUM_SYSTEM_SAVEGAME_H
