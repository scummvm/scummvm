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

#include "phoenixvr/arn.h"
#include "common/file.h"

namespace PhoenixVR {
ARN *ARN::create() {
	Common::ScopedPtr<ARN> arn(new ARN);

	uint idx = 1;
	while (true) {
		auto arnName = Common::String::format("BData%u.arn", idx++);
		Common::File file;
		if (!file.open(Common::Path{arnName})) {
			break;
		}
		Common::Array<byte> data(file.size());
		debug("arn: loading %s, %ld bytes", arnName.c_str(), (long)file.size());
		if (file.read(data.data(), data.size()) != data.size())
			error("arn short read");
		arn->_archives.push_back(Common::move(data));
	}
	debug("loaded %u archives", arn->_archives.size());

	Common::File file;
	if (!file.open("BDataHeader.vit"))
		return nullptr;

	Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);

	auto numEntries = file.readUint32LE();
	auto version = file.readUint32LE();
	debug("arn: found %u entries, version: %u", numEntries, version);
	uint prevArchive = 0;
	uint offset = 0;
	while (numEntries--) {
		auto name = file.readString(0, 32);
		Entry &entry = arn->_surfaces[name];
		auto &rect = entry.rect;
		rect.left = file.readSint32LE();
		rect.top = file.readSint32LE();
		rect.right = file.readSint32LE();
		rect.bottom = file.readSint32LE();
		auto archive = file.readUint32LE();
		auto size = file.readUint32LE();
		debug("arn: %u: %s, %s, %u bytes", archive, name.c_str(), entry.rect.toString().c_str(), size);
		if (archive != prevArchive) {
			offset = 0;
			prevArchive = archive;
		}
		unsigned bpp = format.bytesPerPixel;
		assert(size == bpp * entry.rect.width() * entry.rect.height());

		auto &data = arn->_archives[archive - 1];
		assert(offset + size <= data.size());
		entry.surface.init(rect.width(), rect.height(), rect.width() * bpp, data.data() + offset, format);
		file.skip(4); // unk, usually 1
		offset += size;
	}

	return arn.release();
}

const Graphics::Surface *ARN::get(const Common::String &fname) const {
	auto it = _surfaces.find(fname);
	return it != _surfaces.end() ? &it->_value.surface : nullptr;
}
} // namespace PhoenixVR
