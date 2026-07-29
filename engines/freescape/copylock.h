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

#ifndef FREESCAPE_COPYLOCK_H
#define FREESCAPE_COPYLOCK_H

#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
}

namespace Freescape {

/**
 * Removal of the Rob Northen Copylock wrapper found on several Amiga and
 * Atari ST releases.
 *
 * The protection reads a serial from a specially formatted track of the
 * original disk and decrypts the wrapped program with it. The ciphertext is
 * however produced once, when the release is mastered, so the key is a
 * constant of that release: every copy carries the same encrypted program and
 * the key disk only lets the protection *check* the serial at run time. That
 * is why the wrapper can be removed here without the disk, and without
 * executing any 68000 code.
 *
 * Everything but the key is read from the file: the protection is located by
 * its signature, its own code is decoded (the Trace Vector Decoder used for
 * the loops is static), and the decoding routine found in there tells which
 * cipher wraps the program, and where it is.
 *
 * Both the GEMDOS programs of the Atari ST releases and the hunk executables
 * of the Amiga ones are handled.
 */
class Copylock {
public:
	/**
	 * Unwrap a Copylock protected program.
	 *
	 * Returns the wrapped program on success, or nullptr when the file is not
	 * protected (it can then be used as is) or when the release is unknown.
	 * The caller owns the returned stream; the input stream is left untouched.
	 */
	static Common::SeekableReadStream *unwrap(Common::SeekableReadStream *file);

	/** Whether the file carries a wrapper this class recognizes. */
	static bool isProtected(Common::SeekableReadStream *file);
};

} // End of namespace Freescape

#endif // FREESCAPE_COPYLOCK_H
