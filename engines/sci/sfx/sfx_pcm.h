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

#ifndef SCI_SFX_SFX_PCM_H
#define SCI_SFX_SFX_PCM_H

#include "sci/sfx/sfx_core.h"
#include "sci/sfx/sfx_time.h"

namespace Sci {

#define SFX_PCM_MONO 0
#define SFX_PCM_STEREO_LR 1 /* left sample, then right sample */
#define SFX_PCM_STEREO_RL 2 /* right sample, then left sample */

/* The following are used internally by the mixer */
#define SFX_PCM_FORMAT_LMASK 0x7
#define SFX_PCM_FORMAT_BE 0
#define SFX_PCM_FORMAT_LE 1
#define SFX_PCM_FORMAT_ENDIANNESS 1
#define SFX_PCM_FORMAT_8  0
#define SFX_PCM_FORMAT_16 2


/* Pick one of these formats (including the _NATIVE) ones for your PCM feed */
#define SFX_PCM_FORMAT_U8     (0x0080 | SFX_PCM_FORMAT_8)			/* Unsigned (bias 128) 8 bit format */
#define SFX_PCM_FORMAT_S8     (0x0000 | SFX_PCM_FORMAT_8)			/* Signed 8 bit format */
#define SFX_PCM_FORMAT_U16_LE (0x8000 | SFX_PCM_FORMAT_16 | SFX_PCM_FORMAT_LE)	/* Unsigned (bias 32768) 16 bit LE format */
#define SFX_PCM_FORMAT_S16_LE (0x0000 | SFX_PCM_FORMAT_16 | SFX_PCM_FORMAT_LE)	/* Signed 16 bit format, little endian */
#define SFX_PCM_FORMAT_U16_BE (0x8000 | SFX_PCM_FORMAT_16 | SFX_PCM_FORMAT_BE)	/* Unsigned (bias 32768) 16 bit BE format */
#define SFX_PCM_FORMAT_S16_BE (0x0000 | SFX_PCM_FORMAT_16 | SFX_PCM_FORMAT_BE)	/* Signed 16 bit format, big endian */

#ifdef SCUMM_BIG_ENDIAN
#  define SFX_PCM_FORMAT_U16_NATIVE SFX_PCM_FORMAT_U16_BE
#  define SFX_PCM_FORMAT_S16_NATIVE SFX_PCM_FORMAT_S16_BE
#else
#  define SFX_PCM_FORMAT_U16_NATIVE SFX_PCM_FORMAT_U16_LE
#  define SFX_PCM_FORMAT_S16_NATIVE SFX_PCM_FORMAT_S16_LE
#endif

#define SFX_PCM_FRAME_SIZE(conf) ((conf).stereo? 2 : 1) * (((conf).format & SFX_PCM_FORMAT_16)? 2 : 1)


struct sfx_pcm_config_t {
	int rate;   /* Sampling rate */
	int stereo; /* The stereo mode used (SFX_PCM_MONO or SFX_PCM_STEREO_*) */
	unsigned int format; /* Sample format (SFX_PCM_FORMAT_*) */
};

#define PCM_FEED_TIMESTAMP 0	/* New timestamp available */
#define PCM_FEED_IDLE 1		/* No sound ATM, but new timestamp may be available later */
#define PCM_FEED_EMPTY 2	/* Feed is finished, can be destroyed */

struct sfx_pcm_feed_t {
	/* PCM feeds are sources of input for the PCM mixer. Their member functions
	** are invoked as callbacks on demand, to provide the mixer with input it
	** (in turn) passes on to PCM output devices.
	**   PCM feeds must explicitly register themselves with the mixer in order
	** to be considered.
	*/

	int (*poll)(sfx_pcm_feed_t *self, byte *dest, int size);
	/* Asks the PCM feed to write out the next stuff it would like to have written
	** Parameters: (sfx_pcm_feed_t *) self: Self reference
	**             (byte *) dest: The destination buffer to write to
	**             (int) size: The maximum number of _frames_ (not neccessarily bytes)
	**                         to write
	** Returns   : (int) The number of frames written
	** If the number of frames written is smaller than 'size', the PCM feed will
	** be queried for a new timestamp afterwards, or destroyed if no new timestamp
	** is available.
	*/

	void (*destroy)(sfx_pcm_feed_t *self);
	/* Asks the PCM feed to free all resources it occupies
	** Parameters: (sfx_pcm_feed_t *) self: Self reference
	** free(self) should be part of this function, if applicable.
	*/

	int
	(*get_timestamp)(sfx_pcm_feed_t *self, sfx_timestamp_t *timestamp);
	/* Determines the timestamp of the next frame-to-read
	** Returns   : (sfx_timestamp_t) timestamp: The timestamp of the next frame
	**             (int) PCM_FEED_*
	** This function is OPTIONAL and may be NULL
	*/

	void *internal; /* The private bits of a PCM feed. */

	sfx_pcm_config_t conf; /* The channel's setup */

	const char *debug_name; /* The channel name, for debugging */
	int debug_nr; /* A channel number relative to the channel name, for debugging
		      ** (print in hex)  */
	int frame_size; /* Frame size, computed by the mixer for the feed */

};

int sfx_pcm_available();
/* Determines whether a PCM device is available and has been initialised
** Returns   : (int) zero iff no PCM device is available
*/

} // End of namespace Sci

#endif // SCI_SFX_SFX_PCM_H
