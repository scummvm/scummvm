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

#ifndef MADE_MAGICAL_MPEG_H
#define MADE_MAGICAL_MPEG_H

#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
}

namespace Made {

/**
 * Repairs the nonstandard MPEG-1 streams used by ReelMagic games.
 *
 * Their sequence headers use a reserved frame-rate code, and their P/B picture
 * headers contain scrambled f_code fields. Both are fixed-position bit fields,
 * so restoring them with the key provisioned by the game does not require
 * re-encoding any picture data. Some assets also have incorrect Layer II audio
 * padding bits, which are repaired in the same in-memory pass.
 */
class MagicalMpeg {
public:
	/** Default key of the card, and the one Return to Zork provisions. */
	static const uint32 kDefaultMagicKey = 0x40044041;

	/** Return whether the stream starts with an MPEG program-stream pack. The stream position is preserved. */
	static bool isProgramStream(Common::SeekableReadStream &stream);

	/** Return whether the sequence header contains ReelMagic's marker bit. The stream position is preserved. */
	static bool isMagical(Common::SeekableReadStream &stream);

	/**
	 * Read a stream into memory, restore its scrambled picture headers when
	 * @p magical is true, and correct malformed MPEG audio padding bits. Returns
	 * nullptr if the stream cannot be read. The caller owns the returned stream.
	 */
	static Common::SeekableReadStream *unlock(Common::SeekableReadStream &stream,
		uint32 magicKey, bool magical);

	/** Read the sequence-header frame duration as @p num / @p den ms. */
	static bool getFrameDuration(Common::SeekableReadStream &stream, uint32 &num, uint32 &den);

private:
	static uint32 unlockBuffer(byte *data, uint32 size, uint32 magicKey);
	static uint32 fixAudioPadding(byte *data, uint32 size);
};

} // End of namespace Made

#endif // MADE_MAGICAL_MPEG_H
