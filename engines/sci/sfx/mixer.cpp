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

#include "common/mutex.h"
#include "common/system.h"

#include "sci/tools.h"
#include "sci/sfx/mixer.h"
#include "sci/sci_memory.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Sci {

/* Max. number of milliseconds in difference allowed between independent audio streams */
#define TIMESTAMP_MAX_ALLOWED_DELTA 2

/*#define DEBUG 3*/
/* Set DEBUG to one of the following:
** anything -- high-level debugging (feed subscriptions/deletions etc.)
**     >= 1 -- rough input and output analysis (once per call)
**     >= 2 -- more detailed input analysis (once per call and feed)
**     >= 3 -- fully detailed input and output analysis (once per frame and feed)
*/

//#define DEBUG 1

#define MIN_DELTA_OBSERVATIONS 100 /* Number of times the mixer is called before it starts trying to improve latency */
#define MAX_DELTA_OBSERVATIONS 1000000 /* Number of times the mixer is called before we assume we truly understand timing */

static int diagnosed_too_slow = 0;

#define ACQUIRE_LOCK() P->_mixerLock.lock()
#define RELEASE_LOCK() P->_mixerLock.unlock()

struct sfx_pcm_feed_state_t {
	sfx_pcm_feed_t *feed;

	/* The following fields are for use by the mixer only and must not be
	** touched by pcm_feed code.  */
	byte *buf; /* dynamically allocated buffer for this feed, used in some circumstances. */
	int buf_size; /* Number of frames that fit into the buffer */
	sfx_pcm_urat_t spd; /* source frames per destination frames */
	sfx_pcm_urat_t scount; /* Frame counter, backed up in between calls */
	int frame_bufstart; /* Left-over frames at the beginning of the buffer */
	int mode; /* Whether the feed is alive or pending destruction */

	int pending_review; /* Timestamp needs to be checked for this stream */
	twochannel_data ch_old, ch_new; /* Intermediate results of output computation */
};

struct mixer_private {
	Common::Mutex _mixerLock;
	byte *outbuf; /* Output buffer to write to the PCM device next time */
	sfx_timestamp_t outbuf_timestamp; /* Timestamp associated with the output buffer */
	int have_outbuf_timestamp; /* Whether we really _have_ an associated timestamp */
	byte *writebuf; /* Buffer we're supposed to write to */
	int32 *compbuf_l, *compbuf_r; /* Intermediate buffers for computation */
	int lastbuf_len; /* Number of frames stored in the last buffer */

	uint32 skew; /* Millisecond relative to which we compute time. This is the millisecond
		   ** part of the first time we emitted sound, to simplify some computations.  */
	uint32 lsec; /* Last point in time we updated buffers, if any (seconds since the epoch) */
	int played_this_second; /* Number of frames emitted so far in second lsec */

	int max_delta; /* maximum observed time delta (using 'frames' as a metric unit) */
	int delta_observations; /* Number of times we played; confidence measure for max_delta */

	/* Pause data */
	int paused;

	sfx_pcm_config_t conf;
	int _framesize;
	Audio::AppendableAudioStream *_audioStream;
	Audio::SoundHandle _soundHandle;

	int feeds_nr;
	int feeds_allocd;
	sfx_pcm_feed_state_t *feeds;
};

#define P ((struct mixer_private *)(self->private_bits))

enum {
	BUF_SIZE = 2048 << 1
};

static int mix_init(sfx_pcm_mixer_t *self) {
	self->private_bits = new mixer_private();
	P->outbuf = P->writebuf = NULL;
	P->lastbuf_len = 0;
	P->compbuf_l = (int32*)sci_malloc(sizeof(int32) * BUF_SIZE);
	P->compbuf_r = (int32*)sci_malloc(sizeof(int32) * BUF_SIZE);
	P->played_this_second = 0;
	P->paused = 0;
	
	P->conf.rate = g_system->getMixer()->getOutputRate();
	P->conf.stereo = SFX_PCM_STEREO_LR;
	P->conf.format = SFX_PCM_FORMAT_S16_NATIVE;
	P->_framesize = SFX_PCM_FRAME_SIZE(P->conf);
	
	
	int flags = Audio::Mixer::FLAG_16BITS | Audio::Mixer::FLAG_STEREO;
#ifdef SCUMM_LITTLE_ENDIAN
	flags |= Audio::Mixer::FLAG_LITTLE_ENDIAN;
#endif

	P->_audioStream = Audio::makeAppendableAudioStream(P->conf.rate, flags);
	g_system->getMixer()->playInputStream(Audio::Mixer::kSFXSoundType, &P->_soundHandle, P->_audioStream);

	return SFX_OK;
}

static inline uint gcd(uint a, uint b) {
	while (a) {
		uint c = b % a;
		b = a;
		a = c;
	}
	return b;
}

static sfx_pcm_urat_t urat(unsigned int nom, unsigned int denom) {
	sfx_pcm_urat_t rv;
	unsigned int g;

	rv.val = nom / denom;
	nom -= rv.val * denom;
	if (nom == 0)
		g = 1;
	else
		g = gcd(nom, denom);

	rv.nom = nom / g;
	rv.den = denom / g;

	return rv;
}

static void mix_subscribe(sfx_pcm_mixer_t *self, sfx_pcm_feed_t *feed) {
	sfx_pcm_feed_state_t *fs;
	ACQUIRE_LOCK();
	if (!P->feeds) {
		P->feeds_allocd = 2;
		P->feeds = (sfx_pcm_feed_state_t*)sci_malloc(sizeof(sfx_pcm_feed_state_t)
		              * P->feeds_allocd);
	} else if (P->feeds_allocd == P->feeds_nr) {
		P->feeds_allocd += 2;
		P->feeds = (sfx_pcm_feed_state_t*)sci_realloc(P->feeds,
		              sizeof(sfx_pcm_feed_state_t)
		              * P->feeds_allocd);
	}

	fs = P->feeds + P->feeds_nr++;
	fs->feed = feed;

	feed->frame_size = SFX_PCM_FRAME_SIZE(feed->conf);

	/*	fs->buf_size = (BUF_SIZE
			  * (feed->conf.rate
			     + P->conf.rate - 1))
		/ P->conf.rate;
	*/
	/* For the sake of people without 64 bit CPUs: */
	fs->buf_size = 2 + /* Additional safety */
	               (BUF_SIZE *
	                (1 + (feed->conf.rate / P->conf.rate)));
	fprintf(stderr, " ---> %d/%d/%d/%d = %d\n",
	        BUF_SIZE,
	        feed->conf.rate,
	        P->conf.rate,
	        feed->frame_size,
	        fs->buf_size);

	fs->buf = (byte*)sci_malloc(fs->buf_size * feed->frame_size);
	fprintf(stderr, " ---> --> %d for %p at %p\n", fs->buf_size * feed->frame_size, (void *)fs, (void *)fs->buf);
	{
		int i;
		for (i = 0; i < fs->buf_size * feed->frame_size; i++)
			fs->buf[i] = 0xa5;
	}
	fs->scount = urat(0, 1);
	fs->spd = urat(feed->conf.rate, P->conf.rate);
	fs->scount.den = fs->spd.den;
	fs->ch_old.left = 0;
	fs->ch_old.right = 0;
	fs->ch_new.left = 0;
	fs->ch_new.right = 0;
	fs->mode = SFX_PCM_FEED_MODE_ALIVE;

	/* If the feed can't provide us with timestamps, we don't need to wait for it to do so */
	fs->pending_review = (feed->get_timestamp) ? 1 : 0;

	fs->frame_bufstart = 0;

#ifdef DEBUG
	sciprintf("[soft-mixer] Subscribed %s-%x (%d Hz, %d/%x) at %d+%d/%d, buffer size %d\n",
	          feed->debug_name, feed->debug_nr, feed->conf.rate, feed->conf.stereo, feed->conf.format,
	          fs->spd.val, fs->spd.nom, fs->spd.den, fs->buf_size);
#endif
	RELEASE_LOCK();
}


static void _mix_unsubscribe(sfx_pcm_mixer_t *self, sfx_pcm_feed_t *feed) {
	int i;
#ifdef DEBUG
	sciprintf("[soft-mixer] Unsubscribing %s-%x\n", feed->debug_name, feed->debug_nr);
#endif
	for (i = 0; i < P->feeds_nr; i++) {
		sfx_pcm_feed_state_t *fs = P->feeds + i;

		if (fs->feed == feed) {
			feed->destroy(feed);

			if (fs->buf)
				free(fs->buf);

			P->feeds_nr--;

			/* Copy topmost into deleted so that we don't have any holes */
			if (i != P->feeds_nr)
				P->feeds[i] = P->feeds[P->feeds_nr];

			if (P->feeds_allocd > 8 && P->feeds_allocd > (P->feeds_nr << 1)) {
				/* Limit memory waste */
				P->feeds_allocd >>= 1;
				P->feeds
				= (sfx_pcm_feed_state_t*)sci_realloc(P->feeds,
				                                     sizeof(sfx_pcm_feed_state_t)
				                                     * P->feeds_allocd);
			}

			for (i = 0; i < P->feeds_nr; i++)
				fprintf(stderr, "  Feed #%d: %s-%x\n",
				        i, P->feeds[i].feed->debug_name,
				        P->feeds[i].feed->debug_nr);

			return;
		}
	}

	fprintf(stderr, "[sfx-mixer] Assertion failed: Deleting invalid feed %p out of %d\n",
	        (void *)feed, P->feeds_nr);

	BREAKPOINT();
}

static void mix_exit(sfx_pcm_mixer_t *self) {
	g_system->getMixer()->stopHandle(P->_soundHandle);
	P->_audioStream = 0;

	ACQUIRE_LOCK();
	while (P->feeds_nr)
		_mix_unsubscribe(self, P->feeds[0].feed);
	RELEASE_LOCK();

	free(P->outbuf);
	free(P->writebuf);

	free(P->compbuf_l);
	free(P->compbuf_r);

	delete P;
	self->private_bits = NULL;

#ifdef DEBUG
	sciprintf("[soft-mixer] Uninitialising mixer\n");
#endif
}


#define LIMIT_16_BITS(v)			\
		if (v < -32767)			\
			v = -32768;		\
		else if (v > 32766)		\
			v = 32767

static inline void mix_compute_output(sfx_pcm_mixer_t *self, int outplen) {
	int frame_i;
	const sfx_pcm_config_t conf = P->conf;
	int use_16 = conf.format & SFX_PCM_FORMAT_16;
	int bias = conf.format & ~SFX_PCM_FORMAT_LMASK;
	byte *lchan, *rchan = NULL;
	/* Don't see how this could possibly wind up being
	** used w/o initialisation, but you never know... */
	int32 *lsrc = P->compbuf_l;
	int32 *rsrc = P->compbuf_r;
	int frame_size = SFX_PCM_FRAME_SIZE(conf);


	if (!P->writebuf)
		P->writebuf = (byte*)sci_malloc(BUF_SIZE * frame_size + 4);

	if (conf.stereo) {
		if (conf.stereo == SFX_PCM_STEREO_RL) {
			lchan = P->writebuf + ((use_16) ? 2 : 1);
			rchan = P->writebuf;
		} else {
			lchan = P->writebuf;
			rchan = P->writebuf + ((use_16) ? 2 : 1);
		}
	} else
		lchan = P->writebuf;


	for (frame_i = 0; frame_i < outplen; frame_i++) {
		int left = *lsrc++;
		int right = *rsrc++;

		if (conf.stereo) {
			LIMIT_16_BITS(left);
			LIMIT_16_BITS(right);

			if (!use_16) {
				left >>= 8;
				right >>= 8;
			}

			left += bias;
			right += bias;

			if (use_16) {
				if (SFX_PCM_FORMAT_LE == (conf.format & SFX_PCM_FORMAT_ENDIANNESS)) {
					lchan[0] = left & 0xff;
					lchan[1] = (left >> 8) & 0xff;
					rchan[0] = right & 0xff;
					rchan[1] = (right >> 8) & 0xff;
				} else {
					lchan[1] = left & 0xff;
					lchan[0] = (left >> 8) & 0xff;
					rchan[1] = right & 0xff;
					rchan[0] = (right >> 8) & 0xff;
				}

				lchan += 4;
				rchan += 4;
			} else {
				*lchan = left & 0xff;
				*rchan = right & 0xff;

				lchan += 2;
				rchan += 2;
			}

		} else {
			left += right;
			left >>= 1;
			LIMIT_16_BITS(left);
			if (!use_16)
				left >>= 8;

			left += bias;

			if (use_16) {
				if (SFX_PCM_FORMAT_LE == (conf.format & SFX_PCM_FORMAT_ENDIANNESS)) {
					lchan[0] = left & 0xff;
					lchan[1] = (left >> 8) & 0xff;
				} else {
					lchan[1] = left & 0xff;
					lchan[0] = (left >> 8) & 0xff;
				}

				lchan += 2;
			} else {
				*lchan = left & 0xff;
				lchan += 1;
			}
		}
	}
}

static inline void mix_swap_buffers(sfx_pcm_mixer_t *self) { /* Swap buffers */
	byte *tmp = P->outbuf;
	P->outbuf = P->writebuf;
	P->writebuf = tmp;
}

static inline int mix_compute_buf_len(sfx_pcm_mixer_t *self, int *skip_frames) {
/* Computes the number of frames we ought to write. It tries to minimise the number,
** in order to reduce latency. */
/* It sets 'skip_frames' to the number of frames to assume lost by latency, effectively
** skipping them.  */
	int free_frames;
	int played_frames = 0; /* since the last call */
	uint32 msecs;
	int frame_pos;
	int result_frames;

	msecs = g_system->getMillis();

	if (!P->outbuf) {
		/* Called for the first time ever? */
		P->skew = msecs % 1000;
		P->lsec = msecs / 1000;
		P->max_delta = 0;
		P->delta_observations = 0;
		P->played_this_second = 0;
		*skip_frames = 0;

		return BUF_SIZE;
	}

	/*	fprintf(stderr, "[%d:%d]S%d ", secs, usecs, P->skew);*/

	msecs -= P->skew;

	frame_pos = (msecs % 1000) * P->conf.rate / 1000;

	played_frames = frame_pos - P->played_this_second
	                + ((msecs / 1000 - P->lsec) * P->conf.rate);
	/*
	fprintf(stderr, "%d:%d - %d:%d  => %d\n", secs, frame_pos,
		P->lsec, P->played_this_second, played_frames);
	*/

	if (played_frames > BUF_SIZE)
		played_frames = BUF_SIZE;

	/*
	fprintf(stderr, "Between %d:? offset=%d and %d:%d offset=%d: Played %d at %d\n", P->lsec, P->played_this_second,
	secs, usecs, frame_pos, played_frames, P->conf.rate);
	*/


	if (played_frames > P->max_delta)
		P->max_delta = played_frames;

	free_frames = played_frames;

	if (free_frames > BUF_SIZE) {
		if (!diagnosed_too_slow) {
			sciprintf("[sfx-mixer] Your timer is too slow for your PCM output device (%d/%d), free=%d.\n"
			          "[sfx-mixer] You might want to try changing the device, timer, or mixer, if possible.\n",
			          played_frames, BUF_SIZE, free_frames);
		}
		diagnosed_too_slow = 1;

		*skip_frames = free_frames - BUF_SIZE;
		free_frames = BUF_SIZE;
	} else
		*skip_frames = 0;

	++P->delta_observations;
	if (P->delta_observations > MAX_DELTA_OBSERVATIONS)
		P->delta_observations = MAX_DELTA_OBSERVATIONS;

	/*	/\* Disabled, broken *\/ */
	/*	if (0 && P->delta_observations > MIN_DELTA_OBSERVATIONS) { /\* Start improving after a while *\/ */
	/*		int diff = P->conf.rate - P->max_delta; */

	/*		/\* log-approximate P->max_delta over time *\/ */
	/*		recommended_frames = P->max_delta + */
	/*			((diff * MIN_DELTA_OBSERVATIONS) / P->delta_observations); */
	/* /\* WTF? *\/ */
	/*	} else */
	/*		recommended_frames = BUF_SIZE; /\* Initially, keep the buffer full *\/ */

#if (DEBUG >= 1)
	sciprintf("[soft-mixer] played since last time: %d, free: %d\n",
	          played_frames, free_frames);
#endif

	result_frames = free_frames;

	if (result_frames < 0)
		result_frames = 0;

	P->played_this_second += result_frames;
	while (P->played_this_second >= P->conf.rate) {
		/* Won't normally happen more than once */
		P->played_this_second -= P->conf.rate;
		P->lsec++;
	}

	if (result_frames > BUF_SIZE) {
		fprintf(stderr, "[soft-mixer] Internal assertion failed: frames-to-write %d > %d\n",
		        result_frames, BUF_SIZE);
	}
	return result_frames;
}



#define READ_NEW_VALUES() \
		if (frames_left > 0) {						\
			if (bias) { /* unsigned data */					\
				if (!use_16) {						\
					c_new.left = (*lsrc) << 8;			\
					c_new.right = (*rsrc) << 8;			\
				} else {						\
					if (conf.format & SFX_PCM_FORMAT_LE) {		\
						c_new.left = lsrc[0] | lsrc[1] << 8;	\
						c_new.right = rsrc[0] | rsrc[1] << 8;	\
					} else {					\
						c_new.left = lsrc[1] | lsrc[0] << 8;	\
						c_new.right = rsrc[1] | rsrc[0] << 8;	\
					}						\
				}							\
			} else { /* signed data */							\
				if (!use_16) {								\
					c_new.left = (*((signed char *)lsrc)) << 8;			\
					c_new.right = (*((signed char *)rsrc)) << 8;			\
				} else {								\
					if (conf.format & SFX_PCM_FORMAT_LE) {				\
						c_new.left = lsrc[0] | ((signed char *)lsrc)[1] << 8;	\
						c_new.right = rsrc[0] | ((signed char *)rsrc)[1] << 8;	\
					} else {							\
						c_new.left = lsrc[1] | ((signed char *)lsrc)[0] << 8;	\
						c_new.right = rsrc[1] | ((signed char *)rsrc)[0] << 8;	\
					}								\
				}									\
			}										\
										\
			c_new.left -= bias;					\
			c_new.right -= bias;					\
										\
			lsrc += frame_size;					\
			rsrc += frame_size;					\
		} else {							\
			c_new.left = c_new.right = 0;				\
			break;							\
		}


static volatile int xx_offset;
static volatile int xx_size;

static void mix_compute_input_linear(sfx_pcm_mixer_t *self, int add_result,
									 int len, sfx_timestamp_t *ts, sfx_timestamp_t base_ts) {
/* if add_result is non-zero, P->outbuf should be added to rather than overwritten. */
/* base_ts is the timestamp for the first frame */
	sfx_pcm_feed_state_t *fs = P->feeds + add_result;
	sfx_pcm_feed_t *f = fs->feed;
	sfx_pcm_config_t conf = f->conf;
	int use_16 = conf.format & SFX_PCM_FORMAT_16;
	int32 *lchan = P->compbuf_l;
	int32 *rchan = P->compbuf_r;
	int frame_size = f->frame_size;
	byte *wr_dest = fs->buf + (frame_size * fs->frame_bufstart);
	byte *lsrc = fs->buf;
	byte *rsrc = fs->buf;
	/* Location to write to */
	int frames_nr;
	int bias = (conf.format & ~SFX_PCM_FORMAT_LMASK) ? 0x8000 : 0;
	/* We use this only on a 16 bit level here */

	/* The two most extreme source frames we consider for a
	** destination frame  */
	struct twochannel_data c_old = fs->ch_old;
	struct twochannel_data c_new = fs->ch_new;

	int frames_read = 0;
	int frames_left;
	int write_offset; /* Iterator for translation */
	int delay_frames = 0; /* Number of frames (dest buffer) at the beginning we skip */

	/* First, compute the number of frames we want to retrieve */
	frames_nr = fs->spd.val * len;
	/* A little complicated since we must consider partial frames */
	frames_nr += (fs->spd.nom * len
	              + (fs->scount.den - fs->scount.nom) /* remember that we may have leftovers */
	              + (fs->spd.den - 1 /* round up */)
	             )
	             / fs->spd.den;

	ts->msecs = 0;

	if (frames_nr > fs->buf_size) {
		fprintf(stderr, "%d (%d*%d + somethign) bytes, but only %d allowed!!!!!\n",
		        frames_nr * f->frame_size,
		        fs->spd.val, len,
		        fs->buf_size);
		BREAKPOINT();
	}

	if (fs->pending_review) {
		int newmode = PCM_FEED_EMPTY; /* empty unless a get_timestamp() tells otherwise */

		RELEASE_LOCK();
		/* Retrieve timestamp */
		if (f->get_timestamp)
			newmode = f->get_timestamp(f, ts);
		ACQUIRE_LOCK();

		fs = P->feeds + add_result;
		/* Reset in case of status update */

		switch (newmode) {

		case PCM_FEED_TIMESTAMP: {
			/* Compute the number of frames the returned timestamp is in the future: */
			delay_frames =
			    sfx_timestamp_frame_diff(sfx_timestamp_renormalise(*ts, base_ts.frame_rate),
			                             base_ts);

			if (delay_frames <= 0)
				/* Start ASAP, even if it's too late */
				delay_frames = 0;
			else
				if (delay_frames > len)
					delay_frames = len;
			fs->pending_review = 0;
		}
		break;

		case PCM_FEED_EMPTY:
			fs->mode = SFX_PCM_FEED_MODE_DEAD;

			/* ...fall through... */

		case PCM_FEED_IDLE:
			/* Clear audio buffer, if neccessary, and return */
			if (!add_result) {
				memset(P->compbuf_l, 0, sizeof(int32) * len);
				memset(P->compbuf_r, 0, sizeof(int32) * len);
			}
			return;

		default:
			error("[soft-mixer] Fatal: Invalid mode returned by PCM feed %s-%d's get_timestamp(): %d",
			        f->debug_name, f->debug_nr, newmode);
		}
	}

	RELEASE_LOCK();
	/* Make sure we have sufficient information */
	if (frames_nr > delay_frames + fs->frame_bufstart)
		frames_read =
		    f->poll(f, wr_dest,
		            frames_nr
		            - delay_frames
		            - fs->frame_bufstart);

	ACQUIRE_LOCK();
	fs = P->feeds + add_result;

	frames_read += fs->frame_bufstart;
	frames_left = frames_read;

	/* Reset in case of status update */

	/* Skip at the beginning: */
	if (delay_frames) {
		if (!add_result) {
			memset(lchan, 0, sizeof(int32) * delay_frames);
			memset(rchan, 0, sizeof(int32) * delay_frames);
		}
		lchan += delay_frames;
		rchan += delay_frames;

		len -= delay_frames;
	}


#if (DEBUG >= 2)
	sciprintf("[soft-mixer] Examining %s-%x (frame size %d); read %d/%d/%d, re-using %d frames\n",
	          f->debug_name, f->debug_nr, frame_size, frames_read, frames_nr,
	          fs->buf_size, fs->frame_bufstart);
#endif


	if (conf.stereo == SFX_PCM_STEREO_LR)
		rsrc += (use_16) ? 2 : 1;
	else if (conf.stereo == SFX_PCM_STEREO_RL)
		lsrc += (use_16) ? 2 : 1;
	/* Otherwise, we let both point to the same place */

#if (DEBUG >= 2)
	sciprintf("[soft-mixer] Stretching theoretical %d (physical %d) results to %d\n", frames_nr, frames_left, len);
#endif
	for (write_offset = 0; write_offset < len; write_offset++) {
		int leftsum = 0; /* Sum of any complete frames we used */
		int rightsum = 0;

		int left; /* Sum of the two most extreme source frames
			  ** we considered, i.e. the oldest and newest
			  ** one corresponding to the output frame we are
			  ** computing  */
		int right;

		int frame_steps = fs->spd.val;
		int j;

		if (fs->scount.nom >= fs->scount.den) {
			fs->scount.nom -= fs->scount.den; /* Ensure fractional part < 1 */
			++frame_steps;
		}
		if (frame_steps)
			c_old = c_new;

#if 0
		if (write_offset == 0) {
			READ_NEW_VALUES();
			--frames_left;
#if (DEBUG >= 3)
			sciprintf("[soft-mixer] Initial read %d:%d\n", c_new.left, c_new.right);
#endif
			c_old = c_new;
		}
#endif

		for (j = 0; j < frame_steps; j++) {
			READ_NEW_VALUES();
			--frames_left;
#if (DEBUG >= 3)
			sciprintf("[soft-mixer] Step %d/%d made %d:%d\n", j, frame_steps, c_new.left, c_new.right);
#endif

			/* The last frame will be subject to the fractional
			** part analysis, so we add it to 'left' and 'right'
			** later-- all others are added to (leftsum, rightsum).
			*/
			if (j + 1 < frame_steps) {
				leftsum += c_new.left;
				rightsum += c_new.right;
			}
		}

		left = c_new.left * fs->scount.nom
		       + c_old.left * (fs->scount.den - fs->scount.nom);
		right = c_new.right * fs->scount.nom
		        + c_old.right * (fs->scount.den - fs->scount.nom);

		/* Normalise */
		left  /= fs->spd.den;
		right /= fs->spd.den;


		leftsum += left;
		rightsum += right;


		/* Make sure to divide by the number of frames we added here */
		if (frame_steps > 1) {
			leftsum  /= (frame_steps);
			rightsum /= (frame_steps);
		}


#if (DEBUG >= 3)
		sciprintf("[soft-mixer] Ultimate result: %d:%d (frac %d:%d)\n", leftsum, rightsum, left, right);
#endif

		if (add_result) {
			*(lchan++) += leftsum;
			*(rchan++) += rightsum;
		} else {
			*(lchan++) = leftsum;
			*(rchan++) = rightsum;
		}

		fs->scount.nom += fs->spd.nom; /* Count up fractional part */
	}

	fs->ch_old = c_old;
	fs->ch_new = c_new;

	/* If neccessary, zero out the rest */
	if (write_offset < len && !add_result) {
		memset(lchan, 0, sizeof(int32) * (len - write_offset));
		memset(rchan, 0, sizeof(int32) * (len - write_offset));
	}

	/* Save whether we have a partial frame still stored */
	fs->frame_bufstart = frames_left;

	if (frames_left) {
		xx_offset = ((frames_read - frames_left) * f->frame_size);
		xx_size = frames_left * f->frame_size;
		if (xx_offset + xx_size
		        >= fs->buf_size * f->frame_size) {
			fprintf(stderr, "offset %d >= max %d!\n",
			        (xx_offset + xx_size), fs->buf_size * f->frame_size);
			BREAKPOINT();
		}

		memmove(fs->buf,
		        fs->buf + ((frames_read - frames_left) * f->frame_size),
		        frames_left * f->frame_size);
	}
#if (DEBUG >= 2)
	sciprintf("[soft-mixer] Leaving %d over\n", fs->frame_bufstart);
#endif

	if (frames_read + delay_frames < frames_nr) {
		if (f->get_timestamp) /* Can resume? */
			fs->pending_review = 1;
		else
			fs->mode = SFX_PCM_FEED_MODE_DEAD; /* Done. */
	}
}

static int mix_process_linear(sfx_pcm_mixer_t *self) {
	ACQUIRE_LOCK();
	{
		int src_i; /* source feed index counter */
		int frames_skip; /* Number of frames to discard, rather than to emit */
		int buflen = mix_compute_buf_len(self, &frames_skip); /* Compute # of frames we must compute and write */
		int fake_buflen;
		int timestamp_max_delta = 0;
		int have_timestamp = 0;
		sfx_timestamp_t start_timestamp; /* The timestamp at which the first frame will be played */
		sfx_timestamp_t min_timestamp;
		min_timestamp.msecs = 0;
		sfx_timestamp_t timestamp;

//		if (self->dev->get_output_timestamp)
//			start_timestamp = self->dev->get_output_timestamp(self->dev);
//		else
			start_timestamp = sfx_new_timestamp(g_system->getMillis(), P->conf.rate);

		if ((P->outbuf) && (P->lastbuf_len)) {
			sfx_timestamp_t ts;
			int rv;

			if (P->have_outbuf_timestamp) {
				ts = sfx_timestamp_renormalise(P->outbuf_timestamp, P->conf.rate);
			}

			const int totalBufSize = P->lastbuf_len * P->_framesize;
			byte *buf = new byte[totalBufSize];
			if (!buf) {
				RELEASE_LOCK();
				return rv; /* error */
			}
			memcpy(buf, P->outbuf, totalBufSize);
			P->_audioStream->queueBuffer(buf, totalBufSize);
			// TODO: We currently ignore the timestamp:
			//    (P->have_outbuf_timestamp) ? &ts : NULL
			// Re-add support for that? Maybe by enhancing the ScummVM mixer (i.e.,
			// expanding the getTotalPlayTime() audiostream API to "proper" timestamps?)
		}

#if (DEBUG >= 1)
		if (P->feeds_nr)
			sciprintf("[soft-mixer] Mixing %d output frames on %d input feeds\n", buflen, P->feeds_nr);
#endif
		if (P->feeds_nr && !P->paused) {
			/* Below, we read out all feeds in case we have to skip frames first, then get the
			** most current sound. 'fake_buflen' is either the actual buflen (for the last iteration)
			** or a fraction of the buf length to discard.  */
			do {
				if (frames_skip) {
					if (frames_skip > BUF_SIZE)
						fake_buflen = BUF_SIZE;
					else
						fake_buflen = frames_skip;

					frames_skip -= fake_buflen;
				} else {
					fake_buflen = buflen;
					frames_skip = -1; /* Mark us as being completely done */
				}

				for (src_i = 0; src_i < P->feeds_nr; src_i++) {
					mix_compute_input_linear(self, src_i,
					                         fake_buflen, &timestamp,
					                         start_timestamp);

					if (timestamp.msecs > 0) {
						if (have_timestamp) {
							int diff = sfx_timestamp_msecs_diff(min_timestamp, timestamp);
							if (diff > 0) {
								/* New earlier timestamp */
								timestamp = min_timestamp;
								timestamp_max_delta += diff;
							} else if (diff > timestamp_max_delta)
								timestamp_max_delta = diff;
							/* New max delta for timestamp */
						} else {
							min_timestamp = timestamp;
							have_timestamp = 1;
						}
					}
				}
				/* Destroy all feeds we finished */
				for (src_i = 0; src_i < P->feeds_nr; src_i++)
					if (P->feeds[src_i].mode == SFX_PCM_FEED_MODE_DEAD)
						_mix_unsubscribe(self, P->feeds[src_i].feed);
			} while (frames_skip >= 0);

		} else { /* Zero it out */
			memset(P->compbuf_l, 0, sizeof(int32) * buflen);
			memset(P->compbuf_r, 0, sizeof(int32) * buflen);
		}

#if (DEBUG >= 1)
		if (P->feeds_nr)
			sciprintf("[soft-mixer] Done mixing for this session, the result will be our next output buffer\n");
#endif

#if (DEBUG >= 3)
		if (P->feeds_nr) {
			int i;
			sciprintf("[soft-mixer] Intermediate representation:\n");
			for (i = 0; i < buflen; i++)
				sciprintf("[soft-mixer] Offset %d:\t[%04x:%04x]\t%d:%d\n", i,
				          P->compbuf_l[i] & 0xffff, P->compbuf_r[i] & 0xffff,
				          P->compbuf_l[i], P->compbuf_r[i]);
		}
#endif

		if (timestamp_max_delta > TIMESTAMP_MAX_ALLOWED_DELTA)
			sciprintf("[soft-mixer] Warning: Difference in timestamps between audio feeds is %d ms\n", timestamp_max_delta);

		mix_compute_output(self, buflen);
		P->lastbuf_len = buflen;

		/* Finalize */
		mix_swap_buffers(self);
		if (have_timestamp)
			P->outbuf_timestamp = sfx_timestamp_add(min_timestamp,
			                                        timestamp_max_delta * 500);
		P->have_outbuf_timestamp = have_timestamp;

	}
	RELEASE_LOCK();
	return SFX_OK;
}

static void mix_pause(sfx_pcm_mixer_t *self) {
	ACQUIRE_LOCK();
	P->paused = 1;
	RELEASE_LOCK();
}

static void mix_resume(sfx_pcm_mixer_t *self) {
	ACQUIRE_LOCK();
	P->paused = 0;
	RELEASE_LOCK();
}

sfx_pcm_mixer_t sfx_pcm_mixer_soft_linear = {
	"soft-linear",
	"0.1",

	mix_init,
	mix_exit,
	mix_subscribe,
	mix_pause,
	mix_resume,
	mix_process_linear,

	NULL
};

sfx_pcm_mixer_t *getMixer() { return &sfx_pcm_mixer_soft_linear; }

} // End of namespace Sci
