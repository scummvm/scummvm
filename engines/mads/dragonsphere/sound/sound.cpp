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

#include "mads/dragonsphere/sound/sound.h"
#include "mads/dragonsphere/sound/asound_dragonsphere.h"
#include "mads/dragonsphere/sound/rsound_dragonsphere.h"

namespace MADS {
namespace Dragonsphere {
namespace Sound {

void DragonSoundManager::validate() {
	if (_driverType == SOUND_MT32 && !_isDemo) {
		RSound::validate();
	} else {
		ASound::validate(_isDemo);
	}
}

void DragonSoundManager::loadDriver(int sectionNumber) {
	removeDriver();

	if (_driverType == SOUND_MT32 && !_isDemo) {
		// Roland MT32 drivers
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
		case 6:
			_driver = new RSound6(_mixer);
			break;
		case 9:
			_driver = new RSound9(_mixer);
			break;
		default:
			_driver = nullptr;
			break;
		}
	} else {
		// Adlib drivers
		switch (sectionNumber) {
		case 1:
			if (_isDemo)
				_driver = new ASoundDemo1(_mixer);
			else
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
		case 6:
			_driver = new ASound6(_mixer);
			break;
		case 9:
			if (_isDemo)
				_driver = new ASoundDemo9(_mixer);
			else
				_driver = new ASound9(_mixer);
			break;
		default:
			_driver = nullptr;
			break;
		}
	}
}

} // namespace Sound
} // namespace Dragonsphere
} // namespace MADS
