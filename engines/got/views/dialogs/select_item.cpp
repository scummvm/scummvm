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

#include "got/views/dialogs/select_item.h"
#include "got/vars.h"

namespace Got {
namespace Views {
namespace Dialogs {

#define _HRZSP 24

static const char *ITEM_NAMES[] = {
	"Enchanted Apple", "Lightning Power", "Winged Boots",
	"Wind Power", "Amulet of Protection", "Thunder Power"
};

SelectItem::SelectItem() : Dialog("SelectItem") {
	_selectedItem = -1;
	
	setBounds(Common::Rect(56, 48, 264, 160));
}

void SelectItem::draw() {
	Dialog::draw();
	GfxSurface s = getSurface();

	if (_G(thor_info)._inventory == 0) {
		s.print(Common::Point(44, 52), "No Items Found", 14);
		return;
	}

	uint b = 1;
	for (int l = 0; l < 7; l++, b = b << 1) {
		if (_G(thor_info)._inventory & b) {
			if (l < 6)
				s.simpleBlitFrom(_G(objects[l + 26]), Common::Point(82 - 56 + (l * _HRZSP), 72 - 48));
			else {
				const int objId = _G(thor_info)._object + 10;
				s.simpleBlitFrom(_G(objects[objId]), Common::Point(82 - 56 + (l * _HRZSP), 72 - 48));
			}
		}
	}

	const char *objName;
	if (_selectedItem < 6)
		objName = ITEM_NAMES[_selectedItem];
	else
		objName = _G(thor_info)._objectName;

	s.print(Common::Point((s.w - (strlen(objName) * 8)) / 2, 66), objName, 12);
	s.frameRect(Common::Rect(26 + (_selectedItem * _HRZSP), 22,
							 43 + (_selectedItem * _HRZSP), 42),
				15);
}

bool SelectItem::msgFocus(const FocusMessage &msg) {
	if (_G(thor_info)._inventory == 0) {
		_selectedItem = -1;
	} else {
		_selectedItem = _G(thor_info)._selectedItem - 1;
		if (_selectedItem < 1)
			_selectedItem = 0;

		uint b = 1 << _selectedItem;
		for (;;) {
			if (_G(thor_info)._inventory & b)
				break;
			if (_selectedItem < 7)
				_selectedItem++;
			else
				_selectedItem = 0;

			b = 1 << _selectedItem;
		}
	}

	return true;
}

bool SelectItem::msgAction(const ActionMessage &msg) {
	uint b;

	if (_G(thor_info)._inventory == 0) {
		close();
		return true;
	}

	switch (msg._action) {
	case KEYBIND_ESCAPE:
		close();
		break;

	case KEYBIND_SELECT:
	case KEYBIND_FIRE:
	case KEYBIND_MAGIC:
		selectItem();
		break;

	case KEYBIND_LEFT:
		for (;;) {
			if (_selectedItem > 0)
				_selectedItem--;
			else
				_selectedItem = 8;

			b = 1 << _selectedItem;
			if (_G(thor_info)._inventory & b)
				break;
		}

		playSound(WOOP, true);
		redraw();
		break;

	case KEYBIND_RIGHT:
		while (1) {
			if (_selectedItem < 9)
				_selectedItem++;
			else
				_selectedItem = 0;

			b = 1 << _selectedItem;
			if (_G(thor_info)._inventory & b)
				break;
		}

		playSound(WOOP, true);
		redraw();
		break;

	default:
		break;
	}

	return true;
}

void SelectItem::selectItem() {
	_G(thor_info)._selectedItem = _selectedItem + 1;
	close();
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
