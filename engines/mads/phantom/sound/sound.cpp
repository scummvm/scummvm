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
	switch (_driverType) {
	case SOUND_MT32:
		// TODO
		assert(0 == 1);
		break;

	default:
		// Adlib
		ASound::validate(_isDemo);
		break;
	}
}

void PhantomSoundManager::loadDriver(int sectionNumber) {
	removeDriver();

	switch (_driverType) {
	case SOUND_MT32:
		// Roland MT32 drivers
		assert(sectionNumber == 1);
		_driver = new RSound1(_mixer);
		break;

	default:	
		// Adlib drivers
		if (_isDemo) {
			_driver = new ASoundDemo(_mixer);
			return;
		}

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
			return;
		}
	}
}

} // namespace Sound
} // namespace Phantom
} // namespace MADS
