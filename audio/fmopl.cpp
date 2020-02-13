/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#include "audio/fmopl.h"

#include "audio/mixer.h"
#include "audio/softsynth/opl/dosbox.h"
#include "audio/softsynth/opl/mame.h"
#include "audio/softsynth/opl/nuked.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/timer.h"
#include "common/translation.h"

namespace OPL {

// Factory functions

#ifdef USE_ALSA
namespace ALSA {
	OPL *create(Config::OplType type);
} // End of namespace ALSA
#endif // USE_ALSA

#ifdef ENABLE_OPL2LPT
namespace OPL2LPT {
	OPL *create(Config::OplType type);
} // End of namespace OPL2LPT
#endif // ENABLE_OPL2LPT

// Config implementation

enum OplEmulator {
	kAuto = 0,
	kMame = 1,
	kDOSBox = 2,
	kALSA = 3,
	kNuked = 4,
	kOPL2LPT = 5,
	kOPL3LPT = 6
};

OPL::OPL() {
	if (_hasInstance)
		error("There are multiple OPL output instances running");
	_hasInstance = true;
}

const Config::EmulatorDescription Config::_drivers[] = {
	{ "auto", "<default>", kAuto, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3 },
	{ "mame", _s("MAME OPL emulator"), kMame, kFlagOpl2 },
#ifndef DISABLE_DOSBOX_OPL
	{ "db", _s("DOSBox OPL emulator"), kDOSBox, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3 },
#endif
#ifndef DISABLE_NUKED_OPL
	{ "nuked", _s("Nuked OPL emulator"), kNuked, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3 },
#endif
#ifdef USE_ALSA
	{ "alsa", _s("ALSA Direct FM"), kALSA, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3 },
#endif
#ifdef ENABLE_OPL2LPT
	{ "opl2lpt", _s("OPL2LPT"), kOPL2LPT, kFlagOpl2},
	{ "opl3lpt", _s("OPL3LPT"), kOPL3LPT, kFlagOpl2 | kFlagOpl3 },
#endif
	{ 0, 0, 0, 0 }
};

Config::DriverId Config::parse(const Common::String &name) {
	for (int i = 0; _drivers[i].name; ++i) {
		if (name.equalsIgnoreCase(_drivers[i].name))
			return _drivers[i].id;
	}

	return -1;
}

const Config::EmulatorDescription *Config::findDriver(DriverId id) {
	for (int i = 0; _drivers[i].name; ++i) {
		if (_drivers[i].id == id)
			return &_drivers[i];
	}

	return 0;
}

Config::DriverId Config::detect(OplType type) {
	uint32 flags = 0;
	switch (type) {
	case kOpl2:
		flags = kFlagOpl2;
		break;

	case kDualOpl2:
		flags = kFlagDualOpl2;
		break;

	case kOpl3:
		flags = kFlagOpl3;
		break;

	default:
		break;
	}

	DriverId drv = parse(ConfMan.get("opl_driver"));
	if (drv == kAuto) {
		// Since the "auto" can be explicitly set for a game, and this
		// driver shows up in the GUI as "<default>", check if there is
		// a global setting for it before resorting to auto-detection.
		drv = parse(ConfMan.get("opl_driver", Common::ConfigManager::kApplicationDomain));
	}

	// When a valid driver is selected, check whether it supports
	// the requested OPL chip.
	if (drv != -1 && drv != kAuto) {
		const EmulatorDescription *driverDesc = findDriver(drv);
		// If the chip is supported, just use the driver.
		if (!driverDesc) {
			warning("The selected OPL driver %d could not be found", drv);
		} else if ((flags & driverDesc->flags)) {
			return drv;
		} else {
			// Else we will output a warning and just
			// return that no valid driver is found.
			warning("Your selected OPL driver \"%s\" does not support type %d emulation, which is requested by your game", _drivers[drv].description, type);
			return -1;
		}
	}

	// Detect the first matching emulator
	drv = -1;

	for (int i = 1; _drivers[i].name; ++i) {
		if (_drivers[i].flags & flags) {
			drv = _drivers[i].id;
			break;
		}
	}

	return drv;
}

OPL *Config::create(OplType type) {
	return create(kAuto, type);
}

OPL *Config::create(DriverId driver, OplType type) {
	// On invalid driver selection, we try to do some fallback detection
	if (driver == -1) {
		warning("Invalid OPL driver selected, trying to detect a fallback emulator");
		driver = kAuto;
	}

	// If autodetection is selected, we search for a matching
	// driver.
	if (driver == kAuto) {
		driver = detect(type);

		// No emulator for the specified OPL chip could
		// be found, thus stop here.
		if (driver == -1) {
			warning("No OPL emulator available for type %d", type);
			return 0;
		}
	}

	switch (driver) {
	case kMame:
		if (type == kOpl2)
			return new MAME::OPL();
		else
			warning("MAME OPL emulator only supports OPL2 emulation");
		return 0;

#ifndef DISABLE_DOSBOX_OPL
	case kDOSBox:
		return new DOSBox::OPL(type);
#endif

#ifndef DISABLE_NUKED_OPL
	case kNuked:
		return new NUKED::OPL(type);
#endif

#ifdef USE_ALSA
	case kALSA:
		return ALSA::create(type);
#endif

#ifdef ENABLE_OPL2LPT
	case kOPL2LPT:
		if (type == kOpl2) {
			return OPL2LPT::create(type);
		}

		warning("OPL2LPT only supprts OPL2");
		return 0;
	case kOPL3LPT:
		if (type == kOpl2 || type == kOpl3) {
			return OPL2LPT::create(type);
		}

		warning("OPL3LPT does not support dual OPL2");
		return 0;
#endif

	default:
		warning("Unsupported OPL emulator %d", driver);
		// TODO: Maybe we should add some dummy emulator too, which just outputs
		// silence as sound?
		return 0;
	}
}

void OPL::start(TimerCallback *callback, int timerFrequency) {
	_callback.reset(callback);
	startCallbacks(timerFrequency);
}

void OPL::stop() {
	stopCallbacks();
	_callback.reset();
}

bool OPL::_hasInstance = false;

RealOPL::RealOPL() : _baseFreq(0), _remainingTicks(0) {
}

RealOPL::~RealOPL() {
	// Stop callbacks, just in case. If it's still playing at this
	// point, there's probably a bigger issue, though. The subclass
	// needs to call stop() or the pointer can still use be used in
	// the mixer thread at the same time.
	stop();
}

void RealOPL::setCallbackFrequency(int timerFrequency) {
	stopCallbacks();
	startCallbacks(timerFrequency);
}

void RealOPL::startCallbacks(int timerFrequency) {
	_baseFreq = timerFrequency;
	assert(_baseFreq > 0);

	// We can't request more a timer faster than 100Hz. We'll handle this by calling
	// the proc multiple times in onTimer() later on.
	if (timerFrequency > kMaxFreq)
		timerFrequency = kMaxFreq;

	_remainingTicks = 0;
	g_system->getTimerManager()->installTimerProc(timerProc, 1000000 / timerFrequency, this, "RealOPL");
}

void RealOPL::stopCallbacks() {
	g_system->getTimerManager()->removeTimerProc(timerProc);
	_baseFreq = 0;
	_remainingTicks = 0;
}

void RealOPL::timerProc(void *refCon) {
	static_cast<RealOPL *>(refCon)->onTimer();
}

void RealOPL::onTimer() {
	uint callbacks = 1;

	if (_baseFreq > kMaxFreq) {
		// We run faster than our max, so run the callback multiple
		// times to approximate the actual timer callback frequency.
		uint totalTicks = _baseFreq + _remainingTicks;
		callbacks = totalTicks / kMaxFreq;
		_remainingTicks = totalTicks % kMaxFreq;
	}

	// Call the callback multiple times. The if is on the inside of the
	// loop in case the callback removes itself.
	for (uint i = 0; i < callbacks; i++)
		if (_callback && _callback->isValid())
			(*_callback)();
}

EmulatedOPL::EmulatedOPL() :
	_nextTick(0),
	_samplesPerTick(0),
	_baseFreq(0),
	_handle(new Audio::SoundHandle()) {
}

EmulatedOPL::~EmulatedOPL() {
	// Stop callbacks, just in case. If it's still playing at this
	// point, there's probably a bigger issue, though. The subclass
	// needs to call stop() or the pointer can still use be used in
	// the mixer thread at the same time.
	stop();

	delete _handle;
}

int EmulatedOPL::readBuffer(int16 *buffer, const int numSamples) {
	const int stereoFactor = isStereo() ? 2 : 1;
	int len = numSamples / stereoFactor;
	int step;

	do {
		step = len;
		if (step > (_nextTick >> FIXP_SHIFT))
			step = (_nextTick >> FIXP_SHIFT);

		generateSamples(buffer, step * stereoFactor);

		_nextTick -= step << FIXP_SHIFT;
		if (!(_nextTick >> FIXP_SHIFT)) {
			if (_callback && _callback->isValid())
				(*_callback)();

			_nextTick += _samplesPerTick;
		}

		buffer += step * stereoFactor;
		len -= step;
	} while (len);

	return numSamples;
}

int EmulatedOPL::getRate() const {
	return g_system->getMixer()->getOutputRate();
}

void EmulatedOPL::startCallbacks(int timerFrequency) {
	setCallbackFrequency(timerFrequency);
	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, _handle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

void EmulatedOPL::stopCallbacks() {
	g_system->getMixer()->stopHandle(*_handle);
}

void EmulatedOPL::setCallbackFrequency(int timerFrequency) {
	_baseFreq = timerFrequency;
	assert(_baseFreq != 0);

	int d = getRate() / _baseFreq;
	int r = getRate() % _baseFreq;

	// This is equivalent to (getRate() << FIXP_SHIFT) / BASE_FREQ
	// but less prone to arithmetic overflow.

	_samplesPerTick = (d << FIXP_SHIFT) + (r << FIXP_SHIFT) / _baseFreq;
}

} // End of namespace OPL
