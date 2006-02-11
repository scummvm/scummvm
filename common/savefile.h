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
class InSaveFile : public Common::ReadStream {
public:
	virtual ~InSaveFile() {}

	/**
	 * Skip over the specified (positive) amount of bytes in the input stream.
	 */
	virtual void skip(uint32 offset) = 0;
};

/**
 * A class which allows game engines to save game state data.
 * That typically means "save games", but also includes things like the
 * IQ points in Indy3.
 */
class OutSaveFile : public Common::WriteStream {
public:
	virtual ~OutSaveFile() {}
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
	 * @return pointer to a SaveFile object, or NULL if an error occured.
	 */
	virtual OutSaveFile *openForSaving(const char *filename) = 0;

	/**
	 * Open the file with name filename in the given directory for loading.
	 * @param filename	the filename
	 * @return pointer to a SaveFile object, or NULL if an error occured.
	 */
	virtual InSaveFile *openForLoading(const char *filename) = 0;

	virtual void listSavefiles(const char * /* prefix */, bool *marks, int num) = 0;

	/** Get the path to the save game directory. */
	virtual const char *getSavePath() const;
};

class DefaultSaveFileManager : public SaveFileManager {
public:
	virtual OutSaveFile *openForSaving(const char *filename);
	virtual InSaveFile *openForLoading(const char *filename);
	virtual void listSavefiles(const char * /* prefix */, bool *marks, int num);

protected:
	SaveFile *makeSaveFile(const char *filename, bool saveOrLoad);
};

} // End of namespace Common

#endif
