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

#ifndef GRIM_MOVIE_SMUSH_WAVE_H
#define GRIM_MOVIE_SMUSH_WAVE_H

#include "common/scummsys.h"

namespace Grim {

#define SMUSH_WAVE_PACKET_SIZE 500000

class SmushSound;

class SmushWave {

private:
	SmushSound *_sound;
	byte *_packetBuffer;
	int _trackId;
	uint16 _chunkCounter;
	uint32 _totalSize;
	uint16 _vimaTable[5786];
	bool _vimaTableBuilt;

public:
	SmushWave(SmushSound *sound);
	~SmushWave();

	int moduleStartup();
	int moduleShutdown();

	void flobdecReset();
	void flobdecDecodeHeader(const byte *header);
	void flobdecDecodeBody(const byte *body);

};

} // End of namespace Grim
#endif
