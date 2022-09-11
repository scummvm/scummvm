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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if !defined(BACKEND_SAVES_DEFAULT_H) && !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)
#define BACKEND_SAVES_DEFAULT_H

#include "common/scummsys.h"
#include "common/savefile.h"
#include "common/str.h"
#include "common/fs.h"
#include "common/hash-str.h"

/**
 * Provides a default savefile manager implementation for common platforms.
 */
class DefaultSaveFileManager : public Common::SaveFileManager {
public:
	DefaultSaveFileManager();
	DefaultSaveFileManager(const Common::String &defaultSavepath);

	void updateSavefilesList(Common::StringArray &lockedFiles) override;
	Common::StringArray listSavefiles(const Common::String &pattern) override;
	Common::InSaveFile *openRawFile(const Common::String &filename) override;
	Common::InSaveFile *openForLoading(const Common::String &filename) override;
	Common::OutSaveFile *openForSaving(const Common::String &filename, bool compress = true) override;
	bool removeSavefile(const Common::String &filename) override;
	bool exists(const Common::String &filename) override;

#ifdef USE_LIBCURL

	static const uint32 INVALID_TIMESTAMP = UINT_MAX;
	static const char *TIMESTAMPS_FILENAME;

	static Common::HashMap<Common::String, uint32> loadTimestamps();
	static void saveTimestamps(Common::HashMap<Common::String, uint32> &timestamps);
#endif

	static Common::String concatWithSavesPath(Common::String name);

protected:
	/**
	 * Get the path to the savegame directory.
	 * Should only be used internally since some platforms
	 * might implement savefiles in a completely different way.
	 */
	virtual Common::String getSavePath() const;

	/**
	 * Checks the given path for read access, existence, etc.
	 * Sets the internal error and error message accordingly.
	 */
	virtual void checkPath(const Common::FSNode &dir);

	/**
	 * Removes the given file.
	 * This is called from removeSavefile() with the full file path.
	 */
	virtual Common::ErrorCode removeFile(const Common::String &filepath);

	/**
	 * Assure that the given save path is cached.
	 *
	 * @param savePathName  String representation of save path to cache.
	 */
	void assureCached(const Common::String &savePathName);

	typedef Common::HashMap<Common::String, Common::FSNode, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SaveFileCache;

	/**
	 * Cache of all the save files in the currently cached directory.
	 *
	 * Modify with caution because we only re-cache when the save path changed!
	 * This needs to be updated inside at least openForSaving and
	 * removeSavefile.
	 */
	SaveFileCache _saveFileCache;

	/**
	 * List of "locked" files. These cannot be used for saving/loading
	 * because CloudManager is downloading those.
	 */
	Common::StringArray _lockedFiles;

private:
	/**
	 * The currently cached directory.
	 */
	Common::String _cachedDirectory;
};

#endif
