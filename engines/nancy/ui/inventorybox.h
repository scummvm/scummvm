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

#ifndef NANCY_UI_INVENTORYBOX_H
#define NANCY_UI_INVENTORYBOX_H

#include "engines/nancy/renderobject.h"

#include "engines/nancy/ui/scrollbar.h"
#include "engines/nancy/time.h"

#include "common/array.h"
#include "common/str.h"
#include "common/rect.h"

namespace Nancy {

class NancyEngine;
struct NancyInput;

namespace UI {

class InventoryBox : public RenderObject {
	friend class InventoryScrollbar;
    friend class Shades;

public:
    struct ItemDescription {
        Common::String name; // 0x00
        byte oneTimeUse = 0; // 0x14
        Common::Rect sourceRect; // 0x16
    };

    InventoryBox(RenderObject &redrawFrom) :
        RenderObject(redrawFrom),
        _scrollbar(redrawFrom, this),
        _shades(*this, this) {}

    virtual ~InventoryBox() { _fullInventorySurface.free(); _iconsSurface.free(); }

    virtual void init() override;
    virtual void registerGraphics() override;
    void handleInput(NancyInput &input);

    // To be called from Scene
    void addItem(uint itemID);
    void removeItem(uint itemID, bool hold = true);

    ItemDescription getItemDescription(uint id) { return _itemDescriptions[id]; }

protected:
    virtual uint16 getZOrder() const override { return 6; }
    virtual BlitType getBlitType() const override { return kNoTrans; }

    void onScrollbarMove();

private:
    void onReorder();
    void setHotspots(uint pageNr);

    class InventoryScrollbar : public Scrollbar {
    public:
        InventoryScrollbar(RenderObject &redrawFrom, InventoryBox *parent) :
            Scrollbar(redrawFrom),
            _parent(parent) {}
        virtual ~InventoryScrollbar() =default;

        virtual void init() override;

    protected:
        InventoryBox *_parent;
    };

    class Shades : public RenderObject {
    public:
        Shades(RenderObject &redrawFrom, InventoryBox *parent) :
            RenderObject(redrawFrom),
            _parent(parent) {}
        virtual ~Shades() =default;

        virtual void init() override;
        virtual void updateGraphics() override;

        void setOpen(bool open) { _areOpen = open; }

    protected:
        virtual uint16 getZOrder() const override { return 9; }
        virtual BlitType getBlitType() const override { return kTrans; }

        void setAnimationFrame(uint frame);

        InventoryBox *_parent;

        uint _curFrame;
        Time _nextFrameTime;
        bool _areOpen;      
    };

    struct ItemHotspot {
        int16 itemID = -1;
        Common::Rect hotspot; // in screen coordinates
    };

    Graphics::Surface _iconsSurface;
    Graphics::ManagedSurface _fullInventorySurface;

    InventoryScrollbar _scrollbar;
    Shades _shades;

    Common::Array<uint> _order;
    ItemHotspot _itemHotspots[4]; 

    // INV contents
    Common::Rect _sliderSource; // 0x00
    Common::Point _sliderDefaultDest; // 0x10
    //...
    Common::Rect _shadesSrc[14]; // 0xD6
    // _screenPosition 0x1B6
    uint16 _shadesFrameTime; // 0x1C6
    Common::String _inventoryCursorsImageName; // 0x1D2, should this be here?

    Common::Rect _emptySpace; // 0x1E4
    ItemDescription _itemDescriptions[11]; // 0x1F4

};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_INVENTORYBOX_H
