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

#include "common/util.h"
#include "graphics/managed_surface.h"
#include "dgds/inventory.h"
#include "dgds/dgds.h"
#include "dgds/scene.h"
#include "dgds/image.h"
#include "dgds/font.h"
#include "dgds/request.h"
#include "dgds/includes.h"
#include "dgds/globals.h"

namespace Dgds {

/*static*/ const byte Inventory::HOC_CHARACTER_QUALS[] = { 0, 9, 7, 8 };


Inventory::Inventory() : _isOpen(false), _prevPageBtn(nullptr), _nextPageBtn(nullptr),
	_invClock(nullptr), _itemZoomBox(nullptr), _exitButton(nullptr), _clockSkipMinBtn(nullptr),
	_itemArea(nullptr), _clockSkipHrBtn(nullptr), _dropBtn(nullptr), _itemBox(nullptr),
	_giveToBtn(nullptr), _changeCharBtn(nullptr), _highlightItemNo(-1), _itemOffset(0),
	_openedFromSceneNum(0), _showZoomBox(false), _fullWidth(-1)
{
}

void Inventory::open() {
	// Allow double-open because that's how the inventory shows item
	// descriptions.
	_isOpen = true;
	DgdsEngine *engine = DgdsEngine::getInstance();

	int curScene = engine->getScene()->getNum();
	if (engine->getGameId() == GID_WILLY) {
		_openedFromSceneNum = curScene;
		return;
	}

	if (curScene != 2) {
		_openedFromSceneNum = curScene;
		engine->changeScene(2);
	} else {
		engine->getScene()->runEnterSceneOps();
	}
}

void Inventory::close() {
	if (!_isOpen)
		return;

	_isOpen = false;
	DgdsEngine *engine = DgdsEngine::getInstance();
	if (engine->getGameId() != GID_WILLY) {
		assert(_openedFromSceneNum != 0);
		engine->changeScene(_openedFromSceneNum);
	}

	_showZoomBox = false;
	_openedFromSceneNum = 0;
	_highlightItemNo = -1;
}

void Inventory::setRequestData(const REQFileData &data) {
	_reqData = data;
	if (_reqData._requests.empty()) {
		warning("No inventory request data to load");
		return;
	}

	RequestData &req = _reqData._requests[0];
	_prevPageBtn = dynamic_cast<ButtonGadget *>(req.findGadgetByNumWithFlags3Not0x40(14));
	_nextPageBtn = dynamic_cast<ButtonGadget *>(req.findGadgetByNumWithFlags3Not0x40(15));
	_invClock = dynamic_cast<TextAreaGadget *>(req.findGadgetByNumWithFlags3Not0x40(23));
	_itemBox = req.findGadgetByNumWithFlags3Not0x40(8);
	_itemZoomBox = req.findGadgetByNumWithFlags3Not0x40(9);
	_exitButton = dynamic_cast<ButtonGadget *>(req.findGadgetByNumWithFlags3Not0x40(17));

	_clockSkipMinBtn = dynamic_cast<ButtonGadget *>(req.findGadgetByNumWithFlags3Not0x40(24));
	_clockSkipHrBtn = dynamic_cast<ButtonGadget *>(req.findGadgetByNumWithFlags3Not0x40(25));

	_giveToBtn = dynamic_cast<ButtonGadget *>(req.findGadgetByNumWithFlags3Not0x40(29));
	_changeCharBtn = dynamic_cast<ButtonGadget *>(req.findGadgetByNumWithFlags3Not0x40(27));

	_dropBtn = dynamic_cast<ButtonGadget *>(req.findGadgetByNumWithFlags3Not0x40(16));
	_itemArea = dynamic_cast<ImageGadget *>(req.findGadgetByNumWithFlags3Not0x40(8));

	_fullWidth = req._rect.width;

	// TODO! Beamish doesn't have a zoom box, or it's a different ID?
	if (DgdsEngine::getInstance()->getGameId() == GID_WILLY)
		_itemZoomBox = _itemBox;

	if (!_prevPageBtn || !_nextPageBtn || !_itemZoomBox || !_exitButton || !_itemArea)
		error("Didn't get all expected inventory gadgets");
}

void Inventory::drawHeader(Graphics::ManagedSurface &surf) {
	// This really should be a text area, but it's hard-coded in the game.
	const DgdsFont *font = RequestData::getMenuFont();
	const RequestData &r = _reqData._requests[0];

	static const char *title;

	if (DgdsEngine::getInstance()->getGameLang() == Common::EN_ANY)
		title = "INVENTORY";
	else if (DgdsEngine::getInstance()->getGameLang() == Common::DE_DEU)
		title = "INVENTAR";
	else
		error("Unsupported language %d", DgdsEngine::getInstance()->getGameLang());

	DgdsGameId gameId = DgdsEngine::getInstance()->getGameId();
	byte txtColor = (gameId == GID_HOC ? 25 : 0);

	int titleWidth = font->getStringWidth(title);
	// Dragon always draws the header in the same spot; HoC centers it.
	int x1, y1;
	if (gameId == GID_DRAGON) {
		// Constant offset
		x1 = r._rect.x + 112;
		y1 = r._rect.y + 7;
	} else if (gameId == GID_HOC) {
		// Centered on window
		x1 = r._rect.x + (r._rect.width - font->getStringWidth(title)) / 2 - 3;
		y1 = r._rect.y + 11;
	} else { // GID_WILLY
		// Constant offset
		x1 = r._rect.x + 154;
		y1 = r._rect.y + 8;
	}

	// Draw the border around the text
	byte topColor = (gameId == GID_DRAGON ? 0xdf : (gameId == GID_HOC ? 16 : 15));
	byte botColor = (gameId == GID_DRAGON ? 0xff : (gameId == GID_HOC ? 20 : 19));
	int x2 = x1 + titleWidth + 6;
	int y2 = y1 + font->getFontHeight();
	surf.drawLine(x1, y1, x2, y1, topColor);
	surf.drawLine(x2, y1 + 1, x2, y2, topColor);
	surf.drawLine(x1, y1 + 1, x1, y2, botColor);
	surf.drawLine(x1 + 1, y2, x1 + titleWidth + 5, y2, botColor);

	// In willy also fill the area in the middle
	if (gameId == GID_WILLY)
		surf.fillRect(Common::Rect(Common::Point(x1 + 1, y1 + 1), titleWidth + 5, font->getFontHeight() - 1), 17);

	font->drawString(&surf, title, x1 + 4, y1 + 2, titleWidth, txtColor);

}

void Inventory::draw(Graphics::ManagedSurface &surf, int itemCount) {
	RequestData &boxreq = _reqData._requests[0];
	DgdsEngine *engine = DgdsEngine::getInstance();
	DgdsGameId gameId = engine->getGameId();

	if (_showZoomBox) {
		if (gameId != GID_WILLY)
			_itemZoomBox->setVisible(true);
		boxreq._rect.width = _fullWidth;
	} else {
		boxreq._rect.width = _itemBox->_width + _itemBox->_x * 2;
		if (gameId != GID_WILLY) {
			_itemZoomBox->setVisible(false);
			boxreq._rect.width--;
		}
	}

	//
	// Decide whether the nextpage/prevpage buttons should be visible
	//
	bool needPageButtons =
		(_itemArea->_width / _itemArea->_xStep) *
			(_itemArea->_height / _itemArea->_yStep) < itemCount;
	_prevPageBtn->setVisible(needPageButtons);
	_nextPageBtn->setVisible(needPageButtons);

	//
	// Decide whether the time buttons should be visible (only in Dragon and Willy Beamish)
	// In Willy Beamish the buttons are controlled by a global.
	//
	bool showTimeButtons;
	switch (gameId) {
		case GID_DRAGON:
			showTimeButtons = true;
			break;
		case GID_WILLY:
			showTimeButtons = static_cast<WillyGlobals *>(engine->getGameGlobals())->isDrawTimeSkipButtons();
			break;
		default:
			showTimeButtons = false;
			break;
	}
	if (_clockSkipMinBtn)
		_clockSkipMinBtn->setVisible(showTimeButtons);
	if (_clockSkipHrBtn)
		_clockSkipHrBtn->setVisible(showTimeButtons);

	//
	// Decide whether the give-to and swap char buttons should be visible (only in China)
	//
	int16 otherChar = 0;
	if (gameId == GID_HOC) {
		otherChar = engine->getGDSScene()->getGlobal(0x34);
		_giveToBtn->setVisible(otherChar != 0);
		// This is only used to give the location so it's always false.
		_changeCharBtn->setVisible(false);
	}

	boxreq.drawInvType(&surf);

	if (gameId == GID_HOC && otherChar != 0) {
		int16 swapCharIcon = DgdsEngine::HOC_CHAR_SWAP_ICONS[otherChar];
		Common::Point pt = _changeCharBtn->topLeft();
		engine->getIcons()->drawBitmap(swapCharIcon, pt.x, pt.y, boxreq._rect.toCommonRect(), surf);
	}

	drawHeader(surf);
	drawTime(surf);
	drawItems(surf);
}

void Inventory::drawTime(Graphics::ManagedSurface &surf) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	if (engine->getGameId() == GID_HOC)
		return;

	const DgdsFont *font = RequestData::getMenuFont();
	const Common::String timeStr = engine->getClock().getTimeStr();
	const Common::Point clockpos = _invClock->topLeft();
	if (engine->getGameId() == GID_DRAGON) {
		surf.fillRect(Common::Rect(clockpos, _invClock->_width, _invClock->_height), 0);
		RequestData::drawCorners(&surf, 19, clockpos.x - 2, clockpos.y - 2,
					_invClock->_width + 4, _invClock->_height + 4);
	} else { // GID_WILLY
		surf.fillRect(Common::Rect(Common::Point(clockpos.x, clockpos.y - 1),
					_invClock->_width, _invClock->_height + 2), 0);
		RequestData::drawCorners(&surf, 25, clockpos.x - 2, clockpos.y - 5,
					_invClock->_width + 8, _invClock->_height + 7);
	}
	font->drawString(&surf, timeStr, clockpos.x + 4, clockpos.y, font->getStringWidth(timeStr), _invClock->_col3);
}

void Inventory::drawItems(Graphics::ManagedSurface &surf) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	const Common::SharedPtr<Image> &icons = engine->getIcons();
	int x = (engine->getGameId() == GID_WILLY ? -2 : 0);
	int y = 0;

	const int xstep = _itemArea->_xStep;
	const int ystep = _itemArea->_yStep;
	const int imgAreaX = _itemArea->_parentX + _itemArea->_x;
	const int imgAreaY = _itemArea->_parentY + _itemArea->_y;

	Common::Rect itemRect(Common::Point(imgAreaX, imgAreaY), _itemArea->_width, _itemArea->_height);

	if (!icons)
		return;

	// TODO: does this need to be adjusted ever?
	const Common::Rect drawMask(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	int offset = _itemOffset;
	Common::Array<GameItem> &items = engine->getGDSScene()->getGameItems();
	for (auto &item: items) {
		if (!isItemInInventory(item))
			continue;

		if (offset) {
			offset--;
			continue;
		}

		if (item._num == _highlightItemNo) {
			// draw highlighted
			Common::Rect highlightRect(Common::Point(imgAreaX + x, imgAreaY + y), xstep, ystep);
			surf.fillRect(highlightRect, 4);
		}

		// Update the icon bounds - Note: original doesn't do this here, but if we don't then
		// the Napent in Dragon is weirdly offset in y because its rect has a large height?
		Common::SharedPtr<Graphics::ManagedSurface> icon = icons->getSurface(item._iconNum);
		if (icon) {
			item._rect.width = MIN((int)icon->w, item._rect.width);
			item._rect.height = MIN((int)icon->h, item._rect.height);
		}

		// calculate draw offset for the image
		int drawX = imgAreaX + x + (xstep - item._rect.width) / 2;
		int drawY = imgAreaY + y + (ystep - item._rect.height) / 2;

		icons->drawBitmap(item._iconNum, drawX, drawY, drawMask, surf);

		item._rect.x = drawX;
		item._rect.y = drawY;

		x += xstep;

		// Willy Beamish area is 270 and step is 54, so hack the width slightly.
		if (x >= _itemArea->_width - 2) {
			x = 0;
			y += ystep;
		}
		if (y >= _itemArea->_height) {
			break;
		}
	}
}

void Inventory::mouseMoved(const Common::Point &pt) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	GameItem *dragItem = engine->getScene()->getDragItem();
	if (dragItem) {
		engine->setMouseCursor(dragItem->_iconNum);
		const RequestData &req = _reqData._requests[0];
		if (!req._rect.contains(pt)) {
			// dragged an item outside the inventory
			dragItem->_inSceneNum = _openedFromSceneNum;
			close();
		}
	} else {
		engine->setMouseCursor(kDgdsMouseGameDefault);
	}
}

GameItem *Inventory::itemUnderMouse(const Common::Point &pt) {
	if (!_itemArea)
		return nullptr;

	DgdsEngine *engine = DgdsEngine::getInstance();
	Common::Array<GameItem> &items = engine->getGDSScene()->getGameItems();
	if (_itemArea->containsPoint(pt)) {
		const int imgAreaX = _itemArea->_parentX + _itemArea->_x;
		const int imgAreaY = _itemArea->_parentY + _itemArea->_y;
		const int numAcross = _itemArea->_width / _itemArea->_xStep;
		const int itemrow = (pt.y - imgAreaY) / _itemArea->_yStep;
		const int itemcol = (pt.x - imgAreaX) / _itemArea->_xStep;
		int itemnum = numAcross * itemrow + itemcol + _itemOffset;

		for (auto &item: items) {
			if (!isItemInInventory(item))
				continue;

			if (itemnum) {
				itemnum--;
				continue;
			}
			return &item;
		}
	}
	return nullptr;
}

bool Inventory::isItemInInventory(const GameItem &item) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	DgdsGameId gameId = engine->getGameId();
	bool result = item._inSceneNum == 2; // && (item._flags & 4)
	if (gameId == GID_HOC) {
		int16 currentCharacter = engine->getGDSScene()->getGlobal(0x33);
		assert(currentCharacter < 4);
		result = result && item._quality == HOC_CHARACTER_QUALS[currentCharacter];
	}

	return result;
}

void Inventory::mouseLDown(const Common::Point &pt) {
	RequestData &boxreq = _reqData._requests[0];

	// Ignore this, and close on mouseup.
	if (!boxreq._rect.contains(pt))
		return;

	DgdsEngine *engine = DgdsEngine::getInstance();

	if (engine->getScene()->hasVisibleDialog() || !_itemBox->containsPoint(pt)) {
		return engine->getScene()->mouseLDown(pt);
	} else {
		GameItem *underMouse = itemUnderMouse(pt);
		if (underMouse) {
			_highlightItemNo = underMouse->_num;
			engine->getScene()->runOps(underMouse->onLDownOps);
			engine->getScene()->setDragItem(underMouse);
			if (underMouse->_iconNum)
				engine->setMouseCursor(underMouse->_iconNum);
		}
	}
}

void Inventory::mouseLUp(const Common::Point &pt) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	GameItem *dragItem = engine->getScene()->getDragItem();

	if (dragItem) {
		if (engine->getGameId() == GID_WILLY)
			dragItem->_inSceneNum = 2;
		engine->getScene()->onDragFinish(pt);
		return;
	}

	GDSScene *gds = engine->getGDSScene();

	engine->setMouseCursor(kDgdsMouseGameDefault);

	int itemsPerPage = (_itemArea->_width / _itemArea->_xStep) * (_itemArea->_height / _itemArea->_yStep);
	if (_exitButton->containsPoint(pt)) {
		close();
	} else if (_nextPageBtn->containsPoint(pt) && _nextPageBtn->isVisible()) {
		int numInvItems = 0;
		const Common::Array<GameItem> &items = engine->getGDSScene()->getGameItems();
		for (auto &item: items) {
			if (isItemInInventory(item))
				numInvItems++;
		}
		if (_itemOffset < numInvItems && _itemOffset + itemsPerPage <= numInvItems)
			_itemOffset += itemsPerPage;
	} else if (_prevPageBtn->containsPoint(pt) && _prevPageBtn->isVisible()) {
		if (_itemOffset > 0)
			_itemOffset -= itemsPerPage;
	} else if (_clockSkipMinBtn && _clockSkipMinBtn->isVisible() && _clockSkipMinBtn->containsPoint(pt)) {
		engine->getClock().addGameTime(1);
	} else if (_clockSkipHrBtn && _clockSkipHrBtn->isVisible() && _clockSkipHrBtn->containsPoint(pt)) {
		engine->getClock().addGameTime(60);
	} else if (_giveToBtn && _giveToBtn->isVisible() && _giveToBtn->containsPoint(pt)) {
		Common::Array<GameItem> &items = engine->getGDSScene()->getGameItems();
		for (auto &item: items) {
			if (item._num == _highlightItemNo) {
				item._quality = HOC_CHARACTER_QUALS[gds->getGlobal(0x34)];
				break;
			}
		}
	} else if (_changeCharBtn && _changeCharBtn->containsPoint(pt)) {
		int16 prevChar = gds->getGlobal(0x33);
		gds->setGlobal(0x33, gds->getGlobal(0x34));
		gds->setGlobal(0x34, prevChar);
	} else if (_dropBtn && _dropBtn->containsPoint(pt) && _highlightItemNo >= 0) {
		Common::Array<GameItem> &items = engine->getGDSScene()->getGameItems();
		for (auto &item: items) {
			if (item._num == _highlightItemNo) {
				item._inSceneNum = _openedFromSceneNum;
				break;
			}
		}
	}
}

void Inventory::mouseRUp(const Common::Point &pt) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	if (_itemBox->containsPoint(pt)) {
		GameItem *underMouse = itemUnderMouse(pt);
		if (underMouse) {
			setShowZoomBox(true);
			if (engine->getGameId() == GID_HOC) {
				// Slight hack - blank the background if zooming in HOC because it uses
				// different palettes for zoomed items (original does this too)
				// We also do this on scene transition, but need to do it again
				// here for zooming within the box.
				engine->getBackgroundBuffer().fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
			}
			engine->getScene()->runOps(underMouse->onRClickOps);
		}
	} else {
		engine->getScene()->mouseRUp(pt);
	}
}

Common::Error Inventory::syncState(Common::Serializer &s) {
	s.syncAsUint16LE(_openedFromSceneNum);
	s.syncAsByte(_isOpen);
	s.syncAsSint16LE(_highlightItemNo);
	s.syncAsSint16LE(_itemOffset);

	return Common::kNoError;
}


} // end namespace Dgds
