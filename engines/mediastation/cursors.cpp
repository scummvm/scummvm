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

#include "common/system.h"
#include "common/file.h"
#include "common/formats/winexe_ne.h"
#include "common/formats/winexe_pe.h"
#include "graphics/cursorman.h"
#include "graphics/maccursor.h"
#include "graphics/wincursor.h"

namespace MediaStation {

void CursorManager::showCursor() {
	CursorMan.showMouse(true);
}

void CursorManager::hideCursor() {
	CursorMan.showMouse(false);
}

void CursorManager::setDefaultCursor() {
	Graphics::Cursor *cursor = Graphics::makeDefaultWinCursor();
	CursorMan.replaceCursor(cursor);
	delete cursor;
}

WindowsCursorManager::WindowsCursorManager(const Common::Path &appName) {
	loadCursors(appName);
}

WindowsCursorManager::~WindowsCursorManager() {
	for (auto it = _cursors.begin(); it != _cursors.end(); ++it) {
		delete it->_value;
	}
	_cursors.clear();
}

void WindowsCursorManager::loadCursors(const Common::Path &appName) {
	if (appName.empty()) {
		error("WindowsCursorManager::loadCursors(): No executable to load cursors from");
	} else if (!Common::File::exists(appName)) {
		error("WindowsCursorManager::loadCursors(): Executable %s doesn't exist", appName.toString().c_str());
	}

	Common::WinResources *exe = Common::WinResources::createFromEXE(appName);
	if (!exe->loadFromEXE(appName)) {
		error("WindowsCursorManager::loadCursors(): Could not load resources from executable %s", appName.toString().c_str());
	}

	const Common::Array<Common::WinResourceID> cursorGroups = exe->getIDList(Common::kWinGroupCursor);
	for (Common::WinResourceID cursorGroup : cursorGroups) {
		Common::String resourceString = cursorGroup.getString();
		if (resourceString.empty()) {
			warning("WindowsCursorManager::loadCursors(): Got Windows cursor group with no string ID");
			continue;
		}
		Graphics::WinCursorGroup *group = Graphics::WinCursorGroup::createCursorGroup(exe, cursorGroup);
		_cursors.setVal(resourceString, group);
	}
	delete exe;
}

void WindowsCursorManager::setCursor(const Common::String &name) {
	Graphics::WinCursorGroup *group = _cursors.getValOrDefault(name);
	if (group != nullptr) {
		Graphics::Cursor *cursor = group->cursors[0].cursor;
		CursorMan.replaceCursor(cursor);
	} else {
		error("WindowsCursorManager::setCursor(): Reqested Windows cursor %s not found", name.c_str());
	}
}

MacCursorManager::MacCursorManager(const Common::Path &appName) {
	if (appName.empty()) {
		error("MacCursorManager::loadCursors(): No file to load cursors from");
	} else if (!Common::File::exists(appName)) {
		error("MacCursorManager::loadCursors(): File %s doesn't exist", appName.toString().c_str());
	}

	_resFork = new Common::MacResManager();
	if (!_resFork->open(appName) || !_resFork->hasResFork()) {
		error("MacCursorManager::loadCursors(): Could not load resource fork from %s", appName.toString().c_str());
	}
}

MacCursorManager::~MacCursorManager() {
	delete _resFork;
	_resFork = nullptr;
}

void MacCursorManager::setCursor(const Common::String &name) {
	// Try to load a color cursor first.
	Common::SeekableReadStream *stream = _resFork->getResource(MKTAG('c', 'r', 's', 'r'), name);
	if (stream == nullptr) {
		// Fall back to attempting to load a mnochrome cursor.
		stream = _resFork->getResource(MKTAG('C', 'U', 'R', 'S'), name);
	}

	// Make sure we got a resource.
	if (stream == nullptr) {
		error("MacCursorManager::setCursor(): Reqested Mac cursor %s not found", name.c_str());
	}

	Graphics::MacCursor *macCursor = new Graphics::MacCursor();
	if (!macCursor->readFromStream(*stream)) {
		error("MacCursorManager::setCursor(): Error parsing cursor %s from stream", name.c_str());
	}
	CursorMan.replaceCursor(macCursor);

	delete macCursor;
	delete stream;
}

} // End of namespace MediaStation
