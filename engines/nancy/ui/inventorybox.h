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

#ifndef NANCY_UI_INVENTORYBOX_H
#define NANCY_UI_INVENTORYBOX_H

#include "engines/nancy/time.h"

#include "engines/nancy/renderobject.h"

namespace Nancy {

class NancyEngine;
struct NancyInput;

namespace State {
class Scene;
}

namespace UI {

class Scrollbar;

class InventoryBox : public RenderObject {
	friend class Nancy::State::Scene;

public:
	struct ItemDescription {
		Common::String name; // 0x00
		byte keepItem = kInvItemUseThenLose; // 0x14
		Common::Rect sourceRect; // 0x16
	};

	InventoryBox();
	virtual ~InventoryBox();

	void init() override;
	void updateGraphics() override;
	void registerGraphics() override;
	void handleInput(NancyInput &input);

	void onScrollbarMove();

private:
	// These are private since they should only be called from Scene
	void addItem(int16 itemID);
	void removeItem(int16 itemID);

	void onReorder();
	void setHotspots(uint pageNr);

	class Curtains : public RenderObject {
	public:
		Curtains() :
			RenderObject(10),
			_soundTriggered(false),
			_areOpen(false),
			_curFrame(0) {}
		virtual ~Curtains() = default;

		void init() override;
		void updateGraphics() override;

		void setOpen(bool open) { _areOpen = open; }

		void setAnimationFrame(uint frame);

		uint _curFrame;
		Time _nextFrameTime;
		bool _areOpen;
		bool _soundTriggered;
	};

	struct ItemHotspot {
		int16 itemID = -1;
		Common::Rect hotspot; // in screen coordinates
	};

	Graphics::ManagedSurface _iconsSurface;
	Graphics::ManagedSurface _fullInventorySurface;

	Scrollbar *_scrollbar;
	Curtains _curtains;

	float _scrollbarPos;

	Common::Array<int16> _order;
	ItemHotspot _itemHotspots[4];
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_INVENTORYBOX_H
