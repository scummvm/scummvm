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

/* Song iterator declarations */

#ifndef _SCI_SFX_ITERATOR_H_
#define _SCI_SFX_ITERATOR_H_

#include "sci/sfx/sfx_pcm.h"

namespace Sci {

#define SI_FINISHED -1 /* Song finished playing */
#define SI_LOOP -2 /* Song just looped */
#define SI_ABSOLUTE_CUE -3 /* Found a song cue (absolute) */
#define SI_RELATIVE_CUE -4 /* Found a song cue (relative) */
#define SI_PCM -5 /* Found a PCM */
#define SI_IGNORE -6 /* This event got edited out by the remapper */
#define SI_MORPH -255 /* Song iterator requested self-morph. */

#define FADE_ACTION_NONE              0
#define FADE_ACTION_FADE_AND_STOP     1
#define FADE_ACTION_FADE_AND_CONT     2

typedef struct {
	int ticks_per_step;
	int final_volume;
	int step_size;
	int action;
} fade_params_t;

#define SONG_ITERATOR_MESSAGE_ARGUMENTS_NR 2

/* Helper defs for messages */
/* Base messages */
#define _SIMSG_BASE 0 /* Any base decoder */
#define _SIMSG_BASEMSG_SET_LOOPS 0 /* Set loops */
#define _SIMSG_BASEMSG_CLONE 1 /* Clone object and data. Must provide the
** (possibly negative) number of ticks that have
** passed since the last delay time started being
** used  */
#define _SIMSG_BASEMSG_SET_PLAYMASK 2 /* Set the current playmask for filtering */
#define _SIMSG_BASEMSG_SET_RHYTHM 3 /* Activate/deactivate rhythm channel */
#define _SIMSG_BASEMSG_ACK_MORPH 4 /* Acknowledge self-morph */
#define _SIMSG_BASEMSG_STOP 5 /* Stop iterator */
#define _SIMSG_BASEMSG_PRINT 6 /* Print self to stderr, after printing param1 tabs */
#define _SIMSG_BASEMSG_SET_HOLD 7 /* Set value of hold parameter to expect */
#define _SIMSG_BASEMSG_SET_FADE 8 /* Set fade parameters */

/* "Plastic" (discardable) wrapper messages */
#define _SIMSG_PLASTICWRAP 1 /* Any base decoder */
#define _SIMSG_PLASTICWRAP_ACK_MORPH 4 /* Acknowledge self-morph */

/* Messages */
#define SIMSG_SET_LOOPS(x) _SIMSG_BASE,_SIMSG_BASEMSG_SET_LOOPS,(x),0
#define SIMSG_SET_PLAYMASK(x) _SIMSG_BASE,_SIMSG_BASEMSG_SET_PLAYMASK,(x),0
#define SIMSG_SET_RHYTHM(x) _SIMSG_BASE,_SIMSG_BASEMSG_SET_RHYTHM,(x),0
#define SIMSG_CLONE(x) _SIMSG_BASE,_SIMSG_BASEMSG_CLONE,(x),0
#define SIMSG_ACK_MORPH _SIMSG_PLASTICWRAP,_SIMSG_PLASTICWRAP_ACK_MORPH,0,0
#define SIMSG_STOP _SIMSG_BASE,_SIMSG_BASEMSG_STOP,0,0
#define SIMSG_PRINT(indentation) _SIMSG_BASE,_SIMSG_BASEMSG_PRINT,(indentation),0
#define SIMSG_SET_HOLD(x) _SIMSG_BASE,_SIMSG_BASEMSG_SET_HOLD,(x),0
/*#define SIMSG_SET_FADE(x) _SIMSG_BASE,_SIMSG_BASEMSG_SET_FADE,(x),0*/

/* Message transmission macro: Takes song reference, message reference */
#define SIMSG_SEND(o, m) songit_handle_message(&(o), songit_make_message((o)->ID, m))
#define SIMSG_SEND_FADE(o, m) songit_handle_message(&(o), songit_make_ptr_message((o)->ID, _SIMSG_BASE, _SIMSG_BASEMSG_SET_FADE, m, 0))

/* Event listener interface */
struct listener_t {
	void (*notify)(void *self, void *notifier);
	void *self;
};

typedef unsigned long songit_id_t;

typedef struct {
	songit_id_t ID;
	unsigned int recipient; /* Type of iterator supposed to receive this */
	unsigned int type;
	union {
		unsigned int i;
		void * p;
	} args[SONG_ITERATOR_MESSAGE_ARGUMENTS_NR];
} song_iterator_message_t;

#define INHERITS_SONG_ITERATOR \
	songit_id_t ID;										  \
	uint16 channel_mask;									  \
        fade_params_t fade;                                                                       \
	unsigned int flags;									  \
	int priority;                                                                             \
	int (*next) (song_iterator_t *self, unsigned char *buf, int *buf_size);			  \
	sfx_pcm_feed_t * (*get_pcm_feed) (song_iterator_t *s);					  \
	song_iterator_t * (* handle_message)(song_iterator_t *self, song_iterator_message_t msg); \
	void (*init) (struct _song_iterator *self);						  \
	void (*cleanup) (struct _song_iterator *self);						  \
        int (*get_timepos) (struct _song_iterator *self);                                         \
	listener_t death_listeners[SONGIT_MAX_LISTENERS];					  \
	int death_listeners_nr									  \
 
#define SONGIT_MAX_LISTENERS 2

typedef struct _song_iterator {

	songit_id_t ID;
	uint16 channel_mask; /* Bitmask of all channels this iterator will use */
	fade_params_t fade;
	unsigned int flags;
	int priority;

	int (*next)(struct _song_iterator *self,
	            unsigned char *buf, int *result);
	/* Reads the next MIDI operation _or_ delta time
	** Parameters: (song_iterator_t *) self
	**             (byte *) buf: The buffer to write to (needs to be able to
	**                           store at least 4 bytes)
	** Returns   : (int) zero if a MIDI operation was written, SI_FINISHED
	**                   if the song has finished playing, SI_LOOP if looping
	**                   (after updating the loop variable), SI_CUE if we found
	**                   a cue, SI_PCM if a PCM was found, or the number of ticks
	**                   to wait before this function should be called next.
	**             (int) *result: Number of bytes written to the buffer
	**                   (equals the number of bytes that need to be passed
	**                   to the lower layers) for 0, the cue value for SI_CUE,
	**                   or the number of loops remaining for SI_LOOP.
	**   If SI_PCM is returned, get_pcm() may be used to retrieve the associated
	** PCM, but this must be done before any subsequent calls to next().
	*/

	sfx_pcm_feed_t * (*get_pcm_feed)(struct _song_iterator *self);
	/* Checks for the presence of a pcm sample
	** Parameters: (song_iterator_t *) self
	** Returns   : (sfx_pcm_feed_t *) NULL if no PCM data was found, a
	**				  PCM feed otherwise
	*/


	struct _song_iterator *
				(* handle_message)(struct _song_iterator *self, song_iterator_message_t msg);
	/* Handles a message to the song iterator
	** Parameters: (song_iterator_t *) self
	**             (song_iterator_messag_t) msg: The message to handle
	** Returns   : (song_iterator_t *) NULL if the message was not understood,
	**             self if the message could be handled, or a new song iterator
	**             if the current iterator had to be morphed (but the message could
	**             still be handled)
	** This function is not supposed to be called directly; use
	** songit_handle_message() instead. It should not recurse, since songit_handle_message()
	** takes care of that and makes sure that its delegate received the message (and
	** was morphed) before self.
	*/


	void (*init)(struct _song_iterator *self);
	/* Resets/initializes the sound iterator
	** Parameters: (song_iterator_t *) self
	** Returns   : (void)
	*/

	void (*cleanup)(struct _song_iterator *self);
	/* Frees any content of the iterator structure
	** Parameters: (song_iterator_t *) self
	** Does not physically free(self) yet. May be NULL if nothing needs to be done.
	** Must not recurse on its delegate.
	*/

	int (*get_timepos)(struct _song_iterator *self);
	/* Gets the song position to store in a savegame
	** Parameters: (song_iterator_t *) self
	*/

	/* Death listeners */
	/* These are not reset during initialisation */
	listener_t death_listeners[SONGIT_MAX_LISTENERS];
	int death_listeners_nr;

	/* See songit_* for the constructor and non-virtual member functions */

} song_iterator_t;


/* Song iterator flags */
#define SONGIT_FLAG_CLONE	(1 << 0)	/* This flag is set for clones, which are exclusively used in song players.
** Thus, this flag distinguishes song iterators in the main thread from those
** in the song-player thread. */

void song_iterator_add_death_listener(song_iterator_t *it,
                                 void *client,
                                 void (*notify)(void *self, void *notifier));
/* Adds a death listener to a song iterator
** Parameters: (song_iterator_t *) it: The iterator to add to
**             (void *) client: The object wanting to be notified
**             (void* x void* -> void) notify: The notification function
**                                     to invoke
** Effects:    Fatally terminates the program if no listener slots are
**	       available
** Death listeners are NOT cloned.
*/

void song_iterator_remove_death_listener(song_iterator_t *it,
                                    void *client);
/* Removes a death listener from a song iterator
** Parameters: (song_iterator_t *) it: The iterator to modify
**             (void *) client: The object no longer wanting to be notified
** Effects:    Fatally terminates the program if the listener was not
**	       found
** Death listeners are NOT cloned.
*/

/********************************/
/*-- Song iterator operations --*/
/********************************/

#define SCI_SONG_ITERATOR_TYPE_SCI0 0
#define SCI_SONG_ITERATOR_TYPE_SCI1 1

#define IT_READER_MASK_MIDI	(1 << 0)
#define IT_READER_MASK_DELAY	(1 << 1)
#define IT_READER_MASK_LOOP	(1 << 2)
#define IT_READER_MASK_CUE	(1 << 3)
#define IT_READER_MASK_PCM	(1 << 4)
#define IT_READER_MAY_FREE	(1 << 10) /* Free SI_FINISHED iterators */
#define IT_READER_MAY_CLEAN	(1 << 11)
/* MAY_CLEAN: May instantiate cleanup iterators
** (use for players; this closes open channels at the end of a song) */

#define IT_READER_MASK_ALL (  IT_READER_MASK_MIDI	\
			    | IT_READER_MASK_DELAY	\
			    | IT_READER_MASK_LOOP	\
			    | IT_READER_MASK_CUE	\
			    | IT_READER_MASK_PCM )

int songit_next(song_iterator_t **it, unsigned char *buf, int *result, int mask);
/* Convenience wrapper around it->next
** Parameters: (song_iterator_t **it) Reference to the iterator to access
**             (byte *) buf: The buffer to write to (needs to be able to
**                           store at least 4 bytes)
**             (int) mask: IT_READER_MASK options specifying the events to
**                   listen for
** Returns   : (int) zero if a MIDI operation was written, SI_FINISHED
**                   if the song has finished playing, SI_LOOP if looping
**                   (after updating the loop variable), SI_CUE if we found
**                   a cue, SI_PCM if a PCM was found, or the number of ticks
**                   to wait before this function should be called next.
**             (int) *result: Number of bytes written to the buffer
**                   (equals the number of bytes that need to be passed
**                   to the lower layers) for 0, the cue value for SI_CUE,
**                   or the number of loops remaining for SI_LOOP.
*/

song_iterator_t *songit_new(unsigned char *data, unsigned int size, int type, songit_id_t id);
/* Constructs a new song iterator object
** Parameters: (byte *) data: The song data to iterate over
**             (unsigned int) size: Number of bytes in the song
**             (int) type: One of the SCI_SONG_ITERATOR_TYPEs
**             (songit_id_t) id: An ID for addressing the song iterator
** Returns   : (song_iterator_t *) A newly allocated but uninitialized song
**             iterator, or NULL if 'type' was invalid or unsupported
*/

song_iterator_t *songit_new_tee(song_iterator_t *left, song_iterator_t *right, int may_destroy);
/* Combines two iterators, returns the next event available from either
** Parameters: (song_iterator_t *) left: One of the iterators
**             (song_iterator_t *) right: The other iterator
**             (int) may_destroy: Whether completed song iterators may be
**                                destroyed
** Returns   : (song_iterator_t *) A combined iterator, as suggested above
*/


void songit_free(song_iterator_t *it);
/* Frees a song iterator and the song it wraps
** Parameters: (song_iterator_t *) it: The song iterator to free
** Returns   : (void)
*/

song_iterator_message_t songit_make_message(songit_id_t id,
                    int recipient_class, int type, int a1, int a2);
/* Create a song iterator message
** Parameters: (songit_id_t) id: song ID the message is targetted to
**             (int) recipient_class: Message recipient class
**             (int) type: Message type
**             (int x int) a1, a2: Arguments
** You should only use this with the SIMSG_* macros
*/

song_iterator_message_t songit_make_ptr_message(songit_id_t id,
                        int recipient_class, int type, void * a1, int a2);
/* Create a song iterator message, wherein the first parameter is a pointer
** Parameters: (songit_id_t) id: song ID the message is targetted to
**             (int) recipient_class: Message recipient class
**             (int) type: Message type
**             (void* x int) a1, a2: Arguments
** You should only use this with the SIMSG_* macros
*/

int songit_handle_message(song_iterator_t **it_reg, song_iterator_message_t msg);
/* Handles a message to the song iterator
** Parameters: (song_iterator_t **): A reference to the variable storing the song iterator
** Returns   : (int) Non-zero if the message was understood
** The song iterator may polymorph as result of msg, so a writeable reference is required.
*/


song_iterator_t *songit_clone(song_iterator_t *it, int delta);
/* Clones a song iterator
** Parameters: (song_iterator_t *) it: The iterator to clone
**             (int) delta: Number of ticks that still need to elapse until
**                          the next item should be read from the song iterator
** Returns   : (song_iterator_t *) A shallow clone of 'it'.
** This performs a clone on the bottom-most part (containing the actual song data) _only_.
** The justification for requiring 'delta' to be passed in here is that this
** is typically maintained outside of the song iterator.
*/


int sfx_play_iterator_pcm(song_iterator_t *it, unsigned long handle);
/* Plays a song iterator that found a PCM through a PCM device, if possible
** Parameters: (song_iterator_t *) it: The iterator to play
**             (song_handle_t) handle: Debug handle
** Returns   : (int) 0 if the effect will not be played, nonzero if it will
** This assumes that the last call to 'it->next()' returned SI_PCM.
*/

} // End of namespace Sci

#endif
