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
#include "bagel/hodjnpodj/metagame/views/dialog.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define BACKGROUND_BMP "meta/art/mlscroll.bmp"


bool Dialog::msgOpen(const OpenMessage &msg) {
	ResourceView::msgOpen(msg);
	g_events->showCursor(true);

	_background.loadBitmap(BACKGROUND_BMP);
	_background.setTransparentColor(WHITE);

	Common::Rect r(0, 0, _background.w, _background.h);
	r.moveTo((GAME_WIDTH - _background.w) / 2,
		(GAME_HEIGHT - _background.h) / 2);
	setBounds(r);

	return true;
}

bool Dialog::msgFocus(const FocusMessage &msg) {
	blackScreen();
	return true;
}

bool Dialog::msgClose(const CloseMessage &msg) {
	View::msgClose(msg);
	_background.clear();
	blackScreen();

	return true;
}

void Dialog::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_background);
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
