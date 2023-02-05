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

#include "common/formats/winexe.h"
#include "common/ptr.h"
#include "common/system.h"

#include "graphics/cursorman.h"
#include "graphics/wincursor.h"
#include "graphics/managed_surface.h"

#include "vcruise/runtime.h"



namespace VCruise {

Runtime::Runtime(OSystem *system) : _system(system), _roomNumber(1), _gameState(kGameStateBoot) {
}

Runtime::~Runtime() {
}

void Runtime::loadCursors(const char *exeName) {
	Common::SharedPtr<Common::WinResources> winRes(Common::WinResources::createFromEXE(exeName));
	if (!winRes)
		error("Couldn't open executable file %s", exeName);

	Common::Array<Common::WinResourceID> cursorGroupIDs = winRes->getIDList(Common::kWinGroupCursor);
	for (Common::Array<Common::WinResourceID>::const_iterator it = cursorGroupIDs.begin(), itEnd = cursorGroupIDs.end(); it != itEnd; ++it) {
		const Common::WinResourceID &id = *it;

		Common::SharedPtr<Graphics::WinCursorGroup> cursorGroup(Graphics::WinCursorGroup::createCursorGroup(winRes.get(), *it));
		if (!winRes) {
			warning("Couldn't load cursor group");
			continue;
		}

		Common::String nameStr = id.getString();
		if (nameStr.size() == 8 && nameStr.substr(0, 7) == "CURSOR_") {
			char c = nameStr[7];
			if (c >= '0' && c <= '9') {
				uint shortID = c - '0';
				if (shortID >= _cursorsShort.size())
					_cursorsShort.resize(shortID + 1);
				_cursorsShort[shortID] = cursorGroup;
			}
		} else if (nameStr.size() == 13 && nameStr.substr(0, 11) == "CURSOR_CUR_") {
			char c1 = nameStr[11];
			char c2 = nameStr[12];
			if (c1 >= '0' && c1 <= '9' && c2 >= '0' && c2 <= '9') {
				uint longID = (c1 - '0') * 10 + (c2 - '0');
				if (longID >= _cursors.size())
					_cursors.resize(longID + 1);
				_cursors[longID] = cursorGroup;
			}
		}
	}
}

bool Runtime::runFrame() {
	bool moreActions = true;
	while (moreActions) {
		moreActions = false;
		switch (_gameState) {
		case kGameStateBoot:
			bootGame();
			break;
		case kGameStateQuit:
			return false;
		default:
			error("Unknown game state");
			return false;
		}
	}

	return true;
}

void Runtime::bootGame() {
}

void Runtime::drawFrame() {

	_system->updateScreen();
}

} // End of namespace VCruise
