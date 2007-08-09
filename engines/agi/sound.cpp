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

#include "sound/mixer.h"

#include "agi/agi.h"
#include "agi/sound.h"

namespace Agi {

#define USE_INTERPOLATION
#define USE_CHORUS

/* TODO: add support for variable sampling rate in the output device
 */

#ifdef USE_IIGS_SOUND

struct IIgsEnvelopeSegment {
	uint8 bp;
	uint16 inc; ///< 8b.8b fixed point, big endian?
};

#define ENVELOPE_SEGMENT_COUNT 8
struct IIgsEnvelope {
	IIgsEnvelopeSegment seg[ENVELOPE_SEGMENT_COUNT];
};

// 2**(1/12) i.e. the 12th root of 2
#define SEMITONE 1.059463094359295

// Size of the SIERRASTANDARD file (i.e. the wave file i.e. the sample data used by the instruments).
#define SIERRASTANDARD_SIZE 65536

// Maximum number of instruments in an Apple IIGS instrument set.
// Chosen empirically based on Apple IIGS AGI game data, increase if needed.
#define MAX_INSTRUMENTS 28

struct IIgsWaveInfo {
	uint8 top;
	uint8 addr;
	uint8 size;
// Oscillator channel (Bits 4-7 of mode-byte). Simplified to use only stereo here.
#define MASK_OSC_CHANNEL  (1 << 4)
#define OSC_CHANNEL_LEFT  (1 << 4)
#define OSC_CHANNEL_RIGHT (0 << 4)
// Oscillator halt bit (Bit 0 of mode-byte)
#define MASK_OSC_HALT     (1 << 0)
#define OSC_HALT          (1 << 0)
// Oscillator mode (Bits 1 and 2 of mode-byte)
#define MASK_OSC_MODE     (3 << 1)
#define OSC_MODE_LOOP     (0 << 1)
#define OSC_MODE_ONESHOT  (1 << 1)
#define OSC_MODE_SYNC_AM  (2 << 1)
#define OSC_MODE_SWAP     (3 << 1)
	uint8 mode;
	uint16 relPitch; ///< 8b.8b fixed point, big endian?
};

// Maximum number of waves in an Apple IIGS instrument's wave list.
// Chosen empirically based on Apple IIGS AGI game data, increase if needed.
#define MAX_WAVE_COUNT 4

struct IIgsInstrumentHeader {
	IIgsEnvelope env;
	uint8 relseg;
	uint8 priority;
	uint8 bendrange;
	uint8 vibdepth;
	uint8 vibspeed;
	uint8 spare;
	uint8 wac;
	uint8 wbc;
	IIgsWaveInfo wal[MAX_WAVE_COUNT];
	IIgsWaveInfo wbl[MAX_WAVE_COUNT];
};

struct IIgsSampleHeader {
	uint16 type;
	uint8  pitch; ///< Logarithmic, base is 2**(1/12), unknown multiplier (Possibly in range 1040-1080)
	uint8  unknownByte_Ofs3; // 0x7F in Gold Rush's sound resource 60, 0 in all others.
	uint8  volume; ///< Current guess: Logarithmic in 6 dB steps
	uint8  unknownByte_Ofs5; ///< 0 in all tested samples.
	uint16 instrumentSize; ///< Little endian. 44 in all tested samples. A guess.
	uint16 sampleSize; ///< Little endian. Accurate in all tested samples excluding Manhunter I's sound resource 16.
	IIgsInstrumentHeader instrument;
};

#if 0
static SoundInstrument *instruments;
static int numInstruments;
static uint8 *wave;
#endif

bool readIIgsEnvelope(IIgsEnvelope &envelope, Common::SeekableReadStream &stream) {
	for (int segNum = 0; segNum < ENVELOPE_SEGMENT_COUNT; segNum++) {
		envelope.seg[segNum].bp  = stream.readByte();
		envelope.seg[segNum].inc = stream.readUint16BE();
	}
	return !stream.ioFailed();
}

bool readIIgsWaveInfo(IIgsWaveInfo &waveInfo, Common::SeekableReadStream &stream) {
	waveInfo.top      = stream.readByte();
	waveInfo.addr     = stream.readByte();
	waveInfo.size     = stream.readByte();
	waveInfo.mode     = stream.readByte();
	waveInfo.relPitch = stream.readUint16BE();
	return !stream.ioFailed();
}

/**
 * Read an Apple IIGS instrument header from the given stream.
 * @param header The header to which to write the data.
 * @param stream The source stream from which to read the data.
 * @return True if successful, false otherwise.
 */
bool readIIgsInstrumentHeader(IIgsInstrumentHeader &header, Common::SeekableReadStream &stream) {
	readIIgsEnvelope(header.env, stream);
	header.relseg    = stream.readByte();
	header.priority  = stream.readByte();
	header.bendrange = stream.readByte();
	header.vibdepth  = stream.readByte();
	header.vibspeed  = stream.readByte();
	header.spare     = stream.readByte();
	header.wac       = stream.readByte();
	header.wbc       = stream.readByte();
	for (int waveA = 0; waveA < header.wac; waveA++) // Read A wave lists
		readIIgsWaveInfo(header.wal[waveA], stream);
	for (int waveB = 0; waveB < header.wbc; waveB++) // Read B wave lists
		readIIgsWaveInfo(header.wbl[waveB], stream);
	return !stream.ioFailed();
}

/**
 * Read an Apple IIGS AGI sample header from the given stream.
 * @param header The header to which to write the data.
 * @param stream The source stream from which to read the data.
 * @return True if successful, false otherwise.
 */
bool readIIgsSampleHeader(IIgsSampleHeader &header, Common::SeekableReadStream &stream) {
	header.type             = stream.readUint16LE();
	header.pitch            = stream.readByte();
	header.unknownByte_Ofs3 = stream.readByte();
	header.volume           = stream.readByte();
	header.unknownByte_Ofs5 = stream.readByte();
	header.instrumentSize   = stream.readUint16LE();
	header.sampleSize       = stream.readUint16LE();
	return readIIgsInstrumentHeader(header.instrument, stream);
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
Audio::AudioStream *makeIIgsSampleStream(Common::SeekableReadStream &stream, int resnum = -1) {
	const uint32 startPos = stream.pos();
	IIgsSampleHeader header;
	Audio::AudioStream *result = NULL;
	bool readHeaderOk = readIIgsSampleHeader(header, stream);

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
			// Make an audio stream from the mono, 8 bit, unsigned input data
			byte flags = Audio::Mixer::FLAG_AUTOFREE | Audio::Mixer::FLAG_UNSIGNED;
			int rate = (int) (1076 * pow(SEMITONE, header.pitch));
			result = Audio::makeLinearInputStream(sampleData, header.sampleSize, rate, flags, 0, 0);
		}
	}

	// If couldn't make a sample out of the input stream for any reason then
	// rewind back to stream's starting position and clear I/O failed -status.
	if (result == NULL) {
		stream.seek(startPos);
		stream.clearIOFailed();
	}

	return result;
}

#endif

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

#ifdef USE_IIGS_SOUND

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

#endif /* USE_IIGS_SOUND */

void SoundMgr::unloadSound(int resnum) {
	if (_vm->_game.dirSound[resnum].flags & RES_LOADED) {
		if (_vm->_game.sounds[resnum].flags & SOUND_PLAYING) {
			/* FIXME: Stop playing */
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

	if (_vm->_game.sounds[resnum].flags & SOUND_PLAYING)
		return;

	stopSound();

	if (_vm->_game.sounds[resnum].rdata == NULL)
		return;

	type = READ_LE_UINT16(_vm->_game.sounds[resnum].rdata);

	if (type != AGI_SOUND_SAMPLE && type != AGI_SOUND_MIDI && type != AGI_SOUND_4CHN)
		return;

	_vm->_game.sounds[resnum].flags |= SOUND_PLAYING;
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
			chn[i].ptr = (struct AgiNote *)(song + (song[i << 1] | (song[(i << 1) + 1] << 8)));
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
		_vm->_game.sounds[playingSound].flags &= ~SOUND_PLAYING;
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

#ifdef USE_IIGS_SOUND
	/*loadInstruments("demo.sys"); */
#endif

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

#ifdef USE_CHORUS
	/* Stop chorus ;) */
	if (chn[i].type == AGI_SOUND_4CHN &&
		_vm->_soundemu == SOUND_EMU_NONE && i < 3) {
		stopNote(i + 4);
	}
#endif
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

#ifdef USE_CHORUS
	/* Add chorus ;) */
	if (chn[i].type == AGI_SOUND_4CHN &&
		_vm->_soundemu == SOUND_EMU_NONE && i < 3) {
		int newfreq = freq * 1007 / 1000;
		if (freq == newfreq)
			newfreq++;
		playNote(i + 4, newfreq, vol * 2 / 3);
	}
#endif
}

#ifdef USE_IIGS_SOUND

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
	chn[0].ptr = (struct AgiNote *)p;

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

#endif /* USE_IIGS_SOUND */

void SoundMgr::playAgiSound() {
	int i, freq;

	for (playing = i = 0; i < (_vm->_soundemu == SOUND_EMU_PC ? 1 : 4); i++) {
		playing |= !chn[i].end;

		if (chn[i].end)
			continue;

		if ((--chn[i].timer) <= 0) {
			stopNote(i);
			freq = ((chn[i].ptr->frq0 & 0x3f) << 4) | (int)(chn[i].ptr->frq1 & 0x0f);

			if (freq) {
				uint8 v = chn[i].ptr->vol & 0x0f;
				playNote(i, freq * 10, v == 0xf ? 0 : 0xff - (v << 1));
			}

			chn[i].timer = ((int)chn[i].ptr->durHi << 8) | chn[i].ptr->durLo;

			if (chn[i].timer == 0xffff) {
				chn[i].end = 1;
				chn[i].vol = 0;
				chn[i].env = 0;
#ifdef USE_CHORUS
				/* chorus */
				if (chn[i].type == AGI_SOUND_4CHN && _vm->_soundemu == SOUND_EMU_NONE && i < 3) {
					chn[i + 4].vol = 0;
					chn[i + 4].env = 0;
				}
#endif
			}
			chn[i].ptr++;
		}
	}
}

void SoundMgr::playSound() {
	int i;

	if (endflag == -1)
		return;

#ifdef USE_IIGS_SOUND
	if (chn[0].type == AGI_SOUND_MIDI) {
		/* play_midi_sound (); */
		playing = 0;
	} else if (chn[0].type == AGI_SOUND_SAMPLE) {
		playSampleSound();
	} else
#endif
		playAgiSound();

	if (!playing) {
		for (i = 0; i < NUM_CHANNELS; chn[i++].vol = 0);

		if (endflag != -1)
			_vm->setflag(endflag, true);

		if (playingSound != -1)
			_vm->_game.sounds[playingSound].flags &= ~SOUND_PLAYING;
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

#ifdef USE_IIGS_SOUND

#if 0
void Sound::unloadInstruments() {
	free(instruments);
}
#endif

#endif /* USE_IIGS_SOUND */

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
