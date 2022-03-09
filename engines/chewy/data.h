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

#ifndef CHEWY_DATA_H
#define CHEWY_DATA_H

#include "chewy/chewy.h"
#include "chewy/ngstypes.h"

namespace Chewy {

class Data {
public:
	Data();
	~Data();

	uint16 selectPoolItem(Common::Stream *stream, uint16 nr);
	uint32 load_tmf(Common::Stream *stream, TmfHeader *song);

	uint32 getPoolSize(const char *filename, int16 chunkStart, int16 chunkNr);
};

} // namespace Chewy

#endif
