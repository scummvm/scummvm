/***************************************************************************
 midi_mt32.c Copyright (C) 2000,2001 Rickard Lind, Solomon Peachy
 mt32.c Copyright (C) 2002..04 Christoph Reichenbach

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

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include "../sequencer.h"
#include "instrument-map.h"
#include <resource.h>

#ifdef _WIN32
#  include <win32/sci_win32.h>
#  include <windows.h>
#endif

#ifdef __BEOS__
#  include <be/kernel/OS.h>
#endif

static int delta = 0; /* Accumulated delta time */
static midi_writer_t *midi_writer = NULL;

static int midi_mt32_poke(guint32 address, guint8 *data, unsigned int n);
static int midi_mt32_poke_gather(guint32 address, guint8 *data1, unsigned int count1,
			  guint8 *data2, unsigned int count2);
static int midi_mt32_write_block(guint8 *data, unsigned int count);
static int midi_mt32_sysex_delay(void);
static int midi_mt32_volume(guint8 volume);
static int midi_mt32_reverb(int param);
static int midi_mt32_event(byte command, int argc, byte *argv);
static int midi_mt32_allstop(void);

static int type;
static guint8 sysex_buffer[266] = {0xF0, 0x41, 0x10, 0x16, 0x12};
static guint8 default_reverb;
static char shutdown_msg[20];

static long mt32_init_sec, mt32_init_usec; /* Time at initialisation */
static int mt32_init_delay = 0; /* Used to count the number of ticks (1/60s of a second) we should
				** wait before initialisation has been completed  */

/* timbre, volume, panpot, reverb.  keys 24-87 (64 keys)*/
static guint8 default_rhythm_keymap[256] = { /* MT-32 default */
	0x7f,0x64,7,1,  0x7f,0x64,7,1,  0x7f,0x64,7,1, 0x7f,0x64,7,1, /* 24-27 */
	0x7f,0x64,7,1,  0x7f,0x64,7,1,  0x7f,0x64,7,1, 0x7f,0x64,7,1,
	0x7f,0x64,7,1,  0x7f,0x64,7,1,  0x7f,0x64,7,1, 0x40,0x64,7,1,
	0x40,0x64,7,1,  0x4a,0x64,6,1,  0x41,0x64,7,1, 0x4b,0x64,8,1,
	0x45,0x64,6,1,  0x44,0x64,11,1, 0x46,0x64,6,1, 0x44,0x64,11,1,
	0x5d,0x64,6,1,  0x43,0x64,8,1,  0x47,0x64,6,1, 0x43,0x64,8,1,
	0x42,0x64,3,1,  0x48,0x64,6,1,  0x42,0x64,3,1, 0x49,0x64,8,1,
	0x7f,0x64,7,1,  0x7f,0x64,7,1,  0x56,0x64,9,1, 0x7f,0x64,7,1,
	0x4c,0x64,7,1,  0x7f,0x64,7,1,  0x7f,0x64,7,1, 0x7f,0x64,7,1,
	0x52,0x64,2,1,  0x53,0x64,4,1,  0x4d,0x64,8,1, 0x4e,0x64,9,1,
	0x4f,0x64,10,1, 0x50,0x64,7,1,  0x51,0x64,5,1, 0x54,0x64,2,1,
	0x55,0x64,2,1,  0x5b,0x64,9,1,  0x58,0x64,4,1, 0x5a,0x64,9,1,
	0x59,0x64,9,1,  0x5c,0x64,10,1, 0x7f,0x64,7,1, 0x57,0x64,12,1,
	0x7f,0x64,7,1,  0x7f,0x64,7,1,  0x7f,0x64,7,1, 0x7f,0x64,7,1,
	0x7f,0x64,7,1,  0x7f,0x64,7,1,  0x7f,0x64,7,1, 0x7f,0x64,7,1,
	0x7f,0x64,7,1,  0x7f,0x64,7,1,  0x7f,0x64,7,1, 0x7f,0x64,7,1  /* 84-87 */
};

static guint8 default_partial_reserve[9] = {  /* MT-32 DEFAULT */
	3, 10, 6, 4, 3, 0, 0, 0, 6 };

static struct {
	guint8 mode;
	guint8 time;
	guint8 level;
} mt32_reverb[11];


static int
midiout_write_block(byte *buf, int len, int delta)
{
	if (delta)
		midi_writer->delay(midi_writer, delta);

	return midi_writer->write(midi_writer, buf, len);
}

/* The following is the result of some experimenting, trying to approach the MT32's processing speed */
#define MAGIC_MIDIOUT_DELAY 40

static int
midiout_write_delayed_block(byte *buf, int len)
     /* Only used for initial programming */
{
	int rv = midiout_write_block(buf, len, 0);
	int delay = 1 + (len / MAGIC_MIDIOUT_DELAY);

	midi_writer->delay(midi_writer, delay);

	mt32_init_delay += delay; /* Keep track of delay times */

	return rv;
}

/* send default rhythm map and reserve */
int midi_mt32_defaults(guint8 volume, guint8 reverb) {
	printf("MT-32: Writing Default Rhythm key map\n");
	midi_mt32_poke(0x030110, default_rhythm_keymap, 256);

	printf("MT-32: Writing Default Partial Reserve\n");
	midi_mt32_poke(0x100004, default_partial_reserve, 9);

	if (reverb) {
		mt32_reverb[0].mode = 0;
		mt32_reverb[0].time = 5;
		mt32_reverb[0].level = 3;
		default_reverb = 0;

		printf("MT-32: Setting up default reverb levels\n");
		midi_mt32_reverb(default_reverb);
	}

	if (volume) {
		printf("MT-32: Setting default volume (%d)\n", volume);
		midi_mt32_volume(volume);
	}

	return SFX_OK;
}

int midi_mt32_open(int length, byte *data, int length2, byte *data2, void *dev)
{
	guint8 unknown_sysex[6] = {0x16, 0x16, 0x16, 0x16, 0x16, 0x16};
	guint8 i, memtimbres;
	unsigned int block2, block3;

	if (!dev) {
		fprintf(stderr, "Attempt to use MT-32 sequencer without device\n");
		return SFX_ERROR;
	}

	sci_gettime(&mt32_init_sec, &mt32_init_usec);

	midi_writer = (midi_writer_t *) dev;

	midi_mt32_allstop();

	if (!data) {
		type = SFX_MAP_UNKNOWN;
		sciprintf("MT-32: No patch.001 found, using defaults\n");
	} else {
		type = sfx_instrument_map_detect(data, length);
		if (type == SFX_MAP_UNKNOWN)
			sciprintf("MT-32: Unknown patch.001 format, using defaults\n");
		else
			sciprintf("MT-32: Programming Roland MT-32 with patch.001 (v%i) %d bytes\n", type, length);
	}

	if (type == SFX_MAP_MT32) {
		/* Display MT-32 initialization message */
		printf("MT-32: Displaying Text: \"%.20s\"\n", data + 20);
		midi_mt32_poke(0x200000, data + 20, 20);

		/* Cache a copy of the shutdown message */
		memcpy(shutdown_msg, data + 40, 20);

		/* Write Patches (48 or 96) */
		memtimbres = data[491];
		block2 = (memtimbres * 246) + 492;
		printf("MT-32: Writing Patches #01 - #32\n");
		midi_mt32_poke(0x050000, data + 107, 256);
		if ((length > block2) &&
		    data[block2] == 0xAB &&
		    data[block2 + 1] == 0xCD) {
			printf("MT-32: Writing Patches #33 - #64\n");
			midi_mt32_poke_gather(0x050200, data + 363, 128, data + block2 + 2, 128);
			printf("MT-32: Writing Patches #65 - #96\n");
			midi_mt32_poke(0x050400, data + block2 + 130, 256);
			block3 = block2 + 386;
		} else {
			printf("MT-32: Writing Patches #33 - #48\n");
			midi_mt32_poke(0x050200, data + 363, 128);
			block3 = block2;
		}
		/* Write Memory Timbres */
		for (i = 0; i < memtimbres; i++) {
			printf("MT-32: Writing Memory Timbre #%02d: \"%.10s\"\n",
			       i + 1, data + 492 + i * 246);
			midi_mt32_poke(0x080000 + (i << 9), data + 492 + i * 246, 246);
		}
		/* Write Rhythm key map and Partial Reserve */
		if ((length > block3) &&
		    data[block3] == 0xDC &&
		    data[block3 + 1] == 0xBA) {
			printf("MT-32: Writing Rhythm key map\n");
			midi_mt32_poke(0x030110, data + block3 + 2, 256);
			printf("MT-32: Writing Partial Reserve\n");
			midi_mt32_poke(0x100004, data + block3 + 258, 9);
		} else {
			midi_mt32_defaults(0,0);  /* send default keymap/reserve */
		}
		/* Display MT-32 initialization done message */
		printf("MT-32: Displaying Text: \"%.20s\"\n", data);
		midi_mt32_poke(0x200000, data, 20);
		/* Write undocumented MT-32(?) SysEx */
		printf("MT-32: Writing {F0 41 10 16 12 52 00 0A 16 16 16 16 16 16 20 F7}\n");
		midi_mt32_poke(0x52000A, unknown_sysex, 6);
		printf("MT-32: Setting up reverb levels\n");
		default_reverb = data[0x3e];
		memcpy(mt32_reverb,data+ 0x4a, 3 * 11);
		midi_mt32_reverb(default_reverb);
		printf("MT-32: Setting default volume (%d)\n", data[0x3c]);
		midi_mt32_volume(data[0x3c]);
		return 0;
	} else if (type == SFX_MAP_MT32_GM) {
		printf("MT-32: Loading SysEx bank\n");
		midi_mt32_write_block(data + 1155, (data[1154] << 8) + data[1153]);
		return 0;
	} else {
		midi_mt32_poke(0x200000, (guint8 *)"   FreeSCI Rocks!  ", 20);
		return midi_mt32_defaults(0x0c,1);  /* send defaults in absence of patch data */
	}
	return -1;
}

int midi_mt32_close(void)
{
	midi_mt32_allstop();
	if (type == 0) {
		printf("MT-32: Displaying Text: \"%.20s\"\n", shutdown_msg);
		midi_mt32_poke(0x200000, (unsigned char *) shutdown_msg, 20);
	}
	midi_writer->close(midi_writer);
	return SFX_OK;
}

int midi_mt32_volume(guint8 volume)
{
	volume &= 0x7f; /* (make sure it's not over 127) */
	if (midi_mt32_poke(0x100016, &volume, 1) < 0)
		return -1;

	return 0;
}

int midi_mt32_allstop(void)
{
	byte buf[4];
	int i;

	buf[0] = 0x7b;
	buf[1] = 0;
	buf[2] = 0;

	for (i = 0; i < 16; i++) {
		midi_mt32_event((guint8)(0xb0 | i), 2, buf);
	}

	return 0;
}

int midi_mt32_reverb(int param)
{
	guint8 buffer[3];

	if (param == -1)
		param = default_reverb;

	printf("MT-32: Sending reverb # %d (%d, %d, %d)\n",param, mt32_reverb[param].mode,
	       mt32_reverb[param].time,
	       mt32_reverb[param].level);

	buffer[0] = mt32_reverb[param].mode;
	buffer[1] = mt32_reverb[param].time;
	buffer[2] = mt32_reverb[param].level;
	midi_mt32_poke(0x100001, buffer, 3);

	return 0;
}


static int
midi_mt32_poke(guint32 address, guint8 *data, unsigned int count)
{
	guint8 checksum = 0;
	unsigned int i;

	if (count > 256) return -1;

	checksum -= (sysex_buffer[5] = (char)((address >> 16) & 0x7F));
	checksum -= (sysex_buffer[6] = (char)((address >> 8) & 0x7F));
	checksum -= (sysex_buffer[7] = (char)(address & 0x7F));

	for (i = 0; i < count; i++)
		checksum -= (sysex_buffer[i + 8] = data[i]);

	sysex_buffer[count + 8] = checksum & 0x7F;
	sysex_buffer[count + 9] = 0xF7;

	midiout_write_delayed_block(sysex_buffer, count + 10);
	if (midi_writer->flush)
		midi_writer->flush(midi_writer);
	midi_mt32_sysex_delay();

	return count + 10;

}

static int
midi_mt32_poke_gather(guint32 address, guint8 *data1, unsigned int count1,
		      guint8 *data2, unsigned int count2)
{
	guint8 checksum = 0;
	unsigned int i;

	if ((count1 + count2) > 256) return -1;

	checksum -= (sysex_buffer[5] = (char)((address >> 16) & 0x7F));
	checksum -= (sysex_buffer[6] = (char)((address >> 8) & 0x7F));
	checksum -= (sysex_buffer[7] = (char)(address & 0x7F));

	for (i = 0; i < count1; i++)
		checksum -= (sysex_buffer[i + 8] = data1[i]);
	for (i = 0; i < count2; i++)
		checksum -= (sysex_buffer[i + 8 + count1] = data2[i]);

	sysex_buffer[count1 + count2 + 8] = checksum & 0x7F;
	sysex_buffer[count1 + count2 + 9] = 0xF7;

	midiout_write_delayed_block(sysex_buffer, count1 + count2 + 10);
	if (midi_writer->flush)
		midi_writer->flush(midi_writer);
	midi_mt32_sysex_delay();
	return count1 + count2 + 10;
}


static int
midi_mt32_write_block(guint8 *data, unsigned int count)
{
	unsigned int block_start = 0;
	unsigned int i = 0;

	while (i < count) {
		if ((data[i] == 0xF0) && (i != block_start)) {
			midiout_write_delayed_block(data + block_start, i - block_start);
			block_start = i;
		}
		if (data[i] == 0xF7) {
			midiout_write_delayed_block(data + block_start, i - block_start + 1);
			midi_mt32_sysex_delay();
			block_start = i + 1;
		}
		i++;
	}
	if (count >= block_start) {
		if (midiout_write_delayed_block(data + block_start, count - block_start
					) != (count - block_start)) {
			fprintf(stderr, "midi_mt32_write_block(): midiout_write_block failed!\n");
			return 1;
		}
	}

	return 0;
}

static int
midi_mt32_sysex_delay(void)
{
  /* Under Win32, we won't get any sound, in any case... */
#ifdef HAVE_USLEEP
	usleep(320 * 63); /* One MIDI byte is 320us, 320us * 63 > 20ms */
#elif defined (_WIN32)
	Sleep(((320 * 63) / 1000) + 1);
#elif defined (__BEOS__)
	snooze(320 * 63);
#else
	sleep(1);
#endif
	return 0;
}

static int
midi_mt32_event(byte command, int argc, byte *argv)
{
	byte buf[8];

	buf[0] = command;
	memcpy(buf + 1, argv, argc);

	midiout_write_block(buf, argc + 1, delta);
	delta = 0;

	return SFX_OK;
}


static void
delay_init(void)
{/* Wait for MT-32 initialisation to complete */
	long endsec = mt32_init_sec, uendsec = mt32_init_usec;
	long sec, usec;
	int loopcount = 0;

	uendsec += (mt32_init_delay * 100000) / 6;  /* mt32_init_delay is in ticks (1/60th seconds), uendsecs in microseconds */
	endsec += uendsec / 1000000;
	uendsec %= 1000000;


	do {
		if (loopcount == 1)
			sciprintf("Waiting for MT-32 programming to complete...\n");

		sci_gettime(&sec, &usec);
		sleep(1); /* Idle a bit */
		++loopcount;
	} while ((sec < endsec) || ((sec == endsec) && (usec < uendsec)));

}

static int
midi_mt32_reset_timer(GTimeVal ts)
{
	if (mt32_init_delay) { /* We might still have to wait for initialisation to complete */
		delay_init();
		mt32_init_delay = 0;
	}


	midi_writer->reset_timer(midi_writer);
	return SFX_OK;
}


static int
midi_mt32_delay(int ticks)
{
	delta += ticks; /* Accumulate, write before next command */
	return SFX_OK;
}

static int
midi_mt32_set_option(char *name, char *value)
{
	return SFX_ERROR; /* No options are supported at this time */
}

/* the driver struct */

sfx_sequencer_t sfx_sequencer_mt32 = {
	"MT32",
	"0.1",
	SFX_DEVICE_MIDI, /* No device dependancy-- fixme, this might becomde ossseq */
	&midi_mt32_set_option,
	&midi_mt32_open,
	&midi_mt32_close,
	&midi_mt32_event,
	&midi_mt32_delay,
	&midi_mt32_reset_timer,
	&midi_mt32_allstop,
	&midi_mt32_volume,
	&midi_mt32_reverb,
	001,		/* patch.001 */
	SFX_SEQ_PATCHFILE_NONE,
	0x01,		/* playflag */
	1, 		/* do play channel 9 */
	32,  /* Max polyphony */
	0 /* Does not require any write-ahead by its own */
};
