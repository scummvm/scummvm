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

#ifndef AWE_AIFC_PLAYER_H
#define AWE_AIFC_PLAYER_H

#include "awe/intern.h"
#include "awe/file.h"

namespace Awe {

struct AifcPlayer {

	File _f;
	uint32_t _ssndOffset;
	uint32_t _ssndSize;
	uint32_t _pos;
	int16_t _sampleL, _sampleR;
	Frac _rate;

	AifcPlayer();

	bool play(int mixRate, const char *path, uint32_t offset);
	void stop();

	int8_t readSampleData();
	void decodeSamples();
	void readSamples(int16_t *buf, int len);
};

} // namespace Awe

#endif
