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
#include "common/str.h"
#include "common/error.h"

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
	
protected:
	SFMError _error;
	String _errorDesc;
	
public:
	virtual ~SaveFileManager() {}
	
	/**
	 * Clears the last set error code and string.
	 */
	virtual void clearError() { _error = SFM_NO_ERROR; _errorDesc = ""; }
	
	/**
	 * Returns the last ocurred error code. If none ocurred, returns SFM_NO_ERROR.
	 * 
	 * @return A SFMError indicating the type of the last error.
	 */
	virtual SFMError getError() { return _error; }
	
	/**
	 * Returns the last ocurred error description. If none ocurred, returns 0.
	 * 
	 * @return A string describing the last error.
	 */
	virtual String getErrorDesc() { return _errorDesc; }
	
	/**
	 * Sets the last ocurred error.
	 * @param error Code identifying the last error.
	 * @param errorDesc String describing the last error.
	 */
	virtual void setError(SFMError error, String errorDesc) { _error = error; _errorDesc = errorDesc; }
	
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
	 * Removes the given savefile from the filesystem.
	 * @param filename Filename path pointing to the savefile.
	 * @return true if no error ocurred. false otherwise.
	 */
	virtual bool removeSavefile(const char *filename) = 0;
	
	/**
	 * Request a list of available savegames with a given regex.
	 * @param regex Regular expression to match. Wildcards like * or ? are available.
	 * returns a list of strings for all present file names.
	 */
	virtual Common::StringList listSavefiles(const char *regex) = 0;

	/**
	 * Get the path to the save game directory.
	 * Should only be used for error messages, *never* to construct file paths
	 * from it, since that is highly unportable!
	 */
	virtual const char *getSavePath() const;
};

} // End of namespace Common

#endif
