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

#include "zvision/actions.h"
#include "zvision/zvision.h"
#include "zvision/script_manager.h"
#include "zvision/action_node.h"

namespace ZVision {

//////////////////////////////////////////////////////////////////////////////
// ActionAdd
//////////////////////////////////////////////////////////////////////////////

ActionAdd::ActionAdd(const Common::String &line) {
	sscanf(line.c_str(), ":add(%u,%u)", &_key, &_value);
}

bool ActionAdd::execute(ZVision *engine) {
	engine->getScriptManager()->addToStateValue(_key, _value);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionAssign
//////////////////////////////////////////////////////////////////////////////

ActionAssign::ActionAssign(const Common::String &line) {
	sscanf(line.c_str(), ":assign(%u, %u)", &_key, &_value);
}

bool ActionAssign::execute(ZVision *engine) {
	engine->getScriptManager()->setStateValue(_key, _value);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionAttenuate
//////////////////////////////////////////////////////////////////////////////

ActionAttenuate::ActionAttenuate(const Common::String &line) {
	sscanf(line.c_str(), ":assign(%u, %d)", &_key, &_attenuation);
}

bool ActionAttenuate::execute(ZVision *engine) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionChangeLocation
//////////////////////////////////////////////////////////////////////////////

ActionChangeLocation::ActionChangeLocation(const Common::String &line) {
	sscanf(line.c_str(), ":change_location(%c,%c,%c%c,%u)", &_world, &_room, &_node, &_view, &_x);
}

bool ActionChangeLocation::execute(ZVision *engine) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionCrossfade
//////////////////////////////////////////////////////////////////////////////

ActionCrossfade::ActionCrossfade(const Common::String &line) {
	sscanf(line.c_str(),
           ":crossfade(%u %u %u %u %u %u %u)",
           &_keyOne, &_keyTwo, &_oneStartVolume, &_twoStartVolume, &_oneEndVolume, &_twoEndVolume, &_timeInMillis);
}

bool ActionCrossfade::execute(ZVision *engine) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionPreloadAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPreloadAnimation::ActionPreloadAnimation(const Common::String &line) {
	// The two %*u are always 0 and dont seem to have a use
	sscanf(line.c_str(), ":animpreload:%u(%s %*u %*u %u %u)", &_key, &_fileName, &_mask, &_framerate);

}

bool ActionPreloadAnimation::execute(ZVision *engine) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionPlayAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPlayAnimation::ActionPlayAnimation(const Common::String &line) {
	char fileName[25];

	// The two %*u are always 0 and dont seem to have a use
	sscanf(line.c_str(),
           ":animplay:%u(%s %u %u %u %u %u %u %u %*u %*u %u %u)",
           &_key, fileName, &_x, &_y, &_width, &_height, &_start, &_end, &_loop, &_mask, &_framerate);

	_fileName = Common::String(fileName);
}

bool ActionPlayAnimation::execute(ZVision *engine) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionRandom
//////////////////////////////////////////////////////////////////////////////

ActionRandom::ActionRandom(const Common::String &line) {
	sscanf(line.c_str(), ":random:%u, %u)", &_key, &_max);
}

bool ActionRandom::execute(ZVision *engine) {
	uint randNumber = engine->getRandomSource()->getRandomNumber(_max);
	engine->getScriptManager()->setStateValue(_key, randNumber);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionTimer
//////////////////////////////////////////////////////////////////////////////

ActionTimer::ActionTimer(const Common::String &line) {
	sscanf(line.c_str(), ":timer:%u(%u)", &_key, &_time);
}

}

bool ActionTimer::execute(ZVision *engine) {
	engine->getScriptManager()->addActionNode(new NodeTimer(_key, _time));
	return true;
}

} // End of namespace ZVision
