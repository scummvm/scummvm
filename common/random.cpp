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
	TimeDate time;
	g_system->getTimeAndDate(time);
	uint32 newSeed = time.tm_sec + time.tm_min * 60 + time.tm_hour * 3600;
	newSeed += time.tm_mday * 86400 + time.tm_mon * 86400 * 31;
	newSeed += time.tm_year * 86400 * 366;
	newSeed = newSeed * 1000 + g_system->getMillis();
	setSeed(newSeed);
#endif
}

void RandomSource::setSeed(uint32 seed) {
	if (seed == 0)
		seed++;
	_randSeed = seed;
}

uint RandomSource::getRandomNumber(uint max) {
	scrambleSeed();
	if (max == UINT_MAX)
		return (_randSeed * 0xDEADBF03);
	return (_randSeed * 0xDEADBF03) % (max + 1);
}

uint RandomSource::getRandomBit() {
	scrambleSeed();
	return _randSeed & 1;
}

uint RandomSource::getRandomNumberRng(uint min, uint max) {
	return getRandomNumber(max - min) + min;
}

int RandomSource::getRandomNumberRngSigned(int min, int max) {
	return getRandomNumber(max - min) + min;
}
		
inline void RandomSource::scrambleSeed() {
	//marsaglia's paper says that any of 81 triplets are feasible
	//(11,21,13) was chosen, with (cba) and (>>,<<,>>)
	_randSeed ^= _randSeed >> 13;
	_randSeed ^= _randSeed << 21;
	_randSeed ^= _randSeed >> 11;
}

} // End of namespace Common
