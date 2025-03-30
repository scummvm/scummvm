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

#include "bagel/hodjnpodj/metagame/boardgame/backpack.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define BACKPACK_DX				502
#define BACKPACK_DY				395
#define	BACKPACK_CURL_DX		500
#define	BACKPACK_CURL_DY		50
#define BACKPACK_BORDER_DX		60
#define BACKPACK_BORDER_DY		80

#define BACKPACK_TITLEZONE_DY	10
#define BACKPACK_TEXTZONE_DX	75
#define BACKPACK_TEXTZONE_DY	30
#define BACKPACK_TEXTZONE_DDY	10

#define BACKPACK_BITMAP_DX		59
#define BACKPACK_BITMAP_DY		59
#define BACKPACK_BITMAP_DDX		10
#define BACKPACK_BITMAP_DDY		10

#define	BACKPACK_FONT_SIZE		14
#define	BACKPACK_TEXT_COLOR		PALETTERGB(128,0,128)
#define	BACKPACK_BLURB_COLOR	PALETTERGB(0,0,255)
#define BACKPACK_MORE_COLOR		PALETTERGB(0,0,0)

#define TEXT_MORE_DX		120						// offset of "more" indicator from right margin
#define TEXT_MORE_DY		5                       // offset of "more" indicator bottom of scroll
#define MORE_TEXT_BLURB		"[ More ]"				// actual text to display for "more" indicator

#define	IDC_NOTEBOOK_BOOK	    921
#define	IDC_NOTEBOOK_SOUND	    922

Backpack::Backpack() : Dialog("Backpack"),
_okButton(Common::Rect(210, 355, 290, 380), this),
pInventory(lpMetaGame->m_cHodj.m_pInventory) {
}

bool Backpack::msgOpen(const OpenMessage &msg) {
	Dialog::msgOpen(msg);

	_scrollTopRect = Common::Rect(0, 0, 501, 48);
	_scrollBottomRect = Common::Rect(0, 0, 501, 47);

	GfxSurface s = getSurface();
	s.setFontSize(BACKPACK_FONT_SIZE);
	_titleRect = RectWH(0,
		DIALOG_TOP + BACKPACK_TITLEZONE_DY,
		_bounds.width(), s.getStringHeight());

	// Scroll rects will be compared against mouse pos,
	// so we need to shift them to global screen co-ordinates
	_scrollTopRect.moveTo(_bounds.left, _bounds.top);
	_scrollBottomRect.moveTo(_bounds.left,
		_bounds.bottom - _scrollBottomRect.height());

	updateContent();

	return true;
}

bool Backpack::msgClose(const CloseMessage &msg) {
	Dialog::msgClose(msg);
	return true;
}

bool Backpack::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_UP:
		if (nFirstSlot > 0) {
			nFirstSlot -= (nItemsPerRow * nItemsPerColumn);
			if (nFirstSlot < 0)
				nFirstSlot = 0;
			redraw();
		}
		break;

	case KEYBIND_DOWN:
		if (nFirstSlot + (nItemsPerRow * nItemsPerColumn) < (*pInventory).ItemCount()) {
			nFirstSlot += (nItemsPerRow * nItemsPerColumn);
			redraw();
		}
		break;

	case KEYBIND_SELECT:
	case KEYBIND_ESCAPE:
		close();
		break;

	default:
		return false;
	}

	return true;
}

bool Backpack::msgGame(const GameMessage &msg) {
	if (msg._name == "SHOW") {
		auto &player = msg._value ?
			lpMetaGame->m_cHodj : lpMetaGame->m_cPodj;
		pInventory = player.m_pInventory;
		addView();
		return true;
	} else if (msg._name == "BUTTON" && msg._stringValue == "OK") {
		close();
		return true;
	}

	return false;
}

bool Backpack::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_HOME:
		// Go to first item
		if (nFirstSlot != 0) {
			nFirstSlot = 0;
			redraw();
		}
		break;

	case Common::KEYCODE_END:								// go to last page of text
		nFirstSlot = (*pInventory).ItemCount() - (nItemsPerRow * nItemsPerColumn);
		if (nFirstSlot < 0)
			nFirstSlot = 0;
		redraw();
		break;

	default:
		return false;
	}

	return true;
}

void Backpack::draw() {
	Dialog::draw();

	GfxSurface s = getSurface();
	s.setFontSize(BACKPACK_FONT_SIZE);
	s.writeString(pInventory->GetTitle(), _titleRect,
		BACKPACK_TEXT_COLOR, Graphics::kTextAlignCenter);

	drawItems(s);

	// Handle displaying blurb for any highlighted item
	if (_selectedItem) {
		Common::Rect blurbRect(BACKPACK_TEXTZONE_DX,
			BACKPACK_DY - BACKPACK_BORDER_DY - BACKPACK_TEXTZONE_DY + BACKPACK_TEXTZONE_DDY,
			BACKPACK_DX - BACKPACK_TEXTZONE_DX,
			BACKPACK_DY - BACKPACK_BORDER_DY + BACKPACK_TEXTZONE_DDY);

		s.writeString(_selectedItem->GetDescription(),
			blurbRect, BACKPACK_TEXT_COLOR);
	}
}

void Backpack::updateContent() {
	if ((*pInventory).ItemCount() <= 0)
		return;

	// Calculate the horizontal space we have available
	nBackpack_DX = _bounds.width() - (BACKPACK_BORDER_DX << 1);
	// Estimate number of items that will fit
	nItemsPerRow = nBackpack_DX / BACKPACK_BITMAP_DX;

	for (;;) {
		nItem_DDX = (nBackpack_DX - (nItemsPerRow * BACKPACK_BITMAP_DX)) / (nItemsPerRow - 1);	// now evaluate the distance that would occur between
		if ((nItem_DDX >= BACKPACK_BITMAP_DDX) ||               // ... items, and if is less than the minimum allowed
			(nItemsPerRow == 1))
			break;                                              // ... then reduce the count of items per row
		nItemsPerRow -= 1;
	}

	// calculate the vertical space we have available
	nBackpack_DY = BACKPACK_DY - BACKPACK_TEXTZONE_DY - (BACKPACK_BORDER_DY << 1) - BACKPACK_TITLEZONE_DY;
	// Estimate number of items that will fit
	nItemsPerColumn = nBackpack_DY / BACKPACK_BITMAP_DY;

	for (;;) {
		nItem_DDY = (nBackpack_DY - (nItemsPerColumn * BACKPACK_BITMAP_DY)) / (nItemsPerColumn - 1);    // now evaluate the distance that would occur between
		if ((nItem_DDY >= BACKPACK_BITMAP_DDY) ||               // ... items, and if is less than the minimum allowed
			(nItemsPerColumn == 1))
			break;                                              // ... then reduce the count of items per column
		nItemsPerColumn -= 1;
	}
}

void Backpack::drawItems(GfxSurface &s) {
	// Get first item on this page
	auto pItem = (*pInventory).FetchItem(nFirstSlot);
	int x, y;

	for (int i = 0; (i < (nItemsPerRow * nItemsPerColumn)) && (pItem != nullptr); i++) {							// will thumb through all of them
		x = (i % nItemsPerRow);                                 // calculate its horizontal position
		x *= (BACKPACK_BITMAP_DX + nItem_DDX);                      // ... allowing proper spacing between items
		y = (i / nItemsPerRow);                                 // calculate its vertical position
		y *= (BACKPACK_BITMAP_DY + nItem_DDY);                      // ... allowing proper spacing between items
		drawItem(s, pItem, x + BACKPACK_BORDER_DX, y + BACKPACK_BORDER_DY + BACKPACK_TITLEZONE_DY);		// now show the item
		pItem = pItem->GetNext();
	}
}

void Backpack::drawMore(GfxSurface &s) {
	if (!hasNextPage())
		return;

	int x = BACKPACK_DX - TEXT_MORE_DX;	                            // ... that can be scrolled through
	int y = BACKPACK_DY -
		BACKPACK_CURL_DY +
		((BACKPACK_CURL_DY - s.getStringHeight()) >> 1) -
		TEXT_MORE_DY;

	s.writeString(MORE_TEXT_BLURB, Common::Point(x, y),
		BACKPACK_MORE_COLOR);
}

bool Backpack::hasNextPage() const {
	return (nFirstSlot +
		(nItemsPerRow * nItemsPerColumn)) <
		pInventory->ItemCount();
}

bool Backpack::msgMouseMove(const MouseMoveMessage &msg) {
	int oldIndex = _selectedIndex;
	int index = oldIndex;
	_selectedItem = nullptr;

	if (!Dialog::msgMouseMove(msg)) {
		if (_scrollTopRect.contains(msg._pos)) {
			if (nFirstSlot == 0)
				g_events->setCursor(IDC_RULES_INVALID);
			else
				g_events->setCursor(IDC_RULES_ARROWUP);
		} else if (_scrollBottomRect.contains(msg._pos)) {
			if (!hasNextPage())
				g_events->setCursor(IDC_RULES_INVALID);
			else
				g_events->setCursor(IDC_RULES_ARROWDN);
		} else {
			// Check for highlighted item
			index = selectedItem(msg._pos);

			if ((index >= 0) && ((index + nFirstSlot) < (*pInventory).ItemCount())) {
				_selectedItem = (*pInventory).FetchItem(index + nFirstSlot);
				if (_selectedItem != nullptr) {
					if (_selectedItem->m_nActionCode == ITEM_ACTION_NOTEBOOK) {
						g_events->setCursor(IDC_NOTEBOOK_BOOK);
					} else {
						if (_selectedItem->GetSoundSpec() != nullptr) {
							g_events->setCursor(IDC_NOTEBOOK_SOUND);
							_selectedItem->m_nActionCode = ITEM_ACTION_SOUND;
						} else {
							g_events->setCursor(IDC_ARROW);
						}
					}
				}
			} else {
				g_events->setCursor(IDC_ARROW);
			}
		}
	}

	if (index != oldIndex)
		redraw();
	_selectedIndex = index;

	return true;
}

bool Backpack::msgMouseUp(const MouseUpMessage &msg) {
	if (View::msgMouseUp(msg))
		return true;

	if (msg._button == MouseUpMessage::MB_LEFT) {
		if (_scrollTopRect.contains(msg._pos) && hasPriorPage()) {
			// Move to prior page
			nFirstSlot -= (nItemsPerRow * nItemsPerColumn);
			if (nFirstSlot < 0)
				nFirstSlot = 0;
			redraw();

		} else if (_scrollBottomRect.contains(msg._pos) && hasNextPage()) {
			// Move to the next page
			nFirstSlot += (nItemsPerRow * nItemsPerColumn);
			redraw();
		}
	}

	return true;
}

void Backpack::drawItem(GfxSurface &s, CItem *pItem, int nX, int nY) {
	s.blitFrom(pItem->getArt(), Common::Point(nX, nY));

	if ((pItem->m_nQuantity == 0) ||
		(pItem->m_nQuantity > 1)) {
		Common::String qty = Common::String::format("%d",
			pItem->m_nQuantity);

		s.setFontSize(8);
		s.writeString(qty, Common::Point(nX, nY), BACKPACK_BLURB_COLOR);
	}
}

int Backpack::selectedItem(const Common::Point &point) const {
	int i = -1, x, y, col, row;
	Common::Rect testRect(BACKPACK_BORDER_DX,
		BACKPACK_BORDER_DY + BACKPACK_TITLEZONE_DY,
		BACKPACK_DX - BACKPACK_BORDER_DX,
		BACKPACK_DY - BACKPACK_TEXTZONE_DY - BACKPACK_BORDER_DY);

	if (testRect.contains(point)) {
		col = (point.x - BACKPACK_BORDER_DX) / (BACKPACK_BITMAP_DX + nItem_DDX);
		row = (point.y - BACKPACK_BORDER_DY - BACKPACK_TITLEZONE_DY) / (BACKPACK_BITMAP_DY + nItem_DDY);
		x = col * (BACKPACK_BITMAP_DX + nItem_DDX) + BACKPACK_BITMAP_DX + BACKPACK_BORDER_DX;
		y = row * (BACKPACK_BITMAP_DY + nItem_DDY) + BACKPACK_BITMAP_DX + BACKPACK_BORDER_DY + BACKPACK_TITLEZONE_DY;

		if ((point.x < x) && (point.y < y))
			i = (row * nItemsPerRow) + col;
	}

	return i;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
