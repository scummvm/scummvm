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

#include "common/endian.h"
#include "common/memstream.h"
#include "image/codecs/tm2a.h"
#include "test/system/null_osystem.h"

class TM2ADecoderTestSuite : public CxxTest::TestSuite {
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

	void test_prediction_sampling_and_pixel_formats() {
#ifdef USE_TM2A
		Common::Array<byte> packet = makeFrame();
		Image::TM2ADecoder decoder(16, 16);
		const Graphics::PixelFormat formats[] = {
			Graphics::PixelFormat::createFormatRGBA32(),
			Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)
		};
		for (uint format = 0; format < ARRAYSIZE(formats); ++format) {
			TS_ASSERT(decoder.setOutputPixelFormat(formats[format]));
			Common::MemoryReadStream stream(packet.begin(), packet.size());
			const Graphics::Surface *surface = decoder.decodeFrame(stream);
			TS_ASSERT(surface);
			if (!surface)
				return;
			TS_ASSERT_EQUALS(surface->w, 16);
			TS_ASSERT_EQUALS(surface->h, 16);
			for (uint y = 0; y < 16; ++y) {
				for (uint x = 0; x < 16; ++x) {
					uint luma;
					if (x >= 8)
						luma = y < 8 ? 132 : 140;
					else if (y < 8)
						luma = (x & 1) ? 128 : 129 + y;
					else
						luma = 136 + (y - 7) * (x / 2 + 1);
					byte gray = (luma - 16) * 255 / 219;
					uint32 expected = surface->format.RGBToColor(gray, gray, gray);
					const void *pixel = surface->getBasePtr(x, y);
					uint32 actual = format ? READ_UINT16(pixel) : READ_UINT32(pixel);
					TS_ASSERT_EQUALS(actual, expected);
				}
			}
		}
		TS_ASSERT(!decoder.setOutputPixelFormat(Graphics::PixelFormat::createFormatCLUT8()));
#endif
	}

	void test_truncated_frame_and_recovery() {
#ifdef USE_TM2A
		Common::Array<byte> packet = makeFrame();
		Image::TM2ADecoder decoder(16, 16);
		for (uint size = 0; size < packet.size(); ++size) {
			Common::MemoryReadStream stream(packet.begin(), size);
			TS_ASSERT(!decoder.decodeFrame(stream));
		}
		Common::MemoryReadStream stream(packet.begin(), packet.size());
		TS_ASSERT(decoder.decodeFrame(stream));
#endif
	}

	void test_reject_invalid_indices_and_unsupported_frames() {
#ifdef USE_TM2A
		// Undefined code, undefined mode, displaced prediction, inter frame,
		// incompatible dimensions, invalid table, oversized vector, invalid delta.
		const uint offsets[] = {12, 12, 14, 76, 67, 103, 177, 202};
		const byte values[] = {255, 3, 1, 3, 24, 2, 9, 127};
		Image::TM2ADecoder decoder(16, 16);
		for (uint i = 0; i < ARRAYSIZE(offsets); ++i) {
			Common::Array<byte> packet = makeFrame();
			packet[offsets[i]] = values[i];
			Common::MemoryReadStream stream(packet.begin(), packet.size());
			TS_ASSERT(!decoder.decodeFrame(stream));
		}
#endif
	}

private:
	static Common::Array<byte> makeFrame() {
		// Synthetic 16x16 frame: full resolution, 8x8 replication and 2x1 luma.
		// Includes negative deltas and codebook vectors of up to eight tokens.
		static const byte data[] = {
			0xa0, 0x00, 0x01, 0x06, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x2e,
			0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x07, 0x07, 0x00, 0x00, 0x03, 0x07,
			0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
			0x07, 0x02, 0x01, 0x00, 0x00, 0x01, 0x00, 0x06, 0x06, 0x00, 0x00, 0x03,
			0x05, 0x06, 0x06, 0x05, 0x06, 0x06, 0x05, 0x06, 0x06, 0x05, 0xa0, 0x00,
			0x01, 0x15, 0x00, 0x00, 0x00, 0x12, 0x00, 0x10, 0x01, 0x62, 0x01, 0x19,
			0x01, 0x8c, 0x00, 0x00, 0x00, 0x72, 0x00, 0x00, 0x01, 0x72, 0x00, 0x00,
			0xa0, 0x00, 0x01, 0x09, 0x00, 0x00, 0x00, 0x03, 0x08, 0x02, 0x00, 0xa0,
			0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x04, 0x01, 0x72, 0x00,
			0x01, 0xfe, 0x8d, 0x00, 0x04, 0xa0, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00,
			0x04, 0x01, 0x01, 0x01, 0x72, 0xa0, 0x00, 0x01, 0x0b, 0x00, 0x00, 0x00,
			0x04, 0x01, 0x00, 0x00, 0x72, 0xa0, 0x00, 0x01, 0x0b, 0x00, 0x00, 0x00,
			0x04, 0x0c, 0x00, 0x00, 0x73, 0xa0, 0x00, 0x01, 0x0b, 0x00, 0x00, 0x00,
			0x04, 0x10, 0x00, 0x00, 0x70, 0xa0, 0x00, 0x01, 0x0a, 0x00, 0x00, 0x00,
			0x28, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x01, 0x00, 0x01,
			0x01, 0x01, 0x02, 0x01, 0x03, 0x04, 0x00, 0x00, 0x01, 0x02, 0x04, 0x01,
			0x01, 0x01, 0x01, 0x04, 0x00, 0x00, 0x01, 0x01, 0x08, 0x00, 0x00, 0x01,
			0x02, 0x00, 0x00, 0x01, 0x02,
		};
		return Common::Array<byte>(data, sizeof(data));
	}
};
