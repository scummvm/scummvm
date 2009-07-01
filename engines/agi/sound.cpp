/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/md5.h"
#include "common/config-manager.h"
#include "common/fs.h"
#include "common/algorithm.h"
#include "sound/mixer.h"

#include "agi/agi.h"
#include "agi/sound.h"

namespace Agi {

#define USE_INTERPOLATION
static bool g_useChorus = true;

//
// TODO: add support for variable sampling rate in the output device
//

AgiSound *AgiSound::createFromRawResource(uint8 *data, uint32 len, int resnum, SoundMgr &manager) {
	if (data == NULL || len < 2) // Check for too small resource or no resource at all
		return NULL;
	uint16 type = READ_LE_UINT16(data);

	switch (type) { // Create a sound object based on the type
	case AGI_SOUND_SAMPLE:
		return new IIgsSample(data, len, resnum, manager);
	case AGI_SOUND_MIDI:
		return new IIgsMidi  (data, len, resnum, manager);
	case AGI_SOUND_4CHN:
		return new PCjrSound (data, len, resnum, manager);
	}

	warning("Sound resource (%d) has unknown type (0x%04x). Not using the sound", resnum, type);
	return NULL;
}

IIgsMidi::IIgsMidi(uint8 *data, uint32 len, int resnum, SoundMgr &manager) : AgiSound(manager) {
	_data = data; // Save the resource pointer
	_ptr = _data + 2; // Set current position to just after the header
	_len  = len;  // Save the resource's length
	_type = READ_LE_UINT16(data); // Read sound resource's type
	_midiTicks = _soundBufTicks = 0;
	_isValid = (_type == AGI_SOUND_MIDI) && (_data != NULL) && (_len >= 2);

	if (!_isValid) // Check for errors
		warning("Error creating Apple IIGS midi sound from resource %d (Type %d, length %d)", resnum, _type, len);
}

PCjrSound::PCjrSound(uint8 *data, uint32 len, int resnum, SoundMgr &manager) : AgiSound(manager) {
	_data = data; // Save the resource pointer
	_len  = len;  // Save the resource's length
	_type = READ_LE_UINT16(data); // Read sound resource's type
	_isValid = (_type == AGI_SOUND_4CHN) && (_data != NULL) && (_len >= 2);

	if (!_isValid) // Check for errors
		warning("Error creating PCjr 4-channel sound from resource %d (Type %d, length %d)", resnum, _type, len);
}

const uint8 *PCjrSound::getVoicePointer(uint voiceNum) {
	assert(voiceNum < 4);
	uint16 voiceStartOffset = READ_LE_UINT16(_data + voiceNum * 2);

	return _data + voiceStartOffset;
}

IIgsSample::IIgsSample(uint8 *data, uint32 len, int resnum, SoundMgr &manager) : AgiSound(manager) {
	Common::MemoryReadStream stream(data, len, true);

	// Check that the header was read ok and that it's of the correct type
	if (_header.read(stream) && _header.type == AGI_SOUND_SAMPLE) { // An Apple IIGS AGI sample resource
		uint32 sampleStartPos = stream.pos();
		uint32 tailLen = stream.size() - sampleStartPos;

		if (tailLen < _header.sampleSize) { // Check if there's no room for the sample data in the stream
			// Apple IIGS Manhunter I: Sound resource 16 has only 16074 bytes
			// of sample data although header says it should have 16384 bytes.
			warning("Apple IIGS sample (%d) too short (%d bytes. Should be %d bytes). Using the part that's left",
				resnum, tailLen, _header.sampleSize);

			_header.sampleSize = (uint16) tailLen; // Use the part that's left
		}

		if (_header.pitch > 0x7F) { // Check if the pitch is invalid
			warning("Apple IIGS sample (%d) has too high pitch (0x%02x)", resnum, _header.pitch);

			_header.pitch &= 0x7F; // Apple IIGS AGI probably did it this way too
		}

		// Finalize the header info using the 8-bit unsigned sample data
		_header.finalize(stream);

		// Convert sample data from 8-bit unsigned to 8-bit signed format
		stream.seek(sampleStartPos);
		_sample = new int8[_header.sampleSize];

		if (_sample != NULL)
			_isValid = SoundMgr::convertWave(stream, _sample, _header.sampleSize);
	}

	if (!_isValid) // Check for errors
		warning("Error creating Apple IIGS sample from resource %d (Type %d, length %d)", resnum, _header.type, len);
}

/** Reads an Apple IIGS envelope from then given stream. */
bool IIgsEnvelope::read(Common::SeekableReadStream &stream) {
	for (int segNum = 0; segNum < ENVELOPE_SEGMENT_COUNT; segNum++) {
		seg[segNum].bp  = stream.readByte();
		seg[segNum].inc = stream.readUint16LE();
	}

	return !stream.ioFailed();
}

/** Reads an Apple IIGS wave information structure from the given stream. */
bool IIgsWaveInfo::read(Common::SeekableReadStream &stream, bool ignoreAddr) {
	top  = stream.readByte();
	addr = stream.readByte() * 256;
	size = (1 << (stream.readByte() & 7)) * 256;

	// Read packed mode byte and parse it into parts
	byte packedModeByte = stream.readByte();
	channel = (packedModeByte >> 4) & 1; // Bit 4
	mode    = (packedModeByte >> 1) & 3; // Bits 1-2
	halt    = (packedModeByte & 1) != 0; // Bit 0 (Converted to boolean)

	relPitch = stream.readSint16LE();

	// Zero the wave address if we want to ignore the wave address info
	if (ignoreAddr)
		addr = 0;

	return !stream.ioFailed();
}

bool IIgsWaveInfo::finalize(Common::SeekableReadStream &uint8Wave) {
	uint32 startPos = uint8Wave.pos(); // Save stream's starting position
	uint8Wave.seek(addr, SEEK_CUR); // Seek to wave's address

	// Calculate the true sample size (A zero ends the sample prematurely)
	uint trueSize = size; // Set a default value for the result
	for (uint i = 0; i < size; i++) {
		if (uint8Wave.readByte() == 0) {
			trueSize = i;
			// A zero in the sample stream turns off looping
			// (At least that's what MESS 0.117 and KEGS32 0.91 seem to do)
			if (mode == OSC_MODE_LOOP)
				mode = OSC_MODE_ONESHOT;
			break;
		}
	}
	size = trueSize; // Set the true sample size

	uint8Wave.seek(startPos); // Seek back to the stream's starting position

	return true;
}

bool IIgsOscillator::finalize(Common::SeekableReadStream &uint8Wave) {
	for (uint i = 0; i < WAVES_PER_OSCILLATOR; i++)
		if (!waves[i].finalize(uint8Wave))
			return false;

	return true;
}

bool IIgsOscillatorList::read(Common::SeekableReadStream &stream, uint oscillatorCount, bool ignoreAddr) {
	// First read the A waves and then the B waves for the oscillators
	for (uint waveNum = 0; waveNum < WAVES_PER_OSCILLATOR; waveNum++)
		for (uint oscNum = 0; oscNum < oscillatorCount; oscNum++)
			if (!osc[oscNum].waves[waveNum].read(stream, ignoreAddr))
				return false;

	count = oscillatorCount; // Set the oscillator count

	return true;
}

bool IIgsOscillatorList::finalize(Common::SeekableReadStream &uint8Wave) {
	for (uint i = 0; i < count; i++)
		if (!osc[i].finalize(uint8Wave))
			return false;

	return true;
}

bool IIgsInstrumentHeader::read(Common::SeekableReadStream &stream, bool ignoreAddr) {
	env.read(stream);
	relseg        = stream.readByte();
	/*byte priority =*/ stream.readByte(); // Not needed? 32 in all tested data.
	bendrange     = stream.readByte();
	vibdepth      = stream.readByte();
	vibspeed      = stream.readByte();
	/*byte spare    =*/ stream.readByte(); // Not needed? 0 in all tested data.
	byte wac      = stream.readByte(); // Read A wave count
	byte wbc      = stream.readByte(); // Read B wave count
	oscList.read(stream, wac, ignoreAddr); // Read the oscillators
	return (wac == wbc) && !stream.ioFailed(); // A and B wave counts must match
}

bool IIgsInstrumentHeader::finalize(Common::SeekableReadStream &uint8Wave) {
	return oscList.finalize(uint8Wave);
}

bool IIgsSampleHeader::read(Common::SeekableReadStream &stream) {
	type             = stream.readUint16LE();
	pitch            = stream.readByte();
	unknownByte_Ofs3 = stream.readByte();
	volume           = stream.readByte();
	unknownByte_Ofs5 = stream.readByte();
	instrumentSize   = stream.readUint16LE();
	sampleSize       = stream.readUint16LE();
	// Read the instrument header *ignoring* its wave address info

	return instrument.read(stream, true);
}

bool IIgsSampleHeader::finalize(Common::SeekableReadStream &uint8Wave) {
	return instrument.finalize(uint8Wave);
}

/** Older Apple IIGS AGI MIDI program change to instrument number mapping. */
static const MidiProgramChangeMapping progToInstMappingV1 = {
	{19, 20, 22, 23, 21, 24, 5, 5, 5, 5,
	6, 7, 10, 9, 11, 9, 15, 8, 5, 5,
	17, 16, 18, 12, 14, 5, 5, 5, 5, 5,
	0, 1, 2, 9, 3, 4, 15, 2, 2, 2,
	25, 13, 13, 25},
	5
};

/** Newer Apple IIGS AGI MIDI program change to instrument number mapping. */
static const MidiProgramChangeMapping progToInstMappingV2 = {
	{21, 22, 24, 25, 23, 26, 6, 6, 6, 6,
	7, 9, 12, 8, 13, 11, 17, 10, 6, 6,
	19, 18, 20, 14, 16, 6, 6, 6, 6, 6,
	0, 1, 2, 4, 3, 5, 17, 2, 2, 2,
	27, 15, 15, 27},
	6
};

/** Older Apple IIGS AGI instrument set. Used only by Space Quest I (AGI v1.002). */
static const InstrumentSetInfo instSetV1 = {
	1192, 26, "7ee16bbc135171ffd6b9120cc7ff1af2", "edd3bf8905d9c238e02832b732fb2e18", progToInstMappingV1
};

/** Newer Apple IIGS AGI instrument set (AGI v1.003+). Used by all others than Space Quest I. */
static const InstrumentSetInfo instSetV2 = {
	1292, 28, "b7d428955bb90721996de1cbca25e768", "c05fb0b0e11deefab58bc68fbd2a3d07", progToInstMappingV2
};

/** Information about different Apple IIGS AGI executables. */
static const IIgsExeInfo IIgsExeInfos[] = {
	{GID_SQ1,      "SQ",   0x1002, 138496, 0x80AD, instSetV1},
	{GID_LSL1,     "LL",   0x1003, 141003, 0x844E, instSetV2},
	{GID_AGIDEMO,  "DEMO", 0x1005, 141884, 0x8469, instSetV2},
	{GID_KQ1,      "KQ",   0x1006, 141894, 0x8469, instSetV2},
	{GID_PQ1,      "PQ",   0x1007, 141882, 0x8469, instSetV2},
	{GID_MIXEDUP,  "MG",   0x1013, 142552, 0x84B7, instSetV2},
	{GID_KQ2,      "KQ2",  0x1013, 143775, 0x84B7, instSetV2},
	{GID_KQ3,      "KQ3",  0x1014, 144312, 0x84B7, instSetV2},
	{GID_SQ2,      "SQ2",  0x1014, 107882, 0x6563, instSetV2},
	{GID_MH1,      "MH",   0x2004, 147678, 0x8979, instSetV2},
	{GID_KQ4,      "KQ4",  0x2006, 147652, 0x8979, instSetV2},
	{GID_BC,       "BC",   0x3001, 148192, 0x8979, instSetV2},
	{GID_GOLDRUSH, "GR",   0x3003, 148268, 0x8979, instSetV2}
};

static const int16 waveformRamp[WAVEFORM_SIZE] = {
	0, 8, 16, 24, 32, 40, 48, 56,
	64, 72, 80, 88, 96, 104, 112, 120,
	128, 136, 144, 152, 160, 168, 176, 184,
	192, 200, 208, 216, 224, 232, 240, 255,
	0, -248, -240, -232, -224, -216, -208, -200,
	-192, -184, -176, -168, -160, -152, -144, -136,
	-128, -120, -112, -104, -96, -88, -80, -72,
	-64, -56, -48, -40, -32, -24, -16, -8	// Ramp up
};

static const int16 waveformSquare[WAVEFORM_SIZE] = {
	255, 230, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 110,
	-255, -230, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -110, 0, 0, 0, 0	// Square
};

static const int16 waveformMac[WAVEFORM_SIZE] = {
	45, 110, 135, 161, 167, 173, 175, 176,
	156, 137, 123, 110, 91, 72, 35, -2,
	-60, -118, -142, -165, -170, -176, -177, -179,
	-177, -176, -164, -152, -117, -82, -17, 47,
	92, 137, 151, 166, 170, 173, 171, 169,
	151, 133, 116, 100, 72, 43, -7, -57,
	-99, -141, -156, -170, -174, -177, -178, -179,
	-175, -172, -165, -159, -137, -114, -67, -19
};

static const uint16 period[] = {
	1024, 1085, 1149, 1218, 1290, 1367,
	1448, 1534, 1625, 1722, 1825, 1933
};

#if 0
static int noteToPeriod(int note) {
	return 10 * (period[note % 12] >> (note / 12 - 3));
}
#endif

void SoundMgr::unloadSound(int resnum) {
	if (_vm->_game.dirSound[resnum].flags & RES_LOADED) {
		if (_vm->_game.sounds[resnum]->isPlaying()) {
			_vm->_game.sounds[resnum]->stop();
		}

		// Release the sound resource's data
		delete _vm->_game.sounds[resnum];
		_vm->_game.sounds[resnum] = NULL;
		_vm->_game.dirSound[resnum].flags &= ~RES_LOADED;
	}
}

void SoundMgr::startSound(int resnum, int flag) {
	int i;
	AgiSoundType type;

	if (_vm->_game.sounds[resnum] != NULL && _vm->_game.sounds[resnum]->isPlaying())
		return;

	stopSound();

	if (_vm->_game.sounds[resnum] == NULL) // Is this needed at all?
		return;

	type = (AgiSoundType)_vm->_game.sounds[resnum]->type();

	if (type != AGI_SOUND_SAMPLE && type != AGI_SOUND_MIDI && type != AGI_SOUND_4CHN)
		return;

	_vm->_game.sounds[resnum]->play();
	_playingSound = resnum;

	debugC(3, kDebugLevelSound, "startSound(resnum = %d, flag = %d) type = %d", resnum, flag, type);

	switch (type) {
	case AGI_SOUND_SAMPLE: {
		IIgsSample *sampleRes = (IIgsSample *) _vm->_game.sounds[_playingSound];
		_gsSound.playSampleSound(sampleRes->getHeader(), sampleRes->getSample());
		break;
	}
	case AGI_SOUND_MIDI:
		((IIgsMidi *) _vm->_game.sounds[_playingSound])->rewind();
		break;
	case AGI_SOUND_4CHN:
		PCjrSound *pcjrSound = (PCjrSound *) _vm->_game.sounds[resnum];

		// Initialize channel info
		for (i = 0; i < NUM_CHANNELS; i++) {
			_chn[i].type = type;
			_chn[i].flags = AGI_SOUND_LOOP;

			if (_env) {
				_chn[i].flags |= AGI_SOUND_ENVELOPE;
				_chn[i].adsr = AGI_SOUND_ENV_ATTACK;
			}

			_chn[i].ins = _waveform;
			_chn[i].size = WAVEFORM_SIZE;
			_chn[i].ptr = pcjrSound->getVoicePointer(i % 4);
			_chn[i].timer = 0;
			_chn[i].vol = 0;
			_chn[i].end = 0;
		}
		break;
	}

	memset(_sndBuffer, 0, BUFFER_SIZE << 1);
	_endflag = flag;

	// Nat Budin reports that the flag should be reset when sound starts
	_vm->setflag(_endflag, false);
}

void SoundMgr::stopSound() {
	int i;

	debugC(3, kDebugLevelSound, "stopSound() --> %d", _playingSound);

	_endflag = -1;
	if (_vm->_soundemu != SOUND_EMU_APPLE2GS) {
		for (i = 0; i < NUM_CHANNELS; i++)
			stopNote(i);
	}

	if (_playingSound != -1) {
		if (_vm->_game.sounds[_playingSound]) // sanity checking
			_vm->_game.sounds[_playingSound]->stop();

		if (_vm->_soundemu == SOUND_EMU_APPLE2GS) {
			_gsSound.stopSounds();
		}

		_playingSound = -1;
	}
}

void IIgsSoundMgr::stopSounds() {
	// Stops all sounds on all MIDI channels
	for (iterator iter = _midiChannels.begin(); iter != _midiChannels.end(); iter++)
		iter->stopSounds();
}

bool IIgsSoundMgr::playSampleSound(const IIgsSampleHeader &sampleHeader, const int8 *sample) {
	stopSounds();
	IIgsMidiChannel &channel = _midiChannels[kSfxMidiChannel];

	channel.setInstrument(&sampleHeader.instrument, sample);
	channel.setVolume(sampleHeader.volume);
	channel.noteOn(sampleHeader.pitch, 64); // Use default velocity (i.e. 64)

	return true;
}

void IIgsMidiChannel::stopSounds() {
	// Stops all sounds on this single MIDI channel
	for (iterator iter = _gsChannels.begin(); iter != _gsChannels.end(); iter++)
		iter->stop();

	_gsChannels.clear();
}

int SoundMgr::initSound() {
	int r = -1;

	memset(_sndBuffer, 0, BUFFER_SIZE << 1);
	_env = false;

	switch (_vm->_soundemu) {
	case SOUND_EMU_NONE:
		_waveform = waveformRamp;
		_env = true;
		break;
	case SOUND_EMU_AMIGA:
	case SOUND_EMU_PC:
		_waveform = waveformSquare;
		break;
	case SOUND_EMU_MAC:
		_waveform = waveformMac;
		break;
	case SOUND_EMU_APPLE2GS:
		_disabledMidi = !loadInstruments();
		break;
	case SOUND_EMU_COCO3:
		break;
	}

	report("Initializing sound:\n");

	report("sound: envelopes ");
	if (_env) {
		report("enabled (decay=%d, sustain=%d)\n", ENV_DECAY, ENV_SUSTAIN);
	} else {
		report("disabled\n");
	}

	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, false, true);

	return r;
}

void SoundMgr::deinitSound() {
	debugC(3, kDebugLevelSound, "()");

	_mixer->stopHandle(_soundHandle);
}

void SoundMgr::stopNote(int i) {
	_chn[i].adsr = AGI_SOUND_ENV_RELEASE;

	if (g_useChorus) {
		// Stop chorus ;)
		if (_chn[i].type == AGI_SOUND_4CHN &&
			_vm->_soundemu == SOUND_EMU_NONE && i < 3) {
			stopNote(i + 4);
		}
	}
}

void SoundMgr::playNote(int i, int freq, int vol) {
	if (!_vm->getflag(fSoundOn))
		vol = 0;
	else if (vol && _vm->_soundemu == SOUND_EMU_PC)
		vol = 160;

	_chn[i].phase = 0;
	_chn[i].freq = freq;
	_chn[i].vol = vol;
	_chn[i].env = 0x10000;
	_chn[i].adsr = AGI_SOUND_ENV_ATTACK;

	if (g_useChorus) {
		// Add chorus ;)
		if (_chn[i].type == AGI_SOUND_4CHN &&
			_vm->_soundemu == SOUND_EMU_NONE && i < 3) {

			int newfreq = freq * 1007 / 1000;

			if (freq == newfreq)
				newfreq++;

			playNote(i + 4, newfreq, vol * 2 / 3);
		}
	}
}

void SoundMgr::playMidiSound() {
	if (_disabledMidi)
		return;

	const uint8 *p;
	uint8 parm1, parm2;
	static uint8 cmd, ch;

	if (_playingSound == -1 || _vm->_game.sounds[_playingSound] == NULL) {
		warning("Error playing Apple IIGS MIDI sound resource");
		_playing = false;

		return;
	}

	IIgsMidi *midiObj = (IIgsMidi *) _vm->_game.sounds[_playingSound];

	_playing = true;
	p = midiObj->getPtr();

	midiObj->_soundBufTicks++;

	while (true) {
		uint8 readByte = *p;

		// Check for end of MIDI sequence marker (Can also be here before delta-time)
		if (readByte == MIDI_BYTE_STOP_SEQUENCE) {
			debugC(3, kDebugLevelSound, "End of MIDI sequence (Before reading delta-time)");
			_playing = false;

			midiObj->rewind();

			return;
		} else if (readByte == MIDI_BYTE_TIMER_SYNC) {
			debugC(3, kDebugLevelSound, "Timer sync");
			p++; // Jump over the timer sync byte as it's not needed

			continue;
		}

		uint8 deltaTime = readByte;
		if (midiObj->_midiTicks + deltaTime > midiObj->_soundBufTicks) {
			break;
		}
		midiObj->_midiTicks += deltaTime;
		p++; // Jump over the delta-time byte as it was already taken care of

		// Check for end of MIDI sequence marker (This time it after reading delta-time)
		if (*p == MIDI_BYTE_STOP_SEQUENCE) {
			debugC(3, kDebugLevelSound, "End of MIDI sequence (After reading delta-time)");
			_playing = false;

			midiObj->rewind();

			return;
		}

		// Separate byte into command and channel if it's a command byte.
		// Otherwise use running status (i.e. previously set command and channel).
		if (*p & 0x80) {
			cmd = *p++;
			ch = cmd & 0x0f;
			cmd >>= 4;
		}

		switch (cmd) {
		case MIDI_CMD_NOTE_OFF:
			parm1 = *p++;
			parm2 = *p++;
			_gsSound.midiNoteOff(ch, parm1, parm2);
			break;
		case MIDI_CMD_NOTE_ON:
			parm1 = *p++;
			parm2 = *p++;
			_gsSound.midiNoteOn(ch, parm1, parm2);
			break;
		case MIDI_CMD_CONTROLLER:
			parm1 = *p++;
			parm2 = *p++;
			_gsSound.midiController(ch, parm1, parm2);
			break;
		case MIDI_CMD_PROGRAM_CHANGE:
			parm1 = *p++;
			_gsSound.midiProgramChange(ch, parm1);
			break;
		case MIDI_CMD_PITCH_WHEEL:
			parm1 = *p++;
			parm2 = *p++;

			uint16 wheelPos = ((parm2 & 0x7F) << 7) | (parm1 & 0x7F); // 14-bit value
			_gsSound.midiPitchWheel(wheelPos);
			break;
		}
	}

	midiObj->setPtr(p);
}

void IIgsSoundMgr::midiNoteOff(uint8 channel, uint8 note, uint8 velocity) {
	_midiChannels[channel].noteOff(note, velocity);
	debugC(3, kDebugLevelSound, "note off, channel %02x, note %02x, velocity %02x", channel, note, velocity);
}

void IIgsSoundMgr::midiNoteOn(uint8 channel, uint8 note, uint8 velocity) {
	_midiChannels[channel].noteOn(note, velocity);
	debugC(3, kDebugLevelSound, "note  on, channel %02x, note %02x, velocity %02x", channel, note, velocity);
}

// TODO: Check if controllers behave differently on different MIDI channels
// TODO: Doublecheck what other controllers than the volume controller do
void IIgsSoundMgr::midiController(uint8 channel, uint8 controller, uint8 value) {
	IIgsMidiChannel &midiChannel = _midiChannels[channel];

	// The tested Apple IIGS AGI MIDI resources only used
	// controllers 0 (Bank select?), 7 (Volume) and 64 (Sustain On/Off).
	// Controller 0's parameter was in range 94-127,
	// controller 7's parameter was in range 0-127 and
	// controller 64's parameter was always 0 (i.e. sustain off).
	bool unimplemented = false;
	switch (controller) {
	case 7: // Volume
		midiChannel.setVolume(value);
		break;
	default:
		unimplemented = true;
		break;
	}
	debugC(3, kDebugLevelSound, "controller %02x, ch %02x, val %02x%s", controller, channel, value, unimplemented ? " (Unimplemented)" : "");
}

void IIgsSoundMgr::midiProgramChange(uint8 channel, uint8 program) {
	_midiChannels[channel].setInstrument(getInstrument(program), _wave.begin());
	debugC(3, kDebugLevelSound, "program change %02x, channel %02x", program, channel);
}

void IIgsSoundMgr::midiPitchWheel(uint8 wheelPos) {
	// In all the tested Apple IIGS AGI MIDI resources
	// pitch wheel commands always used 0x2000 (Center position).
	// Therefore it should be quite safe to ignore this command.
	debugC(3, kDebugLevelSound, "pitch wheel position %04x (Unimplemented)", wheelPos);
}

IIgsSoundMgr::IIgsSoundMgr() {
	_midiChannels.resize(16); // Set the amount of available MIDI channels
}

const IIgsInstrumentHeader* IIgsSoundMgr::getInstrument(uint8 program) const {
	return &_instruments[_midiProgToInst->map(program)];
}

void IIgsSoundMgr::setProgramChangeMapping(const MidiProgramChangeMapping *mapping) {
	_midiProgToInst = mapping;
}

void IIgsSoundMgr::removeStoppedSounds() {
	for (Common::Array<IIgsMidiChannel>::iterator iter = _midiChannels.begin(); iter != _midiChannels.end(); iter++)
		iter->removeStoppedSounds();
}

void IIgsMidiChannel::removeStoppedSounds() {
	for (int i = _gsChannels.size() - 1; i >= 0; i--)
		if (!_gsChannels[i].playing())
			_gsChannels.remove_at(i);
}

uint IIgsSoundMgr::activeSounds() const {
	uint result = 0;

	for (Common::Array<IIgsMidiChannel>::const_iterator iter = _midiChannels.begin(); iter != _midiChannels.end(); iter++)
		result += iter->activeSounds();

	return result;
}

uint IIgsMidiChannel::activeSounds() const {
	uint result = 0;

	for (const_iterator iter = _gsChannels.begin(); iter != _gsChannels.end(); iter++)
		if (!iter->end)
			result++;

	return result;
}

void IIgsMidiChannel::setInstrument(const IIgsInstrumentHeader *instrument, const int8 *sample) {
	_instrument = instrument;
	_sample = sample;

	// Set program on each Apple IIGS channel playing on this MIDI channel
	for (iterator iter = _gsChannels.begin(); iter != _gsChannels.end(); iter++)
		iter->setInstrument(instrument, sample);
}

void IIgsMidiChannel::setVolume(uint8 volume) {
	_volume = volume;

	// Set volume on each Apple IIGS channel playing on this MIDI channel
	for (iterator iter = _gsChannels.begin(); iter != _gsChannels.end(); iter++)
		iter->setChannelVolume(volume);
}

void IIgsMidiChannel::noteOff(uint8 note, uint8 velocity) {
	// Go through all the notes playing on this MIDI channel
	// and turn off the ones that are playing the given note
	for (iterator iter = _gsChannels.begin(); iter != _gsChannels.end(); iter++)
		if (iter->origNote == note)
			iter->noteOff(velocity);
}

void IIgsMidiChannel::noteOn(uint8 note, uint8 velocity) {
	IIgsChannelInfo channel;

	// Use the default channel volume and instrument
	channel.setChannelVolume(_volume);
	channel.setInstrument(_instrument, _sample);

	// Set the note on and save the channel
	channel.noteOn(note, velocity);
	_gsChannels.push_back(channel);
}

void IIgsChannelInfo::rewind() {
	this->envVol = this->startEnvVol;
	this->envSeg = 0;
	this->pos = intToFrac(0);
}

void IIgsChannelInfo::setChannelVolume(uint8 volume) {
	this->chanVol = intToFrac(volume);
}

void IIgsChannelInfo::setInstrument(const IIgsInstrumentHeader *instrument, const int8 *sample) {
	assert(instrument != NULL && sample != NULL);
	this->ins = instrument;
	this->unrelocatedSample = sample;
}

// TODO/FIXME: Implement correctly and fully (Take velocity into account etc)
void IIgsChannelInfo::noteOn(uint8 noteParam, uint8 velocity) {
	this->origNote = noteParam;
	this->startEnvVol = intToFrac(0);
	rewind();

	const IIgsWaveInfo *waveInfo = NULL;

	for (uint i = 0; i < ins->oscList.count; i++)
		if (ins->oscList(i).waves[0].top >= noteParam)
			waveInfo = &ins->oscList(i).waves[0];

	assert(waveInfo != NULL);

	this->relocatedSample = this->unrelocatedSample + waveInfo->addr;
	this->posAdd  = intToFrac(0);
	this->note    = intToFrac(noteParam) + doubleToFrac(waveInfo->relPitch/256.0);
	this->vol     = doubleToFrac(fracToDouble(this->envVol) * fracToDouble(this->chanVol) / 127.0);
	this->loop    = (waveInfo->mode == OSC_MODE_LOOP);
	this->size    = waveInfo->size - waveInfo->addr;
	this->end     = waveInfo->halt;
}

// TODO/FIXME: Implement correctly and fully (Take release time and velocity into account etc)
void IIgsChannelInfo::noteOff(uint8 velocity) {
	this->loop = false;
	this->envSeg = ins->relseg;
}

void IIgsChannelInfo::stop() {
	this->end = true;
}

bool IIgsChannelInfo::playing() {
	return !this->end;
}

void SoundMgr::playSampleSound() {
	if (_vm->_soundemu != SOUND_EMU_APPLE2GS) {
		warning("Trying to play a sample but not using Apple IIGS sound emulation mode");
		return;
	}

	if (_playingSound != -1)
		_playing = _gsSound.activeSounds() > 0;
}

static int cocoFrequencies[] = {
	 130,  138,  146,  155,  164,  174,  184,  195,  207,  220,  233,  246,
	 261,  277,  293,  311,  329,  349,  369,  391,  415,  440,  466,  493,
	 523,  554,  587,  622,  659,  698,  739,  783,  830,  880,  932,  987,
	1046, 1108, 1174, 1244, 1318, 1396, 1479, 1567, 1661, 1760, 1864, 1975,
	2093, 2217, 2349, 2489, 2637, 2793, 2959, 3135, 3322, 3520, 3729, 3951
};

void SoundMgr::playCoCoSound() {
	int i = 0;
	CoCoNote note;

	do {
		note.read(_chn[i].ptr);

		if (note.freq != 0xff) {
			playNote(0, cocoFrequencies[note.freq], note.volume);

			uint32 start_time = _vm->_system->getMillis();

			while (_vm->_system->getMillis() < start_time + note.duration) {
                _vm->_system->updateScreen();

                _vm->_system->delayMillis(10);
			}
		}
	} while (note.freq != 0xff);
}

void SoundMgr::playAgiSound() {
	int i;
	AgiNote note;

	_playing = false;
	for (i = 0; i < (_vm->_soundemu == SOUND_EMU_PC ? 1 : 4); i++) {
		_playing |= !_chn[i].end;
		note.read(_chn[i].ptr); // Read a single note (Doesn't advance the pointer)

		if (_chn[i].end)
			continue;

		if ((--_chn[i].timer) <= 0) {
			stopNote(i);

			if (note.freqDiv != 0) {
				int volume = (note.attenuation == 0x0F) ? 0 : (0xFF - note.attenuation * 2);
				playNote(i, note.freqDiv * 10, volume);
			}

			_chn[i].timer = note.duration;

			if (_chn[i].timer == 0xffff) {
				_chn[i].end = 1;
				_chn[i].vol = 0;
				_chn[i].env = 0;

				if (g_useChorus) {
					// chorus
					if (_chn[i].type == AGI_SOUND_4CHN && _vm->_soundemu == SOUND_EMU_NONE && i < 3) {
						_chn[i + 4].vol = 0;
						_chn[i + 4].env = 0;
					}
				}
			}
			_chn[i].ptr += 5; // Advance the pointer to the next note data (5 bytes per note)
		}
	}
}

void SoundMgr::playSound() {
	int i;

	if (_endflag == -1)
		return;

	if (_vm->_soundemu == SOUND_EMU_APPLE2GS) {
		if (_playingSound != -1) {
			if (_vm->_game.sounds[_playingSound]->type() == AGI_SOUND_MIDI) {
				playMidiSound();
				//warning("playSound: Trying to play an Apple IIGS MIDI sound. Not yet implemented");
			} else if (_vm->_game.sounds[_playingSound]->type() == AGI_SOUND_SAMPLE) {
				//debugC(3, kDebugLevelSound, "playSound: Trying to play an Apple IIGS sample");
				playSampleSound();
			}
		}
	} else if (_vm->_soundemu == SOUND_EMU_COCO3) {
		playCoCoSound();
	} else {
		//debugC(3, kDebugLevelSound, "playSound: Trying to play a PCjr 4-channel sound");
		playAgiSound();
	}

	if (!_playing) {
		if (_vm->_soundemu != SOUND_EMU_APPLE2GS) {
			for (i = 0; i < NUM_CHANNELS; _chn[i++].vol = 0)
				;
		}

		if (_endflag != -1)
			_vm->setflag(_endflag, true);

		if (_playingSound != -1)
			_vm->_game.sounds[_playingSound]->stop();
		_playingSound = -1;
		_endflag = -1;
	}
}

uint32 SoundMgr::mixSound(void) {
	register int i, p;
	const int16 *src;
	int c, b, m;

	memset(_sndBuffer, 0, BUFFER_SIZE << 1);

	if (!_playing || _playingSound == -1)
		return BUFFER_SIZE;

	// Handle Apple IIGS sound mixing here
	// TODO: Implement playing both waves in an oscillator
	// TODO: Implement swap-mode in an oscillator
	if (_vm->_soundemu == SOUND_EMU_APPLE2GS) {
		for (uint midiChan = 0; midiChan < _gsSound._midiChannels.size(); midiChan++) {
			for (uint gsChan = 0; gsChan < _gsSound._midiChannels[midiChan]._gsChannels.size(); gsChan++) {
				IIgsChannelInfo &channel = _gsSound._midiChannels[midiChan]._gsChannels[gsChan];
				if (channel.playing()) { // Only mix in actively playing channels
					// Frequency multiplier was 1076.0 based on tests made with MESS 0.117.
					// Tests made with KEGS32 averaged the multiplier to around 1045.
					// So this is a guess but maybe it's 1046.5... i.e. C6's frequency?
					double hertz = C6_FREQ * pow(SEMITONE, fracToDouble(channel.note));
					channel.posAdd = doubleToFrac(hertz / getRate());
					channel.vol = doubleToFrac(fracToDouble(channel.envVol) * fracToDouble(channel.chanVol) / 127.0);
					double tempVol = fracToDouble(channel.vol)/127.0;
					for (i = 0; i < IIGS_BUFFER_SIZE; i++) {
						b = channel.relocatedSample[fracToInt(channel.pos)];
						// TODO: Find out what volume/amplification setting is loud enough
						//       but still doesn't clip when playing many channels on it.
						_sndBuffer[i] += (int16) (b * tempVol * 256/4);
						channel.pos += channel.posAdd;

						if (channel.pos >= intToFrac(channel.size)) {
							if (channel.loop) {
								// Don't divide by zero on zero length samples
								channel.pos %= intToFrac(channel.size + (channel.size == 0));
								// Probably we should loop the envelope too
								channel.envSeg = 0;
								channel.envVol = channel.startEnvVol;
							} else {
								channel.pos = channel.chanVol = 0;
								channel.end = true;
								break;
							}
						}
					}

					if (channel.envSeg < ENVELOPE_SEGMENT_COUNT) {
						const IIgsEnvelopeSegment &seg = channel.ins->env.seg[channel.envSeg];
						// I currently assume enveloping works with the same speed as the MIDI
						// (i.e. with 1/60ths of a second ticks).
						// TODO: Check if enveloping really works with the same speed as MIDI
						frac_t envVolDelta = doubleToFrac(seg.inc/256.0);
						if (intToFrac(seg.bp) >= channel.envVol) {
							channel.envVol += envVolDelta;
							if (channel.envVol >= intToFrac(seg.bp)) {
								channel.envVol = intToFrac(seg.bp);
								channel.envSeg += 1;
							}
						} else {
							channel.envVol -= envVolDelta;
							if (channel.envVol <= intToFrac(seg.bp)) {
								channel.envVol = intToFrac(seg.bp);
								channel.envSeg += 1;
							}
						}
					}
				}
			}
		}
		_gsSound.removeStoppedSounds();
		return IIGS_BUFFER_SIZE;
	} // else ...

	// Handle PCjr 4-channel sound mixing here
	for (c = 0; c < NUM_CHANNELS; c++) {
		if (!_chn[c].vol)
			continue;

		m = _chn[c].flags & AGI_SOUND_ENVELOPE ?
		    _chn[c].vol * _chn[c].env >> 16 : _chn[c].vol;

		if (_chn[c].type != AGI_SOUND_4CHN || c != 3) {
			src = _chn[c].ins;

			p = _chn[c].phase;
			for (i = 0; i < BUFFER_SIZE; i++) {
				b = src[p >> 8];
#ifdef USE_INTERPOLATION
				b += ((src[((p >> 8) + 1) % _chn[c].size] - src[p >> 8]) * (p & 0xff)) >> 8;
#endif
				_sndBuffer[i] += (b * m) >> 4;

				p += (uint32) 118600 *4 / _chn[c].freq;

				// FIXME: Fingolfin asks: why is there a FIXME here? Please either clarify what
				// needs fixing, or remove it!
				// FIXME
				if (_chn[c].flags & AGI_SOUND_LOOP) {
					p %= _chn[c].size << 8;
				} else {
					if (p >= _chn[c].size << 8) {
						p = _chn[c].vol = 0;
						_chn[c].end = 1;
						break;
					}
				}

			}
			_chn[c].phase = p;
		} else {
			// Add white noise
			for (i = 0; i < BUFFER_SIZE; i++) {
				b = _vm->_rnd->getRandomNumber(255) - 128;
				_sndBuffer[i] += (b * m) >> 4;
			}
		}

		switch (_chn[c].adsr) {
		case AGI_SOUND_ENV_ATTACK:
			// not implemented
			_chn[c].adsr = AGI_SOUND_ENV_DECAY;
			break;
		case AGI_SOUND_ENV_DECAY:
			if (_chn[c].env > _chn[c].vol * ENV_SUSTAIN + ENV_DECAY) {
				_chn[c].env -= ENV_DECAY;
			} else {
				_chn[c].env = _chn[c].vol * ENV_SUSTAIN;
				_chn[c].adsr = AGI_SOUND_ENV_SUSTAIN;
			}
			break;
		case AGI_SOUND_ENV_SUSTAIN:
			break;
		case AGI_SOUND_ENV_RELEASE:
			if (_chn[c].env >= ENV_RELEASE) {
				_chn[c].env -= ENV_RELEASE;
			} else {
				_chn[c].env = 0;
			}
		}
	}

	return BUFFER_SIZE;
}

/**
 * Finds information about an Apple IIGS AGI executable based on the game ID.
 * @return A non-null IIgsExeInfo pointer if successful, otherwise NULL.
 */
const IIgsExeInfo *SoundMgr::getIIgsExeInfo(enum AgiGameID gameid) const {
	for (int i = 0; i < ARRAYSIZE(IIgsExeInfos); i++)
		if (IIgsExeInfos[i].gameid == gameid)
			return &IIgsExeInfos[i];
	return NULL;
}

bool IIgsSoundMgr::loadInstrumentHeaders(const Common::FSNode &exePath, const IIgsExeInfo &exeInfo) {
	bool loadedOk = false; // Was loading successful?
	Common::File file;

	// Open the executable file and check that it has correct size
	file.open(exePath);
	if (file.size() != (int32)exeInfo.exeSize) {
		debugC(3, kDebugLevelSound, "Apple IIGS executable (%s) has wrong size (Is %d, should be %d)",
			exePath.getPath().c_str(), file.size(), exeInfo.exeSize);
	}

	// Read the whole executable file into memory
	Common::SharedPtr<Common::MemoryReadStream> data(file.readStream(file.size()));
	file.close();

	// Check that we got enough data to be able to parse the instruments
	if (data && data->size() >= (int32)(exeInfo.instSetStart + exeInfo.instSet.byteCount)) {
		// Check instrument set's length (The info's saved in the executable)
		data->seek(exeInfo.instSetStart - 4);
		uint16 instSetByteCount = data->readUint16LE();
		if (instSetByteCount != exeInfo.instSet.byteCount) {
			debugC(3, kDebugLevelSound, "Wrong instrument set size (Is %d, should be %d) in Apple IIGS executable (%s)",
				instSetByteCount, exeInfo.instSet.byteCount, exePath.getPath().c_str());
		}

		// Check instrument set's md5sum
		data->seek(exeInfo.instSetStart);

		char md5str[32+1];
		Common::md5_file_string(*data, md5str, exeInfo.instSet.byteCount);
		if (scumm_stricmp(md5str, exeInfo.instSet.md5)) {
			warning("Unknown Apple IIGS instrument set (md5: %s) in %s, trying to use it nonetheless",
				md5str, exePath.getPath().c_str());
		}

		// Read in the instrument set one instrument at a time
		data->seek(exeInfo.instSetStart);

		// Load the instruments
		_instruments.clear();
		_instruments.reserve(exeInfo.instSet.instCount);

		IIgsInstrumentHeader instrument;
		for (uint i = 0; i < exeInfo.instSet.instCount; i++) {
			if (!instrument.read(*data)) {
				warning("Error loading Apple IIGS instrument (%d. of %d) from %s, not loading more instruments",
					i + 1, exeInfo.instSet.instCount, exePath.getPath().c_str());
				break;
			}
			_instruments.push_back(instrument); // Add the successfully loaded instrument to the instruments array
		}

		// Loading was successful only if all instruments were loaded successfully
		loadedOk = (_instruments.size() == exeInfo.instSet.instCount);
	} else // Couldn't read enough data from the executable file
		warning("Error loading instruments from Apple IIGS executable (%s)", exePath.getPath().c_str());

	return loadedOk;
}

/**
 * Convert sample from 8-bit unsigned to 8-bit signed format.
 * @param source  Source stream containing the 8-bit unsigned sample data.
 * @param dest  Destination buffer for the 8-bit signed sample data.
 * @param length  Length of the sample data to be converted.
 */
bool SoundMgr::convertWave(Common::SeekableReadStream &source, int8 *dest, uint length) {
	// Convert the wave from 8-bit unsigned to 8-bit signed format
	for (uint i = 0; i < length; i++)
		dest[i] = (int8) ((int) source.readByte() - 128);
	return !source.ioFailed();
}

bool IIgsSoundMgr::loadWaveFile(const Common::FSNode &wavePath, const IIgsExeInfo &exeInfo) {
	Common::File file;

	// Open the wave file and read it into memory
	file.open(wavePath);
	Common::SharedPtr<Common::MemoryReadStream> uint8Wave(file.readStream(file.size()));
	file.close();

	// Check that we got the whole wave file
	if (uint8Wave && uint8Wave->size() == SIERRASTANDARD_SIZE) {
		// Check wave file's md5sum
		char md5str[32+1];
		Common::md5_file_string(*uint8Wave, md5str, SIERRASTANDARD_SIZE);
		if (scumm_stricmp(md5str, exeInfo.instSet.waveFileMd5)) {
			warning("Unknown Apple IIGS wave file (md5: %s, game: %s).\n" \
				"Please report the information on the previous line to the ScummVM team.\n" \
				"Using the wave file as it is - music may sound weird", md5str, exeInfo.exePrefix);
		}

		uint8Wave->seek(0); // Seek wave to its start
		// Convert the wave file from 8-bit unsigned to 8-bit signed and save the result
		_wave.resize(uint8Wave->size());
		return SoundMgr::convertWave(*uint8Wave, _wave.begin(), uint8Wave->size());
	} else { // Couldn't read the wave file or it had incorrect size
		warning("Error loading Apple IIGS wave file (%s), not loading instruments", wavePath.getPath().c_str());
		return false;
	}
}

/**
 * A function object (i.e. a functor) for testing if a Common::FSNode
 * object's name is equal (Ignoring case) to a string or to at least
 * one of the strings in a list of strings. Can be used e.g. with find_if().
 */
struct fsnodeNameEqualsIgnoreCase : public Common::UnaryFunction<const Common::FSNode&, bool> {
// FIXME: This should be replaced; use SearchMan instead
	fsnodeNameEqualsIgnoreCase(const Common::StringList &str) : _str(str) {}
	fsnodeNameEqualsIgnoreCase(const Common::String str) { _str.push_back(str); }
	bool operator()(const Common::FSNode &param) const {
		for (Common::StringList::const_iterator iter = _str.begin(); iter != _str.end(); iter++)
			if (param.getName().equalsIgnoreCase(*iter))
				return true;
		return false;
	}
private:
	Common::StringList _str;
};

bool SoundMgr::loadInstruments() {
	// Check that the platform is Apple IIGS, as only it uses custom instruments
	if (_vm->getPlatform() != Common::kPlatformApple2GS) {
		debugC(3, kDebugLevelSound, "Platform isn't Apple IIGS so not loading any instruments");
		return true;
	}

	// Get info on the particular Apple IIGS AGI game's executable
	const IIgsExeInfo *exeInfo = getIIgsExeInfo((enum AgiGameID) _vm->getGameID());
	if (exeInfo == NULL) {
		warning("Unsupported Apple IIGS game, not loading instruments");
		return false;
	}

	// List files in the game path
	Common::FSList fslist;
	Common::FSNode dir(ConfMan.get("path"));
	if (!dir.getChildren(fslist, Common::FSNode::kListFilesOnly)) {
		warning("Invalid game path (\"%s\"), not loading Apple IIGS instruments", dir.getPath().c_str());
		return false;
	}

	// Populate executable filenames list (Long filename and short filename) for searching
	Common::StringList exeNames;
	exeNames.push_back(Common::String(exeInfo->exePrefix) + ".SYS16");
	exeNames.push_back(Common::String(exeInfo->exePrefix) + ".SYS");

	// Populate wave filenames list (Long filename and short filename) for searching
	Common::StringList waveNames;
	waveNames.push_back("SIERRASTANDARD");
	waveNames.push_back("SIERRAST");

	// Search for the executable file and the wave file (i.e. check if any of the filenames match)
	Common::FSList::const_iterator exeFsnode, waveFsnode;
	exeFsnode  = Common::find_if(fslist.begin(), fslist.end(), fsnodeNameEqualsIgnoreCase(exeNames));
	waveFsnode = Common::find_if(fslist.begin(), fslist.end(), fsnodeNameEqualsIgnoreCase(waveNames));

	// Make sure that we found the executable file
	if (exeFsnode == fslist.end()) {
		warning("Couldn't find Apple IIGS game executable (%s), not loading instruments", exeNames.begin()->c_str());
		return false;
	}

	// Make sure that we found the wave file
	if (waveFsnode == fslist.end()) {
		warning("Couldn't find Apple IIGS wave file (%s), not loading instruments", waveNames.begin()->c_str());
		return false;
	}

	// Set the MIDI program change to instrument number mapping and
	// load the instrument headers and their sample data.
	// None of the tested SIERRASTANDARD-files have zeroes in them so
	// there's no need to check for prematurely ending samples here.
	_gsSound.setProgramChangeMapping(&exeInfo->instSet.progToInst);
	return _gsSound.loadWaveFile(*waveFsnode, *exeInfo) && _gsSound.loadInstrumentHeaders(*exeFsnode, *exeInfo);
}

void SoundMgr::fillAudio(void *udata, int16 *stream, uint len) {
	SoundMgr *soundMgr = (SoundMgr *)udata;
	uint32 p = 0;

	// current number of audio bytes in _sndBuffer
	static uint32 data_available = 0;
	// offset of start of audio bytes in _sndBuffer
	static uint32 data_offset = 0;

	len <<= 2;

	debugC(5, kDebugLevelSound, "(%p, %p, %d)", (void *)udata, (void *)stream, len);

	while (len > data_available) {
		memcpy((uint8 *)stream + p, (uint8*)_sndBuffer + data_offset, data_available);
		p += data_available;
		len -= data_available;

		soundMgr->playSound();
		data_available = soundMgr->mixSound() << 1;
		data_offset = 0;
	}

	memcpy((uint8 *)stream + p, (uint8*)_sndBuffer + data_offset, len);
	data_offset += len;
	data_available -= len;
}

SoundMgr::SoundMgr(AgiBase *agi, Audio::Mixer *pMixer) : _chn() {
	_vm = agi;
	_mixer = pMixer;
	_sampleRate = pMixer->getOutputRate();
	_endflag = -1;
	_playingSound = -1;
	_env = false;
	_playing = false;
	_sndBuffer = (int16 *)calloc(2, BUFFER_SIZE);
	_waveform = 0;
	_disabledMidi = false;
}

void SoundMgr::premixerCall(int16 *data, uint len) {
	fillAudio(this, data, len);
}

void SoundMgr::setVolume(uint8 volume) {
	// TODO
}

SoundMgr::~SoundMgr() {
	free(_sndBuffer);
}

} // End of namespace Agi
