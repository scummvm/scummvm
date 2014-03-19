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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

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
	_facing = 0;
	_descId = 0;
	_field14 = 0;
	_articleNumber = 0;
	_cursor = CURSOR_NONE;
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

int DynamicHotspots::add(int descId, int field14, int seqIndex, const Common::Rect &bounds) {
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
	_entries[idx]._feetPos.x = -3;
	_entries[idx]._feetPos.y = 0;
	_entries[idx]._facing = 5;
	_entries[idx]._field14 = field14;
	_entries[idx]._articleNumber = 6;
	_entries[idx]._cursor = CURSOR_NONE;

	++_count;
	_changed = true;

	if (seqIndex >= 0)
		_vm->_game->_scene._sequences[seqIndex]._dynamicHotspotIndex = idx;

	return idx;
}

int DynamicHotspots::setPosition(int index, int xp, int yp, int facing) {
	if (index >= 0) {
		_entries[index]._feetPos.x = xp;
		_entries[index]._feetPos.y = yp;
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

	if (_entries[index]._active) {
		if (_entries[index]._seqIndex >= 0)
			scene._sequences[_entries[index]._seqIndex]._dynamicHotspotIndex = -1;
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
	for (uint i = 0; i < _entries.size(); ++i) {
		DynamicHotspot &dh = (*this)[i];

		if ((*this)[i]._active) {
			switch (_vm->_game->_screenObjects._v832EC) {
			case 0:
			case 2:
				_vm->_game->_screenObjects.add(dh._bounds, _vm->_game->_scene._layer,
					CAT_12, dh._descId);
				_vm->_game->_screenObjects._v8333C = true;
				break;
			default:
				break;
			}
		}
	}
}

/*------------------------------------------------------------------------*/

void Hotspots::activate(int hotspotId, bool active) {
	warning("TODO: Hotspots::activate");
}

} // End of namespace MADS
