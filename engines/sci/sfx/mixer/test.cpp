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

/* Mixer inspection/test program */


#include "../mixer.h"
#include <time.h>

#if 0
sfx_pcm_mixer_t *mix;

int dev_init(sfx_pcm_device_t *self);
void dev_exit(sfx_pcm_device_t *self);
int dev_option(sfx_pcm_device_t *self, char *name, char *value);
int dev_output(sfx_pcm_device_t *self, byte *buf, int count);

#define MIN_OUTPUT 128
/* Min amount of output to compute */

#define DEVICES_NR 10

sfx_pcm_device_t devices[DEVICES_NR] = {
	{ "test-1", "0", dev_init, dev_exit, dev_option, dev_output,
		{ 200, SFX_PCM_MONO, SFX_PCM_FORMAT_U8 }, 1024, NULL },
#if (DEVICES_NR > 1)
	{ "test-2", "0", dev_init, dev_exit, dev_option, dev_output, { 200, SFX_PCM_STEREO_LR, SFX_PCM_FORMAT_U8 }, 1024, NULL
	},
	{ "test-3", "0", dev_init, dev_exit, dev_option, dev_output,
	  { 200, SFX_PCM_STEREO_RL, SFX_PCM_FORMAT_U8 }, 1024, NULL },
	{ "test-4", "0", dev_init, dev_exit, dev_option, dev_output,
	  { 200, SFX_PCM_MONO, SFX_PCM_FORMAT_S8 }, 1024, NULL },
	{ "test-5", "0", dev_init, dev_exit, dev_option, dev_output,
	  { 200, SFX_PCM_MONO, SFX_PCM_FORMAT_U16_LE }, 1024, NULL },
	{ "test-6", "0", dev_init, dev_exit, dev_option, dev_output,
	  { 200, SFX_PCM_MONO, SFX_PCM_FORMAT_U16_BE }, 1024, NULL },
	{ "test-7", "0", dev_init, dev_exit, dev_option, dev_output,
	  { 200, SFX_PCM_MONO, SFX_PCM_FORMAT_S16_LE }, 1024, NULL },
	{ "test-8", "0", dev_init, dev_exit, dev_option, dev_output,
	  { 200, SFX_PCM_MONO, SFX_PCM_FORMAT_S16_BE }, 1024, NULL },
	{ "test-9", "0", dev_init, dev_exit, dev_option, dev_output,
	  { 200, SFX_PCM_STEREO_RL, SFX_PCM_FORMAT_S16_LE }, 1024, NULL },
	{ "test-10", "0", dev_init, dev_exit, dev_option, dev_output,
	  { 200, SFX_PCM_STEREO_LR, SFX_PCM_FORMAT_U16_BE }, 1024, NULL }
#endif
};

int output_count;

int dev_init(sfx_pcm_device_t *self) {
	output_count = 0;

	fprintf(stderr, "[DEV] Initialised device %p as follows:\n"
	        "\trate = %d\n"
	        "\tstereo = %s\n"
	        "\tbias = %x\n"
	        "\tbytes/sample = %d\n"
	        "\tendianness = %s\n",
	        self,
	        self->conf.rate,
	        self->conf.stereo ? ((self->conf.stereo == SFX_PCM_STEREO_LR) ? "Left, Right" : "Right, Left") : "No",
			        self->conf.format & ~SFX_PCM_FORMAT_LMASK,
			        (self->conf.format & SFX_PCM_FORMAT_16) ? 2 : 1,
			        ((self->conf.format & SFX_PCM_FORMAT_ENDIANNESS) == SFX_PCM_FORMAT_BE) ? "big" : "little");
	return 0;
}

void dev_exit(sfx_pcm_device_t *self) {
	fprintf(stderr, "[DEV] Uninitialising device\n");
}

int dev_option(sfx_pcm_device_t *self, char *name, char *value) {
	fprintf(stderr, "[DEV] Set option '%s' to '%s'\n", name, value);
	return 0;
}

int dev_output_enabled = 0;

int dev_output(sfx_pcm_device_t *self, byte *buf, int count) {
	int mono_sample_size = ((self->conf.format & SFX_PCM_FORMAT_16) ? 2 : 1);
	int sample_size = (self->conf.stereo ? 2 : 1) * mono_sample_size;
	int bias = self->conf.format & ~SFX_PCM_FORMAT_LMASK;
	int is_bigendian = (self->conf.format & SFX_PCM_FORMAT_ENDIANNESS) == SFX_PCM_FORMAT_BE;
	byte *left_channel = buf;
	byte *right_channel = buf;

	if (!dev_output_enabled)
		return 0;

	if (self->conf.format & SFX_PCM_FORMAT_16)
		bias <<= 8;

	if (self->conf.stereo == SFX_PCM_STEREO_LR)
		right_channel += mono_sample_size;
	if (self->conf.stereo == SFX_PCM_STEREO_RL)
		left_channel += mono_sample_size;

	while (count--) {
		int right = right_channel[0];
		int left = left_channel[0];
		int second_byte = ((self->conf.format & SFX_PCM_FORMAT_16) ? 1 : 0);

		if (second_byte) {

			if (is_bigendian) {
				left = left << 8 | left_channel[1];
				right = right << 8 | right_channel[1];
			} else {
				left = left | left_channel[1] << 8;
				right = right | right_channel[1] << 8;
			}
		}

		left -= bias;
		right -= bias;

		if (!second_byte) {
			left <<= 8;
			right <<= 8;
		}

		fprintf(stderr, "[DEV] %p play %04x:\t%04x %04x\n", self, output_count++, left & 0xffff, right & 0xffff);

		left_channel += sample_size;
		right_channel += sample_size;
	}
	return 0;
}

/* Feeds for debugging */

typedef struct {
	int i;
} int_struct;

int feed_poll(sfx_pcm_feed_t *self, byte *dest, int size);
void feed_destroy(sfx_pcm_feed_t *self);

int_struct private_bits[10] = {
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0}
};


typedef struct {
	int start;
	int samples_nr;
	byte *data;
} sample_feed_t;

#define FEEDS_NR 4

sfx_pcm_feed_t feeds[FEEDS_NR] = {
	{ feed_poll, feed_destroy, &(private_bits[0]),
		{ 200, SFX_PCM_MONO, SFX_PCM_FORMAT_S8 }, "test-feed", 0, 0}
#if FEEDS_NR > 1
	, { feed_poll, feed_destroy, &(private_bits[1]),
	    { 400, SFX_PCM_MONO, SFX_PCM_FORMAT_U8 }, "test-feed", 1, 0}
#endif
#if FEEDS_NR > 2
	, { feed_poll, feed_destroy, &(private_bits[2]),
	    { 20, SFX_PCM_MONO, SFX_PCM_FORMAT_S16_LE }, "test-feed", 2, 0}
#endif
#if FEEDS_NR > 3
	, { feed_poll, feed_destroy, &(private_bits[3]),
	    { 150, SFX_PCM_STEREO_LR, SFX_PCM_FORMAT_S8 }, "test-feed", 3, 0}
#endif
	/*
	,{ feed_poll, feed_destroy, &(private_bits[4]),
	  {}, "test-feed", 4, 0}
	,{ feed_poll, feed_destroy, &(private_bits[5]),
	  {}, "test-feed", 5, 0}
	*/
};

byte feed_data_0[] = {0xfd, 0xfe, 0xff, 0, 1, 2, 3, 4, 5, 6};
byte feed_data_1[] = {0x80, 0x90, 0xA0, 0xB0, 0xC0,
                      0xD0, 0xD0, 0xC0, 0xB0, 0xA0, 0x90, 0x80
                     };
byte feed_data_2[] = {0x00, 0x00,
                      0x00, 0x80,
                      0xe8, 0x03
                     };
byte feed_data_3[] = {0x00, 0x10,
                      0x01, 0x20,
                      0x02, 0x30
                     };

sample_feed_t sample_feeds[FEEDS_NR] = {
	{ 1, 10, feed_data_0 }
#if FEEDS_NR > 1
	, { 21, 12, feed_data_1 }
#endif
#if FEEDS_NR > 2
	, { 0, 3, feed_data_2 }
#endif
#if FEEDS_NR > 3
	, { 40, 3, feed_data_3 }
#endif
};

void
feed_destroy(sfx_pcm_feed_t *self) {
	int_struct *s = (int_struct *) self->internal;
	s->i = 0; /* reset */
}


int
feed_poll(sfx_pcm_feed_t *self, byte *dest, int size) {
	int_struct *s = (int_struct *) self->internal;
	int sample_size = self->sample_size;
	sample_feed_t *data = &(sample_feeds[self->debug_nr]);
	int bias = self->conf.format & ~SFX_PCM_FORMAT_LMASK;
	byte neutral[4] = {0, 0, 0, 0};
	int i;
	fprintf(stderr, "[feed] Asked for %d at %p, ss=%d\n", size, dest, sample_size);
	if (bias) {
		byte first = bias >> 8;
		byte second = bias & 0xff;

		if ((self->conf.format & SFX_PCM_FORMAT_ENDIANNESS) == SFX_PCM_FORMAT_LE) {
			int t = first;
			first = second;
			second = t;
		}

		if (self->conf.format & SFX_PCM_FORMAT_16) {
			neutral[0] = first;
			neutral[1] = second;
			neutral[2] = first;
			neutral[3] = second;
		} else {
			neutral[0] = bias;
			neutral[1] = bias;
		}
	}

	for (i = 0; i < size; i++) {
		int t = s->i - data->start;

		if (t >= data->samples_nr)
			return i;

		if (t >= 0)
			memcpy(dest, data->data + t * sample_size, sample_size);
		else
			memcpy(dest, neutral, sample_size);

		dest += sample_size;
		s->i++;
	}
	return size;
}




extern FILE *con_file;

#define DELAY	usleep((rand() / (RAND_MAX / 250L)))


int
main(int argc, char **argv) {
	int dev_nr;

	mix = sfx_pcm_find_mixer(NULL);

	if (!mix) {
		fprintf(stderr, "Error: Could not find a mixer!\n");
		return 1;
	} else {
		fprintf(stderr, "Running %s, v%s\n",
		        mix->name, mix->version);
	}
	con_file = stderr;

	srand(time(NULL));

	for (dev_nr = 0; dev_nr < DEVICES_NR; dev_nr++) {
		sfx_pcm_device_t *dev = &(devices[dev_nr++]);
		int j;
		dev->init(dev);
		mix->init(mix, dev);

		dev_output_enabled = 0;
		/* Prime it to our timing */
		for (j = 0; j < 250; j++) {
			DELAY;
			mix->process(mix);
		}
		dev_output_enabled = 1;

		fprintf(stderr, "[test] Subscribing...\n");

		for (j = 0; j < FEEDS_NR; j++)
			mix->subscribe(mix, &(feeds[j]));

		fprintf(stderr, "[test] Subscribed %d feeds.\n",
		        FEEDS_NR);

		while (output_count < MIN_OUTPUT) {
			DELAY;
			mix->process(mix);
			fprintf(stderr, "<tick>\n");
		}

		fprintf(stderr, "[test] Preparing finalisation\n");
		mix->exit(mix);
		fprintf(stderr, "[test] Mixer uninitialised\n");
	}
}

#else
int main() {}
#endif
