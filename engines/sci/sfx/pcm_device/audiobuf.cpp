/***************************************************************************
 audiobuf.c Copyright (C) 2003 Christoph Reichenbach


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

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#include "audiobuf.h"

#define NO_BUFFER_UNDERRUN		0
#define SAW_BUFFER_UNDERRUN		1
#define REPORTED_BUFFER_UNDERRUN	2

static int
buffer_underrun_status = NO_BUFFER_UNDERRUN;


static sfx_audio_buf_chunk_t *
sfx_audbuf_alloc_chunk(void)
{
	sfx_audio_buf_chunk_t *ch = (sfx_audio_buf_chunk_t*)sci_malloc(sizeof(sfx_audio_buf_chunk_t));
	ch->used = 0;
	ch->next = NULL;
	ch->prev = NULL;

	return ch;
}

void
sfx_audbuf_init(sfx_audio_buf_t *buf, sfx_pcm_config_t pcm_conf)
{
	int framesize = SFX_PCM_FRAME_SIZE(pcm_conf);
	byte silence[16];
	int silencew = pcm_conf.format & ~SFX_PCM_FORMAT_LMASK;

	/* Determine the correct 'silence' for the channel and install it */
	/* Conservatively assume stereo */
	if (pcm_conf.format & SFX_PCM_FORMAT_16) {
		if (pcm_conf.format & SFX_PCM_FORMAT_LE) {
			silence[0] = silencew & 0xff;
			silence[1] = (silencew >> 8) & 0xff;
		} else {
			silence[0] = (silencew >> 8) & 0xff;
			silence[1] = silencew & 0xff;
		}
		memcpy(silence + 2, silence, 2);
	} else {
		silence[0] = silencew;
		silence[1] = silencew;
	}

	buf->last = buf->first = sfx_audbuf_alloc_chunk();
	buf->unused = NULL;
	memcpy(buf->last_frame, silence, framesize);	 /* Initialise, in case we
							 ** underrun before the
							 ** first write  */
	buf->read_offset = 0;
	buf->framesize = framesize;
	buf->read_timestamp.secs = -1; /* Mark as inactive */
	buf->frames_nr = 0;
}

static void
sfx_audbuf_free_chain(sfx_audio_buf_chunk_t *b)
{
	while (b) {
		sfx_audio_buf_chunk_t *n = b->next;
		sci_free(b);
		b = n;
	}
}

void
sfx_audbuf_free(sfx_audio_buf_t *buf)
{
	sfx_audbuf_free_chain(buf->first);
	sfx_audbuf_free_chain(buf->unused);
	buf->first = buf->last = buf->unused = NULL;
	buf->read_offset = (int) 0xdeadbeef;
}

void
sfx_audbuf_write(sfx_audio_buf_t *buf, unsigned char *src, int frames)
{
	/* In here, we compute PER BYTE */
	int data_left = buf->framesize * frames;

	if (!buf->last) {
		fprintf(stderr, "FATAL: Violation of audiobuf.h usage protocol: Must use 'init' before 'write'\n");
		exit(1);
	}

	if (buffer_underrun_status == SAW_BUFFER_UNDERRUN) {
		/* Print here to avoid threadsafeness issues */
		sciprintf("[audiobuf] Buffer underrun\n");
		buffer_underrun_status = REPORTED_BUFFER_UNDERRUN;
	}

	buf->frames_nr += frames;

	while (data_left) {
		int cpsize;
		int buf_free;
		buf_free = SFX_AUDIO_BUF_SIZE - buf->last->used;


		if (buf_free >= data_left)
			cpsize = data_left;
		else
			cpsize = buf_free;

		/* Copy and advance pointers */
		memcpy(buf->last->data + buf->last->used, src, cpsize);
		data_left -= cpsize;
		buf->last->used += cpsize;
		src += cpsize;

		if (buf->last->used == SFX_AUDIO_BUF_SIZE) {
			if (!buf->last->next) {
				sfx_audio_buf_chunk_t *old = buf->last;
				if (buf->unused) { /* Re-use old chunks */
					sfx_audio_buf_chunk_t *buf_next_unused = buf->unused->next;
					buf->unused->next = NULL;
					buf->unused->used = 0;

					buf->last->next = buf->unused;
					buf->unused = buf_next_unused;
				} else /* Allocate */
					buf->last->next =
						sfx_audbuf_alloc_chunk();

				buf->last->prev = old;
			}

			buf->last = buf->last->next;
		}
	}

#ifdef TRACE_BUFFER
	{
		sfx_audio_buf_chunk_t *c = buf->first;
		int t = buf->read_offset;

		while (c) {
			fprintf(stderr, "-> [");
			for (; t < c->used; t++)
				fprintf(stderr, " %02x", c->data[t]);
			t = 0;
			fprintf(stderr, " ] ");
			c = c->next;
		}
		fprintf(stderr, "\n");
	}
#endif

	if (frames && (src - buf->framesize) != buf->last_frame)
	/* Backup last frame, unless we're already filling from it */
		memcpy(buf->last_frame, src - buf->framesize, buf->framesize);
}

int
sfx_audbuf_read(sfx_audio_buf_t *buf, unsigned char *dest, int frames)
{
	int written = 0;

	if (frames <= 0)
		return 0;

	if (buf->read_timestamp.secs >= 0) {
		/* Have a timestamp? Must update it! */
		buf->read_timestamp =
			sfx_timestamp_add(buf->read_timestamp,
					  frames);
							
	}

	buf->frames_nr -= frames;
	if (buf->frames_nr < 0)
		buf->frames_nr = 0;

#ifdef TRACE_BUFFER
	{
		sfx_audio_buf_chunk_t *c = buf->first;
		int t = buf->read_offset;

		while (c) {
			fprintf(stderr, "-> [");
			for (; t < c->used; t++)
				fprintf(stderr, " %02x", c->data[t]);
			t = 0;
			fprintf(stderr, " ] ");
			c = c->next;
		}
		fprintf(stderr, "\n");
	}
#endif

	while (frames) {
		int data_needed = frames * buf->framesize;
		int rdbytes = data_needed;
		int rdframes;

		if (rdbytes > buf->first->used - buf->read_offset)
			rdbytes = buf->first->used - buf->read_offset;

		memcpy(dest, buf->first->data + buf->read_offset, rdbytes);

		buf->read_offset += rdbytes;
		dest += rdbytes;

		if (buf->read_offset == SFX_AUDIO_BUF_SIZE) {
			/* Continue to next, enqueue the current chunk as
			** being unused */
			sfx_audio_buf_chunk_t *lastfirst = buf->first;

			buf->first = buf->first->next;
			lastfirst->next = buf->unused;
			buf->unused = lastfirst;

			buf->read_offset = 0;
		}

		rdframes = (rdbytes / buf->framesize);
		frames -= rdframes;
		written += rdframes;

		if (frames &&
		    (!buf->first || buf->read_offset == buf->first->used)) {
			fprintf(stderr, "Underrun by %d frames at %d\n", frames,
				buf->read_timestamp.frame_rate);
			/* Buffer underrun! */
			if (!buffer_underrun_status == NO_BUFFER_UNDERRUN) {
				buffer_underrun_status = SAW_BUFFER_UNDERRUN;
			}
			do {
				memcpy(dest, buf->last_frame, buf->framesize);
				dest += buf->framesize;
			} while (--frames);
		}

	}

	return written;
}

#if 0
static void
_sfx_audbuf_rewind_stream(sfx_audio_buf_t *buf, int delta)
{
	if (delta > buf->frames_nr)
		delta = buf->frames_nr;


	fprintf(stderr, "Rewinding %d\n", delta);
	buf->frames_nr -= delta;

	/* From here on, 'delta' means the number of BYTES to remove */
	delta *= buf->framesize;

	while (delta) {
		if (buf->last->used >= delta) {
			fprintf(stderr, "Subtracting from  %d  %d\n", buf->last->used, delta);
			buf->last->used -= delta;
			delta = 0;
		} else {
			fprintf(stderr, "Must do block-unuse\n");
			delta -= buf->last->used;
			buf->last->used = 0;
			buf->last->next = buf->unused;
			buf->unused = buf->last;
			buf->last = buf->unused->prev;
			buf->unused->prev = NULL;
		}
	}
}
#endif

void
sfx_audbuf_write_timestamp(sfx_audio_buf_t *buf, sfx_timestamp_t ts)
{
	sfx_timestamp_t newstamp;

	newstamp = sfx_timestamp_add(ts, -buf->frames_nr);


	if (buf->read_timestamp.secs <= 0)
		/* Initial stamp */
		buf->read_timestamp = newstamp;
	else {
		int delta = sfx_timestamp_frame_diff(newstamp, buf->read_timestamp);
		long s1,s2,s3,u1,u2,u3;
		sfx_timestamp_gettime(&(buf->read_timestamp), &s1, &u1);
		sfx_timestamp_gettime(&(newstamp), &s2, &u2);
		sfx_timestamp_gettime(&(ts), &s3, &u3);

		if (delta < 0) {
#if 0
			/*			fprintf(stderr, "[SFX-BUF] audiobuf.c: Timestamp delta %d at %d: Must rewind (not implemented yet)\n",
						delta, buf->read_timestamp.frame_rate);*/
			_sfx_audbuf_rewind_stream(buf, -delta);
			buf->read_timestamp = newstamp;
#endif
		} else if (delta > 0) {
			fprintf(stderr, "[SFX-BUF] audiobuf.c: Timestamp delta %d at %d: Filling in as silence frames\n",
				delta, buf->read_timestamp.frame_rate);
			/* Fill up with silence */
			while (delta--) {
				sfx_audbuf_write(buf, buf->last_frame, 1);
			}
			buf->read_timestamp = newstamp;
		}
	}
}

int
sfx_audbuf_read_timestamp(sfx_audio_buf_t *buf, sfx_timestamp_t *ts)
{
	if (buf->read_timestamp.secs > 0) {
		*ts = buf->read_timestamp;
		return 0;
	} else {
		ts->secs = -1;
		ts->usecs = -1;
		ts->frame_offset = -1;
		ts->frame_rate = -1;
		return 1; /* No timestamp */
	}
}
