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

#include "mediastation/cursors.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

#include "common/system.h"
#include "common/file.h"
#include "common/formats/winexe_ne.h"
#include "common/formats/winexe_pe.h"
#include "graphics/cursorman.h"

namespace MediaStation {

CursorManager::~CursorManager() {
	// It is up to the platform-specific cursor managers
	// to actually delete their resources.
	_cursors.clear();
}

bool CursorManager::attemptToReadFromStream(Chunk &chunk, uint param) {
	bool handledParam = true;
	switch (param) {
	case kCursorManagerInit:
		init(chunk);
		break;

	case kCursorManagerNewCursor:
		newCursor(chunk);
		break;

	case kCursorManagerDisposeCursor:
		disposeCursor(chunk);
		break;

	default:
		handledParam = false;
	}

	return handledParam;
}

void CursorManager::init(Chunk &chunk) {
	_baseCursorId = chunk.readTypedUint16();
	_maxCursorId = chunk.readTypedUint16();

	if (_maxCursorId < _baseCursorId || _baseCursorId == 0) {
		error("%s: Got invalid cursor IDs", __func__);
	}
}

void CursorManager::newCursor(Chunk &chunk) {
	CursorType cursorType = static_cast<CursorType>(chunk.readTypedUint16());
	uint16 cursorId = chunk.readTypedUint16();
	switch (cursorType) {
	case kPlatformCursor: {
		uint16 platformCursorId = chunk.readTypedUint16();
		newPlatformCursor(cursorId, platformCursorId);
		break;
	}

	case kResourceCursor: {
		// This first value isn't actually used.
		chunk.readTypedUint16();
		Common::String resourceName = chunk.readTypedFilename();
		newResourceCursor(cursorId, resourceName);
		break;
	}

	default:
		error("%s: Got unknown cursor type %d", __func__, static_cast<uint>(cursorType));
	}
}

void CursorManager::disposeCursor(Chunk &chunk) {
	uint16 cursorId = chunk.readTypedUint16();
	_cursors.erase(cursorId);
	// We don't actually delete the underlying platform-specific
	// cursor, just remove it from the hashmap. Otherwise, we'd
	// mess up the platform-specific storage.
}

void CursorManager::newPlatformCursor(uint16 platformCursorId, uint16 cursorId) {
	if (cursorId < _baseCursorId || cursorId > _maxCursorId || cursorId == 0) {
		error("%s: Got invalid cursor ID %d", __func__, static_cast<uint>(cursorId));
	}

	warning("STUB: %s: Platform cursor %d, internal cursor %d", __func__, platformCursorId, cursorId);
	// TODO: To implement this, we need have the default platform cursors for Windows and Mac.
}

void CursorManager::newResourceCursor(uint16 cursorId, const Common::String &resourceName) {
	if (cursorId < _baseCursorId || cursorId > _maxCursorId || cursorId == 0) {
		error("%s: Got invalid cursor ID %d", __func__, static_cast<uint>(cursorId));
	}

	Graphics::Cursor *cursor = loadResourceCursor(resourceName);
	_cursors.setVal(cursorId, cursor);
}

void CursorManager::showCursor() {
	CursorMan.showMouse(true);
}

void CursorManager::hideCursor() {
	CursorMan.showMouse(false);
}

void CursorManager::registerAsPermanent(uint16 id) {
	if (id != 0) {
		_permanentCursorId = id;
	}
}

void CursorManager::setAsPermanent(uint16 id) {
	bool cursorAlreadySet = _currentCursorId == id && _permanentCursorId == id;
	bool cursorIsEmpty = id == 0;
	if (cursorAlreadySet || cursorIsEmpty) {
		return;
	}

	_permanentCursorId = id;
	_currentCursorId = id;
	resetCurrent();
}

void CursorManager::setAsTemporary(uint16 id) {
	bool cursorAlreadySet = _currentCursorId == id;
	bool cursorIsEmpty = id == 0;
	if (cursorAlreadySet || cursorIsEmpty) {
		return;
	}

	_currentCursorId = id;
	resetCurrent();
}

void CursorManager::unsetPermanent() {
	_permanentCursorId = 0;
	_currentCursorId = 0;
}

void CursorManager::unsetTemporary() {
	if (_currentCursorId != _permanentCursorId) {
		_currentCursorId = _permanentCursorId;
		resetCurrent();
	}
}

void CursorManager::resetCurrent() {
	if (_currentCursorId != 0) {
		Graphics::Cursor *cursor = _cursors.getVal(_currentCursorId);
		CursorMan.replaceCursor(cursor);
	}
}

void CursorManager::setDefaultCursor() {
	Graphics::Cursor *cursor = Graphics::makeDefaultWinCursor();
	CursorMan.replaceCursor(cursor);
	delete cursor;
}

WindowsCursorManager::WindowsCursorManager(const Common::Path &appName) : CursorManager(appName) {
	if (appName.empty()) {
		error("%s: No executable to load cursors from", __func__);
	} else if (!Common::File::exists(appName)) {
		error("%s: Executable %s doesn't exist", __func__, appName.toString().c_str());
	}

	Common::WinResources *exe = Common::WinResources::createFromEXE(appName);
	if (!exe->loadFromEXE(appName)) {
		error("%s: Could not load resources from executable %s", __func__, appName.toString().c_str());
	}

	const Common::Array<Common::WinResourceID> cursorGroups = exe->getIDList(Common::kWinGroupCursor);
	for (Common::WinResourceID cursorGroup : cursorGroups) {
		Common::String resourceString = cursorGroup.getString();
		if (resourceString.empty()) {
			warning("%s: Got Windows cursor group with no string ID", __func__);
			continue;
		}
		Graphics::WinCursorGroup *group = Graphics::WinCursorGroup::createCursorGroup(exe, cursorGroup);
		_cursorGroups.setVal(resourceString, group);
	}
	delete exe;
}

WindowsCursorManager::~WindowsCursorManager() {
	for (auto it = _cursorGroups.begin(); it != _cursorGroups.end(); ++it) {
		delete it->_value;
	}
	_cursorGroups.clear();

	// We don't need to delete items in _cursors itself,
	// because those cursors are part of _cursorGroups.
}

Graphics::Cursor *WindowsCursorManager::loadResourceCursor(const Common::String &name) {
	Graphics::WinCursorGroup *group = _cursorGroups.getValOrDefault(name);
	if (group != nullptr) {
		Graphics::Cursor *cursor = group->cursors[0].cursor;
		return cursor;
	} else {
		error("%s: Reqested Windows cursor %s not found", __func__, name.c_str());
	}
}

MacCursorManager::MacCursorManager(const Common::Path &appName) : CursorManager(appName) {
	if (appName.empty()) {
		error("%s: No file to load cursors from", __func__);
	} else if (!Common::File::exists(appName)) {
		error("%s: File %s doesn't exist", __func__, appName.toString().c_str());
	}

	_resFork = new Common::MacResManager();
	if (!_resFork->open(appName) || !_resFork->hasResFork()) {
		error("%s: Could not load resource fork from %s", __func__, appName.toString().c_str());
	}
}

MacCursorManager::~MacCursorManager() {
	for (auto it = _cursors.begin(); it != _cursors.end(); ++it) {
		delete it->_value;
	}

	delete _resFork;
	_resFork = nullptr;
}

Graphics::Cursor *MacCursorManager::loadResourceCursor(const Common::String &name) {
	// Try to load a color cursor first.
	Common::SeekableReadStream *stream = _resFork->getResource(MKTAG('c', 'r', 's', 'r'), name);
	if (stream == nullptr) {
		// Fall back to attempting to load a mnochrome cursor.
		stream = _resFork->getResource(MKTAG('C', 'U', 'R', 'S'), name);
	}

	// Make sure we got a resource.
	if (stream == nullptr) {
		error("%s: Reqested Mac cursor %s not found", __func__, name.c_str());
	}

	Graphics::MacCursor *macCursor = new Graphics::MacCursor();
	if (!macCursor->readFromStream(*stream)) {
		error("%s: Error parsing cursor %s from stream", __func__, name.c_str());
	}
	delete stream;
	return macCursor;
}

} // End of namespace MediaStation
