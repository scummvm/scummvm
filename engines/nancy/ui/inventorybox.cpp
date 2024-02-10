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

#include "engines/nancy/ui/inventorybox.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/scrollbar.h"

namespace Nancy {
namespace UI {

InventoryBox::InventoryBox() :
		RenderObject(6),
		_scrollbar(nullptr),
		_scrollbarPos(0),
		_highlightedHotspot(-1),
		_inventoryData(nullptr) {}

InventoryBox::~InventoryBox() {
	_fullInventorySurface.free();
	_iconsSurface.free(); delete _scrollbar;
}

void InventoryBox::init() {
	auto *bootSummary = GetEngineData(BSUM);
	assert(bootSummary);

	_inventoryData = GetEngineData(INV);
	assert(_inventoryData);

	_order.clear();

	moveTo(bootSummary->inventoryBoxScreenPosition);
	g_nancy->_resource->loadImage(_inventoryData->inventoryBoxIconsImageName, _iconsSurface);

	_fullInventorySurface.create(_screenPosition.width(), _screenPosition.height() * ((g_nancy->getStaticData().numItems / 4) + 1), g_nancy->_graphics->getScreenPixelFormat());
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

	_scrollbar = new Scrollbar(	9,
								_inventoryData->scrollbarSrcBounds,
								_inventoryData->scrollbarDefaultPos,
								_inventoryData->scrollbarMaxScroll - _inventoryData->scrollbarDefaultPos.y);
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
	// Disable input when primary video is playing
	if (NancySceneState.getActiveConversation()) {
		return;
	}

	if (_order.size()) {
		_scrollbar->handleInput(input);
	}

	int hoveredHotspot = -1;

	for (uint i = 0; i < 4; ++i) {
		if (_itemHotspots[i].hotspot.contains(input.mousePos)) {
			if (NancySceneState.getHeldItem() != -1) {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
				if (input.input & NancyInput::kLeftMouseButtonUp) {
					NancySceneState.addItemToInventory(NancySceneState.getHeldItem());
					g_nancy->_sound->playSound("BULS");
				}
			} else if (_itemHotspots[i].itemID != -1) {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

				hoveredHotspot = i;

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					uint16 itemID = _itemHotspots[i].itemID;
					INV::ItemDescription item = _inventoryData->itemDescriptions[itemID];
					byte disabled = NancySceneState.getItemDisabledState(itemID);

					if (!disabled) {
						// Item is not disabled
						NancySceneState.removeItemFromInventory(itemID, item.keepItem != kInvItemNewSceneView);
						_highlightedHotspot = -1;
						hoveredHotspot = -1;

						if (item.keepItem == kInvItemNewSceneView) {
							// Transport the player to a close-up scene, temporarily remove the item from the inventory
							NancySceneState.pushScene(itemID);
							SceneChangeDescription sceneChange;
							sceneChange.sceneID = item.sceneID;
							sceneChange.continueSceneSound = item.sceneSoundFlag;
							NancySceneState.changeScene(sceneChange);
						}
					} else {
						// Item is disabled
						if (disabled == 2) {
							// ...and set so it plays the "can't" sound when you click it
							NancySceneState.playItemCantSound(itemID);
						}
					}
				}
			}
			break;
		}
	}

	if (_highlightedHotspot != hoveredHotspot) {
		if (_highlightedHotspot != -1) {
			// Un-highlight last hovered item
			drawItemInSlot(_itemHotspots[_highlightedHotspot].itemID, _itemHotspots[_highlightedHotspot].itemOrder, false);
			_highlightedHotspot = -1;
		}

		if (hoveredHotspot != -1) {
			// Highlight hovered item
			drawItemInSlot(_itemHotspots[hoveredHotspot].itemID, _itemHotspots[hoveredHotspot].itemOrder, true);
			_highlightedHotspot = hoveredHotspot;
		}
	}
}

void InventoryBox::addItem(const int16 itemID) {
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

void InventoryBox::removeItem(const int16 itemID) {
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
		drawItemInSlot(_order[i], i);
	}

	if (_order.size() > 0) {
		_curtains.setOpen(true);
	} else {
		_curtains.setOpen(false);
	}

	_needsRedraw = true;
}

void InventoryBox::setHotspots(const uint pageNr) {
	for (uint i = 0; i < 4; ++i) {
		if (i + pageNr * 4 < _order.size()) {
			_itemHotspots[i].itemID = _order[i + pageNr * 4];
			_itemHotspots[i].itemOrder = i + pageNr * 4;
		} else {
			_itemHotspots[i].itemID = -1;
			_itemHotspots[i].itemOrder = -1;
		}
	}
}

void InventoryBox::drawItemInSlot(const uint itemID, const uint slotID, const bool highlighted) {
	auto &item = _inventoryData->itemDescriptions[itemID];
	Common::Rect dest;

	dest.setWidth(_screenPosition.width() / 2);
	dest.setHeight(_screenPosition.height() / 2);
	dest.moveTo((slotID % 2) * dest.width(), (slotID / 2) * dest.height());
	Common::Point destPoint = Common::Point (dest.left, dest.top);

	_fullInventorySurface.blitFrom(_iconsSurface, highlighted ? item.highlightedSourceRect : item.sourceRect, destPoint);
	_needsRedraw = true;
}

void InventoryBox::onScrollbarMove() {
	float scrollPos = _scrollbar->getPos();

	float numPages = (_order.size() - 1) / 4 + 1;
	float pageFrac = 1 / numPages;
	uint curPage = MIN<uint>(scrollPos / pageFrac, numPages - 1);

	Common::Rect sourceRect = _screenPosition;
	sourceRect.moveTo(0, curPage * (sourceRect.height() - 1));
	_drawSurface.create(_fullInventorySurface, sourceRect);

	setHotspots(curPage);

	_needsRedraw = true;
}

InventoryBox::Curtains::Curtains() :
	RenderObject(10),
	_soundTriggered(false),
	_areOpen(false),
	_curFrame(0),
	_numFrames(g_nancy->getGameType() == kGameTypeVampire ? 10 : 7) {}

void InventoryBox::Curtains::init() {
	auto *inventoryData = GetEngineData(INV);
	assert(inventoryData);

	moveTo(inventoryData->curtainsScreenPosition);
	Common::Rect bounds = _screenPosition;
	bounds.moveTo(0, 0);
	_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getInputPixelFormat());

	if (g_nancy->getGameType() == kGameTypeVampire) {
		uint8 palette[256 * 3];
		g_nancy->_graphics->_object0.grabPalette(palette, 0, 256);
		_drawSurface.setPalette(palette, 0, 256);
	}

	_nextFrameTime = 0;
	setAnimationFrame(_curFrame);

	setTransparent(true);

	RenderObject::init();
}

void InventoryBox::Curtains::updateGraphics() {
	Time time = g_nancy->getTotalPlayTime();
	if (_areOpen) {
		if (_curFrame < _numFrames && time > _nextFrameTime) {
			auto *inventoryData = GetEngineData(INV);
			assert(inventoryData);

			setAnimationFrame(++_curFrame);
			_nextFrameTime = time + inventoryData->curtainsFrameTime;

			if (!_soundTriggered) {
				_soundTriggered = true;
				g_nancy->_sound->playSound("CURT");
			}
		}
	} else {
		if (_curFrame > 0 && time > _nextFrameTime) {
			auto *inventoryData = GetEngineData(INV);
			assert(inventoryData);

			setAnimationFrame(--_curFrame);
			_nextFrameTime = time + inventoryData->curtainsFrameTime;

			if (!_soundTriggered) {
				_soundTriggered = true;
				g_nancy->_sound->playSound("CURT");
			}
		}
	}

	if (_curFrame == 0 || _curFrame == _numFrames) {
		_soundTriggered = false;
	}
}

void InventoryBox::Curtains::setAnimationFrame(uint frame) {
	Graphics::ManagedSurface &_object0 = g_nancy->_graphics->_object0;
	Common::Rect srcRect;
	Common::Point destPoint;

	if (frame > (uint)(_numFrames - 1)) {
		// TVD keeps the last frame visible
		if (g_nancy->getGameType() > kGameTypeVampire) {
			setVisible(false);
		}

		return;
	} else {
		setVisible(true);
	}

	auto *inventoryData = GetEngineData(INV);
	assert(inventoryData);

	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	// Draw left curtain
	srcRect = inventoryData->curtainAnimationSrcs[frame * 2];
	_drawSurface.blitFrom(_object0, srcRect, destPoint);

	// Draw right curtain
	srcRect = inventoryData->curtainAnimationSrcs[frame * 2 + 1];
	destPoint.x = getBounds().width() - srcRect.width();
	_drawSurface.blitFrom(_object0, srcRect, destPoint);

	_needsRedraw = true;
}

} // End of namespace UI
} // End of namespace Nancy
