/***************************************************************************
 sfx_mixer.h Copyright (C) 2003,04 Christoph Reichenbach


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

#ifndef _SFX_MIXER_H_
#define _SFX_MIXER_H_

#include <sfx_pcm.h>


#define SFX_PCM_FEED_MODE_ALIVE 0
#define SFX_PCM_FEED_MODE_DEAD 1

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
	struct twochannel_data ch_old, ch_new; /* Intermediate results of output computation */
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
