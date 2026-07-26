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
#include "mads/nebular/rsound_nebular.h"

namespace MADS {
namespace RexNebular {

void RexSoundManager::validate() {
	if (_isMT32)
		RSound::validate();
	else
		ASound::validate();
}

void RexSoundManager::loadDriver(int sectionNumber) {
	removeDriver();

	if (_isMT32) {
		// Roland MT32 drivers
		assert(sectionNumber == 9);
		_driver = new RexNebular::RSound9(_mixer);

	} else {
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
