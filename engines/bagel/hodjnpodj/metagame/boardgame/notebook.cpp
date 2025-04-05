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

#define NOTEBOOK_BORDER_DX		DIALOG_LEFT
#define NOTEBOOK_BORDER_DY		DIALOG_TOP
#define NOTEBOOK_DX				DIALOG_WIDTH
#define NOTEBOOK_DY				DIALOG_HEIGHT

#define NOTE_BITMAPS_PER_LINE	6
#define NOTE_BITMAP_DX			59
#define NOTE_BITMAP_DY			59
#define NOTE_BITMAP_DDX			25
#define NOTE_BITMAP_DDY			10
#define NOTE_SMALL_BITMAP_DX	50
#define NOTE_SMALL_BITMAP_DY	50

#define NOTE_PERSON_DX			NOTEBOOK_BORDER_DX
#define NOTE_PERSON_DY			NOTEBOOK_BORDER_DY - 2
#define NOTE_PLACE_DX			NOTEBOOK_DX - NOTEBOOK_BORDER_DX - NOTE_BITMAP_DX - 5			
#define NOTE_PLACE_DY			NOTEBOOK_BORDER_DY - 2
#define NOTE_ICON_DY			NOTEBOOK_DY - NOTEBOOK_BORDER_DY - NOTE_BITMAP_DY + 2
#define NOTE_ICON_DDX			50

#define NOTE_TEXT_DX			NOTEBOOK_BORDER_DX
#define NOTE_TEXT_DY			NOTEBOOK_BORDER_DY + NOTE_BITMAP_DY + NOTE_BITMAP_DDY - 4
#define NOTE_TEXT_DDX			NOTEBOOK_DX - (NOTEBOOK_BORDER_DX << 1)
#define NOTE_TEXT_DDY			NOTEBOOK_DY - (NOTEBOOK_BORDER_DY << 1) - (NOTE_BITMAP_DY << 1) - NOTE_BITMAP_DDY + 4
#define	NOTE_TEXT_CHARSPERLINE	30
#define NOTE_TEXT_COLOR			PURPLE
#define NOTE_MORE_COLOR			BLACK
#define MORE_TEXT_BLURB			"[ More ]"				// actual text to display for "more" indicator

Notebook::Notebook() : Dialog("Notebook"),
	_personRect(RectWH(NOTE_PERSON_DX, NOTE_PERSON_DY, NOTE_BITMAP_DX, NOTE_BITMAP_DY)),
	_placeRect(RectWH(NOTE_PLACE_DX, NOTE_PLACE_DY, NOTE_BITMAP_DX, NOTE_BITMAP_DY)) {
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
	case KEYBIND_PAGEUP:
		priorNote();
		break;

	case KEYBIND_DOWN:
	case KEYBIND_PAGEDOWN:
		nextNote();
		break;

	case KEYBIND_HOME:
		firstNote();
		break;

	case KEYBIND_END:
		lastNote();
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

	GfxSurface s = getSurface();
	CNote *pNote;
	int nDeltaX, nDeltaY;
	int x, dx, dy;
	Common::Rect textRect;

	if (pNoteList == nullptr) {
		// Empty notebook
		if (pKeyNote == nullptr) {
			// ... so just say so and leave
			RectWH emptyRect(NOTE_TEXT_DX, NOTE_TEXT_DY,
				NOTE_TEXT_DDX, NOTE_TEXT_DDY);
			s.setFontSize(14);
			s.writeString("The log is empty ...",
				emptyRect, PURPLE, Graphics::kTextAlignCenter);
			return;
		} else {
			// Show the requested note
			pNote = pKeyNote;
		}
	} else {
		// Show the one note list points to
		pNote = pNoteList;
	}

	s.blitFrom(_person, Common::Point(NOTE_PERSON_DX, NOTE_PERSON_DY));
	s.blitFrom(_place, Common::Point(NOTE_PLACE_DX, NOTE_PLACE_DY));

	if (pNote->GetRepeatCount() <= NOTE_BITMAPS_PER_LINE) {	// shrink the bitmaps a little if
		nDeltaX = NOTE_BITMAP_DX;                               // ... the number of repetitions
		nDeltaY = NOTE_BITMAP_DY;                               // ... would force them off the page
	} else {
		nDeltaX = NOTE_SMALL_BITMAP_DX;
		nDeltaY = NOTE_SMALL_BITMAP_DY;
	}
	dx = (pNote->GetRepeatCount() * nDeltaX) + ((pNote->GetRepeatCount() - 1) * (NOTE_BITMAP_DDX / pNote->GetRepeatCount()));
	x = ((NOTEBOOK_DX - (NOTEBOOK_BORDER_DX << 1)) - dx) >> 1;  // establish left most position
	for (int i = 0; i < pNote->GetRepeatCount(); i++)
		// Loop till all icons displayed
		s.blitFrom(_clue, Common::Rect(0, 0, _clue.w, _clue.h),
			RectWH(x + (i * (nDeltaX + (NOTE_BITMAP_DDX / pNote->GetRepeatCount()))) +
				NOTEBOOK_BORDER_DX, NOTE_ICON_DY,
				nDeltaX, nDeltaY));

	s.setTextColor(NOTE_TEXT_COLOR);

	dy = s.getStringHeight() * _description.size() / NOTE_TEXT_CHARSPERLINE;
	if (dy < NOTE_TEXT_DDY)                                     // use the estimated number of lines
		textRect = Common::Rect(NOTE_TEXT_DX,                            // ... of text to see if we can fit into
			NOTE_TEXT_DY + ((NOTE_TEXT_DDY - dy) >> 1),	// ... a smaller rectangle, and thus
			NOTE_TEXT_DX + NOTE_TEXT_DDX,                // ... center the text vertically as well
			NOTE_TEXT_DY + ((NOTE_TEXT_DDY - dy) >> 1) + dy);
	else
		textRect = Common::Rect(NOTE_TEXT_DX,							// nope, so just use the default rectangle
			NOTE_TEXT_DY,
			NOTE_TEXT_DX + NOTE_TEXT_DDX,
			NOTE_TEXT_DY + NOTE_TEXT_DDY);

	GfxSurface textSurface(s, textRect, this);
	Common::StringArray lines;
	textSurface.wordWrapText(_description, lines);

	for (uint i = 0; i < lines.size();
			++i, textRect.top += textSurface.getStringHeight()) {
		textSurface.writeString(lines[i], textRect);
	}

	// Handle More labels for scrolling
	s.setTextColor(NOTE_MORE_COLOR);
	if (hasPriorNote())
		s.writeString(MORE_TEXT_BLURB, _moreTop);
	if (hasNextNote())
		s.writeString(MORE_TEXT_BLURB, _moreBottom);
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

void Notebook::firstNote() {
	if (hasPriorNote()) {
		while (pNoteList->m_pPrev)
			pNoteList = pNoteList->m_pPrev;

		redraw();
	}
}

void Notebook::lastNote() {
	if (hasNextNote()) {
		while (pNoteList->m_pNext)
			pNoteList = pNoteList->m_pNext;

		redraw();
	}
}

void Notebook::updateContent() {
	CNote *pNote;
	const char *pFileSpec;

	if (pNoteList == nullptr) {
		if (pKeyNote == nullptr)
			// Empty log
			return;
		pNote = pKeyNote;
	} else {
		pNote = pNoteList;
	}

	lpsPersonSoundSpec = pNote->GetPersonSoundSpec();
	lpsPlaceSoundSpec = pNote->GetPlaceSoundSpec();

	pFileSpec = pNote->GetPersonArtSpec();
	assert(pFileSpec);
	_person.loadBitmap(pFileSpec);

	pFileSpec = pNote->GetPlaceArtSpec();
	assert(pFileSpec);
	_place.loadBitmap(pFileSpec);

	pFileSpec = pNote->GetClueArtSpec();
	assert(pFileSpec);
	_clue.loadBitmap(pFileSpec);

	_description = pNote->GetDescription();
	assert(!_description.empty());
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
