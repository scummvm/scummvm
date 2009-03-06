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

#ifndef SCI_SFX_SFX_ITERATOR_H
#define SCI_SFX_SFX_ITERATOR_H

#include "sci/sfx/sfx_pcm.h"

namespace Audio {
	class AudioStream;
}

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

struct fade_params_t {
	int ticks_per_step;
	int final_volume;
	int step_size;
	int action;
};

#define SONG_ITERATOR_MESSAGE_ARGUMENTS_NR 2

/* Helper defs for messages */
/* Base messages */
enum {
	_SIMSG_BASE = 0, /* Any base decoder */
	_SIMSG_BASEMSG_SET_LOOPS = 0, /* Set loops */

	/**
	 * Clone object and data. Must provide the (possibly negative)
	 * number of ticks that have passed since the last delay time
	 * started being used.
	 */
	_SIMSG_BASEMSG_CLONE = 1,

	_SIMSG_BASEMSG_SET_PLAYMASK = 2, /* Set the current playmask for filtering */
	_SIMSG_BASEMSG_SET_RHYTHM = 3, /* Activate/deactivate rhythm channel */
	_SIMSG_BASEMSG_ACK_MORPH = 4, /* Acknowledge self-morph */
	_SIMSG_BASEMSG_STOP = 5, /* Stop iterator */
	_SIMSG_BASEMSG_PRINT = 6, /* Print self to stderr, after printing param1 tabs */
	_SIMSG_BASEMSG_SET_HOLD = 7, /* Set value of hold parameter to expect */
	_SIMSG_BASEMSG_SET_FADE = 8 /* Set fade parameters */
};

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
#define SIMSG_SEND(o, m) songit_handle_message(&(o), SongIteratorMessage((o)->ID, m))
#define SIMSG_SEND_FADE(o, m) songit_handle_message(&(o), SongIteratorMessage((o)->ID, _SIMSG_BASE, _SIMSG_BASEMSG_SET_FADE, m, 0))

typedef unsigned long songit_id_t;

struct SongIteratorMessage {
	songit_id_t ID;
	uint recipient; /* Type of iterator supposed to receive this */
	uint type;
	union {
		uint i;
		void *p;
	} args[SONG_ITERATOR_MESSAGE_ARGUMENTS_NR];


	SongIteratorMessage();

	/**
	 * Create a song iterator message.
	 *
	 * @param id: song ID the message is targeted to
	 * @param recipient_class: Message recipient class
	 * @param type	message type
	 * @param a1	first message argument
	 * @param a2	second message argument
	 *
	 * @note You should only use this with the SIMSG_* macros
	 */
	SongIteratorMessage(songit_id_t id, int recipient_class, int type, int a1, int a2);
	
	/**
	 * Create a song iterator message, wherein the first parameter is a pointer.
	 *
	 * @param id: song ID the message is targeted to
	 * @param recipient_class: Message recipient class
	 * @param type	message type
	 * @param a1	first message argument
	 * @param a2	second message argument
	 *
	 * @note You should only use this with the SIMSG_* macros
	 */
	SongIteratorMessage(songit_id_t id, int recipient_class, int type, void *a1, int a2);
};

#define SONGIT_MAX_LISTENERS 2

class TeeSongIterator;

class SongIterator {
public:
	songit_id_t ID;
	uint16 channel_mask; /* Bitmask of all channels this iterator will use */
	fade_params_t fade;
	uint flags;
	int priority;

	/* Death listeners */
	/* These are not reset during initialisation */
	TeeSongIterator *_deathListeners[SONGIT_MAX_LISTENERS];

	/* See songit_* for the constructor and non-virtual member functions */


public:
	SongIterator();
	virtual ~SongIterator();

	/**
	 * Reads the next MIDI operation _or_ delta time.
	 * Parameters: (SongIterator *) self
	 *             (byte *) buf: The buffer to write to (needs to be able to
	 *                           store at least 4 bytes)
	 * Returns   : (int) zero if a MIDI operation was written, SI_FINISHED
	 *                   if the song has finished playing, SI_LOOP if looping
	 *                   (after updating the loop variable), SI_CUE if we found
	 *                   a cue, SI_PCM if a PCM was found, or the number of ticks
	 *                   to wait before this function should be called next.
	 *             (int) *result: Number of bytes written to the buffer
	 *                   (equals the number of bytes that need to be passed
	 *                   to the lower layers) for 0, the cue value for SI_CUE,
	 *                   or the number of loops remaining for SI_LOOP.
	 *   If SI_PCM is returned, get_pcm() may be used to retrieve the associated
	 * PCM, but this must be done before any subsequent calls to next().
	 */
	virtual int nextCommand(byte *buf, int *result) = 0;

	/**
	 Checks for the presence of a pcm sample.
	 * @return NULL if no PCM data was found, an AudioStream otherwise.
	 */
	virtual Audio::AudioStream *getAudioStream() = 0;

	/**
	 * Handles a message to the song iterator.
	 * Parameters: (SongIterator *) self
	 *             (song_iterator_messag_t) msg: The message to handle
	 * Returns   : (SongIterator *) NULL if the message was not understood,
	 *             self if the message could be handled, or a new song iterator
	 *             if the current iterator had to be morphed (but the message could
	 *             still be handled)
	 * This function is not supposed to be called directly; use
	 * songit_handle_message() instead. It should not recurse, since songit_handle_message()
	 * takes care of that and makes sure that its delegate received the message (and
	 * was morphed) before self.
	 */
	virtual SongIterator *handleMessage(SongIteratorMessage msg) = 0;

	/**
	 * Resets/initializes the sound iterator.
	 */
	virtual void init() {}

	/**
	 * Frees any content of the iterator structure.
	 * Does not physically free(self) yet. May be NULL if nothing needs to be done.
	 * Must not recurse on its delegate.
	 */
	virtual void cleanup() {}

	/**
	 * Gets the song position to store in a savegame.
	 */
	virtual int getTimepos() = 0;

};


/* Song iterator flags */
#define SONGIT_FLAG_CLONE	(1 << 0)	/* This flag is set for clones, which are exclusively used in song players.
** Thus, this flag distinguishes song iterators in the main thread from those
** in the song-player thread. */

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

int songit_next(SongIterator **it, unsigned char *buf, int *result, int mask);
/* Convenience wrapper around it->next
** Parameters: (SongIterator **it) Reference to the iterator to access
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

SongIterator *songit_new(unsigned char *data, uint size, int type, songit_id_t id);
/* Constructs a new song iterator object
** Parameters: (byte *) data: The song data to iterate over
**             (uint) size: Number of bytes in the song
**             (int) type: One of the SCI_SONG_ITERATOR_TYPEs
**             (songit_id_t) id: An ID for addressing the song iterator
** Returns   : (SongIterator *) A newly allocated but uninitialized song
**             iterator, or NULL if 'type' was invalid or unsupported
*/

SongIterator *songit_new_tee(SongIterator *left, SongIterator *right);
/* Combines two iterators, returns the next event available from either
** Parameters: (SongIterator *) left: One of the iterators
**             (SongIterator *) right: The other iterator
** Returns   : (SongIterator *) A combined iterator, as suggested above
*/


void songit_free(SongIterator *it);
/* Frees a song iterator and the song it wraps
** Parameters: (SongIterator *) it: The song iterator to free
** Returns   : (void)
*/


int songit_handle_message(SongIterator **it_reg, SongIteratorMessage msg);
/* Handles a message to the song iterator
** Parameters: (SongIterator **): A reference to the variable storing the song iterator
** Returns   : (int) Non-zero if the message was understood
** The song iterator may polymorph as result of msg, so a writeable reference is required.
*/


SongIterator *songit_clone(SongIterator *it, int delta);
/* Clones a song iterator
** Parameters: (SongIterator *) it: The iterator to clone
**             (int) delta: Number of ticks that still need to elapse until
**                          the next item should be read from the song iterator
** Returns   : (SongIterator *) A shallow clone of 'it'.
** This performs a clone on the bottom-most part (containing the actual song data) _only_.
** The justification for requiring 'delta' to be passed in here is that this
** is typically maintained outside of the song iterator.
*/


int sfx_play_iterator_pcm(SongIterator *it, unsigned long handle);
/* Plays a song iterator that found a PCM through a PCM device, if possible
** Parameters: (SongIterator *) it: The iterator to play
**             (song_handle_t) handle: Debug handle
** Returns   : (int) 0 if the effect will not be played, nonzero if it will
** This assumes that the last call to 'it->next()' returned SI_PCM.
*/

} // End of namespace Sci

#endif // SCI_SFX_SFX_ITERATOR_H
