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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "common/memstream.h"
#include "common/ptr.h"
#include "ripper/resources.h"

#include <cxxtest/TestSuite.h>

class RipperResourceTestSuite : public CxxTest::TestSuite {
public:
	void testLegacyLibraryDirectoryAndMemberStreams() {
		const byte data[] = {
			0x02, 0x00, 0x0a, 0x00, 0x00, 0x00,
			0xaa, 0xbb, 0xcc, 0xdd,
			0x06, 0x00, 0x00, 0x00, 'F', 'I', 'R', 'S', 'T', 0x00, 0x00, 0x00,
			0x08, 0x00, 0x00, 0x00, 'S', 'E', 'C', 'O', 'N', 'D', 0x00, 0x00
		};
		Common::MemoryReadStream stream(data, sizeof(data), DisposeAfterUse::NO);
		Ripper::AssetLibrary library;

		TS_ASSERT(library.open(stream, Common::Path("test.pl")));
		TS_ASSERT_EQUALS(library.getEntryCount(), 2U);
		TS_ASSERT(!library.isModernFormat());
		TS_ASSERT(library.hasMember("C:\\RIPPER\\FIRST.BMP"));
		TS_ASSERT(library.hasMember("second"));

		Common::ScopedPtr<Common::SeekableReadStream> member(
			library.createReadStreamForMember("first"));
		TS_ASSERT(member);
		TS_ASSERT_EQUALS(member->size(), 2);
		TS_ASSERT_EQUALS(member->readByte(), 0xaa);
		TS_ASSERT_EQUALS(member->readByte(), 0xbb);
	}

	void testModernLibraryPreservesExtensionInLookup() {
		const byte data[] = {
			0x01, 0x00, 0x0d, 0x00, 0x00, 0x00, '2', 'B', 'I', 'L',
			0x10, 0x20, 0x30,
			0x0a, 0x00, 0x00, 0x00, 'P', 'I', 'C', 'T', 'U', 'R', 'E', 0x00,
			'B', 'M', 'P', 0x00, 0x78, 0x56, 0x34, 0x12
		};
		Common::MemoryReadStream stream(data, sizeof(data), DisposeAfterUse::NO);
		Ripper::AssetLibrary library;

		TS_ASSERT(library.open(stream, Common::Path("modern.pl")));
		TS_ASSERT(library.isModernFormat());
		TS_ASSERT(library.hasMember("picture.bmp"));
		TS_ASSERT(!library.hasMember("picture"));

		Common::ScopedPtr<Common::SeekableReadStream> member(
			library.createReadStreamForMember("PICTURE.BMP"));
		TS_ASSERT(member);
		TS_ASSERT_EQUALS(member->size(), 3);
		TS_ASSERT_EQUALS(member->readByte(), 0x10);
	}

	void testCustomBitmapDescriptorAndRawTail() {
		const byte data[] = {
			0x00, 0x01, 0x00, 0x00, 0x01,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x02, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00,
			0x00, 0x00, 0x01
		};
		Common::MemoryReadStream stream(data, sizeof(data), DisposeAfterUse::NO);
		Ripper::BitmapAssetFrame frame;

		TS_ASSERT(Ripper::decodeBitmapAsset(stream, frame));
		TS_ASSERT_EQUALS(frame.width, 2);
		TS_ASSERT_EQUALS(frame.height, 1);
		TS_ASSERT_EQUALS(frame.transparentColor, 0);
		TS_ASSERT_EQUALS(frame.pixels.size(), 2U);
		TS_ASSERT_EQUALS(frame.pixels[0], 0);
		TS_ASSERT_EQUALS(frame.pixels[1], 1);
		TS_ASSERT(frame.palette.empty());
	}
};
