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

/* Sound subsystem core: Event handler, sound player dispatching */

#include "sci/tools.h"
#include "sci/sfx/core.h"
#include "sci/sfx/iterator.h"
#include "sci/sfx/misc.h"
#include "sci/sfx/sci_midi.h"

#include "sci/sfx/softseq/pcjr.h"
#include "sci/sfx/softseq/adlib.h"

#include "common/system.h"
#include "common/timer.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Sci {

class SfxPlayer;
SfxPlayer *player = NULL;		// FIXME: Avoid static vars


#pragma mark -


class SfxPlayer {
public:
	/** Number of voices that can play simultaneously */
	int _polyphony;

public:
	SfxPlayer() : _polyphony(0) {}
	virtual ~SfxPlayer() {}

	virtual Common::Error init(ResourceManager *resmgr, int expected_latency) = 0;
	/* Initializes the player
	** Parameters: (ResourceManager *) resmgr: A resource manager for driver initialization
	**             (int) expected_latency: Expected delay in between calls to 'maintenance'
	**                   (in microseconds)
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	*/

	virtual Common::Error add_iterator(SongIterator *it, uint32 start_time) = 0;
	/* Adds an iterator to the song player
	** Parameters: (songx_iterator_t *) it: The iterator to play
	**             (uint32) start_time: The time to assume as the
	**                        time the first MIDI command executes at
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	** The iterator should not be cloned (to avoid memory leaks) and
	** may be modified according to the needs of the player.
	** Implementors may use the 'sfx_iterator_combine()' function
	** to add iterators onto their already existing iterators
	*/

	virtual Common::Error stop() = 0;
	/* Stops the currently playing song and deletes the associated iterator
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	*/

	virtual Common::Error iterator_message(const SongIterator::Message &msg) = 0;
	/* Transmits a song iterator message to the active song
	** Parameters: (SongIterator::Message) msg: The message to transmit
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	** OPTIONAL -- may be NULL
	** If this method is not present, sending messages will stop
	** and re-start playing, so it is preferred that it is present
	*/

	virtual Common::Error pause() = 0;
	/* Pauses song playing
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	*/

	virtual Common::Error resume() = 0;
	/* Resumes song playing after a pause
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	*/

	virtual Common::Error exit() = 0;
	/* Stops the player
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	*/

	virtual void tell_synth(int buf_nr, byte *buf) = 0;
	/* Pass a raw MIDI event to the synth
	Parameters: (int) argc: Length of buffer holding the midi event
	           (byte *) argv: The buffer itself
	*/
};

#pragma mark -

class NewPlayer : public SfxPlayer {
protected:
	MidiPlayer *_mididrv;

	SongIterator *_iterator;
	Audio::Timestamp _wakeupTime;
	Audio::Timestamp _currentTime;
	uint32 _pauseTimeDiff;

	bool _paused;
	bool _iteratorIsDone;
	uint32 _tempo;

	Common::Mutex *_mutex;
	int _volume;

	void play_song(SongIterator *it);
	static void player_timer_callback(void *refCon);

public:
	NewPlayer();

	virtual Common::Error init(ResourceManager *resmgr, int expected_latency);
	virtual Common::Error add_iterator(SongIterator *it, uint32 start_time);
	virtual Common::Error stop();
	virtual Common::Error iterator_message(const SongIterator::Message &msg);
	virtual Common::Error pause();
	virtual Common::Error resume();
	virtual Common::Error exit();
	virtual void tell_synth(int buf_nr, byte *buf);
};

NewPlayer::NewPlayer() {
	_mididrv = 0;

	_iterator = NULL;
	_pauseTimeDiff = 0;

	_paused = false;
	_iteratorIsDone = false;
	_tempo = 0;

	_mutex = 0;
	_volume = 15;
}

void NewPlayer::play_song(SongIterator *it) {
	while (_iterator && _wakeupTime.msecsDiff(_currentTime) <= 0) {
		int delay;
		byte buf[8];
		int result;

		switch ((delay = songit_next(&(_iterator),
		                             buf, &result,
		                             IT_READER_MASK_ALL
		                             | IT_READER_MAY_FREE
		                             | IT_READER_MAY_CLEAN))) {

		case SI_FINISHED:
			delete _iterator;
			_iterator = NULL;
			_iteratorIsDone = true;
			return;

		case SI_IGNORE:
		case SI_LOOP:
		case SI_RELATIVE_CUE:
		case SI_ABSOLUTE_CUE:
			break;

		case SI_PCM:
			sfx_play_iterator_pcm(_iterator, 0);
			break;

		case 0:
			static_cast<MidiDriver *>(_mididrv)->send(buf[0], buf[1], buf[2]);

			break;

		default:
			_wakeupTime = _wakeupTime.addFrames(delay);
		}
	}
}

void NewPlayer::tell_synth(int buf_nr, byte *buf) {
	byte op1 = (buf_nr < 2 ? 0 : buf[1]);
	byte op2 = (buf_nr < 3 ? 0 : buf[2]);

	static_cast<MidiDriver *>(_mididrv)->send(buf[0], op1, op2);
}

void NewPlayer::player_timer_callback(void *refCon) {
	NewPlayer *thePlayer = (NewPlayer *)refCon;
	assert(refCon);
	Common::StackLock lock(*thePlayer->_mutex);

	if (thePlayer->_iterator && !thePlayer->_iteratorIsDone && !thePlayer->_paused) {
		thePlayer->play_song(thePlayer->_iterator);
	}

	thePlayer->_currentTime = thePlayer->_currentTime.addFrames(1);
}

/* API implementation */

Common::Error NewPlayer::init(ResourceManager *resmgr, int expected_latency) {
	MidiDriverType musicDriver = MidiDriver::detectMusicDriver(MDT_PCSPK | MDT_ADLIB);

	switch(musicDriver) {
	case MD_ADLIB:
		_mididrv = new MidiPlayer_Adlib();
		break;
	case MD_PCJR:
		_mididrv = new MidiPlayer_PCJr();
		break;
	case MD_PCSPK:
		_mididrv = new MidiPlayer_PCSpeaker();
		break;
	default:
		break;
	}

	assert(_mididrv);

	_polyphony = _mididrv->getPolyphony();

	_tempo = _mididrv->getBaseTempo();
    uint32 time = g_system->getMillis();
	_currentTime = Audio::Timestamp(time, 1000000 / _tempo);
	_wakeupTime = Audio::Timestamp(time, SFX_TICKS_PER_SEC);

	_mutex = new Common::Mutex();

	_mididrv->setTimerCallback(this, player_timer_callback);
	_mididrv->open(resmgr);
	_mididrv->setVolume(_volume);

	return Common::kNoError;
}

Common::Error NewPlayer::add_iterator(SongIterator *it, uint32 start_time) {
	Common::StackLock lock(*_mutex);
	SIMSG_SEND(it, SIMSG_SET_PLAYMASK(_mididrv->getPlayMask()));
	SIMSG_SEND(it, SIMSG_SET_RHYTHM(_mididrv->hasRhythmChannel()));

	if (_iterator == NULL) {
		// Resync with clock
		_currentTime = Audio::Timestamp(g_system->getMillis(), 1000000 / _tempo);
		_wakeupTime = Audio::Timestamp(start_time, SFX_TICKS_PER_SEC);
	}

	_iterator = sfx_iterator_combine(_iterator, it);
	_iteratorIsDone = false;

	return Common::kNoError;
}

Common::Error NewPlayer::stop(void) {
	debug(3, "Player: Stopping song iterator %p", (void *)_iterator);
	Common::StackLock lock(*_mutex);
	delete _iterator;
	_iterator = NULL;
	for (int i = 0; i < MIDI_CHANNELS; i++)
		static_cast<MidiDriver *>(_mididrv)->send(0xb0 + i, SCI_MIDI_CHANNEL_NOTES_OFF, 0);

	return Common::kNoError;
}

Common::Error NewPlayer::iterator_message(const SongIterator::Message &msg) {
	Common::StackLock lock(*_mutex);
	if (!_iterator) {
		return Common::kUnknownError;
	}

	songit_handle_message(&_iterator, msg);

	return Common::kNoError;
}

Common::Error NewPlayer::pause(void) {
	Common::StackLock lock(*_mutex);

	_paused = true;
	_pauseTimeDiff = _wakeupTime.msecsDiff(_currentTime);

	_mididrv->playSwitch(false);

	return Common::kNoError;
}

Common::Error NewPlayer::resume(void) {
	Common::StackLock lock(*_mutex);

	_wakeupTime = Audio::Timestamp(_currentTime.msecs() + _pauseTimeDiff, SFX_TICKS_PER_SEC);
	_mididrv->playSwitch(true);
	_paused = false;

	return Common::kNoError;
}

Common::Error NewPlayer::exit(void) {
	_mididrv->close();
	delete _mididrv;
	delete _mutex;
	delete _iterator;
	_iterator = NULL;

	return Common::kNoError;
}


#pragma mark -


int sfx_pcm_available() {
	return g_system->getMixer()->isReady();
}

void sfx_reset_player() {
	if (player)
		player->stop();
}

void sfx_player_tell_synth(int buf_nr, byte *buf) {
	if (player)
		player->tell_synth(buf_nr, buf);
}

int sfx_get_player_polyphony() {
	if (player)
		return player->_polyphony;
	else
		return 0;
}

static void _freeze_time(SfxState *self) {
	/* Freezes the top song delay time */
	const Audio::Timestamp ctime = Audio::Timestamp(g_system->getMillis(), SFX_TICKS_PER_SEC);
	song_t *song = self->_song;

	while (song) {
		song->_delay = song->_wakeupTime.frameDiff(ctime);
		if (song->_delay < 0)
			song->_delay = 0;

		song = song->next_playing;
	}
}

static void _thaw_time(SfxState *self) {
	/* inverse of _freeze_time() */
	const Audio::Timestamp ctime = Audio::Timestamp(g_system->getMillis(), SFX_TICKS_PER_SEC);
	song_t *song = self->_song;

	while (song) {
		song->_wakeupTime = ctime.addFrames(song->_delay);

		song = song->next_playing;
	}
}

#if 0
// Unreferenced - removed
static void _dump_playing_list(SfxState *self, char *msg) {
	song_t *song = self->_song;

	fprintf(stderr, "[] Song list : [ ");
	song = *(self->_songlib.lib);
	while (song) {
		fprintf(stderr, "%08lx:%d ", song->handle, song->status);
		song = song->next_playing;
	}
	fprintf(stderr, "]\n");

	fprintf(stderr, "[] Play list (%s) : [ " , msg);

	while (song) {
		fprintf(stderr, "%08lx ", song->handle);
		song = song->next_playing;
	}

	fprintf(stderr, "]\n");
}
#endif

static void _dump_songs(SfxState *self) {
#if 0
	song_t *song = self->_song;

	fprintf(stderr, "Cue iterators:\n");
	song = *(self->_songlib.lib);
	while (song) {
		fprintf(stderr, "  **\tHandle %08x (p%d): status %d\n",
		        song->handle, song->priority, song->status);
		SIMSG_SEND(song->it, SIMSG_PRINT(1));
		song = song->next;
	}

	if (player) {
		fprintf(stderr, "Audio iterator:\n");
		player->iterator_message(SongIterator::Message(0, SIMSG_PRINT(1)));
	}
#endif
}

static int is_playing(SfxState *self, song_t *song) {
	song_t *playing_song = self->_song;

	/*	_dump_playing_list(self, "is-playing");*/

	while (playing_song) {
		if (playing_song == song)
			return 1;
		playing_song = playing_song->next_playing;
	}
	return 0;
}

static void _sfx_set_song_status(SfxState *self, song_t *song, int status) {
	const Audio::Timestamp ctime = Audio::Timestamp(g_system->getMillis(), SFX_TICKS_PER_SEC);

	switch (status) {

	case SOUND_STATUS_STOPPED:
		// Reset
		song->it->init();
		break;

	case SOUND_STATUS_SUSPENDED:
	case SOUND_STATUS_WAITING:
		if (song->status == SOUND_STATUS_PLAYING) {
			// Update delay, set wakeup_time
			song->_delay += song->_wakeupTime.frameDiff(ctime);
			song->_wakeupTime = ctime;
		}
		if (status == SOUND_STATUS_SUSPENDED)
			break;

		/* otherwise... */

	case SOUND_STATUS_PLAYING:
		if (song->status == SOUND_STATUS_STOPPED) {
			// Starting anew
			song->_wakeupTime = ctime;
		}

		if (is_playing(self, song))
			status = SOUND_STATUS_PLAYING;
		else
			status = SOUND_STATUS_WAITING;
		break;

	default:
		fprintf(stderr, "%s L%d: Attempt to set invalid song"
		        " state %d!\n", __FILE__, __LINE__, status);
		return;

	}
	song->status = status;
}

/* Update internal state iff only one song may be played */
static void _update_single_song(SfxState *self) {
	song_t *newsong = song_lib_find_active(self->_songlib);

	if (newsong != self->_song) {
		_freeze_time(self); /* Store song delay time */

		if (player)
			player->stop();

		if (newsong) {
			if (!newsong->it)
				return; /* Restore in progress and not ready for this yet */

			/* Change song */
			if (newsong->status == SOUND_STATUS_WAITING)
				_sfx_set_song_status(self, newsong,
				                     SOUND_STATUS_PLAYING);

			/* Change instrument mappings */
		} else {
			/* Turn off sound */
		}
		if (self->_song) {
			if (self->_song->status == SOUND_STATUS_PLAYING)
				_sfx_set_song_status(self, newsong,
				                     SOUND_STATUS_WAITING);
		}

		if (self->_debug & SFX_DEBUG_SONGS) {
			sciprintf("[SFX] Changing active song:");
			if (!self->_song)
				sciprintf(" New song:");
			else
				sciprintf(" pausing %08lx, now playing", self->_song->handle);

			if (newsong)
				sciprintf(" %08lx\n", newsong->handle);
			else
				sciprintf(" none\n");
		}


		self->_song = newsong;
		_thaw_time(self); /* Recover song delay time */

		if (newsong && player) {
			SongIterator *clonesong = newsong->it->clone(newsong->_delay);

			player->add_iterator(clonesong, newsong->_wakeupTime.msecs());
		}
	}
}


static void _update_multi_song(SfxState *self) {
	song_t *oldfirst = self->_song;
	song_t *oldseeker;
	song_t *newsong = song_lib_find_active(self->_songlib);
	song_t *newseeker;
	song_t not_playing_anymore; /* Dummy object, referenced by
				    ** songs which are no longer
				    ** active.  */

	/*	_dump_playing_list(self, "before");*/
	_freeze_time(self); /* Store song delay time */

	if (!newsong)
		return;

	for (newseeker = newsong; newseeker;
	        newseeker = newseeker->next_playing) {
		if (!newseeker || !newseeker->it)
			return; /* Restore in progress and not ready for this yet */
	}

	/* First, put all old songs into the 'stopping' list and
	** mark their 'next-playing' as not_playing_anymore.  */
	for (oldseeker = oldfirst; oldseeker;
	        oldseeker = oldseeker->next_stopping) {
		oldseeker->next_stopping = oldseeker->next_playing;
		oldseeker->next_playing = &not_playing_anymore;

		if (oldseeker == oldseeker->next_playing) { BREAKPOINT(); }
	}

	/* Second, re-generate the new song queue. */
	for (newseeker = newsong; newseeker;
	        newseeker = newseeker->next_playing) {
		newseeker->next_playing
		= song_lib_find_next_active(self->_songlib,
		                            newseeker);

		if (newseeker == newseeker->next_playing) { BREAKPOINT(); }
	}
	/* We now need to update the currently playing song list, because we're
	** going to use some functions that require this list to be in a sane
	** state (particularly is_playing(), indirectly */
	self->_song = newsong;

	/* Third, stop all old songs */
	for (oldseeker = oldfirst; oldseeker;
	        oldseeker = oldseeker->next_stopping)
		if (oldseeker->next_playing == &not_playing_anymore) {
			_sfx_set_song_status(self, oldseeker,
			                     SOUND_STATUS_SUSPENDED);
			if (self->_debug & SFX_DEBUG_SONGS) {
				sciprintf("[SFX] Stopping song %lx\n", oldseeker->handle);
			}
			if (player && oldseeker->it)
				player->iterator_message(SongIterator::Message(oldseeker->it->ID, SIMSG_STOP));
			oldseeker->next_playing = NULL; /* Clear this pointer; we don't need the tag anymore */
		}

	for (newseeker = newsong; newseeker; newseeker = newseeker->next_playing) {
		if (newseeker->status != SOUND_STATUS_PLAYING && player) {
			if (self->_debug & SFX_DEBUG_SONGS)
				sciprintf("[SFX] Adding song %lx\n", newseeker->it->ID);

			SongIterator *clonesong = newseeker->it->clone(newseeker->_delay);
			player->add_iterator(clonesong, g_system->getMillis());
		}
		_sfx_set_song_status(self, newseeker,
		                     SOUND_STATUS_PLAYING);
	}

	self->_song = newsong;
	_thaw_time(self);
	/*	_dump_playing_list(self, "after");*/
}

/* Update internal state */
static void _update(SfxState *self) {
	if (self->_flags & SFX_STATE_FLAG_MULTIPLAY)
		_update_multi_song(self);
	else
		_update_single_song(self);
}

int sfx_play_iterator_pcm(SongIterator *it, song_handle_t handle) {
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Playing PCM: %08lx\n", handle);
#endif
	if (g_system->getMixer()->isReady()) {
		Audio::AudioStream *newfeed = it->getAudioStream();
		if (newfeed) {
			g_system->getMixer()->playInputStream(Audio::Mixer::kSFXSoundType, 0, newfeed);
			return 1;
		}
	}
	return 0;
}

#define DELAY (1000000 / SFX_TICKS_PER_SEC)

void sfx_init(SfxState *self, ResourceManager *resmgr, int flags) {
	song_lib_init(&self->_songlib);
	self->_song = NULL;
	self->_flags = flags;
	self->_debug = 0; /* Disable all debugging by default */
	self->_soundSync = NULL;
	self->_audioResource = NULL;

	player = NULL;

	if (flags & SFX_STATE_FLAG_NOSOUND) {
		warning("[SFX] Sound disabled");
		return;
	}

#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Initialising: flags=%x\n", flags);
#endif

	/*-------------------*/
	/* Initialise player */
	/*-------------------*/

	if (!resmgr) {
		warning("[SFX] Warning: No resource manager present, cannot initialise player");
		return;
	}

	player = new NewPlayer();

	if (!player) {
		sciprintf("[SFX] No song player found\n");
		return;
	}

	if (player->init(resmgr, DELAY / 1000)) {
		warning("[SFX] Song player reported error, disabled");
		delete player;
		player = NULL;
	}
}

void sfx_exit(SfxState *self) {
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Uninitialising\n");
#endif

	if (player) {
		player->exit();
		delete player;
		player = 0;
	}

	g_system->getMixer()->stopAll();

	song_lib_free(self->_songlib);

	// Delete audio resources for CD talkie games
	if (self->_audioResource) {
		delete self->_audioResource;
		self->_audioResource = 0;
	}
}

void sfx_suspend(SfxState *self, int suspend) {
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Suspending? = %d\n", suspend);
#endif
	if (suspend && (!self->_suspended)) {
		/* suspend */

		_freeze_time(self);
		if (player)
			player->pause();
		/* Suspend song player */

	} else if (!suspend && (self->_suspended)) {
		/* unsuspend */

		_thaw_time(self);
		if (player)
			player->resume();

		/* Unsuspend song player */
	}

	self->_suspended = suspend;
}

int sfx_poll(SfxState *self, song_handle_t *handle, int *cue) {
/* Polls the sound server for cues etc.
** Returns   : (int) 0 if the cue queue is empty, SI_LOOP, SI_CUE, or SI_FINISHED otherwise
**             (song_handle_t) *handle: The affected handle
**             (int) *cue: The sound cue number (if SI_CUE)
*/
	if (!self->_song)
		return 0; /* No milk today */

	*handle = self->_song->handle;

#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Polling any (%08lx)\n", *handle);
#endif
	return sfx_poll_specific(self, *handle, cue);
}

int sfx_poll_specific(SfxState *self, song_handle_t handle, int *cue) {
	const Audio::Timestamp ctime = Audio::Timestamp(g_system->getMillis(), SFX_TICKS_PER_SEC);
	song_t *song = self->_song;

	while (song && song->handle != handle)
		song = song->next_playing;

	if (!song)
		return 0; /* Song not playing */

	if (self->_debug & SFX_DEBUG_CUES) {
		fprintf(stderr, "[SFX:CUE] Polled song %08lx ", handle);
	}

	while (1) {
		if (song->_wakeupTime.frameDiff(ctime) > 0)
			return 0; /* Patience, young hacker! */

		byte buf[8];
		int result = songit_next(&(song->it), buf, cue, IT_READER_MASK_ALL);

		switch (result) {

		case SI_FINISHED:
			_sfx_set_song_status(self, song, SOUND_STATUS_STOPPED);
			_update(self);
			/* ...fall through... */
		case SI_LOOP:
		case SI_RELATIVE_CUE:
		case SI_ABSOLUTE_CUE:
			if (self->_debug & SFX_DEBUG_CUES) {
				sciprintf(" => ");

				if (result == SI_FINISHED)
					sciprintf("finished\n");
				else {
					if (result == SI_LOOP)
						sciprintf("Loop: ");
					else
						sciprintf("Cue: ");

					sciprintf("%d (0x%x)", *cue, *cue);
				}
			}
			return result;

		default:
			if (result > 0)
				song->_wakeupTime = song->_wakeupTime.addFrames(result);

			/* Delay */
			break;
		}
	}
	if (self->_debug & SFX_DEBUG_CUES) {
		fprintf(stderr, "\n");
	}
}


/*****************/
/*  Song basics  */
/*****************/

int sfx_add_song(SfxState *self, SongIterator *it, int priority, song_handle_t handle, int number) {
	song_t *song = song_lib_find(self->_songlib, handle);

#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Adding song: %08lx at %d, it=%p\n", handle, priority, it);
#endif
	if (!it) {
		fprintf(stderr, "[SFX] Attempt to add empty song with handle %08lx\n", handle);
		return -1;
	}

	it->init();

	/* If we're already playing this, stop it */
	/* Tell player to shut up */
	_dump_songs(self);

	if (player)
		player->iterator_message(SongIterator::Message(handle, SIMSG_STOP));

	if (song) {
		_sfx_set_song_status(self, song, SOUND_STATUS_STOPPED);

		fprintf(stderr, "Overwriting old song (%08lx) ...\n", handle);
		if (song->status == SOUND_STATUS_PLAYING
		        || song->status == SOUND_STATUS_SUSPENDED) {
			warning("Unexpected (error): Song %ld still playing/suspended (%d)",
			        handle, song->status);
			delete it;
			return -1;
		} else
			song_lib_remove(self->_songlib, handle); /* No duplicates */

	}

	song = song_new(handle, it, priority);
	song->resource_num = number;
	song->hold = 0;
	song->loops = 0;
	song->_wakeupTime = Audio::Timestamp(g_system->getMillis(), SFX_TICKS_PER_SEC);
	song_lib_add(self->_songlib, song);
	self->_song = NULL; /* As above */
	_update(self);

	return 0;
}

void sfx_remove_song(SfxState *self, song_handle_t handle) {
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Removing song: %08lx\n", handle);
#endif
	if (self->_song && self->_song->handle == handle)
		self->_song = NULL;

	song_lib_remove(self->_songlib, handle);
	_update(self);
}



/**********************/
/* Song modifications */
/**********************/

#define ASSERT_SONG(s) if (!(s)) { warning("Looking up song handle %08lx failed in %s, L%d", handle, __FILE__, __LINE__); return; }

void sfx_song_set_status(SfxState *self, song_handle_t handle, int status) {
	song_t *song = song_lib_find(self->_songlib, handle);
	ASSERT_SONG(song);
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Setting song status to %d"
	        " (0:stop, 1:play, 2:susp, 3:wait): %08lx\n", status, handle);
#endif

	_sfx_set_song_status(self, song, status);

	_update(self);
}

void sfx_song_set_fade(SfxState *self, song_handle_t handle,
	fade_params_t *params) {
#ifdef DEBUG_SONG_API
	static const char *stopmsg[] = {"??? Should not happen", "Do not stop afterwards", "Stop afterwards"};
#endif
	song_t *song = song_lib_find(self->_songlib, handle);

	ASSERT_SONG(song);

#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Setting fade params of %08lx to "
	        "final volume %d in steps of %d per %d ticks. %s.",
	        handle, fade->final_volume, fade->step_size, fade->ticks_per_step,
	        stopmsg[fade->action]);
#endif

	SIMSG_SEND_FADE(song->it, params);

	_update(self);
}

void sfx_song_renice(SfxState *self, song_handle_t handle, int priority) {
	song_t *song = song_lib_find(self->_songlib, handle);
	ASSERT_SONG(song);
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Renicing song %08lx to %d\n",
	        handle, priority);
#endif

	song->priority = priority;

	_update(self);
}

void sfx_song_set_loops(SfxState *self, song_handle_t handle, int loops) {
	song_t *song = song_lib_find(self->_songlib, handle);
	SongIterator::Message msg = SongIterator::Message(handle, SIMSG_SET_LOOPS(loops));
	ASSERT_SONG(song);

	song->loops = loops;
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Setting loops on %08lx to %d\n",
	        handle, loops);
#endif
	songit_handle_message(&(song->it), msg);

	if (player/* && player->send_iterator_message*/)
		/* FIXME: The above should be optional! */
		player->iterator_message(msg);
}

void sfx_song_set_hold(SfxState *self, song_handle_t handle, int hold) {
	song_t *song = song_lib_find(self->_songlib, handle);
	SongIterator::Message msg = SongIterator::Message(handle, SIMSG_SET_HOLD(hold));
	ASSERT_SONG(song);

	song->hold = hold;
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Setting hold on %08lx to %d\n",
	        handle, hold);
#endif
	songit_handle_message(&(song->it), msg);

	if (player/* && player->send_iterator_message*/)
		/* FIXME: The above should be optional! */
		player->iterator_message(msg);
}

/* Different from the one in iterator.c */
static const int MIDI_cmdlen[16] = {0, 0, 0, 0, 0, 0, 0, 0,
                                    3, 3, 0, 3, 2, 0, 3, 0
                                   };

static const song_handle_t midi_send_base = 0xffff0000;

Common::Error sfx_send_midi(SfxState *self, song_handle_t handle, int channel,
	int command, int arg1, int arg2) {
	byte buffer[5];

	/* Yes, in that order. SCI channel mutes are actually done via
	   a counting semaphore. 0 means to decrement the counter, 1
	   to increment it. */
	static const char *channel_state[] = {"ON", "OFF"};

	if (command == 0xb0 &&
	        arg1 == SCI_MIDI_CHANNEL_MUTE) {
		sciprintf("TODO: channel mute (channel %d %s)!\n", channel,
		          channel_state[arg2]);
		/* We need to have a GET_PLAYMASK interface to use
		   here. SET_PLAYMASK we've got.
		*/
		return Common::kNoError;
	}

	buffer[0] = channel | command; /* No channel remapping yet */

	switch (command) {
	case 0x80 :
	case 0x90 :
	case 0xb0 :
		buffer[1] = arg1 & 0xff;
		buffer[2] = arg2 & 0xff;
		break;
	case 0xc0 :
		buffer[1] = arg1 & 0xff;
		break;
	case 0xe0 :
		buffer[1] = (arg1 & 0x7f) | 0x80;
		buffer[2] = (arg1 & 0xff00) >> 7;
		break;
	default:
		sciprintf("Unexpected explicit MIDI command %02x\n", command);
		return Common::kUnknownError;
	}

	if (player)
		player->tell_synth(MIDI_cmdlen[command >> 4], buffer);
	return Common::kNoError;
}

int sfx_get_volume(SfxState *self) {
	warning("FIXME: Implement volume");
	return 0;
}

void sfx_set_volume(SfxState *self, int volume) {
	warning("FIXME: Implement volume");
}

void sfx_all_stop(SfxState *self) {
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] All stop\n");
#endif

	song_lib_free(self->_songlib);
	_update(self);
}

} // End of namespace Sci
