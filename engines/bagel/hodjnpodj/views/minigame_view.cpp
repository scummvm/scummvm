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
#include "bagel/hodjnpodj/views/minigame_view.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {

MinigameView::~MinigameView() {
	delete _resources;
}

bool MinigameView::msgOpen(const OpenMessage &msg) {
	SearchMan.add("Resources", this, 0, false);

	if (!_resourceFilename.empty())
		_resources = Common::NEResources::createFromEXE(Common::Path(_resourceFilename));

	g_events->setCursor(IDC_ARROW);

	_showMenuCtr = pGameParams->bPlayingMetagame ? 0 : 2;

	return View::msgOpen(msg);
}

bool MinigameView::msgClose(const CloseMessage &msg) {
	SearchMan.remove("Resources");
	delete _resources;
	_resources = nullptr;

	return View::msgClose(msg);
}

bool MinigameView::msgFocus(const FocusMessage &msg) {
	if (msg._priorView != nullptr) {
		Common::String name = msg._priorView->getName();

		if (name == "Minigames")
			g_engine->_bReturnToZoom = true;
		if (name == "GrandTour")
			g_engine->_bReturnToGrandTour = true;
		if (name == "Metagame")
			g_engine->_bReturnToMeta = true;
	}

	return View::msgFocus(msg);
}

void MinigameView::close() {
	if (g_engine->_bReturnToGrandTour) {
		g_engine->_bReturnToGrandTour = false;
		g_engine->startBackgroundMidi();
		replaceView("GrandTour");
	} else if (g_engine->_bReturnToMeta) {
		g_engine->_bReturnToMeta = false;
		lpMetaGame->m_bRestart = true;

		g_engine->stopBackgroundMidi();
		replaceView("MetaGame");
	} else if (g_engine->_bReturnToZoom) {
		g_engine->_bReturnToZoom = false;
		g_engine->startBackgroundMidi();
		replaceView("Minigames");
	} else {
		View::close();
	}
}

bool MinigameView::tick() {
	if (_showMenuCtr) {
		if (--_showMenuCtr == 0)
			showMainMenu();
	}

	return View::tick();
}

bool MinigameView::hasFile(const Common::Path &path) const {
	return _files.contains(path.toString());
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
	if (!_files.contains(filename))
		return nullptr;

	// TODO: Is there any way to share the _resources field?
	// We're in a const method, and the getResource call
	// isn't const. So maybe not? 
	// Load the resources from the specified file
	if (!winResources.loadFromEXE(Common::Path(_resourceFilename)))
		return nullptr;

	// Get and return the appropriate resource
	return winResources.getResource(
		_files[filename]._type, _files[filename]._id);
}

void MinigameView::drawSprites() {
	GfxSurface s = getSurface();

	for (auto it = _linkedSprites.begin();
			it != _linkedSprites.end(); ++it) {
		Sprite &sprite = **it;

		s.blitFrom(sprite, sprite);
	}
}

} // namespace HodjNPodj
} // namespace Bagel
