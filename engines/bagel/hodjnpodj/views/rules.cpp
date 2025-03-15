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
#include "common/translation.h"
#include "bagel/hodjnpodj/views/rules.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {

#define COLOR_BUTTONS		TRUE
#define SCROLL_PIECES		6						// number of mid-scroll segments
#define WHITE               255

// Scroll bitmaps. Since all minigames have the same bitmaps,
// this just uses one minigame at random
#define SCROLL_SPEC	    "fuge/art/lscroll.bmp"       // path for scroll DIB on disk
#define SCROLL_TOP_SPEC	"fuge/art/lscrollt.bmp"      // path for scroll's top section DIB on disk
#define SCROLL_BOT_SPEC	"fuge/art/lscrollb.bmp"      // path for scroll's bottom section DIB on disk
#define SCROLL_MID_SPEC	"fuge/art/lscrollm.bmp"      // path for scroll's middle section DIB on disk

#define BUTTON_DY			15						// offset for Okay button from scroll baseBUTTON_DY
#define	SCROLL_STRIP_WIDTH	10						// width of scroll middle to reveal per interval 
#define	SCROLL_STRIP_DELAY	1000					// delay to wait after each partial scroll unfurling

#define TEXT_SIZE           12
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

Rules::Rules() : View("Rules"), _more(_s(MORE_TEXT_BLURB)) {
}

void Rules::show(const char *filename, const char *soundFilename) {
	Rules *view = (Rules *)g_events->findView("Rules");
	view->_filename = filename;
	view->_soundFilename  = soundFilename;
	view->addView();
}

bool Rules::msgOpen(const OpenMessage &msg) {
	size_t idx;

	// All minigames share the same bitmaps, so use Fuge's arbitrarily
	_scroll.loadBitmap(SCROLL_SPEC);
	_scrollTop.loadBitmap(SCROLL_TOP_SPEC);
	_scrollBottom.loadBitmap(SCROLL_BOT_SPEC);
	_scrollMiddle.loadBitmap(SCROLL_MID_SPEC);
	_scroll.setTransparentColor(WHITE);
	_scrollTop.setTransparentColor(WHITE);
	_scrollBottom.setTransparentColor(WHITE);
	_scrollMiddle.setTransparentColor(WHITE);
	_scrollTopRect = Common::Rect();
	_scrollBottomRect = Common::Rect();

	Common::Rect r(0, 0, _scroll.w, _scroll.h);
	r.moveTo((GAME_WIDTH - _scroll.w) / 2,
		(GAME_HEIGHT - _scroll.h) / 2);
	setBounds(r);

	// Set the position of the Ok button, but don't immediately
	// add it, since it will only show when the scroll is unfurled
	Common::Rect btnRect(0, 0, 80, 25);
	btnRect.moveTo(_bounds.left + (_bounds.width() - btnRect.width()) / 2,
		_bounds.bottom - btnRect.height() - BUTTON_DY);
	_okButton.setBounds(btnRect);

	// Save a copy of the current background, since it's used
	// for redrawing as the scroll opens
	GfxSurface s = getSurface();
	_background.copyFrom(s);

	// Make sure the cursor is shown
	g_events->setCursor(IDC_ARROW);

	// Read the text content
	Common::File f;
	if (!f.open(_filename))
		error("Could not open - %s", _filename);
	Common::String text = f.readString();

	// Pre-formatting of text into something suitable for wordWrapText
	idx = 0;
	while ((idx = text.find("\r\n", idx)) != Common::String::npos) {
		text.deleteChar(idx);
		text.deleteChar(idx);
	}

	idx = 0;
	while ((idx = text.find("\t", idx)) != Common::String::npos) {
		text.deleteChar(idx);
		text.insertString("    ", idx);
	}

	text.replace('\\', '\n');

	// Line wrap the lines
	s = getSurface(Common::Rect(TEXT_LEFT_MARGIN, _scrollTop.h + TEXT_TOP_MARGIN,
		TEXT_LEFT_MARGIN + TEXT_WIDTH, _scroll.h - _scrollBottom.h - TEXT_BOTTOM_MARGIN));
	s.setFontSize(TEXT_SIZE);
	Common::StringArray lines;
	s.wordWrapText(text, lines);

	// Split the lines into each page's
	_lines.clear();
	int numLines = s.h / (s.getStringHeight() + 2);
	_lines.resize((lines.size() + numLines - 1) / numLines);

	for (idx = 0; idx < lines.size(); ++idx)
		_lines[idx / numLines].push_back(lines[idx]);

	// Set More text positions
	_moreTop = Common::Point(_bounds.width() - 120, 20);
	_moreBottom = Common::Point(_bounds.width() - 120, _bounds.height() - 45);

	// Render the first page of text
	_scrollY = 0;
	_helpPage = 0;
	renderPage();

	// Play dictation
	if (_soundFilename)
		_dictation = new CBofSound(this, _soundFilename,
			SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);

	return View::msgOpen(msg);
}

bool Rules::msgClose(const CloseMessage &msg) {
	_background.clear();

	delete _dictation;
	_dictation = nullptr;

	return View::msgClose(msg);
}

bool Rules::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_SELECT ||
		msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	}

	return false;
}

bool Rules::msgGame(const GameMessage &msg) {
	if (msg._name == "BUTTON") {
		close();
		return true;
	}

	return false;
}

bool Rules::msgMouseMove(const MouseMoveMessage &msg) {
	if (View::msgMouseMove(msg))
		return true;

	if (_scrollTopRect.contains(msg._pos)) {
		if (_helpPage == 0)
			g_events->setCursor(IDC_RULES_INVALID);
		else
			g_events->setCursor(IDC_RULES_ARROWUP);
	} else if (_scrollBottomRect.contains(msg._pos)) {
		if (_helpPage == (_lines.size() - 1))
			g_events->setCursor(IDC_RULES_INVALID);
		else
			g_events->setCursor(IDC_RULES_ARROWDN);
	} else {
		g_events->setCursor(IDC_ARROW);
	}

	return true;
}

bool Rules::msgMouseUp(const MouseUpMessage &msg) {
	if (View::msgMouseUp(msg))
		return true;

	if (msg._button == MouseUpMessage::MB_LEFT) {
		if (_scrollTopRect.contains(msg._pos) && _helpPage > 0) {
			// Move to prior page
			--_helpPage;
			renderPage();
			redraw();
		} else if (_scrollBottomRect.contains(msg._pos) &&
				_helpPage < (_lines.size() - 1)) {
			// Move to the next page
			++_helpPage;
			renderPage();
			redraw();
		}
	}

	return true;
}


void Rules::draw() {
	int scrollHeight = _scroll.h - _scrollTop.h - _scrollBottom.h;
	GfxSurface s = getSurface();
	s.setFontSize(TEXT_SIZE);
	s.blitFrom(_background);

	if (_scrollY < scrollHeight) {
		// Unfurling scroll
		// Top scroll
		int y = (s.h - _scrollTop.h - _scrollY - _scrollBottom.h) / 2;
		s.blitFrom(_scrollTop, Common::Point(0, y));

		// Bottom scroll
		s.blitFrom(_scrollBottom,
			Common::Point(0, y + _scrollTop.h + _scrollY));

		// Partial content of the scroll middle
		const Graphics::ManagedSurface content(_scrollContent,
			Common::Rect(0, _scrollTop.h, _scrollContent.w,
				_scrollContent.h - _scrollBottom.h));

		s.blitFrom(content,
			Common::Rect(0, content.h / 2 - (_scrollY / 2),
				content.w, content.h / 2 + (_scrollY / 2)),
			Common::Point(0, y + _scrollTop.h)
		);

	} else {
		// The scroll is completely unfurled
		s.blitFrom(_scrollContent);

		// Add the Ok button if not already
		if (_children.empty())
			_okButton.setParent(this);

		// Show the more buttons
		if (_helpPage > 0) {
			s.writeString(_more, _moreTop, BLACK);
		}
		if ((_helpPage + 1) < _lines.size()) {
			s.writeString(_more, _moreBottom, BLACK);
		}
	}
}

void Rules::renderPage() {
	Common::String line;

	// Build up scroll image
	_scrollContent.create(_scroll.w, _scroll.h);
	_scrollContent.clear(WHITE);
	_scrollContent.setTransparentColor(WHITE);

	_scrollContent.blitFrom(_scrollTop);
	for (int i = 0; i < SCROLL_PIECES; ++i)
		_scrollContent.blitFrom(_scrollMiddle,
			Common::Point(0, _scrollTop.h + _scrollMiddle.h * i));
	_scrollContent.blitFrom(_scrollBottom,
		Common::Point(0, _scrollTop.h + _scrollMiddle.h * SCROLL_PIECES));

	Common::Rect textRect(TEXT_LEFT_MARGIN, _scrollTop.h + TEXT_TOP_MARGIN,
		TEXT_LEFT_MARGIN + TEXT_WIDTH, _scroll.h - _scrollBottom.h - TEXT_BOTTOM_MARGIN);
	GfxSurface s(_scrollContent, textRect, this);
	s.setFontSize(TEXT_SIZE);

	for (uint idx = 0, y = 0; idx < _lines[_helpPage].size();
			++idx, y += s.getStringHeight() + 2) {
		// Get the next line
		line = _lines[_helpPage][idx];

		// Write it out
		s.writeString(line, Common::Point(0, y), BLACK);
	}
}

bool Rules::tick() {
	int scrollHeight = _scroll.h - _scrollTop.h - _scrollBottom.h;

	if (_scrollY < scrollHeight) {
		_scrollY = MIN(_scrollY + 20, scrollHeight);
		redraw();

		if (_scrollY == scrollHeight && _dictation) {
			_dictation->play();

			_scrollTopRect = Common::Rect(0, 0, 501, 48);
			_scrollBottomRect = Common::Rect(0, 0, 501, 47);
			_scrollTopRect.moveTo(_bounds.left, _bounds.top);
			_scrollBottomRect.moveTo(_bounds.left,
				_bounds.bottom - _scrollBottomRect.height());
		}
	}

	return true;
}

} // namespace HodjNPodj
} // namespace Bagel
