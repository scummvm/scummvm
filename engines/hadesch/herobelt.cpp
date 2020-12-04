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
#include "common/file.h"
#include "hadesch/video.h"
#include "hadesch/pod_image.h"
#include "hadesch/tag_file.h"
#include "hadesch/hadesch.h"
#include "common/system.h"
#include "video/smk_decoder.h"
#include "audio/decoders/aiff.h"
#include "hadesch/pod_file.h"
#include "hadesch/baptr.h"
#include "common/translation.h"

namespace Hadesch {

static const int kHeroBeltMinY = 378;
static const int kHeroBeltMaxY = 471;

static const TranscribedSound powerSounds[3][2] = {
	{
	    { "g0280nc0", _s("That's where hero power of strength will be stored when you earn it") },
	    { "g0280ng0", _s("The power of strength will let you overcome obstacles but you'll need a complete set of hero powers to use it") }
	},
	{
	    { "g0280nb0", _s("That's where hero power of stealth will be stored when you earn it") },
	    { "g0280nf0", _s("The power of stealth allows you to sneak past things but you'll need a complete set of hero powers to use it") }
	},
	{
	    { "g0280ne0", _s("That's where hero power of wisdom will be stored when you earn it") },
	    { "g0280nh0", _s("The power of wisdom will let you outwit and avoid deception but you'll need a complete set of hero powers to use it") }
	},
};

static Common::Array <PodImage> loadImageArray(const Common::String &name) {
	Common::SharedPtr<Common::SeekableReadStream> rs(g_vm->getWdPodFile()->getFileStream(name + ".pod"));
	PodFile pf2(name);
	pf2.openStore(rs);
	return pf2.loadImageArray();
}

static PodImage loadImage(const Common::String &name) {
	return loadImageArray(name)[0];
}

static const struct {
	const char *background;
	const char *iconNames;
	const char *icons;

	// TODO: figure out how the original handles
	// cursors.
	const char *iconCursors;
	const char *iconCursorsBright;

	const char *scrollBg;
	const char *scrollBgHades;
	const char *scrollTextCrete;
	const char *scrollTextTroyFemale;
	const char *scrollTextTroyMale;
	const char *scrollTextMedusa;
	const char *scrollTextHades;
	const char *powerImageWisdom;
	const char *powerImageStrength;
	const char *powerImageStealth;
} assetTable[3] = {
	// Warm
	{
		"g0010oa0",
		"g0091ba0",
		"g0091bb0",
		"g0091bc0",
		"g0093bc0",
		"g0015oy0",
		"g0015oy3",
		"g0015td0",
		"g0015te0",
		"g0015te3",
		"g0015tf0",
		"g0015tg0",
		"g0290oa0",
		"g0290ob0",
		"g0290oc0"
	},

	// Cold
	{
		"g0010ob0",
		"g0092ba0",
		"g0092bb0",
		"g0091bc0",
		"g0093bc0",
		"g0015oy2",
		"g0015oy5",
		"g0015td2",
		"g0015te2",
		"g0015te5",
		"g0015tf2",
		"g0015tg2",
		"g0092oa0",
		"g0092ob0",
		"g0092oc0"
	},

	// Cool
	{
		"g0010oc0",
		"g0093ba0",
		"g0093bb0",
		"g0091bc0",
		"g0093bc0",
		"g0015oy1",
		"g0015oy4",
		"g0015td1",
		"g0015te1",
		"g0015te4",
		"g0015tf1",
		"g0015tg1",
		"g0093oa0",
		"g0093ob0",
		"g0093oc0"
	}
};

HeroBelt::HeroBelt() {
	for (int i = 0; i < 3; i++) {
		_background[i] = loadImage(assetTable[i].background);
		_iconNames[i] = loadImageArray(assetTable[i].iconNames);
		_icons[i] = loadImageArray(assetTable[i].icons);
		_iconCursors[i] = loadImageArray(assetTable[i].iconCursors);
		_iconCursorsBright[i] = loadImageArray(assetTable[i].iconCursorsBright);
		_powerImages[0][i] = loadImageArray(assetTable[i].powerImageStrength);
		_powerImages[1][i] = loadImageArray(assetTable[i].powerImageStealth);
		_powerImages[2][i] = loadImageArray(assetTable[i].powerImageWisdom);

		_scrollBg[i] = loadImage(assetTable[i].scrollBg);
		_scrollBgHades[i] = loadImage(assetTable[i].scrollBgHades);
		_scrollTextCrete[i] = loadImage(assetTable[i].scrollTextCrete);
		_scrollTextTroyFemale[i] = loadImage(assetTable[i].scrollTextTroyFemale);
		_scrollTextTroyMale[i] = loadImage(assetTable[i].scrollTextTroyMale);
		_scrollTextMedusa[i] = loadImage(assetTable[i].scrollTextMedusa);
		_scrollTextHades[i] = loadImage(assetTable[i].scrollTextHades);
	}

	_branchOfLife = loadImageArray("v7150ba0");
	_overHeroBelt = false;
	_heroBeltY = kHeroBeltMaxY;
	_bottomEdge = false;
	_heroBeltSpeed = 0;
	_edgeStartTime = 0;
	_animateItem = kNone;
	_animateItemTargetSlot = -1;
	_hotZone = -1;
	_holdingItem = kNone;
	_holdingSlot = -1;
	_highlightTextIdx = -1;
	_showScroll = false;
	_hotZones.readHotzones(
		Common::SharedPtr<Common::SeekableReadStream>(g_vm->getWdPodFile()->getFileStream("HeroBelt.HOT")),
		true);
}

void HeroBelt::computeHotZone(int time, Common::Point mousePos, bool mouseEnabled) {
	bool wasBottomEdge = _bottomEdge;

	_overHeroBelt = false;
	_bottomEdge = false;

	_mousePos = mousePos;

	if (!mouseEnabled) {
		return;
	}

	_bottomEdge = mousePos.y > 460;
	_overHeroBelt = (_bottomEdge && _heroBeltSpeed < 0) || mousePos.y > _heroBeltY;

	if (!wasBottomEdge && _bottomEdge)
		_edgeStartTime = time;

	_currentTime = time;

	int wasHotZone = _hotZone;
	_hotZone = _hotZones.pointToIndex(mousePos);
	if (_hotZone >= 0 && wasHotZone < 0) {
		_startHotTime = time;
	}

	computeHighlight();
}

static bool isInFrieze() {
	Persistent *persistent = g_vm->getPersistent();
	return (persistent->_currentRoomId == kMinotaurPuzzle && persistent->_quest != kCreteQuest)
		|| (persistent->_currentRoomId == kTrojanHorsePuzzle && persistent->_quest != kTroyQuest)
		|| (persistent->_currentRoomId == kMedusaPuzzle && persistent->_quest != kMedusaQuest)
		|| (persistent->_currentRoomId == kFerrymanPuzzle && persistent->_quest != kRescuePhilQuest)
		|| (persistent->_currentRoomId == kMonsterPuzzle && persistent->_quest != kRescuePhilQuest);
}

void HeroBelt::render(Common::SharedPtr<GfxContext> context, int time, Common::Point viewPoint) {
	Persistent *persistent = g_vm->getPersistent();
	Common::Point beltPoint;

	// TODO: use beltopen hotzone instead?
	if (persistent->_currentRoomId == kMonsterPuzzle) {
		_heroBeltY = kHeroBeltMinY;
	} else {
		if (_bottomEdge && _heroBeltY == kHeroBeltMaxY
		    && (time > _edgeStartTime + 500)) {
			_heroBeltSpeed = -10;
		}

		if (!_overHeroBelt && _heroBeltY != kHeroBeltMaxY
		    && _animateItemTargetSlot == -1) {
			_showScroll = false;
			_heroBeltSpeed = +10;
		}

		if (_heroBeltSpeed != 0) {
			_heroBeltY += _heroBeltSpeed;
			if (_heroBeltY <= kHeroBeltMinY) {
				_heroBeltY = kHeroBeltMinY;
				_heroBeltSpeed = 0;
			}
			if (_heroBeltY >= kHeroBeltMaxY) {
				_heroBeltY = kHeroBeltMaxY;
				_heroBeltSpeed = 0;
			}		
		}
	}

	_currentTime = time;

	beltPoint = Common::Point(0, _heroBeltY) + viewPoint;

	_background[_colour].render(context, beltPoint);

	if (_animateItem != kNone) {
		if (_currentTime > _animateItemStartTime + _animItemTime) {
			_animateItem = kNone;
			_animateItemTargetSlot = -1;
			_animItemCallbackEvent();
		}
	}

	if (_animateItem != kNone) {
		Common::Point target = computeSlotPoint(_animateItemTargetSlot, true);
		Common::Point delta1 = target - _animateItemStartPoint;
		int elapsed = _currentTime - _animateItemStartTime;
		double fraction = ((elapsed + 0.0) / _animItemTime);
		Common::Point deltaPartial = fraction * delta1;
		Common::Point cur = _animateItemStartPoint + deltaPartial;
		_iconCursors[_colour][_animateItem - 1].render(
			context, cur + viewPoint);
	}

	if (persistent->_currentRoomId == kMonsterPuzzle) {
		_icons[_colour][kNumberI].render(
				context,
				computeSlotPoint(0, false) + viewPoint
				+ Common::Point(0, 4));
		_icons[_colour][kNumberII].render(
				context,
				computeSlotPoint(1, false) + viewPoint);
		_icons[_colour][kNumberIII].render(
				context,
				computeSlotPoint(2, false) + viewPoint
				+ Common::Point(0, 4));
		for (unsigned i = 0; i < 3; i++) {
			_icons[_colour][_thunderboltFrame].render(
				context,
				computeSlotPoint(3 + i, false) + viewPoint);
		}

		_branchOfLife[_branchOfLifeFrame].render(
			context, beltPoint);

	} else {
		for (int i = 0; i < inventorySize; i++) {
			if (i == _animateItemTargetSlot || i == _holdingSlot
			    || persistent->_inventory[i] == kNone) {
				continue;
			}
			_icons[_colour][persistent->_inventory[i] - 1].render(
				context,
				computeSlotPoint(i, false) + viewPoint);
		}
	}

	_icons[_colour][persistent->_hintsAreEnabled ? kActiveHints : kInactiveHints].render(
		context,
		computeSlotPoint(6, false) + viewPoint);

	if (isInFrieze())
		_icons[_colour][kReturnToWall].render(
			context,
			computeSlotPoint(7, false) + viewPoint);
	else
		_icons[_colour][kQuestScroll].render(
			context,
			computeSlotPoint(7, false) + viewPoint);

	_icons[_colour][kOptionsButton].render(
		context,
		computeSlotPoint(8, false) + viewPoint);

	if (_highlightTextIdx >= 0) {
		_iconNames[_colour][_highlightTextIdx].render(
			context, beltPoint);
	}

	for (unsigned i = 0; i < ARRAYSIZE(persistent->_powerLevel); i++)
		if (persistent->_powerLevel[i] > 0) {
			_powerImages[i][_colour][(int)i == _selectedPower].render(
				context, beltPoint);
		}

	if (_showScroll) {
		PodImage *bg = _scrollBg, *text = nullptr;
		switch (persistent->_quest) {
		case kCreteQuest:
			text = _scrollTextCrete;
			break;
		case kTroyQuest:
			if (persistent->_gender == kMale)
				text = _scrollTextTroyMale;
			else
				text = _scrollTextTroyFemale;
			break;
		case kMedusaQuest:
			text = _scrollTextMedusa;
			break;
		case kRescuePhilQuest:
			text = _scrollTextHades;
			bg = _scrollBgHades;
			break;
		case kEndGame:
		case kNoQuest:
		case kNumQuests:
			break;
		}			
		bg[_colour].render(context, viewPoint);
		if (text != nullptr)
			text[_colour].render(
				context, viewPoint);
	}
}

bool HeroBelt::isPositionOverHeroBelt(Common::Point mousePos) const {
	return mousePos.y > _heroBeltY;
}

void HeroBelt::handleClick(Common::Point mousePos) {
	Persistent *persistent = g_vm->getPersistent();
	Common::String q = _hotZones.pointToName(mousePos);
	debug("handling belt click on <%s>", q.c_str());
	for (int i = 0; i < inventorySize; i++) {
		if (q == inventoryName(i)) {
			if (_holdingItem != kNone) {
				if (persistent->_inventory[i] != kNone &&
				    _holdingSlot != i)
					return;
				persistent->_inventory[_holdingSlot] = kNone;
				persistent->_inventory[i] = _holdingItem;
				_holdingItem = kNone;
				_holdingSlot = -1;
				return;
			}
			if (i == _animateItemTargetSlot
			    || persistent->_inventory[i] == kNone)
				return;
			_holdingItem = persistent->_inventory[i];
			_holdingSlot = i;
			return;
		}
	}

	if (q == "quest scroll") {
		if (isInFrieze())
			g_vm->moveToRoom(kWallOfFameRoom);
		else
			_showScroll = true;
	}

	if (q == "hints") {
		persistent->_hintsAreEnabled = !persistent->_hintsAreEnabled;
	}

	if (q == "options") {
		g_vm->enterOptions();
	}

	if (q == "strength")
		clickPower(kPowerStrength);
	if (q == "stealth")
		clickPower(kPowerStealth);
	if (q == "wisdom")
		clickPower(kPowerWisdom);
}

void HeroBelt::clickPower(HeroPower pwr) {
	Persistent *persistent = g_vm->getPersistent();
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

	if (persistent->_currentRoomId == kMonsterPuzzle) {
		_selectedPower = pwr;
		return;
	}

	if (persistent->_quest == kRescuePhilQuest)
		return;

	room->playSpeech(powerSounds[pwr][!!persistent->_powerLevel[pwr]]);
}

void HeroBelt::computeHighlight() {
	Common::String hotZone = _hotZones.indexToName(_hotZone);
	Persistent *persistent = g_vm->getPersistent();
	for (int i = 0; i < inventorySize; i++) {
		if (hotZone == inventoryName(i)) {
			if (persistent->_currentRoomId == kMonsterPuzzle) {
				_highlightTextIdx = i < 3 ? kNumberI + i : kLightning1 + i - 3;
				return;
			} else {
				if (persistent->_inventory[i] != kNone &&
				    _holdingSlot != i) {
					_highlightTextIdx = persistent->_inventory[i] - 1;
					return;
				}
			}
		}
	}

	if (hotZone == "quest scroll") {
		// TODO: what's with 30 and 28?
		if (isInFrieze())
			_highlightTextIdx = kReturnToWall;
		else if (persistent->_quest == kRescuePhilQuest)
			_highlightTextIdx = kHadesScroll;
		else
			_highlightTextIdx = kQuestScroll;
		return;
	}

	if (hotZone == "stealth") {
		_highlightTextIdx = kPowerOfStealth;
		return;
	}

	if (hotZone == "strength") {
		_highlightTextIdx = kPowerOfStrength;
		return;
	}

	if (hotZone == "wisdom") {
		_highlightTextIdx = kPowerOfWisdom;
		return;
	}

	if (hotZone == "hints") {
		_highlightTextIdx = persistent->_hintsAreEnabled ? kActiveHints : kInactiveHints;
		return;
	}

	if (hotZone == "options") {
		_highlightTextIdx = kOptionsButton;
		return;
	}

	_highlightTextIdx = -1;
}

void HeroBelt::placeToInventory(InventoryItem item, EventHandlerWrapper callbackEvent) {
	Persistent *persistent = g_vm->getPersistent();
	unsigned i;
	for (i = 0; i < inventorySize; i++) {
		if (persistent->_inventory[i] == kNone)
			break;
	}

	if (i == inventorySize) {
		debug("Out of inventory space");
		return;
	}

	persistent->_inventory[i] = item;
	_animateItem = item;
	_animItemCallbackEvent = callbackEvent;
	_animateItemStartPoint = _mousePos;
	_animateItemTargetSlot = i;
	_animateItemStartTime = _currentTime;
	_animItemTime = 2000;
	_heroBeltSpeed = -10;
}

void HeroBelt::removeFromInventory(InventoryItem item) {
	Persistent *persistent = g_vm->getPersistent();
	for (unsigned i = 0; i < inventorySize; i++) {
		if (persistent->_inventory[i] == item) {
			persistent->_inventory[i] = kNone;
		}
	}

	if (_holdingItem == item) {
		_holdingItem = kNone;
		_holdingSlot = -1;
	}

	if (_animateItem == item) {
		_animateItem = kNone;
		_animateItemTargetSlot = -1;
	}
}

void HeroBelt::clearHold() {
	_holdingItem = kNone;
	_holdingSlot = -1;
}

Common::Point HeroBelt::computeSlotPoint(int slot, bool fullyExtended) {
	Common::Point ret = Common::Point(19, 35);
	ret += Common::Point(0, (fullyExtended ? kHeroBeltMinY : _heroBeltY));
	ret += Common::Point(39 * slot, slot % 2 ? 4 : 0);
	if (slot >= 6) {
		ret += Common::Point(253, 0);
	}
	return ret;
}

int HeroBelt::getCursor(int time) {
	Common::String q = _hotZones.indexToName(_hotZone);
	Persistent *persistent = g_vm->getPersistent();
	if (q == "")
		return 0;
	for (unsigned i = 0; i < inventorySize; i++) {
		if (q == inventoryName(i)) {
			if ((int) i != _animateItemTargetSlot
			    && persistent->_inventory[i] != kNone)
				return (time - _startHotTime) / kDefaultSpeed % 3;
			return 0;
		}
	}

	return (time - _startHotTime) / kDefaultSpeed % 3;
}

Common::String HeroBelt::inventoryName(int slot) {
	return Common::String::format("inventory%d", slot);
}

bool HeroBelt::isHoldingItem() const {
	return _holdingItem != kNone;
}

const Graphics::Cursor *HeroBelt::getHoldingItemCursor(int cursorAnimationFrame) const {
	if ((cursorAnimationFrame / 2) % 2 == 1)
		return &_iconCursorsBright[_colour][_holdingItem - 1];
	else
		return &_iconCursors[_colour][_holdingItem - 1];
}

}
