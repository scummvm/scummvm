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

/***************************************************************************
 opl2.c Copyright (C) 2002/04 Solomon Peachy, Christoph Reichenbach

 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.

***************************************************************************/

#include "common/util.h"

#include "sci/tools.h"
#include "sci/sfx/iterator.h"
#include "../softseq.h"
#include "../adlib.h"

#include "sound/fmopl.h"

namespace Sci {

// FIXME: Instead of hardcoding SAMPLE_RATE we should call Mixer::getOutputRate()
#ifdef __DC__
#define SAMPLE_RATE 22050
#elif defined (__WII__)
#define SAMPLE_RATE 48000
#else
#define SAMPLE_RATE 44100
#endif

#define CHANNELS SFX_PCM_MONO
#define STEREO 0

/* local function declarations */

static void opl2_allstop(sfx_softseq_t *self);

//#define DEBUG_ADLIB

/* portions shamelessly lifted from claudio's XMP */
/* other portions lifted from sound/opl3.c in the Linux kernel */

static int ready = 0;

static int register_base[11] = {
	0x20, 0x23, 0x40, 0x43,
	0x60, 0x63, 0x80, 0x83,
	0xe0, 0xe3, 0xc0
};

static int register_offset[12] = {
	/* Channel           1     2     3     4     5     6     7     8     9  */
	/* Operator 1 */   0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12, 0x18, 0x19, 0x1A

};

static int ym3812_note[13] = {
	0x157, 0x16b, 0x181, 0x198, 0x1b0, 0x1ca,
	0x1e5, 0x202, 0x220, 0x241, 0x263, 0x287,
	0x2ae
};

static uint8 sci_adlib_vol_base[16] = {
	0x00, 0x11, 0x15, 0x19, 0x1D, 0x22, 0x26, 0x2A,
	0x2E, 0x23, 0x37, 0x3B, 0x3F, 0x3F, 0x3F, 0x3F
};
static uint8 sci_adlib_vol_tables[16][64];

/* back to your regularly scheduled definitions */

static uint8 instr[MIDI_CHANNELS];
static uint16 pitch[MIDI_CHANNELS];
static uint8 vol[MIDI_CHANNELS];
static uint8 pan[MIDI_CHANNELS];
static int free_voices = ADLIB_VOICES;
static uint8 oper_note[ADLIB_VOICES];
static uint8 oper_chn[ADLIB_VOICES];

static FM_OPL *ym3812 = NULL;

static uint8 adlib_reg[256];
static uint8 adlib_master;


/* initialise note/operator lists, etc. */
void adlibemu_init_lists() {
	int i;

	int j;

	for (i = 0 ; i < 16 ; i++) {
		for (j = 0; j < 64 ; j++) {
			sci_adlib_vol_tables[i][j] = ((uint16)sci_adlib_vol_base[i]) * j / 63;
		}
	}

	for (i = 0; i < MIDI_CHANNELS ; i++) {
		pitch[i] = 8192;  /* center the pitch wheel */
	}

	free_voices = ADLIB_VOICES;

	memset(instr, 0, sizeof(instr));
	memset(vol, 0x7f, sizeof(vol));
	memset(pan, 0x3f, sizeof(pan));
	memset(adlib_reg, 0, sizeof(adlib_reg));
	memset(oper_chn, 0xff, sizeof(oper_chn));
	memset(oper_note, 0xff, sizeof(oper_note));
	adlib_master = 12;
}

/* more shamelessly lifted from xmp and adplug.  And altered.  :) */

static void opl_write(int a, int v) {
	adlib_reg[a] = v;
	OPLWrite(ym3812, 0x388, a);
	OPLWrite(ym3812, 0x389, v);
}

void synth_setpatch(int voice, uint8 *data) {
	int i;

	opl_write(0xBD, 0);

	for (i = 0; i < 10; i++)
		opl_write(register_base[i] + register_offset[voice], data[i]);

	opl_write(register_base[10] + voice, data[10]);

	/* mute voice after patch change */
	opl_write(0xb0 + voice, adlib_reg[0xb0+voice] & 0xdf);

#ifdef DEBUG_ADLIB
	for (i = 0; i < 10; i++)
		printf("%02x ", adlib_reg[register_base[i] + register_offset[voice]]);
	printf("%02x ", adlib_reg[register_base[10] + voice]);
#endif

}

void synth_setvolume(int voice, int volume) {
	int8 level1, level2;

	level1 = ~adlib_reg[register_base[2] + register_offset[voice]] & 0x3f;
	level2 = ~adlib_reg[register_base[3] + register_offset[voice]] & 0x3f;

	if (level1) {
		level1 += sci_adlib_vol_tables[adlib_master][volume>>1];
	}

	if (level2) {
		level2 += sci_adlib_vol_tables[adlib_master][volume>>1];
	}

	if (level1 > 0x3f)
		level1 = 0x3f;
	if (level1 < 0)
		level1 = 0;

	if (level2 > 0x3f)
		level2 = 0x3f;
	if (level2 < 0)
		level2 = 0;

	/* algorithm-dependent; we may need to set both operators. */
	if (adlib_reg[register_base[10] + voice] & 1)
		opl_write(register_base[2] + register_offset[voice],
		            (uint8)((~level1 &0x3f) |
		                     (adlib_reg[register_base[2] + register_offset[voice]]&0xc0)));

	opl_write(register_base[3] + register_offset[voice],
	            (uint8)((~level2 &0x3f) |
	                     (adlib_reg[register_base[3] + register_offset[voice]]&0xc0)));

}

void synth_setnote(int voice, int note, int bend) {
	int n, fre, oct;
	float delta;

	delta = 0;

	n = note % 12;

	if (bend < 8192)
		bend = 8192 - bend;
	delta = pow(2.0, (float)(bend % 8192) / 8192.0);

	if (bend > 8192)
		fre = (int)(ym3812_note[n] * delta);
	else
		fre = (int)(ym3812_note[n] / delta);

	oct = note / 12 - 1;

	if (oct < 0)
		oct = 0;

	opl_write(0xa0 + voice, fre & 0xff);
	opl_write(0xb0 + voice,
	          0x20 | ((oct << 2) & 0x1c) | ((fre >> 8) & 0x03));
#ifdef DEBUG_ADLIB
	printf("-- %02x %02x\n", adlib_reg[0xa0+voice], adlib_reg[0xb0+voice]);
#endif

}


/* back to your regularly scheduled driver */

int adlibemu_stop_note(int chn, int note, int velocity) {
	int i, op = 255;

	//  sciprintf("Note off %d %d %d\n", chn, note, velocity);

	for (i = 0;i < ADLIB_VOICES && op == 255;i++) {
		if (oper_chn[i] == chn)
			if (oper_note[i] == note)
				op = i;
	}

	if (op == 255) {
#ifdef DEBUG_ADLIB
		printf("ADLIB: can't stop note: C%02x N%02x V%02x\n", chn, note, velocity);
		printf("C ");
		for (i = 0; i < ADLIB_VOICES ; i++) {
			printf("%02x ", oper_chn[i]);
		}
		printf("\n");
		printf("N ");
		for (i = 0; i < ADLIB_VOICES ; i++) {
			printf("%02x ", oper_note[i]);
		}
		printf("\n");
#endif
		return -1; /* that note isn't playing.. */
	}

	opl_write(0xb0 + op, (adlib_reg[0xb0+op] & 0xdf));

	oper_chn[op] = 255;
	oper_note[op] = 255;

	free_voices++;

#ifdef DEBUG_ADLIB
	printf("stop voice %d (%d rem):  C%02x N%02x V%02x\n", op, free_voices, chn, note, velocity);
#endif

	return 0;
}

int adlibemu_start_note(int chn, int note, int velocity) {
	int op, volume, inst = 0;

	//  sciprintf("Note on %d %d %d\n", chn, note, velocity);

	if (velocity == 0) {
		return adlibemu_stop_note(chn, note, velocity);
	}

	if (free_voices <= 0) {
		printf("ADLIB: All voices full\n");  /* XXX implement overflow code */
		return -1;
	}

	for (op = 0; op < ADLIB_VOICES ; op++)
		if (oper_chn[op] == 255)
			break;

	if (op == ADLIB_VOICES) {
		printf("ADLIB:  WTF?  We couldn't find a voice yet it we have %d left.\n", free_voices);
		return -1;
	}

	/* Scale channel volume */
	volume = velocity * vol[chn] / 127;
	inst = instr[chn];

	synth_setpatch(op, adlib_sbi[inst]);
	synth_setvolume(op, volume);
	synth_setnote(op, note, pitch[chn]);

	oper_chn[op] = chn;
	oper_note[op] = note;
	free_voices--;

#ifdef DEBUG_ADLIB
	printf("play voice %d (%d rem):  C%02x N%02x V%02x/%02x P%02x (%02x/%02x)\n", op, free_voices, chn, note, velocity, volume, inst,
	       adlib_reg[register_base[2] + register_offset[op]] & 0x3f,
	       adlib_reg[register_base[3] + register_offset[op]] & 0x3f);
#endif

	return 0;
}

static void adlibemu_update_pitch(int chn, int note, int newpitch) {
	int i;
	int matched = 0;

	pitch[chn] = newpitch;

	for (i = 0;i < ADLIB_VOICES;i++) {
		if (oper_chn[i] == chn) {
			matched++;
			synth_setnote(i, oper_note[i], newpitch);
		}
	}

//  printf("Matched %d notes on channel %d.\n", matched, chn);
}

void test_adlib() {

	int voice = 0;
#if 0
	uint8 data[] = { 0x25, 0x21, 0x48, 0x48, 0xf0, 0xf2, 0xf0, 0xa5, 0x00, 0x00, 0x06 };
#else
	uint8 *data = adlib_sbi[0x0a];
#endif

#if 1
	opl_write(register_base[0] + register_offset[voice], data[0]);
	opl_write(register_base[1] + register_offset[voice], data[1]);
	opl_write(register_base[2] + register_offset[voice], data[2]);
	opl_write(register_base[3] + register_offset[voice], data[3]);
	opl_write(register_base[4] + register_offset[voice], data[4]);
	opl_write(register_base[5] + register_offset[voice], data[5]);
	opl_write(register_base[6] + register_offset[voice], data[6]);
	opl_write(register_base[7] + register_offset[voice], data[7]);
	opl_write(register_base[8] + register_offset[voice], data[8]);
	opl_write(register_base[9] + register_offset[voice], data[9]);
	opl_write(register_base[10] + register_offset[voice], data[10]);
#else
	synth_setpatch(voice, data);
#endif

#if 0
	opl_write(0xA0 + voice, 0x57);
	opl_write(0xB0 + voice, 0x2d);
#else
	synth_setvolume(voice, 0x50);
	synth_setnote(voice, 0x30, 0);
#endif

	/*
	instr[0x0e] = 0x0a;
	instr[0x03] = 0x26;

	adlibemu_start_note(0x0e, 0x30, 0x40);
	sleep(1);
	adlibemu_start_note(0x03, 0x48, 0x40);
	sleep(1);
	*/
}


/* count is # of FRAMES, not bytes.
   We assume 16-bit stereo frames (ie 4 bytes)
*/
static void opl2_poll(sfx_softseq_t *self, byte *dest, int count) {
	int16 *ptr = (int16 *)dest;

	if (!ready)
		error("synth_mixer(): !ready \n");

	if (!ptr)
		error("synth_mixer(): !buffer \n");

	YM3812UpdateOne(ym3812, ptr, count);
}

static int opl2_init(sfx_softseq_t *self, byte *data_ptr, int data_length, byte *data2_ptr,
	int data2_length) {
	int i;

	/* load up the patch.003 file, parse out the instruments */
	if (data_length < 1344) {
		sciprintf("[sfx:seq:opl2] Invalid patch.003: Expected %d, got %d\n", 1344, data_length);
		return -1;
	}

	for (i = 0; i < 48; i++)
		make_sbi((adlib_def *)(data_ptr + (28 * i)), adlib_sbi[i]);

	if (data_length > 1344)
		for (i = 48; i < 96; i++)
			make_sbi((adlib_def *)(data_ptr + 2 + (28 * i)), adlib_sbi[i]);

	if (!(ym3812 = makeAdlibOPL(SAMPLE_RATE))) {
		sciprintf("[sfx:seq:opl2] Failure: Emulator init failed!\n");
		return SFX_ERROR;
	}

	ready = 1;

	opl2_allstop(self);
	return SFX_OK;
}


static void opl2_exit(sfx_softseq_t *self) {
	OPLDestroy(ym3812);
	ym3812 = NULL;

	// XXX deregister with pcm layer.
}

static void opl2_allstop(sfx_softseq_t *self) {
	//  printf("AdlibEmu:  Reset\n");
	if (! ready)
		return;

	adlibemu_init_lists();

	OPLResetChip(ym3812);

	opl_write(0x01, 0x20);
	opl_write(0xBD, 0xc0);

#ifdef DEBUG_ADLIB
	printf("ADLIB: reset complete\n");
#endif
	//  test_adlib();
}

int midi_adlibemu_reverb(short param) {
	printf("ADLIB: reverb NYI %04x \n", param);
	return 0;
}

int midi_adlibemu_event(uint8 command, uint8 note, uint8 velocity, uint32 delta) {
	uint8 channel, oper;

	channel = command & 0x0f;
	oper = command & 0xf0;

	switch (oper) {
	case 0x80:
		return adlibemu_stop_note(channel, note, velocity);
	case 0x90:  /* noteon and noteoff */
		return adlibemu_start_note(channel, note, velocity);
	case 0xe0: { /* Pitch bend */
		int bend = (note & 0x7f) | ((velocity & 0x7f) << 7);
//      printf("Bend to %d\n", bend);
		adlibemu_update_pitch(channel, note, bend);
	}
	case 0xb0:    /* CC changes. */
		switch (note) {
		case 0x07:
			vol[channel] = velocity;
			break;
		case 0x0a:
			pan[channel] = velocity;
			break;
		case 0x4b:
			break;
		case 0x7b:  { /* all notes off */
			int i = 0;
			for (i = 0;i < ADLIB_VOICES;i++)
				if (oper_chn[i] == channel)
					adlibemu_stop_note(channel, oper_note[i], 0);
			break;
		}
		default:
			; /* XXXX ignore everything else for now */
		}
		return 0;
	case 0xd0:    /* aftertouch */
		/* XXX Aftertouch in the OPL thing? */
		return 0;
	default:
		printf("ADLIB: Unknown event %02x\n", command);
		return 0;
	}

	return 0;
}

int midi_adlibemu_event2(uint8 command, uint8 param, uint32 delta) {
	uint8 channel;
	uint8 oper;

	channel = command & 0x0f;
	oper = command & 0xf0;
	switch (oper) {
	case 0xc0:   /* change instrument */
#ifdef DEBUG_ADLIB
		printf("ADLIB: Selecting instrument %d on channel %d\n", param, channel);
#endif
		instr[channel] = param;
		return 0;
	default:
		printf("ADLIB: Unknown event %02x\n", command);
	}

	return 0;
}

static void opl2_volume(sfx_softseq_t *self, int volume) {
	uint8 i;

	i = (uint8)volume * 15 / 100;

	adlib_master = i;

#ifdef DEBUG_ADLIB
	printf("ADLIB: master volume set to %d\n", adlib_master);
#endif
}

int opl2_set_option(sfx_softseq_t *self, const char *name, const char *value) {
	return SFX_ERROR;
}

void opl2_event(sfx_softseq_t *self, byte cmd, int argc, byte *argv) {
	if (argc == 1)
		midi_adlibemu_event2(cmd, argv[0], 0);
	else if (argc == 2)
		midi_adlibemu_event(cmd, argv[0], argv[1], 0);
}

/* the driver struct */

sfx_softseq_t sfx_softseq_opl2 = {
	"adlibemu",
	"0.1",
	opl2_set_option,
	opl2_init,
	opl2_exit,
	opl2_volume,
	opl2_event,
	opl2_poll,
	opl2_allstop,
	NULL,
	3,	/* use patch.003 */
	SFX_SEQ_PATCHFILE_NONE,
	0x4,	/* Play flags */
	0,	/* No rhythm channel (9) */
	ADLIB_VOICES, /* # of voices */
	{SAMPLE_RATE, CHANNELS, SFX_PCM_FORMAT_S16_NATIVE}
};

} // End of namespace Sci
