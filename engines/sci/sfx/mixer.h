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

#ifndef _SFX_MIXER_H_
#define _SFX_MIXER_H_

#include "sci/include/sfx_pcm.h"


#define SFX_PCM_FEED_MODE_ALIVE 0
#define SFX_PCM_FEED_MODE_DEAD 1

/** Finitary unsigned rational numbers */
struct sfx_pcm_urat_t {
	int nom, den;
	int val;

	/* Total value: val + nom/den, where (nom < den) guaranteed. */
};

struct twochannel_data {
	int left, right;
};

typedef struct {
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
} sfx_pcm_feed_state_t;


typedef struct _sfx_pcm_mixer {
	/* Mixers are the heart of all matters PCM. They take PCM data from subscribed feeds,
	** mix it (hence the name) and ask the pcm device they are attached to to play the
	** result.  */

	const char *name;
	const char *version;

	int (*init)(struct _sfx_pcm_mixer *self, sfx_pcm_device_t *device);
	/* Initialises the mixer
	** Parameters: (sfx_pcm_mixer_t *) self: Self reference
	**             (sfx_pcm_device_t *) device: An _already initialised_ PCM output driver
	** Returns   : (int) SFX_OK on success, SFX_ERROR otherwise
	*/

	void (*exit)(struct _sfx_pcm_mixer *self);
	/* Uninitialises the mixer
	** Parameters: (sfx_pcm_mixer_t *) self: Self reference
	** Also uninitialises all feeds and the attached output device.
	*/

	void (*subscribe)(struct _sfx_pcm_mixer *self, sfx_pcm_feed_t *feed);
	/* Subscribes the mixer to a new feed
	** Parameters: (sfx_pcm_mixer_t *) self: Self reference
	**             (sfx_pcm_feed_t *) feed: The feed to subscribe to
	*/

	void (*pause)(struct _sfx_pcm_mixer *self);
	/* Pauses the processing of input and output
	*/

	void (*resume)(struct _sfx_pcm_mixer *self);
	/* Resumes the processing of input and output after a pause
	*/

	int (*process)(struct _sfx_pcm_mixer *self);
	/* Processes all feeds, mixes their results, and passes everything to the output device
	** Returns  : (int) SFX_OK on success, SFX_ERROR otherwise (output device error or
	**                  internal assertion failure)
	** Effects  : All feeds are poll()ed, and the device is asked to output(). Buffer size
	**            depends on the time that has passed since the last call to process(), if
	**            any.
	*/

	int feeds_nr;
	int feeds_allocd;
	sfx_pcm_feed_state_t *feeds;
	sfx_pcm_device_t *dev;

	void *private_bits;
} sfx_pcm_mixer_t;

sfx_pcm_mixer_t *
sfx_pcm_find_mixer(char *name);
/* Looks up a mixer by name, or a default mixer
** Parameters: (char *) name: Name of the mixer to look for, or NULL to
**                            take a default
*/

extern sfx_pcm_mixer_t *mixer; /* _THE_ global pcm mixer */

#endif /* !defined(_SFX_MIXER_H_) */
