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

#ifndef COMMON_SAVEFILE_H
#define COMMON_SAVEFILE_H

#include "common/stdafx.h"
#include "common/noncopyable.h"
#include "common/scummsys.h"
#include "common/stream.h"

namespace Common {

/**
 * A class which allows game engines to load game state data.
 * That typically means "save games", but also includes things like the
 * IQ points in Indy3.
 */
class InSaveFile : public SeekableReadStream {};

/**
 * A class which allows game engines to save game state data.
 * That typically means "save games", but also includes things like the
 * IQ points in Indy3.
 */
class OutSaveFile : public WriteStream {
public:
	/**
	 * Close this savefile, to be called right before destruction of this
	 * savefile. The idea is that this ways, I/O errors that occur
	 * during closing/flushing of the file can still be handled by the
	 * game engine.
	 *
	 * By default, this just flushes the stream.
	 */
	virtual void finalize() {
		flush();
	}
};


/**
 * The SaveFileManager is serving as a factor for InSaveFile
 * and OutSaveFile objects.
 *
 * Engines and other code should use SaveFiles whenever they need to
 * store data which they need to be able to retrieve later on --
 * i.e. typically save states, but also configuration files and similar
 * things.
 *
 * While not declared as a singleton,
 * it is effectively used as such, with OSystem::getSavefileManager
 * returning the single SaveFileManager instances to be used.
 */
class SaveFileManager : NonCopyable {

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
	 * Should only be used for error messages, *never* to construct file paths
	 * from it, since that is highly unportable!
	 */
	virtual const char *getSavePath() const;
};

} // End of namespace Common

#endif
