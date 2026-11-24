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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "dune/dune.h"
#include "dune/actor_pool.h"
#include "dune/sprite_sheet.h"
#include "common/stream.h"
#include "common/system.h"

namespace Dune {

bool DuneEngine::mapAnimationVisible() const {
	return _onWorld && !_onGlobe && !_inCabin && !_onMixer &&
		_menuKind != kMenuTroopNotice && _menuKind != kMenuLeaving;
}

void DuneEngine::clearMapUnits() {
	if (!_mapActorPoolOrder.empty())
		WRITE_LE_UINT16(_state.begin() + kSpriteActorCount, 0);
	_mapActorPoolOrder.clear(); // 5DA5 clears the count, never the backing records.
	for (uint i = 0; i < ARRAYSIZE(_mapUnits); i++)
		_mapUnits[i] = MapUnitAnimation();
	_mapRing = MapUnitAnimation();
	_mapRingTroop = -1;
	_mapRingOrder = 0;
	_mapUnitOrder.clear();
	_mapActorNeedsRepaint = false;
}

/** C5CF/C60B: retain ONMAP records in the pool later reused by ATTACK. */
void DuneEngine::appendMapActor(uint key) {
	const MapUnitAnimation &unit = key == ARRAYSIZE(_mapUnits) ? _mapRing : _mapUnits[key];
	if (!unit.initialList)
		return;
	openSpriteSheet("ONMAP.HSQ", false);
	const Common::Rect bounds = mapUnitBounds(unit);
	const bool ring = key == ARRAYSIZE(_mapUnits);
	const uint at = appendSpriteActor(_state.begin(), bounds.left, bounds.top, bounds.right, bounds.bottom,
			unit.sprite, 0x8aa + 27 * (ring ? _mapRingTroop : key), unit.flags | (ring ? 0x40 : 0));
	if (!at)
		return;
	_mapActorPoolOrder.push_back(key);
	if (unit.flags & 1) {
		WRITE_LE_UINT16(_state.begin() + at + 13, unit.next);
		WRITE_LE_UINT16(_state.begin() + at + 15, unit.list);
	}
	if (ring)
		WRITE_LE_UINT16(_state.begin() + 0x4752, at); // 697C
}

/** 6B34/C661/7C24 update a registered record, not a freshly cleared slot. */
void DuneEngine::updateMapActor(uint key, bool framePointersOnly) {
	for (uint slot = 0; slot < _mapActorPoolOrder.size(); slot++) {
		if (_mapActorPoolOrder[slot] != key)
			continue;
		const MapUnitAnimation &unit = key == ARRAYSIZE(_mapUnits) ? _mapRing : _mapUnits[key];
		byte *record = _state.begin() + kSpriteActorPool + kSpriteActorSize * slot;
		if (unit.flags & 1) {
			WRITE_LE_UINT16(record + 13, unit.next);
			WRITE_LE_UINT16(record + 15, unit.list);
		}
		if (!framePointersOnly) {
			const Common::Rect bounds = mapUnitBounds(unit);
			const bool ring = key == ARRAYSIZE(_mapUnits);
			writeSpriteActor(record, bounds.left, bounds.top, bounds.right, bounds.bottom,
					unit.sprite, 0x8aa + 27 * (ring ? _mapRingTroop : key), unit.flags | (ring ? 0x40 : 0));
		}
		return;
	}
}

void DuneEngine::removeMapActor(uint key) {
	for (uint slot = 0; slot < _mapActorPoolOrder.size(); slot++) {
		if (_mapActorPoolOrder[slot] == key) {
			removeSpriteActor(_state.begin(), kSpriteActorPool + kSpriteActorSize * slot);
			_mapActorPoolOrder.remove_at(slot);
			return;
		}
	}
}

void DuneEngine::startMapRing(uint troop, int x, int y, uint16 &seed) {
	removeMapRing();
	_mapRing.start(_state.begin(), _state.size(), 0x18fd, x, y, seed);
	_mapRingTroop = troop;
	_mapRingOrder = _mapUnitOrder.size();
	appendMapActor(ARRAYSIZE(_mapUnits));
}

void DuneEngine::removeMapRing() {
	WRITE_LE_UINT16(_state.begin() + 0x4752, 0); // 69A3 clears before C58A relocates handles.
	removeMapActor(ARRAYSIZE(_mapUnits));
	_mapRing = MapUnitAnimation();
	_mapRingTroop = -1;
}

/** 66B1 removes the troop figure while the contact keeps its selection ring. */
bool DuneEngine::hasRemovedTroopContact(uint troop) const {
	if (troop >= ARRAYSIZE(_mapUnits) || troop != _orderingTroop || !hasTroopContact())
		return false;
	const uint record = 0x8aa + 27 * troop;
	return READ_LE_UINT16(_state.begin() + 0x46ef) == record &&
		_state[record + 3] == 0xa0 && READ_LE_UINT16(_state.begin() + record + 4) == 0xfbc &&
		_state[record + 0x1a] == 0;
}

/** 5D82: recreate the whole pool, without re-registering its timer. */
void DuneEngine::rebuildMapUnits() {
	clearMapUnits();
	WRITE_LE_UINT16(_state.begin() + kSpriteActorCount, 0); // 5DA5, including an empty old map.
	// 5DB9 calls 697C, not 7C24: even a remote contact gets a fresh 18FD
	// ring. Preserve 4C's dialogue/range gate, but do not reuse its 1916 list.
	syncMapUnits(false);
}

void DuneEngine::syncMapUnits(bool restoreContactRange) {
	// DOS 6B34 advances the already registered objects without calling 6715.
	// Our animation uses the retained compositor, but that must not turn its
	// partial redraw into a fresh troop-chain scan. A real 5A9A/5D82 map
	// redraw still uses the scan below.
	// 8831 has already moved the camera while 7B36 restores the old report.
	// Its C6AD painters still use the retained actors, not a newly projected
	// pool. Only the following 5A9A rebuild may use the new centre.
	if (_mapAnimationRedraw && _mapAnimationActive)
		return;
	if (!_mapAnimationActive || _mapUnitsLongitude != _worldLongitude || _mapUnitsLatitude != _worldLatitude) {
		clearMapUnits(); // 5A9A/5D82 rebuild the pool after moving the viewport.
		WRITE_LE_UINT16(_state.begin() + kSpriteActorCount, 0);
		_mapUnitsLongitude = _worldLongitude;
		_mapUnitsLatitude = _worldLatitude;
	}
	if (!_mapAnimationActive) {
		_mapAnimationActive = true;
		_mapAnimationRemainder = 0;
		_mapAnimationTime = _system->getMillis() / 5;
	}
	const Common::Array<Troop> &troops = _map.troops();
	bool seen[67] = {};
	uint16 seed = READ_LE_UINT16(_state.begin() + 0xd828);
	auto add = [&](uint i) {
		if (i >= troops.size() || i >= ARRAYSIZE(_mapUnits) || seen[i])
			return;
		const Troop &troop = troops[i];
		const uint list = troopSpriteList(troop, i);
		if (!list)
			return;
		const Common::Point at = troopOnScreen(troop, i);
		// 68AF permits travelling actors just beyond the viewport; their
		// individual sprites are subsequently clipped by C6AD.
		if ((troop.state & 0x40) && (at.x <= -16 || at.y <= -16 || at.x >= 328 || at.y >= 160))
			return;
		seen[i] = true;
		MapUnitAnimation &unit = _mapUnits[i];
		// 8326/C653 keeps an existing traveller's frame list even if its
		// live bearing has changed. Assignment/arrival use 8461 explicitly.
		const bool newList = unit.initialList != list && !((troop.state & 0x40) && unit.initialList);
		if (newList || unit.anchorX != at.x || unit.anchorY != at.y) {
			removeMapActor(i);
			// 8461 removes the previous object and appends its replacement.
			// Preserve that registration order for 6946's reverse hit test.
			for (uint entry = 0; entry < _mapUnitOrder.size(); entry++) {
				if (_mapUnitOrder[entry] == i) {
					if (entry < _mapRingOrder)
						_mapRingOrder--;
					_mapUnitOrder.remove_at(entry);
					break;
				}
			}
			unit.start(_state.begin(), _state.size(), list, at.x, at.y, seed);
			_mapUnitOrder.push_back(i);
			appendMapActor(i);
		}
	};
	// 6715 visits each visible place's linked troop chain, then travellers
	// in record order. This is also the order of C5CF's E3DF draws.
	uint descriptor = 0xa5c0;
	for (uint p = 0; p < _map.places().size(); p++) {
		const Place &place = _map.places()[p];
		const Common::Point at = placeOnScreen(place);
		if (!place.isKnown() || at.x < 4 || at.x >= 316 || at.y < 4 || at.y >= 148)
			continue;
		// Retain 5DCE's place list for 5ED0/846C. A fort revealed since
		// this rebuild is not on the displayed map yet, even if 46EC's
		// refresh request wrapped to zero. Actor discovery must not add it.
		WRITE_LE_UINT16(_state.begin() + descriptor, 0x100 + 28 * p);
		WRITE_LE_UINT16(_state.begin() + descriptor + 2, at.x);
		WRITE_LE_UINT16(_state.begin() + descriptor + 4, at.y | 0x8000);
		descriptor += 6;
		uint id = _state[0x100 + 28 * p + 9];
		for (uint count = 0; id && id <= troops.size() && count < 67; count++) {
			add(id - 1);
			id = _state[0x8aa + 27 * (id - 1) + 1];
		}
	}
	WRITE_LE_UINT16(_state.begin() + descriptor, 0);
	for (uint i = 0; i < troops.size() && i < ARRAYSIZE(_mapUnits); i++) {
		if (troops[i].state & 0x40)
			add(i);
		if (!seen[i])
			_mapUnits[i] = MapUnitAnimation();
	}
	for (uint entry = _mapUnitOrder.size(); entry > 0; entry--) {
		if (!seen[_mapUnitOrder[entry - 1]]) {
			removeMapActor(_mapUnitOrder[entry - 1]);
			if (entry - 1 < _mapRingOrder)
				_mapRingOrder--;
			_mapUnitOrder.remove_at(entry - 1);
		}
	}
	if (hasTroopContact() && _orderingTroop < ARRAYSIZE(_mapUnits) && seen[_orderingTroop]) {
		const MapUnitAnimation &unit = _mapUnits[_orderingTroop];
		if (_mapRingTroop != (int)_orderingTroop || _mapRing.anchorX != unit.anchorX || _mapRing.anchorY != unit.anchorY) {
			startMapRing(_orderingTroop, unit.anchorX, unit.anchorY, seed);
			if (restoreContactRange && _state[0x4c]) { // 7C24 replaces only the two frame pointers.
				_mapRing.list = _mapRing.next = 0x1916;
				updateMapActor(ARRAYSIZE(_mapUnits), true);
			}
		}
	} else if (_mapRing.initialList && !hasRemovedTroopContact(_mapRingTroop)) {
		removeMapRing();
	}
	WRITE_LE_UINT16(_state.begin() + 0xd828, seed);
}

/** DOS 6917/C58A: remove an actor without registering a replacement. */
void DuneEngine::removeMapUnit(uint index) {
	// 6917 checks the signed mode byte, not whether the native view flags
	// currently expose the map. A covering modal can still own live actors,
	// whereas globe, cabin and closed-map modes cannot replace them.
	if (!_mapAnimationActive || !(_state[0x46eb] & 0x80) ||
		index >= ARRAYSIZE(_mapUnits) ||
		!_mapUnits[index].initialList)
		return; // 6917 found no active actor for this troop.
	openSpriteSheet("ONMAP.HSQ", false);
	const Common::Rect oldBounds = mapUnitBounds(_mapUnits[index]);
	removeMapActor(index);
	for (uint entry = 0; entry < _mapUnitOrder.size(); entry++) {
		if (_mapUnitOrder[entry] == index) {
			if (entry < _mapRingOrder)
				_mapRingOrder--;
			_mapUnitOrder.remove_at(entry);
			break;
		}
	}
	_mapUnits[index] = MapUnitAnimation(); // C58A removes the old object first.
	paintMapActorChange(oldBounds); // C58A/C6AD presents before replacing it.
}

/** DOS 8461: an order change replaces the live actor, even if its list is unchanged. */
void DuneEngine::replaceMapUnit(uint index) {
	if (!_mapAnimationActive || !(_state[0x46eb] & 0x80) ||
		index >= ARRAYSIZE(_mapUnits) || !_mapUnits[index].initialList)
		return;
	_map.readTroops(_state.begin(), _state.size());
	removeMapUnit(index);
	registerMapUnit(index);
}

/** DOS 846C: register and paint a newly visible figure, without removing one. */
void DuneEngine::registerMapUnit(uint index) {
	if (!_mapAnimationActive || !(_state[0x46eb] & 0x80) || index >= ARRAYSIZE(_mapUnits))
		return;
	_map.readTroops(_state.begin(), _state.size());
	if (index >= _map.troops().size())
		return;

	const Troop &troop = _map.troops()[index];
	const uint list = troopSpriteList(troop, index);
	if (!list)
		return;
	Common::Point at = troopOnScreen(troop, index);
	if (troop.state & 0x40) {
		// 68AF's travelling-object margin precedes the sprite's own clipping.
		if (at.x <= -16 || at.y <= -16 || at.x >= 328 || at.y >= 160)
			return;
	} else {
		uint descriptor = 0xa5c0;
		while (descriptor + 6 <= _state.size() &&
				READ_LE_UINT16(_state.begin() + descriptor) &&
				READ_LE_UINT16(_state.begin() + descriptor) != troop.place)
			descriptor += 6;
		if (descriptor + 6 > _state.size() ||
				READ_LE_UINT16(_state.begin() + descriptor) != troop.place)
			return;
		if (_state[descriptor + 5] < 0x80)
			return; // 687B rejects an unsigned (non-strategic) site descriptor.
		// 846C/5ED0 uses the retained site's screen position. During 8831's
		// equipment unwind the camera has moved, but 5A9A has not rebuilt
		// these descriptors yet. Keep the troop's standing-point offset.
		if (troop.place < 0x100 || (uint)(troop.place - 0x100) / 28 >= _map.places().size())
			return;
		at -= placeOnScreen(_map.places()[(troop.place - 0x100) / 28]);
		at += Common::Point(READ_LE_INT16(_state.begin() + descriptor + 2),
				_state[descriptor + 4]); // 687B clears the entire flag byte, not just bit 15.
	}

	uint16 seed = READ_LE_UINT16(_state.begin() + 0xd828);
	_mapUnits[index].start(_state.begin(), _state.size(), list, at.x, at.y, seed);
	WRITE_LE_UINT16(_state.begin() + 0xd828, seed);
	if (_mapUnits[index].initialList) {
		_mapUnitOrder.push_back(index); // 8461 appends the replacement.
		appendMapActor(index);
		paintMapActorChange(mapUnitBounds(_mapUnits[index])); // 847F/C6AD
	}
}

/** DOS 8326: move a traveller's existing actor without restarting its frames. */
void DuneEngine::updateTravellingMapUnit(uint index) {
	if (!_mapAnimationActive || !(_state[0x46eb] & 0x80) || index >= ARRAYSIZE(_mapUnits))
		return;
	_map.readTroops(_state.begin(), _state.size());
	if (index >= _map.troops().size())
		return;
	const Troop &troop = _map.troops()[index];
	const Common::Point at = troopOnScreen(troop, index);
	if (at.x <= -16 || at.y <= -16 || at.x >= 328 || at.y >= 160) {
		removeMapUnit(index); // 8333/C58A
		return;
	}
	MapUnitAnimation &unit = _mapUnits[index];
	openSpriteSheet("ONMAP.HSQ", false);
	if (unit.initialList) {
		// C653 centres the CURRENT frame, then translates its retained bounding
		// box. Its list, frame, phase and random seed are untouched. The panel
		// may currently own ICONES, so query dimensions from ONMAP directly.
		Resource bank = _archive.openMember("ONMAP.HSQ");
		if (!bank)
			return;
		SpriteSheet onmap(this, bank, false);
		int firstWidth, firstHeight, width, height;
		if (!onmap.size(_state[unit.initialList], &firstWidth, &firstHeight) ||
			!onmap.size(unit.sprite, &width, &height))
			return;
		Common::Rect dirty = mapUnitBounds(unit);
		unit.anchorX = (int16)(at.x - width / 2 + firstWidth / 2 - unit.x);
		unit.anchorY = (int16)(at.y - height / 2 + firstHeight / 2 - unit.y);
		updateMapActor(index);
		dirty.extend(mapUnitBounds(unit));
		paintMapActorChange(dirty); // C653/C661 presents the translated union.
		return;
	}
	const uint list = troopSpriteList(troop, index);
	if (!list)
		return;
	uint16 seed = READ_LE_UINT16(_state.begin() + 0xd828);
	unit.start(_state.begin(), _state.size(), list, at.x, at.y, seed); // 833C/C5CF
	WRITE_LE_UINT16(_state.begin() + 0xd828, seed);
	if (unit.initialList) {
		_mapUnitOrder.push_back(index);
		appendMapActor(index);
		paintMapActorChange(mapUnitBounds(unit)); // 8346/C6AD, not C5CF alone.
	}
}

/** C6AD/C51E: restore and present only the affected actor rectangle. */
void DuneEngine::paintMapActorChange(Common::Rect bounds) {
	bounds.clip(Common::Rect(4, 4, 316, 148));
	if (bounds.isEmpty())
		return;
	// The compositor retains the current pool and foregrounds, and must not
	// consume another pending host repaint or change neighbouring phases.
	restoreMapInformationBackground(bounds);
	_system->copyRectToScreen(_framebuffer1 + bounds.top * kScreenWidth + bounds.left,
			kScreenWidth, bounds.left, bounds.top, bounds.width(), bounds.height());
	_system->updateScreen();
}

Common::Rect DuneEngine::mapUnitBounds(const MapUnitAnimation &unit) const {
	int width, height;
	if (!unit.initialList || !currentSpriteSheet || !currentSpriteSheet->size(_state[unit.initialList], &width, &height))
		return Common::Rect();
	const int x = (int16)(unit.anchorX - width / 2 + unit.x);
	const int y = (int16)(unit.anchorY - height / 2 + unit.y);
	// C60B retains the initial frame's extents; C661 translates that box
	// without re-centring it when the next sprite has different dimensions.
	return Common::Rect(x, y, x + width, y + height);
}

Common::Rect DuneEngine::drawMapUnit(const MapUnitAnimation &unit, const Common::Rect *dirty) {
	const Common::Rect bounds = mapUnitBounds(unit);
	const Common::Rect clip = dirty ? *dirty : Common::Rect(4, 4, 316, 148);
	if (!bounds.isEmpty() && bounds.intersects(clip))
		currentSpriteSheet->drawMapSprite(unit.sprite, bounds.left, bounds.top, clip);
	return bounds;
}

void DuneEngine::drawMapUnits(const Common::Rect *dirty) {
	syncMapUnits();
	_troopRects.clear();
	_troopRects.resize(_map.troops().size());
	// Composite through the original map viewport, leaving its four-pixel
	// border and the panel untouched even for a partially visible actor.
	memcpy(_framebuffer2, _framebuffer1, kScreenWidth * kScreenHeight);
	_graphics.setFrameBuffer(_framebuffer2);
	for (uint i = 0; i < _mapUnitOrder.size(); i++)
		_troopRects[_mapUnitOrder[i]] = mapUnitBounds(_mapUnits[_mapUnitOrder[i]]);
	// 5A86 selects C835: paint in increasing right+bottom order, choosing
	// the later registered actor first on ties. The ring is always last.
	Common::Array<uint> paintOrder = _mapUnitOrder;
	for (uint i = 1; i < paintOrder.size(); i++) {
		const uint troop = paintOrder[i];
		const uint16 depth = _troopRects[troop].right + _troopRects[troop].bottom;
		uint j = i;
		while (j && (uint16)(_troopRects[paintOrder[j - 1]].right + _troopRects[paintOrder[j - 1]].bottom) >= depth) {
			paintOrder[j] = paintOrder[j - 1];
			j--;
		}
		paintOrder[j] = troop;
	}
	for (uint i = 0; i < paintOrder.size(); i++)
		drawMapUnit(_mapUnits[paintOrder[i]], dirty);
	if (_mapRing.list && !_mapRebuildRedraw)
		drawMapUnit(_mapRing, dirty);
	for (int y = 4; y < 148; y++)
		memcpy(_framebuffer1 + y * kScreenWidth + 4, _framebuffer2 + y * kScreenWidth + 4, 312);
	_graphics.setFrameBuffer(_framebuffer1);
	_mapActorNeedsRepaint = false;
}

/** C6AD/C7D4: paint actors beneath, not through, retained map windows. */
void DuneEngine::repaintMapActors(bool present, const Common::Rect *dirty) {
	const Common::Rect view(4, 4, 316, 148);
	Common::Array<byte> visible(kScreenWidth * kScreenHeight);
	memcpy(visible.begin(), _framebuffer1, visible.size());
	const bool dragging = _surveyDragging && canDragSurvey();
	if (dragging)
		drawSurveyDragOutline(visible.begin(), kScreenWidth, _surveyDragPosition);
	_graphics.setFrameBuffer(_framebuffer1);
	// C443 restores C412's clean page, not freshly rendered terrain. Planting
	// outside the request viewport can affect visible trees, but DOS doesn't
	// display that growth until a real 5A9A/5D82 redraw updates the background.
	if (_mapCleanPage.size() == visible.size()) {
		memcpy(_framebuffer1, _mapCleanPage.begin(), _mapCleanPage.size());
	} else {
		// A callback normally follows showWorld; keep direct actor setup usable.
		// Select banks without changing a contacted chief's palette.
		_map.drawStrategic(_framebuffer1, kScreenWidth, _worldLongitude, _worldLatitude);
		drawMapScenery();
	}
	openSpriteSheet("ONMAP.HSQ", false);
	drawMapUnits(dirty);
	// C780 restores the panel crest only for dirty rectangles that reach
	// its part of the viewport, before C7D4 restores foreground windows.
	const Common::Rect &paint = dirty ? *dirty : view;
	if (!_state[0x227d] && paint.bottom >= 137 && paint.right >= 126 && paint.left < 194) {
		_graphics.setFrameBuffer(_framebuffer2);
		drawFace();
		_graphics.setFrameBuffer(_framebuffer1);
		openSpriteSheet("ONMAP.HSQ", false); // 1797 restores the caller's bank.
	}

	// C51E presents only the map viewport; its border and the command panel
	// are not part of this callback. drawMapUnits left the new actor page in 2.
	memcpy(_framebuffer1, visible.begin(), visible.size());
	for (int y = view.top; y < view.bottom; y++)
		memcpy(_framebuffer1 + y * kScreenWidth + view.left,
				_framebuffer2 + y * kScreenWidth + view.left, view.width());
	retainMapForegrounds(visible.begin());
	if (dragging)
		drawSurveyDragOutline(_framebuffer1, kScreenWidth, _surveyDragPosition);
	if (present) {
		_system->copyRectToScreen(_framebuffer1 + view.top * kScreenWidth + view.left,
				kScreenWidth, view.left, view.top, view.width(), view.height());
		_system->updateScreen();
	}
}

/** C7D4: the current displayed windows survive actor paints and 5D82. */
void DuneEngine::retainMapForegrounds(const byte *visible) {
	const Common::Rect view(4, 4, 316, 148);
	auto retain = [this, visible, &view](Common::Rect box) {
		box.clip(view);
		if (box.isEmpty())
			return;
		for (int y = box.top; y < box.bottom; y++)
			memcpy(_framebuffer1 + y * kScreenWidth + box.left,
					visible + y * kScreenWidth + box.left, box.width());
	};
	// Native foreground geometry corresponds to DOS DBE0/DBE2. Its window
	// descriptors are not all mirrored into DS, so use the same bounds as
	// the full painters, never infer them from nonzero/opaque sprite pixels.
	if (_menuKind == kMenuMap && READ_LE_UINT16(_state.begin() + 0xdbe0) == 0x194a) {
		const byte *box = _state.begin() + 0x194a;
		retain(Common::Rect(READ_LE_UINT16(box), READ_LE_UINT16(box + 2),
				READ_LE_UINT16(box + 4), READ_LE_UINT16(box + 6)));
	}
	if (hasTroopContact() && READ_LE_UINT16(_state.begin() + 0xdbe0) == 0x18e9) {
		const int top = _troopContactTop ? _troopContactTop : 5;
		retain(Common::Rect(5, top, 232, top + 67));
		if (_menuKind == kMenuEquipment && _state[0x46f5]) {
			retain(equipmentCardRect(true));
		}
	}
	// C7A8 skips both foreground slots when DBE0 is empty. During 8685,
	// the old chief is released before MAP2: DBE2 still names the child,
	// but its pixels no longer survive the chief's closing repaint.
	const uint16 mainWindow = READ_LE_UINT16(_state.begin() + 0xdbe0);
	if (mainWindow == 0x4710 || (mainWindow &&
			READ_LE_UINT16(_state.begin() + 0xdbe2) == 0x4710)) {
		const byte *box = _state.begin() + 0x4710;
		retain(Common::Rect(READ_LE_INT16(box), READ_LE_INT16(box + 2),
				READ_LE_INT16(box + 4), READ_LE_INT16(box + 6)));
	}
	if ((_menuKind == kMenuPlace || _menuKind == kMenuMap) && _pressedPlace >= 0) {
		// C7D4 reads the displayed frame, not 6252's new report kind. Work
		// may have changed that kind before 1C18 replaces the old window.
		if (READ_LE_UINT16(_state.begin() + 0x46f8) == 0x100 + 28 * _pressedPlace &&
				READ_LE_UINT16(_state.begin() + 0xdbe0) == 0x1668) {
			const byte *box = _state.begin() + 0x1668;
			retain(Common::Rect(READ_LE_INT16(box), READ_LE_INT16(box + 2),
					READ_LE_INT16(box + 4), READ_LE_INT16(box + 6)));
		} else {
			// Full redraw reconstructs older native saves' missing DS fields.
			retain(_mapWindowGeometryValid ? _mapWindowBounds : mapPlaceCardBounds(_pressedPlace));
		}
	}
	// C7D4 keeps the compact report even after its actor was removed. Only
	// 78E9/1C18 queries actor presence and closes it; an animation repaint
	// must neither perform that query nor replace the visible card pixels.
	if (_menuKind == kMenuMap && mapTroopCard() >= 0)
		retain(mapTroopCardBounds());
}

void DuneEngine::animateMap() {
	if (!mapAnimationVisible())
		return;
	_mapAnimationFrame++;
	openSpriteSheet("ONMAP.HSQ", false);
	auto advance = [this](MapUnitAnimation &unit, uint key) {
		if (!(unit.flags & 1) || !unit.list || unit.next >= _state.size() ||
				(!_state[unit.next] && (unit.flags & 2)))
			return; // 6B55/6B67 do not repaint static or finished objects.
		const uint frame = _state[unit.next] ? unit.next : unit.list;
		if (frame + 2 >= _state.size() || !_state[frame])
			return;
		Common::Rect dirty = mapUnitBounds(unit);
		unit.step(_state.begin(), _state.size());
		updateMapActor(key);
		dirty.extend(mapUnitBounds(unit)); // C661 restores the old/new union.
		dirty.clip(Common::Rect(4, 4, 316, 148));
		if (dirty.isEmpty())
			return;
		// 6B7F/C6AD paints one changed object at a time, retaining every
		// pixel outside its dirty rectangle, even an unregistered old legend.
		paintMapActorChange(dirty);
	};
	// 6B34 updates only the selected ring on three out of four callbacks.
	// The fourth updates every actor (including the ring) once.
	if (!(++_state[0x46f6] & 3)) {
		for (uint i = 0; i <= _mapUnitOrder.size(); i++) {
			if (i == _mapRingOrder)
				advance(_mapRing, ARRAYSIZE(_mapUnits));
			if (i < _mapUnitOrder.size())
				advance(_mapUnits[_mapUnitOrder[i]], _mapUnitOrder[i]);
		}
	} else {
		advance(_mapRing, ARRAYSIZE(_mapUnits));
	}
}

bool DuneEngine::pollMapAnimation(uint16 ticks) {
	if (!_mapAnimationActive || !mapAnimationVisible())
		return false;
	// D9D2 adds elapsed ticks in AX, calls at most once, and retains modulo
	// 15. Long stalls and the 16-bit clock wrap must not replay every frame.
	const uint16 elapsed = ticks + _mapAnimationRemainder;
	_mapAnimationRemainder = elapsed % 15;
	if (elapsed < 15) {
		// Actor lifecycle actions already present their own dirty rectangles.
		// Keep the full-composite fallback for an explicitly pending host paint.
		if (_mapActorNeedsRepaint) {
			_mapAnimationRedraw = true;
			showWorld();
			_mapAnimationRedraw = false;
		}
		return false;
	}
	animateMap();
	return true;
}

void DuneEngine::updateMapAnimation() {
	const uint32 now = animationClockTicks();
	if (!mapAnimationVisible()) {
		if (!_onWorld) {
			_mapAnimationActive = false;
			clearMapUnits();
		}
		_mapAnimationTime = now;
		return;
	}
	const uint16 elapsed = animationElapsedTicks(_mapAnimationTime, now);
	_mapAnimationTime = now;
	pollMapAnimation(elapsed);
}

void DuneEngine::saveMapAnimations(Common::WriteStream *stream) const {
	const bool active = _mapAnimationActive && mapAnimationVisible();
	stream->writeByte(active);
	if (!active)
		return;
	stream->writeUint16LE(_mapAnimationRemainder);
	stream->writeUint32LE(_mapAnimationFrame);
	stream->writeSint16LE(_mapRingTroop);
	for (uint i = 0; i <= ARRAYSIZE(_mapUnits); i++) {
		const MapUnitAnimation &unit = i == ARRAYSIZE(_mapUnits) ? _mapRing : _mapUnits[i];
		stream->writeUint16LE(unit.list);
		stream->writeUint16LE(unit.initialList);
		stream->writeUint16LE(unit.next);
		stream->writeUint16LE(unit.sprite);
		stream->writeSint16LE(unit.x);
		stream->writeSint16LE(unit.y);
		stream->writeSint16LE(unit.anchorX);
		stream->writeSint16LE(unit.anchorY);
		stream->writeByte(unit.flags);
	}
	stream->writeByte(_mapUnitOrder.size());
	for (uint i = 0; i < _mapUnitOrder.size(); i++)
		stream->writeByte(_mapUnitOrder[i]);
	stream->writeByte(_mapRing.list ? _mapRingOrder : _mapUnitOrder.size());
}

bool DuneEngine::loadMapAnimations(Common::ReadStream *stream) {
	// DS has already been loaded. Discard the previous runtime identities
	// without clearing the new save's (possibly ATTACK-owned) pool count.
	_mapActorPoolOrder.clear();
	clearMapUnits();
	_mapAnimationActive = false;
	_mapAnimationRemainder = 0;
	_mapAnimationTime = _system->getMillis() / 5;
	const byte active = stream->readByte();
	if (active > 1 || stream->err() || stream->eos())
		return false;
	if (!active)
		return true;
	_mapAnimationRemainder = stream->readUint16LE();
	_mapAnimationFrame = stream->readUint32LE();
	_mapRingTroop = stream->readSint16LE();
	if (!mapAnimationVisible() || _mapAnimationRemainder >= 15 || _mapRingTroop < -1 || _mapRingTroop >= 67)
		return false;
	for (uint i = 0; i <= ARRAYSIZE(_mapUnits); i++) {
		MapUnitAnimation &unit = i == ARRAYSIZE(_mapUnits) ? _mapRing : _mapUnits[i];
		unit.list = stream->readUint16LE();
		unit.initialList = stream->readUint16LE();
		unit.next = stream->readUint16LE();
		unit.sprite = stream->readUint16LE();
		unit.x = stream->readSint16LE();
		unit.y = stream->readSint16LE();
		unit.anchorX = stream->readSint16LE();
		unit.anchorY = stream->readSint16LE();
		unit.flags = stream->readByte();
		if (!unit.initialList) {
			if (unit.list)
				return false;
			continue;
		}
		if (unit.initialList < 0x16a2 || unit.initialList > 0x1916)
			return false;
		const uint initialCount = mapAnimationLength(_state.begin(), _state.size(), unit.initialList);
		const uint count = unit.list ? mapAnimationLength(_state.begin(), _state.size(), unit.list) : 0;
		if (!initialCount || (unit.list && (!count || unit.list < 0x16a2 || unit.list > 0x1916)) ||
			(!unit.list && (initialCount != 1 || unit.flags || unit.next)) ||
				unit.sprite > 255 || (unit.flags & ~3) || ((unit.flags & 1) &&
				(unit.next < unit.list || unit.next > unit.list + 3 * count || (unit.next - unit.list) % 3)))
			return false;
	}
	const uint count = stream->readByte();
	if (count > ARRAYSIZE(_mapUnits))
		return false;
	bool seen[67] = {};
	for (uint i = 0; i < count; i++) {
		const uint troop = stream->readByte();
		if (troop >= ARRAYSIZE(_mapUnits) || seen[troop] || !_mapUnits[troop].initialList)
			return false;
		seen[troop] = true;
		_mapUnitOrder.push_back(troop);
	}
	for (uint i = 0; i < ARRAYSIZE(_mapUnits); i++) {
		if (seen[i] != (_mapUnits[i].initialList != 0))
			return false;
	}
	_mapRingOrder = stream->readByte();
	if (_mapRingOrder > count)
		return false;
	if (stream->err() || stream->eos() || (_mapRingTroop >= 0) != (_mapRing.list != 0))
		return false;
	if (_mapRingTroop >= 0 && (_mapRingTroop != (int)_orderingTroop ||
			(!seen[_mapRingTroop] && !hasRemovedTroopContact(_mapRingTroop))))
		return false;
	_mapUnitsLongitude = _worldLongitude;
	_mapUnitsLatitude = _worldLatitude;
	_mapAnimationActive = true;
	// The registration order is already serialized. Reconstruct only runtime
	// identities and live fields; retain all saved dormant slot bytes.
	openSpriteSheet("ONMAP.HSQ", false);
	WRITE_LE_UINT16(_state.begin() + kSpriteActorCount, 0);
	for (uint i = 0; i <= _mapUnitOrder.size(); i++) {
		if (_mapRing.list && i == _mapRingOrder)
			appendMapActor(ARRAYSIZE(_mapUnits));
		if (i < _mapUnitOrder.size())
			appendMapActor(_mapUnitOrder[i]);
	}
	return true;
}

} // namespace Dune
