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

#include "audio/midiparser.h"
#include "audio/miles.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/textconsole.h"

#include "eem/detection.h"
#include "eem/music.h"

namespace EEM {

const int kMidiDriverFlags = MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32;

MusicPlayer::MusicPlayer(bool isFloppy) : _isFloppy(isFloppy) {
	// _InitMIDI @ 20a2:013a — `_AIL_register_driver` against
	// ADLIB.ADV / SBFM.ADV / MT32MPU.ADV. We honour the launcher's
	// "Music driver" setting and prefer MT-32 when unset.
	const MidiDriver::DeviceHandle dev =
		MidiDriver::detectDevice(kMidiDriverFlags);
	MusicType musicType = MidiDriver::getMusicType(dev);
	if (musicType == MT_GM && ConfMan.getBool("native_mt32"))
		musicType = MT_MT32;

	switch (musicType) {
	case MT_ADLIB:
		// _MIDIPlayFile @ 20a2:024c opens SAMPLE.AD (string at 29be:14d6)
		// and installs every patch the sequence requests via
		// `_AIL_install_timbre`.
		_milesAudioMode = true;
		_driver = Audio::MidiDriver_Miles_AdLib_create(
			Common::Path("SAMPLE.AD"), Common::Path());
		break;
	case MT_MT32:
		// MT32MPU.ADV in the original. No Miles MT-32 bank ships with
		// EEM, so use the standard MT-32 driver.
		_milesAudioMode = true;
		_driver = Audio::MidiDriver_Miles_MT32_create(Common::Path());
		break;
	default:
		_milesAudioMode = false;
		createDriver(kMidiDriverFlags);
		break;
	}

	if (_driver) {
		const int ret = _driver->open();
		if (ret != 0) {
			warning("MusicPlayer: MidiDriver::open() failed (%d)", ret);
			delete _driver;
			_driver = nullptr;
		} else {
			// Miles AdLib handles its own reset.
			if (musicType != MT_ADLIB) {
				if (musicType == MT_MT32 || _nativeMT32)
					_driver->sendMT32Reset();
				else
					_driver->sendGMReset();
			}
			_driver->setTimerCallback(this, &timerCallback);
		}
	} else {
		debugC(1, kDebugSound, "MusicPlayer: no MIDI driver — music disabled");
	}
}

void MusicPlayer::send(uint32 b) {
	// Miles drivers (both AdLib and MT-32) implement their own per-
	// source-channel mixing and timbre installation, so forward the raw
	// event.
	if (_milesAudioMode) {
		_driver->send(b);
		return;
	}
	Audio::MidiPlayer::send(b);
}

void MusicPlayer::playFile(const Common::Path &xmiPath, bool loop) {
	if (!_driver)
		return;

	Common::StackLock lock(_mutex);
	stop();


	Common::File f;
	if (!f.open(xmiPath)) {
		warning("MusicPlayer: %s missing", xmiPath.toString().c_str());
		return;
	}
	const uint32 size = f.size();
	if (size == 0) {
		warning("MusicPlayer: %s is empty", xmiPath.toString().c_str());
		return;
	}
	_xmiData.resize(size);
	if (f.read(_xmiData.data(), size) != size) {
		warning("MusicPlayer: short read on %s",
				xmiPath.toString().c_str());
		_xmiData.clear();
		return;
	}

	_parser = MidiParser::createParser_XMIDI(nullptr, nullptr, 0);
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());
	_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
	_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);

	if (!_parser->loadMusic(_xmiData.data(), _xmiData.size())) {
		warning("MusicPlayer: XMIDI parser rejected %s",
				xmiPath.toString().c_str());
		delete _parser;
		_parser = nullptr;
		_xmiData.clear();
		return;
	}

	_isLooping = loop;
	_parser->property(MidiParser::mpAutoLoop, loop ? 1 : 0);
	_parser->setTrack(0);

	syncVolume();
	_isPlaying = true;
	debugC(1, kDebugSound, "MusicPlayer: playing %s (%u bytes, loop=%d, miles=%d)",
		   xmiPath.toString().c_str(), size, loop, _milesAudioMode);
}

void MusicPlayer::playMus(uint num, bool loop) {
	// CD format string "mus%05d.xmi" at 29be:1525. Floppy maps the same
	// numeric slots to different filenames:
	//   0..4 → travel music. Table at 2608:1399-13cd holds 5 entries
	//          (Travel-6, Travel-4, Travel-7, Travel-1, Travel-8) used by
	//          `_StartTravelMusic` via `siteNumber % 5`.
	//   5    → FANFARE2.XMI (winner).
	//   6    → no equivalent on floppy (loser sting in `_DisplayAlibi`
	//          is CD-only); skip.
	if (_isFloppy) {
		static const char *const kTravelTracks[5] = {
			"Travel-6.XMI", "Travel-4.XMI", "Travel-7.XMI",
			"Travel-1.XMI", "Travel-8.XMI",
		};
		Common::String name;
		if (num < 5) {
			name = kTravelTracks[num];
		} else if (num == 5) {
			name = "FANFARE2.XMI";
		} else {
			return; // num == 6 (loser sting): not present on floppy
		}
		playFile(Common::Path(name), loop);
		return;
	}
	const Common::String name = Common::String::format("MUS%05u.XMI", num);
	playFile(Common::Path(name), loop);
}

} // End of namespace EEM
