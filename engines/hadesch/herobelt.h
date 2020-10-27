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
 * Copyright 2020 Google
 *
 */
#ifndef HADESCH_HEROBELT_H
#define HADESCH_HEROBELT_H

#include "common/array.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/rect.h"
#include "common/ptr.h"
#include "hadesch/pod_file.h"
#include "hadesch/pod_image.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "hadesch/enums.h"
#include "hadesch/event.h"

namespace Hadesch {
class HeroBelt {
public:
	HeroBelt();

	enum HeroBeltColour {
		kWarm,
		kCold,
		kCool,
		kNumColours
	};

	void render(Common::SharedPtr<GfxContext> context, int time, Common::Point viewPoint);
	void computeHotZone(int time, Common::Point mousePos, bool mouseEnabled);
	void placeToInventory(InventoryItem item, EventHandlerWrapper callbackEvent = EventHandlerWrapper());
	void removeFromInventory(InventoryItem item);
	bool isOverHeroBelt() const {
		return _overHeroBelt;
	}
	bool isPositionOverHeroBelt(Common::Point mousePos) const;
	void handleClick(Common::Point mousePos);
	int getCursor(int time);
	bool isHoldingItem() const;
	InventoryItem getHoldingItem() const {
		return _holdingItem;
	}
	const Graphics::Cursor *getHoldingItemCursor(int cursorAnimationFrame) const;
	void clearHold();
	void setColour(HeroBeltColour colour) {
		_colour = colour;
	}
	void reset() {
		_colour = HeroBelt::kWarm;
		_selectedPower = kPowerNone;
		_branchOfLifeFrame = 0;
		_thunderboltFrame = kLightning1;
	}
	void setBranchOfLifeFrame(int frame) {
		_branchOfLifeFrame = frame;
	}
	void setThunderboltFrame(HeroBeltFrame frame) {
		_thunderboltFrame = frame;
	}
	HeroPower getSelectedStrength() {
		return _selectedPower;
	}

private:
	Common::Point computeSlotPoint(int slot, bool fullyExtended);
	Common::String inventoryName(int slot);
	void computeHighlight();
	void clickPower(HeroPower pwr);

	PodImage _background[kNumColours];
	Common::Array<PodImage> _iconNames[kNumColours];
	Common::Array<PodImage> _icons[kNumColours];
	Common::Array<PodImage> _iconCursors[kNumColours];
	Common::Array<PodImage> _iconCursorsBright[kNumColours];
	PodImage _scrollBg[kNumColours];
	PodImage _scrollBgHades[kNumColours];
	PodImage _scrollTextCrete[kNumColours];
	PodImage _scrollTextTroyMale[kNumColours];
	PodImage _scrollTextTroyFemale[kNumColours];
	PodImage _scrollTextMedusa[kNumColours];
	PodImage _scrollTextHades[kNumColours];
	Common::Array<PodImage> _powerImages[3][kNumColours];
	Common::Array<PodImage> _branchOfLife;

	Common::Point _mousePos;
	EventHandlerWrapper _animItemCallbackEvent;
	HotZoneArray _hotZones;
	HeroBeltColour _colour;
	int _heroBeltY;
	int _heroBeltSpeed;
	bool _overHeroBelt;
	bool _bottomEdge;
	int _edgeStartTime;
	InventoryItem _animateItem;
	Common::Point _animateItemStartPoint;
	int _animateItemTargetSlot;
	int _animateItemStartTime;
	int _currentTime;
	int _animItemTime;
	int _hotZone;
	int _startHotTime;
	int _branchOfLifeFrame;
	InventoryItem _holdingItem;
	int _holdingSlot;
	int _highlightTextIdx;
	bool _showScroll;
	HeroPower _selectedPower;
	HeroBeltFrame _thunderboltFrame;
};
}
#endif
