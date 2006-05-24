/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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

namespace Agi {

#define USE_INTERPOLATION
#define USE_CHORUS

/* TODO: add support for variable sampling rate in the output device
 */

#ifdef USE_IIGS_SOUND

/**
 * AGI engine sound envelope structure.
 */
struct sound_envelope {
	uint8 bp;
	uint8 inc_hi;
	uint8 inc_lo;
};

struct sound_wavelist {
	uint8 top;
	uint8 addr;
	uint8 size;
	uint8 mode;
	uint8 rel_hi;
	uint8 rel_lo;
};

struct sound_instrument {
	struct sound_envelope env[8];
	uint8 relseg;
	uint8 priority;
	uint8 bendrange;
	uint8 vibdepth;
	uint8 vibspeed;
	uint8 spare;
	uint8 wac;
	uint8 wbc;
	struct sound_wavelist wal[8];
	struct sound_wavelist wbl[8];
};

struct sound_iigs_sample {
	uint8 type_lo;
	uint8 type_hi;
	uint8 srate_lo;
	uint8 srate_hi;
	uint16 unknown[2];
	uint8 size_lo;
	uint8 size_hi;
	uint16 unknown2[13];
};

#if 0
static struct sound_instrument *instruments;
static int num_instruments;
static uint8 *wave;
#endif

#endif

static int playing;
static struct channel_info chn[NUM_CHANNELS];
static int endflag = -1;
static int playing_sound = -1;
static uint8 *song;
static uint8 env;

struct sound_driver *snd;

extern struct sound_driver sound_dummy;

static void stop_note(int i);
static void play_note(int i, int freq, int vol);


int16 *snd_buffer;
static int16 *waveform;

static int16 waveform_ramp[WAVEFORM_SIZE] = {
	0, 8, 16, 24, 32, 40, 48, 56,
	64, 72, 80, 88, 96, 104, 112, 120,
	128, 136, 144, 152, 160, 168, 176, 184,
	192, 200, 208, 216, 224, 232, 240, 255,
	0, -248, -240, -232, -224, -216, -208, -200,
	-192, -184, -176, -168, -160, -152, -144, -136,
	-128, -120, -112, -104, -96, -88, -80, -72,
	-64, -56, -48, -40, -32, -24, -16, -8	/* Ramp up */
};

static int16 waveform_square[WAVEFORM_SIZE] = {
	255, 230, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 110,
	-255, -230, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -110, 0, 0, 0, 0	/* Square */
};

static int16 waveform_mac[WAVEFORM_SIZE] = {
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

static struct agi_note play_sample[] = {
	{0xff, 0x7f, 0x18, 0x00, 0x7f},
	{0xff, 0xff, 0x00, 0x00, 0x00},
	{0xff, 0xff, 0x00, 0x00, 0x00},
	{0xff, 0xff, 0x00, 0x00, 0x00}
};

static int note_to_period(int note) {
	return 10 * (period[note % 12] >> (note / 12 - 3));
}

#endif				/* USE_IIGS_SOUND */

void unload_sound(int resnum) {
	if (game.dir_sound[resnum].flags & RES_LOADED) {
		if (game.sounds[resnum].flags & SOUND_PLAYING)
			/* FIXME: Stop playing */
			;

		/* Release RAW data for sound */
		free(game.sounds[resnum].rdata);
		game.sounds[resnum].rdata = NULL;
		game.dir_sound[resnum].flags &= ~RES_LOADED;
	}
}

void decode_sound(int resnum) {
#ifdef USE_IIGS_SOUND
	int type, size;
	int16 *buf;
	uint8 *src;
	struct sound_iigs_sample *smp;

	debugC(3, kDebugLevelSound, "(%d)", resnum);
	type = READ_LE_UINT16(game.sounds[resnum].rdata);

	if (type == AGI_SOUND_SAMPLE) {
		/* Convert sample data to 16 bit signed format
		 */
		smp = (struct sound_iigs_sample *)game.sounds[resnum].rdata;
		size = ((int)smp->size_hi << 8) + smp->size_lo;
		src = (uint8 *) game.sounds[resnum].rdata;
		buf = (int16 *) calloc(1, 54 + (size << 1) + 100);	/* FIXME */
		memcpy(buf, src, 54);
		for (; size--; buf[size + 54] = ((int16) src[size + 54] - 0x80) << 4);	/* FIXME */
		game.sounds[resnum].rdata = (uint8 *) buf;
		free(src);
	}
#endif				/* USE_IIGS_SOUND */
}

void start_sound(int resnum, int flag) {
	int i, type;
#ifdef USE_IIGS_SOUND
	struct sound_iigs_sample *smp;
#endif

	if (game.sounds[resnum].flags & SOUND_PLAYING)
		return;

	stop_sound();

	if (game.sounds[resnum].rdata == NULL)
		return;

	type = READ_LE_UINT16(game.sounds[resnum].rdata);

	if (type != AGI_SOUND_SAMPLE && type != AGI_SOUND_MIDI && type != AGI_SOUND_4CHN)
		return;

	game.sounds[resnum].flags |= SOUND_PLAYING;
	game.sounds[resnum].type = type;
	playing_sound = resnum;
	song = (uint8 *) game.sounds[resnum].rdata;

	switch (type) {
#ifdef USE_IIGS_SOUND
	case AGI_SOUND_SAMPLE:
		debugC(3, kDebugLevelSound, "IIGS sample");
		smp = (struct sound_iigs_sample *)game.sounds[resnum].rdata;
		for (i = 0; i < NUM_CHANNELS; i++) {
			chn[i].type = type;
			chn[i].flags = 0;
			chn[i].ins = (int16 *) & game.sounds[resnum].rdata[54];
			chn[i].size = ((int)smp->size_hi << 8) + smp->size_lo;
			chn[i].ptr = &play_sample[i];
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
		chn[0].ptr = (struct agi_note *)(song + 3);
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
			chn[i].ptr = (struct agi_note *)(song + (song[i << 1] | (song[(i << 1) + 1] << 8)));
			chn[i].timer = 0;
			chn[i].vol = 0;
			chn[i].end = 0;
		}
		break;
	}

	memset(snd_buffer, 0, BUFFER_SIZE << 1);
	endflag = flag;

	/* Nat Budin reports that the flag should be reset when sound starts
	 */
	setflag(endflag, false);

	/* FIXME: should wait for sound time instead of setting the flag
	 *        immediately
	 */
	if (opt.nosound) {
		setflag(endflag, true);
		stop_sound();
	}
}

void stop_sound() {
	int i;

	endflag = -1;
	for (i = 0; i < NUM_CHANNELS; i++)
		stop_note(i);

	if (playing_sound != -1) {
		game.sounds[playing_sound].flags &= ~SOUND_PLAYING;
		playing_sound = -1;
	}
}

static int16 *buffer;

int init_sound() {
	int r = -1;

	buffer = snd_buffer = (int16 *) calloc(2, BUFFER_SIZE);

	env = false;

	switch (opt.soundemu) {
	case SOUND_EMU_NONE:
		waveform = waveform_ramp;
		env = true;
		break;
	case SOUND_EMU_AMIGA:
	case SOUND_EMU_PC:
		waveform = waveform_square;
		break;
	case SOUND_EMU_MAC:
		waveform = waveform_mac;
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
	/*load_instruments ("demo.sys"); */
#endif

	return r;
}

void deinit_sound(void) {
	debugC(3, kDebugLevelSound, "()");
	if (snd)
		snd->deinit();
	free(snd_buffer);
}

static void stop_note(int i) {
	chn[i].adsr = AGI_SOUND_ENV_RELEASE;

#ifdef USE_CHORUS
	/* Stop chorus ;) */
	if (chn[i].type == AGI_SOUND_4CHN &&
	    opt.soundemu == SOUND_EMU_NONE && i < 3) {
		stop_note(i + 4);
	}
#endif

#ifdef __TURBOC__
	if (i == 0)
		nosound();
#endif
}

static void play_note(int i, int freq, int vol) {
	if (!getflag(F_sound_on))
		vol = 0;
	else if (vol && opt.soundemu == SOUND_EMU_PC)
		vol = 160;

	chn[i].phase = 0;
	chn[i].freq = freq;
	chn[i].vol = vol;
	chn[i].env = 0x10000;
	chn[i].adsr = AGI_SOUND_ENV_ATTACK;

#ifdef USE_CHORUS
	/* Add chorus ;) */
	if (chn[i].type == AGI_SOUND_4CHN &&
	    opt.soundemu == SOUND_EMU_NONE && i < 3) {
		int newfreq = freq * 1007 / 1000;
		if (freq == newfreq)
			newfreq++;
		play_note(i + 4, newfreq, vol * 2 / 3);
	}
#endif

#ifdef __TURBOC__
	if (i == 0)
		sound(freq);
#endif
}

#ifdef USE_IIGS_SOUND

void play_midi_sound() {
	uint8 *p;
	uint8 parm1, parm2;
	static uint8 cmd, ch;

	playing = 1;

	if (chn[0].timer > 0) {
		chn[0].timer -= 2;
		return;
	}

	p = (uint8 *) chn[0].ptr;

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
			stop_note(ch);
		break;
	case 0x09:
		parm1 = *p++;
		parm2 = *p++;
		if (ch < NUM_CHANNELS)
			play_note(ch, note_to_period(parm1), 127);
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
			chn[ch].ins = (uint16 *) & wave[waveaddr[parm1]];
			chn[ch].size = wavesize[parm1];
		}
		debugC(3, kDebugLevelSound, "set patch %02x (%d,%d), ch %02x",
				parm1, waveaddr[parm1], wavesize[parm1], ch);
#endif
		break;
	}

	chn[0].timer = *p++;
	chn[0].ptr = (struct agi_note *)p;

	if (*p >= 0xfc) {
		debugC(3, kDebugLevelSound, "end of sequence");
		playing = 0;
		return;
	}
}

void play_sample_sound() {
	play_note(0, 11025 * 10, 200);
	playing = 1;
}

#endif				/* USE_IIGS_SOUND */

void play_agi_sound() {
	int i, freq;

	for (playing = i = 0; i < (opt.soundemu == SOUND_EMU_PC ? 1 : 4); i++) {
		playing |= !chn[i].end;

		if (chn[i].end)
			continue;

		if ((--chn[i].timer) <= 0) {
			stop_note(i);
			freq = ((chn[i].ptr->frq_0 & 0x3f) << 4) | (int)(chn[i].ptr->frq_1 & 0x0f);

			if (freq) {
				uint8 v = chn[i].ptr->vol & 0x0f;
				play_note(i, freq * 10, v == 0xf ? 0 : 0xff - (v << 1));
			}

			chn[i].timer = ((int)chn[i].ptr->dur_hi << 8) | chn[i].ptr->dur_lo;

			if (chn[i].timer == 0xffff) {
				chn[i].end = 1;
				chn[i].vol = 0;
				chn[i].env = 0;
#ifdef USE_CHORUS
				/* chorus */
				if (chn[i].type == AGI_SOUND_4CHN && opt.soundemu == SOUND_EMU_NONE && i < 3) {
					chn[i + 4].vol = 0;
					chn[i + 4].env = 0;
				}
#endif
			}
			chn[i].ptr++;
		}
	}
}

void play_sound() {
	int i;

	if (endflag == -1)
		return;

#ifdef USE_IIGS_SOUND
	if (chn[0].type == AGI_SOUND_MIDI) {
		/* play_midi_sound (); */
		playing = 0;
	} else if (chn[0].type == AGI_SOUND_SAMPLE) {
		play_sample_sound();
	} else
#endif
		play_agi_sound();

	if (!playing) {
		for (i = 0; i < NUM_CHANNELS; chn[i++].vol = 0);

		if (endflag != -1)
			setflag(endflag, true);

		if (playing_sound != -1)
			game.sounds[playing_sound].flags &= ~SOUND_PLAYING;
		playing_sound = -1;
		endflag = -1;
	}
}

uint32 mix_sound(void) {
	register int i, p;
	int16 *src;
	int c, b, m;

	memset(snd_buffer, 0, BUFFER_SIZE << 1);

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
				snd_buffer[i] += (b * m) >> 4;

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
				b = rnd->getRandomNumber(255) - 128;
				snd_buffer[i] += (b * m) >> 4;
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
int load_instruments(char *fname) {
	Common::File fp;
	int i, j, k;
	struct sound_instrument ai;
	int num_wav;
	char *path;

	path = "sierrast";

	if (!fp.open(path))
		return err_BadFileOpen;
	report("Loading samples: %s\n", path);

	if ((wave = malloc(0x10000 * 2)) == NULL)
		return err_NotEnoughMemory;

	fp.read(wave, 0x10000);
	fp.close();
	for (i = 0x10000; i--;) {
		((int16 *) wave)[i] = 2 * ((int16) wave[i] - 128);
	}

	fp = fopen("bla", "w");
	fwrite(wave, 2, 0x10000, fp);
	fclose(fp);

	report("Loading instruments: %s\n", path);

	if ((fp = fopen(path, "rb")) == NULL)
		return err_BadFileOpen;

	fseek(fp, 0x8469, SEEK_SET);

	for (num_wav = j = 0; j < 40; j++) {
		fread(&ai, 1, 32, fp);

		if (ai.env[0].bp > 0x7f)
			break;

#if 0
		printf("Instrument %d loaded ----------------\n", j);
		printf("Envelope:\n");
		for (i = 0; i < 8; i++)
			printf("[seg %d]: BP %02x Inc %04x\n", i, ai.env[i].bp,
					((int)ai.env[i].inc_hi << 8) | ai.env[i].inc_lo);
		printf("rel seg: %d, pri inc: %d, bend range: %d, vib dep: %d, "
				"vib spd: %d\n", ai.relseg, ai.priority,
				ai.bendrange, ai.vibdepth, ai.vibspeed);
		printf("A wave count: %d, B wave count: %d\n", ai.wac, ai.wbc);
#endif

		for (k = 0; k < ai.wac; k++, num_wav++) {
			fread(&ai.wal[k], 1, 6, fp);
#if 0
			printf("[A %d of %d] top: %02x, wave address: %02x, "
					"size: %02x, mode: %02x, relPitch: %04x\n", k + 1,
					ai.wac, ai.wal[k].top, ai.wal[k].addr, ai.wal[k].size,
					ai.wal[k].mode, ((int)ai.wal[k].rel_hi << 8) | ai.wal[k].rel_lo);
#endif
		}

		for (k = 0; k < ai.wbc; k++, num_wav++) {
			fread(&ai.wbl[k], 1, 6, fp);
#if 0
			printf("[B %d of %d] top: %02x, wave address: %02x, "
					"size: %02x, mode: %02x, relPitch: %04x\n", k + 1, ai.wbc,
					ai.wbl[k].top, ai.wbl[k].addr, ai.wbl[k].size,
					ai.wbl[k].mode, ((int)ai.wbl[k].rel_hi << 8) | ai.wbl[k].rel_lo);
#endif
		}
		waveaddr[j] = 256 * ai.wal[0].addr;
		wavesize[j] = 256 * (1 << ((ai.wal[0].size) & 0x07));
#if 1
		printf("%d addr = %d\n", j, waveaddr[j]);
		printf("   size = %d\n", wavesize[j]);
#endif
	}

	num_instruments = j;
	printf("%d Ensoniq 5503 instruments loaded. (%d waveforms)\n", num_instruments, num_wav);

	fclose(fp);

	return err_OK;
}

void unload_instruments() {
	free(instruments);
}
#endif

#endif				/* USE_IIGS_SOUND */

static void fill_audio(void *udata, int16 * stream, uint len) {
	len <<= 2;

	uint32 p = 0;
	static uint32 n = 0, s = 0;

	debugC(5, kDebugLevelSound, "(%p, %p, %d)", (void *)udata, (void *)stream, len);
	memcpy(stream, (uint8 *) buffer + s, p = n);
	for (n = 0, len -= p; n < len; p += n, len -= n) {
		play_sound();
		n = mix_sound() << 1;
		if (len < n) {
			memcpy((uint8 *) stream + p, buffer, len);
			s = len;
			n -= s;
			return;
		} else {
			memcpy((uint8 *) stream + p, buffer, n);
		}
	}
	play_sound();
	n = mix_sound() << 1;
	memcpy((uint8 *) stream + p, buffer, s = len);
	n -= s;
}

AGIMusic::AGIMusic(Audio::Mixer *pMixer) {
	_mixer = pMixer;
	_sampleRate = pMixer->getOutputRate();
	_mixer->setupPremix(this);
}

void AGIMusic::premixerCall(int16 *data, uint len) {
	Agi::fill_audio(NULL, data, len);
}

void AGIMusic::setVolume(uint8 volume) {
	// TODO
}

AGIMusic::~AGIMusic(void) {
	_mixer->setupPremix(NULL);
}

AGIMusic *g_agi_music;

} // End of namespace Agi
