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

#ifndef EEM_ANIMATION_H
#define EEM_ANIMATION_H

#include "common/array.h"
#include "common/file.h"
#include "common/path.h"
#include "common/scummsys.h"

namespace EEM {

/**
 * Decoder for BOLT.ANM, TITLE.ANM, ANIM01.A..ANIM20.A. Format from
 * Load_Sequence @ 2503:0006 / OpenDifferenceAnimation @ 2520:0337:
 *   - 0x300 bytes: 6-bit VGA palette
 *   - u16: frame count
 *   - 12 bytes: header (height @ +2, width @ +4, rest unused)
 *   - frames*u16: packed length per frame
 *   - per frame: lengths[i] bytes of RLE delta data (decodeAnmFrameRLE).
 */
class ANMDecoder {
public:
	ANMDecoder() = default;
	~ANMDecoder() { close(); }

	/// Open @p path and parse the file header. Returns false on error.
	bool open(const Common::Path &path);

	/// Release the open file and the persistent frame buffer.
	void close();

	/// Number of frames in the animation.
	uint16 frameCount() const { return _frameCount; }
	uint16 width()      const { return _width; }
	uint16 height()     const { return _height; }

	/// 768 bytes of 6-bit VGA palette (already shifted into 8-bit on getPalette8).
	const byte *palette6() const { return _palette; }

	/// Fill @p out (768 bytes) with the 8-bit-shifted palette. Convenience.
	void getPalette8(byte *out) const;

	/// Seed the persistent frame buffer from the current 8-bit screen pixels.
	void seedFrameBuffer(const byte *pixels, uint pitch);

	/**
	 * Decode the next frame in place into the internal buffer. Returns a
	 * pointer to the @c width()*@c height() byte image, or nullptr at EOF.
	 */
	const byte *nextFrame();

private:
	Common::File _file;
	Common::Array<uint16> _lengths;
	Common::Array<byte> _buffer;   ///< Persistent unpacked frame, width*height bytes.
	Common::Array<byte> _packed;   ///< Per-frame scratch packed buffer.

	byte _palette[768] = {};
	uint16 _frameCount = 0;
	uint16 _width = 0;
	uint16 _height = 0;
	uint16 _nextFrameIdx = 0;
};

/// Decode a single ANM frame's RLE-encoded delta payload into @p dst.
/// `dst` already holds the previous frame; skip opcodes leave those pixels
/// untouched (difference encoding). Original symbol: `_ASM_Decompress`
/// @ 1000:0953.
void decodeAnmFrameRLE(const byte *src, uint srcSize, byte *dst, uint dstSize);

} // End of namespace EEM

#endif
