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

#include "phoenixvr/bigf.h"
#include "common/debug.h"
#include "common/file.h"

namespace PhoenixVR {
BIGF::BIGF(const Common::String &path) : _path(path) {
	Common::File file;
	if (!file.open(Common::Path{path})) {
		error("can't open %s", path.c_str());
	}
	auto magic = file.readString(0, 0x50);
	if (magic != "BIGF\r\n") {
		error("invalid BIGF magic");
	}
	while (true) {
		auto name = file.readString();
		if (name.empty())
			break;
		debug("name %s", name.c_str());
		auto size = file.readUint32LE();
		auto offset = file.readUint32LE();
		debug("size: %08x, offset: %08x", size, offset);
	}
	_dataPos = file.pos();
	debug("data at %08zx", _dataPos);
}

} // namespace PhoenixVR
