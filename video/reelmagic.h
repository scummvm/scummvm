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

#ifndef VIDEO_REELMAGIC_H
#define VIDEO_REELMAGIC_H

#include "common/scummsys.h"
#include "common/rect.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Video {

/**
 * Repairs the nonstandard MPEG-1 streams used by ReelMagic games.
 *
 * Their sequence headers use a reserved frame-rate code, and their P/B picture
 * headers contain scrambled f_code fields. Both are fixed-position bit fields,
 * so restoring them with the key provisioned by the game does not require
 * re-encoding any picture data. Some assets also have incorrect Layer II audio
 * padding bits, which are repaired in the same in-memory pass.
 *
 * The format and f_code recovery algorithm were documented by Jon Dennis
 * (jrdennisoss) for DOSBox ReelMagic. This implementation builds on that work;
 * see doc/reelmagic.txt. Playback and script behavior belong to the consumer.
 */
class ReelMagicStream {
public:
	/**
	 * The card's default key. Callers pass the key chosen by their title;
	 * The Horde, for example, uses 0xC39D7088 instead.
	 */
	static const uint32 kDefaultMagicKey = 0x40044041;

	/** Return whether the stream starts with an MPEG program-stream pack. The stream position is preserved. */
	static bool isProgramStream(Common::SeekableReadStream &stream);

	/**
	 * Probe the first 16 KB for the sequence header's ReelMagic marker.
	 * The stream position is preserved. The probe scans raw bytes, so a header
	 * split across PES packets is not recognized.
	 */
	static bool isMagical(Common::SeekableReadStream &stream);

	/**
	 * Read a stream into memory, restore its scrambled picture headers when
	 * @p magical is true, and correct malformed MPEG audio padding bits. Returns
	 * nullptr if the stream cannot be read. The caller owns the returned stream.
	 */
	static Common::SeekableReadStream *unlock(Common::SeekableReadStream &stream,
		uint32 magicKey, bool magical);

	/** Read the frame duration as @p num / @p den ms, using the same probe as isMagical(). */
	static bool getFrameDuration(Common::SeekableReadStream &stream, uint32 &num, uint32 &den);

private:
	static uint32 unlockBuffer(byte *data, uint32 size, uint32 magicKey);
	static uint32 fixAudioPadding(byte *data, uint32 size);
};

/** Composites paletted VGA graphics over a decoded ReelMagic picture. */
class ReelMagicCompositor {
public:
	/** Scale the complete decoded picture to a surface in the same format. */
	static void scaleVideo(const Graphics::Surface &source, Graphics::Surface &destination);

	/**
	 * Replicate graphics pixels by integer factors, keying the transparent
	 * index to the video. Source points to the top-left of the graphics rect;
	 * palette contains 256 colors in the destination format. Video, when set,
	 * must match the destination's dimensions and format. Returns the changed
	 * output rect, clipped to the destination.
	 */
	static Common::Rect blitGraphics(const byte *source, int pitch, const uint32 *palette,
		byte transparentIndex, Graphics::Surface &destination, const Graphics::Surface *video,
		const Common::Rect &rect, int scaleX, int scaleY);
};

} // End of namespace Video

#endif // VIDEO_REELMAGIC_H
