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

#include "sci/sfx/iterator.h"
#include "sci/sfx/sci_midi.h"

#include "common/array.h"
#include "common/list.h"

namespace Sci {

/* Iterator types */

enum {
	SI_STATE_UNINITIALISED		= -1,
	SI_STATE_DELTA_TIME			= 0,	///< Now at a delta time
	SI_STATE_COMMAND			= 1,	///< Now at a MIDI operation
	SI_STATE_PENDING			= 2,	///< Pending for loop
	SI_STATE_FINISHED			= 3,	///< End of song
	SI_STATE_PCM				= 4,	///< Should report a PCM next (-> DELTA_TIME)
	SI_STATE_PCM_MAGIC_DELTA	= 5		///< Should report a ``magic'' one tick delta time next (goes on to FINISHED)
};

struct SongIteratorChannel {

	int state;	///< State of this song iterator channel
	int offset;	///< Offset into the data chunk */
	int end;	///< Last allowed byte in track */
	int id;		///< Some channel ID */

	/**
	 * Number of ticks before the specified channel is next used, or
	 * CHANNEL_DELAY_MISSING to indicate that the delay has not yet
	 * been read.
	 */
	int delay;

	/* Two additional offsets for recovering: */
	int loop_offset;
	int initial_offset;

	int playmask;			///< Active playmask (MIDI channels to play in here) */
	int notes_played;		///< #of notes played since the last loop start */
	int loop_timepos;		///< Total delay for this channel's loop marker */
	int total_timepos;		///< Number of ticks since the beginning, ignoring loops */
	int timepos_increment;	///< Number of ticks until the next command (to add) */

	int saw_notes;			///< Bitmask of channels we have currently played notes on */
	byte last_cmd;			///< Last operation executed, for running status */

public:
	void init(int id, int offset, int end);
	void resetSynthChannels();
};

class BaseSongIterator : public SongIterator {
public:
	int _polyphony[MIDI_CHANNELS];	///< # of simultaneous notes on each
	int _importance[MIDI_CHANNELS];	///< priority rating for each channel, 0 means unrated.


	int _ccc;					///< Cumulative cue counter, for those who need it
	byte _resetflag;			///< for 0x4C -- on DoSound StopSound, do we return to start?
	int _deviceId;				///< ID of the device we generating events for
	int _numActiveChannels;		///< Number of active channels
	Common::Array<byte> _data;	///< Song data

	int _loops; ///< Number of loops remaining

public:
	BaseSongIterator(byte *data, uint size, songit_id_t id);

protected:
	int parseMidiCommand(byte *buf, int *result, SongIteratorChannel *channel, int flags);
	int processMidi(byte *buf, int *result, SongIteratorChannel *channel, int flags);
};

/********************************/
/*--------- SCI 0 --------------*/
/********************************/

class Sci0SongIterator : public BaseSongIterator {
public:
	SongIteratorChannel _channel;

public:
	Sci0SongIterator(byte *data, uint size, songit_id_t id);

	int nextCommand(byte *buf, int *result);
	Audio::AudioStream *getAudioStream();
	SongIterator *handleMessage(Message msg);
	void init();
	int getTimepos();
	SongIterator *clone(int delta);
};


/********************************/
/*--------- SCI 1 --------------*/
/********************************/


struct Sci1Sample {
	/**
	 * Time left-- initially, this is 'Sample point 1'.
	 * After initialisation, it is 'sample point 1 minus the sample
	 * point of the previous sample'
	 */
	int delta;
	int size;
	bool announced; /* Announced for download (SI_PCM) */
	sfx_pcm_config_t format;
	byte *_data;
};

class Sci1SongIterator : public BaseSongIterator {
public:
	SongIteratorChannel _channels[MIDI_CHANNELS];

	/* Invariant: Whenever channels[i].delay == CHANNEL_DELAY_MISSING,
	** channel_offset[i] points to a delta time object. */

	bool _initialised; /**!< Whether the MIDI channel setup has been initialised */
	int _numChannels; /**!< Number of channels actually used */
	Common::List<Sci1Sample> _samples;
	int _numLoopedChannels; /**!< Number of channels that are ready to loop */

	int _delayRemaining; /**!< Number of ticks that haven't been polled yet */
	int _hold;

public:
	Sci1SongIterator(byte *data, uint size, songit_id_t id);
	~Sci1SongIterator();

	int nextCommand(byte *buf, int *result);
	Audio::AudioStream *getAudioStream();
	SongIterator *handleMessage(Message msg);
	void init();
	int getTimepos();
	SongIterator *clone(int delta);

private:
	int initSample(const int offset);
	int initSong();

	int getSmallestDelta() const;

	void updateDelta(int delta);

	/** Checks that none of the channels is waiting for its delta to be read */
	bool noDeltaTime() const;

	/** Determine the channel # of the next active event, or -1 */
	int getCommandIndex() const;
};

#define PLAYMASK_NONE 0x0

/***************************/
/*--------- Timer ---------*/
/***************************/

/**
 * A song iterator which waits a specified time and then fires
 * SI_FINISHED. Used by DoSound, where audio resources are played (SCI1)
 */
class TimerSongIterator : public SongIterator {
protected:
	int _delta; /**!< Remaining time */

public:
	TimerSongIterator(int delta);

	int nextCommand(byte *buf, int *result);
	Audio::AudioStream *getAudioStream();
	SongIterator *handleMessage(Message msg);
	int getTimepos();
	SongIterator *clone(int delta);
};

/**********************************/
/*--------- Fast Forward ---------*/
/**********************************/

/**
 * A song iterator which fast-forwards another iterator.
 * Skips all delta times until a specified 'delta' has been used up.
 */
class FastForwardSongIterator : public SongIterator {
protected:
	SongIterator *_delegate;
	int _delta; /**!< Remaining time */

public:
	FastForwardSongIterator(SongIterator *capsit, int delta);

	int nextCommand(byte *buf, int *result);
	Audio::AudioStream *getAudioStream();
	SongIterator *handleMessage(Message msg);
	int getTimepos();
	SongIterator *clone(int delta);
};


/**********************************/
/*--------- Tee iterator ---------*/
/**********************************/

enum {
	TEE_LEFT = 0,
	TEE_RIGHT = 1,
	TEE_LEFT_ACTIVE  = (1<<0),
	TEE_RIGHT_ACTIVE = (1<<1),
	TEE_LEFT_READY  = (1<<2), /**!< left result is ready */
	TEE_RIGHT_READY = (1<<3), /**!< right result is ready */
	TEE_LEFT_PCM = (1<<4),
	TEE_RIGHT_PCM = (1<<5)
};

/**
 * This iterator combines two iterators, returns the next event available from either.
 */
class TeeSongIterator : public SongIterator {
public:
	int _status;

	bool _readyToMorph; /**!< One of TEE_MORPH_* above */

	struct {
		SongIterator *it;
		byte buf[4];
		int result;
		int retval;

		/* Remapping for channels */
		byte channel_remap[MIDI_CHANNELS];

	} _children[2];

public:
	TeeSongIterator(SongIterator *left, SongIterator *right);
	~TeeSongIterator();

	int nextCommand(byte *buf, int *result);
	Audio::AudioStream *getAudioStream();
	SongIterator *handleMessage(Message msg);
	void init();
	int getTimepos() { return 0; }
	SongIterator *clone(int delta);
};

} // End of namespace Sci

#endif // SCI_SFX_SFX_ITERATOR_INTERNAL
