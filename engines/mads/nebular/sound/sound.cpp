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

#include "audio/fmopl.h"
#include "common/textconsole.h"
#include "mads/nebular/sound/asound_nebular.h"
#include "mads/nebular/sound/isound_nebular.h"
#include "mads/nebular/sound/psound_nebular.h"
#include "mads/nebular/sound/rsound_nebular.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

RexSoundManager::RexSoundManager(Audio::Mixer *mixer, bool &soundFlag,
		bool usePas, bool isDemo) :
		SoundManager(mixer, soundFlag), _isDemo(isDemo) {
	if (usePas && _driverType == SOUND_ADLIB) {
		if (OPL::Config::detect(OPL::Config::kOpl3) >= 0) {
			_driverType = SOUND_PAS;
		} else {
			warning("Pro Audio Spectrum 16 requires OPL3 output; "
					"falling back to AdLib");
		}
	}
}

void RexSoundManager::validate() {
	// The demo has distinct AdLib, MT-32 and PAS overlays, but no ISOUND set.
	if (_isDemo && _driverType == SOUND_PCSPEAKER)
		_driverType = SOUND_ADLIB;

	switch (_driverType) {
	case SOUND_MT32:
		RSound::validate(_isDemo);
		break;

	case SOUND_PCSPEAKER:
		ISound::validate();
		break;

	case SOUND_PAS:
		PSound::validate(_isDemo);
		break;

	default:
		ASound::validate(_isDemo);
		break;
	}
}

void RexSoundManager::loadDriver(int sectionNumber) {
	removeDriver();

	if (_isDemo && _driverType == SOUND_ADLIB) {
		switch (sectionNumber) {
		case 1:
			_driver = new ASoundDemo1(_mixer);
			break;
		case 9:
			_driver = new ASoundDemo9(_mixer);
			break;
		default:
			return;
		}
		return;
	}

	switch (_driverType) {
	case SOUND_MT32:
		// Roland MT32 drivers
		if (_isDemo) {
			// The demo shares RSOUND.001 across numbered gameplay sections
			// and uses RSOUND.009 only for its opening presentation.
			if (sectionNumber == 9)
				_driver = new RSoundDemo9(_mixer);
			else
				_driver = new RSoundDemo1(_mixer);
			break;
		}

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

	case SOUND_PAS:
		// Pro Audio Spectrum drivers
		if (_isDemo) {
			switch (sectionNumber) {
			case 1:
				_driver = new PSoundDemo1(_mixer);
				break;
			case 9:
				_driver = new PSoundDemo9(_mixer);
				break;
			default:
				return;
			}
		} else {
			switch (sectionNumber) {
			case 1:
				_driver = new PSound1(_mixer);
				break;
			case 2:
				_driver = new PSound2(_mixer);
				break;
			case 3:
				_driver = new PSound3(_mixer);
				break;
			case 4:
				_driver = new PSound4(_mixer);
				break;
			case 5:
				_driver = new PSound5(_mixer);
				break;
			case 6:
				_driver = new PSound6(_mixer);
				break;
			case 7:
				_driver = new PSound7(_mixer);
				break;
			case 8:
				_driver = new PSound8(_mixer);
				break;
			case 9:
				_driver = new PSound9(_mixer);
				break;
			default:
				return;
			}
		}

		if (_driver && !static_cast<PSound *>(_driver)->isReady()) {
			warning("Could not initialize Pro Audio Spectrum 16 OPL3 output; "
					"falling back to AdLib");
			removeDriver();
			_driverType = SOUND_ADLIB;
			loadDriver(sectionNumber);
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
