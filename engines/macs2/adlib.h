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

#ifndef MACS2_ADLIB_H
#define MACS2_ADLIB_H

#include "common/scummsys.h"
#include "common/array.h"

namespace Common {
class MemoryReadStream;
}

namespace OPL {
	class OPL;
}

namespace Macs2 {

	class Adlib {

	private:
		OPL::OPL *_opl;

		void Func1A03();

		// 01D7:1AA7
		void OnTimer();

		uint16 Func19BE(uint8 offset);

		// TODO: Maybe need to add the caller
		void Func24FD();

		// TODO: Maybe we need to add the caller, fn0017_24FD proc
		void Func2686();

		void Func2792(byte registerIndex, byte value);
		void Func2792r(byte value, byte registerIndex) {
			Func2792(registerIndex, value);
		}
		
		// TODO: Consider adding the caller
		void Func2839();

		// 	[2254h] and [2256h]
		uint32 _nextEventTimer;

		// [2258h] - TODO: Not sure about size - Initialization
		uint8 g2258;

		// [223Eh] - Seems to be a 16 bit value - Initial value?
		uint16 g223E;

		// [2291h] - Not sure how used - initialization?
		uint16 g2291;

		// [2296h] - TODO: Initialization
		uint16 g2296;

		// [2298h] - TODO: Initialization
		uint16 g2298;

		// [225Ah] - TODO: Initial value?
		uint16 g225A;

		// Array at [222Ch] - accessed with byte values
		// TODO: Initialization, access pattern
		Common::Array<uint8> gArray222C;

		// Array at [225Fh] - accessed with byte values
		// TODO: Initialization, access pattern
		Common::Array<uint8> gArray225F;

		// Array at [227Fh] - accessed with byte values
		// TODO: Initialization, access pattern, size
		Common::Array<uint8> gArray227F;

		// Array at [2288h] - accessed with byte values
		// TODO: Initialization, access pattern, size
		Common::Array<uint8> gArray2288;

		// Array at [2235h] - accessed with bytes values
		// TODO: Initialization, access pattern, size
		Common::Array<uint8> gArray2235;

		bool g229A;

		// [229Bh] - seems to contain song byte - TODO: Initial value?
		uint8 g229B;

		// Reads a byte without advancing the read stream
		uint8 peekByte();

		// Reads a byte from the specified offset without changing the position in the stream
		uint8 peekByteAt(uint16 offset);
	

	public:
	void Init();

	void Deinit();

		// TODO: Check where these contents live
		// Memory pointed to by [2250] global
		Common::MemoryReadStream *data = nullptr;
	};

} // namespace Macs2

#endif
