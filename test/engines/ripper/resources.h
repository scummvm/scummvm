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

#include "common/endian.h"
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

	void testLibraryRejectsDuplicateNormalizedMemberNames() {
		const byte data[] = {
			0x02, 0x00, 0x08, 0x00, 0x00, 0x00,
			0xaa, 0xbb,
			0x06, 0x00, 0x00, 0x00, 'D', 'U', 'P', 0x00, 0x00, 0x00, 0x00, 0x00,
			0x07, 0x00, 0x00, 0x00, 'd', 'u', 'p', 0x00, 0x00, 0x00, 0x00, 0x00
		};
		Common::MemoryReadStream stream(data, sizeof(data), DisposeAfterUse::NO);
		Ripper::AssetLibrary library;

		TS_ASSERT(!library.open(stream, Common::Path("duplicate.pl")));
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

	void testPresentationFrameRegionTable() {
		byte data[0x68 + 12 + 18];
		memset(data, 0, sizeof(data));
		WRITE_LE_UINT32(data + 0x64, 2);
		data[0x68] = 7;
		data[0x69] = 1;
		data[0x6a] = 42;
		WRITE_LE_UINT16(data + 0x6c, 0);
		data[0x6e] = 8;
		data[0x6f] = 1;
		data[0x70] = 43;
		WRITE_LE_UINT16(data + 0x72, 1);
		data[0x74] = 0;
		WRITE_LE_UINT16(data + 0x75, 10);
		WRITE_LE_UINT16(data + 0x77, 20);
		WRITE_LE_UINT16(data + 0x79, 30);
		WRITE_LE_UINT16(data + 0x7b, 40);
		data[0x7d] = 1;
		WRITE_LE_UINT16(data + 0x7e, (uint16)-5);
		WRITE_LE_UINT16(data + 0x80, 25);
		WRITE_LE_UINT16(data + 0x82, 35);
		WRITE_LE_UINT16(data + 0x84, 45);
		Common::MemoryReadStream stream(data, sizeof(data), DisposeAfterUse::NO);
		Ripper::PresentationFrameRegionTable table;

		TS_ASSERT(Ripper::decodePresentationFrameRegionTable(stream, table));
		TS_ASSERT_EQUALS(table.frames.size(), 2U);
		TS_ASSERT_EQUALS(table.frames[0].state, 7);
		TS_ASSERT_EQUALS(table.frames[0].auxiliary, 42);
		TS_ASSERT_EQUALS(table.frames[0].regions.size(), 1U);
		TS_ASSERT_EQUALS(table.frames[0].regions[0].coordinate1, 10);
		TS_ASSERT_EQUALS(table.frames[0].regions[0].coordinate2, 20);
		TS_ASSERT_EQUALS(table.frames[0].regions[0].extent1, 30);
		TS_ASSERT_EQUALS(table.frames[0].regions[0].extent2, 40);
		TS_ASSERT_EQUALS(table.frames[1].regions[0].type, 1);
		TS_ASSERT_EQUALS(table.frames[1].regions[0].coordinate1, -5);
	}

	void testPresentationFrameAudioMap() {
		byte data[0x6c + 60 + 4];
		memset(data, 0, sizeof(data));
		WRITE_LE_UINT16(data + 0x64, 15);
		WRITE_LE_UINT16(data + 0x66, 2);
		WRITE_LE_UINT32(data + 0x68, 1);
		const char path[] = "r\\combat\\KD0.WAV";
		memcpy(data + 0x6c, path, sizeof(path));
		data[0x6c + 60] = 0;
		data[0x6c + 61] = 75;
		data[0x6c + 62] = 0xff;
		data[0x6c + 63] = 0;
		Common::MemoryReadStream stream(data, sizeof(data), DisposeAfterUse::NO);
		Ripper::PresentationFrameAudioMap map;

		TS_ASSERT(Ripper::decodePresentationFrameAudioMap(stream, 2, map));
		TS_ASSERT_EQUALS(map.frameRate, 15U);
		TS_ASSERT_EQUALS(map.sounds.size(), 1U);
		TS_ASSERT_EQUALS(map.sounds[0], "r\\combat\\KD0.WAV");
		TS_ASSERT_EQUALS(map.cues.size(), 2U);
		TS_ASSERT_EQUALS(map.cues[0].soundIndex, 0);
		TS_ASSERT_EQUALS(map.cues[0].volume, 75);
		TS_ASSERT_EQUALS(map.cues[1].soundIndex, -1);

		WRITE_LE_UINT16(data + 0x66, 3);
		Common::MemoryReadStream mismatchedStream(
			data, sizeof(data), DisposeAfterUse::NO);
		TS_ASSERT(!Ripper::decodePresentationFrameAudioMap(
			mismatchedStream, 2, map));
	}
};
