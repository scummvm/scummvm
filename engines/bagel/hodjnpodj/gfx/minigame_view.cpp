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

#include "common/file.h"
#include "common/formats/winexe_ne.h"
#include "bagel/hodjnpodj/gfx/minigame_view.h"

namespace Bagel {
namespace HodjNPodj {

bool MinigameView::msgOpen(const OpenMessage &msg) {
	SearchMan.add("Resources", this, 0, false);

	return View::msgOpen(msg);
}

bool MinigameView::msgClose(const CloseMessage &msg) {
	SearchMan.remove("Resources");

	return View::msgClose(msg);
}

bool MinigameView::hasFile(const Common::Path &path) const {
	return _bitmapFiles.contains(path.toString()) ||
		_soundFiles.contains(path.toString());
}

int MinigameView::listMembers(Common::ArchiveMemberList &list) const {
	return 0;
}

const Common::ArchiveMemberPtr MinigameView::getMember(const Common::Path &path) const {
	return Common::ArchiveMemberPtr();
}

Common::SeekableReadStream *MinigameView::createReadStreamForMember(const Common::Path &path) const {
	Common::NEResources winResources;
	Common::String filename = path.toString();

	// See if it's a filename that's been registered
	if (!_bitmapFiles.contains(filename) &&
		!_soundFiles.contains(filename))
		return nullptr;

	// Load the resources from the specified file
	if (!winResources.loadFromEXE(Common::Path(_resourceFilename)))
		return nullptr;

	// Get and return the appropriate resource
	if (_bitmapFiles.contains(filename)) {
		return winResources.getResource(Common::kWinBitmap,
			_bitmapFiles[filename]);
	} else {
		return winResources.getResource(Common::WinResourceID("WAVE"),
			_soundFiles[filename]);
	}

	return nullptr;
}

} // namespace HodjNPodj
} // namespace Bagel
