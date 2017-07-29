/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "titanic/pet_control/pet_inventory_glyphs.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/pet_control/pet_inventory.h"
#include "titanic/messages/pet_messages.h"
#include "titanic/titanic.h"

namespace Titanic {

const uint ITEM_MODES[40] = {
	0, 2, 11, 10, 12, 13, 9, 40, 7, 6,
	4, 5, 8, 15, 19, 24, 25, 26, 30, 20,
	21, 22, 23, 36, 39, 39, 31, 31, 32, 32,
	33, 34, 35, 38, 41, 42, 43, 44, 45, 37
};

void CPetInventoryGlyph::enter() {
	startRepeatedMovie();
}

void CPetInventoryGlyph::leave() {
	stopMovie();
}

void CPetInventoryGlyph::drawAt(CScreenManager *screenManager, const Point &pt, bool isHighlighted_) {
	if (!_active)
		return;

	if (_singular) {
		if (_singular->hasActiveMovie()) {
			if (isHighlighted_)
				_singular->draw(screenManager);
			else
				_singular->draw(screenManager, pt);
			return;
		}

		_singular = nullptr;
		if (_repeated && isHighlighted_) {
			_repeated->setPosition(pt);
			startRepeatedMovie();
		}
	}

	if (_repeated) {
		if (isHighlighted_)
			_repeated->draw(screenManager);
		else
			_repeated->draw(screenManager, pt);
	} else if (_singular) {
		_singular->draw(screenManager, pt, Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	}
}


void CPetInventoryGlyph::unhighlightCurrent() {
	if (_singular) {
		_singular->setPosition(Point(0, 0));
		stopMovie();
	} else if (_repeated) {
		_repeated->setPosition(Point(0, 0));
		_repeated->loadFrame(0);
		stopMovie();
	}
}

void CPetInventoryGlyph::highlightCurrent(const Point &pt) {
	reposition(pt);
	if (_item) {
		CPETObjectSelectedMsg selectedMsg;
		selectedMsg.execute(_item);
	}
}

void CPetInventoryGlyph::glyphFocused(const Point &topLeft, bool flag) {
	if (_repeated && flag)
		_repeated->setPosition(topLeft);
}

bool CPetInventoryGlyph::dragGlyph(const Point &topLeft, CMouseDragStartMsg *msg) {
	if (!_item)
		return false;

	if (_repeated) {
		_active = false;
		stopMovie();
	}

	CPetControl *petControl = getPetControl();
	if (!petControl)
		return false;

	CGameObject *carryParcel = petControl->getHiddenObject("CarryParcel");
	CGameObject *item = _item;

	if (petControl->isSuccUBusActive() && carryParcel) {
		petControl->removeFromInventory(_item, carryParcel, false, true);
		petControl->removeFromInventory(_item, false, false);

		carryParcel->setPosition(Point(msg->_mousePos.x - carryParcel->_bounds.width() / 2,
			msg->_mousePos.y - carryParcel->_bounds.height() / 2));
		carryParcel->setPosition(Point(SCREEN_WIDTH, SCREEN_HEIGHT));
		item = carryParcel;
	} else {
		petControl->removeFromInventory(_item, false, true);

		_item->setPosition(Point(msg->_mousePos.x - _item->_bounds.width() / 2,
			msg->_mousePos.y - _item->_bounds.height() / 2));
		_item->setVisible(true);
	}

	msg->_handled = true;
	if (msg->execute(item)) {
		_item = nullptr;
		_repeated = nullptr;
		_active = false;
		petControl->setAreaChangeType(1);
		return true;
	} else {
		petControl->addToInventory(item);
		return false;
	}
}

void CPetInventoryGlyph::getTooltip(CTextControl *text) {
	if (text) {
		text->setText("");

		if (_active && _item) {
			int itemIndex = populateItem(_item, 0);
			if (itemIndex >= 14 && itemIndex <= 18) {
				// Variations of the chicken
				CPETObjectStateMsg stateMsg(0);
				stateMsg.execute(_item);

				CString temperature = g_vm->_strings[stateMsg._value ? A_HOT : A_COLD];
				text->setText(CString::format("%s %s", temperature.c_str(),
					g_vm->_itemDescriptions[itemIndex].c_str()
				));

			} else {
				text->setText(g_vm->_itemDescriptions[itemIndex]);
			}
		}
	}
}

bool CPetInventoryGlyph::doAction(CGlyphAction *action) {
	CInventoryGlyphAction *invAction = static_cast<CInventoryGlyphAction *>(action);
	CPetInventoryGlyphs *owner = dynamic_cast<CPetInventoryGlyphs *>(_owner);
	if (!invAction)
		return false;

	switch (invAction->getMode()) {
	case ACTION_REMOVED:
		if (invAction->_item == _item) {
			_item = nullptr;
			_repeated = nullptr;
			_active = false;
		}
		break;

	case ACTION_CHANGE:
		if (_item == invAction->_item && _owner) {
			int v = populateItem(_item, 0);
			_repeated = owner->getBackground(v);

			if (isHighlighted()) {
				Point glyphPos = _owner->getHighlightedGlyphPos();
				reposition(glyphPos);
				updateTooltip();
			}
		}
		break;

	default:
		break;
	}

	return true;
}

void CPetInventoryGlyph::setItem(CGameObject *item, bool isLoading) {
	_item = item;

	if (_owner && item) {
		int idx = populateItem(item, isLoading);
		_repeated = static_cast<CPetInventoryGlyphs *>(_owner)->getBackground(idx);
		_singular = static_cast<CPetInventory *>(getPetSection())->getTransformAnimation(idx);
	}
}

int CPetInventoryGlyph::populateItem(CGameObject *item, bool isLoading) {
	// Scan the master item names list
	CString itemName = item->getName();
	int itemIndex = -1;
	for (int idx = 0; idx < 40 && itemIndex == -1; ++idx) {
		if (itemName == g_vm->_itemIds[idx])
			itemIndex = idx;
	}
	if (itemIndex == -1)
		return -1;

	// Some objects can be in multiple different states. These are handled 
	// below to give each the correct inventory glyph and description
	switch (ITEM_MODES[itemIndex]) {
	case 0:
		// Maitre d'Bot's left arm
		switch (getItemIndex(item, isLoading)) {
		case 0:
		case 1:
			return 0;
		case 2:
		case 3:
			return 1;
		default:
			return 0;
		}

	case 2:
		// Maitre d'Bot's right arm
		switch (getItemIndex(item, isLoading)) {
		case 0:
			return 2;
		default:
			return 3;
		}
		break;

	case 15:
		// Chicken
		switch (getItemIndex(item, isLoading)) {
		case 0:
		case 1:
			return 14;
		case 2:
			return 16;
		case 3:
			return 15;
		case 4:
			return 17;
		case 5:
			return 18;
		default:
			return 15;
		}
		break;

	case 26:
		// Beer glass
		switch (getItemIndex(item, isLoading)) {
		case 0:
			return 26;
		case 1:
			return 29;
		case 2:
			return 28;
		case 3:
			return 27;
		default:
			return 26;
		}
		break;

	default:
		break;
	}

	return ITEM_MODES[itemIndex];
}

int CPetInventoryGlyph::getItemIndex(CGameObject *item, bool isLoading) {
	int frameNum = item->getFrameNumber();
	int movieFrame = item->getMovieFrame();

	if (isLoading && frameNum != -1 && frameNum != movieFrame) {
		item->loadFrame(frameNum);
		movieFrame = frameNum;
	}

	return movieFrame;
}

void CPetInventoryGlyph::startRepeatedMovie() {
	if (_owner) {
		CPetInventory *section = dynamic_cast<CPetInventory *>(_owner->getOwner());
		if (section)
			section->playMovie(_repeated, true);
	}
}

void CPetInventoryGlyph::startSingularMovie() {
	if (_owner) {
		CPetInventory *section = dynamic_cast<CPetInventory *>(_owner->getOwner());
		if (section)
			section->playMovie(_singular, false);
	}
}

void CPetInventoryGlyph::stopMovie() {
	if (_owner) {
		CPetInventory *section = dynamic_cast<CPetInventory *>(_owner->getOwner());
		if (section)
			section->playMovie(nullptr);
	}
}

void CPetInventoryGlyph::reposition(const Point &pt) {
	if (_singular) {
		// Special transformation of item to piece of Titania
		_singular->setPosition(pt);
		startSingularMovie();
	} else if (_repeated) {
		// Standard repeating animation
		_repeated->setPosition(pt);
		startRepeatedMovie();
	}
}

/*------------------------------------------------------------------------*/

bool CPetInventoryGlyphs::doAction(CInventoryGlyphAction *action) {
	for (iterator i = begin(); i != end(); ++i) {
		(*i)->doAction(action);
	}

	return true;
}

CGameObject *CPetInventoryGlyphs::getBackground(int index) {
	return _owner ? _owner->getBackground(index) : nullptr;
}

} // End of namespace Titanic
