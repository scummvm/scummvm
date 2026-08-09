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

#include "mads/nebular/sound/sound.h"
#include "mads/nebular/sound/asound_nebular.h"
#include "mads/nebular/sound/isound_nebular.h"
#include "mads/nebular/sound/rsound_nebular.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

void RexSoundManager::validate() {
	if (_isDemo)
		_driverType = SOUND_ADLIB;

	switch (_driverType) {
	case SOUND_MT32:
		RSound::validate();
		break;

	case SOUND_PCSPEAKER:
		ISound::validate();
		break;

	default:
		ASound::validate(_isDemo);
		break;
	}
}

void RexSoundManager::loadDriver(int sectionNumber) {
	removeDriver();

	if (_isDemo) {
		assert(sectionNumber == 1 || sectionNumber == 9);
		if (sectionNumber == 1)
			_driver = new ASoundDemo1(_mixer);
		else
			_driver = new ASoundDemo9(_mixer);
		return;
	}

	switch (_driverType) {
	case SOUND_MT32:
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
		case 7:
			_driver = new RSound7(_mixer);
			break;
		case 8:
			_driver = new RSound8(_mixer);
			break;
		case 9:
			_driver = new RSound9(_mixer);
			break;
		default:
			return;
		}
		break;

	case SOUND_PCSPEAKER:
		switch (sectionNumber) {
		case 1:
			_driver = new ISound1(_mixer);
			break;
		case 2:
			_driver = new ISound2(_mixer);
			break;
		case 3:
			_driver = new ISound3(_mixer);
			break;
		case 4:
			_driver = new ISound4(_mixer);
			break;
		case 5:
			_driver = new ISound5(_mixer);
			break;
		case 6:
			_driver = new ISound6(_mixer);
			break;
		case 7:
			_driver = new ISound7(_mixer);
			break;
		case 8:
			_driver = new ISound8(_mixer);
			break;
		case 9:
			_driver = new ISound9(_mixer);
			break;
		default:
			return;
		}
		break;

	default:
		// Adlib drivers
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
		case 6:
			_driver = new ASound6(_mixer);
			break;
		case 7:
			_driver = new ASound7(_mixer);
			break;
		case 8:
			_driver = new ASound8(_mixer);
			break;
		case 9:
			_driver = new ASound9(_mixer);
			break;
		default:
			return;
		}
	}
}

} // namespace Sound
} // namespace RexNebular
} // namespace MADS
