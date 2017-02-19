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

#include "common/file.h"
#include "common/memstream.h"
#include "titanic/support/files_manager.h"
#include "titanic/game_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

CFilesManager::CFilesManager(TitanicEngine *vm) : _vm(vm), _gameManager(nullptr),
		_assetsPath("Assets"), _drive(-1) {
	loadResourceIndex();
}

CFilesManager::~CFilesManager() {
	_datFile.close();
}

void CFilesManager::loadResourceIndex() {
	if (!_datFile.open("titanic.dat"))
		error("Could not find titanic.dat data file");

	uint headerId = _datFile.readUint32BE();
	uint version = _datFile.readUint16LE();
	if (headerId != MKTAG('S', 'V', 'T', 'N') || version < 1)
		error("Invalid data file");

	// Read in entries
	uint offset, size;
	char c;
	Common::String resourceName;
	for (;;) {
		offset = _datFile.readUint32LE();
		size = _datFile.readUint32LE();
		if (offset == 0 && size == 0)
			break;

		Common::String resName;
		while ((c = _datFile.readByte()) != '\0')
			resName += c;

		_resources[resName] = ResourceEntry(offset, size);
	}
}

bool CFilesManager::fileExists(const CString &name) {
	Common::File f;
	return f.exists(name);
}

bool CFilesManager::scanForFile(const CString &name) {
	if (name.empty())
		return false;

	CString filename = name;
	filename.toLowercase();

	if (filename[0] == 'y' || filename[0] == 'z')
		return true;
	else if (filename[0] < 'a' || filename[0] > 'c')
		return false;

	CString fname = filename;
	int idx = fname.indexOf('#');
	if (idx >= 0) {
		fname = fname.left(idx);
		fname += ".st";
	}

	// Return true if the file exists
	if (fileExists(fname))
		return true;

	// Couldn't find file. Start by calling the game manager's viewChange
	// method, which handles all active scene objects freeing their resources
	if (_gameManager)
		_gameManager->viewChange();

	return false;
}

void CFilesManager::loadDrive() {
	assert(_drive == -1);
	resetView();
}

void CFilesManager::insertCD(CScreenManager *screenManager) {
	// We don't support running the game directly from the original CDs,
	// so this method can remain stubbed
}

void CFilesManager::resetView() {
	if (_gameManager) {
		_gameManager->_gameState.setMode(GSMODE_INTERACTIVE);
		_gameManager->markAllDirty();
	}
}

void CFilesManager::preload(const CString &name) {
	// We don't currently do any preloading of resources
}

Common::SeekableReadStream *CFilesManager::getResource(const CString &str) {
	ResourceEntry resEntry = _resources[str];

	// If we're running the German version, check for the existance of
	// a German specific version of the given resource
	if (_vm->isGerman() && _resources.contains(str + "/DE"))
		resEntry = _resources[str + "/DE"];

	_datFile.seek(resEntry._offset);

	return (resEntry._size > 0) ? _datFile.readStream(resEntry._size) :
		new Common::MemoryReadStream(nullptr, 0);
}

} // End of namespace Titanic
