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

#include "common/rect.h"
#include "graphics/cursorman.h"

#include "common/formats/winexe_ne.h"
#include "common/formats/winexe_pe.h"

#include "common/macresman.h"

#include "graphics/maccursor.h"
#include "graphics/wincursor.h"

#include "private/private.h"

#include "engines/advancedDetector.h"

namespace Private {

struct CursorEntry {
	const char *name;
 	const char *aname;
	uint id;
};

void PrivateEngine::loadCursors() {
	if (_platform == Common::kPlatformWindows) {
		const CursorEntry cursorIDReference[] = {
			{ "kTurnLeft",  "k1", 23 },
			{ "kTurnRight", "k2", 9  },
			{ "kZoomIn",    "k3", 17 },
			{ "kZoomOut",   "k4", 11 },
			{ "kExit",      "k5", 7  },
			{ "kPhone",     "k6", 25 },
			{ "kInventory", "k7", 19 },
			{ nullptr, nullptr,   0  }
		};

		if (_installerArchive.open("SUPPORT/PVTEYE.Z")) {
			Common::SharedPtr<Common::WinResources> exe(Common::WinResources::createFromEXE(_installerArchive.createReadStreamForMember("PVTEYE.EXE")));
			
			if (exe == nullptr) {
				error("Executable not found");
			}
	
			const Common::Array<Common::WinResourceID> cursorIDs = exe->getIDList(Common::kWinGroupCursor);
	
			_cursors.resize(cursorIDs.size());
			for (uint i = 0; i < cursorIDs.size(); i++) {
				_cursors[i].winCursorGroup = Graphics::WinCursorGroup::createCursorGroup(exe.get(), cursorIDs[i]);
				_cursors[i].cursor = _cursors[i].winCursorGroup->cursors[0].cursor;
	
				const CursorEntry *entry = cursorIDReference;
				while (entry->name != nullptr) {
					if (entry->id == _cursors[i].winCursorGroup->cursors[0].id.getID()) {
						_cursors[i].name = entry->name;
						_cursors[i].aname = entry->aname;
						break;
					}
					entry++;
				}
			}
		}
	} else {
		const CursorEntry cursorIDReference[] = {
			{ "kTurnLeft",  "k1", 133 },
			{ "kTurnRight", "k2", 132 },
			{ "kZoomIn",    "k3", 138 },
			{ "kZoomOut",   "k4", 135 },
			{ "kExit",      "k5", 130 },
			{ "kPhone",     "k6", 141 },
			{ "kInventory", "k7", 139 },
			{ nullptr, nullptr,   0   }
		};

		Common::MacResManager resMan;

		Common::String path = isDemo() ? "SUPPORT/Private Eye Demo" : "SUPPORT/Private Eye";
		if (resMan.open(path.c_str())) {
			const Common::MacResIDArray cursorResIDs = resMan.getResIDArray(MKTAG('C', 'U', 'R', 'S'));
			_cursors.resize(cursorResIDs.size());

			for (uint i = 0; i < cursorResIDs.size(); i++) {
				Common::SharedPtr<Common::SeekableReadStream> resData(resMan.getResource(MKTAG('C', 'U', 'R', 'S'), cursorResIDs[i]));
				Graphics::MacCursor *cursor = new Graphics::MacCursor();
				cursor->readFromStream(*resData);
				_cursors[i].cursor = cursor;
				_cursors[i].winCursorGroup = nullptr;

				const CursorEntry *entry = cursorIDReference;
				while (entry->name != nullptr) {
					if (entry->id == cursorResIDs[i]) {
						_cursors[i].name = entry->name;
						_cursors[i].aname = entry->aname;
						break;
					}
					entry++;
				}
			}
		}
	}
}

void PrivateEngine::changeCursor(const Common::String &cursor) {
	if (_currentCursor == cursor) {
		return;
	}

	if (cursor == "default") {
		CursorMan.replaceCursor(Graphics::makeDefaultWinCursor());
	} else {
		for (uint i = 0; i < _cursors.size(); i++) {
			if (_cursors[i].name == cursor || _cursors[i].aname == cursor) {
				CursorMan.replaceCursor(_cursors[i].cursor);
				break;
			}
		}
	}

	_currentCursor = cursor;

	CursorMan.showMouse(true);
}

} // End of namespace Private
