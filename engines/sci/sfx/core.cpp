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
#include "sci/sci.h"
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

/* Plays a song iterator that found a PCM through a PCM device, if possible
** Parameters: (SongIterator *) it: The iterator to play
**             (SongHandle) handle: Debug handle
** Returns   : (int) 0 if the effect will not be played, nonzero if it will
** This assumes that the last call to 'it->next()' returned SI_PCM.
*/
static int sfx_play_iterator_pcm(SongIterator *it, SongHandle handle);


#pragma mark -


class SfxPlayer {
public:
	/** Number of voices that can play simultaneously */
	int _polyphony;

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
	SfxPlayer();
	~SfxPlayer();

	/* Initializes the player
	** Parameters: (ResourceManager *) resMan: A resource manager for driver initialization
	**             (int) expected_latency: Expected delay in between calls to 'maintenance'
	**                   (in microseconds)
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	*/
	Common::Error init(ResourceManager *resMan, int expected_latency);

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
	Common::Error add_iterator(SongIterator *it, uint32 start_time);

	/* Stops the currently playing song and deletes the associated iterator
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	*/
	Common::Error stop();

	/* Transmits a song iterator message to the active song
	** Parameters: (SongIterator::Message) msg: The message to transmit
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	** OPTIONAL -- may be NULL
	** If this method is not present, sending messages will stop
	** and re-start playing, so it is preferred that it is present
	*/
	Common::Error iterator_message(const SongIterator::Message &msg);

	/* Pauses song playing
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	*/
	Common::Error pause();

	/* Resumes song playing after a pause
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	*/
	Common::Error resume();

	/* Pass a raw MIDI event to the synth
	Parameters: (int) argc: Length of buffer holding the midi event
	           (byte *) argv: The buffer itself
	*/
	void tell_synth(int buf_nr, byte *buf);
};

SfxPlayer::SfxPlayer() {
	_polyphony = 0;

	_mididrv = 0;

	_iterator = NULL;
	_pauseTimeDiff = 0;

	_paused = false;
	_iteratorIsDone = false;
	_tempo = 0;

	_mutex = 0;
	_volume = 15;
}

SfxPlayer::~SfxPlayer() {
	if (_mididrv) {
		_mididrv->close();
		delete _mididrv;
	}
	delete _mutex;
	delete _iterator;
	_iterator = NULL;
}

void SfxPlayer::play_song(SongIterator *it) {
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

void SfxPlayer::tell_synth(int buf_nr, byte *buf) {
	byte op1 = (buf_nr < 2 ? 0 : buf[1]);
	byte op2 = (buf_nr < 3 ? 0 : buf[2]);

	static_cast<MidiDriver *>(_mididrv)->send(buf[0], op1, op2);
}

void SfxPlayer::player_timer_callback(void *refCon) {
	SfxPlayer *thePlayer = (SfxPlayer *)refCon;
	assert(refCon);
	Common::StackLock lock(*thePlayer->_mutex);

	if (thePlayer->_iterator && !thePlayer->_iteratorIsDone && !thePlayer->_paused) {
		thePlayer->play_song(thePlayer->_iterator);
	}

	thePlayer->_currentTime = thePlayer->_currentTime.addFrames(1);
}

/* API implementation */

Common::Error SfxPlayer::init(ResourceManager *resMan, int expected_latency) {
	MidiDriverType musicDriver = MidiDriver::detectMusicDriver(MDT_PCSPK | MDT_ADLIB);

	switch (musicDriver) {
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
	_mididrv->open(resMan);
	_mididrv->setVolume(_volume);

	return Common::kNoError;
}

Common::Error SfxPlayer::add_iterator(SongIterator *it, uint32 start_time) {
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

Common::Error SfxPlayer::stop() {
	debug(3, "Player: Stopping song iterator %p", (void *)_iterator);
	Common::StackLock lock(*_mutex);
	delete _iterator;
	_iterator = NULL;
	for (int i = 0; i < MIDI_CHANNELS; i++)
		static_cast<MidiDriver *>(_mididrv)->send(0xb0 + i, SCI_MIDI_CHANNEL_NOTES_OFF, 0);

	return Common::kNoError;
}

Common::Error SfxPlayer::iterator_message(const SongIterator::Message &msg) {
	Common::StackLock lock(*_mutex);
	if (!_iterator) {
		return Common::kUnknownError;
	}

	songit_handle_message(&_iterator, msg);

	return Common::kNoError;
}

Common::Error SfxPlayer::pause() {
	Common::StackLock lock(*_mutex);

	_paused = true;
	_pauseTimeDiff = _wakeupTime.msecsDiff(_currentTime);

	_mididrv->playSwitch(false);

	return Common::kNoError;
}

Common::Error SfxPlayer::resume() {
	Common::StackLock lock(*_mutex);

	_wakeupTime = Audio::Timestamp(_currentTime.msecs() + _pauseTimeDiff, SFX_TICKS_PER_SEC);
	_mididrv->playSwitch(true);
	_paused = false;

	return Common::kNoError;
}


#pragma mark -


int sfx_pcm_available() {
	return g_system->getMixer()->isReady();
}

void SfxState::sfx_reset_player() {
	if (_player)
		_player->stop();
}

void SfxState::sfx_player_tell_synth(int buf_nr, byte *buf) {
	if (_player)
		_player->tell_synth(buf_nr, buf);
}

int SfxState::sfx_get_player_polyphony() {
	if (_player)
		return _player->_polyphony;
	else
		return 0;
}

SfxState::SfxState() {
	_player = NULL;
	_it = NULL;
	_flags = 0;
	_song = NULL;
	_suspended = 0;
	_syncResource = NULL;
	_audioRate = 11025;
}

SfxState::~SfxState() {
	if (_syncResource)
		_resMan->unlockResource(_syncResource);
}


void SfxState::freezeTime() {
	/* Freezes the top song delay time */
	const Audio::Timestamp ctime = Audio::Timestamp(g_system->getMillis(), SFX_TICKS_PER_SEC);
	Song *song = _song;

	while (song) {
		song->_delay = song->_wakeupTime.frameDiff(ctime);
		if (song->_delay < 0)
			song->_delay = 0;

		song = song->_nextPlaying;
	}
}

void SfxState::thawTime() {
	/* inverse of freezeTime() */
	const Audio::Timestamp ctime = Audio::Timestamp(g_system->getMillis(), SFX_TICKS_PER_SEC);
	Song *song = _song;

	while (song) {
		song->_wakeupTime = ctime.addFrames(song->_delay);

		song = song->_nextPlaying;
	}
}

#if 0
// Unreferenced - removed
static void _dump_playing_list(SfxState *self, char *msg) {
	Song *song = self->_song;

	fprintf(stderr, "[] Song list : [ ");
	song = *(self->_songlib.lib);
	while (song) {
		fprintf(stderr, "%08lx:%d ", song->handle, song->_status);
		song = song->_nextPlaying;
	}
	fprintf(stderr, "]\n");

	fprintf(stderr, "[] Play list (%s) : [ " , msg);

	while (song) {
		fprintf(stderr, "%08lx ", song->handle);
		song = song->_nextPlaying;
	}

	fprintf(stderr, "]\n");
}
#endif

#if 0
static void _dump_songs(SfxState *self) {
	Song *song = self->_song;

	fprintf(stderr, "Cue iterators:\n");
	song = *(self->_songlib.lib);
	while (song) {
		fprintf(stderr, "  **\tHandle %08x (p%d): status %d\n",
		        song->handle, song->_priority, song->_status);
		SIMSG_SEND(song->_it, SIMSG_PRINT(1));
		song = song->_next;
	}

	if (self->_player) {
		fprintf(stderr, "Audio iterator:\n");
		self->_player->iterator_message(SongIterator::Message(0, SIMSG_PRINT(1)));
	}
}
#endif

bool SfxState::isPlaying(Song *song) {
	Song *playing_song = _song;

	/*	_dump_playing_list(this, "is-playing");*/

	while (playing_song) {
		if (playing_song == song)
			return true;
		playing_song = playing_song->_nextPlaying;
	}
	return false;
}

void SfxState::setSongStatus(Song *song, int status) {
	const Audio::Timestamp ctime = Audio::Timestamp(g_system->getMillis(), SFX_TICKS_PER_SEC);

	switch (status) {

	case SOUND_STATUS_STOPPED:
		// Reset
		song->_it->init();
		break;

	case SOUND_STATUS_SUSPENDED:
	case SOUND_STATUS_WAITING:
		if (song->_status == SOUND_STATUS_PLAYING) {
			// Update delay, set wakeup_time
			song->_delay += song->_wakeupTime.frameDiff(ctime);
			song->_wakeupTime = ctime;
		}
		if (status == SOUND_STATUS_SUSPENDED)
			break;

		/* otherwise... */

	case SOUND_STATUS_PLAYING:
		if (song->_status == SOUND_STATUS_STOPPED) {
			// Starting anew
			song->_wakeupTime = ctime;
		}

		if (isPlaying(song))
			status = SOUND_STATUS_PLAYING;
		else
			status = SOUND_STATUS_WAITING;
		break;

	default:
		fprintf(stderr, "%s L%d: Attempt to set invalid song"
		        " state %d!\n", __FILE__, __LINE__, status);
		return;

	}
	song->_status = status;
}

/* Update internal state iff only one song may be played */
void SfxState::updateSingleSong() {
	Song *newsong = _songlib.findFirstActive();

	if (newsong != _song) {
		freezeTime(); /* Store song delay time */

		if (_player)
			_player->stop();

		if (newsong) {
			if (!newsong->_it)
				return; /* Restore in progress and not ready for this yet */

			/* Change song */
			if (newsong->_status == SOUND_STATUS_WAITING)
				setSongStatus(newsong, SOUND_STATUS_PLAYING);

			/* Change instrument mappings */
		} else {
			/* Turn off sound */
		}
		if (_song) {
			if (_song->_status == SOUND_STATUS_PLAYING)
				setSongStatus(newsong, SOUND_STATUS_WAITING);
		}

		Common::String debugMessage = "[SFX] Changing active song:";
		if (!_song) {
			debugMessage += " New song:";
		} else {
			char tmp[50];
			sprintf(tmp, " pausing %08lx, now playing ", _song->_handle);
			debugMessage += tmp;
		}

		if (newsong) {
			char tmp[20];
			sprintf(tmp, "%08lx\n", newsong->_handle);
			debugMessage += tmp;
		} else {
			debugMessage += " none\n";
		}

		debugC(2, kDebugLevelSound, "%s", debugMessage.c_str());
				
		_song = newsong;
		thawTime(); /* Recover song delay time */

		if (newsong && _player) {
			SongIterator *clonesong = newsong->_it->clone(newsong->_delay);

			_player->add_iterator(clonesong, newsong->_wakeupTime.msecs());
		}
	}
}


void SfxState::updateMultiSong() {
	Song *oldfirst = _song;
	Song *oldseeker;
	Song *newsong = _songlib.findFirstActive();
	Song *newseeker;
	Song not_playing_anymore; /* Dummy object, referenced by
				    ** songs which are no longer
				    ** active.  */

	/*	_dump_playing_list(this, "before");*/
	freezeTime(); /* Store song delay time */

	if (!newsong)
		return;

	for (newseeker = newsong; newseeker;
	        newseeker = newseeker->_nextPlaying) {
		if (!newseeker || !newseeker->_it)
			return; /* Restore in progress and not ready for this yet */
	}

	/* First, put all old songs into the 'stopping' list and
	** mark their 'next-playing' as not_playing_anymore.  */
	for (oldseeker = oldfirst; oldseeker;
	        oldseeker = oldseeker->_nextStopping) {
		oldseeker->_nextStopping = oldseeker->_nextPlaying;
		oldseeker->_nextPlaying = &not_playing_anymore;

		if (oldseeker == oldseeker->_nextPlaying) { 
			error("updateMultiSong() failed. Breakpoint in %s, line %d", __FILE__, __LINE__);
		}
	}

	/* Second, re-generate the new song queue. */
	for (newseeker = newsong; newseeker; newseeker = newseeker->_nextPlaying) {
		newseeker->_nextPlaying = _songlib.findNextActive(newseeker);

		if (newseeker == newseeker->_nextPlaying) { 
			error("updateMultiSong() failed. Breakpoint in %s, line %d", __FILE__, __LINE__);
		}
	}
	/* We now need to update the currently playing song list, because we're
	** going to use some functions that require this list to be in a sane
	** state (particularly isPlaying(), indirectly */
	_song = newsong;

	/* Third, stop all old songs */
	for (oldseeker = oldfirst; oldseeker;
	        oldseeker = oldseeker->_nextStopping)
		if (oldseeker->_nextPlaying == &not_playing_anymore) {
			setSongStatus(oldseeker, SOUND_STATUS_SUSPENDED);
			debugC(2, kDebugLevelSound, "[SFX] Stopping song %lx\n", oldseeker->_handle);

			if (_player && oldseeker->_it)
				_player->iterator_message(SongIterator::Message(oldseeker->_it->ID, SIMSG_STOP));
			oldseeker->_nextPlaying = NULL; /* Clear this pointer; we don't need the tag anymore */
		}

	for (newseeker = newsong; newseeker; newseeker = newseeker->_nextPlaying) {
		if (newseeker->_status != SOUND_STATUS_PLAYING && _player) {
			debugC(2, kDebugLevelSound, "[SFX] Adding song %lx\n", newseeker->_it->ID);

			SongIterator *clonesong = newseeker->_it->clone(newseeker->_delay);
			_player->add_iterator(clonesong, g_system->getMillis());
		}
		setSongStatus(newseeker, SOUND_STATUS_PLAYING);
	}

	_song = newsong;
	thawTime();
	/*	_dump_playing_list(this, "after");*/
}

/* Update internal state */
void SfxState::update() {
	if (_flags & SFX_STATE_FLAG_MULTIPLAY)
		updateMultiSong();
	else
		updateSingleSong();
}

static int sfx_play_iterator_pcm(SongIterator *it, SongHandle handle) {
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

void SfxState::sfx_init(ResourceManager *resMan, int flags) {
	_songlib._lib = 0;
	_song = NULL;
	_flags = flags;
	_syncResource = NULL;
	_syncOffset = 0;

	_player = NULL;

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

	if (!resMan) {
		warning("[SFX] Warning: No resource manager present, cannot initialise player");
		return;
	}

	_player = new SfxPlayer();

	if (!_player) {
		warning("[SFX] No song player found");
		return;
	}

	if (_player->init(resMan, DELAY / 1000)) {
		warning("[SFX] Song player reported error, disabled");
		delete _player;
		_player = NULL;
	}

	_resMan = resMan;
}

void SfxState::sfx_exit() {
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Uninitialising\n");
#endif

	delete _player;
	_player = 0;

	g_system->getMixer()->stopAll();

	_songlib.freeSounds();
}

void SfxState::sfx_suspend(bool suspend) {
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Suspending? = %d\n", suspend);
#endif
	if (suspend && (!_suspended)) {
		/* suspend */

		freezeTime();
		if (_player)
			_player->pause();
		/* Suspend song player */

	} else if (!suspend && (_suspended)) {
		/* unsuspend */

		thawTime();
		if (_player)
			_player->resume();

		/* Unsuspend song player */
	}

	_suspended = suspend;
}

int SfxState::sfx_poll(SongHandle *handle, int *cue) {
	if (!_song)
		return 0; /* No milk today */

	*handle = _song->_handle;

#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Polling any (%08lx)\n", *handle);
#endif
	return sfx_poll_specific(*handle, cue);
}

int SfxState::sfx_poll_specific(SongHandle handle, int *cue) {
	const Audio::Timestamp ctime = Audio::Timestamp(g_system->getMillis(), SFX_TICKS_PER_SEC);
	Song *song = _song;

	while (song && song->_handle != handle)
		song = song->_nextPlaying;

	if (!song)
		return 0; /* Song not playing */

	debugC(2, kDebugLevelSound, "[SFX:CUE] Polled song %08lx ", handle);

	while (1) {
		if (song->_wakeupTime.frameDiff(ctime) > 0)
			return 0; /* Patience, young hacker! */

		byte buf[8];
		int result = songit_next(&(song->_it), buf, cue, IT_READER_MASK_ALL);

		switch (result) {

		case SI_FINISHED:
			setSongStatus(song, SOUND_STATUS_STOPPED);
			update();
			/* ...fall through... */
		case SI_LOOP:
		case SI_RELATIVE_CUE:
		case SI_ABSOLUTE_CUE:
			if (result == SI_FINISHED)
				debugC(2, kDebugLevelSound, " => finished");
			else {
				if (result == SI_LOOP)
					debugC(2, kDebugLevelSound, " => Loop: %d (0x%x)", *cue, *cue);
				else
					debugC(2, kDebugLevelSound, " => Cue: %d (0x%x)", *cue, *cue);

			}
			return result;

		default:
			if (result > 0)
				song->_wakeupTime = song->_wakeupTime.addFrames(result);

			/* Delay */
			break;
		}
	}

}


/*****************/
/*  Song basics  */
/*****************/

void SfxState::sfx_add_song(SongIterator *it, int priority, SongHandle handle, int number) {
	Song *song = _songlib.findSong(handle);

#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Adding song: %08lx at %d, it=%p\n", handle, priority, it);
#endif
	if (!it) {
		error("[SFX] Attempt to add empty song with handle %08lx", handle);
		return;
	}

	it->init();

	/* If we're already playing this, stop it */
	/* Tell player to shut up */
//	_dump_songs(this);

	if (_player)
		_player->iterator_message(SongIterator::Message(handle, SIMSG_STOP));

	if (song) {
		setSongStatus( song, SOUND_STATUS_STOPPED);

		fprintf(stderr, "Overwriting old song (%08lx) ...\n", handle);
		if (song->_status == SOUND_STATUS_PLAYING || song->_status == SOUND_STATUS_SUSPENDED) {
			delete it;
			error("Unexpected (error): Song %ld still playing/suspended (%d)",
			        handle, song->_status);
			return;
		} else {
			_songlib.removeSong(handle); /* No duplicates */
		}

	}

	song = new Song(handle, it, priority);
	song->_resourceNum = number;
	song->_hold = 0;
	song->_loops = 0;
	song->_wakeupTime = Audio::Timestamp(g_system->getMillis(), SFX_TICKS_PER_SEC);
	_songlib.addSong(song);
	_song = NULL; /* As above */
	update();

	return;
}

void SfxState::sfx_remove_song(SongHandle handle) {
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Removing song: %08lx\n", handle);
#endif
	if (_song && _song->_handle == handle)
		_song = NULL;

	_songlib.removeSong(handle);
	update();
}



/**********************/
/* Song modifications */
/**********************/

#define ASSERT_SONG(s) if (!(s)) { warning("Looking up song handle %08lx failed in %s, L%d", handle, __FILE__, __LINE__); return; }

void SfxState::sfx_song_set_status(SongHandle handle, int status) {
	Song *song = _songlib.findSong(handle);
	ASSERT_SONG(song);
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Setting song status to %d"
	        " (0:stop, 1:play, 2:susp, 3:wait): %08lx\n", status, handle);
#endif

	setSongStatus(song, status);

	update();
}

void SfxState::sfx_song_set_fade(SongHandle handle, fade_params_t *params) {
#ifdef DEBUG_SONG_API
	static const char *stopmsg[] = {"??? Should not happen", "Do not stop afterwards", "Stop afterwards"};
#endif
	Song *song = _songlib.findSong(handle);

	ASSERT_SONG(song);

#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Setting fade params of %08lx to "
	        "final volume %d in steps of %d per %d ticks. %s.",
	        handle, fade->final_volume, fade->step_size, fade->ticks_per_step,
	        stopmsg[fade->action]);
#endif

	SIMSG_SEND_FADE(song->_it, params);

	update();
}

void SfxState::sfx_song_renice(SongHandle handle, int priority) {
	Song *song = _songlib.findSong(handle);
	ASSERT_SONG(song);
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Renicing song %08lx to %d\n",
	        handle, priority);
#endif

	song->_priority = priority;

	update();
}

void SfxState::sfx_song_set_loops(SongHandle handle, int loops) {
	Song *song = _songlib.findSong(handle);
	SongIterator::Message msg = SongIterator::Message(handle, SIMSG_SET_LOOPS(loops));
	ASSERT_SONG(song);

	song->_loops = loops;
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Setting loops on %08lx to %d\n",
	        handle, loops);
#endif
	songit_handle_message(&(song->_it), msg);

	if (_player/* && _player->send_iterator_message*/)
		/* FIXME: The above should be optional! */
		_player->iterator_message(msg);
}

void SfxState::sfx_song_set_hold(SongHandle handle, int hold) {
	Song *song = _songlib.findSong(handle);
	SongIterator::Message msg = SongIterator::Message(handle, SIMSG_SET_HOLD(hold));
	ASSERT_SONG(song);

	song->_hold = hold;
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Setting hold on %08lx to %d\n",
	        handle, hold);
#endif
	songit_handle_message(&(song->_it), msg);

	if (_player/* && _player->send_iterator_message*/)
		/* FIXME: The above should be optional! */
		_player->iterator_message(msg);
}

/* Different from the one in iterator.c */
static const int MIDI_cmdlen[16] = {0, 0, 0, 0, 0, 0, 0, 0,
                                    3, 3, 0, 3, 2, 0, 3, 0
                                   };

static const SongHandle midi_send_base = 0xffff0000;

Common::Error SfxState::sfx_send_midi(SongHandle handle, int channel,
	int command, int arg1, int arg2) {
	byte buffer[5];

	/* Yes, in that order. SCI channel mutes are actually done via
	   a counting semaphore. 0 means to decrement the counter, 1
	   to increment it. */
	static const char *channel_state[] = {"ON", "OFF"};

	if (command == 0xb0 &&
	        arg1 == SCI_MIDI_CHANNEL_MUTE) {
		warning("TODO: channel mute (channel %d %s)", channel, channel_state[arg2]);
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
		warning("Unexpected explicit MIDI command %02x", command);
		return Common::kUnknownError;
	}

	if (_player)
		_player->tell_synth(MIDI_cmdlen[command >> 4], buffer);
	return Common::kNoError;
}

int SfxState::sfx_get_volume() {
	warning("FIXME: Implement volume");
	return 0;
}

void SfxState::sfx_set_volume(int volume) {
	warning("FIXME: Implement volume");
}

void SfxState::sfx_all_stop() {
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] All stop\n");
#endif

	_songlib.freeSounds();
	update();
}

int SfxState::startAudio(uint16 module, uint32 number) {
	int sampleLen;
	Audio::AudioStream *audioStream = getAudioStream(number, module, &sampleLen);

	if (audioStream) {
		g_system->getMixer()->playInputStream(Audio::Mixer::kSpeechSoundType, &_audioHandle, audioStream);
		return sampleLen;
	}

	return 0;
}

int SfxState::getAudioPosition() {
	if (g_system->getMixer()->isSoundHandleActive(_audioHandle))
		return g_system->getMixer()->getSoundElapsedTime(_audioHandle) * 6 / 100; // return elapsed time in ticks
	else
		return -1; // Sound finished
}

enum SolFlags {
	kSolFlagCompressed = 1 << 0,
	kSolFlagUnknown    = 1 << 1,
	kSolFlag16Bit      = 1 << 2,
	kSolFlagIsSigned   = 1 << 3
};

// FIXME: Move this to sound/adpcm.cpp?
// Note that the 16-bit version is also used in coktelvideo.cpp
static const uint16 tableDPCM16[128] = {
	0x0000, 0x0008, 0x0010, 0x0020, 0x0030, 0x0040, 0x0050, 0x0060, 0x0070, 0x0080,
	0x0090, 0x00A0, 0x00B0, 0x00C0, 0x00D0, 0x00E0, 0x00F0, 0x0100, 0x0110, 0x0120,
	0x0130, 0x0140, 0x0150, 0x0160, 0x0170, 0x0180, 0x0190, 0x01A0, 0x01B0, 0x01C0,
	0x01D0, 0x01E0, 0x01F0, 0x0200, 0x0208, 0x0210, 0x0218, 0x0220, 0x0228, 0x0230,
	0x0238, 0x0240, 0x0248, 0x0250, 0x0258, 0x0260, 0x0268, 0x0270, 0x0278, 0x0280,
	0x0288, 0x0290, 0x0298, 0x02A0, 0x02A8, 0x02B0, 0x02B8, 0x02C0, 0x02C8, 0x02D0,
	0x02D8, 0x02E0, 0x02E8, 0x02F0, 0x02F8, 0x0300, 0x0308, 0x0310, 0x0318, 0x0320,
	0x0328, 0x0330, 0x0338, 0x0340, 0x0348, 0x0350, 0x0358, 0x0360, 0x0368, 0x0370,
	0x0378, 0x0380, 0x0388, 0x0390, 0x0398, 0x03A0, 0x03A8, 0x03B0, 0x03B8, 0x03C0,
	0x03C8, 0x03D0, 0x03D8, 0x03E0, 0x03E8, 0x03F0, 0x03F8, 0x0400, 0x0440, 0x0480,
	0x04C0, 0x0500, 0x0540, 0x0580, 0x05C0, 0x0600, 0x0640, 0x0680, 0x06C0, 0x0700,
	0x0740, 0x0780, 0x07C0, 0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00,
	0x0F00, 0x1000, 0x1400, 0x1800, 0x1C00, 0x2000, 0x3000, 0x4000
};

static const byte tableDPCM8[8] = {0, 1, 2, 3, 6, 10, 15, 21};

static void deDPCM16(byte *soundBuf, Common::SeekableReadStream &audioStream, uint32 n) {
	int16 *out = (int16 *) soundBuf;

	int32 s = 0;
	for (uint32 i = 0; i < n; i++) {
		byte b = audioStream.readByte();
		if (b & 0x80)
			s -= tableDPCM16[b & 0x7f];
		else
			s += tableDPCM16[b];

		s = CLIP<int32>(s, -32768, 32767);
		*out++ = s;
	}
}

static void deDPCM8Nibble(byte *soundBuf, int32 &s, byte b) {
	if (b & 8)
		s -= tableDPCM8[7 - (b & 7)];
	else
		s += tableDPCM8[b & 7];
	s = CLIP<int32>(s, 0, 255);
	*soundBuf = TO_LE_16(s);
}

static void deDPCM8(byte *soundBuf, Common::SeekableReadStream &audioStream, uint32 n) {
	int32 s = 0x80;

	for (uint i = 0; i < n; i++) {
		byte b = audioStream.readByte();

		deDPCM8Nibble(soundBuf++, s, b >> 4);
		deDPCM8Nibble(soundBuf++, s, b & 0xf);
	}
}

// Sierra SOL audio file reader
// Check here for more info: http://wiki.multimedia.cx/index.php?title=Sierra_Audio
static bool readSOLHeader(Common::SeekableReadStream *audioStream, int headerSize, uint32 &size, uint16 &audioRate, byte &audioFlags) {
	if (headerSize != 11 && headerSize != 12) {
		warning("SOL audio header of size %i not supported", headerSize);
		return false;
	}

	audioStream->readUint32LE();			// skip "SOL" + 0 (4 bytes)
	audioRate = audioStream->readUint16LE();
	audioFlags = audioStream->readByte();

	size = audioStream->readUint32LE();
	return true;
}

static byte* readSOLAudio(Common::SeekableReadStream *audioStream, uint32 &size, byte audioFlags, byte &flags) {
	byte *buffer;

	// Convert the SOL stream flags to our own format
	flags = 0;
	if (audioFlags & kSolFlag16Bit)
		flags |= Audio::Mixer::FLAG_16BITS | Audio::Mixer::FLAG_LITTLE_ENDIAN;

	if (!(audioFlags & kSolFlagIsSigned))
		flags |= Audio::Mixer::FLAG_UNSIGNED;

	if (audioFlags & kSolFlagCompressed) {
		buffer = (byte *)malloc(size * 2);

		if (audioFlags & kSolFlag16Bit)
			deDPCM16(buffer, *audioStream, size);
		else
			deDPCM8(buffer, *audioStream, size);

		size *= 2;
	} else {
		// We assume that the sound data is raw PCM
		buffer = (byte *)malloc(size);
		audioStream->read(buffer, size);
	}

	return buffer;
}

Audio::AudioStream* SfxState::getAudioStream(uint32 number, uint32 volume, int *sampleLen) {
	Audio::AudioStream *audioStream = 0;
	uint32 size = 0;
	byte *data = 0;
	byte flags = 0;
	Sci::Resource* audioRes;

	if (volume == 65535) {
		audioRes = _resMan->findResource(ResourceId(kResourceTypeAudio, number), false);
		if (!audioRes) {
			warning("Failed to find audio entry %i", number);
			return NULL;
		}
	} else {
		audioRes = _resMan->findResource(ResourceId(kResourceTypeAudio36, volume, number), false);
		if (!audioRes) {
			warning("Failed to find audio entry (%i, %i, %i, %i, %i)", volume, (number >> 24) & 0xff,
					(number >> 16) & 0xff, (number >> 8) & 0xff, number & 0xff);
			return NULL;
		}
	}

	byte audioFlags;

	if (audioRes->headerSize > 0) {
		// SCI1.1
		Common::MemoryReadStream headerStream(audioRes->header, audioRes->headerSize, false);

		if (readSOLHeader(&headerStream, audioRes->headerSize, size, _audioRate, audioFlags)) {
			Common::MemoryReadStream dataStream(audioRes->data, audioRes->size, false);
			data = readSOLAudio(&dataStream, size, audioFlags, flags);
		}
	} else {
		// SCI1
		size = audioRes->size;
		data = (byte *)malloc(size);
		assert(data);
		memcpy(data, audioRes->data, size);
		flags = Audio::Mixer::FLAG_UNSIGNED;
	}

	if (data) {
		audioStream = Audio::makeLinearInputStream(data, size, _audioRate, 
										flags | Audio::Mixer::FLAG_AUTOFREE, 0, 0);
		if (audioStream) {
			*sampleLen = (flags & Audio::Mixer::FLAG_16BITS ? size >> 1 : size) * 60 / _audioRate;
			return audioStream;
		}
	}

	return NULL;
}

} // End of namespace Sci
