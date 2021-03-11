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

#ifndef AGDS_PATCH_H
#define AGDS_PATCH_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Common	{ class ReadStream; class WriteStream; }

namespace AGDS {

class AGDSEngine;
class Object;

struct ObjectPatch {
	static constexpr unsigned Size = 66;

	Common::String text;
	Common::String region;
	int z;
	void load(Common::ReadStream *stream);
	void save(Common::WriteStream *stream) const;
};

struct Patch {
	struct Object {
		Common::String 	name;
		int				flag;

		Object(const Common::String &n, int f): name(n), flag(f) {}
	};

	Common::String screenRegionName;
	Common::String prevScreenName;

	uint hasPreviousScreen = 0;
	Common::Point characterPosition;
	uint characterDirection = 0;
	bool characterPresent = false;

	byte palette[0x300] = {};
	Common::String defaultMouseCursor;
	Common::Array<Object> objects;

	void load(Common::ReadStream *stream);
	void save(Common::WriteStream *stream);
	int getFlag(const Common::String & name) const;
	int incRef(const Common::String & name);
	int decRef(const Common::String & name);
};


} // End of namespace AGDS

#endif /* AGDS_PATCH_H */
