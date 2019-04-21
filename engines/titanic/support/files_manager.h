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

#ifndef TITANIC_FILES_MANAGER_H
#define TITANIC_FILES_MANAGER_H

#include "common/hashmap.h"
#include "titanic/core/list.h"
#include "titanic/support/screen_manager.h"

namespace Titanic {

enum ResourceFlag { FLAG_COMPRESSED = 1 };

class TitanicEngine;
class CGameManager;

class CFilesManagerList : public List<ListItem> {
};

class CFilesManager {
	struct ResourceEntry {
		uint _offset;
		uint _size;
		uint _flags;

		ResourceEntry() : _offset(0), _size(0), _flags(0) {}
		ResourceEntry(uint offset, uint size, uint flags) :
			_offset(offset), _size(size), _flags(flags) {}
	};
	typedef Common::HashMap<Common::String, ResourceEntry> ResourceHash;
private:
	TitanicEngine *_vm;
	CGameManager *_gameManager;
	Common::File _datFile;
	ResourceHash _resources;
	CFilesManagerList _list;
	int _drive;
	const CString _assetsPath;
	int _version;
public:
	CFilesManager(TitanicEngine *vm);
	~CFilesManager();

	/**
	 * Opens up the titanic.dat support file and loads it's index
	 */
	bool loadResourceIndex();

	/**
	 * Sets the game manager
	 */
	void setGameManager(CGameManager *gameManager) {
		_gameManager = gameManager;
	}

	/**
	 * Returns true if a file of the given name exists
	 */
	static bool fileExists(const CString &name);

	/**
	 * Scans for a file with a matching name
	 */
	bool scanForFile(const CString &name);

	/**
	 * Handles displaying a load drive view if necessary
	 */
	void loadDrive();

	/**
	 * Shows a dialog for inserting a new CD
	 */
	void insertCD(CScreenManager *screenManager);

	/**
	 * Resets the view being displayed
	 */
	void resetView();

	/**
	 * Preloads and caches a file for access shortly
	 */
	void preload(const CString &name);

	/**
	 * Get a resource from the executable
	 */
	Common::SeekableReadStream *getResource(const CString &str);
};

} // End of namespace Titanic

#endif /* TITANIC_FILES_MANAGER_H */
