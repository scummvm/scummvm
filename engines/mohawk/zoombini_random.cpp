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

#include "mohawk/resource.h"

#include "common/config-manager.h"
#include "common/random.h"
#include "common/system.h"
#include "gui/EventRecorder.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_metaengine.h"
#include "mohawk/zoombini_random.h"

namespace Mohawk {

ZoombiniRandom::ZoombiniRandom(const Common::String &name) : _scummRnd(name) {
	_useOriginal = ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionOriginalPRNG);

#ifdef ENABLE_EVENTRECORDER
	assert(g_system);
	setSeed(g_eventRec.getRandomSeed(name));
#else
	setSeed(generateNewSeed());
#endif
}

void ZoombiniRandom::setSeed(uint32 seed) {
	if (seed == 0)
		seed += 1;
	_randSeed = seed;
}

uint32 ZoombiniRandom::generateNewSeed() {
	return Common::RandomSource::generateNewSeed();
}

uint16 ZoombiniRandom::getOriginalRandomNumber(uint32 max) {
	// A zero-width range is still a wrapper call, but it consumes no LCG step.
	if (max == 0)
		return 0;

	_randSeed = 214013u * _randSeed + 2531011u;
	return static_cast<uint16>((_randSeed >> 16) % (max + 1));
}

int16 ZoombiniRandom::getRandomNumber(int16 max) {
	if (max < 0) {
		warning("ZoombiniRandom::getRandomNumber: max(%d) is smaller than min(0), swapping", max);
		return getRandomNumber(max, 0);
	}

	if (_useOriginal)
		return static_cast<int16>(getOriginalRandomNumber(static_cast<uint32>(max)));

	return static_cast<int16>(_scummRnd.getRandomNumber(max));
}

int16 ZoombiniRandom::getRandomNumber(int16 min, int16 max) {
	if (max < min) {
		warning("ZoombiniRandom::getRandomNumber: max(%d) is smaller than min(%d), swapping", max, min);
		int16 tmp = max;
		max = min;
		min = tmp;
	}

	uint32 span = static_cast<uint32>(static_cast<int32>(max) - static_cast<int32>(min));
	uint16 offset;
	if (_useOriginal)
		offset = getOriginalRandomNumber(span);
	else
		offset = static_cast<uint16>(_scummRnd.getRandomNumber(span));

	return static_cast<int16>(static_cast<int32>(min) + offset);
}

bool ZoombiniRandom::getRandomBool() {
	return getRandomNumber(1) != 0;
}

uint16 ZoombiniRandom::getNonRepeatRandom(uint16 poolSize, uint32 &bitmask) {
	assert(poolSize <= 32);
	uint32 fullMask = (poolSize < 32) ? ((1u << poolSize) - 1u) : 0xFFFFFFFFu;
	if ((bitmask & fullMask) == fullMask)
		bitmask = 0;

	uint16 poolIdx = getRandomNumber(poolSize - 1);
	while (bitmask & (1u << poolIdx))
		poolIdx = (poolIdx + 1) % poolSize;
	bitmask |= (1u << poolIdx);
	return poolIdx;
}

} // End of namespace Mohawk
