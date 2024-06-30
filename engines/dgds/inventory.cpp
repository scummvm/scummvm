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

namespace Dgds {

Inventory::Inventory() : _isOpen(false), _prevPageBtn(nullptr), _nextPageBtn(nullptr),
	_invClock(nullptr), _itemZoomBox(nullptr), _exitButton(nullptr), _clockSkipMinBtn(nullptr),
	_itemArea(nullptr), _clockSkipHrBtn(nullptr), _dropBtn(nullptr), _highlightItemNo(-1),
	_itemOffset(0), _openedFromSceneNum(-1)
{
}

void Inventory::open() {
	if (_isOpen)
		return;;
	_isOpen = true;
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	_openedFromSceneNum = engine->getScene()->getNum();
	engine->changeScene(2, false);
}

void Inventory::close() {
	if (!_isOpen)
		return;;
	_isOpen = false;
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	engine->changeScene(_openedFromSceneNum, false);
	_openedFromSceneNum = -1;
}

void Inventory::setRequestData(const REQFileData &data) {
	_reqData = data;
	assert(_reqData._requests.size() > 0);
	RequestData *req = _reqData._requests.data();
	_prevPageBtn = dynamic_cast<ButtonGadget *>(req->findGadgetByNumWithFlags3Not0x40(14));
	_nextPageBtn = dynamic_cast<ButtonGadget *>(req->findGadgetByNumWithFlags3Not0x40(15));
	_invClock = dynamic_cast<TextAreaGadget *>(req->findGadgetByNumWithFlags3Not0x40(23));
	_itemBox = req->findGadgetByNumWithFlags3Not0x40(8);
	_itemZoomBox = req->findGadgetByNumWithFlags3Not0x40(9);
	_exitButton = dynamic_cast<ButtonGadget *>(req->findGadgetByNumWithFlags3Not0x40(17));

	_clockSkipMinBtn = dynamic_cast<ButtonGadget *>(req->findGadgetByNumWithFlags3Not0x40(24));
	_clockSkipHrBtn = dynamic_cast<ButtonGadget *>(req->findGadgetByNumWithFlags3Not0x40(25));
	_dropBtn = dynamic_cast<ButtonGadget *>(req->findGadgetByNumWithFlags3Not0x40(16));
	_itemArea = dynamic_cast<ImageGadget *>(_reqData._requests[0].findGadgetByNumWithFlags3Not0x40(8));

	if (!_prevPageBtn || !_nextPageBtn || !_invClock || !_itemZoomBox || !_exitButton || !_itemArea)
		error("Didn't get all expected inventory gadgets");
}

void Inventory::drawHeader(Graphics::ManagedSurface &surf) {
	// This really should be a text area, but it's hard-coded in the game.
	const Font *font = RequestData::getMenuFont();
	const RequestData &r = _reqData._requests[0];

	static const char *title = "INVENTORY";
	int titleWidth = font->getStringWidth(title);
	int y1 = r._y + 7;
	int x1 = r._x + 112;
	font->drawString(&surf, title, x1 + 4, y1 + 2, titleWidth, 0);

	int x2 = x1 + titleWidth + 6;
	int y2 = y1 + font->getFontHeight();
	surf.drawLine(x1, y1, x2, y1, 0xdf);
	surf.drawLine(x2, y1 + 1, x2, y2, 0xdf);
	surf.drawLine(x1, y1 + 1, x1, y2, 0xff);
	surf.drawLine(x1 + 1, y2, x1 + titleWidth + 5, y2, 0xff);
}

void Inventory::draw(Graphics::ManagedSurface &surf, int itemCount, bool isRestarting) {
	//DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	//_if (engine->getScene()->getNum() == 2)
	//	return;

	if (isRestarting) {
		warning("TODO: Handle inventory redraw on restart");
	} else {
		_itemZoomBox->_flags3 &= 0x40;
	}

	//
	// Decide whether the nextpage/prevpage buttons should be visible
	//
	if ((_itemArea->_width / _itemArea->_xStep) *
			(_itemArea->_height / _itemArea->_yStep) > itemCount) {
		// not visible.
		_prevPageBtn->_flags3 |= 0x40;
		_nextPageBtn->_flags3 |= 0x40;
	} else {
		// clear flag 0x40 - visible.
		_prevPageBtn->_flags3 &= ~0x40;
		_nextPageBtn->_flags3 &= ~0x40;
	}
	_reqData._requests[0].drawInvType(&surf);

	drawHeader(surf);
	drawTime(surf);
	drawItems(surf);
}

void Inventory::drawTime(Graphics::ManagedSurface &surf) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	const Font *font = RequestData::getMenuFont();
	const Common::String timeStr = engine->getClock().getTimeStr();
	Common::Point clockpos = Common::Point(_invClock->_x + _invClock->_parentX, _invClock->_y + _invClock->_parentY);
	surf.fillRect(Common::Rect(clockpos, _invClock->_width, _invClock->_height), 0);
	RequestData::drawCorners(&surf, 19, clockpos.x - 2, clockpos.y - 2,
								_invClock->_width + 4, _invClock->_height + 4);
	font->drawString(&surf, timeStr, clockpos.x, clockpos.y, font->getStringWidth(timeStr), _invClock->_col3);
}

void Inventory::drawItems(Graphics::ManagedSurface &surf) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	const Common::SharedPtr<Image> &icons = engine->getIcons();
	int x = 0;
	int y = 0;

	const int xstep = _itemArea->_xStep;
	const int ystep = _itemArea->_yStep;
	const int imgAreaX = _itemArea->_parentX + _itemArea->_x;
	const int imgAreaY = _itemArea->_parentY + _itemArea->_y;

	Common::Rect itemRect(Common::Point(imgAreaX, imgAreaY), _itemArea->_width, _itemArea->_height);
	//surf.fillRect(itemRect, (byte)imgArea->_col1);

	if (!icons)
		return;

	// TODO: does this need to be adjusted ever?
	const Common::Rect drawMask(0, 0, 320, 200);
	int offset = _itemOffset;
	Common::Array<struct GameItem> &items = engine->getGDSScene()->getGameItems();
	for (auto & item: items) {
		if (item._inSceneNum != 2) //  || !(item._flags & 4))
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
			item.rect.width = MIN((int)icon->w, item.rect.width);
			item.rect.height = MIN((int)icon->h, item.rect.height);
		}

		// calculate draw offset for the image
		int drawX = imgAreaX + x + (xstep - item.rect.width) / 2;
		int drawY = imgAreaY + y +  (ystep - item.rect.height) / 2;

		icons->drawBitmap(item._iconNum, drawX, drawY, drawMask, surf);

		item.rect.x = drawX;
		item.rect.y = drawY;

		x += xstep;
		if (x >= _itemArea->_width) {
			x = 0;
			y += ystep;
		}
		if (y >= _itemArea->_height) {
			break;
		}
	}
}

void Inventory::mouseMoved(const Common::Point &pt) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	const GameItem *dragItem = engine->getScene()->getDragItem();
	if (dragItem) {
		engine->setMouseCursor(dragItem->_iconNum);
		const RequestData &req = _reqData._requests[0];
		const Common::Rect bgsize(Common::Point(req._x, req._y), req._width, req._height);
		if (!bgsize.contains(pt)) {
			// dragged an item outside the inventory
			close();
		}
	} else {
		engine->setMouseCursor(0);
	}
}

GameItem *Inventory::itemUnderMouse(const Common::Point &pt) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	if (!_itemArea || !_itemArea->containsPoint(pt))
		return nullptr;

	const int imgAreaX = _itemArea->_parentX + _itemArea->_x;
	const int imgAreaY = _itemArea->_parentY + _itemArea->_y;
	const int numacross = _itemArea->_width / _itemArea->_xStep;
	const int itemrow = (pt.y - imgAreaY) / _itemArea->_yStep;
	const int itemcol = (pt.x - imgAreaX) / _itemArea->_xStep;
	int itemnum = numacross * itemrow + itemcol;

	Common::Array<struct GameItem> &items = engine->getGDSScene()->getGameItems();
	for (auto &item: items) {
		if (item._inSceneNum != 2) // || !(item._flags & 4))
			continue;

		if (itemnum) {
			itemnum--;
			continue;
		}
		return &item;
	}
	return nullptr;
}

void Inventory::mouseLDown(const Common::Point &pt) {
	GameItem *underMouse = itemUnderMouse(pt);
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	if (underMouse) {
		_highlightItemNo = underMouse->_num;
		engine->getScene()->setDragItem(underMouse);
		if (underMouse->_iconNum)
			engine->setMouseCursor(underMouse->_iconNum);
	}
}

void Inventory::mouseLUp(const Common::Point &pt) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	const GameItem *dragItem = engine->getScene()->getDragItem();
	engine->setMouseCursor(0);

	if (dragItem) {
		engine->getScene()->mouseLUp(pt);
		// this will clear the drag item.
		return;
	}

	int itemsPerPage = (_itemArea->_width / _itemArea->_xStep) * (_itemArea->_height / _itemArea->_yStep);
	if (_exitButton->containsPoint(pt)) {
		close();
		return;
	} else if (_nextPageBtn->containsPoint(pt) && !(_nextPageBtn->_flags3 & 0x40)) {
		int numInvItems = 0;
		Common::Array<struct GameItem> &items = engine->getGDSScene()->getGameItems();
		for (auto &item: items) {
			if (item._inSceneNum == 2) // && item._flags & 4)
				numInvItems++;
		}
		if (_itemOffset < numInvItems)
			_itemOffset += itemsPerPage;
	} else if (_prevPageBtn->containsPoint(pt) && !(_prevPageBtn->_flags3 & 0x40)) {
		if (_itemOffset > 0)
			_itemOffset -= itemsPerPage;
	} else if (_clockSkipMinBtn && _clockSkipMinBtn->containsPoint(pt)) {
		engine->getClock().addGameTime(1);
	} else if (_clockSkipHrBtn && _clockSkipHrBtn->containsPoint(pt)) {
		engine->getClock().addGameTime(60);
	} else if (_dropBtn && _dropBtn->containsPoint(pt)) {
		if (_highlightItemNo >= 0) {
			Common::Array<struct GameItem> &items = engine->getGDSScene()->getGameItems();
			for (auto &item: items) {
				if (item._num == _highlightItemNo) {
					item._inSceneNum = _openedFromSceneNum;
					break;
				}
			}
			_highlightItemNo = -1;
		}
	}
}

void Inventory::mouseRUp(const Common::Point &pt) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	GameItem *underMouse = itemUnderMouse(pt);
	if (underMouse) {
		engine->getScene()->runOps(underMouse->onRClickOps);
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
