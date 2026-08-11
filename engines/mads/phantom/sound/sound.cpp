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

#include "mads/phantom/sound/sound.h"
#include "mads/phantom/sound/asound_phantom.h"
#include "mads/phantom/sound/rsound_phantom.h"

namespace MADS {
namespace Phantom {
namespace Sound {

void PhantomSoundManager::validate() {
	if (_driverType == SOUND_MT32 && !_isDemo) {
		// MT32
		RSound::validate();
	} else {
		// Adlib
		ASound::validate(_isDemo);
	}
}

void PhantomSoundManager::loadDriver(int sectionNumber) {
	removeDriver();

	if (_isDemo) {
		_driver = new ASoundDemo(_mixer);

	} else if (_driverType == SOUND_MT32) {
		// MT32
		switch (sectionNumber) {
		case 1:
			_driver = new RSound1(_mixer);
			break;
		case 2:
			_driver = new RSound2(_mixer);
			break;
		case 3:
			_driver = new RSound3(_mixer);
			break;
		case 4:
			_driver = new RSound4(_mixer);
			break;
		case 5:
			_driver = new RSound5(_mixer);
			break;
		case 9:
			_driver = new RSound9(_mixer);
			break;
		default:
			_driver = nullptr;
			break;
		}
	} else {
		// Adlib
		switch (sectionNumber) {
		case 1:
			_driver = new ASound1(_mixer);
			break;
		case 2:
			_driver = new ASound2(_mixer);
			break;
		case 3:
			_driver = new ASound3(_mixer);
			break;
		case 4:
			_driver = new ASound4(_mixer);
			break;
		case 5:
			_driver = new ASound5(_mixer);
			break;
		case 9:
			_driver = new ASound9(_mixer);
			break;
		default:
			_driver = nullptr;
			break;
		}
	}
}

} // namespace Sound
} // namespace Phantom
} // namespace MADS
