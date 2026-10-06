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
#include "audio/fmopl.h"
#include "common/textconsole.h"
#include "mads/phantom/sound/asound_phantom.h"
#include "mads/phantom/sound/isound_phantom.h"
#include "mads/phantom/sound/psound_phantom.h"
#include "mads/phantom/sound/rsound_phantom.h"

namespace MADS {
namespace Phantom {
namespace Sound {

namespace {

const int kRetailSections[] = { 1, 2, 3, 4, 5, 9 };

Common::Path getISoundFilename(int sectionNumber) {
	return Common::Path(Common::String::format("ISOUND.PH%d", sectionNumber));
}

SoundDriver *createASound(Audio::Mixer *mixer, int sectionNumber) {
	switch (sectionNumber) {
	case 1:
		return new ASound1(mixer);
	case 2:
		return new ASound2(mixer);
	case 3:
		return new ASound3(mixer);
	case 4:
		return new ASound4(mixer);
	case 5:
		return new ASound5(mixer);
	case 9:
		return new ASound9(mixer);
	default:
		return nullptr;
	}
}

SoundDriver *createISound(Audio::Mixer *mixer, int sectionNumber) {
	switch (sectionNumber) {
	case 1:
		return new ISound1(mixer);
	case 2:
		return new ISound2(mixer);
	case 3:
		return new ISound3(mixer);
	case 4:
		return new ISound4(mixer);
	case 5:
		return new ISound5(mixer);
	case 9:
		return new ISound9(mixer);
	default:
		return nullptr;
	}
}

SoundDriver *createPSound(Audio::Mixer *mixer, int sectionNumber) {
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
	case 9:
		return new PSound9(mixer);
	default:
		return nullptr;
	}
}

} // namespace

PhantomSoundManager::PhantomSoundManager(Audio::Mixer *mixer,
		bool &soundFlag, bool usePas, bool isDemo) :
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

void PhantomSoundManager::validate() {
	if (_driverType == SOUND_PAS) {
		bool valid = true;
		if (_isDemo) {
			Common::String reason;
			valid = validatePhantomPSoundFile(1, true, &reason);
			if (!valid)
				warning("Cannot use Phantom demo PSOUND: %s; using AdLib",
						reason.c_str());
		} else {
			for (uint index = 0; index < ARRAYSIZE(kRetailSections); ++index) {
				Common::String reason;
				if (!validatePhantomPSoundFile(kRetailSections[index], false,
						&reason)) {
					warning("Cannot use Phantom PSOUND section %d: %s; "
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
		if (_isDemo) {
			Common::String reason;
			if (!RSoundDemoPHA::validate(&reason))
				error("Cannot use Phantom demo RSOUND.PHA: %s", reason.c_str());
		} else {
			RSound::validate();
		}
	} else if (_driverType == SOUND_PCSPEAKER && !_isDemo) {
		bool needsAdlibFallback = false;
		for (uint index = 0; index < ARRAYSIZE(kRetailSections); ++index) {
			Common::String reason;
			if (!ISoundSection::isOverlaySupported(kRetailSections[index], &reason)) {
				const Common::Path filename = getISoundFilename(kRetailSections[index]);
				warning("Cannot use %s: %s; section %d will use AdLib",
					filename.toString().c_str(), reason.c_str(), kRetailSections[index]);
				needsAdlibFallback = true;
			}
		}
		if (needsAdlibFallback)
			ASound::validate(false);
	} else {
		// Adlib
		ASound::validate(_isDemo);
	}
}

void PhantomSoundManager::loadDriver(int sectionNumber) {
	removeDriver();

	if (_driverType == SOUND_PAS) {
		if (_isDemo)
			_driver = new PSoundDemo(_mixer);
		else
			_driver = createPSound(_mixer, sectionNumber);
		if (_driver && !static_cast<PSound *>(_driver)->isReady()) {
			warning("Could not initialize Pro Audio Spectrum 16 OPL3 output; "
					"falling back to AdLib");
			removeDriver();
			_driverType = SOUND_ADLIB;
			loadDriver(sectionNumber);
		}
	} else if (_driverType == SOUND_MT32) {
		if (_isDemo) {
			_driver = new RSoundDemoPHA(_mixer);
		} else {
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
		}
	} else if (_isDemo) {
		_driver = new ASoundDemo(_mixer);
		_driver->command(0, 0);
	} else if (_driverType == SOUND_PCSPEAKER) {
		const Common::Path filename = getISoundFilename(sectionNumber);
		Common::String reason;
		if (ISoundSection::isOverlaySupported(sectionNumber, &reason)) {
			_driver = createISound(_mixer, sectionNumber);
		} else {
			warning("Cannot use %s: %s; using AdLib for section %d",
				filename.toString().c_str(), reason.c_str(), sectionNumber);
			_driver = createASound(_mixer, sectionNumber);
			if (_driver)
				_driver->command(0, 0);
		}
	} else {
		// Adlib
		_driver = createASound(_mixer, sectionNumber);
		if (_driver)
			_driver->command(0, 0);
	}
}

} // namespace Sound
} // namespace Phantom
} // namespace MADS
