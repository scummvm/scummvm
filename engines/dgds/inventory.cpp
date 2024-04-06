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

namespace Dgds {

Inventory::Inventory() : _isOpen(false), _prevPageBtn(nullptr), _nextPageBtn(nullptr),
	_invClock(nullptr), _itemZoomBox(nullptr), _exitButton(nullptr), _highlightItemNo(-1),
	_itemOffset(0)
{
}

void Inventory::setRequestData(const REQFileData &data) {
	_requestData = data;
	assert(_requestData._requests.size() > 0);
	RequestData *req = _requestData._requests.data();
	_prevPageBtn = dynamic_cast<ButtonGadget *>(req->findGadgetByNumWithFlags3Not0x40(14));
	_nextPageBtn = dynamic_cast<ButtonGadget *>(req->findGadgetByNumWithFlags3Not0x40(15));
	_invClock = dynamic_cast<TextAreaGadget *>(req->findGadgetByNumWithFlags3Not0x40(23));
	_itemZoomBox = req->findGadgetByNumWithFlags3Not0x40(9);
	_exitButton = dynamic_cast<ButtonGadget *>(req->findGadgetByNumWithFlags3Not0x40(17));
	if (!_prevPageBtn || !_nextPageBtn || !_invClock || !_itemZoomBox || !_exitButton)
		error("Didn't get all expected inventory gadgets");
}

void Inventory::draw(Graphics::ManagedSurface &surf, int itemCount, bool isRestarting) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	if (engine->getScene()->getNum() == 2)
		return;

	ImageGadget *itemImgArea = dynamic_cast<ImageGadget *>(_requestData._requests[0].findGadgetByNumWithFlags3Not0x40(8));
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
	if ((itemImgArea->_width / itemImgArea->_gadget8_i1) *
		(itemImgArea->_height / itemImgArea->_gadget8_i2) > itemCount) {
		warning("TODO: Enable prev page / next page buttons in inventory");
	}

	_requestData._requests[0].drawInvType(&surf);

	drawTime(surf);

	drawItems(surf, itemImgArea);
}

void Inventory::drawTime(Graphics::ManagedSurface &surf) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	_invClock->_buttonName = engine->getClock().getTimeStr();
	_invClock->draw(surf.surfacePtr());
}

void Inventory::drawItems(Graphics::ManagedSurface &surf, ImageGadget *imgArea) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	const Common::Array<struct GameItem> &items = engine->getGDSScene()->getGameItems();
	const Common::SharedPtr<Image> &icons = engine->getIcons();
	int x = 0;
	int y = 0;

	const int xstep = imgArea->_gadget8_i1;
	const int ystep = imgArea->_gadget8_i2;

	Common::Rect itemRect(Common::Point(imgArea->_parentX + imgArea->_x, imgArea->_parentY + imgArea->_y), imgArea->_width, imgArea->_height);
	surf.fillRect(itemRect, (byte)imgArea->_field15_0x22);

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

}

void Inventory::mouseRClicked(const Common::Point &pt) {

}


} // end namespace Dgds
