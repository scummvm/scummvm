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

#include "mads/nebular/sound.h"
#include "mads/nebular/asound_nebular.h"
#include "mads/nebular/isound_nebular.h"
#include "mads/nebular/rsound_nebular.h"

namespace MADS {
namespace RexNebular {

void RexSoundManager::validate() {
	switch (_driverType) {
	case SOUND_MT32:
		RSound::validate();
		break;

	case SOUND_PCSPEAKER:
		// No validation needed
		break;

	default:
		ASound::validate();
		break;
	}
}

void RexSoundManager::loadDriver(int sectionNumber) {
	removeDriver();

	switch (_driverType) {
	case SOUND_MT32:
		// Roland MT32 drivers
		switch (sectionNumber) {
		case 1:
			_driver = new RexNebular::RSound1(_mixer);
			break;
		case 2:
			_driver = new RexNebular::RSound2(_mixer);
			break;
		case 3:
			_driver = new RexNebular::RSound3(_mixer);
			break;
		case 4:
			_driver = new RexNebular::RSound4(_mixer);
			break;
		case 5:
			_driver = new RexNebular::RSound5(_mixer);
			break;
		case 6:
			_driver = new RexNebular::RSound6(_mixer);
			break;
		case 7:
			_driver = new RexNebular::RSound7(_mixer);
			break;
		case 8:
			_driver = new RexNebular::RSound8(_mixer);
			break;
		case 9:
			_driver = new RexNebular::RSound9(_mixer);
			break;
		default:
			return;
		}
		break;

	case SOUND_PCSPEAKER:
		switch (sectionNumber) {
		case 1:
			_driver = new RexNebular::ISound1(_mixer);
			break;
		case 2:
			_driver = new RexNebular::ISound2(_mixer);
			break;
		case 3:
			_driver = new RexNebular::ISound3(_mixer);
			break;
		case 4:
			_driver = new RexNebular::ISound4(_mixer);
			break;
		case 5:
			_driver = new RexNebular::ISound5(_mixer);
			break;
		case 6:
			_driver = new RexNebular::ISound6(_mixer);
			break;
		case 7:
			_driver = new RexNebular::ISound7(_mixer);
			break;
		case 8:
			_driver = new RexNebular::ISound8(_mixer);
			break;
		case 9:
			_driver = new RexNebular::ISound9(_mixer);
			break;
		default:
			return;
		}
		break;

	default:
		// Adlib drivers
		switch (sectionNumber) {
		case 1:
			_driver = new RexNebular::ASound1(_mixer);
			break;
		case 2:
			_driver = new RexNebular::ASound2(_mixer);
			break;
		case 3:
			_driver = new RexNebular::ASound3(_mixer);
			break;
		case 4:
			_driver = new RexNebular::ASound4(_mixer);
			break;
		case 5:
			_driver = new RexNebular::ASound5(_mixer);
			break;
		case 6:
			_driver = new RexNebular::ASound6(_mixer);
			break;
		case 7:
			_driver = new RexNebular::ASound7(_mixer);
			break;
		case 8:
			_driver = new RexNebular::ASound8(_mixer);
			break;
		case 9:
			_driver = new RexNebular::ASound9(_mixer);
			break;
		default:
			return;
		}
	}
}

} // namespace RexNebular
} // namespace MADS
