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

#define MORE_TEXT_BLURB		"[ More ]"				// actual text to display for "more" indicator

#define BACKPACK_TITLEZONE_DY	10

#define BACKPACK_TEXTZONE_DX	75
#define BACKPACK_TEXTZONE_DY	30
#define BACKPACK_TEXTZONE_DDY	10

Backpack::Backpack() : Dialog("Backpack"),
		_okButton(Common::Rect(210, 355, 290, 380), this),
		pInventory(lpMetaGame->m_cHodj.m_pInventory) {
}

bool Backpack::msgOpen(const OpenMessage &msg) {
	Dialog::msgOpen(msg);

	_scrollTopRect = Common::Rect(0, 0, 501, 48);
	_scrollBottomRect = Common::Rect(0, 0, 501, 47);

	GfxSurface s = getSurface();
	s.setFontSize(14);
	_titleRect = RectWH(0,
		DIALOG_TOP + BACKPACK_TITLEZONE_DY,
		_bounds.width(), s.getStringHeight());

	// Scroll rects will be compared against mouse pos,
	// so we need to shift them to global screen co-ordinates
	_scrollTopRect.moveTo(_bounds.left, _bounds.top);
	_scrollBottomRect.moveTo(_bounds.left,
		_bounds.bottom - _scrollBottomRect.height());

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
	s.setFontSize(14);
	s.writeString(pInventory->GetTitle(), _titleRect,
		PURPLE, Graphics::kTextAlignCenter);
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
