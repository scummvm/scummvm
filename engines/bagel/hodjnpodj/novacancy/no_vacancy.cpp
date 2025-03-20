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

#include "common/system.h"
#include "common/file.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/novacancy/no_vacancy.h"
#include "bagel/hodjnpodj/novacancy/defines.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/views/main_menu.h"
#include "bagel/hodjnpodj/views/message_box.h"
#include "bagel/hodjnpodj/views/rules.h"

namespace Bagel {
namespace HodjNPodj {
namespace NoVacancy {

NoVacancy::NoVacancy() : MinigameView("NoVacancy", "novac/hnpnova.dll"),
		_scrollButton("Scroll", this, Common::Rect(
			SCROLL_BUTTON_X, SCROLL_BUTTON_Y,
			SCROLL_BUTTON_X + SCROLL_BUTTON_DX - 1,
			SCROLL_BUTTON_Y + SCROLL_BUTTON_DY - 1)),
		m_rNewGameButton(15, 4, 233, 20),
		m_rectGameArea(0, 0, GAME_WIDTH, GAME_HEIGHT) {
	addResource(SCROLLUP_BMP, SCROLLUP);
	addResource(SCROLLDOWN_BMP, SCROLLDOWN);
}

bool NoVacancy::msgOpen(const OpenMessage &msg) {
	MinigameView::msgOpen(msg);

	resetFields();
	loadBitmaps();

	return true;
}

bool NoVacancy::msgClose(const CloseMessage &msg) { return true; }
bool NoVacancy::msgAction(const ActionMessage &msg) { return true; }
bool NoVacancy::msgKeypress(const KeypressMessage &msg) { return true; }
bool NoVacancy::msgMouseDown(const MouseDownMessage &msg) { return true; }
bool NoVacancy::msgGame(const GameMessage &msg) { return true; }

void NoVacancy::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_background);
}

bool NoVacancy::tick() {
	return true;
}

void NoVacancy::showMainMenu() {

}

void NoVacancy::resetFields() {
	m_bSound = true;

	m_bGameLoadUp = true;
	m_bPause = false;
	m_bGameActive = false;
	m_cActiveDoor = OPEN;			// The game has just begun and no door is active. The 0-th door is always open.
	m_cDoorCount = 0;				//	no door is closed.    

	m_cUnDoableThrows = 0;			//	no undoable throw has been registered yet.
	m_bDiceJustThrown = false;		// dice haven't been thrown yet
	m_bOneDieCase = false;			// always start w/ two dice on floor.     

}

void NoVacancy::loadBitmaps() {
	Image::BitmapDecoder decoder;
	Common::File f;

	if (!f.open(BACKGROUND_BMP) || !decoder.loadStream(f))
		error("Could not load - %s", BACKGROUND_BMP);
	loadPalette(decoder.getPalette());

	_background.copyFrom(*decoder.getSurface());

	_scrollButton.loadBitmaps(SCROLLUP_BMP, SCROLLDOWN_BMP,
		nullptr, nullptr);
}

} // namespace NoVacancy
} // namespace HodjNPodj
} // namespace Bagel
