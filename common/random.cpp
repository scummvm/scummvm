/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/random.h"
#include "common/system.h"
#include "gui/EventRecorder.h"


namespace Common {

RandomSource::RandomSource(const String &name) {
	// Use system time as RNG seed. Normally not a good idea, if you are using
	// a RNG for security purposes, but good enough for our purposes.
	assert(g_system);

#ifdef ENABLE_EVENTRECORDER
	setSeed(g_eventRec.getRandomSeed(name));
#else
	setSeed(g_system->getMillis());
#endif
}

void RandomSource::setSeed(uint32 seed) {
	_randSeed = seed;
}

uint RandomSource::getRandomNumber(uint max) {
	_randSeed = 0xDEADBF03 * (_randSeed + 1);
	_randSeed = (_randSeed >> 13) | (_randSeed << 19);
	return _randSeed % (max + 1);
}

uint RandomSource::getRandomBit() {
	_randSeed = 0xDEADBF03 * (_randSeed + 1);
	_randSeed = (_randSeed >> 13) | (_randSeed << 19);
	return _randSeed & 1;
}

uint RandomSource::getRandomNumberRng(uint min, uint max) {
	return getRandomNumber(max - min) + min;
}

int RandomSource::getRandomNumberRngSigned(int min, int max) {
	return getRandomNumber(max - min) + min;
}

} // End of namespace Common
