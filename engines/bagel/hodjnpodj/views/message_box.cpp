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

#include "bagel/hodjnpodj/views/message_box.h"
#include "bagel/hodjnpodj/globals.h"

namespace Bagel {
namespace HodjNPodj {

#define	TEXT_COLOR			BLACK

// First message box positioning
#define	MESSAGE_COL			16
#define	MESSAGE_ROW     	60

// Standard message box dimensions
#define	MESSAGE_WIDTH		186
#define	MESSAGE_HEIGHT		20

// Row offset from first 
#define MESSAGE2_ROW_OFFSET ( MESSAGE_HEIGHT + 4 )


void MessageBox::show(const Common::String &title,
		const Common::String &msg, ViewCloseCallback callback) {
	MessageBox *view = (MessageBox *)g_events->findView("MessageBox");
	view->_title = title;
	view->_message = msg;
	view->_callback = callback;
	view->addView();
}

bool MessageBox::msgOpen(const OpenMessage &msg) {
	// All minigames share the same bitmaps, so use Fuge's arbitrarily
	_background.loadBitmap("fuge/art/sscroll.bmp");
	_background.setTransparentColor(WHITE);
	Common::Rect r(0, 0, _background.w, _background.h);
	r.moveTo((GAME_WIDTH - _background.w) / 2,
		(GAME_HEIGHT - _background.h) / 2);
	setBounds(r);

	// Make sure the cursor is shown
	g_events->setCursor(IDC_ARROW);

	return View::msgOpen(msg);
}

bool MessageBox::msgClose(const CloseMessage &msg) {
	_background.clear();
	return View::msgClose(msg);
}

void MessageBox::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_background);

	Common::Rect r(MESSAGE_COL, MESSAGE_ROW,
		MESSAGE_COL + MESSAGE_WIDTH, MESSAGE_ROW + MESSAGE_HEIGHT);
	s.writeString(_title, r, BLACK, Graphics::kTextAlignCenter);

	r.translate(0, MESSAGE2_ROW_OFFSET);
	s.writeString(_message, r, BLACK, Graphics::kTextAlignCenter);
}

} // namespace HodjNPodj
} // namespace Bagel
