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
#include "rom.h"

#include "common/file.h"
#include "common/util.h"

#include "scooby/common/make_unique.h"

namespace Scooby {

namespace {
const long kExpectedLength = 0x200000;
} // namespace

ScoobyDooRom::ScoobyDooRom(Common::Array<uint8> bytes)
	: _bytes(Common::move(bytes)) {
}

Common::ScopedPtr<ScoobyDooRom> ScoobyDooRom::load(const Common::Path &path, Common::String &outError) {
	Common::File fd;
	if (!fd.open(path)) {
		outError = "Could not open the original ROM";
		return nullptr;
	}
	long fileLength = fd.size();

	Common::Array<uint8> bytes;
	if (fileLength > 0) {
		bytes.resize(static_cast<size_t>(fileLength));
		size_t readCount = fd.read(bytes.data(), fileLength);
		if (readCount != bytes.size()) {
			fd.close();
			outError = "Could not open the original ROM: short read";
			return nullptr;
		}
	}
	fd.close();

	if (static_cast<long>(bytes.size()) != kExpectedLength) {
		outError = Common::String::format("Unsupported ROM length %u; the analyzed US image is %ld bytes.",
										  bytes.size(), kExpectedLength);
		return nullptr;
	}

	// ScoobyDooRom's constructor is private (this is its only caller), so it
	// is constructed directly here rather than through MakeUnique,
	// which as a free function has no access to a private constructor.
	return Common::ScopedPtr<ScoobyDooRom>(new ScoobyDooRom(Common::move(bytes)));
}

} // namespace Scooby
