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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"
#include "engines/nancy/constants.h"

#include "engines/nancy/ui/inventorybox.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/scrollbar.h"

namespace Nancy {
namespace UI {

InventoryBox::InventoryBox(RenderObject &redrawFrom) :
		RenderObject(redrawFrom, 6),
		_scrollbar(nullptr),
		_curtains(*this, this),
		_scrollbarPos(0),
		_curtainsFrameTime(0) {}

InventoryBox::~InventoryBox() {
	_fullInventorySurface.free();
	_iconsSurface.free(); delete _scrollbar;
}

void InventoryBox::init() {
	Common::SeekableReadStream &stream = *g_nancy->getBootChunkStream("INV");
	stream.seek(0, SEEK_SET);

	_order.clear();

	Common::Rect scrollbarSrcBounds;
	readRect(stream, scrollbarSrcBounds);
	Common::Point scrollbarDefaultPos;
	scrollbarDefaultPos.x = stream.readUint16LE();
	scrollbarDefaultPos.y = stream.readUint16LE();
	uint16 scrollbarMaxScroll = stream.readUint16LE();

	stream.seek(0xD6, SEEK_SET);

	uint numFrames = g_nancy->getConstants().numCurtainAnimationFrames;
	_curtainsSrc.resize(numFrames * 2);
	for (uint i = 0; i < numFrames * 2; ++i) {
		readRect(stream, _curtainsSrc[i]);
	}

	readRect(stream, _screenPosition);
	_curtainsFrameTime = stream.readUint16LE();

	Common::String inventoryBoxIconsImageName;
	readFilename(stream, inventoryBoxIconsImageName);
	readFilename(stream, _inventoryCursorsImageName);

	stream.skip(8);
	readRect(stream, _emptySpace);

	char itemName[20];
	uint itemNameLength = g_nancy->getGameType() == kGameTypeVampire ? 15 : 20;

	_itemDescriptions.reserve(g_nancy->getConstants().numItems);
	for (uint i = 0; i < g_nancy->getConstants().numItems; ++i) {
		stream.read(itemName, itemNameLength);
		itemName[itemNameLength - 1] = '\0';
		_itemDescriptions.push_back(ItemDescription());
		ItemDescription &desc = _itemDescriptions.back();
		desc.name = Common::String(itemName);
		desc.oneTimeUse = stream.readUint16LE();
		readRect(stream, desc.sourceRect);
	}

	g_nancy->_resource->loadImage(inventoryBoxIconsImageName, _iconsSurface);

	_fullInventorySurface.create(_screenPosition.width(), _screenPosition.height() * ((g_nancy->getConstants().numItems / 4) + 1), g_nancy->_graphicsManager->getScreenPixelFormat());
	Common::Rect sourceRect = _screenPosition;
	sourceRect.moveTo(0, 0);
	_drawSurface.create(_fullInventorySurface, sourceRect);

	for (uint i = 0; i < 4; ++i) {
		Common::Rect &r = _itemHotspots[i].hotspot;
		r = _screenPosition;
		r.setWidth(r.width() / 2);
		r.setHeight(r.height() / 2);
		r.translate((i % 2) * r.width(), (i / 2) * r.height());
	}

	RenderObject::init();

	_scrollbar = new Scrollbar(NancySceneState.getFrame(), 9, scrollbarSrcBounds, scrollbarDefaultPos, scrollbarMaxScroll - scrollbarDefaultPos.y);
	_scrollbar->init();
	_curtains.init();
}

void InventoryBox::updateGraphics() {
	if (_scrollbarPos != _scrollbar->getPos()) {
		_scrollbarPos = _scrollbar->getPos();

		onScrollbarMove();
	}
}

void InventoryBox::registerGraphics() {
	RenderObject::registerGraphics();
	_scrollbar->registerGraphics();
	_curtains.registerGraphics();
}

void InventoryBox::handleInput(NancyInput &input) {
	if (_order.size()) {
		_scrollbar->handleInput(input);
	}

	for (uint i = 0; i < 4; ++i) {
		if (_itemHotspots[i].hotspot.contains(input.mousePos)) {
			if (NancySceneState.getHeldItem() != -1) {
				g_nancy->_cursorManager->setCursorType(CursorManager::kHotspotArrow);
				if (input.input & NancyInput::kLeftMouseButtonUp) {
					NancySceneState.addItemToInventory(NancySceneState.getHeldItem());
					g_nancy->_sound->playSound("BULS");
				}
			} else if (_itemHotspots[i].itemID != -1) {
				g_nancy->_cursorManager->setCursorType(CursorManager::kHotspotArrow);
				if (input.input & NancyInput::kLeftMouseButtonUp) {
					NancySceneState.removeItemFromInventory(_itemHotspots[i].itemID);
					g_nancy->_sound->playSound("GLOB");
				}
			}
			break;
		}
	}
}

void InventoryBox::addItem(int16 itemID) {
	if (_order.size() == 0) {
		// Adds first item, start curtains animation
		_curtains.setOpen(true);
	}
	Common::Array<int16> back = _order;
	_order.clear();
	_order.push_back(itemID);
	_order.push_back(back);

	onReorder();
}

void InventoryBox::removeItem(int16 itemID) {
	for (auto &i : _order) {
		if (i == itemID) {
			_order.erase(&i);
			onReorder();
			break;
		}
	}
}

void InventoryBox::onReorder() {
	onScrollbarMove();

	_fullInventorySurface.clear();
	for (uint i = 0; i < _order.size(); ++i) {
		Common::Rect dest;
		dest.setWidth(_screenPosition.width() / 2);
		dest.setHeight(_screenPosition.height() / 2);
		dest.moveTo((i % 2) * dest.width(), (i / 2) * dest.height());
		Common::Point destPoint = Common::Point (dest.left, dest.top);

		_fullInventorySurface.blitFrom(_iconsSurface, _itemDescriptions[_order[i]].sourceRect, destPoint);
	}

	if (_order.size() > 0) {
		_curtains.setOpen(true);
	} else {
		_curtains.setOpen(false);
	}

	_needsRedraw = true;
}

void InventoryBox::setHotspots(uint pageNr) {
	for (uint i = 0; i < 4; ++i) {
		if (i + pageNr * 4 < _order.size()) {
			_itemHotspots[i].itemID = _order[i +  pageNr * 4];
		} else {
			_itemHotspots[i].itemID = -1;
		}
	}
}

void InventoryBox::onScrollbarMove() {
	float scrollPos = _scrollbar->getPos();

	float numPages = (_order.size() - 1) / 4 + 1;
	float pageFrac = 1 / numPages;
	uint curPage = MIN<uint>(scrollPos / pageFrac, numPages - 1);

	Common::Rect sourceRect = _screenPosition;
	sourceRect.moveTo(0, curPage * sourceRect.height());
	_drawSurface.create(_fullInventorySurface, sourceRect);

	setHotspots(curPage);

	_needsRedraw = true;
}

void InventoryBox::Curtains::init() {
	Common::Rect bounds = _parent->getBounds();
	_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphicsManager->getInputPixelFormat());

	if (g_nancy->getGameType() == kGameTypeVampire) {
		_drawSurface.setPalette(g_nancy->_graphicsManager->_object0.getPalette(), 0, 256);
	}

	_screenPosition = _parent->getScreenPosition();
	_nextFrameTime = 0;
	setAnimationFrame(_curFrame);

	setTransparent(true);

	RenderObject::init();
}

void InventoryBox::Curtains::updateGraphics() {
	Time time = g_nancy->getTotalPlayTime();
	if (_areOpen) {
		if (_curFrame < g_nancy->getConstants().numCurtainAnimationFrames && time > _nextFrameTime) {
			setAnimationFrame(++_curFrame);
			_nextFrameTime = time + _parent->_curtainsFrameTime;

			if (!_soundTriggered) {
				_soundTriggered = true;
				g_nancy->_sound->playSound("CURT");
			}
		}
	} else {
		if (_curFrame > 0 && time > _nextFrameTime) {
			setAnimationFrame(--_curFrame);
			_nextFrameTime = time + _parent->_curtainsFrameTime;

			if (!_soundTriggered) {
				_soundTriggered = true;
				g_nancy->_sound->playSound("CURT");
			}
		}
	}

	if (_curFrame == 0 || _curFrame == g_nancy->getConstants().numCurtainAnimationFrames) {
		_soundTriggered = false;
	}
}

void InventoryBox::Curtains::setAnimationFrame(uint frame) {
	Graphics::ManagedSurface &_object0 = g_nancy->_graphicsManager->_object0;
	Common::Rect srcRect;
	Common::Point destPoint;

	if (frame > g_nancy->getConstants().numCurtainAnimationFrames - 1) {
		setVisible(false);
		return;
	} else {
		setVisible(true);
	}

	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());

	// Draw left shade
	srcRect = _parent->_curtainsSrc[frame * 2];
	_drawSurface.blitFrom(_object0, srcRect, destPoint);

	// Draw right shade
	srcRect = _parent->_curtainsSrc[frame * 2 + 1];
	destPoint.x = getBounds().width() - srcRect.width();
	_drawSurface.blitFrom(_object0, srcRect, destPoint);

	_needsRedraw = true;
}

} // End of namespace UI
} // End of namespace Nancy
