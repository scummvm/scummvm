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

#ifndef COMMON_SAVEFILE_H
#define COMMON_SAVEFILE_H

#include "common/noncopyable.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/str-array.h"
#include "common/error.h"

namespace Common {

/**
 * @defgroup common_savefile Save files
 * @ingroup common
 *
 * @brief API for managing save files.
 *
 * @{
 */

/**
 * A class which allows game engines to load game state data.
 * This typically means "save games", but also includes things like the
 * IQ points in Indy3.
 */
typedef SeekableReadStream InSaveFile;

/**
 * A class which allows game engines to save game state data.
 * That typically means "save games", but also includes things like the
 * IQ points in Indy3.
 */
class OutSaveFile: public WriteStream {
protected:
	WriteStream *_wrapped; /*!< @todo Doc required. */

public:
	OutSaveFile(WriteStream *w); /*!< Create an OutSaveFile that uses the given WriteStream to write the data. */
	virtual ~OutSaveFile();

    /**
	 * Return true if an I/O failure occurred.
	 * This flag is never cleared automatically. In order to clear it,
	 * you must call clearErr() explicitly.
	 */
	virtual bool err() const;
	
    /**
	 * Reset the I/O error status as returned by err().
	 */
	virtual void clearErr();
	
	/**
	 * Finalize and close this stream. To be called right before this
	 * stream instance is deleted. The goal here is to enable calling
	 * code to detect and handle I/O errors which might occur when
	 * closing (and flushing, if buffered) the stream.
	 *
	 * After this method has been called, no further writes may be
	 * performed on the stream. Calling err() is allowed.
	 *
	 * By default, this just flushes the stream.
	 */
	virtual void finalize();

	/**
	 * Commit any buffered data to the underlying channel or
	 * storage medium. Unbuffered streams can use the default
	 * implementation
	 */
	virtual bool flush();

	/**
     * Write data into the stream.
	 *
	 * @param dataPtr	Pointer to the data to be written.
	 * @param dataSize	Number of bytes to be written.
     */
	virtual uint32 write(const void *dataPtr, uint32 dataSize);

	/**
	* Obtain the current value of the stream position indicator of the
	* stream.
	*
	* @return The current position indicator, or -1 if an error occurred.
	 */
	virtual int32 pos() const;
};

/**
 * The SaveFileManager serves as a factory for InSaveFile
 * and OutSaveFile objects.
 *
 * Engines and other code should use SaveFiles whenever they need to
 * store data that they need to retrieve again later on --
 * i.e. typically save states, but also configuration files and similar
 * objects.
 *
 * Save file names represent SaveFiles. These names are case insensitive. That
 * means a name of "Kq1.000" represents the same save file as "kq1.000". In
 * addition, SaveFileManager does not allow for names that contain path
 * separators like '/' or '\'. This is because directories are not supported
 * in SaveFileManager.
 *
 * While not declared as a singleton, it is effectively used as such,
 * with OSystem::getSavefileManager returning a pointer to single
 * SaveFileManager instances to be used.
 */
class SaveFileManager : NonCopyable {

protected:
	Error _error;      /*!< Error code. */
	String _errorDesc; /*!< Description of an error. */

	/**
	 * Set some information about the last error that occurred.
	 * @param error     Code identifying the last error.
	 * @param errorDesc String describing the last error.
	 */
	virtual void setError(Error error, const String &errorDesc) { _error = error; _errorDesc = errorDesc; }

public:
	virtual ~SaveFileManager() {}

	/**
	 * Clear the last set error code and string.
	 */
	virtual void clearError() { _error = kNoError; _errorDesc.clear(); }

	/**
	 * Return the last occurred error code. If none occurred, return kNoError.
	 *
	 * @return A value indicating the type of the last error.
	 */
	virtual Error getError() { return _error; }

	/**
	 * Return the last occurred error description. If none occurred, return 0.
	 *
	 * @return A string describing the last error.
	 */
	virtual String getErrorDesc() { return _errorDesc; }

	/**
	 * Return the last occurred error description. If none occurred, return 0.
	 * Also, clear the last error state and description.
	 *
	 * @return A string describing the last error.
	 */
	virtual String popErrorDesc();

	/**
	 * Open the save file with the specified @p name in the given directory for
	 * saving.
	 *
	 * Saved games are compressed by default, and engines are expected to
	 * always write compressed saves.
	 *
	 * A notable exception is when uncompressed files are needed for
	 * compatibility with games not supported by ScummVM, such as character
	 * exports from the Quest for Glory series. QfG5 is a 3D game and will not be
	 * supported by ScummVM.
	 *
	 * @param name      Name of the save file.
	 * @param compress  Whether to compress the resulting save file (default) or not.
	 * 
	 * @return Pointer to an OutSaveFile, or NULL if an error occurred.
	 */
	virtual OutSaveFile *openForSaving(const String &name, bool compress = true) = 0;

	/**
	 * Open the file with the specified @p name in the given directory for loading.
	 *
	 * @param name  Name of the save file.
	 * @return Pointer to an InSaveFile, or NULL if an error occurred.
	 */
	virtual InSaveFile *openForLoading(const String &name) = 0;

	/**
	* Open the file with the specified name in the given directory for loading.
	* In contrast to openForLoading(), it returns a raw file instead of unpacked.
	*
	* @param name  Name of the save file.
	* @return Pointer to an InSaveFile, or NULL if an error occurred.
	*/
	virtual InSaveFile *openRawFile(const String &name) = 0;

	/**
	 * Remove the given save file from the system.
	 *
	 * @param name  Name of the save file to be removed.
	 * @return True if no error occurred, false otherwise.
	 */
	virtual bool removeSavefile(const String &name) = 0;

	/**
	 * Rename the given save file.
	 *
	 * @param oldName   Old name.
	 * @param newName   New name.
	 * @param compress  Whether to compress the resulting save file (default) or not.
	 * 
	 * @return True if no error occurred, false otherwise.
	 */
	virtual bool renameSavefile(const String &oldName, const String &newName, bool compress = true);

	/**
	 * Copy the given save file.
	 *
	 * @param oldName   Old name.
	 * @param newName   New name.
	 * @param compress  Whether to compress the resulting save file (default) or not.
	 * 
	 * @return true if no error occurred. false otherwise.
	 */
	virtual bool copySavefile(const String &oldName, const String &newName, bool compress = true);

	/**
	 * List available save files matching a given pattern.
	 *
	 * The pattern format is based on DOS patterns, also known as "glob" in the
	 * POSIX world. Refer to the Common::matchString() function for information
	 * about the precise pattern format.
	 *
	 * @param pattern  Pattern to match. Wildcards like * or ? are allowed.
	 * @return List of strings for all present file names.
	 * @sa Common::matchString()
	 */
	virtual StringArray listSavefiles(const String &pattern) = 0;

	/**
	 * Refresh the save files list (because some new files might have been added)
	 * and remember the "locked" files list. These files cannot be used
	 * for saving or loading because they are being synced by CloudManager.
	 */
	virtual void updateSavefilesList(StringArray &lockedFiles) = 0;
};

/** @} */

} // End of namespace Common

#endif
