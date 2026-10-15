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
#include "common/endian.h"
#include "common/textconsole.h"
#include "igor/midi.h"

#ifndef DISABLE_NUKED_OPL
#include "audio/softsynth/opl/nuked.h"
#endif

namespace Igor {

MidiParser_CTMF::MidiParser_CTMF()
	: _instrumentsCount(0) {
	memset(_instruments, 0, sizeof(_instruments));
}

void MidiParser_CTMF::decodeHeader(const uint8 *p) {
	_instrumentsDataOffset = READ_LE_UINT16(p); p += 2;
	_midiDataOffset = READ_LE_UINT16(p); p += 2;
	_ticksPerQuarter = READ_LE_UINT16(p); p += 2;
	_ticksPerSecond = READ_LE_UINT16(p); p += 2;
	p += 22;
	_instrumentsCount = READ_LE_UINT16(p); p += 2;
	_basicTempo = READ_LE_UINT16(p); p += 2;
}

void MidiParser_CTMF::decodeAdlibInstrument(struct AdlibInstrument *ins, const uint8 *p) {
	ins->chr[kAdlibCarrier] = p[0];
	ins->chr[kAdlibModulator] = p[1];
	ins->scale[kAdlibCarrier] = p[2];
	ins->scale[kAdlibModulator] = p[3];
	ins->attack[kAdlibCarrier] = p[4];
	ins->attack[kAdlibModulator] = p[5];
	ins->sustain[kAdlibCarrier] = p[6];
	ins->sustain[kAdlibModulator] = p[7];
	ins->waveSel[kAdlibCarrier] = p[8];
	ins->waveSel[kAdlibModulator] = p[9];
	ins->feedback = p[10];
}

bool MidiParser_CTMF::loadMusic(const byte *data, uint32 size) {
	if (memcmp(data, "CTMF", 4) == 0 && READ_LE_UINT16(data + 4) == 0x101) {
		decodeHeader(data + 6);
		assert(_instrumentsCount <= kMaxInstruments);
		for (int i = 0; i < _instrumentsCount; ++i) {
			decodeAdlibInstrument(&_instruments[i], data + _instrumentsDataOffset + i * 16);
		}
		// reset parser
		_numTracks = 1;
		_tracks[0][0] = data + _midiDataOffset;
		_ppqn = _ticksPerQuarter;
		setTempo(500000);
		setTrack(0);
		return true;
	}
	return false;
}

void MidiParser_CTMF::parseNextEvent(EventInfo &info) {
	info.start = _position._subtracks[0]._playPos;
	info.delta = readVLQ(_position._subtracks[0]._playPos);

	if ((_position._subtracks[0]._playPos[0] & 0xF0) >= 0x80) {
		info.event = *_position._subtracks[0]._playPos++;
	} else {
		info.event = _position._subtracks[0]._runningStatus;
	}

	if ((info.event & 0x80) == 0) {
		return;
	}

	_position._subtracks[0]._runningStatus = info.event;
	switch (info.command()) {
	case 0x8: // Note Off
	case 0x9: // Note On
	case 0xB: // Control Mode Change
		info.basic.param1 = *_position._subtracks[0]._playPos++;
		info.basic.param2 = *_position._subtracks[0]._playPos++;
		if (info.command() == 0x9 && info.basic.param2 == 0) {
			info.event = info.channel() | 0x80; // Note Off
		}
		return;
	case 0xC: // Program Change
		info.basic.param1 = *(_position._subtracks[0]._playPos++);
		info.basic.param2 = 0;
		return;
	case 0xF:
		switch (info.event & 15) {
		case 0xF:
			info.ext.type = *(_position._subtracks[0]._playPos++);
			info.length = readVLQ(_position._subtracks[0]._playPos);
			info.ext.data = _position._subtracks[0]._playPos;
			_position._subtracks[0]._playPos += info.length;
			return;
		}
	}
	warning("MidiParser_CTMF::parseNextEvent: Unhandled event code %x", info.event);
}

int AdlibMidiDriver::open() {
	MidiDriver_Emulated::open();
	_opl = OPL::Config::create(OPL::Config::kOpl2);
	if (!_opl || !_opl->init())
		error("Unable to initialize Igor AdLib music");
	_oplStream = dynamic_cast<Audio::EmulatedChip *>(_opl);
#ifndef DISABLE_NUKED_OPL
	// Nuked produces stereo samples even when emulating an OPL2 chip.
	_oplStereo = dynamic_cast<OPL::NUKED::OPL *>(_opl) != nullptr;
#endif
	if (_oplStream)
		_oplStream->setCallbackFrequency(_baseFreq);
	memset(_adlibData, 0, sizeof(_adlibData));
	_adlibRhythmMode = false;
	for (int i = 0; i < kAdlibChannelsCount; ++i) {
		_adlibChannels[i].ch = -1;
		_adlibChannels[i].lt = _adlibChannels[i].note = 0;
	}
	memset(_adlibInstrumentsMappingTable, 0, sizeof(_adlibInstrumentsMappingTable));
	adlibSetupCard();
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_mixerSoundHandle, this, -1,
		Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
	return 0;
}

void AdlibMidiDriver::close() {
	_mixer->stopHandle(_mixerSoundHandle);
	delete _opl;
	_opl = nullptr;
	_oplStream = nullptr;
	_oplStereo = false;
	_isOpen = false;
}

void AdlibMidiDriver::send(uint32 b) {
	int channel = b & 15;
	int cmd = (b >> 4) & 7;
	int param1 = (b >> 8) & 255;
	int param2 = (b >> 16) & 255;
	switch (cmd) {
	case 0:
		adlibTurnNoteOff(channel, param1);
		break;
	case 1:
		adlibTurnNoteOn(channel, param1, param2);
		break;
	case 3:
		adlibControlChange(channel, param1, param2);
		break;
	case 4:
		adlibProgramChange(channel, param1);
		break;
	default:
		warning("Unhandled cmd %d channel %d (0x%X)", cmd, channel, b);
		break;
	}
}

void AdlibMidiDriver::generateSamples(int16 *data, int len) {
	const int samples = len * (isStereo() ? 2 : 1);
	if (_oplStream)
		_oplStream->readBuffer(data, samples);
	else
		memset(data, 0, sizeof(int16) * samples);
}

void AdlibMidiDriver::adlibWrite(int port, int value) {
	_opl->writeReg(port, value);
	_adlibData[port & 255] = value & 255;
}

void AdlibMidiDriver::adlibSetupCard() {
	for (int i = 0; i < 256; ++i) {
		adlibWrite(i, 0);
	}
	adlibWrite(1, 0x20);
	adlibWrite(0xBD, 0xC0);
}

void AdlibMidiDriver::adlibTurnNoteOff(int channel, int note) {
	for (int i = 0; i < kAdlibChannelsCount; ++i) {
		if (_adlibChannels[i].ch == channel && _adlibChannels[i].note == note) {
			adlibEndNote(i);
			_adlibChannels[i].ch = -1;
		}
	}
}

void AdlibMidiDriver::adlibTurnNoteOn(int channel, int note, int velocity) {
	assert(velocity != 0);

	for (int i = 0; i < kAdlibChannelsCount; ++i) {
		if (_adlibChannels[i].ch != -1) {
			++_adlibChannels[i].lt;
		}
	}

	int ch = -1;
	if (!_adlibRhythmMode || channel < 11) {
		int maxLt = -1;
		int maxCh = -1;
		for (int i = 0; i < (_adlibRhythmMode ? 6 : 9); ++i) {
			if (_adlibChannels[i].ch == -1) {
				ch = i;
				break;
			}
			if (_adlibChannels[i].lt > maxLt) {
				maxLt = _adlibChannels[i].lt;
				maxCh = i;
			}
		}
		if (ch == -1) {
			assert(maxCh != -1);
			ch = maxCh;
			adlibEndNote(ch);
		}
	} else {
		ch = _adlibPercussionsMappingTable[channel - 11];
	}

	const AdlibInstrument &ins = _adlibInstruments[_adlibInstrumentsMappingTable[channel]];
	if (!_adlibRhythmMode || channel < 12) {
		adlibSetupInstrument(ch, ins);
	} else {
		adlibSetupPercussion(channel, ins);
	}
	adlibSetupNote(ch, note - 13, velocity);
	_adlibChannels[ch].ch = channel;
	_adlibChannels[ch].note = note;
	_adlibChannels[ch].lt = 0;
}

void AdlibMidiDriver::adlibSetupInstrument(int channel, const AdlibInstrument &ins) {
	adlibWrite(0x20 + _adlibOperatorsTable[channel], ins.chr[kAdlibCarrier]);
	adlibWrite(0x23 + _adlibOperatorsTable[channel], ins.chr[kAdlibModulator]);
	adlibWrite(0x40 + _adlibOperatorsTable[channel], ins.scale[kAdlibCarrier]);
	if ((ins.feedback & 1) == 0) {
		adlibWrite(0x43 + _adlibOperatorsTable[channel], ins.scale[kAdlibModulator]);
	} else {
		adlibWrite(0x43 + _adlibOperatorsTable[channel], 0);
	}
	adlibWrite(0x60 + _adlibOperatorsTable[channel], ins.attack[kAdlibCarrier]);
	adlibWrite(0x63 + _adlibOperatorsTable[channel], ins.attack[kAdlibModulator]);
	adlibWrite(0x80 + _adlibOperatorsTable[channel], ins.sustain[kAdlibCarrier]);
	adlibWrite(0x83 + _adlibOperatorsTable[channel], ins.sustain[kAdlibModulator]);
	adlibWrite(0xE0 + _adlibOperatorsTable[channel], ins.waveSel[kAdlibCarrier]);
	adlibWrite(0xE3 + _adlibOperatorsTable[channel], ins.waveSel[kAdlibModulator]);
	adlibWrite(0xC0 + channel, ins.feedback);
}

void AdlibMidiDriver::adlibSetupPercussion(int channel, const AdlibInstrument &ins) {
	channel = _adlibChannelsMappingTable[channel - 12];
	adlibWrite(0x20 + channel, ins.chr[kAdlibCarrier]);
	adlibWrite(0x40 + channel, ins.scale[kAdlibCarrier]);
	adlibWrite(0x60 + channel, ins.attack[kAdlibCarrier]);
	adlibWrite(0x80 + channel, ins.sustain[kAdlibCarrier]);
	adlibWrite(0xE0 + channel, ins.waveSel[kAdlibCarrier]);
	adlibWrite(0xC0 + channel, ins.feedback);
}

void AdlibMidiDriver::adlibSetupNote(int channel, int note, int velocity) {
	adlibSetVolume(channel, velocity);
	int f = _adlibNoteFreqTable[note % 12];
	adlibWrite(0xA0 + channel, f);
	int oct = note / 12;
	int c = ((f & 0x300) >> 8) + (oct << 2);
	if (!_adlibRhythmMode || channel < 6) {
		c |= 0x20;
	}
	adlibWrite(0xB0 + channel, c);
}

void AdlibMidiDriver::adlibEndNote(int channel) {
	adlibWrite(0xB0 + channel, _adlibData[0xB0 + channel] & ~0x20);
}

void AdlibMidiDriver::adlibSetVolume(int channel, int volume) {
	volume = 63 - (volume >> 1);
	if ((_adlibData[0xC0 + channel] & 1) == 1) {
		adlibWrite(0x40 + _adlibOperatorsTable[channel], volume | (_adlibData[0x40 + _adlibOperatorsTable[channel]] & 0xC0));
	}
	adlibWrite(0x43 + _adlibOperatorsTable[channel], volume | (_adlibData[0x43 + _adlibOperatorsTable[channel]] & 0xC0));
}

void AdlibMidiDriver::adlibControlChange(int channel, int control, int param) {
	switch (control) {
	case 0x67:
		_adlibRhythmMode = param != 0;
		if (_adlibRhythmMode) {
			adlibWrite(0xBD, _adlibData[0xBD] | 0x20);
		} else {
			adlibWrite(0xBD, _adlibData[0xBD] & ~0x20);
		}
		break;
	case 0x7B:
		adlibTurnNoteOff(channel, -1);
		break;
	default:
		warning("Unhandled adlibControlChange 0x%X %d", control, param);
		break;
	}
}

void AdlibMidiDriver::adlibProgramChange(int channel, int num) {
	_adlibInstrumentsMappingTable[channel] = num;
}

const uint8 AdlibMidiDriver::_adlibOperatorsTable[] = { 0, 1, 2, 8, 9, 10, 16, 17, 18 };

const uint8 AdlibMidiDriver::_adlibChannelsMappingTable[] = { 20, 18, 21, 17 };

const int16 AdlibMidiDriver::_adlibNoteFreqTable[] = { 363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647, 686 };

const uint8 AdlibMidiDriver::_adlibPercussionsMappingTable[] = { 6, 7, 8, 8, 7 };

MidiPlayer::MidiPlayer(Audio::Mixer *mixer) : _isPlaying(false) {
	_driver = new AdlibMidiDriver(mixer);
	_driver->open();
	_parser = new MidiParser_CTMF;
	_parser->setMidiDriver(_driver);
	_parser->setTimerRate(_driver->getBaseTempo());
	_driver->setTimerCallback(this, &MidiPlayer::updateTimerCallback);
}

MidiPlayer::~MidiPlayer() {
	stopMusic();
	_driver->setTimerCallback(0, 0);
	_driver->close();
	delete _parser;
	delete _driver;
}

void MidiPlayer::playMusic(uint8 *data, uint32 size) {
	stopMusic();
	_mutex.lock();
	_isPlaying = _parser->loadMusic(data, size);
	_parser->setTrack(0);
	_driver->setInstruments(&_parser->_instruments[0]);
	_mutex.unlock();
}

void MidiPlayer::stopMusic() {
	_mutex.lock();
	if (_isPlaying) {
		_isPlaying = false;
		_parser->unloadMusic();
	}
	_mutex.unlock();
}

void MidiPlayer::updateTimer() {
	_mutex.lock();
	if (_isPlaying) {
		_parser->onTimer();
	}
	_mutex.unlock();
}

} // namespace Igor
