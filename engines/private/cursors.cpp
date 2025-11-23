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

#include "common/compression/installshieldv3_archive.h"

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
	uint japaneseId;
};

void PrivateEngine::loadCursors() {
	_defaultCursor = Graphics::makeDefaultWinCursor();

	if (_platform == Common::kPlatformWindows) {
		const CursorEntry cursorIDReference[] = {
			{ "kTurnLeft",  "k1", 23, 17 },
			{ "kTurnRight", "k2", 9,  3  },
			{ "kZoomIn",    "k3", 17, 11 },
			{ "kZoomOut",   "k4", 11, 5  },
			{ "kExit",      "k5", 7,  1  },
			{ "kPhone",     "k6", 25, 19 },
			{ "kInventory", "k7", 19, 13 },
			{ nullptr, nullptr,   0,  0  }
		};

		Common::WinResources *exe = nullptr;
		Common::SeekableReadStream *exeStream = nullptr;
		Common::ArchiveMemberList members;
		Common::InstallShieldV3 installerArchive;
		if (installerArchive.open("SUPPORT/PVTEYE.Z")) {
			const char *exeNames[] = {
				"PVTEYE.EXE",
				"PvteyeJ.EXE", // Japan
				"PVTDEMO.EXE"
			};
			for (uint i = 0; i < ARRAYSIZE(exeNames) && exeStream == nullptr; i++) {
				exeStream = installerArchive.createReadStreamForMember(exeNames[i]);
			}
			if (exeStream == nullptr) {
				error("Executable not found in PVTEYE.Z");
			}
		} else {
			Common::File *file = new Common::File();
			if (!file->open("SUPPORT/PVTEYE.EX_")) {
				error("PVTEYE.EX_ not found");
			}
			exeStream = file;
		}

		exe = Common::WinResources::createFromEXE(exeStream);
		if (exe == nullptr) {
			error("Executable not found");
		}

		const Common::Array<Common::WinResourceID> cursorIDs = exe->getIDList(Common::kWinGroupCursor);

		_cursors.resize(cursorIDs.size());
		assert(cursorIDs.size() > 0);
		for (uint i = 0; i < cursorIDs.size(); i++) {
			_cursors[i].winCursorGroup = Graphics::WinCursorGroup::createCursorGroup(exe, cursorIDs[i]);
			_cursors[i].cursor = _cursors[i].winCursorGroup->cursors[0].cursor;

			const CursorEntry *entry = cursorIDReference;
			while (entry->name != nullptr) {
				uint entryId = (_language == Common::JA_JPN) ? entry->japaneseId : entry->id;
				if (entryId == _cursors[i].winCursorGroup->cursors[0].id.getID()) {
					_cursors[i].name = entry->name;
					_cursors[i].aname = entry->aname;
					break;
				}
				entry++;
			}
		}

		delete exe;
		delete exeStream;
	} else {
		const CursorEntry cursorIDReference[] = {
			{ "kTurnLeft",  "k1", 133, 0 },
			{ "kTurnRight", "k2", 132, 0 },
			{ "kZoomIn",    "k3", 138, 0 },
			{ "kZoomOut",   "k4", 135, 0 },
			{ "kExit",      "k5", 130, 0 },
			{ "kPhone",     "k6", 141, 0 },
			{ "kInventory", "k7", 139, 0 },
			{ nullptr, nullptr,   0,   0 }
		};

		Common::MacResManager resMan;

		const char *executableFilePath = isDemo() ? "SUPPORT/Private Eye Demo" : "SUPPORT/Private Eye";
		const char *executableInstallerPath = isDemo() ? "Private Eye Demo" : "Private Eye";
		Common::ScopedPtr<Common::Archive> macInstaller(loadMacInstaller());
		if (resMan.open(executableFilePath) || (macInstaller && resMan.open(executableInstallerPath, *macInstaller))) {
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
		CursorMan.replaceCursor(_defaultCursor);
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
