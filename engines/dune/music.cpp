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

#include "common/config-manager.h"
#include "common/system.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/textconsole.h"

#include "dune/dune.h"
#include "dune/adlib.h"
#include "dune/music.h"

namespace Dune {

// The PC timer calls HERAD at 1193182 / 0x1745 = 200.3 Hz.  The word at +0x32
// is an 8.8 fixed-point divider: the driver subtracts 0x100 per interrupt and
// advances the notation when the accumulator crosses zero.  Thus ARRAKIS's
// 1067 gives 48.1 notation ticks per second, not the fixed 192 previously used.
// HERAD writes 24 ticks to a quarter note and 96 to each repeat measure.
static const uint32 kPitClock = 1193182;
static const uint32 kPitDivisor = 0x1745;

enum {
	kSongHeaderSize = 0x34,
	kEndOfSong = 0xff
};

bool MidiParser_Dune::loadMusic(const byte *data, uint32 size) {
	unloadMusic();

	if (!data || size < kSongHeaderSize)
		return false;

	const uint16 speed = READ_LE_UINT16(data + 0x32);
	if (speed == 0)
		return false;

	const uint16 trackOffset = READ_LE_UINT16(data + 2);
	if (trackOffset + 2u >= size)
		return false;

	_numTracks = 1;
	_numSubtracks[0] = 1;

	// The first track begins after the fixed header (and, for AGD, its Gold
	// register block).  The offset is relative to the word that holds it.
	_tracks[0][0] = data + trackOffset + 2;

	// A delta counts single timer ticks, so a tick is the whole of the
	// parser's resolution.
	_ppqn = 1;

	resetTracking();
	const uint32 tickMicroseconds = (uint32)((uint64)speed * 1000000 * kPitDivisor
										   / (0x100 * (uint64)kPitClock));
	setTempo(tickMicroseconds);
	setTrack(0);

	return true;
}

void MidiParser_Dune::parseNextEvent(EventInfo &info) {
	const byte *playPos = _position._subtracks[0]._playPos;

	info.start = playPos;
	info.length = 0;
	info.loop = false;
	info.noop = false;

	info.delta = readVLQ(playPos);

	if (playPos[0] & 0x80) {
		info.event = *(playPos++);
		_position._subtracks[0]._runningStatus = info.event;
	} else {
		info.event = _position._subtracks[0]._runningStatus;
	}

	switch (info.command()) {
	case 0x9: // note on
		info.basic.param1 = *(playPos++);
		info.basic.param2 = *(playPos++);

		// A note struck with no force at all is the note being let go of.
		if (info.basic.param2 == 0)
			info.event = info.channel() | 0x80;
		break;

	case 0x8: // note off
	case 0xa: // key pressure
	case 0xb: // controller
	case 0xe: // pitch bend
		info.basic.param1 = *(playPos++);
		info.basic.param2 = *(playPos++);
		break;

	case 0xc: // program change
	case 0xd: // channel pressure, which the songs swell with
		info.basic.param1 = *(playPos++);
		info.basic.param2 = 0;
		break;

	default:
		// A song ends with a single 0xFF and nothing after it.
		if (info.event != kEndOfSong)
			warning("Dune: song holds an event of %02x", info.event);

		info.event = 0xff;
		info.ext.type = 0x2f; // end of track
		info.length = 0;
		info.loop = true;
		break;
	}

	_position._subtracks[0]._playPos = playPos;
}

MusicPlayer::MusicPlayer(DuneEngine *vm) : _vm(vm) {
	// The game keeps every song in three arrangements: `.M32` for the MT-32,
	// `.HSQ` for an ordinary AdLib, and `.AGD` for the AdLib Gold.  The Gold
	// arrangement carries the chip's own instruments and uses both OPL3 banks;
	// DNADG.HSQ is the original driver that says how they are written.  Use
	// that native FM arrangement when an AdLib device is selected.
	// Putting the MT-32's note data through the General MIDI patches instead
	// gave the right notes on the wrong instruments altogether.
	const MidiDriver::DeviceHandle device =
		MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32);

	if (MidiDriver::getMusicType(device) == MT_ADLIB) {
		_adlib = new AdLibPlayer();

		if (_adlib->open()) {
			syncVolume();
			return;
		}

		delete _adlib;
		_adlib = nullptr;
	}

	createDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32);

	if (!_driver) {
		warning("Dune: no music device, the game will play without its songs");
		return;
	}

	const int result = _driver->open();
	if (result != 0) {
		warning("Dune: the music device would not open (%d)", result);
		delete _driver;
		_driver = nullptr;
		return;
	}

	debugC(1, kDebugSound, "music device open, %s", _nativeMT32 ? "an MT-32" : "not an MT-32");

	if (_nativeMT32)
		_driver->sendMT32Reset();
	else
		_driver->sendGMReset();

	_driver->setTimerCallback(this, &timerCallback);

	// The player starts out at a master volume of nothing at all, which
	// scales every channel's volume to silence.
	syncVolume();
}

bool MusicPlayer::play(const Common::String &name, bool loop) {
	stop();

	if (_adlib) {
		// A650/05AB retain the ordinary Gold input gain; 066C restores it
		// when a song starts. Host music volume is applied separately.
		if (_vm->_state.size() > 0x28ae)
			_adlib->setDriverVolume(MAX<byte>(4, _vm->_state[0x2896]), _vm->_state[0x28ae]);
		return _adlib->play(_vm->_archive, name, loop);
	}

	if (!_driver)
		return false;

	// The MT-32 songs are the ones that carry plain MIDI; the AdLib pair,
	// .HSQ and .AGD, need the instruments the original drivers set up.
	Resource song = _vm->_archive.openMember(name + ".M32");
	if (!song) {
		warning("Dune: no song called %s", name.c_str());
		return false;
	}

	const uint32 size = song->size();
	byte *data = (byte *)malloc(size);
	song->read(data, size);

	MidiParser_Dune *parser = new MidiParser_Dune();
	if (!parser->loadMusic(data, size)) {
		warning("Dune: %s is not a song after all", name.c_str());
		delete parser;
		free(data);
		return false;
	}

	// The song's own repeat, the same three words the AdLib driver reads out
	// of its half of the pair: 0x418 keeps the voices at the first, puts them
	// back at the second, and does that as many times as the third says.
	// Looping the whole song instead played the opening over and over with
	// it.
	_repeatFrom = READ_LE_UINT16(data + 0x2c);
	_repeatTo = READ_LE_UINT16(data + 0x2e);
	_repeats = (int)READ_LE_UINT16(data + 0x30) - 1;
	_armed = false;

	parser->setTrack(0);
	parser->setMidiDriver(this);
	parser->setTimerRate(_driver->getBaseTempo());
	parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
	parser->property(MidiParser::mpAutoLoop, loop ? 1 : 0);

	Common::StackLock lock(_mutex);
	_parser = parser;
	_midiData = data;
	_isLooping = loop;
	if (_vm->_state.size() > 0x2896)
		_midiVolume.setBase(MAX<byte>(4, _vm->_state[0x2896]));
	_midiVolume.play();
	// 02D2 resets the nine song channels to 60h on each load.
	memset(_channelsVolume, 0x60, 9);
	_isPlaying = true;
	updateMidiVolumes();
	debugC(1, kDebugSound, "MIDI song gain: volume=%u", _midiVolume.current());

	debugC(1, kDebugSound, "playing %s, %u bytes", name.c_str(), size);
	return true;
}

void MusicPlayer::setLooping(bool loop) {
	if (_adlib) {
		_adlib->setLooping(loop);
		return;
	}
	Common::StackLock lock(_mutex);
	_isLooping = loop;
	if (_parser)
		_parser->property(MidiParser::mpAutoLoop, loop ? 1 : 0);
}

void MusicPlayer::stop() {
	// A song stopped outright is not fading any more, and the volume it was
	// being taken down from is given back.
	if (_fadeFor != 0) {
		_fadeFor = 0;
		setVolume(_fadeVolume);
	}

	if (_adlib) {
		_adlib->stop();
		return;
	}

	Audio::MidiPlayer::stop();
	Common::StackLock lock(_mutex);
	_midiVolume.stop();
}

/**
 * A song's own repeat.  0x398 counts its ticks in measures of ninety-six and
 * 0x418 looks at the count: at the first of the three words in the song's head
 * the voices are kept as they stand, and at the second they are put back and
 * the count is taken down by one.  Here there is one parser rather than the
 * AGD's several tracks, so going back is a jump to the tick the first word
 * names.
 */
void MusicPlayer::onTimer() {
	enum { kMeasureTicks = 96 };

	{
		Common::StackLock lock(_mutex);

		if (_parser && _repeatTo != 0 && _repeats > 0) {
			const uint32 measure = _parser->getTick() / kMeasureTicks;

			if (!_armed && measure >= _repeatFrom) {
				_armed = true;
			} else if (_armed && measure >= _repeatTo) {
				_repeats--;
				_parser->jumpToTick(_repeatFrom * kMeasureTicks, false, true);
			}
		}
	}

	Audio::MidiPlayer::onTimer();

	Common::StackLock lock(_mutex);
	// Device callbacks need not arrive at the original PIT rate. Retain the
	// fractional interval and run every original timer tick in order.
	_midiTimerFraction += (uint64)_driver->getBaseTempo() * kPitClock;
	const uint64 interval = (uint64)1000000 * kPitDivisor;
	while (_midiTimerFraction >= interval) {
		_midiTimerFraction -= interval;
		if (!_isPlaying)
			continue;
		const bool wasFading = _midiVolume.fading();
		const bool changed = _midiVolume.tick();
		if (wasFading && !_midiVolume.fading())
			debugC(1, kDebugSound, "MIDI fade settled: volume=%u", _midiVolume.current());
		if (!changed)
			continue;
		updateMidiVolumes();
		if (!_midiVolume.playing()) {
			// DNMID 0523 clears playback without discarding the loaded song.
			_isPlaying = false;
			break;
		}
	}
}

bool MusicPlayer::playing() const {
	return _adlib ? _adlib->playing() : (_isPlaying && _parser != nullptr);
}

void MusicPlayer::setVolume(int volume) {
	if (_adlib) {
		_adlib->setVolume(volume);
		return;
	}

	Common::StackLock lock(_mutex);
	_masterVolume = CLIP(volume, 0, 255);
	updateMidiVolumes();
}

void MusicPlayer::updateMidiVolumes() {
	if (!_driver)
		return;
	// DNMID 0509 writes B0..B9 directly. Allocating channels here could
	// reserve a currently unused MT-32 part before its first song event.
	for (uint ch = 0; ch < 10; ++ch) {
		const byte volume = _midiVolume.channel(_channelsVolume[ch]) * _masterVolume / 255;
		_driver->send(0x07b0 | ch | (uint32)volume << 16);
	}
}

void MusicPlayer::fadeDriverVolume(uint16 duration, byte volume, byte pan) {
	if (_adlib) {
		_adlib->fadeDriverVolume(duration, volume, pan);
		return;
	}
	Common::StackLock lock(_mutex);
	_midiVolume.request(duration, volume);
	debugC(1, kDebugSound, "MIDI fade request: duration=%u volume=%u", duration, volume);
}

int MusicPlayer::volume() const {
	return _adlib ? _adlib->getVolume() : getVolume();
}

/**
 * 0xADBE takes the song that is playing down to nothing rather than cutting
 * it off, and only once it has gone does 0xAD37 put the next one on.  Without
 * it every change of place snapped one song into the next.
 */
void MusicPlayer::fadeOut(uint32 overMs) {
	if (!playing() || _fadeFor != 0)
		return;

	_fadeVolume = volume();
	_fadeFrom = g_system->getMillis();
	_fadeFor = overMs != 0 ? overMs : 1;
}

/**
 * Carries the fade along, and stops the song once it has gone.
 */
void MusicPlayer::fade() {
	if (_fadeFor == 0)
		return;

	const uint32 gone = g_system->getMillis() - _fadeFrom;

	if (gone >= _fadeFor) {
		const int was = _fadeVolume;

		_fadeFor = 0;
		stop();
		setVolume(was);
		return;
	}

	setVolume((int)(_fadeVolume * (_fadeFor - gone) / _fadeFor));
}

/**
 * The songs are written for the MT-32's own bank of sounds, and their program
 * changes name that bank's numbers.  On an MT-32 they are passed through as
 * they stand; on any other device the numbers are put across the table the
 * drivers keep of the MT-32's names to the General MIDI ones, which is what
 * stands the right instrument up for every part.  Without it the numbers were
 * read as General MIDI's own, and the palace's song played its strings with
 * timpani, its horns with a xylophone.
 */
void MusicPlayer::send(uint32 b) {
	if ((b & 0xfff0) == 0x07b0) {
		const byte ch = b & 15;
		_channelsVolume[ch] = (b >> 16) & 127;
		const byte volume = _midiVolume.channel(_channelsVolume[ch]) * _masterVolume / 255;
		if (_driver)
			_driver->send((b & 0xff00ffff) | (uint32)volume << 16);
		return;
	}
	if (!_nativeMT32 && (b & 0xF0) == 0xC0) {
		const byte program = (byte)((b >> 8) & 0xff);
		b = (b & 0xffff00ff) | (uint32)MidiDriver::_mt32ToGm[program] << 8;
	}

	// DNMID 05A6 preserves each song's channel number, including percussion.
	if (_driver)
		_driver->send(b);
}

} // End of namespace Dune
