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

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include "../sequencer.h"

#ifdef HAVE_SYS_SOUNDCARD_H

#include "../adlib.h"

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/soundcard.h>
#include <sfx_iterator.h> /* for some MIDI information */

namespace Sci {

#if 1
SEQ_DEFINEBUF(2048);
static int seqfd;
#else
extern unsigned char _seqbuf[2048];
extern int _seqbuflen;
extern int _seqbufptr;
extern int seqfd;
#endif

static guint8 instr[MIDI_CHANNELS];
static int dev;
static int free_voices = ADLIB_VOICES;
static long note_time[ADLIB_VOICES];
static unsigned char oper_note[ADLIB_VOICES];
static unsigned char oper_chn[ADLIB_VOICES];

#if 1
void seqbuf_dump(void) { /* OSS upcall */
	if (_seqbufptr)
		if (write(seqfd, _seqbuf, _seqbufptr) == -1) {
			perror("ADLIB write ");
			exit(-1);
		}
	_seqbufptr = 0;
}
#endif

/* initialise note/operator lists, etc. */
void adlib_init_lists(void) {
	int i;
	for (i = 0 ; i < ADLIB_VOICES ; i++) {
		oper_note[i] = 255;
		oper_chn[i] = 255;
		note_time[i] = 0;
	}
	free_voices = ADLIB_VOICES;
}

int adlib_stop_note(int chn, int note, int velocity) {
	int i, op = 255;

	for (i = 0;i < ADLIB_VOICES && op == 255;i++) {
		if (oper_chn[i] == chn)
			if (oper_note[i] == note)
				op = i;
	}

	if (op == 255) {
		printf("can't stop.. chn %d %d %d\n", chn, note, velocity);
		return 255;	/* not playing */
	}

	SEQ_STOP_NOTE(dev, op, note, velocity);
	SEQ_DUMPBUF();

	oper_chn[op] = 255;
	oper_note[op] = 255;
	note_time[op] = 0;

	free_voices++;

	return op;
}

int adlib_kill_one_note(int chn) {
	int oldest = 255, i = 255;
	long time = 0;

	if (free_voices >= ADLIB_VOICES) {
		printf("Free list empty but no notes playing\n");
		return 255;
	}	/* No notes playing */

	for (i = 0; i < ADLIB_VOICES ; i++) {
		if (oper_chn[i] != chn)
			continue;
		if (note_time[i] == 0)
			continue;
		if (time == 0) {
			time = note_time[i];
			oldest = i;
			continue;
		}
		if (note_time[i] < time) {
			time = note_time[i];
			oldest = i;
		}
	}

	/*	printf("Killing chn %d, oper %d\n", chn, oldest); */

	if (oldest == 255)
		return 255;	/* Was already stopped. Why? */

	SEQ_STOP_NOTE(dev, oldest, oper_note[oldest], 0);
	SEQ_DUMPBUF();

	oper_chn[oldest] = 255;
	oper_note[oldest] = 255;
	note_time[oldest] = 0;
	free_voices++;

	return oldest;
}

static void
adlib_start_note(int chn, int note, int velocity) {
	int free;
	struct timeval now;

	if (velocity == 0) {
		adlib_stop_note(chn, note, velocity);
		return;
	}

	gettimeofday(&now, NULL);

	if (free_voices <= 0)
		free = adlib_kill_one_note(chn);
	else
		for (free = 0; free < ADLIB_VOICES ; free++)
			if (oper_chn[free] == 255)
				break;

	/*  printf("play operator %d/%d:  %d %d %d\n", free, free_voices, chn, note, velocity); */

	oper_chn[free] = chn;
	oper_note[free] = note;
	note_time[free] = now.tv_sec * 1000000 + now.tv_usec;
	free_voices--;

	SEQ_SET_PATCH(dev, free, instr[chn]);
	SEQ_START_NOTE(dev, free, note, velocity);
	SEQ_DUMPBUF();
}

static int
midi_adlib_open(int data_length, byte *data_ptr, int data2_length,
                byte *data2_ptr, void *seq) {
	int nrdevs, i, n;
	struct synth_info info;
	struct sbi_instrument sbi;

	if (data_length < 1344) {
		printf("invalid patch.003");
		return -1;
	}

	for (i = 0; i < 48; i++)
		make_sbi((adlib_def *)(data_ptr + (28 * i)), adlib_sbi[i]);

	if (data_length > 1344)
		for (i = 48; i < 96; i++)
			make_sbi((adlib_def *)(data_ptr + 2 + (28 * i)), adlib_sbi[i]);

	memset(instr, 0, sizeof(instr));

	if (!IS_VALID_FD(seqfd = open("/dev/sequencer", O_WRONLY, 0))) {
		perror("/dev/sequencer");
		return(-1);
	}
	if (ioctl(seqfd, SNDCTL_SEQ_NRSYNTHS, &nrdevs) == -1) {
		perror("/dev/sequencer");
		return(-1);
	}
	for (i = 0;i < nrdevs && dev == -1;i++) {
		info.device = i;
		if (ioctl(seqfd, SNDCTL_SYNTH_INFO, &info) == -1) {
			perror("info: /dev/sequencer");
			return(-1);
		}
		if (info.synth_type == SYNTH_TYPE_FM)
			dev = i;
	}
	if (dev == -1) {
		fprintf(stderr, "ADLIB: FM synthesizer not detected\n");
		return(-1);
	}

	/*  free_voices = info.nr_voices; */
	adlib_init_lists();

	printf("ADLIB: Loading patches into synthesizer\n");
	sbi.device = dev;
	sbi.key = FM_PATCH;
	for (i = 0; i < 96; i++) {
		for (n = 0; n < 32; n++)
			memcpy(sbi.operators, &adlib_sbi[i], sizeof(sbi_instr_data));
		sbi.channel = i;
		SEQ_WRPATCH(&sbi, sizeof(sbi));
		SEQ_DUMPBUF();
	}
	SEQ_START_TIMER();
	SEQ_SET_TEMPO(60);
	SEQ_DUMPBUF();
	return 0;
}


static int
midi_adlib_close(void) {
	SEQ_DUMPBUF();
	return close(seqfd);
}


static int
midi_adlib_allstop(void) {
	int i;
	for (i = 0; i < ADLIB_VOICES ; i++) {
		if (oper_chn[i] == 255)
			continue;
		adlib_stop_note(oper_chn[i], oper_note[i], 0);
	}
	adlib_init_lists();

	return 0;
}

static int
midi_adlib_reverb(int param) {
	printf("reverb NYI %04x \n", param);
	return 0;
}

static inline int
midi_adlib_event1(guint8 command, guint8 note, guint8 velocity) {
	guint8 channel, oper;

	channel = command & 0x0f;
	oper = command & 0xf0;

	switch (oper) {
	case 0x80:
		adlib_stop_note(channel, note, velocity);
		return 0;
	case 0x90:
		adlib_start_note(channel, note, velocity);
		return 0;
	case 0xe0:    /* Pitch bend needs scaling? */
		SEQ_BENDER(dev, channel, ((note << 8) & velocity));
		SEQ_DUMPBUF();
		break;
	case 0xb0:    /* CC changes.  we ignore. */
		/* XXXX we need to parse out 0x07 volume, at least. */
		return 0;
	case 0xd0:    /* aftertouch */
		SEQ_CHN_PRESSURE(dev, channel, note);
		SEQ_DUMPBUF();
		return 0;
	default:
		printf("ADLIB: Unknown event %02x\n", command);
		return 0;
	}

	SEQ_DUMPBUF();
	return 0;
}

static inline int
midi_adlib_event2(guint8 command, guint8 param) {
	guint8 channel;
	guint8 oper;

	channel = command & 0x0f;
	oper = command & 0xf0;
	switch (oper) {
	case 0xc0: {  /* change instrument */
		int inst = param;
		instr[channel] = inst; /* XXXX offset? */
		//    SEQ_SET_PATCH(dev, channel, inst);
		//    SEQ_DUMPBUF();
		return 0;
	}
	default:
		printf("ADLIB: Unknown event %02x\n", command);
	}

	SEQ_DUMPBUF();
	return 0;
}

static int
midi_adlib_event(byte command, int argc, byte *argv) {
	if (argc > 1)
		return midi_adlib_event1(command, argv[0], argv[1]);
	else
		return midi_adlib_event2(command, argv[0]);
}

static int
midi_adlib_delay(int ticks) {
	SEQ_DELTA_TIME(ticks);
	return SFX_OK;
}

static int
midi_adlib_set_option(char *name, char *value) {
	return SFX_ERROR; /* No options are supported at this time */
}

/* the driver struct */

sfx_sequencer_t sfx_sequencer_oss_adlib = {
	"adlib",
	"0.1",
	SFX_DEVICE_NONE, /* No device dependancy-- fixme, this might become ossseq */
	&midi_adlib_set_option,
	&midi_adlib_open,
	&midi_adlib_close,
	&midi_adlib_event,
	&midi_adlib_delay,
	NULL,
	&midi_adlib_allstop,
	NULL,
	&midi_adlib_reverb,
	003,		/* patch.003 */
	SFX_SEQ_PATCHFILE_NONE,
	0x04,		/* playflag */
	0, 		/* do not play channel 9 */
	ADLIB_VOICES,  /* Max polyphony */
	0 /* Does not require any write-ahead by its own */
};

} // End of namespace Sci

#endif /* HAVE_SYS_SOUNDCARD_H */
