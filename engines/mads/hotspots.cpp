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
 */

#include "mads/mads.h"
#include "mads/hotspots.h"

namespace MADS {

DynamicHotspot::DynamicHotspot() {
	_seqIndex = 0;
	_animIndex = -1;
	_facing = FACING_NONE;
	_descId = 0;
	_verbId = 0;
	_valid = false;	// V2
	_articleNumber = 0;
	_syntax = 0;	// V2
	_cursor = CURSOR_NONE;
	_active = false;
}

void DynamicHotspot::synchronize(Common::Serializer &s) {

}

/*------------------------------------------------------------------------*/

DynamicHotspots::DynamicHotspots(MADSEngine *vm) : _vm(vm) {
	for (int i = 0; i < DYNAMIC_HOTSPOTS_SIZE; ++i) {
		DynamicHotspot rec;
		rec._active = false;
		_entries.push_back(rec);
	}

	_changed = true;
	_count = 0;
}

int DynamicHotspots::add(int descId, int verbId, byte syntax, int seqIndex, const Common::Rect &bounds) {
	// Find a free slot
	uint idx = 0;
	while ((idx < _entries.size()) && _entries[idx]._active)
		++idx;
	if (idx == _entries.size())
		error("DynamicHotspots overflow");

	_entries[idx]._active = true;
	_entries[idx]._descId = descId;
	_entries[idx]._seqIndex = seqIndex;
	_entries[idx]._bounds = bounds;
	_entries[idx]._feetPos = Common::Point(-3, 0);
	_entries[idx]._facing = FACING_NONE;
	_entries[idx]._verbId = verbId;
	_entries[idx]._articleNumber = PREP_IN;
	_entries[idx]._syntax = syntax;
	_entries[idx]._cursor = CURSOR_NONE;
	_entries[idx]._valid = true;
	_entries[idx]._animIndex = -1;

	++_count;
	_changed = true;

	if (seqIndex >= 0) {
		_vm->_game->_scene._sequences[seqIndex]._dynamicHotspotIndex = idx;
		_entries[idx]._valid = false;
	}

	return idx;
}

int DynamicHotspots::add(int descId, int verbId, int seqIndex, const Common::Rect &bounds) {
	// Find a free slot
	uint idx = 0;
	while ((idx < _entries.size()) && _entries[idx]._active)
		++idx;
	if (idx == _entries.size())
		error("DynamicHotspots overflow");

	_entries[idx]._active = true;
	_entries[idx]._descId = descId;
	_entries[idx]._seqIndex = seqIndex;
	_entries[idx]._bounds = bounds;
	_entries[idx]._feetPos = Common::Point(-3, 0);
	_entries[idx]._facing = FACING_NONE;
	_entries[idx]._verbId = verbId;
	_entries[idx]._articleNumber = PREP_IN;
	_entries[idx]._cursor = CURSOR_NONE;
	_entries[idx]._animIndex = -1;

	++_count;
	_changed = true;

	if (seqIndex >= 0)
		_vm->_game->_scene._sequences[seqIndex]._dynamicHotspotIndex = idx;

	return idx;
}

int DynamicHotspots::setPosition(int index, const Common::Point &pos, Facing facing) {
	if (index >= 0) {
		_entries[index]._feetPos = pos;
		_entries[index]._facing = facing;
	}

	return index;
}

int DynamicHotspots::setCursor(int index, CursorType cursor) {
	if (index >= 0)
		_entries[index]._cursor = cursor;

	return index;
}

void DynamicHotspots::remove(int index) {
	Scene &scene = _vm->_game->_scene;

	if (index >= 0 && _entries[index]._active) {
		if (_entries[index]._seqIndex >= 0)
			scene._sequences[_entries[index]._seqIndex]._dynamicHotspotIndex = -1;
		if (_entries[index]._animIndex >= 0)
			scene._animation[_entries[index]._animIndex]->_dynamicHotspotIndex = -1;
		_entries[index]._active = false;

		--_count;
		_changed = true;
	}
}

void DynamicHotspots::clear() {
	for (uint i = 0; i < _entries.size(); ++i)
		_entries[i]._active = false;

	_changed = false;
	_count = 0;
}

void DynamicHotspots::reset() {
	for (uint i = 0; i < _entries.size(); ++i)
		remove(i);

	_count = 0;
	_changed = false;
}

void DynamicHotspots::refresh() {
	// Reset the screen objects back to only contain UI elements
	ScreenObjects &scrObjects = _vm->_game->_screenObjects;
	scrObjects.resize(scrObjects._uiCount);

	 bool isV2 = (_vm->getGameID() != GType_RexNebular);

	// Loop through adding hotspots
	for (uint i = 0; i < _entries.size(); ++i) {
		DynamicHotspot &dh = (*this)[i];

		if ((*this)[i]._active && (!isV2 || (*this)[i]._valid)) {
			switch (scrObjects._inputMode) {
			case kInputBuildingSentences:
			case kInputLimitedSentences:
				scrObjects.add(dh._bounds, _vm->_game->_scene._mode, CAT_12, dh._descId);
				scrObjects._forceRescan = true;
				break;
			default:
				break;
			}
		}
	}

	// Reset the list's changed flag
	_changed = false;
}

DynamicHotspot &DynamicHotspots::get(int index) {
	for (uint i = 0; i < _entries.size(); ++i) {
		if (_entries[i]._active && index-- == 0)
			return _entries[i];
	}

	error("Could not find dynamic hotspot");
}

void DynamicHotspots::synchronize(Common::Serializer &s) {
	int count = _entries.size();
	s.syncAsSint16LE(count);

	// The MIN in the below loop is a workaround to fix earlier savegame
	// loading accidentally adding new dynamic hotspots to the fixed list
	for (int i = 0; i < count; ++i) {
		_entries[MIN(i, (int)_entries.size() - 1)].synchronize(s);
	}
}

/*------------------------------------------------------------------------*/

Hotspot::Hotspot() {
	_facing = FACING_NONE;
	_articleNumber = 0;
	_cursor = CURSOR_NONE;
	_vocabId = 0;
	_verbId = 0;
	_active = false;
}

Hotspot::Hotspot(Common::SeekableReadStream &f, bool isV2) {
	_bounds.left = f.readSint16LE();
	_bounds.top = f.readSint16LE();
	_bounds.right = f.readSint16LE();
	_bounds.bottom = f.readSint16LE();
	_feetPos.x = f.readSint16LE();
	_feetPos.y = f.readSint16LE();
	_facing = (Facing)f.readByte();
	_articleNumber = f.readByte();
	_active = f.readByte() != 0;
	_cursor = (CursorType)f.readByte();
	if (isV2) {
		f.skip(1);		// cursor
		_syntax = f.readByte();
	}
	_vocabId = f.readUint16LE();
	_verbId = f.readUint16LE();
}

/*------------------------------------------------------------------------*/

void Hotspots::activate(int vocabId, bool active) {
	for (uint idx = 0; idx < size(); ++idx) {
		Hotspot &hotspot = (*this)[idx];
		if (hotspot._vocabId == vocabId) {
			hotspot._active = active;
			_vm->_game->_screenObjects.setActive(CAT_HOTSPOT, idx, active);
		}
	}
}

void Hotspots::activateAtPos(int vocabId, bool active, Common::Point pos) {
	for (uint idx = 0; idx < size(); ++idx) {
		Hotspot &hotspot = (*this)[idx];
		if ((hotspot._vocabId == vocabId) && (pos.x >= hotspot._bounds.left) &&
		    (pos.x <= hotspot._bounds.right) && (pos.y >= hotspot._bounds.top) &&
		    (pos.y <= hotspot._bounds.bottom)) {
			hotspot._active = active;
			_vm->_game->_screenObjects.setActive(CAT_HOTSPOT, idx, active);
		}
	}
}

} // End of namespace MADS
