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

#include "bagel/hodjnpodj/metagame/boardgame/pawn_shop.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define DIALOG_DX				502
#define DIALOG_DY				395
#define	DIALOG_CURL_DX		500
#define	DIALOG_CURL_DY		50
#define DIALOG_BORDER_DX		60
#define DIALOG_BORDER_DY		80

#define DIALOG_TITLEZONE_DY	10
#define DIALOG_TEXTZONE_DX	75
#define DIALOG_TEXTZONE_DY	30
#define DIALOG_TEXTZONE_DDY	10

#define DIALOG_BITMAP_DX		59
#define DIALOG_BITMAP_DY		59
#define DIALOG_BITMAP_DDX		10
#define DIALOG_BITMAP_DDY		9

#define	DIALOG_FONT_SIZE		14
#define DIALOG_BLURB_SIZE		8
#define	DIALOG_TEXT_COLOR		PALETTERGB(128,0,128)
#define	DIALOG_BLURB_COLOR	PALETTERGB(0,0,255)
#define DIALOG_MORE_COLOR		PALETTERGB(0,0,0)

#define TEXT_MORE_DX		120						// offset of "more" indicator from right margin
#define TEXT_MORE_DY		5                       // offset of "more" indicator bottom of scroll
#define MORE_TEXT_BLURB		"[ More ]"				// actual text to display for "more" indicator
#define BUY_MESSAGE_TIMEOUT	100
#define IDC_PAWN_DOLLAR		941

PawnShop::PawnShop() : Dialog("PawnShop", "meta/hnpmeta.dll"),
		_okButton(Common::Rect(210, 355, 290, 380), this) {
}

void PawnShop::show(CInventory *pStore, CInventory *pInvent) {
	PawnShop *view = dynamic_cast<PawnShop *>(
		g_events->findView("PawnShop"));
	view->pGeneralStore = pStore;
	view->pInventory = pInvent;
	view->addView();
}

bool PawnShop::msgOpen(const OpenMessage &msg) {
	Dialog::msgOpen(msg);

	if (!pGeneralStore) {
		// For debug purposes
		pGeneralStore = lpMetaGame->m_cHodj.m_pBlackMarket;
		pInventory = lpMetaGame->m_cHodj.m_pInventory;
	}

	bPlayingHodj = pInventory->FindItem(MG_OBJ_HODJ_NOTEBOOK) != nullptr;

	_scrollTopRect = Common::Rect(0, 0, 501, 48);
	_scrollBottomRect = Common::Rect(0, 0, 501, 47);

	GfxSurface s = getSurface();
	s.setFontSize(DIALOG_FONT_SIZE);
	_titleRect = RectWH(0,
		DIALOG_TOP + DIALOG_TITLEZONE_DY,
		_bounds.width(), s.getStringHeight());

	s.setFontSize(DIALOG_BLURB_SIZE);
	_blurbRect = RectWH(0,
		_bounds.height() - DIALOG_BOTTOM -
		(s.getStringHeight() + 5) * 2 - 5,
		_bounds.width(),
		(s.getStringHeight() + 5) * 2);

	// Scroll rects will be compared against mouse pos,
	// so we need to shift them to global screen co-ordinates
	_scrollTopRect.moveTo(_bounds.left, _bounds.top);
	_scrollBottomRect.moveTo(_bounds.left,
		_bounds.bottom - _scrollBottomRect.height());

	updateContent();

	return true;
}

bool PawnShop::msgClose(const CloseMessage &msg) {
	Dialog::msgClose(msg);
	return true;
}

bool PawnShop::msgAction(const ActionMessage &msg) {
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
		if (nFirstSlot + (nItemsPerRow * nItemsPerColumn) < pInventory->ItemCount()) {
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

bool PawnShop::msgGame(const GameMessage &msg) {
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

bool PawnShop::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_HOME:
		// Go to first item
		if (nFirstSlot != 0) {
			nFirstSlot = 0;
			redraw();
		}
		break;

	case Common::KEYCODE_END:								// go to last page of text
		nFirstSlot = pInventory->ItemCount() - (nItemsPerRow * nItemsPerColumn);
		if (nFirstSlot < 0)
			nFirstSlot = 0;
		redraw();
		break;

	default:
		return false;
	}

	return true;
}

bool PawnShop::tick() {
	Dialog::tick();

	if (_sellMessageCtr && !--_sellMessageCtr) {
		_sellMessage.clear();
		redraw();
	}

	return true;
}

void PawnShop::draw() {
	Dialog::draw();

	GfxSurface s = getSurface();
	s.setFontSize(DIALOG_FONT_SIZE);
	s.writeString("Pawn Shop", _titleRect,
		DIALOG_TEXT_COLOR, Graphics::kTextAlignCenter);

	drawItems(s);

	s.setFontSize(DIALOG_BLURB_SIZE);

	if (!_sellMessage.empty()) {
		s.writeString(_sellMessage, _blurbRect, DIALOG_TEXT_COLOR,
			Graphics::kTextAlignCenter);
	} else if (_selectedItem) {
		drawBlurb(s);
	} else {
		drawFinances(s);
	}
}

void PawnShop::updateContent() {
	if (pInventory->ItemCount() <= 0)
		return;

	// Calculate the horizontal space we have available
	nDialog_DX = _bounds.width() - (DIALOG_BORDER_DX << 1);
	// Estimate number of items that will fit
	nItemsPerRow = nDialog_DX / DIALOG_BITMAP_DX;

	for (;;) {
		nItem_DDX = (nDialog_DX - (nItemsPerRow * DIALOG_BITMAP_DX)) / (nItemsPerRow - 1);	// now evaluate the distance that would occur between
		if ((nItem_DDX >= DIALOG_BITMAP_DDX) ||               // ... items, and if is less than the minimum allowed
			(nItemsPerRow == 1))
			break;                                              // ... then reduce the count of items per row
		nItemsPerRow -= 1;
	}

	// calculate the vertical space we have available
	nDialog_DY = DIALOG_DY - DIALOG_TEXTZONE_DY - (DIALOG_BORDER_DY << 1) - DIALOG_TITLEZONE_DY;
	// Estimate number of items that will fit
	nItemsPerColumn = nDialog_DY / DIALOG_BITMAP_DY;

	for (;;) {
		nItem_DDY = (nDialog_DY - (nItemsPerColumn * DIALOG_BITMAP_DY)) / (nItemsPerColumn - 1);    // now evaluate the distance that would occur between
		if ((nItem_DDY >= DIALOG_BITMAP_DDY) ||               // ... items, and if is less than the minimum allowed
			(nItemsPerColumn == 1))
			break;                                              // ... then reduce the count of items per column
		nItemsPerColumn -= 1;
	}
}

Common::Rect PawnShop::getItemRect(int index) const {
	int x = (index % nItemsPerRow);
	x *= (DIALOG_BITMAP_DX + nItem_DDX);                      // ... allowing proper spacing between items
	int y = (index / nItemsPerRow);                                 // calculate its vertical position
	y *= (DIALOG_BITMAP_DY + nItem_DDY);                      // ... allowing proper spacing between items

	x += DIALOG_BORDER_DX;
	y += DIALOG_BORDER_DY + DIALOG_TITLEZONE_DY;

	return Common::Rect(x, y, x + DIALOG_BITMAP_DX,
		y + DIALOG_BITMAP_DY);
}

void PawnShop::drawItems(GfxSurface &s) {
	// Get first item on this page
	auto pItem = pInventory->FetchItem(nFirstSlot);

	for (int i = 0; (i < (nItemsPerRow * nItemsPerColumn)) && (pItem != nullptr); i++) {							// will thumb through all of them
		const Common::Rect r = getItemRect(i);
		drawItem(s, pItem, r.left, r.top);
		pItem = pItem->GetNext();
	}
}

void PawnShop::drawMore(GfxSurface &s) {
	if (!hasNextPage())
		return;

	int x = DIALOG_DX - TEXT_MORE_DX;	                            // ... that can be scrolled through
	int y = DIALOG_DY -
		DIALOG_CURL_DY +
		((DIALOG_CURL_DY - s.getStringHeight()) >> 1) -
		TEXT_MORE_DY;

	s.writeString(MORE_TEXT_BLURB, Common::Point(x, y),
		DIALOG_MORE_COLOR);
}

void PawnShop::drawBlurb(GfxSurface &s) {
	s.writeString(_selectedItem->GetDescription(),
		_blurbRect, DIALOG_TEXT_COLOR,
		Graphics::kTextAlignCenter);

	Common::Rect costRect(_blurbRect.left,
		(_blurbRect.top + _blurbRect.bottom) / 2,
		_blurbRect.right, _blurbRect.bottom);
	Common::String msg;
	int nPrice = _selectedItem->GetValue();

	if (nPrice == 0)
		msg = "That can't be sold here";
	else {
		if (_selectedItem->GetQuantity() == 1) {
			if (nPrice == 1)
				msg = "It can be sold for 1 Crown";
			else
				msg = Common::String::format(
					"It can be sold for %d Crowns", nPrice);
		} else {
			if (nPrice == 1)
				msg = "One can be sold for 1 Crown";
			else
				msg = Common::String::format(
					"One can be sold for %d Crowns", nPrice);
		}
	}

	s.writeString(msg, costRect, DIALOG_TEXT_COLOR,
		Graphics::kTextAlignCenter);
}

void PawnShop::drawFinances(GfxSurface &s) {
	Common::Rect msgRect(_blurbRect.left,
		(_blurbRect.top + _blurbRect.bottom) / 2,
		_blurbRect.right, _blurbRect.bottom);
	CItem *pItem = pInventory->FindItem(MG_OBJ_CROWN);
	Common::String msg;

	if ((pItem == nullptr) || ((*pItem).GetQuantity() < 1)) {
		if (bPlayingHodj)
			msg = "Hodj has no Crowns";
		else
			msg = "Podj has no Crowns";
	} else {
		if (bPlayingHodj)
			msg = Common::String::format("Hodj has %ld Crowns",
				pItem->GetQuantity());
		else
			msg = Common::String::format("Podj has %ld Crowns",
				pItem->GetQuantity());
	}

	s.writeString(msg, msgRect, DIALOG_TEXT_COLOR,
		Graphics::kTextAlignCenter);
}

bool PawnShop::hasNextPage() const {
	return (nFirstSlot +
		(nItemsPerRow * nItemsPerColumn)) <
		pInventory->ItemCount();
}

bool PawnShop::msgMouseMove(const MouseMoveMessage &msg) {
	int oldIndex = _selectedIndex;
	int index = oldIndex;
	_selectedItem = nullptr;

	if (!Dialog::msgMouseMove(msg)) {
		if (_scrollTopRect.contains(msg._pos)) {
			g_events->setCursor(IDC_RULES_INVALID);
		} else if (_scrollBottomRect.contains(msg._pos)) {
			g_events->setCursor(IDC_RULES_INVALID);
		} else {
			// Check for highlighted item
			g_events->setCursor(IDC_ARROW);

			index = getItemAtPos(msg._pos);
			if (index >= 0) {
				_selectedItem = pInventory->FetchItem(index);
				if (_selectedItem != nullptr &&
						_selectedItem->GetValue() != 0) {
					g_events->setCursor(IDC_PAWN_DOLLAR);
				}
			}
		}
	}

	if (index != oldIndex)
		redraw();
	_selectedIndex = index;

	return true;
}

bool PawnShop::msgMouseUp(const MouseUpMessage &msg) {
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
		} else if (_selectedItem && _selectedItem->GetValue() != 0) {
			sellItem();
		}
	}

	return true;
}

void PawnShop::drawItem(GfxSurface &s, CItem *pItem, int nX, int nY) {
	s.blitFrom(pItem->getArt(), Common::Point(nX, nY));

	if ((pItem->m_nQuantity == 0) ||
		(pItem->m_nQuantity > 1)) {
		Common::String qty = Common::String::format("%d",
			pItem->m_nQuantity);

		s.setFontSize(8);
		s.writeString(qty, Common::Point(nX, nY), DIALOG_BLURB_COLOR);
	}
}

int PawnShop::getItemAtPos(const Common::Point &point) const {
	Common::Point p(point.x - _bounds.left, point.y - _bounds.top);
	const int itemCount = pInventory->ItemCount();

	for (int i = 0; i < (nItemsPerRow * nItemsPerColumn) &&
			(nFirstSlot + i) < itemCount; ++i) {
		const Common::Rect r = getItemRect(i);

		if (r.contains(p))
			return nFirstSlot + i;
	}

	return -1;
}

void PawnShop::sellItem() {
	int nPrice = _selectedItem->GetValue();
	CSound *pSound = new CSound(this,
		(bPlayingHodj ? "meta/sound/gsps7.wav" : "meta/sound/gsps8.wav"),
		SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);
	pSound->play();

	pInventory->AddItem(MG_OBJ_CROWN, nPrice);

	if (_selectedItem->GetQuantity() > 1) {
		(*pInventory).DiscardItem(_selectedItem, 1);
		pGeneralStore->AddItem(_selectedItem->GetID(), 1);
	} else {
		(*pInventory).RemoveItem(_selectedItem);
		pGeneralStore->AddItem(_selectedItem);
	}

	_sellMessageCtr = BUY_MESSAGE_TIMEOUT;
	_sellMessage = "Thank you!!!";
	redraw();
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
