/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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


#include "engines/grim/grim.h"
#include "engines/grim/set.h"
#include "engines/grim/debug.h"
#include "engines/grim/costume.h"

#include "engines/grim/costume/sound_component.h"

#include "engines/grim/imuse/imuse.h"

namespace Grim {


SoundComponent::SoundComponent(Component *p, int parentID, const char *filename, tag32 t) :
		Component(p, parentID, filename, t) {
	const char *comma = strchr(filename, ',');
	if (comma) {
		_name = Common::String(filename, comma);
	}
}

SoundComponent::~SoundComponent() {
	// Stop the sound if it's in progress
	reset();
}

void SoundComponent::setKey(int val) {
	switch (val) {
	case 0: // "Play"
		// No longer a need to check the sound status, if it's already playing
		// then it will just use the existing handle
		g_imuse->startSfx(_name.c_str());
		if (g_grim->getCurrSet()) {
			Math::Vector3d pos = _cost->getMatrix().getPosition();
			g_grim->getCurrSet()->setSoundPosition(_name.c_str(), pos);
		}
		break;
	case 1: // "Stop"
		g_imuse->stopSound(_name.c_str());
		break;
	case 2: // "Stop Looping"
		g_imuse->setHookId(_name.c_str(), 0x80);
		break;
	default:
		Debug::warning(Debug::Costumes, "Unknown key %d for sound %s", val, _name.c_str());
	}
}

void SoundComponent::reset() {
	// A lot of the sound components this gets called against aren't actually running
	if (g_imuse && g_imuse->getSoundStatus(_name.c_str()))
		g_imuse->stopSound(_name.c_str());
}

} // end of namespace Grim
