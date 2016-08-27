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
	startBackgroundMovie();
}

void CPetInventoryGlyph::leave() {
	stopMovie();
}

void CPetInventoryGlyph::drawAt(CScreenManager *screenManager, const Point &pt, bool isHighlighted_) {
	if (!_field34)
		return;

	if (_image) {
		if (_image->hasActiveMovie()) {
			if (isHighlighted_)
				_image->draw(screenManager);
			else
				_image->draw(screenManager, pt);
			return;
		}

		_image = nullptr;
		if (_background && isHighlighted_) {
			_background->setPosition(pt);
			startBackgroundMovie();
		}
	}

	if (_background) {
		if (isHighlighted_)
			_background->draw(screenManager);
		else
			_background->draw(screenManager, pt);
	} else if (_image) {
		_image->draw(screenManager, pt, Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	}
}


void CPetInventoryGlyph::unhighlightCurrent() {
	if (_image) {
		_image->setPosition(Point(0, 0));
		stopMovie();
	} else if (_background) {
		_background->setPosition(Point(0, 0));
		_background->loadFrame(0);
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
	if (_background && flag)
		_background->setPosition(topLeft);
}

bool CPetInventoryGlyph::dragGlyph(const Point &topLeft, CMouseDragStartMsg *msg) {
	if (!_item)
		return false;

	if (_background) {
		_field34 = 0;
		stopMovie();
	}

	CPetControl *petControl = getPetControl();
	if (!petControl)
		return false;

	CGameObject *carryParcel = petControl->getHiddenObject("CarryParcel");

	if (petControl->isSuccUBusActive() && carryParcel) {
		petControl->removeFromInventory(_item, carryParcel, false, true);
		petControl->removeFromInventory(_item, false, false);

		carryParcel->setPosition(Point(msg->_mousePos.x - carryParcel->getBounds().width() / 2,
			msg->_mousePos.y - carryParcel->getBounds().height() / 2));
		_item->setPosition(Point(SCREEN_WIDTH, SCREEN_HEIGHT));
	} else {
		petControl->removeFromInventory(_item, false, true);

		_item->setPosition(Point(msg->_mousePos.x - carryParcel->getBounds().width() / 2,
			msg->_mousePos.y - carryParcel->getBounds().height() / 2));
		_item->setVisible(true);
	}

	msg->_handled = true;
	if (msg->execute(carryParcel)) {
		_item = nullptr;
		_background = nullptr;
		_field34 = 0;
		petControl->setAreaChangeType(1);
		return true;
	} else {
		petControl->addToInventory(carryParcel);
		return false;
	}
}

void CPetInventoryGlyph::getTooltip(CPetText *text) {
	if (text) {
		text->setText("");

		if (_field34 && _item) {
			int itemIndex = populateItem(_item, 0);
			if (itemIndex >= 14 && itemIndex <= 18) {
				CPETObjectStateMsg stateMsg(0);
				stateMsg.execute(_item);

				text->setText(CString::format("%s %s",
					stateMsg._value ? "A hot " : "A cold ",
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
	CPetInventoryGlyphs *owner = static_cast<CPetInventoryGlyphs *>(_owner);
	if (!invAction)
		return false;

	switch (invAction->getMode()) {
	case ACTION_REMOVED:
		if (invAction->_item == _item) {
			_item = nullptr;
			_background = nullptr;
			_field34 = 0;
		}
		break;

	case ACTION_REMOVE:
		if (_item == invAction->_item && _owner) {
			int v = populateItem(_item, 0);
			_background = owner->getBackground(v);

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

void CPetInventoryGlyph::setItem(CGameObject *item, int val) {
	_item = item;

	if (_owner && item) {
		int v1 = populateItem(item, val);
		_background = static_cast<CPetInventoryGlyphs *>(_owner)->getBackground(v1);
		_image = static_cast<CPetInventory *>(getPetSection())->getImage(v1);
	}
}

int CPetInventoryGlyph::populateItem(CGameObject *item, int val) {
	// Scan the master item names list
	CString itemName = item->getName();
	int itemIndex = -1;
	for (int idx = 0; idx < 40 && itemIndex == -1; ++idx) {
		if (itemName == g_vm->_itemIds[idx])
			itemIndex = idx;
	}
	if (itemIndex == -1)
		return -1;

	switch (ITEM_MODES[itemIndex]) {
	case 0:
		switch (subMode(item, val)) {
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
		switch (subMode(item, val)) {
		case 0:
			return 2;
		default:
			return 3;
		}
		break;

	case 15:
		switch (subMode(item, val)) {
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
		switch (subMode(item, val)) {
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

int CPetInventoryGlyph::subMode(CGameObject *item, int val) {
	int frameNum = item->getFrameNumber();
	int movieFrame = item->getMovieFrame();

	if (val && frameNum != -1 && frameNum != movieFrame)
		item->loadFrame(frameNum);

	return frameNum;
}

void CPetInventoryGlyph::startBackgroundMovie() {
	if (_owner) {
		CPetInventory *section = static_cast<CPetInventory *>(_owner->getOwner());
		if (section)
			section->playMovie(_background, 1);
	}
}

void CPetInventoryGlyph::startForegroundMovie() {
	if (_owner) {
		CPetInventory *section = static_cast<CPetInventory *>(_owner->getOwner());
		if (section)
			section->playMovie(_image, 1);
	}
}

void CPetInventoryGlyph::stopMovie() {
	if (_owner) {
		CPetInventory *section = static_cast<CPetInventory *>(_owner->getOwner());
		if (section)
			section->playMovie(nullptr, 1);
	}
}

void CPetInventoryGlyph::reposition(const Point &pt) {
	if (_image) {
		_image->setPosition(pt);
		startForegroundMovie();
	} else if (_background) {
		_background->setPosition(pt);
		startBackgroundMovie();
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
