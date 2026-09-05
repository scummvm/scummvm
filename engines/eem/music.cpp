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
#include "audio/mixer.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "eem/detection.h"
#include "eem/music.h"
#include "eem/resource.h"

namespace EEM {

const int kMidiDriverFlags = MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32;
const uint16 kInvalidMacSongResource = 0xffff;

Common::String musicNameFromPath(const Common::Path &path) {
	Common::String name = path.baseName();
	const size_t dot = name.findLastOf('.');
	if (dot != Common::String::npos)
		name = name.substr(0, dot);
	name.toUppercase();
	return name;
}

static bool copyMacMidiVLQ(Common::SeekableReadStream &stream, int64 end,
						  Common::WriteStream &output, uint32 &value) {
	value = 0;
	for (uint i = 0; i < 4 && stream.pos() < end; ++i) {
		const byte b = stream.readByte();
		output.writeByte(b);
		value = (value << 7) | (b & 0x7f);
		if (!(b & 0x80))
			return true;
	}
	return false;
}

static Common::SeekableReadStream *expandMacMidiRunningStatus(Common::SeekableReadStream &stream) {
	if (stream.size() < 14 || stream.readUint32BE() != MKTAG('M', 'T', 'h', 'd'))
		return nullptr;
	stream.seek(0);

	// Halestorm shares running status between tracks. Expand it here so each
	// EEM MIDI event carries its own command and channel, including note-offs.
	Common::MemoryWriteStreamDynamic output(DisposeAfterUse::YES);
	while (stream.size() - stream.pos() >= 8) {
		const uint32 tag = stream.readUint32BE();
		const uint32 size = stream.readUint32BE();
		if (size > stream.size() - stream.pos())
			return nullptr;
		const int64 end = stream.pos() + size;
		output.writeUint32BE(tag);
		const uint32 sizeOffset = output.pos();
		output.writeUint32BE(size);

		if (tag != MKTAG('M', 'T', 'r', 'k')) {
			if (output.writeStream(&stream, size) != size)
				return nullptr;
			continue;
		}

		byte runningStatus = 0;
		while (stream.pos() < end) {
			uint32 delta;
			if (!copyMacMidiVLQ(stream, end, output, delta) || stream.pos() == end)
				return nullptr;

			byte status = stream.readByte();
			if (status < 0x80) {
				if (!runningStatus)
					return nullptr;
				stream.seek(-1, SEEK_CUR);
				status = runningStatus;
			}
			output.writeByte(status);

			uint32 dataSize;
			if (status < 0xf0) {
				runningStatus = status;
				dataSize = ((status & 0xf0) == 0xc0 || (status & 0xf0) == 0xd0) ? 1 : 2;
			} else if (status == 0xff) {
				if (stream.pos() == end)
					return nullptr;
				output.writeByte(stream.readByte());
				if (!copyMacMidiVLQ(stream, end, output, dataSize))
					return nullptr;
			} else if (status == 0xf0 || status == 0xf7) {
				runningStatus = 0;
				if (!copyMacMidiVLQ(stream, end, output, dataSize))
					return nullptr;
			} else {
				return nullptr;
			}

			if (dataSize > end - stream.pos() || output.writeStream(&stream, dataSize) != dataSize)
				return nullptr;
		}
		WRITE_BE_UINT32(output.getData() + sizeOffset, output.size() - sizeOffset - 4);
	}
	if (stream.pos() != stream.size() || stream.err())
		return nullptr;

	Common::MemoryReadStream expanded(output.getData(), output.size());
	return expanded.readStream(expanded.size());
}

Common::SeekableReadStream *MusicPlayer::getResource(uint16 id, uint32 type) {
	static const char *const kMacMusicForks[] = {
		"EEM Sound&Music",
		"rsrc/EEM Sound&Music",
		"EEM London CD",
		"rsrc/EEM London CD",
	};
	static const uint32 kMacMidiTypes[] = {
		MKTAG('c', 'm', 'i', 'd'),
		MKTAG('M', 'I', 'D', 'I'),
		MKTAG('M', 'i', 'd', 'i'),
	};

	const uint firstFork = _isLondon ? 2 : 0;
	for (uint i = firstFork; i < firstFork + 2; i++) {
		const Common::Path path(kMacMusicForks[i]);
		if (type == MKTAG('M', 'I', 'D', 'I') || type == MKTAG('M', 'i', 'd', 'i')) {
			for (uint j = 0; j < ARRAYSIZE(kMacMidiTypes); j++) {
				Common::SeekableReadStream *stream = openMacResource(path, kMacMidiTypes[j], id);
				if (stream) {
					Common::SeekableReadStream *expanded = expandMacMidiRunningStatus(*stream);
					delete stream;
					if (!expanded)
						warning("MusicPlayer: invalid Mac MIDI resource %u", id);
					return expanded;
				}
			}
			continue;
		} else if (type == MKTAG('s', 'n', 'd', ' ')) {
			// London stores most instrument samples as delta-compressed csnd.
			Common::SeekableReadStream *packed = openMacResource(path, MKTAG('c', 's', 'n', 'd'), id);
			if (packed) {
				Common::SeekableReadStream *stream = decompressMacSound(*packed);
				delete packed;
				return stream;
			}
		}

		Common::SeekableReadStream *stream = openMacResource(path, type, id);
		if (stream)
			return stream;
	}

	return nullptr;
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
	if (name == "MUS00101")
		return 1101;
	if (name == "MUS00102")
		return 1102;
	if (name == "MUS00103")
		return 1103;
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

MusicPlayer::MusicPlayer(bool isFloppy, bool isMacintosh, bool isLondon) :
	_isFloppy(isFloppy), _isMacintosh(isMacintosh), _isLondon(isLondon) {
	if (_isMacintosh)
		return;

	// _InitMIDI @ 20a2:013a — `_AIL_register_driver` against
	// ADLIB.ADV / SBFM.ADV / MT32MPU.ADV. We honour the launcher's
	// "Music driver" setting and prefer MT-32 when unset.
	const MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(kMidiDriverFlags);
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

MusicPlayer::~MusicPlayer() {
	stop();
}

void MusicPlayer::stop() {
	if (_isMacintosh) {
		// Halestorm may already report the song as finished and ignore abort.
		// Releasing EEM's driver also stops any remaining sample voices.
		delete _macDriver;
		_macDriver = nullptr;
	} else {
		Audio::MidiPlayer::stop();
	}
}

bool MusicPlayer::isPlaying() const {
	if (_isMacintosh)
		return _macDriver && _macDriver->doCommand(Audio::HalestormDriver::kSongIsPlaying);
	return Audio::MidiPlayer::isPlaying();
}

void MusicPlayer::setVolume(int volume) {
	if (_isMacintosh) {
		_masterVolume = CLIP<int>(volume, 0, Audio::Mixer::kMaxMixerVolume);
		if (_macDriver)
			_macDriver->setMusicVolume(_masterVolume);
	} else {
		Audio::MidiPlayer::setVolume(volume);
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

void MusicPlayer::startLoadedMusic(const Common::String &name, bool loop) {
	_parser = MidiParser::createParser_XMIDI(nullptr, nullptr, 0);
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());
	_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
	_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);

	if (!_parser->loadMusic(_xmiData.data(), _xmiData.size())) {
		warning("MusicPlayer: XMIDI parser rejected %s", name.c_str());
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
		   "MusicPlayer: playing %s (%u bytes, loop=%d, miles=%d)",
		   name.c_str(), _xmiData.size(), loop, _milesAudioMode);
}

void MusicPlayer::playMacSongResource(uint16 resourceId, bool loop) {
	stop();
	if (resourceId == kInvalidMacSongResource)
		return;

	_macDriver = new Audio::HalestormDriver(this, g_system->getMixer());
	if (!_macDriver->init(true, Audio::HalestormDriver::kSimple, 0, false)) {
		warning("MusicPlayer: Halestorm initialization failed");
		stop();
		return;
	}

	syncVolume();
	const int command = loop ? Audio::HalestormDriver::kSongPlayLoop : Audio::HalestormDriver::kSongPlayOnce;
	const int result = _macDriver->doCommand(command, resourceId);
	if (result) {
		warning("MusicPlayer: failed to play Mac SONG resource %u (%d)", resourceId, result);
		stop();
		return;
	}

	debugC(1, kDebugSound, "MusicPlayer: playing Mac SONG %u (loop=%d)", resourceId, loop);
}

void MusicPlayer::playFile(const Common::Path &xmiPath, bool loop) {
	if (_isMacintosh) {
		const uint16 resourceId = macSongResourceIdForFile(xmiPath);
		if (resourceId == kInvalidMacSongResource) {
			stop();
			warning("MusicPlayer: no Mac SONG mapping for %s",
					xmiPath.toString().c_str());
			return;
		}
		playMacSongResource(resourceId, loop);
		return;
	}

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

	startLoadedMusic(xmiPath.toString(), loop);
}

void MusicPlayer::playMus(uint num, bool loop) {
	if (_isMacintosh) {
		// London SONG ids are 1000 + the MUS number; EEM1 uses named tracks.
		const uint16 resourceId = _isLondon
			? (num < kInvalidMacSongResource - 1000 ? 1000 + num : kInvalidMacSongResource)
			: macSongResourceIdForMus(num);
		playMacSongResource(resourceId, loop);
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
