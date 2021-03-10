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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/nancy/ui/inventorybox.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/input.h"

#include "common/events.h"

namespace Nancy {
namespace UI {

void InventoryBox::init() {
    Common::SeekableReadStream &stream = *NanEngine.getBootChunkStream("INV");
    stream.seek(0, SEEK_SET);

    _order.clear();

    readRect(stream, _sliderSource);
    _sliderDefaultDest.x = stream.readUint16LE();
    _sliderDefaultDest.y = stream.readUint16LE();

    stream.seek(0xD6, SEEK_SET);

    for (uint i = 0; i < 14; ++i) {
        readRect(stream, _shadesSrc[i]);
    }

    readRect(stream, _screenPosition);
    _shadesFrameTime = stream.readUint16LE();

    char name[10];
    stream.read(name, 10);
    Common::String inventoryBoxIconsImageName = Common::String(name);
    stream.read(name, 10);
    _inventoryCursorsImageName = Common::String(name);

    stream.skip(8);
    readRect(stream, _emptySpace);

    char itemName[0x14];

    for (uint i = 0; i < 11; ++i) {
        stream.read(itemName, 0x14);
        _itemDescriptions[i].name = Common::String(itemName);
        _itemDescriptions[i].oneTimeUse = stream.readUint16LE();
        readRect(stream, _itemDescriptions[i].sourceRect);
    }

    NanEngine.resource->loadImage("ciftree", inventoryBoxIconsImageName, _iconsSurface);
    
    uint numItems = 11; // TODO
    _fullInventorySurface.create(_screenPosition.width(), _screenPosition.height() * ((numItems / 4) + 1), GraphicsManager::pixelFormat);
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

    _scrollbar.init();
    _shades.init();
}

void InventoryBox::updateGraphics() {
    if (_scrollbarPos != _scrollbar.getPos()) {
        _scrollbarPos = _scrollbar.getPos();

        onScrollbarMove();
    }
}

void InventoryBox::registerGraphics() {
    RenderObject::registerGraphics();
    _scrollbar.registerGraphics();
    _shades.registerGraphics();
}

void InventoryBox::handleInput(NancyInput &input) {
    if (_order.size()) {
        _scrollbar.handleInput(input);
    }
    
    for (uint i = 0; i < 4; ++i) {
        if (_itemHotspots[i].hotspot.contains(input.mousePos)) {
            if (NancySceneState.getHeldItem() != -1) {
                NanEngine.cursorManager->setCursorType(CursorManager::kHotspotArrow);
                if (input.input & NancyInput::kLeftMouseButtonUp) {
                    NancySceneState.addItemToInventory(NancySceneState.getHeldItem());
                    NanEngine.sound->playSound(0x16);
                }                
            } else if (_itemHotspots[i].itemID != -1) {
                NanEngine.cursorManager->setCursorType(CursorManager::kHotspotArrow);
                if (input.input & NancyInput::kLeftMouseButtonUp) {
                    NancySceneState.removeItemFromInventory(_itemHotspots[i].itemID);
                    NanEngine.sound->playSound(0x18);
                }
            }
            break;
        }
    }
}

void InventoryBox::addItem(int16 itemID) {
    if (_order.size() == 0) {
        // Adds first item, start shades animation
        _shades.setOpen(true);
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
        _shades.setOpen(true);
    } else {
        _shades.setOpen(false);
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
    float scrollPos = _scrollbar.getPos();

    float numPages = (_order.size() - 1) / 4 + 1;
    float pageFrac = 1 / numPages;
    uint curPage = MIN<uint>(scrollPos / pageFrac, numPages - 1);

    Common::Rect sourceRect = _screenPosition;
    sourceRect.moveTo(0, curPage * sourceRect.height());
    _drawSurface.create(_fullInventorySurface, sourceRect);

    setHotspots(curPage);

    _needsRedraw = true;
}

void InventoryBox::InventoryScrollbar::init() {
    Common::Rect &srcBounds = _parent->_sliderSource;
    Common::Point &topPosition = _parent->_sliderDefaultDest;

    _drawSurface.create(NanEngine.graphicsManager->object0, srcBounds);

    _startPosition = topPosition;
    _startPosition.x -= srcBounds.width() / 2;

    _screenPosition = srcBounds;
    _screenPosition.moveTo(_startPosition);
    
    _maxDist = _parent->getBounds().height() - _drawSurface.h;
    
    RenderObject::init();
}

void InventoryBox::Shades::init() {
    Common::Rect bounds = _parent->getBounds();
    _drawSurface.create(bounds.width(), bounds.height(), GraphicsManager::pixelFormat);
    _screenPosition = _parent->getScreenPosition();
    _nextFrameTime = 0;
    setAnimationFrame(_curFrame);

    RenderObject::init();
}

void InventoryBox::Shades::updateGraphics() {
    Time time = NanEngine.getTotalPlayTime();
    if (_areOpen) {
        if (_curFrame < 7 && time > _nextFrameTime) {
            setAnimationFrame(++_curFrame);
            _nextFrameTime = time + _parent->_shadesFrameTime;

            if (!_soundTriggered) {
                _soundTriggered = true;
                NanEngine.sound->playSound(0x12);
            }
        }
    } else {
        if (_curFrame > 0 && time > _nextFrameTime) {
            setAnimationFrame(--_curFrame);
            _nextFrameTime = time + _parent->_shadesFrameTime;

            if (!_soundTriggered) {
                _soundTriggered = true;
                NanEngine.sound->playSound(0x12);
            }
        }
    }

    if (_curFrame == 0 || _curFrame == 7) {
        _soundTriggered = false;
    }
}

void InventoryBox::Shades::setAnimationFrame(uint frame) {
    Graphics::ManagedSurface &object0 = NanEngine.graphicsManager->object0;
    Common::Rect srcRect;
    Common::Point destPoint;

    if (frame > 6) {
        setVisible(false);
        return;
    } else {
        setVisible(true);
    }

    _drawSurface.clear(GraphicsManager::transColor);

    // Draw left shade
    srcRect = _parent->_shadesSrc[frame * 2];
    _drawSurface.blitFrom(object0, srcRect, destPoint);

    // Draw right shade
    srcRect = _parent->_shadesSrc[frame * 2 + 1];
    destPoint.x = getBounds().width() - srcRect.width();
    _drawSurface.blitFrom(object0, srcRect, destPoint);

    _needsRedraw = true;
}

} // End of namespace UI
} // End of namespace Nancy
