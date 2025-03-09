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
#include "image/bmp.h"
#include "bagel/hodjnpodj/metagame/views/title_menu.h"
#include "bagel/hodjnpodj/metagame/views/movie.h"
#include "bagel/hodjnpodj/libs/dialog_unit.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define	BACKGROUND_BMP		"meta/art/mlscroll.bmp"
#define FONT_SIZE 12

TitleMenu::TitleMenu() : View("TitleMenu"),
	_newGame("NewGame", "&Start a New Board Game",
		DialogRect(FONT_SIZE, 46, 29, 108, 15), this),
	_restoreGame("RestoreGame", "&Restore an Old Board Game",
		DialogRect(FONT_SIZE, 46, 47, 108, 15), this),
	_standAlone("StandAlone", "&Play Mini-Games Stand-Alone",
		DialogRect(FONT_SIZE, 46, 65, 108, 15), this),
	_grandTour("GrandTour", "&Take the Grand Tour",
		DialogRect(FONT_SIZE, 46, 83, 108, 15), this),
	_viewFairyTale("RestartMovie", "&View the Fairy Tale",
		DialogRect(FONT_SIZE, 46, 101, 108, 15), this),
	_quit("Quit", "&Quit",
		DialogRect(FONT_SIZE, 46, 119, 108, 15), this) {
}

bool TitleMenu::msgOpen(const OpenMessage &msg) {
	g_events->showCursor(true);

	Common::File f;
	Image::BitmapDecoder decoder;
	if (!f.open(BACKGROUND_BMP) || !decoder.loadStream(f))
		error("Error loading - %s", BACKGROUND_BMP);

	const Graphics::Surface *surf = decoder.getSurface();
	_background.copyFrom(surf);
	_background.setTransparentColor(255);

	Common::Rect r(0, 0, surf->w, surf->h);
	r.moveTo((GAME_WIDTH - surf->w) / 2, (GAME_HEIGHT - surf->h) / 2);
	setBounds(r);

	return View::msgOpen(msg);
}

bool TitleMenu::msgGame(const GameMessage &msg) {
	if (msg._name == "BUTTON") {
		if (msg._stringValue == "RestartMovie") {
			send("Movie", GameMessage("MOVIE", STARTUP_MOVIE, MOVIE_ID_INTRO));

		} else if (msg._stringValue == "Quit") {
			g_engine->stopBackgroundMidi();
			g_engine->quitGame();
		}

		return true;
	}

	return false;
}

void TitleMenu::draw() {
	GfxSurface s = getSurface();
	s.setFontSize(FONT_SIZE);

	s.clear();
	s.blitFrom(_background);
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
