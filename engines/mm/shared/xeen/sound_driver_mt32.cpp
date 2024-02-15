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
#include "mm/shared/xeen/sound_driver_mt32.h"
#include "mm/mm.h"

namespace MM {
namespace Shared {
namespace Xeen {

const uint8 SoundDriverMT32::MIDI_NOTE_MAP[24] = {
	0x00, 0x0C, 0x0E, 0x10, 0x11, 0x13, 0x15, 0x17,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18,
	0x00, 0x0B, 0x0D, 0x0F, 0x10, 0x12, 0x14, 0x16
};

#define MT32_ADJUST_VOLUME

/*------------------------------------------------------------------------*/
static void timerCallback(void *param) {
	SoundDriverMT32 *_driver = (SoundDriverMT32*)param;
	if (!_driver || !_driver->_midiDriver)
		return;

	_driver->_timerCount += _driver->_midiDriver->getBaseTempo();
	if (_driver->_timerCount > ((float)1000000 / CALLBACKS_PER_SECOND)) {
		_driver->_timerCount -= (float)1000000 / CALLBACKS_PER_SECOND;
		_driver->onTimer();
	}
}

SoundDriverMT32::SoundDriverMT32() : _field180(0), _field181(0), _field182(0),
_musicVolume(0), _sfxVolume(0), _timerCount(0), _midiDriver(nullptr) {
	Common::fill(&_musInstrumentPtrs[0], &_musInstrumentPtrs[16], (const byte *)nullptr);
	Common::fill(&_fxInstrumentPtrs[0], &_fxInstrumentPtrs[16], (const byte *)nullptr);
	Common::fill(&_last_notes[0], &_last_notes[16], 0xFF);

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_PREFER_MT32);
	_midiDriver = MidiDriver::createMidi(dev);
	int ret = _midiDriver->open();
	if (ret) {
		error("Can't open midi device. Errorcode: %d", ret);
	} else {
		_midiDriver->setTimerCallback(this, timerCallback);
		initialize();
	}
}

SoundDriverMT32::~SoundDriverMT32() {
	if (_midiDriver) {
		_midiDriver->close();
		delete _midiDriver;
		_midiDriver = nullptr;
	}
}

void SoundDriverMT32::onTimer() {
	Common::StackLock slock(_driverMutex);
	execute();
	flush();
}

void SoundDriverMT32::initialize() {
	_midiDriver->sendMT32Reset();

	// set volume
	for (int idx = 0; idx < CHANNEL_COUNT; idx++)
	 	write(0xB1 + idx, 0x07, idx == 8 ? 0x7F : 0x4F);

	resetFrequencies();

	SoundDriverMT32::resetFX();
}

void SoundDriverMT32::playFX(uint effectId, const byte *data) {
	Common::StackLock slock(_driverMutex);

	SoundDriver::playFX(effectId, data);
}

void SoundDriverMT32::playSong(const byte *data) {
	Common::StackLock slock(_driverMutex);
	SoundDriver::playSong(data);
	_field180 = 0;
	resetFrequencies();
}

int SoundDriverMT32::songCommand(uint commandId, byte musicVolume, byte sfxVolume) {
	Common::StackLock slock(_driverMutex);
	SoundDriver::songCommand(commandId, musicVolume, sfxVolume);

	if (commandId == STOP_SONG) {
		_field180 = 0;
		resetFrequencies();
	} else if (commandId == RESTART_SONG) {
		_field180 = 0;
		_streams[stMUSIC]._playing = true;
	} else if (commandId < 0x100) {
		if (_streams[stMUSIC]._playing) {
			_field180 = commandId;
			_field182 = 0x7F;
		}
	} else if (commandId == SET_VOLUME) {
		_musicVolume = musicVolume;
		_sfxVolume = sfxVolume;
	} else if (commandId == GET_STATUS) {
		return _field180;
	}

	return 0;
}

void SoundDriverMT32::write(uint8 command, uint8 op1, uint8 op2) {
	MidiValue v(command, op1, op2);
	_queue.push(v);
	debugC(9, kDebugSound, "push %08x", v._val);
}

void SoundDriverMT32::flush() {
	Common::StackLock slock(_driverMutex);

	while (!_queue.empty()) {
		MidiValue v = _queue.pop();
		debugC(9, kDebugSound, "pop %08x", v._val);
		_midiDriver->send(v._val);
	}
}

byte SoundDriverMT32::noteMap(byte note) {
	assert((note & 0x1F) < sizeof(MIDI_NOTE_MAP));
	uint8 freq = (note & 0xE0) >> 2;
	uint8 result = MIDI_NOTE_MAP[note & 0x1F];
	result+= freq + (freq >> 1);
	return result;
}

void SoundDriverMT32::pausePostProcess() {
	if (_field180 && ((_field181 += _field180) < 0)) {
		if (--_field182 < 0) {
			_streams[stMUSIC]._playing = false;
			_field180 = 0;
			resetFrequencies();
		} else {
			for (int channelNum = 8; channelNum >= 0; --channelNum) {
				if (channelNum == 7)
					return;

				if (_channels[channelNum]._volume >= 40) {
					_channels[channelNum]._volume--;
					// set volume expression
					write(0xB1 + channelNum, 0x0B, _channels[channelNum]._volume);
				}
			}
		}
	}

	byte channelNum = 7;
	if (_channels[channelNum]._freqChange) {
		_channels[channelNum]._frequency += _channels[channelNum]._freqChange;
		// pitch bend
		write(0xE1 + channelNum, _channels[channelNum]._frequency & 0x7F, (_channels[channelNum]._frequency >> 8) & 0x7F);
	}
}

void SoundDriverMT32::resetFX() {
	// notes off on 8ch
	write(0xB8, 0x7B, 0x00);
	// pitch bend on 8ch
	write(0xE8, 0x00, 0x40);
}

void SoundDriverMT32::resetFrequencies() {
	// set pitch
	for (int idx = CHANNEL_COUNT - 1; idx >= 0; idx--)
		write(0xE1 + idx, 0x00, 0x40);
	// set pan
	for (int idx = CHANNEL_COUNT - 1; idx >= 0; idx--)
		write(0xB1 + idx, 0x0A, 0x3F);
	// notes off
	for (int idx = CHANNEL_COUNT - 1; idx >= 0; idx--)
		write(0xB1 + idx, 0x7B, 0x00);
}

void SoundDriverMT32::playInstrument(byte channelNum, const byte *data, bool isFx) {
	debugC(2, kDebugSound, "---START-playInstrument - %d", channelNum);

	write(0xC1 + channelNum, *data, 0x00);

	debugC(2, kDebugSound, "---END-playInstrument");
}

bool SoundDriverMT32::musSetInstrument(const byte *&srcP, byte param) {
	srcP += 24;
	debugC(3, kDebugSound, "musSetInstrument %d -> %x", param, *srcP);
	_musInstrumentPtrs[param] = srcP;
	srcP += 2;

	return false;
}

bool SoundDriverMT32::musSetPitchWheel(const byte *&srcP, byte param) {
	byte pitch1 = *srcP++;
	byte pitch2 = *srcP++;
	debugC(3, kDebugSound, "musSetPitchWheel");
	write(0xE1 + param, pitch1, pitch2);

	return false;
}

bool SoundDriverMT32::musSetPanning(const byte *&srcP, byte param) {
	byte pan = *srcP++;
	debugC(3, kDebugSound, "mmusSetPanning");
	write(0xB1 + param, 0x0A, pan);

	return false;
}

bool SoundDriverMT32::musFade(const byte *&srcP, byte param) {
	byte note = *srcP++;
	debugC(3, kDebugSound, "musFade: %x", note);
	if (param != 8)
		note = noteMap(note);

	write(0x81 + param, note & 0x7F, 0x40);
	_last_notes[param] = 0xFF;

	return false;
}

bool SoundDriverMT32::musStartNote(const byte *&srcP, byte param) {
	byte note = *srcP++;
	byte fade = *srcP++;

	debugC(3, kDebugSound, "musStartNote %x, %x", note, fade);
	if (param != 8) {
		note = noteMap(note);
	}

	if (param != 8) {
		if (param != 7)
			write(0x81 + param, _last_notes[param] & 0x7F, 0x7F);
		else
			write(0x81 + param, note & 0x7F, 0x7F);
	}

#if defined(MT32_ADJUST_VOLUME)
	byte level = calculateLevel(fade, false);
	if (level > 0)
		write(0x91 + param, note & 0x7F, level);
#else
	write(0x91 + param, note & 0x7F, fade);
#endif
	_last_notes[param] = note & 0x7F;

	return false;
}

bool SoundDriverMT32::musSetVolume(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "musSetVolume %d", (int)*srcP);

	uint8 status = *srcP++;
	uint8 volume = *srcP++;

	if (status == 0 && !_field180) {
		_channels[param]._volume = volume;
#if defined(MT32_ADJUST_VOLUME)
		byte level = calculateLevel(volume, true);
		write(0xB1 + param, 0x0B, level);
#else
		write(0xB1 + param, 0x0B, volume);
#endif
	}

	return false;
}

bool SoundDriverMT32::musInjectMidi(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "musInjectMidi");
	
	// TODO: When this happens? Wrap it in sysex as rolmus do
	sysExMessage(srcP);

	return false;
}

bool SoundDriverMT32::musPlayInstrument(const byte *&srcP, byte param) {
	byte instrument = *srcP++;
	debugC(3, kDebugSound, "musPlayInstrument %d -> %d", param, instrument);

	// TODO: rolmus also have condition to use _fxInstrumentPtrs instead if isFx is set in timer, ignore for now
	playInstrument(param, _musInstrumentPtrs[instrument], false);

	return false;
}

bool SoundDriverMT32::fxSetInstrument(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxSetInstrument %d -> %x", param, *srcP);
	_fxInstrumentPtrs[param] = srcP;
	++srcP;

	return false;
}

bool SoundDriverMT32::fxSetVolume(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxSetVolume %d", (int)*srcP);

	uint8 volume = *srcP++;

	if (!_field180) {
		_channels[param]._volume = volume;
#if defined(MT32_ADJUST_VOLUME)
		byte level = calculateLevel(volume, true);
		write(0xB1 + param, 0x0B, level);
#else
		write(0xB1 + param, 0x0B, volume);
#endif
	}

	return false;
}

bool SoundDriverMT32::fxMidiReset(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxMidiReset");

	_channels[param]._freqChange = 0;

	return false;
}

bool SoundDriverMT32::fxMidiDword(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxMidiDword");

	_channels[param]._freqChange = READ_LE_UINT16(*&srcP);
	srcP += 2;
	_channels[param]._frequency = READ_LE_UINT16(*&srcP);
	srcP += 2;

	return false;
}

bool SoundDriverMT32::fxSetPanning(const byte *&srcP, byte param) {
	byte pan = *srcP++;
	debugC(3, kDebugSound, "fxSetPanning - %x", pan);

	write(0xB1 + param, 0x0A, pan);

	return false;
}

bool SoundDriverMT32::fxChannelOff(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxChannelOff %d", param);

	return false;
}

bool SoundDriverMT32::fxFade(const byte *&srcP, byte param) {
	byte note = *srcP++;

	debugC(3, kDebugSound, "fxFade %d %x", param, note);
	note = noteMap(note);

	if (param == 7)
		write(0x81 + param, _last_notes[param] & 0x7F, 0x7F);
	else
		write(0x81 + param, note & 0x7F, 0x7F);
	
	_last_notes[param] = 0xFF;

	return false;
}

bool SoundDriverMT32::fxStartNote(const byte *&srcP, byte param) {
	byte note = *srcP++;
	byte fade = *srcP++;

	debugC(3, kDebugSound, "fxStartNote %x, %x", note, fade);

	if (param != 8)
		note = noteMap(note);

#if defined(MT32_ADJUST_VOLUME)
	byte level = calculateLevel(fade, true);
	if (level > 0)
		write(0x91 + param, note & 0x7F, level);
	else
		write(0x81 + param, note & 0x7F, 0x7f);
#else
		write(0x91 + param, note & 0x7F, fade);
#endif
	_last_notes[param] = note & 0x7F;
	
	return false;
}

bool SoundDriverMT32::fxInjectMidi(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxInjectMidi");

	return false;
}

bool SoundDriverMT32::fxPlayInstrument(const byte *&srcP, byte param) {
	byte instrument = *srcP++;
	debugC(3, kDebugSound, "fxPlayInstrument %d, %d", param, instrument);

	playInstrument(param, _fxInstrumentPtrs[instrument], true);

	return false;
}

byte SoundDriverMT32::calculateLevel(byte level, bool isFx) {
	uint volume = isFx ? _sfxVolume : _musicVolume;
	float scaling = 127.0f / 255.0f;
	uint totalLevel = volume * scaling;

	return totalLevel;
}

bool SoundDriverMT32::cmdFreezeFrequency(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "cmdNoOperation");

	return false;
}

bool SoundDriverMT32::cmdChangeFrequency(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "cmdChangeFrequency");

	srcP += 3;

	return false;
}

void SoundDriverMT32::sysExMessage(const byte *&data) {
	byte   sysExMessage[270];
	uint16 sysExPos      = 0;
	byte   sysExByte     = 0;
	uint16 sysExChecksum = 0;

	memset(&sysExMessage, 0, sizeof(sysExMessage));

	sysExMessage[0] = 0x41; // Roland
	sysExMessage[1] = 0x10; // Device ID
	sysExMessage[2] = 0x16; // Model MT32
	sysExMessage[3] = 0x12; // Command DT1

	sysExPos      = 4;
	sysExChecksum = 0;
	while (1) {
		sysExByte = *data++;
		if (sysExByte == 0xF7)
			break; // Message done

		// if we need bigger buffer then buffer in MidiDriver_ALSA::sysEx should also be increased!
		assert(sysExPos < sizeof(sysExMessage));
		sysExMessage[sysExPos++] = sysExByte;
		sysExChecksum -= sysExByte;
	}

	// Calculate checksum
	assert(sysExPos < sizeof(sysExMessage));
	sysExMessage[sysExPos++] = sysExChecksum & 0x7f;

	debugC(3, "sending sysex message, size %d", sysExPos);

	// Send SysEx message
	_midiDriver->sysEx(sysExMessage, sysExPos);
}

} // namespace Xeen
} // namespace Shared
} // namespace MM
