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

#include "bagel/hodjnpodj/views/minigame_view.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {

MinigameView::MinigameView(const Common::String &name, const Common::String &resFilename) :
		SpritesView(name, resFilename),
		_settings(g_engine->_settings[name]) {
}

bool MinigameView::msgOpen(const OpenMessage &msg) {
	_showMenuCtr = pGameParams->bPlayingMetagame ? 0 : 2;

	return ResourceView::msgOpen(msg);
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

bool MinigameView::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_q &&
			(msg.flags & Common::KBD_ALT)) {
		if (pGameParams->bPlayingMetagame)
			pGameParams->lScore = 0;
		close();
		return true;
	} else {
		return View::msgKeypress(msg);
	}
}

void MinigameView::close() {
	g_events->setCursor(1);

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

} // namespace HodjNPodj
} // namespace Bagel
