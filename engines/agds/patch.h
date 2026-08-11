/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PATCH_H
#define PATCH_H

#include "agds/screenLoadingType.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Common {
class ReadStream;
class WriteStream;
} // namespace Common

namespace AGDS {

class AGDSEngine;
class Object;

struct ObjectPatch {
	static constexpr unsigned Size = 66;

	Common::String text;
	Common::String region;
	int z;
	void load(Common::ReadStream &stream);
	void save(Common::WriteStream &stream) const;
};

struct Patch {
	struct Object {
		Common::String name;
		int16 flag;

		Object(const Common::String &n, int f) : name(n), flag(f) {}
	};

	bool screenSaved = false;
	Common::String screenRegionName;
	Common::String prevScreenName;

	ScreenLoadingType loadingType = ScreenLoadingType::Normal;
	Common::Point characterPosition;
	int characterDirection = 0;
	bool characterPresent = false;

	byte palette[0x300] = {};
	Common::String defaultMouseCursor;
	Common::Array<Object> objects;

	void load(Common::ReadStream &stream);
	void save(Common::WriteStream &stream);
	void setFlag(const Common::String &name, int flag);
	int getFlag(const Common::String &name) const;
	int incRef(const Common::String &name);
	int decRef(const Common::String &name);
};

} // End of namespace AGDS

#endif /* AGDS_PATCH_H */
