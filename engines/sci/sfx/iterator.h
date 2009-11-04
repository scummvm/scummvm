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
#include "sci/sfx/sci_midi.h"

namespace Audio {
	class AudioStream;
}

namespace Sci {

enum SongIteratorStatus {
	SI_FINISHED = -1,		/**< Song finished playing */
	SI_LOOP = -2,			/**< Song just looped */
	SI_ABSOLUTE_CUE = -3,	/**< Found a song cue (absolute) */
	SI_RELATIVE_CUE = -4,	/**< Found a song cue (relative) */
	SI_PCM = -5,			/**< Found a PCM */
	SI_IGNORE = -6,			/**< This event got edited out by the remapper */
	SI_MORPH = -255			/**< Song iterator requested self-morph. */
};

#define FADE_ACTION_NONE              0
#define FADE_ACTION_FADE_AND_STOP     1
#define FADE_ACTION_FADE_AND_CONT     2

struct fade_params_t {
	int ticks_per_step;
	int final_volume;
	int step_size;
	int action;
};

/* Helper defs for messages */
enum {
	_SIMSG_BASE, /* Any base decoder */
	_SIMSG_PLASTICWRAP  /* Any "Plastic" (discardable) wrapper decoder */
};

/* Base messages */
enum {
	_SIMSG_BASEMSG_SET_LOOPS, /* Set loops */
	_SIMSG_BASEMSG_SET_PLAYMASK, /* Set the current playmask for filtering */
	_SIMSG_BASEMSG_SET_RHYTHM, /* Activate/deactivate rhythm channel */
	_SIMSG_BASEMSG_ACK_MORPH, /* Acknowledge self-morph */
	_SIMSG_BASEMSG_STOP, /* Stop iterator */
	_SIMSG_BASEMSG_PRINT, /* Print self to stderr, after printing param1 tabs */
	_SIMSG_BASEMSG_SET_HOLD, /* Set value of hold parameter to expect */
	_SIMSG_BASEMSG_SET_FADE /* Set fade parameters */
};

/* "Plastic" (discardable) wrapper messages */
enum {
	_SIMSG_PLASTICWRAP_ACK_MORPH = _SIMSG_BASEMSG_ACK_MORPH /* Acknowledge self-morph */
};

/* Messages */
#define SIMSG_SET_LOOPS(x) _SIMSG_BASE,_SIMSG_BASEMSG_SET_LOOPS,(x)
#define SIMSG_SET_PLAYMASK(x) _SIMSG_BASE,_SIMSG_BASEMSG_SET_PLAYMASK,(x)
#define SIMSG_SET_RHYTHM(x) _SIMSG_BASE,_SIMSG_BASEMSG_SET_RHYTHM,(x)
#define SIMSG_ACK_MORPH _SIMSG_PLASTICWRAP,_SIMSG_PLASTICWRAP_ACK_MORPH,0
#define SIMSG_STOP _SIMSG_BASE,_SIMSG_BASEMSG_STOP,0
#define SIMSG_PRINT(indentation) _SIMSG_BASE,_SIMSG_BASEMSG_PRINT,(indentation)
#define SIMSG_SET_HOLD(x) _SIMSG_BASE,_SIMSG_BASEMSG_SET_HOLD,(x)

/* Message transmission macro: Takes song reference, message reference */
#define SIMSG_SEND(o, m) songit_handle_message(&(o), SongIterator::Message((o)->ID, m))
#define SIMSG_SEND_FADE(o, m) songit_handle_message(&(o), SongIterator::Message((o)->ID, _SIMSG_BASE, _SIMSG_BASEMSG_SET_FADE, m))

typedef unsigned long songit_id_t;


#define SONGIT_MAX_LISTENERS 2

class TeeSongIterator;

class SongIterator {
public:
	struct Message {
		songit_id_t ID;
		uint _class; /* Type of iterator supposed to receive this */
		uint _type;
		union {
			uint i;
			void *p;
		} _arg;

		Message() : ID(0), _class(0xFFFF), _type(0xFFFF) {}

		/**
		 * Create a song iterator message.
		 *
		 * @param id: song ID the message is targeted to
		 * @param recipient_class: Message recipient class
		 * @param type	message type
		 * @param a		argument
		 *
		 * @note You should only use this with the SIMSG_* macros
		 */
		Message(songit_id_t id, int recipient_class, int type, int a)
			: ID(id), _class(recipient_class), _type(type) {
			_arg.i = a;
		}

		/**
		 * Create a song iterator message, wherein the first parameter is a pointer.
		 *
		 * @param id: song ID the message is targeted to
		 * @param recipient_class: Message recipient class
		 * @param type	message type
		 * @param a		argument
		 *
		 * @note You should only use this with the SIMSG_* macros
		 */
		Message(songit_id_t id, int recipient_class, int type, void *a)
			: ID(id), _class(recipient_class), _type(type) {
			_arg.p = a;
		}
	};

public:
	songit_id_t ID;
	uint16 channel_mask; /* Bitmask of all channels this iterator will use */
	fade_params_t fade;
	int priority;

	/* Death listeners */
	/* These are not reset during initialisation */
	TeeSongIterator *_deathListeners[SONGIT_MAX_LISTENERS];

	/* See songit_* for the constructor and non-virtual member functions */

	byte channel_remap[MIDI_CHANNELS]; ///< Remapping for channels

public:
	SongIterator();
	SongIterator(const SongIterator &);
	virtual ~SongIterator();

	/**
	 * Resets/initializes the sound iterator.
	 */
	virtual void init() {}

	/**
	 * Reads the next MIDI operation _or_ delta time.
	 * @param buf		The buffer to write to (needs to be able to store at least 4 bytes)
	 * @param result	Number of bytes written to the buffer
	 *                   (equals the number of bytes that need to be passed
	 *                   to the lower layers) for 0, the cue value for SI_CUE,
	 *                   or the number of loops remaining for SI_LOOP.
	 * @return zero if a MIDI operation was written, SI_FINISHED
	 *                   if the song has finished playing, SI_LOOP if looping
	 *                   (after updating the loop variable), SI_CUE if we found
	 *                   a cue, SI_PCM if a PCM was found, or the number of ticks
	 *                   to wait before this function should be called next.
	 *
	 * @note	If SI_PCM is returned, get_pcm() may be used to retrieve the associated
	 * PCM, but this must be done before any subsequent calls to next().
	 *
	 * @todo	The actual buffer size should either be specified or passed in, so that
	 *			we can detect buffer overruns.
	 */
	virtual int nextCommand(byte *buf, int *result) = 0;

	/**
	 Checks for the presence of a pcm sample.
	 * @return NULL if no PCM data was found, an AudioStream otherwise.
	 */
	virtual Audio::AudioStream *getAudioStream() = 0;

	/**
	 * Handles a message to the song iterator.
	 * @param msg	the message to handle
	 * @return NULL if the message was not understood,
	 *             this if the message could be handled, or a new song iterator
	 *             if the current iterator had to be morphed (but the message could
	 *             still be handled)
	 *
	 * @note This function is not supposed to be called directly; use
	 * songit_handle_message() instead. It should not recurse, since songit_handle_message()
	 * takes care of that and makes sure that its delegate received the message (and
	 * was morphed) before self.
	 */
	virtual SongIterator *handleMessage(Message msg) = 0;

	/**
	 * Gets the song position to store in a savegame.
	 */
	virtual int getTimepos() = 0;

	/**
	 * Clone this song iterator.
	 * @param delta		number of ticks that still need to elapse until the
	 * 					next item should be read from the song iterator
	 */
	virtual SongIterator *clone(int delta) = 0;


private:
	// Make the assignment operator unreachable, just in case...
	SongIterator& operator=(const SongIterator&);
};


/********************************/
/*-- Song iterator operations --*/
/********************************/

enum SongIteratorType {
	SCI_SONG_ITERATOR_TYPE_SCI0 = 0,
	SCI_SONG_ITERATOR_TYPE_SCI1 = 1
};

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
int songit_next(SongIterator **it, byte *buf, int *result, int mask);

/* Constructs a new song iterator object
** Parameters: (byte *) data: The song data to iterate over
**             (uint) size: Number of bytes in the song
**             (int) type: One of the SCI_SONG_ITERATOR_TYPEs
**             (songit_id_t) id: An ID for addressing the song iterator
** Returns   : (SongIterator *) A newly allocated but uninitialized song
**             iterator, or NULL if 'type' was invalid or unsupported
*/
SongIterator *songit_new(byte *data, uint size, SongIteratorType type, songit_id_t id);

/* Constructs a new song timer iterator object
** Parameters: (int) delta: The delta after which to fire SI_FINISHED
** Returns   : (SongIterator *) A newly allocated but uninitialized song
**             iterator
*/
SongIterator *new_timer_iterator(int delta);

/* Handles a message to the song iterator
** Parameters: (SongIterator **): A reference to the variable storing the song iterator
** Returns   : (int) Non-zero if the message was understood
** The song iterator may polymorph as result of msg, so a writeable reference is required.
*/
int songit_handle_message(SongIterator **it_reg, SongIterator::Message msg);


/* Creates a new song iterator which fast-forwards
** Parameters: (SongIterator *) it: The iterator to wrap
**             (int) delta: The number of ticks to skip
** Returns   : (SongIterator) A newly created song iterator
**                               which skips all delta times
**                               until 'delta' has been used up
*/
SongIterator *new_fast_forward_iterator(SongIterator *it, int delta);



} // End of namespace Sci

#endif // SCI_SFX_SFX_ITERATOR_H
