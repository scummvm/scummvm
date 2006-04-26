/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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

#ifndef COMMON_SAVEFILE_H
#define COMMON_SAVEFILE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/stream.h"

namespace Common {

/**
 * A class which allows game engines to load game state data.
 * That typically means "save games", but also includes things like the
 * IQ points in Indy3.
 */
class InSaveFile : public SeekableReadStream {
};

/**
 * A class which allows game engines to save game state data.
 * That typically means "save games", but also includes things like the
 * IQ points in Indy3.
 */
class OutSaveFile : public WriteStream {
};

/**
 * Convenience intermediate class, to be removed.
 */
class SaveFile : public InSaveFile, public OutSaveFile {
public:
};

class SaveFileManager {

public:
	virtual ~SaveFileManager() {}

	/**
	 * Open the file with name filename in the given directory for saving.
	 * @param filename	the filename
	 * @return pointer to an OutSaveFile, or NULL if an error occured.
	 */
	virtual OutSaveFile *openForSaving(const char *filename) = 0;

	/**
	 * Open the file with name filename in the given directory for loading.
	 * @param filename	the filename
	 * @return pointer to an InSaveFile, or NULL if an error occured.
	 */
	virtual InSaveFile *openForLoading(const char *filename) = 0;

	/**
	 * Request a list of available savegames with a given prefix.
	 * TODO: Document this better!
	 * TODO: Or even replace it with a better API. For example, one that
	 * returns a list of strings for all present file names. 
	 */
	virtual void listSavefiles(const char * /* prefix */, bool *marks, int num) = 0;

	/**
	 * Get the path to the save game directory.
	 * Should only be used for error messages, and not to construct file paths
	 * from it, since that is highl unportable.
	 */
	virtual const char *getSavePath() const;
};

class DefaultSaveFileManager : public SaveFileManager {
public:
	virtual OutSaveFile *openForSaving(const char *filename);
	virtual InSaveFile *openForLoading(const char *filename);
	virtual void listSavefiles(const char * /* prefix */, bool *marks, int num);
};

} // End of namespace Common

#endif
