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

#include "audio/fmopl.h"

#ifdef USE_RETROWAVE
	#include "audio/rwopl3.h"
#endif

#ifdef USE_NFM
	#include "audio/nfmopl.h"
#endif

#include "audio/softsynth/opl/dosbox.h"
#include "audio/softsynth/opl/mame.h"
#include "audio/softsynth/opl/nuked.h"

#include "common/config-manager.h"
#include "common/textconsole.h"
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

#ifdef USE_RETROWAVE
namespace RetroWaveOPL3 {
OPL *create(Config::OplType type);
} // End of namespace RetroWaveOPL3
#endif // ENABLE_RETROWAVE_OPL3

#ifdef USE_NFM
namespace NfmOPL {
namespace RealChip {
OPL *create(Config::OplType type, enum NfmOPL::OplDevice dt);
} // End of namespace RealChip

namespace EmulatedChip {
OPL *create(Config::OplType type, enum NfmOPL::OplDevice dt);
} // End of namespace EmulatedChip
} // End of namespace NfmOPL
#endif

// Config implementation

enum OplEmulator {
	kNull = 0,
	kAuto = 1,
	kMame = 2,
	kDOSBox = 3,
	kALSA = 4,
	kNuked = 5,
	kOPL2LPT = 6,
	kOPL3LPT = 7,
	kRWOPL3 = 8
#ifdef USE_NFM
	,kNfmNokturnFM2 = 9,
	kNfmNokturnFM3 = 10,
	kNfmRWOpl3Express = 11,
	kNfmOPL2LPT = 12,
	kNfmOPL3LPT = 13,
	kNfmCeOPL2AudioBoard = 14,
	kNfmCeOPL3Duo = 15,
	kNfmStBusIsaVmeSb = 16,
	kNfmNatfeatsNull = 17,
	kNfmNukedOpl3 = 18
#endif
};

OPL::OPL() {
	if (_hasInstance)
		error("There are multiple OPL output instances running");
	_hasInstance = true;
	_rhythmMode = false;
	_connectionFeedbackValues[0] = 0;
	_connectionFeedbackValues[1] = 0;
	_connectionFeedbackValues[2] = 0;
}

const Config::EmulatorDescription Config::_drivers[] = {
	{ "auto", "<default>", kAuto, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3 },
	{ "null", _s("None"), kNull, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3 },
#ifndef DISABLE_MAME_OPL
	{ "mame", _s("MAME OPL emulator"), kMame, kFlagOpl2 },
#endif
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
	{ "opl3lpt", _s("OPL3LPT"), kOPL3LPT, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3 },
#endif
#ifdef USE_RETROWAVE
	{"rwopl3", _s("RetroWave OPL3"), kRWOPL3, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3},
#endif
#ifdef USE_NFM
	{"nfm_nokturnfm2", _s("[nFM] NokturnFM2 (OPL2)"), kNfmNokturnFM2, kFlagOpl2 },
	{"nfm_nokturnfm3", _s("[nFM] NokturnFM3 (OPL3)"), kNfmNokturnFM3, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3},
	{"nfm_rwopl3", _s("[nFM] RetroWave USB OPL3 Express (OPL3)"), kNfmRWOpl3Express, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3},
	{"nfm_opl2lpt", _s("[nFM] Serdaco OPL2LPT (OPL2)"), kNfmOPL2LPT, kFlagOpl2 },
	{"nfm_opl3lpt", _s("[nFM] Serdaco OPL3LPT (OPL3)"), kNfmOPL3LPT, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3},
	{"nfm_ce_opl2ab", _s("[nFM] Cheerful Electronics OPL2 Audio Board (OPL2)"), kNfmCeOPL2AudioBoard, kFlagOpl2 },
	{"nfm_ce_opl3duo", _s("[nFM] Cheerful Electronics OPL3 Duo! (2xOPL3)"), kNfmCeOPL3Duo, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3},
	{"nfm_ce_stbus_isa_vme", _s("[nFM] ST Bus ISA / VME SoundBlaster"), kNfmStBusIsaVmeSb, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3},
	{"nfm_natfeats_null", _s("[nFM] NatFeats / NULL"), kNfmNatfeatsNull, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3},
	{"nfm_nuked_opl3", _s("[nFM] Nuked-OPL3 softsynth (OPL3)"), kNfmNukedOpl3, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3},
#endif
	{ nullptr, nullptr, 0, 0 }
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

	return nullptr;
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
			warning("Your selected OPL driver \"%s\" does not support type %d emulation, which is requested by your game", driverDesc->description, type);
			return -1;
		}
	}

	// Detect the first matching emulator
	drv = -1;

	for (int i = 2; _drivers[i].name; ++i) {
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
			return nullptr;
		}
	}

	switch (driver) {
#ifndef DISABLE_MAME_OPL
	case kMame:
		if (type == kOpl2)
			return new MAME::OPL();
		else
			warning("MAME OPL emulator only supports OPL2 emulation");
		return nullptr;
#endif

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
		return nullptr;
	case kOPL3LPT:
		return OPL2LPT::create(type);
#endif

#ifdef USE_RETROWAVE
	case kRWOPL3:
		return RetroWaveOPL3::create(type);
#endif

#ifdef USE_NFM
	case kNfmNokturnFM2: {
		if (type == kOpl2) {
			return NfmOPL::RealChip::create(type, NfmOPL::dtNokturnFM2);
		}
		warning("NokturnFM2 supports only OPL2");
		return nullptr;
	};

	case kNfmNokturnFM3:
		return NfmOPL::RealChip::create(type, NfmOPL::dtNokturnFM3);

	case kNfmRWOpl3Express:
		return NfmOPL::RealChip::create(type, NfmOPL::dtRWOpl3Express);

	case kNfmOPL2LPT: {
		if (type == kOpl2) {
			return NfmOPL::RealChip::create(type, NfmOPL::dtOPL2LPT);
		}
		warning("OPL2LPT supports only OPL2");
		return nullptr;
	};

	case kNfmOPL3LPT:
		return NfmOPL::RealChip::create(type, NfmOPL::dtOPL3LPT);

	case kNfmCeOPL2AudioBoard: {
		if (type == kOpl2) {
			return NfmOPL::RealChip::create(type, NfmOPL::dtOPL2AudioBoard);
		}

		warning("OPL2 Audio Board supports only OPL2");
		return nullptr;
	};

	case kNfmCeOPL3Duo:
		return NfmOPL::RealChip::create(type, NfmOPL::dtOPL3Duo);
	case kNfmStBusIsaVmeSb:
		return NfmOPL::RealChip::create(type, NfmOPL::dtStBusIsaVmeSb);
	case kNfmNatfeatsNull:
		return NfmOPL::RealChip::create(type, NfmOPL::dtNatfeatsOpl);
	case kNfmNukedOpl3:
		return NfmOPL::EmulatedChip::create(type, NfmOPL::dtNukedOpl3);
#endif

	case kNull:
		return new NullOPL();

	default:
		warning("Unsupported OPL emulator %d", driver);
		return nullptr;
	}
}

void OPL::initDualOpl2OnOpl3(Config::OplType oplType) {
	if (oplType != Config::OplType::kDualOpl2)
		return;

	// Enable OPL3 mode.
	writeReg(0x105, 1);

	// Set panning for channels 0-8 and 9-17 to right and left, respectively.
	for (int i = 0; i <= 0x100; i += 0x100) {
		for (int j = 0xC0; j <= 0xC8; j++) {
			writeReg(i | j, i == 0 ? 0x20 : 0x10);
		}
	}
}

bool OPL::emulateDualOpl2OnOpl3(int r, int v, Config::OplType oplType) {
	if (oplType != Config::OplType::kDualOpl2)
		return true;

	// Prevent writes to the following registers of the second set:
	// - 01 - Test register. Setting any bit here will disable output.
	// - 04 - Connection select. This is used to enable 4 operator instruments,
	//        which are not used for dual OPL2.
	// - 05 - New. Only allow writes which set bit 0 to 1, which enables OPL3
	//        features.
	if (r == 0x101 || r == 0x104 || (r == 0x105 && ((v & 1) == 0)))
		return false;

	// Clear bit 2 of waveform select register writes. This will prevent
	// selection of OPL3-specific waveforms, which are not used for dual OPL2.
	if ((r & 0xFF) >= 0xE0 && (r & 0xFF) <= 0xF5 && ((v & 4) > 0)) {
		writeReg(r, v & ~4);
		return false;
	}

	// Handle rhythm mode register writes.
	if ((r & 0xFF) == 0xBD) {
		// Check if rhythm mode is enabled or disabled.
		bool newRhythmMode = (v & 0x20) > 0;
		if (newRhythmMode != _rhythmMode) {
			_rhythmMode = newRhythmMode;
			// Set panning for channels 6-8 (used by rhythm mode instruments)
			// to center or right if rhythm mode is enabled or disabled,
			// respectively.
			writeReg(0xC6, (_rhythmMode ? 0x30 : 0x20) | _connectionFeedbackValues[0]);
			writeReg(0xC7, (_rhythmMode ? 0x30 : 0x20) | _connectionFeedbackValues[1]);
			writeReg(0xC8, (_rhythmMode ? 0x30 : 0x20) | _connectionFeedbackValues[2]);
		}
		if (r == 0x1BD) {
			// Send writes to the rhythm mode register on the 2nd OPL2 to the
			// single rhythm mode register on the OPL3.
			writeReg(0xBD, v);
			return false;
		}
	}

	// Keep track of the connection and feedback values set for channels 6-8.
	// This is necessary for handling rhythm mode panning (see above).
	if (r >= 0xC6 && r <= 0xC8) {
		_connectionFeedbackValues[r - 0xC6] = v & 0xF;
	}

	// Add panning bits to writes to the connection/feedback registers.
	if ((r & 0xFF) >= 0xC0 && (r & 0xFF) <= 0xC8) {
		// Add right or left panning for the first or second OPL2, respectively.
		int newValue = (r < 0x100 ? 0x20 : 0x10) | (v & 0xF);
		if (_rhythmMode && r >= 0xC6 && r <= 0xC8) {
			// If rhythm mode is enabled, pan channels 6-8 center.
			newValue = 0x30 | (v & 0xF);
		}
		if (v == newValue) {
			// Panning bits are already correct.
			return true;
		} else {
			// Write the new value with the correct panning bits instead.
			writeReg(r, newValue);
			return false;
		}
	}

	// Any other register writes can be processed normally.
	return true;
}

bool OPL::_hasInstance = false;

} // End of namespace OPL
