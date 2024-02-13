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


		// TODO: Maybe we need to add the caller, fn0017_24FD proc
		void Func2686();
		void Func2792(byte registerIndex, byte value);
		void Func2792r(byte value, byte registerIndex) {
			Func2792(registerIndex, value);
		}
		// TODO: Maybe need to add the caller
		void Func24FD();
		// TODO: Consider adding the caller
		void Func2839();

		uint16 Func19BE(uint8 offset);

		void Func1A03();


		// 01D7:1AA7
		void OnTimer();

		// 	[2254h] and [2256h]
		uint32 _nextEventTimer;

	public:
	void Init();

		// TODO: Check where these contents live
		// Memory pointed to by [2250] global
		Common::MemoryReadStream *data = nullptr;
	};

} // namespace Macs2

#endif
