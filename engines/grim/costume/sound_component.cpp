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

#include "engines/grim/grim.h"
#include "engines/grim/set.h"
#include "engines/grim/debug.h"
#include "engines/grim/costume.h"
#include "engines/grim/costume/sound_component.h"
#include "engines/grim/imuse/dimuse_engine.h"

namespace Grim {

SoundComponent::SoundComponent(Component *p, int parentID, const char *filename, tag32 t) :
		Component(p, parentID, filename, t), _maxVolume(127), _pan(64), _priority(100), _minVolume(1) {
	// The name can be followed by the maximum volume, the pan, the priority and the minimum volume
	const char *comma = strchr(filename, ',');
	if (comma) {
		_name = Common::String(filename, comma);
		sscanf(comma + 1, "%d,%d,%d,%d", &_maxVolume, &_pan, &_priority, &_minVolume);
	} else {
		_name = filename;
	}
}

SoundComponent::~SoundComponent() {
	// Stop the sound if it's in progress
	reset();
}

void SoundComponent::setKey(int val) {
	switch (val) {
	case 0: // "Play"
		if (!g_imuse->soundsIsSoundPlaying(_name.c_str())) {
			if (isComponentType('I','M','S','S')) {
				g_imuse->soundStartVoiceStream(_name.c_str(), _maxVolume, _pan);
				g_imuse->soundsSetParam(_name.c_str(), DIMUSE_P_GROUP, DIMUSE_GROUP_SFX);
				g_imuse->soundsSetParam(_name.c_str(), DIMUSE_P_PRIORITY, _priority);
			} else {
				g_imuse->soundsLoadStartSound(_name.c_str(), _priority, DIMUSE_GROUP_SFX);

				Set *currSet = g_grim->getCurrSet();
				if (currSet) {
					float maxDist, panFactor, maxRadius;
					int roomMinVolume, roomMaxVolume;
					currSet->getSoundParameters(&maxDist, &roomMinVolume, &roomMaxVolume, &panFactor, &maxRadius);

					Math::Vector3d pos = _cost->getMatrix().getPosition();
					currSet->setSoundPosition(_name.c_str(), pos, MAX(roomMinVolume, _minVolume), MIN(roomMaxVolume, _maxVolume), maxDist, maxRadius);
				}
			}
		}
		break;
	case 1: // "Stop"
		if (isComponentType('I','M','S','S'))
			g_imuse->soundStopVoiceStream();
		else
			g_imuse->soundsStopSound(_name.c_str());
		break;
	case 2: // "Stop Looping"
		g_imuse->iMUSESetHook(_name.c_str(), 0x80);
		break;
	default:
		Debug::warning(Debug::Costumes, "Unknown key %d for sound %s", val, _name.c_str());
	}
}

void SoundComponent::reset() {
	// A lot of the sound components this gets called against aren't actually running
	if (g_imuse && !_name.empty())
		g_imuse->soundsStopSound(_name.c_str());
}

} // end of namespace Grim
