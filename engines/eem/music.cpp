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
#include "common/endian.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "eem/detection.h"
#include "eem/music.h"
#include "eem/resource.h"

namespace EEM {

const int kMidiDriverFlags = MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32;
const int kMacMidiDriverFlags = MDT_MIDI | MDT_PREFER_GM;
const uint16 kInvalidMacMidiResource = 0xffff;
const uint16 kInvalidMacSongResource = 0xffff;
const byte kNoMacInstrument = 0xff;

Common::String musicNameFromPath(const Common::Path &path) {
	Common::String name = path.baseName();
	const size_t dot = name.findLastOf('.');
	if (dot != Common::String::npos)
		name = name.substr(0, dot);
	name.toUppercase();
	return name;
}

Common::SeekableReadStream *openMacMidiResource(uint16 resourceId) {
	Common::SeekableReadStream *stream =
		openMacResource(Common::Path("EEM Sound&Music"),
						MKTAG('c', 'm', 'i', 'd'), resourceId);
	if (stream)
		return stream;

	stream = openMacResource(Common::Path("EEM Sound&Music"),
							 MKTAG('M', 'I', 'D', 'I'), resourceId);
	if (stream)
		return stream;

	return openMacResource(Common::Path("EEM Sound&Music"),
						   MKTAG('M', 'i', 'd', 'i'), resourceId);
}

uint16 macSongResourceIdForFile(const Common::Path &path) {
	const Common::String name = musicNameFromPath(path);
	if (name == "THEME" || name == "THEME SONG")
		return 1000;
	if (name == "WIN1" || name == "FANFARE2")
		return 1001;
	if (name == "TRAVEL-1")
		return 1006;
	if (name == "TRAVEL-4")
		return 1002;
	if (name == "TRAVEL-6")
		return 1004;
	if (name == "TRAVEL-7")
		return 1003;
	if (name == "TRAVEL-8")
		return 1005;
	if (name == "WRONG2")
		return 1007;
	return kInvalidMacSongResource;
}

uint16 macSongResourceIdForMus(uint num) {
	static const uint16 kTravelTracks[5] = {
		1004, 1002, 1003, 1006, 1005 // Travel-6/4/7/1/8
	};
	if (num < ARRAYSIZE(kTravelTracks))
		return kTravelTracks[num];
	if (num == 5)
		return 1001; // Win1
	if (num == 6)
		return 1007; // Wrong2
	return kInvalidMacSongResource;
}

MusicPlayer::MusicPlayer(bool isFloppy, bool isMacintosh) :
	_isFloppy(isFloppy), _isMacintosh(isMacintosh) {
	clearMacInstrumentMap();

	// _InitMIDI @ 20a2:013a — `_AIL_register_driver` against
	// ADLIB.ADV / SBFM.ADV / MT32MPU.ADV. We honour the launcher's
	// "Music driver" setting and prefer MT-32 when unset.
	const MidiDriver::DeviceHandle dev =
		MidiDriver::detectDevice(_isMacintosh ? kMacMidiDriverFlags
											  : kMidiDriverFlags);
	MusicType musicType = MidiDriver::getMusicType(dev);
	if (!_isMacintosh && musicType == MT_GM &&
		ConfMan.getBool("native_mt32"))
		musicType = MT_MT32;

	if (_isMacintosh) {
		_milesAudioMode = false;
		createDriver(kMacMidiDriverFlags);
	} else {
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
	}

	if (_driver) {
		const int ret = _driver->open();
		if (ret != 0) {
			warning("MusicPlayer: MidiDriver::open() failed (%d)", ret);
			delete _driver;
			_driver = nullptr;
		} else {
			// Miles AdLib handles its own reset.
			if (_isMacintosh) {
				_driver->sendGMReset();
			} else if (musicType != MT_ADLIB) {
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
	if (_isMacintosh && (b & 0xF0) == 0xC0) {
		const byte channel = (byte)(b & 0x0F);
		const byte rawProgram = (byte)((b >> 8) & 0x7F);
		const byte inst = _macChannelInstrument[channel] != kNoMacInstrument
			? _macChannelInstrument[channel] : rawProgram;
		const byte gmProgram = mapMacInstrumentToGM(inst, channel);
		b = (b & 0xFFFF00FF) | ((uint32)gmProgram << 8);
	}

	// Miles drivers (both AdLib and MT-32) implement their own per-
	// source-channel mixing and timbre installation, so forward the raw
	// event.
	if (_milesAudioMode) {
		_driver->send(b);
		return;
	}
	Audio::MidiPlayer::send(b);
}

void MusicPlayer::startLoadedMusic(const Common::String &name, bool loop,
								   bool smf) {
	_parser = smf ? MidiParser::createParser_SMF()
				  : MidiParser::createParser_XMIDI(nullptr, nullptr, 0);
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());
	_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
	_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);

	if (!_parser->loadMusic(_xmiData.data(), _xmiData.size())) {
		warning("MusicPlayer: %s parser rejected %s",
				smf ? "SMF" : "XMIDI", name.c_str());
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
	debugC(1, kDebugSound,
		   "MusicPlayer: playing %s (%u bytes, loop=%d, miles=%d, smf=%d)",
		   name.c_str(), _xmiData.size(), loop, _milesAudioMode, smf);
}

void MusicPlayer::clearMacInstrumentMap() {
	memset(_macChannelInstrument, kNoMacInstrument,
		   sizeof(_macChannelInstrument));
}

byte MusicPlayer::mapMacInstrumentToGM(byte inst, byte channel) const {
	if (channel == 9)
		return 0;

	switch (inst) {
	case 2:
	case 3:
		return 0;   // Piano
	case 11:
		return 16;  // Organ
	case 60:
		return 24;  // Guitar
	case 64:
	case 65:
		return 33;  // Bass
	case 73:
	case 74:
	case 75:
		return 73;  // Flute
	case 83:
	case 84:
		return 71;  // Clarinet
	default:
		return inst < 128 ? inst : 0;
	}
}

bool MusicPlayer::loadMacSong(uint16 resourceId, uint16 &midiId) {
	midiId = kInvalidMacMidiResource;
	clearMacInstrumentMap();

	Common::SeekableReadStream *stream =
		openMacResource(Common::Path("EEM Sound&Music"),
						MKTAG('S', 'O', 'N', 'G'), resourceId);
	if (!stream) {
		warning("MusicPlayer: Mac SONG resource %u missing", resourceId);
		return false;
	}

	const uint32 size = (uint32)stream->size();
	if (size < 18) {
		delete stream;
		warning("MusicPlayer: Mac SONG resource %u too short", resourceId);
		return false;
	}

	Common::Array<byte> song;
	song.resize(size);
	if (stream->read(song.data(), size) != size) {
		delete stream;
		warning("MusicPlayer: short read on Mac SONG resource %u", resourceId);
		return false;
	}
	delete stream;

	midiId = READ_BE_UINT16(song.data());
	const uint16 instCount = READ_BE_UINT16(song.data() + 16);
	uint32 pos = 18;
	for (uint i = 0; i < instCount && pos + 4 <= size; i++, pos += 4) {
		const uint16 channel = READ_BE_UINT16(song.data() + pos);
		const uint16 inst = READ_BE_UINT16(song.data() + pos + 2);
		if (channel < ARRAYSIZE(_macChannelInstrument) && inst < 128)
			_macChannelInstrument[channel] = (byte)inst;
	}

	return true;
}

void MusicPlayer::playMacSongResource(uint16 resourceId, bool loop) {
	if (resourceId == kInvalidMacSongResource)
		return;

	uint16 midiId = kInvalidMacMidiResource;
	if (!loadMacSong(resourceId, midiId))
		return;

	playMacMidiResource(midiId, loop);
}

void MusicPlayer::playMacMidiResource(uint16 resourceId, bool loop) {
	if (resourceId == kInvalidMacMidiResource)
		return;

	Common::SeekableReadStream *stream = openMacMidiResource(resourceId);
	if (!stream) {
		warning("MusicPlayer: Mac Midi resource %u missing", resourceId);
		return;
	}

	const uint32 size = (uint32)stream->size();
	if (size == 0) {
		delete stream;
		warning("MusicPlayer: Mac Midi resource %u is empty", resourceId);
		return;
	}
	_xmiData.resize(size);
	if (stream->read(_xmiData.data(), size) != size) {
		delete stream;
		_xmiData.clear();
		warning("MusicPlayer: short read on Mac Midi resource %u", resourceId);
		return;
	}
	delete stream;

	startLoadedMusic(Common::String::format("Mac Midi %u", resourceId), loop,
					 /* smf= */ true);
}

void MusicPlayer::playFile(const Common::Path &xmiPath, bool loop) {
	if (!_driver)
		return;

	Common::StackLock lock(_mutex);
	stop();

	if (_isMacintosh) {
		const uint16 resourceId = macSongResourceIdForFile(xmiPath);
		if (resourceId == kInvalidMacSongResource) {
			warning("MusicPlayer: no Mac SONG mapping for %s",
					xmiPath.toString().c_str());
			return;
		}
		playMacSongResource(resourceId, loop);
		return;
	}

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

	startLoadedMusic(xmiPath.toString(), loop, /* smf= */ false);
}

void MusicPlayer::playMus(uint num, bool loop) {
	if (_isMacintosh) {
		Common::StackLock lock(_mutex);
		stop();
		playMacSongResource(macSongResourceIdForMus(num), loop);
		return;
	}

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
