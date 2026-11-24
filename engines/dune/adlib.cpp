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

#include "common/debug.h"
#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "dune/adlib.h"
#include "dune/archive.h"
#include "dune/dune.h"

namespace Dune {

// The game calls its music driver from the 200.3 Hz timer interrupt.  The
// word at +0x32 of every HERAD song is an 8.8 fixed-point divider for that
// clock; it is not itself a tempo in ticks per second.
static const int kDriverTicksASecond = 200;

// HERAD has 24 ticks to a quarter note and counts its repeat points in
// four-quarter measures of ninety-six ticks.
static const uint32 kMeasureTicks = 96;

// Which of the chip's eighteen operators the two of a channel are.
static const int kFirstOperator[9] = { 0, 1, 2, 8, 9, 10, 16, 17, 18 };

// The twelve notes of an octave, as the chip counts frequency: the driver
// works its own table out as it starts, and these are the numbers that come
// of it - a note is the chip's clock over 2^(20 - block) times its frequency.
static const uint16 kNoteFrequency[12] = {
	343, 364, 385, 408, 433, 459,
	486, 515, 546, 579, 614, 650
};

static const byte kFineBend[13] = {
	19, 21, 21, 23, 25, 26, 27, 29, 31, 33, 35, 36, 37
};

static const byte kCoarseBend[10] = {
	0, 5, 10, 15, 20, 0, 6, 12, 18, 24
};

// A HERAD instrument is forty bytes.  Its two operators are not two identical
// thirteen-byte records: the carrier begins at +0x0f and +0x11 is its OPL3
// panning byte, while the macros follow both operators from +0x1b onward.
enum {
	kInstrumentSize = 40,
	kModKeyScale = 0x02, kModMultiple = 0x03, kFeedback = 0x04,
	kModAttack = 0x05, kModSustain = 0x06, kModEnvelope = 0x07,
	kModDecay = 0x08, kModRelease = 0x09, kModLevel = 0x0a,
	kModTremolo = 0x0b, kModVibrato = 0x0c, kModKeyScaleRate = 0x0d,
	kJoined = 0x0e,
	kCarKeyScale = 0x0f, kCarMultiple = 0x10, kPanning = 0x11,
	kCarAttack = 0x12, kCarSustain = 0x13, kCarEnvelope = 0x14,
	kCarDecay = 0x15, kCarRelease = 0x16, kCarLevel = 0x17,
	kCarTremolo = 0x18, kCarVibrato = 0x19, kCarKeyScaleRate = 0x1a,
	kFeedbackAftertouch = 0x1b, kModWave = 0x1c, kCarWave = 0x1d,
	kModVelocity = 0x1e, kCarVelocity = 0x1f, kFeedbackVelocity = 0x20,
	kCoarseSlide = 0x21, kTranspose = 0x22, kSlideDuration = 0x23,
	kSlideRange = 0x24, kModAftertouch = 0x26, kCarAftertouch = 0x27,
	kSongSpeed = 0x32, kFirstTrack = 0x02
};

AdLibPlayer::AdLibPlayer() {
}

AdLibPlayer::~AdLibPlayer() {
	stop();

	if (_opl) {
		_opl->stop();
		delete _opl;
		_opl = nullptr;
	}
}

bool AdLibPlayer::open() {
	// Dune keeps the AdLib Gold (AGD) arrangement: up to eighteen channels,
	// panning and all eight OPL3 waveforms.  Treating it as an OPL2 song threw
	// away every track after the ninth and masked its instruments' waveforms.
	_opl = OPL::Config::create(OPL::Config::kOpl3);

	if (!_opl || !_opl->init()) {
		warning("Dune: the AdLib would not open");
		delete _opl;
		_opl = nullptr;
		return false;
	}

	_hasOutputGain = _opl->setOutputGain(1, 1, 1);

	// The chip is put in its ordinary HERAD state: OPL3 on, four-operator mode
	// off, no rhythm, note select on, and all the waveforms available.
	writeReg(0x105, 0x01);
	writeReg(0x104, 0x00);
	writeReg(0x01, 0x20);
	writeReg(0x08, 0x40);
	writeReg(0xbd, 0x00);

	silence();

	_opl->start(new Common::Functor0Mem<void, AdLibPlayer>(this, &AdLibPlayer::tick),
				kDriverTicksASecond);

	return true;
}

void AdLibPlayer::writeReg(int reg, int value, uint channel) {
	if (_opl)
		_opl->writeReg(reg | (channel >= 9 ? 0x100 : 0), value);
}

void AdLibPlayer::silence() {
	for (uint i = 0; i != kMaxVoices; i++) {
		writeReg(0xa0 + i % 9, 0, i);
		writeReg(0xb0 + i % 9, 0, i);
	}

	for (uint channel = 0; channel != kMaxVoices; channel++) {
		for (int half = 0; half != 2; half++) {
			const int op = kFirstOperator[channel % 9] + half * 3;

			writeReg(0x40 + op, 0x3f, channel);
			writeReg(0x60 + op, 0xff, channel);
			writeReg(0x80 + op, 0x0f, channel);
		}
	}
}

/**
 * 0x949 and what follows it write an instrument into the chip's registers.
 * The two operators are laid out the same way, and only the first carries how
 * far it feeds back into itself and whether the two are joined.
 */
void AdLibPlayer::loadInstrument(uint channel, uint which) {
	if (channel >= _voiceCount || which >= _instruments)
		return;

	const byte *at = _song.begin() + _bank + which * kInstrumentSize;

	static const byte kKeyScale[2] = { kModKeyScale, kCarKeyScale };
	static const byte kMultiple[2] = { kModMultiple, kCarMultiple };
	static const byte kAttack[2] = { kModAttack, kCarAttack };
	static const byte kSustain[2] = { kModSustain, kCarSustain };
	static const byte kEnvelope[2] = { kModEnvelope, kCarEnvelope };
	static const byte kDecay[2] = { kModDecay, kCarDecay };
	static const byte kRelease[2] = { kModRelease, kCarRelease };
	static const byte kLevel[2] = { kModLevel, kCarLevel };
	static const byte kTremolo[2] = { kModTremolo, kCarTremolo };
	static const byte kVibrato[2] = { kModVibrato, kCarVibrato };
	static const byte kKeyScaleRate[2] = { kModKeyScaleRate, kCarKeyScaleRate };
	static const byte kWave[2] = { kModWave, kCarWave };
	const int turned = _hasOutputGain ? 0 : (255 - _volume) >> 2;
	const int quieter = MIN(turned * turned / 0x3f, 0x3f);

	for (int half = 0; half != 2; half++) {
		const int reg = kFirstOperator[channel % 9] + half * 3;

		// 0x96C: the level is taken against how loud the music is to be, and
		// held at the quietest the chip knows.  0x200 works that out of the
		// volume: the volume is turned about, shifted down two, squared, and
		// divided by 63, so the quiet end of the range falls away faster than
		// a straight line would.
		const int level = MIN<int>((at[kLevel[half]] & 0x3f) + quieter, 0x3f);

		writeReg(0x20 + reg, ((at[kTremolo[half]] & 1) << 7)
							 | ((at[kVibrato[half]] & 1) << 6)
							 | ((at[kEnvelope[half]] != 0) << 5)
							 | ((at[kKeyScaleRate[half]] & 1) << 4)
							 | (at[kMultiple[half]] & 0x0f), channel);
		writeReg(0x40 + reg, ((at[kKeyScale[half]] & 3) << 6) | level, channel);
		writeReg(0x60 + reg, ((at[kAttack[half]] & 0x0f) << 4)
							 | (at[kDecay[half]] & 0x0f), channel);
		writeReg(0x80 + reg, ((at[kSustain[half]] & 0x0f) << 4)
							 | (at[kRelease[half]] & 0x0f), channel);
		writeReg(0xe0 + reg, at[kWave[half]] & 7, channel);
	}

	// 0x958: how far the first operator feeds back into itself, and whether
	// the two are joined - which the driver keeps the other way round.  An
	// invalid Gold panning value means both speakers.
	const byte pan = at[kPanning] >= 1 && at[kPanning] <= 3 ? at[kPanning] : 3;
	writeReg(0xc0 + channel % 9, (pan << 4) | ((at[kFeedback] & 7) << 1)
							 | (at[kJoined] != 0 ? 0 : 1), channel);
}

/**
 * 0x9EA works the note out: thirty is added to it, it is divided by twelve for
 * the octave and what is left over names the frequency, and the octave stands
 * in the three bits above it.
 */
void AdLibPlayer::writeNote(uint channel, byte note, bool on, bool update) {
	if (channel >= _voiceCount || _voices[channel].instrument < 0)
		return;

	const byte *instrument = _song.begin() + _bank
							 + _voices[channel].instrument * kInstrumentSize;

	// HERAD first applies the instrument's signed root-note macro a byte at a
	// time, then moves MIDI C1..B8 down to the chip's range by subtracting 24.
	if (instrument[kTranspose] != 0)
		note = (byte)(note + instrument[kTranspose]);

	note = (byte)(note - 24);
	if (!update && note >= 0x60)
		note = 0;

	int octave = note / 12;
	int key = note % 12;
	int detune = 0;
	const int bend = _voices[channel].bend;

	if ((instrument[kCoarseSlide] & 1) == 0) {
		if (bend < 0x40) {
			const int amount = 0x40 - bend;
			key -= amount >> 5;
			if (key < 0) {
				key += 12;
				octave--;
			}
			if (octave < 0) {
				key = 0;
				octave = 0;
			}
			detune = -((kFineBend[key] * ((amount << 3) & 0xff)) >> 8);
		} else {
			const int amount = bend - 0x40;
			key += amount >> 5;
			if (key >= 12) {
				key -= 12;
				octave++;
			}
			detune = (kFineBend[key + 1] * ((amount << 3) & 0xff)) >> 8;
		}
	} else {
		if (bend < 0x40) {
			const int amount = 0x40 - bend;
			key -= amount / 5;
			if (key < 0) {
				key += 12;
				octave--;
			}
			if (octave < 0) {
				key = 0;
				octave = 0;
			}
			const int offset = amount % 5 + (key >= 6 ? 5 : 0);
			detune = -kCoarseBend[offset];
		} else {
			const int amount = bend - 0x40;
			key += amount / 5;
			if (key >= 12) {
				key -= 12;
				octave++;
			}
			const int offset = amount % 5 + (key >= 6 ? 5 : 0);
			detune = kCoarseBend[offset];
		}
	}

	const uint16 frequency = kNoteFrequency[key] + detune;
	writeReg(0xa0 + channel % 9, frequency & 0xff, channel);
	writeReg(0xb0 + channel % 9, (on ? 0x20 : 0) | ((octave & 7) << 2)
							 | ((frequency >> 8) & 3), channel);
}

void AdLibPlayer::noteOn(uint channel, byte note, byte velocity) {
	if (channel >= _voiceCount || _voices[channel].instrument < 0)
		return;

	Voice &voice = _voices[channel];
	if (voice.keyOn)
		writeNote(channel, (byte)voice.note, false);

	voice.note = note;
	voice.keyOn = true;
	voice.bend = 0x40;
	writeNote(channel, note, true);

	const byte *instrument = _song.begin() + _bank + voice.instrument * kInstrumentSize;
	if (instrument[kSlideDuration] != 0)
		voice.slideDuration = instrument[kSlideDuration];
	if (instrument[kModVelocity] != 0)
		scaleOperator(channel, false, (int8)instrument[kModVelocity], velocity);
	if (instrument[kCarVelocity] != 0)
		scaleOperator(channel, true, (int8)instrument[kCarVelocity], velocity);
	if (instrument[kFeedbackVelocity] != 0)
		scaleFeedback(channel, (int8)instrument[kFeedbackVelocity], velocity);
}

void AdLibPlayer::noteOff(uint channel, byte note) {
	if (channel >= _voiceCount)
		return;

	Voice &voice = _voices[channel];
	if (!voice.keyOn || voice.note != note)
		return;

	writeNote(channel, note, false);
	voice.keyOn = false;
	voice.slideDuration = 0;
}

void AdLibPlayer::scaleOperator(uint channel, bool carrier, int sensitivity, byte velocity) {
	if (channel >= _voiceCount || _voices[channel].instrument < 0
			|| sensitivity < -4 || sensitivity > 4)
		return;

	const byte *instrument = _song.begin() + _bank
							 + _voices[channel].instrument * kInstrumentSize;
	int level;
	if (sensitivity < 0)
		level = velocity >> (sensitivity + 4);
	else
		level = (0x80 - velocity) >> (4 - sensitivity);

	level = MIN(level, 0x3f);
	level += instrument[carrier ? kCarLevel : kModLevel] & 0x3f;

	const int turned = _hasOutputGain ? 0 : (255 - _volume) >> 2;
	level += MIN(turned * turned / 0x3f, 0x3f);
	level = MIN(level, 0x3f);

	const int reg = kFirstOperator[channel % 9] + (carrier ? 3 : 0);
	const int keyScale = instrument[carrier ? kCarKeyScale : kModKeyScale] & 3;
	writeReg(0x40 + reg, (keyScale << 6) | level, channel);
}

void AdLibPlayer::scaleFeedback(uint channel, int sensitivity, byte velocity) {
	if (channel >= _voiceCount || _voices[channel].instrument < 0
			|| sensitivity < -6 || sensitivity > 6)
		return;

	const byte *instrument = _song.begin() + _bank
							 + _voices[channel].instrument * kInstrumentSize;
	int feedback;
	if (sensitivity < 0)
		feedback = velocity >> (sensitivity + 7);
	else
		feedback = (0x80 - velocity) >> (7 - sensitivity);

	feedback = MIN(feedback, 7);
	feedback = MIN(feedback + (instrument[kFeedback] & 7), 7);
	const byte pan = instrument[kPanning] >= 1 && instrument[kPanning] <= 3
					 ? instrument[kPanning] : 3;
	writeReg(0xc0 + channel % 9, (pan << 4) | (feedback << 1)
							 | (instrument[kJoined] != 0 ? 0 : 1), channel);
}

void AdLibPlayer::aftertouch(uint channel, byte velocity) {
	if (channel >= _voiceCount || _voices[channel].instrument < 0)
		return;

	const byte *instrument = _song.begin() + _bank
							 + _voices[channel].instrument * kInstrumentSize;
	if (instrument[kModAftertouch] != 0)
		scaleOperator(channel, false, (int8)instrument[kModAftertouch], velocity);
	if (instrument[kCarAftertouch] != 0 && instrument[kCarVelocity] != 0)
		scaleOperator(channel, true, (int8)instrument[kCarAftertouch], velocity);
	if (instrument[kFeedbackAftertouch] != 0)
		scaleFeedback(channel, (int8)instrument[kFeedbackAftertouch], velocity);
}

void AdLibPlayer::pitchBend(uint channel, byte bend) {
	if (channel >= _voiceCount)
		return;

	Voice &voice = _voices[channel];
	voice.bend = bend;
	if (voice.keyOn)
		writeNote(channel, (byte)voice.note, true, true);
}

void AdLibPlayer::slide(uint channel) {
	if (channel >= _voiceCount)
		return;

	Voice &voice = _voices[channel];
	if (!voice.keyOn || voice.slideDuration == 0 || voice.instrument < 0)
		return;

	const byte *instrument = _song.begin() + _bank + voice.instrument * kInstrumentSize;
	voice.slideDuration--;
	voice.bend = (byte)(voice.bend + (int8)instrument[kSlideRange]);
	if ((voice.note & 0x7f) != 0)
		writeNote(channel, (byte)voice.note, true, true);
}

bool AdLibPlayer::play(Archive &archive, const Common::String &name, bool loop) {
	stop();

	if (!_opl)
		return false;

	Resource file = archive.openMember(name + ".AGD");
	if (!file) {
		warning("Dune: no AdLib song called %s", name.c_str());
		return false;
	}

	const uint32 size = file->size();
	if (size < 0x54)
		return false;

	Common::Array<byte> song;
	song.resize(size);
	file->read(song.begin(), size);

	// Where the streams end is where the instruments begin, and there are
	// forty bytes to each of them.
	const uint32 bank = READ_LE_UINT16(song.begin());

	if (bank >= size || (size - bank) % kInstrumentSize != 0)
		return false;

	const uint16 speed = READ_LE_UINT16(song.begin() + kSongSpeed);
	if (speed == 0)
		return false;

	uint voiceCount = 0;
	while (voiceCount != kMaxVoices
			&& kFirstTrack + 2 * voiceCount + 1 < size
			&& READ_LE_UINT16(song.begin() + kFirstTrack + 2 * voiceCount) != 0)
		voiceCount++;

	if (voiceCount == 0)
		return false;

	uint32 starts[kMaxVoices + 1];
	for (uint i = 0; i != voiceCount; i++) {
		starts[i] = READ_LE_UINT16(song.begin() + kFirstTrack + 2 * i) + 2;
		if (starts[i] >= bank || (i != 0 && starts[i] < starts[i - 1]))
			return false;
	}
	starts[voiceCount] = bank;

	Common::StackLock lock(_mutex);

	_song = song;
	_bank = bank;
	_instruments = (size - bank) / kInstrumentSize;
	_voiceCount = voiceCount;
	_speed = speed;
	_time = 0;

	// AGD songs may use both banks of the OPL3.  WORMINTR, for example, has
	// thirteen tracks; stopping at nine discarded almost a third of the song.
	for (uint i = 0; i != kMaxVoices; i++) {
		Voice &voice = _voices[i];

		voice = Voice();
		if (i < _voiceCount) {
			voice.from = starts[i];
			voice.to = starts[i + 1];
			voice.at = voice.from;
			voice.done = voice.at >= voice.to;
		} else {
			voice.done = true;
		}
	}

	silence();

	// 0x418 counts measures of ninety-six ticks and reads three words out of
	// the song: at the first of them it keeps all the voices as they
	// stand and arms the count in the third, and at the second it puts them
	// back and goes round again.  So a song is an opening, a middle played
	// over, and whatever follows - not the whole of it round and round.
	_repeatFrom = READ_LE_UINT16(_song.begin() + 0x2c);
	_repeatTo = READ_LE_UINT16(_song.begin() + 0x2e);
	_repeats = 0;
	_armed = false;
	// 0x309 starts the count at one.
	_measure = 1;
	_inMeasure = kMeasureTicks;

	_loop = loop;
	_goldVolume.play(); // 066C restores the driver's saved ordinary level.
	debugC(1, kDebugSound, "Gold song gain: packed=%02x left=%u right=%u",
			_goldVolume.current(), _goldVolume.left(), _goldVolume.right());
	_playing = true;

	debugC(1, kDebugSound, "playing %s on the AdLib Gold, %u bytes, %u tracks and %u instruments",
		   name.c_str(), size, _voiceCount, _instruments);

	return true;
}

void AdLibPlayer::setLooping(bool loop) {
	Common::StackLock lock(_mutex);
	_loop = loop;
}

void AdLibPlayer::stop() {
	Common::StackLock lock(_mutex);
	_goldVolume.stop();

	if (!_playing)
		return;

	_playing = false;
	silence();
}

void AdLibPlayer::setDriverVolume(byte volume, byte pan) {
	Common::StackLock lock(_mutex);
	_goldVolume.setBase(volume, pan);
}

void AdLibPlayer::fadeDriverVolume(uint16 duration, byte volume, byte pan) {
	Common::StackLock lock(_mutex);
	_goldVolume.request(duration, volume, pan, _playing);
	debugC(1, kDebugSound, "Gold fade request: duration=%u volume=%u pan=%u packed=%02x output=%s",
			duration, volume, pan, GoldVolume::packed(volume, pan), _hasOutputGain ? "pcm" : "hardware");
}

void AdLibPlayer::setVolume(int volume) {
	Common::StackLock lock(_mutex);

	_volume = CLIP(volume, 0, 255);

	if (_hasOutputGain)
		return; // Apply host volume after synthesis on the next driver tick.

	// Whatever is sounding is set again, so the change is heard at once.
	for (uint i = 0; i != _voiceCount; i++) {
		if (_voices[i].instrument >= 0)
			loadInstrument(i, (uint)_voices[i].instrument);
	}
}

/**
 * 0x430 keeps the voices as they stand - where each of them is in its
 * stream and how long it has to wait - and 0x458 puts them back again.
 */
void AdLibPlayer::keepTheVoices() {
	for (uint i = 0; i != _voiceCount; i++)
		_kept[i] = _voices[i];
}

void AdLibPlayer::putTheVoicesBack() {
	for (uint i = 0; i != _voiceCount; i++) {
		_voices[i].at = _kept[i].at;
		_voices[i].waiting = _kept[i].waiting;
		_voices[i].running = _kept[i].running;
		_voices[i].done = _kept[i].done;
		_voices[i].first = _kept[i].first;
		_voices[i].nextDelta = _kept[i].nextDelta;
	}
}

void AdLibPlayer::onTimer(void *self) {
	((AdLibPlayer *)self)->tick();
}

bool AdLibPlayer::readDelta(Voice &voice, uint32 &delta) {
	delta = 0;
	while (voice.at < voice.to) {
		const byte b = _song[voice.at++];
		delta = (delta << 7) | (b & 0x7f);
		if ((b & 0x80) == 0)
			return true;
	}

	voice.done = true;
	return false;
}

void AdLibPlayer::executeEvent(uint channel) {
	Voice &voice = _voices[channel];
	if (voice.at >= voice.to) {
		voice.done = true;
		return;
	}

	if ((_song[voice.at] & 0x80) != 0)
		voice.running = _song[voice.at++];
	if (voice.running == 0xff) {
		voice.done = true;
		return;
	}

	switch (voice.running >> 4) {
	case 0x8: {
		if (voice.at + 1 >= voice.to) { voice.done = true; break; }
		const byte note = _song[voice.at];
		voice.at += 2;
		noteOff(channel, note);
		break;
	}

	case 0x9: {
		if (voice.at + 1 >= voice.to) { voice.done = true; break; }
		const byte note = _song[voice.at++];
		const byte velocity = _song[voice.at++];
		if (velocity != 0)
			noteOn(channel, note, velocity);
		else
			noteOff(channel, note);
		break;
	}

	case 0xa:
	case 0xb:
		if (voice.at + 1 >= voice.to) { voice.done = true; break; }
		voice.at += 2;
		break;

	case 0xc: {
		if (voice.at >= voice.to) { voice.done = true; break; }
		const byte which = _song[voice.at++];
		if (which < _instruments) {
			voice.instrument = which;
			loadInstrument(channel, which);
		}
		break;
	}

	case 0xd:
		if (voice.at >= voice.to) { voice.done = true; break; }
		aftertouch(channel, _song[voice.at++]);
		break;

	case 0xe:
		// HERAD's bend is one full-range byte rather than MIDI's two
		// seven-bit bytes, and applies only to the note sounding now.
		if (voice.at >= voice.to) { voice.done = true; break; }
		pitchBend(channel, _song[voice.at++]);
		break;

	default:
		voice.done = true;
		break;
	}
}

/**
 * One tick of the notation.  A delta stands before its event; the old player
 * executed the event first and waited afterward, shifting every note-off and
 * program change onto the preceding event's delay.
 */
void AdLibPlayer::songTick() {
	// 0x418, before anything else: the song's own repeat.
	if (_inMeasure == kMeasureTicks) {
		if (!_armed && _repeatTo != 0 && _measure == _repeatFrom) {
			keepTheVoices();

			_repeats = (int)READ_LE_UINT16(_song.begin() + 0x30) - 1;
			_armed = true;

			debugC(2, kDebugSound, "the song is kept at measure %u, to go round %d more times",
				   (uint)_measure, _repeats);
		} else if (_armed && _repeats > 0 && _measure == _repeatTo) {
			putTheVoicesBack();

			_repeats--;
			_measure = _repeatFrom;

			debugC(2, kDebugSound, "back to measure %u, %d to go", (uint)_measure, _repeats);
		}
	}

	bool anyLeft = false;

	for (uint i = 0; i != _voiceCount; i++) {
		Voice &voice = _voices[i];
		slide(i);

		if (voice.done)
			continue;

		anyLeft = true;

		if (voice.nextDelta) {
			uint32 delta = 0;
			if (!readDelta(voice, delta))
				continue;
			// The original sequencer has one excess timer turn before a
			// track whose first event is delayed.
			if (voice.first && delta != 0)
				delta++;
			voice.first = false;
			voice.nextDelta = false;
			voice.waiting = delta;
		}

		if (voice.waiting != 0) {
			voice.waiting--;
			if (voice.waiting != 0)
				continue;
		}

		executeEvent(i);
		voice.nextDelta = true;

		// Zero-delta events at the same moment follow without another tick.
		while (!voice.done && voice.at < voice.to && _song[voice.at] == 0) {
			voice.at++;
			executeEvent(i);
		}
	}

	// The repeat measure the song stands at.
	if (--_inMeasure == 0) {
		_inMeasure = kMeasureTicks;
		_measure++;
	}

	if (!anyLeft) {
		if (!_loop) {
			_playing = false;
			silence();
			return;
		}

		for (uint i = 0; i != _voiceCount; i++) {
			Voice &voice = _voices[i];

			voice.at = voice.from;
			voice.waiting = 0;
			voice.running = 0;
			voice.done = voice.at >= voice.to;
			voice.first = true;
			voice.nextDelta = true;
			voice.note = -1;
			voice.instrument = -1;
			voice.bend = 0x40;
			voice.slideDuration = 0;
			voice.keyOn = false;
		}

		_repeats = 0;
		_armed = false;
		_measure = 1;
		_inMeasure = kMeasureTicks;

		silence();
	}
}

void AdLibPlayer::tick() {
	Common::StackLock lock(_mutex);

	if (!_playing)
		return;

	_time -= 0x100;
	if (_time < 0) {
		_time += _speed;
		songTick();
	}

	// DNADG06F6 advances the Gold mixer after the song work. Its controls
	// are independent of instrument levels and of the host's music setting.
	const bool wasFading = _goldVolume.fading();
	const byte oldVolume = _goldVolume.current();
	_goldVolume.tick();
	if (wasFading && !_goldVolume.fading())
		debugC(1, kDebugSound, "Gold fade settled: packed=%02x", _goldVolume.current());
	if (_hasOutputGain)
		_opl->setOutputGain(_goldVolume.left() * _volume,
				_goldVolume.right() * _volume, 31 * 255);
	if (oldVolume != 0 && _goldVolume.current() == 0) {
		_playing = false; // 0F15 stops the song when both FM channels reach zero.
		silence();
	}
}

} // End of namespace Dune
