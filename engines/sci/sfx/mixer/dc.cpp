/***************************************************************************
 dc.c Copyright (C) 2005 Walter van Niftrik


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


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Walter van Niftrik <w.f.b.w.v.niftrik@stud.tue.nl>

***************************************************************************/

#include "../mixer.h"
#include <sci_memory.h>
#include <dc/sound/sound.h>
#include <stream.h>
#include <sys/queue.h>

#define FEED_MODE_ALIVE 0
#define FEED_MODE_IDLE 1
#define FEED_MODE_DIEING 2
#define FEED_MODE_DEAD 3
#define FEED_MODE_RESTART 4

typedef struct feed_state {
	/* Queue entry. */
	TAILQ_ENTRY(feed_state) entry;

	/* Whether feed is alive or dead. */
	int mode;

	/* Blank gap in frames. */
	int gap;

	/* Stream handle. */
	snd_stream_hnd_t handle;

	/* Feed. */
	sfx_pcm_feed_t *feed;

	/* Timestamp of next frame requested by stream driver. */
	sfx_timestamp_t time;
} feed_state_t;

TAILQ_HEAD(feed_list, feed_state) feeds;

/* Buffer size in samples. */
#define BUF_SIZE 0x4000

static char buf[BUF_SIZE * 2];

static feed_state_t *
find_feed_state(snd_stream_hnd_t hnd) {
	feed_state_t *state;
	TAILQ_FOREACH(state, &feeds, entry) {
		if (state->handle == hnd)
			return state;
	}

	return NULL;
}

static void
query_timestamp(feed_state_t *state) {
	sfx_pcm_feed_t *feed = state->feed;

	if (feed->get_timestamp) {
		sfx_timestamp_t stamp;
		int val = feed->get_timestamp(feed, &stamp);

		switch (val) {
		case PCM_FEED_TIMESTAMP:
			state->gap = sfx_timestamp_frame_diff(stamp, state->time);

			if (state->gap >= 0)
				state->mode = FEED_MODE_ALIVE;
			else {
				long secs, usecs;

				state->mode = FEED_MODE_RESTART;
				sci_gettime(&secs, &usecs);
				state->time = sfx_new_timestamp(secs, usecs, feed->conf.rate);
				state->gap = sfx_timestamp_frame_diff(stamp, state->time);

				if (state->gap < 0)
					state->gap = 0;
			}
			break;
		case PCM_FEED_IDLE:
			state->mode = FEED_MODE_IDLE;
			break;
		case PCM_FEED_EMPTY:
			state->mode = FEED_MODE_DIEING;
			state->gap = BUF_SIZE;
		}
	} else {
		state->mode = FEED_MODE_DIEING;
		state->gap = BUF_SIZE;
	}
}

void
U8_to_S16(char *buf, int frames, int stereo) {
	int samples = frames * (stereo ? 2 : 1);
	int i;

	for (i = samples - 1; i >= 0; i--) {
		buf[i * 2 + 1] = (unsigned char) buf[i] - 128;
		buf[i * 2] = 0;
	}
}

static void *
callback(snd_stream_hnd_t hnd, sfx_timestamp_t timestamp, int bytes_req, int *bytes_recv) {
	feed_state_t *state = find_feed_state(hnd);
	sfx_pcm_feed_t *feed;
	int channels, frames_req;
	int frames_recv = 0;

	assert(state);

	state->time = timestamp;
	feed = state->feed;
	channels = feed->conf.stereo == SFX_PCM_MONO ? 1 : 2;
	frames_req = bytes_req / 2 / channels;

	while (frames_req != frames_recv) {
		int frames_left = frames_req - frames_recv;
		char *buf_pos = buf + frames_recv * channels * 2;

		if (state->mode == FEED_MODE_IDLE)
			query_timestamp(state);

		if (state->mode == FEED_MODE_IDLE) {
			memset(buf_pos, 0, frames_left * channels * 2);

			state->time = sfx_timestamp_add(state->time, frames_left);
			break;
		}

		if (state->gap) {
			int frames = state->gap;

			if (frames > frames_left)
				frames = frames_left;

			memset(buf_pos, 0, frames * channels * 2);

			state->gap -= frames;
			frames_recv += frames;
			state->time = sfx_timestamp_add(state->time, frames);
			if (!state->gap && state->mode == FEED_MODE_DIEING) {
				state->mode = FEED_MODE_DEAD;
				break;
			}
		} else {
			int frames = feed->poll(feed, buf_pos, frames_left);

			if (feed->conf.format == SFX_PCM_FORMAT_U8)
				U8_to_S16(buf_pos, frames, feed->conf.stereo != SFX_PCM_MONO);

			frames_recv += frames;
			state->time = sfx_timestamp_add(state->time, frames);

			if (frames < frames_left)
				query_timestamp(state);
		}
	}

	*bytes_recv = bytes_req;
	return buf;
}

static int
mix_init(sfx_pcm_mixer_t *self, sfx_pcm_device_t *device) {
	if (snd_stream_init() < 0) {
		fprintf(stderr, "[dc-mixer] Failed to initialize streaming sound driver\n");
		return SFX_ERROR;
	}

	TAILQ_INIT(&feeds);

	return SFX_OK;
}

static void
mix_subscribe(sfx_pcm_mixer_t *self, sfx_pcm_feed_t *feed) {
	feed_state_t *state = sci_malloc(sizeof(feed_state_t));
	long secs, usecs;

	if ((feed->conf.format != SFX_PCM_FORMAT_S16_LE) &&
	        (feed->conf.format != SFX_PCM_FORMAT_U8)) {
		fprintf(stderr, "[dc-mixer] Unsupported feed format\n");
		feed->destroy(feed);
		return;
	}

	state->handle = snd_stream_alloc(callback, BUF_SIZE);

	if (state->handle == SND_STREAM_INVALID) {
		fprintf(stderr, "[dc-mixer] Failed to allocate stream handle\n");
		feed->destroy(feed);
		return;
	}

	feed->frame_size = SFX_PCM_FRAME_SIZE(feed->conf);
	state->mode = FEED_MODE_ALIVE;
	state->feed = feed;
	state->gap = 0;

	TAILQ_INSERT_TAIL(&feeds, state, entry);

	sci_gettime(&secs, &usecs);
	state->time = sfx_new_timestamp(secs, usecs, feed->conf.rate);
	snd_stream_start(state->handle, feed->conf.rate,
	                 feed->conf.stereo != SFX_PCM_MONO);
}

static void
mix_exit(sfx_pcm_mixer_t *self) {
	snd_stream_shutdown();
}

static int
mix_process(sfx_pcm_mixer_t *self) {
	feed_state_t *state, *state_next;

	TAILQ_FOREACH(state, &feeds, entry) {
		snd_stream_poll(state->handle);
	}

	state = TAILQ_FIRST(&feeds);
	while (state) {
		state_next = TAILQ_NEXT(state, entry);
		if (state->mode == FEED_MODE_DEAD) {
			snd_stream_stop(state->handle);
			snd_stream_destroy(state->handle);
			state->feed->destroy(state->feed);
			TAILQ_REMOVE(&feeds, state, entry);
		} else if (state->mode == FEED_MODE_RESTART) {
			snd_stream_stop(state->handle);
			snd_stream_start(state->handle, state->feed->conf.rate,
			                 state->feed->conf.stereo != SFX_PCM_MONO);
			state->mode = FEED_MODE_ALIVE;
		}
		state = state_next;
	}

	return SFX_OK;
}

static void
mix_pause(sfx_pcm_mixer_t *self) {
}

static void
mix_resume(sfx_pcm_mixer_t *self) {
}

static int
pcm_init(sfx_pcm_device_t *self) {
	return SFX_OK;
}

static void
pcm_exit(sfx_pcm_device_t *self) {
}

sfx_pcm_device_t sfx_pcm_driver_dc = {
	"dc",
	"0.1",

	pcm_init,
	pcm_exit,
	NULL,
	NULL,
	NULL,

	{0, 0, 0},
	0,
	NULL,
	NULL
};

sfx_pcm_mixer_t sfx_pcm_mixer_dc = {
	"dc",
	"0.1",

	mix_init,
	mix_exit,
	mix_subscribe,
	mix_pause,
	mix_resume,
	mix_process,

	0,
	0,
	NULL,
	NULL,
	NULL
};
