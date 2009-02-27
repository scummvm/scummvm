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

#ifndef SCI_SFX_SFX_ITERATOR_INTERNAL
#define SCI_SFX_SFX_ITERATOR_INTERNAL

#include "sci/sfx/sfx_iterator.h"
#include "sci/sfx/sci_midi.h"

namespace Sci {

/* States */

#define SI_STATE_UNINITIALISED -1
#define SI_STATE_DELTA_TIME 0 /* Now at a delta time */
#define SI_STATE_COMMAND 1 /* Now at a MIDI operation */
#define SI_STATE_PENDING 2 /* Pending for loop */
#define SI_STATE_FINISHED 3 /* End of song */
#define SI_STATE_PCM 4 /* Should report a PCM next (-> DELTA_TIME) */
#define SI_STATE_PCM_MAGIC_DELTA 5 /* Should report a ``magic'' one tick delta time next (goes on to FINISHED) */


/* Iterator types */

#define SCI_SONG_ITERATOR_TYPE_SCI0 0
#define SCI_SONG_ITERATOR_TYPE_SCI1 1

#define SIPFX __FILE__" : "


struct song_iterator_channel_t {
	int state;	/* SI_STATE_* */
	int offset;     /* Offset into the data chunk */
	int end;	/* Last allowed byte in track */
	int id;		/* Some channel ID */
	int loop_offset;
	int delay;	/* Number of ticks before the
			** specified channel is next
			** used, or
			** CHANNEL_DELAY_MISSING to
			** indicate that the delay has
			** not yet been read  */

	/* Two additional offsets for recovering: */
	int initial_offset;
	int playmask; /* Active playmask (MIDI channels to play in here) */
	int notes_played; /* #of notes played since the last loop start */
	int loop_timepos; /* Total delay for this channel's loop marker */
	int total_timepos; /* Number of ticks since the beginning, ignoring loops */
	int timepos_increment; /* Number of ticks until the next command (to add) */

	int saw_notes;  /* Bitmask of channels we have currently played notes on */
	byte last_cmd;	/* Last operation executed, for running status */
};

#define INHERITS_BASE_SONG_ITERATOR								\
	INHERITS_SONG_ITERATOR; /* aka "extends song iterator" */				\
												\
	int polyphony[MIDI_CHANNELS]; /* # of simultaneous notes on each */			\
	int importance[MIDI_CHANNELS]; /* priority rating for each channel, 0 means unrated. */			\
												\
												\
	int ccc; /* Cumulative cue counter, for those who need it */				\
	unsigned char resetflag; /* for 0x4C -- on DoSound StopSound, do we return to start? */	\
	int device_id; /* ID of the device we generating events for */				\
	int active_channels; /* Number of active channels */					\
	unsigned int size; /* Song size */							\
	unsigned char *data;									\
												\
	int loops; /* Number of loops remaining */						\
	int recover_delay

struct base_song_iterator_t {
	INHERITS_BASE_SONG_ITERATOR;
};

/********************************/
/*--------- SCI 0 --------------*/
/********************************/

struct sci0_song_iterator_t {
	INHERITS_BASE_SONG_ITERATOR;
	song_iterator_channel_t channel;
	int delay_remaining; /* Number of ticks that haven't been polled yet */
};


/********************************/
/*--------- SCI 1 --------------*/
/********************************/


struct sci1_sample_t {
	int delta; /* Time left-- initially, this is 'Sample point 1'.
		   ** After initialisation, it is 'sample point 1 minus the sample point of the previous sample'  */
	int size;
	int announced; /* Announced for download (SI_PCM) */
	sfx_pcm_config_t format;
	byte *data;
	sci1_sample_t *next;
};

struct sci1_song_iterator_t {
	INHERITS_BASE_SONG_ITERATOR;
	song_iterator_channel_t channels[MIDI_CHANNELS];

	/* Invariant: Whenever channels[i].delay == CHANNEL_DELAY_MISSING,
	** channel_offset[i] points to a delta time object. */

	int initialised; /* Whether the MIDI channel setup has been initialised */
	int channels_nr; /* Number of channels actually used */
	sci1_sample_t *next_sample;
	int channels_looped; /* Number of channels that are ready to loop */

	int delay_remaining; /* Number of ticks that haven't been polled yet */
	int hold;
};

#define PLAYMASK_NONE 0x0

/*********************************/
/*---------- Cleanup ------------*/
/*********************************/


song_iterator_t *new_cleanup_iterator(unsigned int channels);
/* Creates a new song iterator with the purpose of sending notes-off channel commands
** Parameters: (unsigned int) channels: Channel mask to send these commands for
** Returns   : A song iterator with the aforementioned purpose
*/

int is_cleanup_iterator(song_iterator_t *it);
/* Determines whether a given song iterator is a cleanup song iterator
** Parameters: (song_iterator_t *) it: The iterator to check
** Returns   : (int) 1 iff 'it' is a cleanup song iterator
** No deep recursion/delegation is considered.
*/


/**********************************/
/*--------- Fast Forward ---------*/
/**********************************/

struct fast_forward_song_iterator_t {
	INHERITS_SONG_ITERATOR;
	song_iterator_t *delegate;
	int delta; /* Remaining time */
};


song_iterator_t *new_fast_forward_iterator(song_iterator_t *it, int delta);
/* Creates a new song iterator which fast-forwards
** Parameters: (song_iterator_t *) it: The iterator to wrap
**             (int) delta: The number of ticks to skip
** Returns   : (song_iterator_t) A newly created song iterator
**                               which skips all delta times
**                               until 'delta' has been used up
*/

/**********************************/
/*--------- Fast Forward ---------*/
/**********************************/

#define MAX_BUF_SIZE 4

#define TEE_LEFT 0
#define TEE_RIGHT 1
#define TEE_LEFT_ACTIVE  (1<<0)
#define TEE_RIGHT_ACTIVE (1<<1)
#define TEE_LEFT_READY  (1<<2) /* left result is ready */
#define TEE_RIGHT_READY (1<<3) /* right result is ready */
#define TEE_LEFT_PCM (1<<4)
#define TEE_RIGHT_PCM (1<<5)

#define TEE_MORPH_NONE 0 /* Not waiting to self-morph */
#define TEE_MORPH_READY 1 /* Ready to self-morph */

struct tee_song_iterator_t {
	INHERITS_SONG_ITERATOR;

	int status;

	int may_destroy; /* May destroy song iterators */

	int morph_deferred; /* One of TEE_MORPH_* above */

	struct {
		song_iterator_t *it;
		byte buf[MAX_BUF_SIZE];
		int result;
		int retval;

		byte channel_remap[MIDI_CHANNELS];
		/* Remapping for channels */

	} children[2];
};


sfx_pcm_feed_t *sfx_iterator_make_feed(byte *base_data, int offset,
	int size, sfx_pcm_config_t conf);
/* Generates a feed for a song iterator
** Parameters: (byte *) base_data: A refcounted memory chunk containing
**                                 (among other things) PCM data
**             (int) offset; Offset into base_data
**             (int) size: Number of bytes to consider
**             (pcm_data_internal_t) conf: PCM encoding
*/

} // End of namespace Sci

#endif // SCI_SFX_SFX_ITERATOR_INTERNAL
