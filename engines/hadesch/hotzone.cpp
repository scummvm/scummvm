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

#include "hadesch/hotzone.h"

namespace Hadesch {
bool HotZone::isEnabled() const {
	return _enabled;
}

void HotZone::setEnabled(bool val) {
	_enabled = val;
}


void HotZone::setOffset(Common::Point offset) {
	_offset = offset;
}

// Following function copied from sword 2.5 and simplified
bool HotZone::isInside(const Common::Point &point_in) const {
	Common::Point point = point_in - _offset;
	int rcross = 0; // Number of right-side overlaps

	// Each edge is checked whether it cuts the outgoing stream from the point
	for (unsigned i = 0; i < _polygon.size(); i++) {
		const Common::Point &edgeStart = _polygon[i];
		const Common::Point &edgeEnd = _polygon[(i + 1) % _polygon.size()];

		// A vertex is a point? Then it lies on one edge of the polygon
		if (point == edgeStart)
			return true;

		if ((edgeStart.y > point.y) != (edgeEnd.y > point.y)) {
			int term1 = (edgeStart.x - point.x) * (edgeEnd.y - point.y) - (edgeEnd.x - point.x) * (edgeStart.y - point.y);
			int term2 = (edgeEnd.y - point.y) - (edgeStart.y - edgeEnd.y);
			if ((term1 > 0) == (term2 >= 0))
				rcross++;
		}
	}

	// The point is strictly inside the polygon if and only if the number of overlaps is odd
	return ((rcross % 2) == 1);
}

const Common::String &HotZone::getID() const {
	return _hotid;
}

int HotZone::getICSH() const {
	return _icsh;
}

HotZone::HotZone(const Common::Array<Common::Point> &polygon,
		 const Common::String &hotid, bool enabled,
		 int icsh) : _polygon(polygon), _hotid(hotid),
			     _enabled(enabled), _icsh(icsh) {
}

void HotZoneArray::readHotzones(Common::SharedPtr<Common::SeekableReadStream> hzFile, bool enable, Common::Point offset) {
	if (!hzFile) {
		debug("Invalid hzFile");
		return;
	}
	TagFile tf;
	tf.openStoreHot(hzFile);
	Common::ScopedPtr<Common::SeekableReadStream> tcshStream(tf.getFileStream(MKTAG('T', 'C', 'S', 'H')));
	int hzCnt = tcshStream->readUint32LE();
	
	for (int idx = 0; idx < hzCnt; idx++) {
		Common::SharedPtr<Common::SeekableReadStream> tdshFile(tf.getFileStream(MKTAG('T', 'D', 'S', 'H'), idx));
		TagFile tdsh;
		tdsh.openStoreHotSub(tdshFile);

		Common::SharedPtr<Common::SeekableReadStream> tvshFile(tdsh.getFileStream(MKTAG('T', 'V', 'S', 'H')));

		Common::Array<Common::Point> polygon;

		for (int j = 0; j < tvshFile->size() / 8; j++) {
			uint32 x = tvshFile->readUint32LE();
			uint32 y = tvshFile->readUint32LE();
			polygon.push_back(Common::Point(x, y) + offset);
		}

		Common::SharedPtr<Common::SeekableReadStream> mnshFile(tdsh.getFileStream(MKTAG('M', 'N', 'S', 'H')));

		int mnshSize = mnshFile->size();
		char *name = new (std::nothrow) char[mnshSize + 1];
		mnshFile->read(name, mnshSize);
		name[mnshSize] = 0;

		Common::SharedPtr<Common::SeekableReadStream> icshFile(tdsh.getFileStream(MKTAG('I', 'C', 'S', 'H')));

		int icsh = icshFile->readUint32LE();
		
		_hotZones.push_back(HotZone(polygon, name, enable, icsh));

		delete[] name;		
	}
}

HotZoneArray::HotZoneArray() {
}

HotZoneArray::HotZoneArray(Common::SharedPtr<Common::SeekableReadStream> hzFile, bool enable) {
	readHotzones(hzFile, enable);
}

void HotZoneArray::setHotzoneEnabled(Common::String const &name, bool val) {
	for (unsigned i = 0; i < _hotZones.size(); i++) {
		if (_hotZones[i].getID() == name)
			_hotZones[i].setEnabled(val);
	}
}

int HotZoneArray::pointToIndex(Common::Point point) {
	for (unsigned i = 0; i < _hotZones.size(); i++) {
		if (_hotZones[i].isEnabled() && _hotZones[i].isInside(point)) {
			return i;
		}
	}

	return -1;
}

void HotZoneArray::setHotZoneOffset(const Common::String &name, Common::Point offset) {
	for (unsigned i = 0; i < _hotZones.size(); i++) {
		if (_hotZones[i].getID() == name) {
			_hotZones[i].setOffset(offset);
		}
	}
}

Common::String HotZoneArray::pointToName(Common::Point point) {
	for (unsigned i = 0; i < _hotZones.size(); i++) {
		if (_hotZones[i].isEnabled() && _hotZones[i].isInside(point)) {
			return _hotZones[i].getID();
		}
	}
	return "";
}

Common::String HotZoneArray::indexToName(int idx) {
	if (idx >= 0 && idx < (int)_hotZones.size()) {
		return _hotZones[idx].getID();
	} else
		return "";
}

int HotZoneArray::indexToICSH(int idx) {
	if (idx < 0 || idx >= (int)_hotZones.size()) {
		return -1;
	}

	return _hotZones[idx].getICSH();
}

int HotZoneArray::indexToCursor(int idx, int frame) {
	if (idx < 0 || idx >= (int)_hotZones.size()) {
		return 0;
	}

	switch (_hotZones[idx].getICSH()) {
	default:
		return frame % 3;
	case 1:
		return 0;
	case 2:
		return 14; // left arrow
	case 3:
		return 16; // right arrow
	case 4:
		return 13; // up arrow
	case 5:
		return 15; // down arrow, never used, just a guess
	}
}

}
