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

/**
 * Unit tests for MDSDisc -- also serve as usage examples for the API.
 *
 * All tests use synthetic in-memory MDS data built from byte arrays,
 * parsed via the stream-based MDSDisc::open(SeekableReadStream*) overload.
 * No real .mds/.mdf file is required.
 *
 * Key patterns demonstrated:
 *   - Opening an MDS from a memory stream
 *   - Querying track metadata: getTrack(), getTrackCount()
 *   - Handling sentinel values for missing tracks
 *   - Close/reopen lifecycle
 */

#include <cxxtest/TestSuite.h>
#include "backends/audiocd/mds/mds-disc.h"
#include "common/memstream.h"
#include "common/str.h"

// Helpers
static void mdsWriteLE16(byte *buf, int pos, uint16 v) {
	buf[pos+0] = static_cast<byte>(v     );
	buf[pos+1] = static_cast<byte>(v >> 8);
}

static void mdsWriteLE32(byte *buf, int pos, uint32 v) {
	buf[pos+0] = static_cast<byte>(v      );
	buf[pos+1] = static_cast<byte>(v >>  8);
	buf[pos+2] = static_cast<byte>(v >> 16);
	buf[pos+3] = static_cast<byte>(v >> 24);
}

static void mdsWriteLE64(byte *buf, int pos, uint64 v) {
	for (int i = 0; i < 8; i++) {
		buf[pos+i] = static_cast<byte>(v >> (8 * i));
	}
}

// MDS track-mode byte values (from MDSRawMode enum in mds-disc.cpp):
//   0xA9 = Audio        -> MDSDisc::kModeAudio
//   0xAA = Mode 1 data  -> MDSDisc::kModeMode1
// Builders below use these raw bytes because the enum is file-scoped.

// Offsets mirrored from mds-disc.cpp (must stay in sync)
enum {
	kMDSOfsSignature   = 0x00,
	kMDSOfsNumSessions = 0x14,
	kMDSOfsSessionOfs  = 0x50, // session blocks offset (NOT disc structures at 0x34)
	kMDSHeaderSize     = 0x58,

	kSessionNumBlocks   = 0x0A,
	kSessionNumLeadIn   = 0x0B,
	kSessionFirstTrack  = 0x0C,
	kSessionLastTrack   = 0x0E,
	kSessionTrackOfs    = 0x14,
	kSessionBlockSize   = 0x18,

	kTrackMode        = 0x00,
	kTrackNumber      = 0x03,
	kTrackPoint       = 0x04,
	kTrackExtraOfs    = 0x0C, // uint32 LE: offset to extra data block
	kTrackSectorSize  = 0x10, // uint16 LE
	kTrackStartLBA    = 0x24, // uint32 LE
	kTrackMDFOffset   = 0x28, // uint64 LE
	kTrackBlockSize   = 0x50,

	kExtraPregap      = 0x00,
	kExtraSectorCount = 0x04,
	kExtraBlockSize   = 0x08
};

static const char kMDSSig[] = "MEDIA DESCRIPTOR";  // 16 bytes, no NUL in file

// MDS buffer builders
/**
 * Build a minimal valid MDS buffer with one data track and one audio track.
 *
 * @param outSize  Receives buffer size.
 * @return         Heap-allocated buffer (caller must delete[]).
 */
static byte *buildMDS(uint32 *outSize) {
	// Lay out: header + 1 session block + 2 track blocks + 2 extra blocks
	uint32 sessionOfs = kMDSHeaderSize;
	uint32 trackOfs   = sessionOfs + kSessionBlockSize;
	uint32 extraOfs   = trackOfs + 2 * kTrackBlockSize;
	uint32 bufSize    = extraOfs + 2 * kExtraBlockSize;

	byte *buf = new byte[bufSize];
	memset(buf, 0, bufSize);

	// Header
	memcpy(buf + kMDSOfsSignature, kMDSSig, 16);
	buf[0x10] = 1; buf[0x11] = 0;   // version 1.0
	mdsWriteLE16(buf, kMDSOfsNumSessions, 1);
	mdsWriteLE32(buf, kMDSOfsSessionOfs, sessionOfs);

	// Session block: 2 track blocks, 0 lead-in, tracks 1-2
	byte *sess = buf + sessionOfs;
	sess[kSessionNumBlocks]  = 2;  // total track block entries
	sess[kSessionNumLeadIn]  = 0;  // no lead-in TOC entries in this test
	mdsWriteLE16(sess, kSessionFirstTrack, 1);
	mdsWriteLE16(sess, kSessionLastTrack, 2);
	mdsWriteLE32(sess, kSessionTrackOfs, trackOfs);

	// Track 1: Mode 1 cooked data (2048 bytes/sector)
	byte *tr1 = buf + trackOfs;
	tr1[kTrackMode]   = 0xAA; // Mode 1 cooked
	tr1[kTrackPoint]  = 1;
	mdsWriteLE16(tr1, kTrackSectorSize, 2048);
	mdsWriteLE64(tr1, kTrackMDFOffset, 0);
	mdsWriteLE32(tr1, kTrackStartLBA, 0);
	mdsWriteLE32(tr1, kTrackExtraOfs, extraOfs);

	// Track 1 extra block: pregap=0, length=250
	mdsWriteLE32(buf, extraOfs + kExtraPregap, 0);
	mdsWriteLE32(buf, extraOfs + kExtraSectorCount, 250);

	// Track 2: Audio (2352 bytes/sector)
	byte *tr2 = buf + trackOfs + kTrackBlockSize;
	tr2[kTrackMode]   = 0xA9; // Audio
	tr2[kTrackPoint]  = 2;
	mdsWriteLE16(tr2, kTrackSectorSize, 2352);
	mdsWriteLE64(tr2, kTrackMDFOffset, static_cast<uint64>(250) * 2048); // starts after data
	mdsWriteLE32(tr2, kTrackStartLBA, 250);
	mdsWriteLE32(tr2, kTrackExtraOfs, extraOfs + kExtraBlockSize);

	// Track 2 extra block: pregap=0, length=100
	mdsWriteLE32(buf, extraOfs + kExtraBlockSize + kExtraPregap, 0);
	mdsWriteLE32(buf, extraOfs + kExtraBlockSize + kExtraSectorCount, 100);

	*outSize = bufSize;
	return buf;
}

/**
 * Build an MDS buffer with 3 lead-in entries + 2 real tracks.
 *
 * Lead-in entries have points 0xA0, 0xA1, 0xA2 and should be skipped
 * by the parser.
 */
static byte *buildMDSWithLeadIn(uint32 *outSize) {
	uint32 sessionOfs = kMDSHeaderSize;
	uint32 trackOfs   = sessionOfs + kSessionBlockSize;
	uint32 extraOfs   = trackOfs + 5 * kTrackBlockSize;
	uint32 bufSize    = extraOfs + 2 * kExtraBlockSize;

	byte *buf = new byte[bufSize];
	memset(buf, 0, bufSize);

	// Header
	memcpy(buf + kMDSOfsSignature, kMDSSig, 16);
	buf[0x10] = 1; buf[0x11] = 0;
	mdsWriteLE16(buf, kMDSOfsNumSessions, 1);
	mdsWriteLE32(buf, kMDSOfsSessionOfs, sessionOfs);

	// Session: 5 blocks total (3 lead-in + 2 real), tracks 1-2
	byte *sess = buf + sessionOfs;
	sess[kSessionNumBlocks]  = 5;
	sess[kSessionNumLeadIn]  = 3;
	mdsWriteLE16(sess, kSessionFirstTrack, 1);
	mdsWriteLE16(sess, kSessionLastTrack, 2);
	mdsWriteLE32(sess, kSessionTrackOfs, trackOfs);

	// Lead-in entries (points 0xA0, 0xA1, 0xA2)
	for (int i = 0; i < 3; i++) {
		byte *tr = buf + trackOfs + i * kTrackBlockSize;
		tr[kTrackMode]  = 0xAA;
		tr[kTrackPoint] = static_cast<byte>(0xA0 + i);
		mdsWriteLE16(tr, kTrackSectorSize, 2048);
	}

	// Real track 1: Mode 1
	byte *tr1 = buf + trackOfs + 3 * kTrackBlockSize;
	tr1[kTrackMode]  = 0xAA;
	tr1[kTrackPoint] = 1;
	mdsWriteLE16(tr1, kTrackSectorSize, 2048);
	mdsWriteLE64(tr1, kTrackMDFOffset, 0);
	mdsWriteLE32(tr1, kTrackStartLBA, 0);
	mdsWriteLE32(tr1, kTrackExtraOfs, extraOfs);
	mdsWriteLE32(buf, extraOfs + kExtraPregap, 0);
	mdsWriteLE32(buf, extraOfs + kExtraSectorCount, 250);

	// Real track 2: Audio
	byte *tr2 = buf + trackOfs + 4 * kTrackBlockSize;
	tr2[kTrackMode]  = 0xA9;
	tr2[kTrackPoint] = 2;
	mdsWriteLE16(tr2, kTrackSectorSize, 2352);
	mdsWriteLE64(tr2, kTrackMDFOffset, static_cast<uint64>(250) * 2048);
	mdsWriteLE32(tr2, kTrackStartLBA, 250);
	mdsWriteLE32(tr2, kTrackExtraOfs, extraOfs + kExtraBlockSize);
	mdsWriteLE32(buf, extraOfs + kExtraBlockSize + kExtraPregap, 0);
	mdsWriteLE32(buf, extraOfs + kExtraBlockSize + kExtraSectorCount, 100);

	*outSize = bufSize;
	return buf;
}

/**
 * Build an MDS with 3 tracks using the given sector sizes.
 * Used to test that tracks with invalid sector sizes are rejected.
 */
static byte *buildMDSThreeTracks(uint32 *outSize, uint16 size1, uint16 size2, uint16 size3) {
	uint32 sessionOfs = kMDSHeaderSize;
	uint32 trackOfs   = sessionOfs + kSessionBlockSize;
	uint32 extraOfs   = trackOfs + 3 * kTrackBlockSize;
	uint32 bufSize    = extraOfs + 3 * kExtraBlockSize;

	byte *buf = new byte[bufSize];
	memset(buf, 0, bufSize);

	memcpy(buf + kMDSOfsSignature, kMDSSig, 16);
	buf[0x10] = 1; buf[0x11] = 0;
	mdsWriteLE16(buf, kMDSOfsNumSessions, 1);
	mdsWriteLE32(buf, kMDSOfsSessionOfs, sessionOfs);

	byte *sess = buf + sessionOfs;
	sess[kSessionNumBlocks]  = 3;
	sess[kSessionNumLeadIn]  = 0;
	mdsWriteLE16(sess, kSessionFirstTrack, 1);
	mdsWriteLE16(sess, kSessionLastTrack, 3);
	mdsWriteLE32(sess, kSessionTrackOfs, trackOfs);

	uint16 sizes[] = {size1, size2, size3};
	for (int i = 0; i < 3; i++) {
		byte *tr = buf + trackOfs + i * kTrackBlockSize;
		tr[kTrackMode]  = 0xAA;
		tr[kTrackPoint] = static_cast<byte>(i + 1);
		mdsWriteLE16(tr, kTrackSectorSize, sizes[i]);
		mdsWriteLE64(tr, kTrackMDFOffset, 0);
		mdsWriteLE32(tr, kTrackStartLBA, 0);
		mdsWriteLE32(tr, kTrackExtraOfs, extraOfs + i * kExtraBlockSize);
		mdsWriteLE32(buf, extraOfs + i * kExtraBlockSize + kExtraPregap, 0);
		mdsWriteLE32(buf, extraOfs + i * kExtraBlockSize + kExtraSectorCount, 100);
	}

	*outSize = bufSize;
	return buf;
}

// Test suite
class MDSTestSuite : public CxxTest::TestSuite {
public:

	MDSDisc openStandardDisc() {
		uint32 size;
		byte *buf = buildMDS(&size);
		Common::MemoryReadStream stream(buf, size);
		MDSDisc disc;
		disc.open(&stream);
		delete[] buf;  // safe: open() copies the stream data internally
		return disc;
	}

	void testIsDiscImageFile() {
		TS_ASSERT(isDiscImageFile(Common::String("game.iso")));
		TS_ASSERT(isDiscImageFile(Common::String("game.mds")));
		TS_ASSERT(isDiscImageFile(Common::String("game.mdf")));
		TS_ASSERT(isDiscImageFile(Common::String("GAME.ISO")));
		TS_ASSERT(isDiscImageFile(Common::String("GAME.MDS")));
		TS_ASSERT(!isDiscImageFile(Common::String("game.bin")));
		TS_ASSERT(!isDiscImageFile(Common::String("game.cue")));
		TS_ASSERT(!isDiscImageFile(Common::String("")));
	}

	void testOpenAndTrackCount() {
		MDSDisc disc = openStandardDisc();
		TS_ASSERT(disc.isOpen());
		TS_ASSERT_EQUALS(disc.getTrackCount(), 2);
	}

	void testDataTrackMode() {
		MDSDisc disc = openStandardDisc();

		MDSDisc::TrackInfo t = disc.getTrack(1);
		TS_ASSERT_EQUALS(t.mode, MDSDisc::kModeMode1);
		TS_ASSERT_EQUALS(t.sectorSize, static_cast<uint16>(2048));
	}

	void testAudioTrackMode() {
		MDSDisc disc = openStandardDisc();

		MDSDisc::TrackInfo t = disc.getTrack(2);
		TS_ASSERT_EQUALS(t.mode, MDSDisc::kModeAudio);
		TS_ASSERT_EQUALS(t.sectorSize, static_cast<uint16>(2352));
	}

	void testLeadInEntriesSkipped() {
		uint32 size;
		byte *buf = buildMDSWithLeadIn(&size);
		Common::MemoryReadStream stream(buf, size);

		MDSDisc disc;
		TS_ASSERT(disc.open(&stream));
		// 5 track blocks (3 lead-in + 2 real) -> only 2 real tracks
		TS_ASSERT_EQUALS(disc.getTrackCount(), 2);
		delete[] buf;
	}

	void testInvalidSectorSizeSkipped() {
		uint32 size;
		// Track sector sizes: 0, 9999, 2048
		// sectorSize==0 and sectorSize>2448 are rejected by the parser
		byte *buf = buildMDSThreeTracks(&size, 0, 9999, 2048);
		Common::MemoryReadStream stream(buf, size);

		MDSDisc disc;
		TS_ASSERT(disc.open(&stream));
		TS_ASSERT_EQUALS(disc.getTrackCount(), 1);
		delete[] buf;
	}

	void testTrackByteOffset() {
		MDSDisc disc = openStandardDisc();

		MDSDisc::TrackInfo t = disc.getTrack(2);
		TS_ASSERT_EQUALS(t.mdfOffset, static_cast<uint64>(250) * 2048);
	}

	void testInvalidSignatureFailsOpen() {
		uint32 size;
		byte *buf = buildMDS(&size);
		buf[0] = 'X'; // corrupt signature
		Common::MemoryReadStream stream(buf, size);

		MDSDisc disc;
		TS_ASSERT(!disc.open(&stream));
		TS_ASSERT(!disc.isOpen());
		delete[] buf;
	}

	void testGetTrackNonexistentReturnsSentinel() {
		MDSDisc disc = openStandardDisc();
		MDSDisc::TrackInfo t = disc.getTrack(99);
		TS_ASSERT_EQUALS(t.number, -1);
		TS_ASSERT_EQUALS(t.mode, MDSDisc::kModeUnknown);
	}

	void testCloseAndReopen() {
		uint32 size;
		byte *buf = buildMDS(&size);

		MDSDisc disc;
		Common::MemoryReadStream stream1(buf, size);
		TS_ASSERT(disc.open(&stream1));
		TS_ASSERT_EQUALS(disc.getTrackCount(), 2);

		disc.close();
		TS_ASSERT(!disc.isOpen());
		TS_ASSERT_EQUALS(disc.getTrackCount(), 0);

		Common::MemoryReadStream stream2(buf, size);
		TS_ASSERT(disc.open(&stream2));
		TS_ASSERT_EQUALS(disc.getTrackCount(), 2);

		delete[] buf;
	}

	void testTrackPregapAndSectorCount() {
		MDSDisc disc = openStandardDisc();

		MDSDisc::TrackInfo t1 = disc.getTrack(1);
		TS_ASSERT_EQUALS(t1.sectorCount, static_cast<uint32>(250));
		TS_ASSERT_EQUALS(t1.pregapSectors, static_cast<uint32>(0));

		MDSDisc::TrackInfo t2 = disc.getTrack(2);
		TS_ASSERT_EQUALS(t2.sectorCount, static_cast<uint32>(100));
		TS_ASSERT_EQUALS(t2.pregapSectors, static_cast<uint32>(0));
	}
};
