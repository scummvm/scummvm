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

#ifndef ASYLUM_SAVEGAME_H
#define ASYLUM_SAVEGAME_H

#include "asylum/shared.h"

#include "common/savefile.h"
#include "common/serializer.h"
#include "common/util.h"

namespace Asylum {

#define SAVEGAME_COUNT 25

class AsylumEngine;

class Savegame {
public:
	Savegame(AsylumEngine *engine);
	~Savegame();

	/**
	 * Checks if saved games are present
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool hasSavegames();

	/**
	 * Loads the list of saved games.
	 */
	void loadList();

	/**
	 * Loads a game
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool load();

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

	//////////////////////////////////////////////////////////////////////////
	// Movies
	//////////////////////////////////////////////////////////////////////////
	void setMovieViewed(uint32 index);
	uint32 getMoviesViewed(byte *movieList);
	void loadMoviesViewed();

private:
	AsylumEngine* _vm;

	uint32 _index;
	byte _moviesViewed[196];
	uint32 _savegameToScene[SAVEGAME_COUNT];
	bool _savegames[SAVEGAME_COUNT];
	Common::String _names[SAVEGAME_COUNT];
	bool _valid;

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Checks if a savegame is valid
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool check();

	/**
	 * Gets a filename for a given save index
	 *
	 * @param index Zero-based index of the savegame
	 *
	 * @return The filename.
	 */
	Common::String getFilename(uint32 index);

	/**
	 * Check if a specific savegame exists
	 *
	 * @param filename Filename of the file.
	 *
	 * @return true if savegame present, false if not.
	 */
	bool isSavegamePresent(Common::String filename);

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
	void writeHeader(Common::OutSaveFile *file);

	/**
	 * Loads savegame data
	 *
	 * @param filename Filename of the file.
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool loadData(Common::String filename);

	/**
	 * Save savegame data.
	 *
	 * @param filename Filename of the file.
	 * @param name 	   The name.
	 * @param chapter  The chapter.
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool saveData(Common::String filename, Common::String name, ChapterIndex chapter);

	/**
	 * Seeks to a specific place in the file
	 *
	 * @param [in,out] file If non-null, the file.
	 * @param offset 		Offset index of the info into the file
	 * @param description   The description.
	 */
	void seek(Common::InSaveFile *file, uint32 offset, Common::String description);

	/**
	 * Reads data from a file. 
	 *
	 * @param [in,out] file If non-null, the file. 
	 * @param description   The description. 
	 *
	 * @return the value
	 */
	uint32 read(Common::InSaveFile *file, Common::String description);

	/**
	 * Reads data from a file. 
	 *
	 * @param [in,out] file If non-null, the file. 
	 * @param strLength 	Length of the string. 
	 * @param description   The description. 
	 *
	 * @return the string
	 */
	Common::String read(Common::InSaveFile *file, uint32 strLength, Common::String description);

	/**
	 * Reads data from a file.
	 *
	 * @param [in,out] file If non-null, the file.
	 * @param [in,out] data If non-null, the data.
	 * @param size 			The size.
	 * @param count 		Number of.
	 * @param description   The description.
	 */
	void read(Common::InSaveFile *file, Common::Serializable *data, uint32 size, uint32 count, Common::String description);

	/**
	 * Writes data to a file.
	 *
	 * @param [in,out] file If non-null, the file.
	 * @param val 			The value
	 * @param description   The description.
	 */
	void write(Common::OutSaveFile *file, uint32 val, Common::String description);

	/**
	 * Writes data to a file.
	 *
	 * @param [in,out] file If non-null, the file.
	 * @param val 			The string
	 * @param count 		The size of the string.
	 * @param description   The description.
	 */
	void write(Common::OutSaveFile *file, Common::String val, uint32 count, Common::String description);

	/**
	 * Writes data to a file.
	 *
	 * @param [in,out] file If non-null, the file.
	 * @param [in,out] data If non-null, the data.
	 * @param size 			The size.
	 * @param count 		Number of.
	 * @param description   The description.
	 */
	void write(Common::OutSaveFile *file, Common::Serializable *data, uint32 size, uint32 count, Common::String description);
};

} // End of namespace Asylum

#endif // ASYLUM_SAVEGAME_H
