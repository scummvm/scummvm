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
#include "bagel/hodjnpodj/metagame/boardgame/notebook.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#if 0
#define COLOR_BUTTONS		TRUE
#define SCROLL_PIECES		6						// number of mid-scroll segments
#define WHITE               255

#define BUTTON_DY			15						// offset for Okay button from scroll baseBUTTON_DY
#define	SCROLL_STRIP_WIDTH	10						// width of scroll middle to reveal per interval 
#define	SCROLL_STRIP_DELAY	1000					// delay to wait after each partial scroll unfurling

#define TEXT_SIZE           8
#define	TEXT_BUFFER_SIZE	512						// # characters in the text input buffer
#define	TEXT_LEFT_MARGIN	55						// left margin offset for display of text
#define	TEXT_RIGHT_MARGIN	45						// right margin offset for display of text
#define	TEXT_TOP_MARGIN		5                       // top margin offset for display of text
#define	TEXT_BOTTOM_MARGIN	20                      // bottom margin offset for display of text
#define	TEXT_WIDTH			(490-TEXT_LEFT_MARGIN-TEXT_RIGHT_MARGIN)	// Width of text display area
#define TEXT_MORE_DX		120						// offset of "more" indicator from right margin
#define TEXT_MORE_DY		10                      // offset of "more" indicator bottom of scroll
#define MORE_TEXT_BLURB		"[ More ]"				// actual text to display for "more" indicator
#define MORE_TEXT_LENGTH	8                       // # characters in "more" indicator string
#define TEXT_NEWLINE		'\\'                    // character that indicates enforced line break
#endif

Notebook::Notebook() : Dialog("Notebook") {
}

void Notebook::show(CNote *pNotes, CNote *pNote) {
	Notebook *view = (Notebook *)g_events->findView("Notebook");
	view->pNoteList = pNotes;								// retain pointer to note list
	view->pKeyNote = pNote;
	view->addView();
}

bool Notebook::msgOpen(const OpenMessage &msg) {
	Dialog::msgOpen(msg);

	if (!pNoteList) {
		// For debug purposes
		auto &inv = lpMetaGame->m_cHodj.m_pInventory;
		auto *pItem = inv->FindItem(MG_OBJ_HODJ_NOTEBOOK);
		pNoteList = pItem->GetFirstNote();
		pKeyNote = nullptr;
	}

	_scrollTopRect = Common::Rect(0, 0, 501, 48);
	_scrollBottomRect = Common::Rect(0, 0, 501, 47);

	// Scroll rects will be compared against mouse pos,
	// so we need to shift them to global screen co-ordinates
	_scrollTopRect.moveTo(_bounds.left, _bounds.top);
	_scrollBottomRect.moveTo(_bounds.left,
		_bounds.bottom - _scrollBottomRect.height());


	// Make sure the cursor is shown
	g_events->setCursor(IDC_ARROW);

	// Set More text positions
	_moreTop = Common::Point(_bounds.width() - 120, 20);
	_moreBottom = Common::Point(_bounds.width() - 120, _bounds.height() - 45);

	return true;
}

bool Notebook::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_SELECT:
	case KEYBIND_ESCAPE:
		close();
		break;

	case KEYBIND_UP:
		priorNote();
		break;

	case KEYBIND_DOWN:
		nextNote();
		break;

	default:
		return false;
	}

	return true;
}

bool Notebook::msgKeypress(const KeypressMessage &msg) {
	if (Dialog::msgKeypress(msg))
		return true;

	switch (msg.keycode) {
	case Common::KEYCODE_PAGEUP:
		priorNote();
		break;

	case Common::KEYCODE_PAGEDOWN:
		nextNote();
		break;

	default:
		return false;
	}

	return true;
}

bool Notebook::msgGame(const GameMessage &msg) {
	if (msg._name == "BUTTON") {
		close();
		return true;
	}

	return false;
}

bool Notebook::msgMouseUp(const MouseUpMessage &msg) {
	if (View::msgMouseUp(msg))
		return true;

	if (msg._button == MouseUpMessage::MB_LEFT) {
		if (_scrollTopRect.contains(msg._pos)) {
			priorNote();
		} else if (_scrollBottomRect.contains(msg._pos)) {
			nextNote();
		}
	}

	return true;
}


void Notebook::draw() {
	Dialog::draw();

#if TODO
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
#endif
}

bool Notebook::hasPriorNote() const {
	return (pKeyNote == nullptr) && (pNoteList != nullptr) &&
		(pNoteList->m_pPrev != nullptr);
}

bool Notebook::hasNextNote() const {
	return (pKeyNote == nullptr) && (pNoteList != nullptr) &&
		(pNoteList->m_pNext != nullptr);
}

void Notebook::priorNote() {
	if (hasPriorNote()) {
		pNoteList = pNoteList->m_pPrev;
		redraw();
	}
}

void Notebook::nextNote() {
	if (hasNextNote()) {
		pNoteList = pNoteList->m_pNext;
		redraw();
	}
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
