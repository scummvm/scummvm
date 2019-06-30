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

#ifndef BLADERUNNER_REGIONS_H
#define BLADERUNNER_REGIONS_H

#include "bladerunner/bladerunner.h"

#include "common/array.h"
#include "common/rect.h"

namespace BladeRunner {

class SaveFileReadStream;
class SaveFileWriteStream;

class Regions {
	friend class Debugger;

	struct Region {
		Common::Rect rectangle;
		int          type;     // Arrow Icon on mouse-over (has meaning only for Exits) 0: Upward , 1: Right, 2: Downward, 3: Left
		int          present;
	};

	Common::Array<Region> _regions;
	bool                  _enabled;

public:
	Regions();

	void clear();
	bool add(int index, Common::Rect rect, int type);
	bool remove(int index);

	int getTypeAtXY(int x, int y) const;
	int getRegionAtXY(int x, int y) const;

	void setEnabled(bool enabled);
	void enable();

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);
};

} // End of namespace BladeRunner

#endif
