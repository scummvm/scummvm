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

#include "engines/freescape/games/castle/c64.music.h"

#include "audio/softsynth/sid.h"
#include "common/mutex.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "freescape/freescape.h"
#include "freescape/games/castle/castle.musicdata.h"
#include "freescape/sid.h"

using namespace Freescape::CastleMusicData;

namespace Freescape {

// The original $cfd8 selects subtune index + 1 of the driver at $c800,
// but ignores effects while music is selected. Interpret effects at 50 Hz
// during playback, releasing the SID when switching back to music.
class CastleC64Sound : public Sound, private Audio::AudioStream {
public:
	CastleC64Sound(Audio::Mixer *mixer, const Common::Array<byte> &data)
		: _mixer(mixer), _sid(nullptr), _type(kTypeNormal), _enabled(false),
		  _ghostPlaying(false), _volume(0), _active(false), _finished(true),
		  _index(0), _ticks(0), _samplesUntilTick(0), _sampleRemainder(0),
		  _releaseTicks(0), _quietTicks(0), _peak(0) {
		// Keep only the original score and instrument data, using C64 addresses.
		_data.assign(data.begin(), data.begin() + 0x1047);
	}

	~CastleC64Sound() override { setEnabled(false); }

	void setEnabled(bool enabled) {
		_enabled = enabled;
		if (!enabled) {
			stopSound(_type);
			delete _sid;
			_sid = nullptr;
		}
	}

	void playSound(int index, Type type) override {
		if (!isSoundAvailable(index) || !_mixer->isReady())
			return;

		// Remove the mixer callback before changing its SID or sequencer state.
		_mixer->stopHandle(_handle);
		if (!_sid) {
			_sid = new Resid::SID(SID::Config::kSidPAL);
			_sid->init();
			_sid->setCallbackFrequency(50);
		}
		_sid->reset();
		memset(_channels, 0, sizeof(_channels));
		_type = type;
		_ghostPlaying = false;
		_volume = 15;
		_active = true;
		_finished = false;
		_index = index;
		_ticks = 0;
		_samplesUntilTick = _sampleRemainder = 0;
		_releaseTicks = _quietTicks = _peak = 0;
		for (int ch = 0; ch < 3; ch++) {
			Channel &c = _channels[ch];
			uint address = 0x0d71 + (index + 1) * 6 + ch * 2;
			c.order = read(address) | (read(address + 1) << 8);
			for (int i = 0; i < 8; i++)
				c.instruments[i] = read(0x102f + ch * 8 + i);
		}

		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handle, this, -1,
			Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
	}

	void stopSound(Type type) override {
		if (_type == type) {
			_mixer->stopHandle(_handle);
			_active = false;
			_finished = true;
			_ghostPlaying = false;
		}
	}

	bool isPlayingSound(Type type) const override {
		if (_type != type || !_mixer->isSoundHandleActive(_handle))
			return false;
		Common::StackLock lock(_mixer->mutex());
		// The sequence's end releases SOUND waits while the SID decays.
		return _active;
	}

	bool isSoundAvailable(int index) const override { return _enabled && index >= 0 && index < 16; }

	void updateGhost(bool active) {
		if (!_enabled)
			return;
		if (!active) {
			if (_ghostPlaying)
				stopSound(_type);
		} else if (!isPlayingSound(_type)) {
			playSound(15, kTypeNormal);
			_ghostPlaying = true;
		}
	}

private:
	struct Channel {
		uint16 order, pattern;
		byte orderPosition, patternPosition, repeats;
		byte transpose, detune, instruments[8];
		byte registers[7];
		byte duration, rest, gateOffTime;
		byte vibrato, vibratoDelay, vibratoStep;
		byte waveform, waveformDelay, waveformStep;
	};

	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;
	Common::Array<byte> _data;
	Resid::SID *_sid;
	Type _type;
	bool _enabled;
	bool _ghostPlaying;
	Channel _channels[3];
	byte _volume;
	bool _active;
	bool _finished;
	int _index;
	int _ticks;
	int _samplesUntilTick;
	int _sampleRemainder;
	int _releaseTicks;
	int _quietTicks;
	int _peak;

	int getRate() const override { return _mixer->getOutputRate(); }
	bool isStereo() const override { return false; }
	bool endOfData() const override { return _finished; }

	int readBuffer(int16 *buffer, int numSamples) override {
		int generated = 0;
		while (generated < numSamples && !_finished) {
			if (!_samplesUntilTick) {
				if (_active) {
					// Subtune 1 is a longer musical cue; bound malformed data
					// without cutting that sequence short.
					if (++_ticks > 180 * 50)
						error("Unterminated Castle C64 sound %d", _index);
					advance();
				}
				_sampleRemainder += getRate();
				_samplesUntilTick = _sampleRemainder / 50;
				_sampleRemainder %= 50;
				_peak = 0;
			}
			int count = MIN(numSamples - generated, _samplesUntilTick);
			_sid->readBuffer(buffer + generated, count);
			if (!_active) {
				for (int i = 0; i < count; i++)
					_peak = MAX(_peak, ABS(int(buffer[generated + i])));
			}
			generated += count;
			_samplesUntilTick -= count;
			if (!_active && !_samplesUntilTick) {
				// Let the release envelopes finish in the live stream.
				_quietTicks = _peak < 8 ? _quietTicks + 1 : 0;
				_finished = ++_releaseTicks >= 30 * 50 || _quietTicks >= 5;
			}
		}
		return generated;
	}

	byte read(uint address) const {
		if (address >= _data.size())
			error("Invalid Castle C64 sound address %x", address);
		return _data[address];
	}

	byte readOrder(Channel &c) { return read(c.order + c.orderPosition++); }
	byte readPattern(Channel &c) { return read(c.pattern + c.patternPosition++); }

	bool nextPattern(Channel &c) {
		if (c.repeats) {
			c.repeats--;
			c.patternPosition = 0;
			return true;
		}
		for (int commands = 0; commands < 256; commands++) {
			byte command = readOrder(c);
			if (command == 0xff)
				return false;
			if (command < 0x80) {
				if (command >= 51)
					error("Invalid Castle C64 sound pattern %d", command);
				c.pattern = read(0x0d07 + command) | (read(0x0d3c + command) << 8);
				c.patternPosition = 0;
				return true;
			}
			// $ca72: transpose, repeat, detune, or per-channel instrument map.
			switch (command & 0x60) {
			case 0x00:
				c.transpose = readOrder(c);
				break;
			case 0x20:
				c.repeats = (command & 0x1f) - 1;
				break;
			case 0x40:
				c.detune = readOrder(c);
				break;
			case 0x60:
				if (command == 0xe8)
					_volume = readOrder(c);
				else
					c.instruments[command & 7] = readOrder(c);
				break;
			}
		}
		error("Invalid Castle C64 sound order list");
	}

	void setInstrument(Channel &c, byte instrument) {
		if (instrument >= 8)
			error("Invalid Castle C64 sound instrument %d", instrument);
		uint address = 0x0f15 + c.instruments[instrument];
		// All instruments used by the effect subtunes have fixed pulse width.
		if (read(address) != 0)
			error("Unsupported Castle C64 sound pulse modulation");
		for (int reg = 3; reg < 7; reg++)
			c.registers[reg] = read(address + reg - 2);
		c.gateOffTime = read(address + 5);
		c.vibrato = read(address + 7);
		c.waveform = read(address + 8);
	}

	bool parse(Channel &c, int ch) {
		for (int commands = 0; commands < 256; commands++) {
			if (!c.pattern && !nextPattern(c))
				return false;
			byte command = readPattern(c);
			if (command == 0xff) {
				if (!nextPattern(c))
					return false;
			} else if (command >= 0x80 && command < 0x90) {
				setInstrument(c, command & 15);
			} else if (command == 0xa0) {
				c.duration = c.rest = readPattern(c);
				_sid->writeReg(ch * 7 + kSIDV1Ctrl, 0);
				return true;
			} else if (command < 0x80) {
				byte note = command + c.transpose + 20;
				if (note >= 96)
					error("Invalid Castle C64 sound note %d", note);
				uint16 frequency = (read(0x065a + note) | (read(0x06ba + note) << 8)) + c.detune;
				c.registers[0] = frequency;
				c.registers[1] = frequency >> 8;
				c.registers[4] |= 1;
				c.duration = readPattern(c);
				if (c.vibrato) {
					c.vibratoDelay = read(0x0fe2 + c.vibrato);
					c.vibratoStep = 1;
				}
				if (c.waveform) {
					byte entry = read(0x100e + c.waveform);
					c.waveformDelay = (entry >> 4) + 1;
					c.waveformStep = 1;
					c.registers[4] = read(0x100a + (entry & 7)) | 1;
				}
				return true;
			} else {
				// The sixteen effect subtunes use notes, instruments and rests.
				error("Unsupported Castle C64 sound command %x", command);
			}
		}
		error("Invalid Castle C64 sound pattern");
	}

	void updateEffects(Channel &c) {
		// $cd97: signed frequency deltas, with the high bit of the step
		// reversing the table after each seven-entry vibrato half-cycle.
		if (c.vibrato && c.vibratoDelay != 0x80) {
			if (c.vibratoDelay && !(c.vibratoDelay & 0x80)) {
				c.vibratoDelay--;
			} else {
				int delta = int8(read(0x0fe2 + c.vibrato + (c.vibratoStep & 0x7f)));
				if (c.vibratoStep & 0x80)
					delta = -delta;
				uint16 frequency = (c.registers[0] | (c.registers[1] << 8)) + delta;
				c.registers[0] = frequency;
				c.registers[1] = frequency >> 8;
				if ((++c.vibratoStep & 0x7f) == 8) {
					c.vibratoStep ^= 0x89;
					if (c.vibratoStep == 1 && c.vibratoDelay)
						c.vibratoDelay--;
				}
			}
		}
		// $ce26: timed waveform changes, including the noise attack used by
		// the collision/menu instrument. Preserve the current gate bit.
		if (c.waveform && !--c.waveformDelay && !(c.waveformStep & 0x80)) {
			byte entry = read(0x100e + c.waveform + c.waveformStep);
			c.waveformDelay = entry >> 4;
			c.registers[4] = (c.registers[4] & 1) | read(0x100a + (entry & 7));
			byte control = read(0x100d + c.waveform);
			if (++c.waveformStep == (control & 7))
				c.waveformStep = control & 0x80 ? 0 : 0xff;
		}
	}

	void advance() {
		// $c885 parses the next note at duration 1, suppressing that frame's
		// register copy so that the previous gate-off reaches the SID first.
		bool skipWrites = false;
		for (int ch = 0; ch < 3; ch++) {
			Channel &c = _channels[ch];
			if (c.duration == 1) {
				c.duration = c.rest = 0;
				skipWrites = true;
			}
		}
		for (int ch = 0; ch < 3; ch++) {
			Channel &c = _channels[ch];
			if (!c.duration) {
				if (!parse(c, ch)) {
					_active = false;
					break;
				}
			} else {
				if (c.duration == c.gateOffTime)
					c.registers[4] &= 0xfe;
				c.duration--;
				if (c.rest)
					c.rest--;
				else if (!skipWrites)
					updateEffects(c);
			}
		}
		if (!_active) {
			for (int ch = 0; ch < 3; ch++)
				_sid->writeReg(ch * 7 + kSIDV1Ctrl, 0);
		} else if (!skipWrites) {
			for (int ch = 0; ch < 3; ch++) {
				if (!_channels[ch].rest) {
					for (int reg = 0; reg < 7; reg++)
						_sid->writeReg(ch * 7 + reg, _channels[ch].registers[reg]);
				}
			}
			_sid->writeReg(kSIDVolume, _volume);
		}
	}
};

Sound *createCastleC64Sound(Audio::Mixer *mixer, const Common::Array<byte> &data) {
	if (data.size() < 0x1047)
		error("Missing Castle C64 sound data");
	if (mixer->isReady())
		return new CastleC64Sound(mixer, data);
	return nullptr;
}

void enableCastleC64Sound(Sound *sound, bool enabled) {
	if (sound)
		static_cast<CastleC64Sound *>(sound)->setEnabled(enabled);
}

void updateCastleC64GhostSound(Sound *sound, bool active) {
	if (sound)
		static_cast<CastleC64Sound *>(sound)->updateGhost(active);
}

const int kCastleSIDVoiceOffset[] = { 0, 7, 14 };

void CastleC64MusicPlayer::ChannelState::reset(const byte *channelOrderList) {
	orderList = channelOrderList;
	orderPosition = 0;
	patternIndex = 0;
	patternDataOffset = 0;
	patternOffset = 0;
	delay = 0;
	instrument = 0;
	transpose = 0;
	currentNote = 0;
	baseFrequency = 0;
	frequencyOffset = 0;
	vibratoStep = 1;
	vibratoReverse = false;
	control = 0;
	active = false;
}

CastleC64MusicPlayer::CastleC64MusicPlayer(Audio::Mixer *mixer)
	: _sid(nullptr),
	  _mixer(mixer),
	  _musicActive(false),
	  _samplesUntilTick(0),
	  _sampleRemainder(0),
	  _tick(0) {
}

CastleC64MusicPlayer::~CastleC64MusicPlayer() {
	stopMusic();
}

void CastleC64MusicPlayer::initSID() {
	destroySID();

	_sid = SID::Config::create(SID::Config::kSidPAL);
	if (!_sid || !_sid->init()) {
		warning("CastleC64MusicPlayer: Failed to create SID emulator");
		destroySID();
		return;
	}

	// Pull samples through our own music stream. Chip::start() would attach
	// the SID as plain audio, bypassing ScummVM's music volume control.
	_sid->setCallbackFrequency(50);
}

void CastleC64MusicPlayer::destroySID() {
	if (_sid) {
		_sid->stop();
		delete _sid;
		_sid = nullptr;
	}
}

void CastleC64MusicPlayer::sidWrite(int reg, byte data) {
	if (_sid)
		_sid->writeReg(reg, data);
}

void CastleC64MusicPlayer::startMusic() {
	stopMusic();
	if (!_mixer->isReady())
		return;
	initSID();
	if (!_sid)
		return;

	setupSong();
	_samplesUntilTick = 0;
	_sampleRemainder = 0;
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_handle, this, -1,
		Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
}

void CastleC64MusicPlayer::stopMusic() {
	// Stop the mixer callback before touching its SID or channel state.
	_mixer->stopHandle(_handle);
	_musicActive = false;
	silenceAll();
	destroySID();
}

bool CastleC64MusicPlayer::isPlaying() const {
	return _mixer->isSoundHandleActive(_handle);
}

int CastleC64MusicPlayer::getRate() const {
	return _mixer->getOutputRate();
}

int CastleC64MusicPlayer::readBuffer(int16 *buffer, int numSamples) {
	int generated = 0;
	while (generated < numSamples) {
		if (!_samplesUntilTick) {
			onTimer();
			_sampleRemainder += getRate();
			_samplesUntilTick = _sampleRemainder / 50;
			_sampleRemainder %= 50;
		}
		int count = MIN(numSamples - generated, _samplesUntilTick);
		static_cast<Resid::SID *>(_sid)->readBuffer(buffer + generated, count);
		generated += count;
		_samplesUntilTick -= count;
	}
	return numSamples;
}

void CastleC64MusicPlayer::silenceAll() {
	for (int i = 0; i <= kSIDVolume; i++)
		sidWrite(i, 0);
}

void CastleC64MusicPlayer::setupSong() {
	silenceAll();
	_tick = 0;

	sidWrite(kSIDFilterLo, 0x00);
	sidWrite(kSIDFilterHi, 0x00);
	sidWrite(kSIDFilterCtrl, 0x00);
	sidWrite(kSIDVolume, 0x0F);

	for (int i = 0; i < kChannelCount; i++) {
		_channels[i].reset(kChannelOrderLists[i]);
		loadNextPattern(i);
	}

	_musicActive = true;
}

void CastleC64MusicPlayer::onTimer() {
	if (!_musicActive)
		return;

	for (int i = 0; i < kChannelCount; i++) {
		if (_channels[i].delay > 0) {
			if (_channels[i].active) {
				const InstrumentData &instrument = kInstruments[_channels[i].instrument % ARRAYSIZE(kInstruments)];
				if (_channels[i].delay == instrument.gateOffTime)
					gateOff(i);
				else
					applyFrameEffects(i);
			}

			_channels[i].delay--;
			continue;
		}

		parseCommands(i);
	}

	_tick++;
}

void CastleC64MusicPlayer::loadNextPattern(int channel) {
	ChannelState &c = _channels[channel];
	int safety = 128;

	while (safety-- > 0) {
		byte value = c.orderList[c.orderPosition++];

		if (value == kOrderEnd) {
			c.orderPosition = 0;
			continue;
		}

		if (value == kOrderTranspose) {
			byte transpose = c.orderList[c.orderPosition++];
			c.transpose = transpose >= 0x80 ? transpose - 0x100 : transpose;
			continue;
		}

		if (value < ARRAYSIZE(kPatternOffsets)) {
			c.patternIndex = value;
			c.patternDataOffset = kPatternOffsets[value];
			c.patternOffset = 0;
			debugC(1, kFreescapeDebugMedia,
				"Castle SID t=%u ch=%d order=%u PATTERN %u dataOffset=%u transpose=%d",
				(uint)_tick, channel, c.orderPosition - 1, c.patternIndex, c.patternDataOffset, c.transpose);
			return;
		}
	}

	c.patternDataOffset = 0;
	c.patternOffset = 0;
}

byte CastleC64MusicPlayer::readPatternByte(int channel) {
	ChannelState &c = _channels[channel];
	uint16 offset = c.patternDataOffset + c.patternOffset;
	if (offset >= ARRAYSIZE(kPatternData)) {
		loadNextPattern(channel);
		offset = c.patternDataOffset + c.patternOffset;
	}

	c.patternOffset++;
	return kPatternData[offset];
}

void CastleC64MusicPlayer::parseCommands(int channel) {
	ChannelState &c = _channels[channel];
	int safety = 128;

	while (safety-- > 0) {
		byte command = readPatternByte(channel);
		uint16 commandOffset = c.patternOffset - 1;

		if (command == 0xFF) {
			loadNextPattern(channel);
			continue;
		}

		if (command >= 0x80 && command < 0x90) {
			c.instrument = command & 0x0F;
			debugC(1, kFreescapeDebugMedia,
				"Castle SID t=%u ch=%d pat=%u pos=%u INST %u",
				(uint)_tick, channel, c.patternIndex, commandOffset, c.instrument);
			continue;
		}

		if (command == 0xA0) {
			byte duration = readPatternByte(channel);
			debugC(1, kFreescapeDebugMedia,
				"Castle SID t=%u ch=%d pat=%u pos=%u REST dur=%u inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, duration, c.instrument);
			rest(channel);
			c.delay = MAX<uint16>(1, duration);
			return;
		}

		if (command >= 0x90 && command < 0xC0) {
			debugC(1, kFreescapeDebugMedia,
				"Castle SID t=%u ch=%d pat=%u pos=%u EFFECT $%02x skipped inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, command, c.instrument);
			continue;
		}

		if (command >= 0xC0) {
			byte slideDuration = readPatternByte(channel);
			byte slideLo = readPatternByte(channel);
			byte slideHi = readPatternByte(channel);
			debugC(1, kFreescapeDebugMedia,
				"Castle SID t=%u ch=%d pat=%u pos=%u SLIDE $%02x dur=%u delta=$%02x%02x skipped inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, command, slideDuration, slideHi, slideLo, c.instrument);
			continue;
		}

		byte duration = readPatternByte(channel);
		if (command == 0) {
			debugC(1, kFreescapeDebugMedia,
				"Castle SID t=%u ch=%d pat=%u pos=%u OFF dur=%u inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, duration, c.instrument);
			rest(channel);
		} else {
			int effectiveNote = command + c.transpose + 20;
			noteOn(channel, command);
			debugC(1, kFreescapeDebugMedia,
				"Castle SID t=%u ch=%d pat=%u pos=%u NOTE raw=$%02x effective=%d inst=%u transpose=%d dur=%u freq=$%04x ctrl=$%02x",
				(uint)_tick, channel, c.patternIndex, commandOffset, command, effectiveNote, c.instrument,
				c.transpose, duration, c.baseFrequency, c.control);
		}
		c.delay = MAX<uint16>(1, duration);
		return;
	}

	debugC(1, kFreescapeDebugMedia,
		"Castle SID t=%u ch=%d pat=%u parser safety stop inst=%u",
		(uint)_tick, channel, c.patternIndex, c.instrument);
	rest(channel);
	c.delay = 12;
}

void CastleC64MusicPlayer::noteOn(int channel, byte note) {
	ChannelState &c = _channels[channel];
	const InstrumentData &instrument = kInstruments[c.instrument % ARRAYSIZE(kInstruments)];
	int voiceOffset = kCastleSIDVoiceOffset[channel];

	int effectiveNote = note + c.transpose + 20;
	c.currentNote = CLIP<int>(effectiveNote, 0, kMaxNote);
	c.baseFrequency = noteToSIDFrequency(effectiveNote);
	c.frequencyOffset = 0;
	c.vibratoStep = 1;
	c.vibratoReverse = false;
	c.control = sidControlForInstrument(c.instrument);
	c.active = true;

	sidWrite(voiceOffset + kSIDV1Ctrl, 0x00);
	writeFrequency(channel, c.baseFrequency);
	sidWrite(voiceOffset + kSIDV1PwLo, 0x00);
	sidWrite(voiceOffset + kSIDV1PwHi, instrument.pulseWidth & 0x0F);
	sidWrite(voiceOffset + kSIDV1AD, instrument.attackDecay);
	sidWrite(voiceOffset + kSIDV1SR, instrument.sustainRelease);
	sidWrite(voiceOffset + kSIDV1Ctrl, c.control);
}

void CastleC64MusicPlayer::rest(int channel) {
	ChannelState &c = _channels[channel];
	int voiceOffset = kCastleSIDVoiceOffset[channel];

	c.active = false;
	c.currentNote = 0;
	c.baseFrequency = 0;
	c.frequencyOffset = 0;
	c.control = 0;
	sidWrite(voiceOffset + kSIDV1Ctrl, 0x00);
}

void CastleC64MusicPlayer::gateOff(int channel) {
	ChannelState &c = _channels[channel];
	int voiceOffset = kCastleSIDVoiceOffset[channel];

	c.control &= 0xFE;
	sidWrite(voiceOffset + kSIDV1Ctrl, c.control);
}

void CastleC64MusicPlayer::writeFrequency(int channel, uint16 frequency) {
	int voiceOffset = kCastleSIDVoiceOffset[channel];
	sidWrite(voiceOffset + kSIDV1FreqLo, frequency & 0xFF);
	sidWrite(voiceOffset + kSIDV1FreqHi, frequency >> 8);
}

uint16 CastleC64MusicPlayer::noteToSIDFrequency(int note) const {
	return getCastleSIDFrequency(note);
}

void CastleC64MusicPlayer::applyFrameEffects(int channel) {
	ChannelState &c = _channels[channel];
	if (!c.active || c.currentNote == 0 || c.baseFrequency == 0)
		return;

	const InstrumentData &instrument = kInstruments[c.instrument % ARRAYSIZE(kInstruments)];
	const int8 *vibratoTable = getCastleVibratoTable(instrument.vibrato);
	if (!vibratoTable)
		return;

	int8 sidDelta = vibratoTable[c.vibratoStep & 0x07];
	c.frequencyOffset += c.vibratoReverse ? -sidDelta : sidDelta;
	c.vibratoStep++;
	if (c.vibratoStep >= 8) {
		c.vibratoStep = 1;
		c.vibratoReverse = !c.vibratoReverse;
	}

	int32 frequency = (int32)c.baseFrequency + c.frequencyOffset;
	writeFrequency(channel, CLIP<int32>(frequency, 0, 0xFFFF));
}

byte CastleC64MusicPlayer::sidControlForInstrument(byte instrument) const {
	return getCastleSIDControlForInstrument(instrument);
}

} // namespace Freescape
