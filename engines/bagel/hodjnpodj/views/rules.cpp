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

#include "bagel/hodjnpodj/views/rules.h"
#include "bagel/hodjnpodj/globals.h"

namespace Bagel {
namespace HodjNPodj {

#define COLOR_BUTTONS		TRUE
#define SCROLL_PIECES		6						// number of mid-scroll segments

// Scroll bitmaps. Since all minigames have the same bitmaps,
// this just uses one minigame at random
#define SCROLL_SPEC	    "fuge/art/lscroll.bmp"       // path for scroll DIB on disk
#define SCROLL_TOP_SPEC	"fuge/art/lscrollt.bmp"      // path for scroll's top section DIB on disk
#define SCROLL_BOT_SPEC	"fuge/art/lscrollb.bmp"      // path for scroll's bottom section DIB on disk
#define SCROLL_MID_SPEC	"fuge/art/lscrollm.bmp"      // path for scroll's middle section DIB on disk

#define BUTTON_DY			15						// offset for Okay button from scroll base

#define	SCROLL_STRIP_WIDTH	10						// width of scroll middle to reveal per interval 
#define	SCROLL_STRIP_DELAY	1000					// delay to wait after each partial scroll unfurling

#define	TEXT_BUFFER_SIZE	512						// # characters in the text input buffer
#define	TEXT_LEFT_MARGIN	55						// left margin offset for display of text
#define	TEXT_TOP_MARGIN		5                       // top margin offset for display of text
#define	TEXT_BOTTOM_MARGIN	20                      // bottom margin offset for display of text
#define	TEXT_WIDTH			435                     // width of text display area
#define TEXT_MORE_DX		120						// offset of "more" indicator from right margin
#define TEXT_MORE_DY		10                      // offset of "more" indicator bottom of scroll
#define MORE_TEXT_BLURB		"[ More ]"				// actual text to display for "more" indicator
#define MORE_TEXT_LENGTH	8                       // # characters in "more" indicator string
#define TEXT_NEWLINE		'\\'                    // character that indicates enforced line break

Rules::Rules() : View("Rules") {
}

void Rules::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_background);
}

void Rules::show(const Common::String &filename,
		const Common::String &waveFile,
		ViewCloseCallback callback) {
	Rules *view = (Rules *)g_events->findView("Rules");
	view->_filename = filename;
	view->_waveFilename = waveFile;
	view->_callback = callback;
	view->addView();
}

bool Rules::msgOpen(const OpenMessage &msg) {
	// All minigames share the same bitmaps, so use Fuge's arbitrarily
	_background.loadBitmap(SCROLL_SPEC);
	_background.setTransparentColor(255);
	Common::Rect r(0, 0, _background.w, _background.h);
	r.moveTo((GAME_WIDTH - _background.w) / 2,
		(GAME_HEIGHT - _background.h) / 2);
	setBounds(r);
#if 0
	Common::Rect btnRect(0, 0, 80, 25);
	btnRect.moveTo(_bounds.left + (_bounds.width() - btnRect.width()) / 2,
		_bounds.bottom - 54);
	_okButton.setBounds(btnRect);
#endif
	// Make sure the cursor is shown
	g_events->setCursor(IDC_ARROW);

	return View::msgOpen(msg);
}

bool Rules::msgClose(const CloseMessage &msg) {
	_background.clear();
	return View::msgClose(msg);
}

} // namespace HodjNPodj
} // namespace Bagel
