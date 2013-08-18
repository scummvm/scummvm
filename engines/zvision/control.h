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

#ifndef ZVISION_CONTROL_H
#define ZVISION_CONTROL_H

#include "common/types.h"

namespace Common {
class SeekableReadStream;
}

namespace ZVision {

class ZVision;

class Control {
public:
	Control() : _key(0), _enabled(false) {}
	virtual ~Control() {}
	virtual bool enable(ZVision *engine) = 0;
	virtual bool disable(ZVision *engine) { return true; }

public:
	uint32 _key;

protected:
	bool _enabled;

// Static member functions
public:
	static void parseFlatControl(ZVision *engine);
	static void parsePanoramaControl(ZVision *engine, Common::SeekableReadStream &stream);
	static void parseTiltControl(ZVision *engine, Common::SeekableReadStream &stream);
	static void parsePushToggleControl(uint32 key, ZVision *engine, Common::SeekableReadStream &stream);
};

} // End of namespace ZVision

#endif
