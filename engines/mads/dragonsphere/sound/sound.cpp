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
#include "audio/fmopl.h"
#include "common/textconsole.h"
#include "mads/dragonsphere/sound/asound_dragonsphere.h"
#include "mads/dragonsphere/sound/gsound_dragonsphere.h"
#include "mads/dragonsphere/sound/psound_dragonsphere.h"
#include "mads/dragonsphere/sound/rsound_dragonsphere.h"

namespace MADS {
namespace Dragonsphere {
namespace Sound {

namespace {

const int kRetailSections[] = { 1, 2, 3, 4, 5, 6, 9 };

SoundDriver *createPSound(Audio::Mixer *mixer, int sectionNumber,
		bool isDemo) {
	if (isDemo) {
		switch (sectionNumber) {
		case 1:
			return new PSoundDemo1(mixer);
		case 9:
			return new PSoundDemo9(mixer);
		default:
			return nullptr;
		}
	}

	switch (sectionNumber) {
	case 1:
		return new PSound1(mixer);
	case 2:
		return new PSound2(mixer);
	case 3:
		return new PSound3(mixer);
	case 4:
		return new PSound4(mixer);
	case 5:
		return new PSound5(mixer);
	case 6:
		return new PSound6(mixer);
	case 9:
		return new PSound9(mixer);
	default:
		return nullptr;
	}
}

SoundDriver *createGSound(Audio::Mixer *mixer, int sectionNumber) {
	switch (sectionNumber) {
	case 1: return new GSound1(mixer);
	case 2: return new GSound2(mixer);
	case 3: return new GSound3(mixer);
	case 4: return new GSound4(mixer);
	case 5: return new GSound5(mixer);
	case 6: return new GSound6(mixer);
	case 9: return new GSound9(mixer);
	default: return nullptr;
	}
}

} // namespace

DragonSoundManager::DragonSoundManager(Audio::Mixer *mixer,
		bool &soundFlag, bool usePas, bool isDemo) :
		SoundManager(mixer, soundFlag, !isDemo), _isDemo(isDemo) {
	if (usePas && _driverType == SOUND_ADLIB) {
		if (OPL::Config::detect(OPL::Config::kOpl3) >= 0) {
			_driverType = SOUND_PAS;
		} else {
			warning("Pro Audio Spectrum 16 requires OPL3 output; "
					"falling back to AdLib");
		}
	}
}

void DragonSoundManager::validate() {
	if (_driverType == SOUND_GM) {
		if (!_isDemo && validateDragonsphereGSoundFiles())
			return;
		warning("Cannot use Dragonsphere General MIDI sound data; using AdLib");
		_driverType = SOUND_ADLIB;
		ASound::validate(_isDemo);
	} else if (_driverType == SOUND_PAS) {
		bool valid = true;
		if (_isDemo) {
			const int demoSections[] = { 1, 9 };
			for (uint index = 0; index < ARRAYSIZE(demoSections); ++index) {
				Common::String reason;
				if (!validateDragonspherePSoundFile(demoSections[index], true,
						&reason)) {
					warning("Cannot use Dragonsphere demo PSOUND section %d: "
							"%s; using AdLib", demoSections[index],
							reason.c_str());
					valid = false;
				}
			}
		} else {
			for (uint index = 0; index < ARRAYSIZE(kRetailSections); ++index) {
				Common::String reason;
				if (!validateDragonspherePSoundFile(kRetailSections[index], false,
						&reason)) {
					warning("Cannot use Dragonsphere PSOUND section %d: %s; "
							"using AdLib", kRetailSections[index],
							reason.c_str());
					valid = false;
				}
			}
		}
		if (valid)
			return;
		_driverType = SOUND_ADLIB;
		ASound::validate(_isDemo);
	} else if (_driverType == SOUND_MT32) {
		if (_isDemo)
			RSoundDemo::validate();
		else
			RSound::validate();
	} else {
		ASound::validate(_isDemo);
	}
}

void DragonSoundManager::loadDriver(int sectionNumber) {
	removeDriver();

	if (_driverType == SOUND_GM) {
		_driver = createGSound(_mixer, sectionNumber);
		if (_driver && !static_cast<GSound *>(_driver)->isReady()) {
			warning("Could not initialize Dragonsphere General MIDI output; "
					"falling back to AdLib");
			removeDriver();
			_driverType = SOUND_ADLIB;
			loadDriver(sectionNumber);
		}
	} else if (_driverType == SOUND_PAS) {
		_driver = createPSound(_mixer, sectionNumber, _isDemo);
		if (_driver && !static_cast<PSound *>(_driver)->isReady()) {
			warning("Could not initialize Pro Audio Spectrum 16 OPL3 output; "
					"falling back to AdLib");
			removeDriver();
			_driverType = SOUND_ADLIB;
			loadDriver(sectionNumber);
		}
	} else if (_driverType == SOUND_MT32) {
		// Roland MT32 drivers
		if (_isDemo) {
			switch (sectionNumber) {
			case 1:
				_driver = new RSoundDemo1(_mixer);
				break;
			case 9:
				_driver = new RSoundDemo9(_mixer);
				break;
			default:
				_driver = nullptr;
				break;
			}
			return;
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
