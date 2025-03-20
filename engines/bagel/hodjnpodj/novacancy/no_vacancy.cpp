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
		SCROLL_BUTTON_Y + SCROLL_BUTTON_DY - 1)) {
}

bool NoVacancy::msgOpen(const OpenMessage &msg) {
	MinigameView::msgOpen(msg);
	return true;
}

bool NoVacancy::msgClose(const CloseMessage &msg) { return true; }
bool NoVacancy::msgAction(const ActionMessage &msg) { return true; }
bool NoVacancy::msgKeypress(const KeypressMessage &msg) { return true; }
bool NoVacancy::msgMouseDown(const MouseDownMessage &msg) { return true; }
bool NoVacancy::msgGame(const GameMessage &msg) { return true; }

void NoVacancy::draw() {

}

bool NoVacancy::tick() {
	return true;
}

void NoVacancy::showMainMenu() {

}

} // namespace NoVacancy
} // namespace HodjNPodj
} // namespace Bagel
