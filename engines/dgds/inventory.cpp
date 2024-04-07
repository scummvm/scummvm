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

#include "graphics/managed_surface.h"
#include "dgds/inventory.h"
#include "dgds/dgds.h"
#include "dgds/scene.h"
#include "dgds/image.h"
#include "dgds/font.h"
#include "dgds/request.h"

namespace Dgds {

Inventory::Inventory() : _isOpen(false), _prevPageBtn(nullptr), _nextPageBtn(nullptr),
	_invClock(nullptr), _itemZoomBox(nullptr), _exitButton(nullptr), _highlightItemNo(-1),
	_itemOffset(0)
{
}

void Inventory::setRequestData(const REQFileData &data) {
	_reqData = data;
	assert(_reqData._requests.size() > 0);
	RequestData *req = _reqData._requests.data();
	_prevPageBtn = dynamic_cast<ButtonGadget *>(req->findGadgetByNumWithFlags3Not0x40(14));
	_nextPageBtn = dynamic_cast<ButtonGadget *>(req->findGadgetByNumWithFlags3Not0x40(15));
	_invClock = dynamic_cast<TextAreaGadget *>(req->findGadgetByNumWithFlags3Not0x40(23));
	_itemZoomBox = req->findGadgetByNumWithFlags3Not0x40(9);
	_exitButton = dynamic_cast<ButtonGadget *>(req->findGadgetByNumWithFlags3Not0x40(17));
	if (!_prevPageBtn || !_nextPageBtn || !_invClock || !_itemZoomBox || !_exitButton)
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
		DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	if (engine->getScene()->getNum() == 2)
		return;

	ImageGadget *itemImgArea = dynamic_cast<ImageGadget *>(_reqData._requests[0].findGadgetByNumWithFlags3Not0x40(8));
	if (isRestarting) {
		warning("TODO: Handle inventory redraw on restart");
	} else {
		_itemZoomBox->_flags3 &= 0x40;
	}

	if (!itemImgArea)
		error("Couldn't get img area for inventory");

	//
	// Decide whether the nextpage/prevpage buttons should be visible
	//
	if ((itemImgArea->_width / itemImgArea->_xStep) *
			(itemImgArea->_height / itemImgArea->_yStep) > itemCount) {
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

	drawItems(surf, itemImgArea);
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

void Inventory::drawItems(Graphics::ManagedSurface &surf, ImageGadget *imgArea) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	const Common::Array<struct GameItem> &items = engine->getGDSScene()->getGameItems();
	const Common::SharedPtr<Image> &icons = engine->getIcons();
	int x = 0;
	int y = 0;

	const int xstep = imgArea->_xStep;
	const int ystep = imgArea->_yStep;

	Common::Point pos(imgArea->_parentX + imgArea->_x, imgArea->_parentY + imgArea->_y);
	Common::Rect itemRect(pos, imgArea->_width, imgArea->_height);
	//surf.fillRect(itemRect, (byte)imgArea->_col1);

	if (!icons)
		return;

	// TODO: does this need to be adjusted ever?
	const Common::Rect drawMask(0, 0, 320, 200);
	int offset = _itemOffset;
	for (const auto & item: items) {
		if (item._inSceneNum != 2 || !(item._flags & 4))
			continue;

		if (offset) {
			offset--;
			continue;
		}

		if (item._num == _highlightItemNo) {
			// draw highlighted
			Common::Rect highlightRect(Common::Point(x, y), xstep, ystep);
			surf.fillRect(highlightRect, 4);
		}

		// draw offset for the image
		int xoff = x + (xstep - item.rect.width) / 2;
		int yoff = y + (ystep - item.rect.height) / 2;

		icons->drawBitmap(item._iconNum, xoff, yoff, drawMask, surf);

		x += xstep;
		if (x > imgArea->_x + imgArea->_width) {
			x = 0;
			y += ystep;
		}
		if (y > imgArea->_x + imgArea->_width) {
			break;
		}
	}
}

void Inventory::mouseMoved(const Common::Point &pt) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	engine->setMouseCursor(0);
}

void Inventory::mouseLClicked(const Common::Point &pt) {
	if (_exitButton->containsPoint(pt)) {
		close();
		return;
	}
}

void Inventory::mouseRClicked(const Common::Point &pt) {

}


} // end namespace Dgds
