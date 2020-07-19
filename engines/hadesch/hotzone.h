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
#ifndef HADESCH_HOTZONE_H
#define HADESCH_HOTZONE_H

#include "common/ptr.h"
#include "common/str.h"
#include "common/rect.h"

#include "hadesch/tag_file.h"

namespace Hadesch {

class HotZone {
public:
	HotZone(const Common::Array<Common::Point> &polygon,
		const Common::String &hotid, bool enabled,
		int icsh);
	bool isInside(const Common::Point &pnt) const;
	const Common::String &getID() const;
	void load(const TagFile &file, int idx);
	void setEnabled(bool enabled);
	bool isEnabled() const;
	void setOffset(Common::Point offset);
	int getICSH() const;
private:
	Common::String _hotid;
	Common::Array<Common::Point> _polygon;
	Common::Point _offset;
	bool _enabled;
	int _icsh;
};

class HotZoneArray {
public:
	HotZoneArray();
	HotZoneArray(Common::SharedPtr<Common::SeekableReadStream> hzFile, bool enable);

	void setHotzoneEnabled(const Common::String &name, bool enabled);
	void readHotzones(Common::SharedPtr<Common::SeekableReadStream> hzFile,
			  bool enable, Common::Point offset = Common::Point(0, 0));
	int pointToIndex(Common::Point point);
	Common::String pointToName(Common::Point point);
	void setHotZoneOffset(const Common::String &name, Common::Point offset);
	Common::String indexToName(int idx);
	int indexToCursor(int idx, int frame);
	int indexToICSH(int idx);
private:
	Common::Array<HotZone> _hotZones;
};

}
#endif
