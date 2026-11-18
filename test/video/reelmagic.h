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

#include <cxxtest/TestSuite.h>

#include "common/array.h"
#include "common/memstream.h"
#include "graphics/surface.h"
#include "video/reelmagic.h"
#include "test/system/null_osystem.h"

#ifdef USE_MPEG2
#include "video/mpegps_decoder.h"
#endif

class ReelMagicTestSuite : public CxxTest::TestSuite {
public:
	void setUp() {
#if NULL_OSYSTEM_IS_AVAILABLE
		Common::install_null_g_system();
#endif
	}

	void tearDown() {
#if NULL_OSYSTEM_IS_AVAILABLE
		Common::uninstall_null_g_system();
#endif
	}

	void testFrameRateAndStreamPosition() {
		byte data[] = { 0xff, 0, 0, 1, 0xb3, 0x14, 0, 0xf0, 0x1d, 0, 0, 0, 0 };
		Common::MemoryReadStream stream(data, sizeof(data));
		stream.seek(1);
		TS_ASSERT(Video::ReelMagicStream::isMagical(stream));
		TS_ASSERT_EQUALS(stream.pos(), 1);
		uint32 num = 0, den = 0;
		TS_ASSERT(Video::ReelMagicStream::getFrameDuration(stream, num, den));
		TS_ASSERT_EQUALS(num, 1000U);
		TS_ASSERT_EQUALS(den, 30U);
		TS_ASSERT_EQUALS(stream.pos(), 1);

		// Standard code 8 is 60 fps, not a ReelMagic marker with rate 0.
		data[8] = 0x18;
		TS_ASSERT(!Video::ReelMagicStream::isMagical(stream));
		TS_ASSERT(Video::ReelMagicStream::getFrameDuration(stream, num, den));
		TS_ASSERT_EQUALS(num, 1000U);
		TS_ASSERT_EQUALS(den, 60U);
		TS_ASSERT_EQUALS(stream.pos(), 1);
	}

	void testShortAndEmptyStreams() {
		const byte data[] = { 0, 0, 1 };
		Common::MemoryReadStream stream(data, sizeof(data));
		TS_ASSERT(!Video::ReelMagicStream::isProgramStream(stream));
		TS_ASSERT_EQUALS(stream.pos(), 0);
		TS_ASSERT(!Video::ReelMagicStream::isMagical(stream));
		stream.seek(sizeof(data));
		TS_ASSERT_EQUALS(Video::ReelMagicStream::unlock(stream, 0x40044041, true), nullptr);
	}

	void testBothMagicKeysAndPictureFields() {
		for (int key = 0; key < 2; key++) {
			byte data[] = {
				0, 0, 1, 0xb3, 0x14, 0, 0xf0, 0x1d, 0, 0, 0, 0,
				0, 0, 1, 0, 0, 0x10, 0, 0xd1, 0x59,
				0, 0, 1, 0, 0, 0x18, 0, 0xd1, 0x59
			};
			if (key) {
				data[19] = data[28] = 0xd2;
				data[20] = data[29] = 0xf1;
			}
			Common::MemoryReadStream stream(data, sizeof(data));
			Common::SeekableReadStream *decoded = Video::ReelMagicStream::unlock(stream,
				key ? 0xc39d7088 : 0x40044041, true);
			TS_ASSERT(decoded != nullptr);
			if (!decoded)
				continue;
			byte result[sizeof(data)];
			TS_ASSERT_EQUALS(decoded->read(result, sizeof(result)), sizeof(result));
			TS_ASSERT_EQUALS(result[7], 0x15);
			TS_ASSERT_EQUALS(result[19], 0xd0);
			TS_ASSERT_EQUALS(result[20], key ? 0xf1 : 0xd9);
			TS_ASSERT_EQUALS(result[28], 0xd0);
			TS_ASSERT_EQUALS(result[29], 0xd1);
			delete decoded;
		}
	}

	void testPictureHeaderAcrossPackets() {
		const byte elementary[] = {
			0, 0, 1, 0xb3, 0x14, 0, 0xf0, 0x1d, 0, 0, 0, 0,
			0, 0, 1, 0, 0, 0x10, 0, 0xd1, 0x59
		};
		Common::Array<byte> data = programStream();
		const uint first = appendPacket(data, 0xe0, elementary, 20);
		const uint second = appendPacket(data, 0xe0, elementary + 20, 1);
		Common::MemoryReadStream input(&data[0], data.size());
		Common::SeekableReadStream *output = Video::ReelMagicStream::unlock(input, 0x40044041, true);
		TS_ASSERT(output);
		if (!output)
			return;
		output->seek(first + 7);
		TS_ASSERT_EQUALS(output->readByte(), 0x15);
		output->seek(first + 19);
		TS_ASSERT_EQUALS(output->readByte(), 0xd0);
		output->seek(second);
		TS_ASSERT_EQUALS(output->readByte(), 0xd9);
		delete output;
	}

	void testTemporalSequenceWrap() {
		byte elementary[] = {
			0, 0, 1, 0xb3, 0x14, 0, 0xf0, 0x1d, 0, 0, 0, 0,
			0, 0, 1, 0, 0, 0x10, 0, 0xd1, 0x59,
			0, 0, 1, 0, 14, 0x10, 0, 0xd1, 0x59
		};
		Common::MemoryReadStream input(elementary, sizeof(elementary));
		Common::SeekableReadStream *output = Video::ReelMagicStream::unlock(input, 0x40044041, true);
		TS_ASSERT(output);
		if (!output)
			return;
		byte result[sizeof(elementary)];
		output->read(result, sizeof(result));
		TS_ASSERT_EQUALS(result[19], 0xd0);
		TS_ASSERT_EQUALS(result[20], 0xd9);
		TS_ASSERT_EQUALS(result[28], result[19]);
		TS_ASSERT_EQUALS(result[29], result[20]);
		delete output;
	}

	void testAudioPaddingAcrossPackets() {
		for (uint padded = 0; padded < 2; ++padded) {
			Common::Array<byte> audio;
			audio.resize(834 + padded);
			memset(&audio[0], 0, audio.size());
			audio[0] = audio[417 + padded] = 0xff;
			audio[1] = audio[418 + padded] = 0xfd;
			audio[2] = 0x82; // Correct only when the first frame has 418 bytes.
			audio[419 + padded] = 0x80;
			audio[3] = audio[420 + padded] = 0xc0;
			Common::Array<byte> data = programStream();
			const uint first = appendPacket(data, 0xc0, &audio[0], 300);
			appendPacket(data, 0xc0, &audio[300], audio.size() - 300);
			Common::MemoryReadStream input(&data[0], data.size());
			Common::SeekableReadStream *output = Video::ReelMagicStream::unlock(input, 0x40044041, false);
			TS_ASSERT(output);
			if (!output)
				continue;
			if (!padded)
				data[first + 2] = 0x80;
			Common::Array<byte> actual;
			actual.resize(data.size());
			TS_ASSERT_EQUALS(output->read(&actual[0], actual.size()), actual.size());
			TS_ASSERT_EQUALS(memcmp(&actual[0], &data[0], data.size()), 0);
			delete output;
		}
	}

	void testQueuedPicturesAndEOF() {
#if defined(USE_MPEG2) && NULL_OSYSTEM_IS_AVAILABLE
		// Six 32x32 black I/P/B pictures, generated with ffmpeg mpeg1video,
		// GOP 6, two B pictures. No game data is included.
		static const byte elementary[] = {
			0x00, 0x00, 0x01, 0xb3, 0x02, 0x00, 0x20, 0x15, 0xff, 0xff, 0xe0, 0x18,
			0x00, 0x00, 0x01, 0xb8, 0x00, 0x08, 0x00, 0x40, 0x00, 0x00, 0x01, 0x00,
			0x00, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x01, 0x01, 0x13, 0xf8, 0x7d, 0x29,
			0x48, 0x8b, 0x94, 0xa5, 0x22, 0x20, 0x00, 0x00, 0x01, 0x02, 0x13, 0xf8,
			0x7d, 0x29, 0x48, 0x8b, 0x94, 0xa5, 0x22, 0x20, 0x00, 0x00, 0x01, 0x00,
			0x00, 0xd7, 0xff, 0xf8, 0x80, 0x00, 0x00, 0x01, 0x01, 0x12, 0x79, 0xc0,
			0x00, 0x00, 0x01, 0x02, 0x12, 0x79, 0xc0, 0x00, 0x00, 0x01, 0x00, 0x00,
			0x5f, 0xff, 0xf8, 0x88, 0x00, 0x00, 0x01, 0x01, 0x1a, 0x5c, 0xb0, 0x00,
			0x00, 0x01, 0x02, 0x1a, 0x5c, 0xb0, 0x00, 0x00, 0x01, 0x00, 0x00, 0x9f,
			0xff, 0xf8, 0x88, 0x00, 0x00, 0x01, 0x01, 0x1a, 0xba, 0xc0, 0x00, 0x00,
			0x01, 0x02, 0x1a, 0xba, 0xc0, 0x00, 0x00, 0x01, 0x00, 0x01, 0x57, 0xff,
			0xf8, 0x80, 0x00, 0x00, 0x01, 0x01, 0x12, 0x79, 0xc0, 0x00, 0x00, 0x01,
			0x02, 0x12, 0x79, 0xc0, 0x00, 0x00, 0x01, 0x00, 0x01, 0x1f, 0xff, 0xf8,
			0x88, 0x00, 0x00, 0x01, 0x01, 0x1a, 0xba, 0xc0, 0x00, 0x00, 0x01, 0x02,
			0x1a, 0xba, 0xc0,
		};
		for (uint endCode = 0; endCode < 2; ++endCode) {
			Common::Array<byte> data;
			appendBytes(data, elementary, sizeof(elementary));
			if (endCode) {
				const byte end[] = { 0, 0, 1, 0xb7 };
				appendBytes(data, end, sizeof(end));
			}
			Video::MPEGPSDecoder decoder;
			decoder.setStopAtFirstFrame(true);
			TS_ASSERT(decoder.loadStream(new Common::MemoryReadStream(&data[0], data.size())));
			uint frames = 0;
			uint calls = 0;
			while (!decoder.endOfVideoTracks() && calls++ < 20) {
				if (decoder.decodeNextFrame())
					++frames;
			}
			TS_ASSERT_EQUALS(frames, 6U);
			TS_ASSERT(decoder.endOfVideoTracks());
			TS_ASSERT_EQUALS(decoder.decodeNextFrame(), nullptr);
			TS_ASSERT_EQUALS(decoder.decodeNextFrame(), nullptr);
		}
#endif
	}

	void testIntegerGraphicsKeyingAndClipping() {
		for (int bytes = 2; bytes <= 4; bytes += 2) {
			const Graphics::PixelFormat format = bytes == 2
				? Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)
				: Graphics::PixelFormat(4, 8, 8, 8, 0, 16, 8, 0, 0);
			Graphics::Surface output, video;
			output.create(6, 4, format);
			video.create(6, 4, format);
			output.fillRect(output.getRect(), 9);
			video.fillRect(video.getRect(), 7);
			const byte artwork[] = { 1, 0, 2, 1, 2, 0 };
			uint32 palette[256] = { 0, 3, 5 };
			const Common::Rect changed = Video::ReelMagicCompositor::blitGraphics(
				artwork, 3, palette, 0, output, &video, Common::Rect(3, 2), 2, 2);
			TS_ASSERT(changed == Common::Rect(6, 4));
			const uint expected[] = { 3, 7, 5, 3, 5, 7 };
			for (int y = 0; y < 4; y++) {
				for (int x = 0; x < 6; x++) {
					const uint color = bytes == 2 ? *(const uint16 *)output.getBasePtr(x, y)
						: *(const uint32 *)output.getBasePtr(x, y);
					TS_ASSERT_EQUALS(color, expected[(y / 2) * 3 + x / 2]);
				}
			}
			// A partially offscreen dirty rect must not write beyond the surface.
			const byte patch[] = { 1, 2 };
			TS_ASSERT(Video::ReelMagicCompositor::blitGraphics(patch, 2, palette, 0,
				output, nullptr, Common::Rect(-1, 0, 1, 1), 2, 2) == Common::Rect(0, 0, 2, 2));
			TS_ASSERT_EQUALS(bytes == 2 ? *(const uint16 *)output.getBasePtr(0, 0)
				: *(const uint32 *)output.getBasePtr(0, 0), 5U);
			output.free();
			video.free();
		}
	}

	void testVideoScalingKeepsEverySourceRow() {
		Graphics::Surface source, output;
		const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
		source.create(320, 240, format);
		output.create(640, 400, format);
		for (int y = 0; y < 240; y++)
			source.fillRect(Common::Rect(0, y, 320, y + 1), y + 1);
		Video::ReelMagicCompositor::scaleVideo(source, output);
		bool seen[240] = {};
		for (int y = 0; y < 400; y++) {
			const uint16 *row = (const uint16 *)output.getBasePtr(0, y);
			TS_ASSERT(row[0] >= 1 && row[0] <= 240);
			seen[row[0] - 1] = true;
			TS_ASSERT_EQUALS(row[639], row[0]);
		}
		for (int y = 0; y < 240; y++)
			TS_ASSERT(seen[y]);
		source.free();
		output.free();
	}

private:
	static void appendBytes(Common::Array<byte> &data, const byte *source, uint size) {
		const uint start = data.size();
		data.resize(start + size);
		memcpy(&data[start], source, size);
	}

	static Common::Array<byte> programStream() {
		const byte pack[] = { 0, 0, 1, 0xba, 0x21, 0, 1, 0, 1, 0, 1, 0 };
		Common::Array<byte> data;
		appendBytes(data, pack, sizeof(pack));
		return data;
	}

	static uint appendPacket(Common::Array<byte> &data, byte id, const byte *payload, uint size) {
		const byte header[] = { 0, 0, 1, id, (byte)((size + 1) >> 8), (byte)(size + 1), 0x0f };
		appendBytes(data, header, sizeof(header));
		const uint start = data.size();
		appendBytes(data, payload, size);
		return start;
	}
};
