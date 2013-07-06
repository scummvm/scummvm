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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/result_action.h"
#include "zvision/zvision.h"

namespace ZVision {

//////////////////////////////////////////////////////////////////////////////
// ActionAdd
//////////////////////////////////////////////////////////////////////////////

ActionAdd::ActionAdd(Common::String line) {
	sscanf(line.c_str(), ":add(%u,%hhu)", &_key, &_value);
}

bool ActionAdd::execute(ZVision *zVision) {
	zVision->getScriptManager()->addToStateValue(_key, _value);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionAssign
//////////////////////////////////////////////////////////////////////////////

ActionAssign::ActionAssign(Common::String line) {
	sscanf(line.c_str(), ":assign(%u, %hhu)", &_key, &_value);
}

bool ActionAssign::execute(ZVision *zVision) {
	zVision->getScriptManager()->setStateValue(_key, _value);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionAttenuate
//////////////////////////////////////////////////////////////////////////////

ActionAttenuate::ActionAttenuate(Common::String line) {
	sscanf(line.c_str(), ":assign(%u, %hd)", &_key, &_attenuation);
}

bool ActionAttenuate::execute(ZVision *zVision) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionChangeLocation
//////////////////////////////////////////////////////////////////////////////

ActionChangeLocation::ActionChangeLocation(Common::String line) {
	sscanf(line.c_str(), ":change_location(%c,%c,%2c,%hu)", &_world, &_room, &_nodeview, &_x);
}

bool ActionChangeLocation::execute(ZVision *zVision) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionCrossfade
//////////////////////////////////////////////////////////////////////////////

ActionCrossfade::ActionCrossfade(Common::String line) {
	sscanf(line.c_str(), 
           ":crossfade(%u %u %hhu %hhu %hhu %hhu %hu)",
           &_keyOne, &_keyTwo, &_oneStartVolume, &_twoStartVolume, &_oneEndVolume, &_twoEndVolume, &_timeInMillis);
}

bool ActionCrossfade::execute(ZVision *zVision) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionPreloadAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPreloadAnimation::ActionPreloadAnimation(Common::String line) {
	// The two %*hhu are always 0 and dont seem to have a use
	sscanf(line.c_str(), ":animpreload:%u(%s %*hhu %*hhu %u %hhu)", &_key, &_fileName, &_mask, &_framerate);
}

bool ActionPreloadAnimation::execute(ZVision *zVision) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionPlayAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPlayAnimation::ActionPlayAnimation(Common::String line) {
	// The two %*hhu are always 0 and dont seem to have a use
	sscanf(line.c_str(), 
           ":animplay:%u(%s %u %u %u %u %u %u %hhu %*hhu %*hhu %u %hhu)",
           &_key, &_x, &_y, &_width, &_height, &_start, &_end, &_loop, &_mask, &_framerate);
}

bool ActionPlayAnimation::execute(ZVision *zVision) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionRandom
//////////////////////////////////////////////////////////////////////////////

ActionRandom::ActionRandom(Common::String line) {
	sscanf(line.c_str(), ":random:%u, %u)", &_key, &_max);
}

bool ActionRandom::execute(ZVision *zVision) {
	uint32 randNumber = zVision->getRandomSource()->getRandomNumber(_max);
	zVision->getScriptManager()->setStateValue(_key, randNumber);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionTimer
//////////////////////////////////////////////////////////////////////////////

ActionTimer::ActionTimer(Common::String line) {
	sscanf(line.c_str(), ":timer:%u(%hu)", &_key, &_time);
}

bool ActionTimer::execute(ZVision *zVision) {
	// TODO: Implement
	return true;
}

} // End of namespace ZVision
