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
#include "bagel/metaengine.h"

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


void MessageBox::show(const Common::String &line1,
		const Common::String &line2, ViewCloseCallback callback) {
	MessageBox *view = (MessageBox *)g_events->findView("MessageBox");
	view->_line1 = line1;
	view->_line2 = line2;
	view->_callback = callback;
	view->addView();
}

MessageBox::MessageBox() : View("MessageBox"),
		_okButton(this) {
}

bool MessageBox::msgOpen(const OpenMessage &msg) {
	// All minigames share the same bitmaps, so use Fuge's arbitrarily
	_background.loadBitmap("fuge/art/sscroll.bmp");
	_background.setTransparentColor(WHITE);
	Common::Rect r(0, 0, _background.w, _background.h);
	r.moveTo((GAME_WIDTH - _background.w) / 2,
		(GAME_HEIGHT - _background.h) / 2);
	setBounds(r);

	Common::Rect btnRect(0, 0, 80, 25);
	btnRect.moveTo(_bounds.left + (_bounds.width() - btnRect.width()) / 2,
		_bounds.bottom - 54);
	_okButton.setBounds(btnRect);

	// Make sure the cursor is shown
	g_events->setCursor(IDC_ARROW);

	return View::msgOpen(msg);
}

bool MessageBox::msgClose(const CloseMessage &msg) {
	_background.clear();
	return View::msgClose(msg);
}

bool MessageBox::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_SELECT ||
		msg._action == KEYBIND_ESCAPE) {
		closeDialog();
		return true;
	}

	return false;
}

bool MessageBox::msgGame(const GameMessage &msg) {
	if (msg._name == "BUTTON") {
		closeDialog();
		return true;
	}

	return false;
}

void MessageBox::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_background);

	Common::Rect r(MESSAGE_COL, MESSAGE_ROW,
		MESSAGE_COL + MESSAGE_WIDTH, MESSAGE_ROW + MESSAGE_HEIGHT);
	s.writeString(_line1, r, BLACK, Graphics::kTextAlignCenter);

	r.translate(0, MESSAGE2_ROW_OFFSET);
	s.writeString(_line2, r, BLACK, Graphics::kTextAlignCenter);
}

void MessageBox::closeDialog() {
	close();
	_callback();
}

} // namespace HodjNPodj
} // namespace Bagel
