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

#include "common/debug.h"
#include "common/file.h"
#include "common/textconsole.h"

#include "eem/detection.h"
#include "eem/music.h"

namespace EEM {

MusicPlayer::MusicPlayer() {
	// Mirrors `_InitMIDI @ 20a2:013a` which used `_AIL_register_driver`
	// to walk the .ADV files (ADLIB.ADV, SBFM.ADV, MT32MPU.ADV, etc.)
	// and pick a backend. We honour the launcher's "Music driver"
	// setting and only force AdLib / MT-32 paths through Miles when the
	// detected device matches.
	const MidiDriver::DeviceHandle dev =
		MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB);
	const MusicType musicType = MidiDriver::getMusicType(dev);

	switch (musicType) {
	case MT_ADLIB:
		// `_MIDIPlayFile` (20a2:024c) opens "SAMPLE.AD" (29be:14d6) and
		// installs every patch the sequence requests via
		// `_AIL_install_timbre`. ScummVM's Miles AdLib driver does the
		// same: it reads SAMPLE.AD on construction and serves timbres
		// out of that bank, which is what makes the notes match the
		// 1993 release. SAMPLE.OPL would be the OPL3 variant; the game
		// only ships SAMPLE.AD, so an empty path falls back to OPL2.
		_milesAudioMode = true;
		_driver = Audio::MidiDriver_Miles_AdLib_create(
			Common::Path("SAMPLE.AD"), Common::Path());
		break;
	case MT_MT32:
		// `MT32MPU.ADV` was the original MT-32 driver; ScummVM has no
		// Miles MT-32 instrument bank for EEM, so we use the standard
		// MT-32 driver and let the XMIDI's own program changes drive
		// the patch selection.
		_milesAudioMode = true;
		_driver = Audio::MidiDriver_Miles_MT32_create(Common::Path());
		break;
	default:
		_milesAudioMode = false;
		createDriver(MDT_MIDI | MDT_ADLIB);
		break;
	}

	if (_driver) {
		const int ret = _driver->open();
		if (ret != 0) {
			warning("MusicPlayer: MidiDriver::open() failed (%d)", ret);
			delete _driver;
			_driver = nullptr;
		} else {
			// No GM/MT-32 reset for AdLib (Miles AdLib handles its own
			// state); for MT-32/GM the original would've sent its own
			// initialisation patches via `_AIL_install_timbre`, but we
			// don't have those banks for non-AdLib devices.
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
	// source-channel mixing and timbre installation, so just forward
	// the raw event. Going through `MidiPlayer::send` would re-wrap
	// CC 7 against `_masterVolume` AND remap the source channel via
	// `sendToChannel`/`allocateChannel`, both of which the Miles driver
	// already handles internally (and break the timbre selection if
	// double-applied).
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

	// Mirrors `_MIDIPlayFile`'s `_fopen` + `_fread` (20a2:024c-029e).
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

	// Mirrors `_LoopMIDI = 0xFFFF` (the count register the original
	// engine uses for indefinite looping in `_DoOpeningAnims`).
	_isLooping = loop;
	_parser->property(MidiParser::mpAutoLoop, loop ? 1 : 0);
	_parser->setTrack(0);

	// Pull the launcher's music_volume slider into `_masterVolume` so
	// the non-Miles `Audio::MidiPlayer::send` path scales correctly.
	// (Miles drivers do their own volume handling on the Multisource
	// path, but they also honour `MidiDriver::syncSoundSettings` which
	// `Engine::syncSoundSettings` triggers.)
	syncVolume();
	_isPlaying = true;
	debugC(1, kDebugSound, "MusicPlayer: playing %s (%u bytes, loop=%d, miles=%d)",
		   xmiPath.toString().c_str(), size, loop, _milesAudioMode);
}

void MusicPlayer::playMus(uint num, bool loop) {
	// Format string verified at `29be:1525` ("mus%05d.xmi").
	const Common::String name = Common::String::format("MUS%05u.XMI", num);
	playFile(Common::Path(name), loop);
}

} // End of namespace EEM
