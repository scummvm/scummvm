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

#include "common/stdafx.h"

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

/* TODO: add support for variable sampling rate in the output device
 */

/** Reads an Apple IIGS envelope from then given stream. */
bool IIgsEnvelope::read(Common::SeekableReadStream &stream) {
	for (int segNum = 0; segNum < ENVELOPE_SEGMENT_COUNT; segNum++) {
		seg[segNum].bp  = stream.readByte();
		seg[segNum].inc = stream.readUint16BE();
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

	relPitch = stream.readUint16BE();

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
	byte priority = stream.readByte(); // Not needed? 32 in all tested data.
	bendrange     = stream.readByte();
	vibdepth      = stream.readByte();
	vibspeed      = stream.readByte();
	byte spare    = stream.readByte(); // Not needed? 0 in all tested data.
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

/** Older Apple IIGS AGI instrument set. Used only by Space Quest I (AGI v1.002). */
static const instrumentSetInfo instSetV1 = {
	1192, 26, "7ee16bbc135171ffd6b9120cc7ff1af2", "edd3bf8905d9c238e02832b732fb2e18"
};

/** Newer Apple IIGS AGI instrument set (AGI v1.003+). Used by all others than Space Quest I. */
static const instrumentSetInfo instSetV2 = {
	1292, 28, "b7d428955bb90721996de1cbca25e768", "c05fb0b0e11deefab58bc68fbd2a3d07"
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

static IIgsInstrumentHeader g_instruments[MAX_INSTRUMENTS];
static uint g_numInstruments = 0;
static int16 g_wave[SIERRASTANDARD_SIZE]; // FIXME? Should this be allocated from the heap? (Size is 128KiB)

bool SoundMgr::finalizeInstruments(Common::SeekableReadStream &uint8Wave) {
	for (uint i = 0; i < g_numInstruments; i++)
		if (!g_instruments[i].finalize(uint8Wave))
			return false;
	return true;
}

/**
 * Load an Apple IIGS AGI sample resource from the given stream and
 * create an AudioStream out of it.
 *
 * @param stream The source stream.
 * @param resnum Sound resource number. Optional. Used for error messages.
 * @return A non-null AudioStream pointer if successful, NULL otherwise.
 * @note In case of failure (i.e. NULL is returned), stream is reset back
 *       to its original position and its I/O failed -status is cleared.
 * TODO: Add better handling of invalid resource number when printing error messages.
 * TODO: Add support for looping sounds.
 * FIXME: Fix sample rate calculation, it's probably not accurate at the moment.
 */
Audio::AudioStream *SoundMgr::makeIIgsSampleStream(Common::SeekableReadStream &stream, int resnum) {
	const uint32 startPos = stream.pos();
	IIgsSampleHeader header;
	Audio::AudioStream *result = NULL;
	bool readHeaderOk = header.read(stream);

	// Check that the header was read ok and that it's of the correct type
	// and that there's room for the sample data in the stream.
	if (readHeaderOk && header.type == AGI_SOUND_SAMPLE) { // An Apple IIGS AGI sample resource
		uint32 tailLen = stream.size() - stream.pos();
		if (tailLen < header.sampleSize) { // Check if there's no room for the sample data in the stream
			// Apple IIGS Manhunter I: Sound resource 16 has only 16074 bytes
			// of sample data although header says it should have 16384 bytes.
			warning("Apple IIGS sample (%d) too short (%d bytes. Should be %d bytes). Using the part that's left", resnum, tailLen, header.sampleSize);
			header.sampleSize = (uint16) tailLen; // Use the part that's left
		}
		if (header.pitch > 0x7F) { // Check if the pitch is invalid
			warning("Apple IIGS sample (%d) has too high pitch (0x%02x)", resnum, header.pitch);
			header.pitch &= 0x7F; // Apple IIGS AGI probably did it this way too
		}
		// Allocate memory for the sample data and read it in
		byte *sampleData = (byte *) malloc(header.sampleSize);
		uint32 readBytes = stream.read(sampleData, header.sampleSize);
		if (readBytes == header.sampleSize) { // Check that we got all the data we requested
			// Create a stream out of the read sample data (Needed by the finalize-function)
			Common::MemoryReadStream sampleStream(sampleData, readBytes);
			header.finalize(sampleStream);
			// Make an audio stream from the mono, 8 bit, unsigned input data
			byte flags = Audio::Mixer::FLAG_AUTOFREE | Audio::Mixer::FLAG_UNSIGNED;
			int rate = (int) (1076 * pow(SEMITONE, header.pitch));
			result = Audio::makeLinearInputStream(sampleData, header.sampleSize, rate, flags, 0, 0);
		} else // Couldn't read enough data, so let's delete the sample data buffer
			delete sampleData;
	}

	// If couldn't make a sample out of the input stream for any reason then
	// rewind back to stream's starting position and clear I/O failed -status.
	if (result == NULL) {
		stream.seek(startPos);
		stream.clearIOFailed();
	}

	return result;
}

static int playing;
static ChannelInfo chn[NUM_CHANNELS];
static int endflag = -1;
static int playingSound = -1;
static uint8 *song;
static uint8 env;


static int16 *sndBuffer;
static int16 *waveform;

static int16 waveformRamp[WAVEFORM_SIZE] = {
	0, 8, 16, 24, 32, 40, 48, 56,
	64, 72, 80, 88, 96, 104, 112, 120,
	128, 136, 144, 152, 160, 168, 176, 184,
	192, 200, 208, 216, 224, 232, 240, 255,
	0, -248, -240, -232, -224, -216, -208, -200,
	-192, -184, -176, -168, -160, -152, -144, -136,
	-128, -120, -112, -104, -96, -88, -80, -72,
	-64, -56, -48, -40, -32, -24, -16, -8	/* Ramp up */
};

static int16 waveformSquare[WAVEFORM_SIZE] = {
	255, 230, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 110,
	-255, -230, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -110, 0, 0, 0, 0	/* Square */
};

static int16 waveformMac[WAVEFORM_SIZE] = {
	45, 110, 135, 161, 167, 173, 175, 176,
	156, 137, 123, 110, 91, 72, 35, -2,
	-60, -118, -142, -165, -170, -176, -177, -179,
	-177, -176, -164, -152, -117, -82, -17, 47,
	92, 137, 151, 166, 170, 173, 171, 169,
	151, 133, 116, 100, 72, 43, -7, -57,
	-99, -141, -156, -170, -174, -177, -178, -179,
	-175, -172, -165, -159, -137, -114, -67, -19
};

static uint16 period[] = {
	1024, 1085, 1149, 1218, 1290, 1367,
	1448, 1534, 1625, 1722, 1825, 1933
};

#if 0
static struct AgiNote playSample[] = {
	{0xff, 0x7f, 0x18, 0x00, 0x7f},
	{0xff, 0xff, 0x00, 0x00, 0x00},
	{0xff, 0xff, 0x00, 0x00, 0x00},
	{0xff, 0xff, 0x00, 0x00, 0x00}
};
#endif

static int noteToPeriod(int note) {
	return 10 * (period[note % 12] >> (note / 12 - 3));
}

void SoundMgr::unloadSound(int resnum) {
	if (_vm->_game.dirSound[resnum].flags & RES_LOADED) {
		if (_vm->_game.sounds[resnum].isPlaying()) {
			_vm->_game.sounds[resnum].stop();
		}	

		/* Release RAW data for sound */
		free(_vm->_game.sounds[resnum].rdata);
		_vm->_game.sounds[resnum].rdata = NULL;
		_vm->_game.dirSound[resnum].flags &= ~RES_LOADED;
	}
}

void SoundMgr::decodeSound(int resnum) {
#if 0
	int type, size;
	int16 *buf;
	uint8 *src;
	struct SoundIIgsSample *smp;

	debugC(3, kDebugLevelSound, "(%d)", resnum);
	type = READ_LE_UINT16(_vm->_game.sounds[resnum].rdata);

	if (type == AGI_SOUND_SAMPLE) {
		/* Convert sample data to 16 bit signed format
		 */
		smp = (struct SoundIIgsSample *)_vm->_game.sounds[resnum].rdata;
		size = ((int)smp->sizeHi << 8) + smp->sizeLo;
		src = (uint8 *)_vm->_game.sounds[resnum].rdata;
		buf = (int16 *)calloc(1, 54 + (size << 1) + 100);	/* FIXME */
		memcpy(buf, src, 54);
		for (; size--; buf[size + 54] = ((int16)src[size + 54] - 0x80) << 4);	/* FIXME */
		_vm->_game.sounds[resnum].rdata = (uint8 *) buf;
		free(src);
	}
#endif
}

void SoundMgr::startSound(int resnum, int flag) {
	int i, type;
#if 0
	struct SoundIIgsSample *smp;
#endif

	if (_vm->_game.sounds[resnum].isPlaying())
		return;

	stopSound();

	if (_vm->_game.sounds[resnum].rdata == NULL)
		return;

	type = READ_LE_UINT16(_vm->_game.sounds[resnum].rdata);

	if (type != AGI_SOUND_SAMPLE && type != AGI_SOUND_MIDI && type != AGI_SOUND_4CHN)
		return;

	_vm->_game.sounds[resnum].play();
	_vm->_game.sounds[resnum].type = type;
	playingSound = resnum;
	song = (uint8 *)_vm->_game.sounds[resnum].rdata;

	switch (type) {
#if 0
	case AGI_SOUND_SAMPLE:
		debugC(3, kDebugLevelSound, "IIGS sample");
		smp = (struct SoundIIgsSample *)_vm->_game.sounds[resnum].rdata;
		for (i = 0; i < NUM_CHANNELS; i++) {
			chn[i].type = type;
			chn[i].flags = 0;
			chn[i].ins = (int16 *)&_vm->_game.sounds[resnum].rdata[54];
			chn[i].size = ((int)smp->sizeHi << 8) + smp->sizeLo;
			chn[i].ptr = &playSample[i];
			chn[i].timer = 0;
			chn[i].vol = 0;
			chn[i].end = 0;
		}
		break;
	case AGI_SOUND_MIDI:
		debugC(3, kDebugLevelSound, "IIGS MIDI sequence");

		for (i = 0; i < NUM_CHANNELS; i++) {
			chn[i].type = type;
			chn[i].flags = AGI_SOUND_LOOP | AGI_SOUND_ENVELOPE;
			chn[i].ins = waveform;
			chn[i].size = WAVEFORM_SIZE;
			chn[i].vol = 0;
			chn[i].end = 0;
		}

		chn[0].timer = *(song + 2);
		chn[0].ptr = (struct AgiNote *)(song + 3);
		break;
#endif
	case AGI_SOUND_4CHN:
		/* Initialize channel info */
		for (i = 0; i < NUM_CHANNELS; i++) {
			chn[i].type = type;
			chn[i].flags = AGI_SOUND_LOOP;
			if (env) {
				chn[i].flags |= AGI_SOUND_ENVELOPE;
				chn[i].adsr = AGI_SOUND_ENV_ATTACK;
			}
			chn[i].ins = waveform;
			chn[i].size = WAVEFORM_SIZE;
			chn[i].ptr = song + READ_LE_UINT16(song + i * 2);
			chn[i].timer = 0;
			chn[i].vol = 0;
			chn[i].end = 0;
		}
		break;
	}

	memset(sndBuffer, 0, BUFFER_SIZE << 1);
	endflag = flag;

	/* Nat Budin reports that the flag should be reset when sound starts
	 */
	_vm->setflag(endflag, false);
}

void SoundMgr::stopSound() {
	int i;

	endflag = -1;
	for (i = 0; i < NUM_CHANNELS; i++)
		stopNote(i);

	if (playingSound != -1) {
		_vm->_game.sounds[playingSound].stop();
		playingSound = -1;
	}
}

static int16 *buffer;

int SoundMgr::initSound() {
	int r = -1;

	buffer = sndBuffer = (int16 *)calloc(2, BUFFER_SIZE);

	env = false;

	switch (_vm->_soundemu) {
	case SOUND_EMU_NONE:
		waveform = waveformRamp;
		env = true;
		break;
	case SOUND_EMU_AMIGA:
	case SOUND_EMU_PC:
		waveform = waveformSquare;
		break;
	case SOUND_EMU_MAC:
		waveform = waveformMac;
		break;
	}

	report("Initializing sound:\n");

	report("sound: envelopes ");
	if (env) {
		report("enabled (decay=%d, sustain=%d)\n", ENV_DECAY, ENV_SUSTAIN);
	} else {
		report("disabled\n");
	}

	loadInstruments();

	_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, false, true);

	return r;
}

void SoundMgr::deinitSound() {
	debugC(3, kDebugLevelSound, "()");
	_mixer->stopHandle(_soundHandle);
	free(sndBuffer);
}

void SoundMgr::stopNote(int i) {
	chn[i].adsr = AGI_SOUND_ENV_RELEASE;

	if (g_useChorus) {
		/* Stop chorus ;) */
		if (chn[i].type == AGI_SOUND_4CHN &&
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

	chn[i].phase = 0;
	chn[i].freq = freq;
	chn[i].vol = vol;
	chn[i].env = 0x10000;
	chn[i].adsr = AGI_SOUND_ENV_ATTACK;

	if (g_useChorus) {
		/* Add chorus ;) */
		if (chn[i].type == AGI_SOUND_4CHN &&
			_vm->_soundemu == SOUND_EMU_NONE && i < 3) {
			int newfreq = freq * 1007 / 1000;
			if (freq == newfreq)
				newfreq++;
			playNote(i + 4, newfreq, vol * 2 / 3);
		}
	}
}

void SoundMgr::playMidiSound() {
	uint8 *p;
	uint8 parm1, parm2;
	static uint8 cmd, ch;

	playing = 1;

	if (chn[0].timer > 0) {
		chn[0].timer -= 2;
		return;
	}

	p = (uint8 *)chn[0].ptr;

	if (*p & 0x80) {
		cmd = *p++;
		ch = cmd & 0x0f;
		cmd >>= 4;
	}

	switch (cmd) {
	case 0x08:
		parm1 = *p++;
		parm2 = *p++;
		if (ch < NUM_CHANNELS)
			stopNote(ch);
		break;
	case 0x09:
		parm1 = *p++;
		parm2 = *p++;
		if (ch < NUM_CHANNELS)
			playNote(ch, noteToPeriod(parm1), 127);
		break;
	case 0x0b:
		parm1 = *p++;
		parm2 = *p++;
		debugC(3, kDebugLevelSound, "controller %02x, ch %02x, val %02x", parm1, ch, parm2);
		break;
	case 0x0c:
		parm1 = *p++;
#if 0
		if (ch < NUM_CHANNELS) {
			chn[ch].ins = (uint16 *)&wave[waveaddr[parm1]];
			chn[ch].size = wavesize[parm1];
		}
		debugC(3, kDebugLevelSound, "set patch %02x (%d,%d), ch %02x",
				parm1, waveaddr[parm1], wavesize[parm1], ch);
#endif
		break;
	}

	chn[0].timer = *p++;
	chn[0].ptr = p;

	if (*p >= 0xfc) {
		debugC(3, kDebugLevelSound, "end of sequence");
		playing = 0;
		return;
	}
}

void SoundMgr::playSampleSound() {
	playNote(0, 11025 * 10, 200);
	playing = 1;
}

void SoundMgr::playAgiSound() {
	int i;
	AgiNote note;

	for (playing = i = 0; i < (_vm->_soundemu == SOUND_EMU_PC ? 1 : 4); i++) {
		playing |= !chn[i].end;
		note.read(chn[i].ptr); // Read a single note (Doesn't advance the pointer)

		if (chn[i].end)
			continue;

		if ((--chn[i].timer) <= 0) {
			stopNote(i);

			if (note.freqDiv != 0) {
				int volume = (note.attenuation == 0x0F) ? 0 : (0xFF - note.attenuation * 2);
				playNote(i, note.freqDiv * 10, volume);
			}

			chn[i].timer = note.duration;

			if (chn[i].timer == 0xffff) {
				chn[i].end = 1;
				chn[i].vol = 0;
				chn[i].env = 0;

				if (g_useChorus) {
					/* chorus */
					if (chn[i].type == AGI_SOUND_4CHN && _vm->_soundemu == SOUND_EMU_NONE && i < 3) {
						chn[i + 4].vol = 0;
						chn[i + 4].env = 0;
					}
				}
			}
			chn[i].ptr += 5; // Advance the pointer to the next note data (5 bytes per note)
		}
	}
}

void SoundMgr::playSound() {
	int i;

	if (endflag == -1)
		return;

	if (chn[0].type == AGI_SOUND_MIDI) {
		/* play_midi_sound (); */
		playing = 0;
	} else if (chn[0].type == AGI_SOUND_SAMPLE) {
		playSampleSound();
	} else
		playAgiSound();

	if (!playing) {
		for (i = 0; i < NUM_CHANNELS; chn[i++].vol = 0);

		if (endflag != -1)
			_vm->setflag(endflag, true);

		if (playingSound != -1)
			_vm->_game.sounds[playingSound].stop();
		playingSound = -1;
		endflag = -1;
	}
}

uint32 SoundMgr::mixSound(void) {
	register int i, p;
	int16 *src;
	int c, b, m;

	memset(sndBuffer, 0, BUFFER_SIZE << 1);

	for (c = 0; c < NUM_CHANNELS; c++) {
		if (!chn[c].vol)
			continue;

		m = chn[c].flags & AGI_SOUND_ENVELOPE ?
		    chn[c].vol * chn[c].env >> 16 : chn[c].vol;

		if (chn[c].type != AGI_SOUND_4CHN || c != 3) {
			src = chn[c].ins;

			p = chn[c].phase;
			for (i = 0; i < BUFFER_SIZE; i++) {
				b = src[p >> 8];
#ifdef USE_INTERPOLATION
				b += ((src[((p >> 8) + 1) % chn[c].size] - src[p >> 8]) * (p & 0xff)) >> 8;
#endif
				sndBuffer[i] += (b * m) >> 4;

				p += (uint32) 118600 *4 / chn[c].freq;

				/* FIXME */
				if (chn[c].flags & AGI_SOUND_LOOP) {
					p %= chn[c].size << 8;
				} else {
					if (p >= chn[c].size << 8) {
						p = chn[c].vol = 0;
						chn[c].end = 1;
						break;
					}
				}

			}
			chn[c].phase = p;
		} else {
			/* Add white noise */
			for (i = 0; i < BUFFER_SIZE; i++) {
				b = _vm->_rnd->getRandomNumber(255) - 128;
				sndBuffer[i] += (b * m) >> 4;
			}
		}

		switch (chn[c].adsr) {
		case AGI_SOUND_ENV_ATTACK:
			/* not implemented */
			chn[c].adsr = AGI_SOUND_ENV_DECAY;
			break;
		case AGI_SOUND_ENV_DECAY:
			if (chn[c].env > chn[c].vol * ENV_SUSTAIN + ENV_DECAY) {
				chn[c].env -= ENV_DECAY;
			} else {
				chn[c].env = chn[c].vol * ENV_SUSTAIN;
				chn[c].adsr = AGI_SOUND_ENV_SUSTAIN;
			}
			break;
		case AGI_SOUND_ENV_SUSTAIN:
			break;
		case AGI_SOUND_ENV_RELEASE:
			if (chn[c].env >= ENV_RELEASE) {
				chn[c].env -= ENV_RELEASE;
			} else {
				chn[c].env = 0;
			}
		}
	}

	return BUFFER_SIZE;
}

#if 0
void Sound::unloadInstruments() {
	free(instruments);
}
#endif

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

bool SoundMgr::loadInstrumentHeaders(const Common::String &exePath, const IIgsExeInfo &exeInfo) {
	bool loadedOk = false; // Was loading successful?
	Common::File file;

	// Open the executable file and check that it has correct size
	file.open(exePath);
	if (file.size() != exeInfo.exeSize) {
		debugC(3, kDebugLevelSound, "Apple IIGS executable (%s) has wrong size (Is %d, should be %d)",
			exePath.c_str(), file.size(), exeInfo.exeSize);
	}

	// Read the whole executable file into memory
	Common::MemoryReadStream *data = file.readStream(file.size());
	file.close();

	// Check that we got enough data to be able to parse the instruments
	if (data != NULL && data->size() >= (exeInfo.instSetStart + exeInfo.instSet.byteCount)) {
		// Check instrument set's length (The info's saved in the executable)
		data->seek(exeInfo.instSetStart - 4);
		uint16 instSetByteCount = data->readUint16LE();
		if (instSetByteCount != exeInfo.instSet.byteCount) {
			debugC(3, kDebugLevelSound, "Wrong instrument set size (Is %d, should be %d) in Apple IIGS executable (%s)",
				instSetByteCount, exeInfo.instSet.byteCount, exePath.c_str());
		}

		// Check instrument set's md5sum
		data->seek(exeInfo.instSetStart);
		char md5str[32+1];
		Common::md5_file_string(*data, md5str, exeInfo.instSet.byteCount);
		if (scumm_stricmp(md5str, exeInfo.instSet.md5)) {
			warning("Unknown Apple IIGS instrument set (md5: %s) in %s, trying to use it nonetheless",
				md5str, exePath.c_str());
		}

		// Read in the instrument set one instrument at a time
		data->seek(exeInfo.instSetStart);
		g_numInstruments = 0; // Zero number of successfully loaded instruments
		for (uint i = 0; i < exeInfo.instSet.instCount; i++) {
			if (!g_instruments[i].read(*data)) {
				warning("Error loading Apple IIGS instrument (%d. of %d) from %s, not loading more instruments",
					i + 1, exeInfo.instSet.instCount, exePath.c_str());
				break;
			}
			g_numInstruments++; // Increase number of successfully loaded instruments
		}

		// Loading was successful only if all instruments were loaded successfully
		loadedOk = (g_numInstruments == exeInfo.instSet.instCount);
	} else // Couldn't read enough data from the executable file
		warning("Error loading instruments from Apple IIGS executable (%s)", exePath.c_str());

	delete data; // Free the memory buffer allocated for reading the executable file
	return loadedOk;
}

/**
 * Convert sample from 8-bit unsigned to 16-bit signed format.
 * @param source  Source stream containing the 8-bit unsigned sample data.
 * @param dest  Destination buffer for the 16-bit signed sample data.
 * @param length  Length of the sample data to be converted.
 */
bool SoundMgr::convertWave(Common::SeekableReadStream &source, int16 *dest, uint length) {
	// Convert the wave from 8-bit unsigned to 16-bit signed format
	for (uint i = 0; i < length; i++)
		dest[i] = (int16) ((source.readByte() - 128) * 256);
	return !source.ioFailed();
}

Common::MemoryReadStream *SoundMgr::loadWaveFile(const Common::String &wavePath, const IIgsExeInfo &exeInfo) {
	bool loadedOk = false; // Was loading successful?
	Common::File file;

	// Open the wave file and read it into memory
	file.open(wavePath);
	Common::MemoryReadStream *uint8Wave = file.readStream(file.size());
	file.close();

	// Check that we got the whole wave file
	if (uint8Wave != NULL && uint8Wave->size() == SIERRASTANDARD_SIZE) {
		// Check wave file's md5sum
		char md5str[32+1];
		Common::md5_file_string(*uint8Wave, md5str, SIERRASTANDARD_SIZE);
		if (scumm_stricmp(md5str, exeInfo.instSet.waveFileMd5)) {
			warning("Unknown Apple IIGS wave file (md5: %s, game: %s).\n" \
				"Please report the information on the previous line to the ScummVM team.\n" \
				"Using the wave file as it is - music may sound weird", md5str, exeInfo.exePrefix);
		}
		return uint8Wave;
	} else { // Couldn't read the wave file or it had incorrect size
		warning("Error loading Apple IIGS wave file (%s), not loading instruments", wavePath.c_str());
		delete uint8Wave; // Free the memory buffer allocated for reading the wave file
		return NULL;
	}
}

/**
 * A function object (i.e. a functor) for testing if a FilesystemNode
 * object's name is equal (Ignoring case) to a string or to at least
 * one of the strings in a list of strings. Can be used e.g. with find_if().
 */
struct fsnodeNameEqualsIgnoreCase : public Common::UnaryFunction<const FilesystemNode&, bool> {
	fsnodeNameEqualsIgnoreCase(const Common::StringList &str) : _str(str) {}
	fsnodeNameEqualsIgnoreCase(const Common::String str) { _str.push_back(str); }
	bool operator()(const FilesystemNode &param) const {
		for (Common::StringList::const_iterator iter = _str.begin(); iter != _str.end(); iter++)
			if (param.name().equalsIgnoreCase(*iter))
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
	FSList fslist;
	FilesystemNode dir(ConfMan.get("path"));
	if (!dir.listDir(fslist, FilesystemNode::kListFilesOnly)) {
		warning("Invalid game path (\"%s\"), not loading Apple IIGS instruments", dir.path().c_str());
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
	FSList::const_iterator exeFsnode, waveFsnode;
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

	// First load the wave file and then load the instrument headers.
	// Finally fix the instruments' lengths using the wave file data
	// (A zero in the wave file data can end the sample prematurely)
	// and convert the wave file from 8-bit unsigned to 16-bit signed format.
	Common::MemoryReadStream *uint8Wave = loadWaveFile(waveFsnode->path(), *exeInfo);
	bool result = uint8Wave != NULL && loadInstrumentHeaders(exeFsnode->path(), *exeInfo) &&
		finalizeInstruments(*uint8Wave) && convertWave(*uint8Wave, g_wave, uint8Wave->size());

	delete uint8Wave; // Free the 8-bit unsigned wave file buffer
	return result;
}

static void fillAudio(void *udata, int16 *stream, uint len) {
	SoundMgr *soundMgr = (SoundMgr *)udata;
	uint32 p = 0;
	static uint32 n = 0, s = 0;

	len <<= 2;

	debugC(5, kDebugLevelSound, "(%p, %p, %d)", (void *)udata, (void *)stream, len);
	memcpy(stream, (uint8 *)buffer + s, p = n);
	for (n = 0, len -= p; n < len; p += n, len -= n) {
		soundMgr->playSound();
		n = soundMgr->mixSound() << 1;
		if (len < n) {
			memcpy((uint8 *)stream + p, buffer, len);
			s = len;
			n -= s;
			return;
		} else {
			memcpy((uint8 *)stream + p, buffer, n);
		}
	}
	soundMgr->playSound();
	n = soundMgr->mixSound() << 1;
	memcpy((uint8 *)stream + p, buffer, s = len);
	n -= s;
}

SoundMgr::SoundMgr(AgiEngine *agi, Audio::Mixer *pMixer) {
	_vm = agi;
	_mixer = pMixer;
	_sampleRate = pMixer->getOutputRate();
}

void SoundMgr::premixerCall(int16 *data, uint len) {
	fillAudio(this, data, len);
}

void SoundMgr::setVolume(uint8 volume) {
	// TODO
}

SoundMgr::~SoundMgr() {
}

} // End of namespace Agi
