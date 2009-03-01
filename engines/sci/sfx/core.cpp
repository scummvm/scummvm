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
#include "sci/sfx/iterator_internal.h"
#include "sci/sfx/player.h"
#include "sci/sfx/mixer.h"
#include "sci/sfx/sci_midi.h"

#include "common/system.h"
#include "common/timer.h"
#include "sound/mixer.h"

namespace Sci {

/*#define DEBUG_SONG_API*/
/*#define DEBUG_CUES*/
#ifdef DEBUG_CUES
int sciprintf(char *msg, ...);
#endif

static sfx_player_t *player = NULL;


int sfx_pcm_available() {
	return g_system->getMixer()->isReady();
}

void sfx_reset_player() {
	if (player)
		player->stop();
}

tell_synth_func *sfx_get_player_tell_func() {
	if (player)
		return player->tell_synth;
	else
		return NULL;
}

int sfx_get_player_polyphony() {
	if (player)
		return player->polyphony;
	else
		return 0;
}

static void _freeze_time(sfx_state_t *self) {
	/* Freezes the top song delay time */
	uint32 ctime = g_system->getMillis();
	song_t *song = self->song;

	while (song) {
		if (ctime > song->wakeup_time)
			song->delay = 0;
		else
			song->delay = song->wakeup_time - ctime;

		song = song->next_playing;
	}
}

#if 0
// Unreferenced - removed
static void _dump_playing_list(sfx_state_t *self, char *msg) {
	song_t *song = self->song;

	fprintf(stderr, "[] Song list : [ ");
	song = *(self->songlib.lib);
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

static void _dump_songs(sfx_state_t *self) {
#if 0
	song_t *song = self->song;

	fprintf(stderr, "Cue iterators:\n");
	song = *(self->songlib.lib);
	while (song) {
		fprintf(stderr, "  **\tHandle %08x (p%d): status %d\n",
		        song->handle, song->priority, song->status);
		SIMSG_SEND(song->it, SIMSG_PRINT(1));
		song = song->next;
	}

	if (player) {
		fprintf(stderr, "Audio iterator:\n");
		player->iterator_message(songit_make_message(0, SIMSG_PRINT(1)));
	}
#endif
}

static void _thaw_time(sfx_state_t *self) {
	/* inverse of _freeze_time() */
	uint32 ctime = g_system->getMillis();
	song_t *song = self->song;

	while (song) {
		song->wakeup_time = ctime + song->delay;

		song = song->next_playing;
	}
}

static int is_playing(sfx_state_t *self, song_t *song) {
	song_t *playing_song = self->song;

	/*	_dump_playing_list(self, "is-playing");*/

	while (playing_song) {
		if (playing_song == song)
			return 1;
		playing_song = playing_song->next_playing;
	}
	return 0;
}

static void _sfx_set_song_status(sfx_state_t *self, song_t *song, int status) {
	switch (status) {

	case SOUND_STATUS_STOPPED:
		/* Reset */
		song->it->init(song->it);
		break;

	case SOUND_STATUS_SUSPENDED:
	case SOUND_STATUS_WAITING:
		if (song->status == SOUND_STATUS_PLAYING) {
			/* Update delay, set wakeup_time */
			uint32 time = g_system->getMillis();

			song->delay -= long(time) - long(song->wakeup_time);
			song->wakeup_time = time;
		}
		if (status == SOUND_STATUS_SUSPENDED)
			break;

		/* otherwise... */

	case SOUND_STATUS_PLAYING:
		if (song->status == SOUND_STATUS_STOPPED)
			/* Starting anew */
			song->wakeup_time = g_system->getMillis();

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
static void _update_single_song(sfx_state_t *self) {
	song_t *newsong = song_lib_find_active(self->songlib);

	if (newsong != self->song) {
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
		if (self->song) {
			if (self->song->status == SOUND_STATUS_PLAYING)
				_sfx_set_song_status(self, newsong,
				                     SOUND_STATUS_WAITING);
		}

		if (self->debug & SFX_DEBUG_SONGS) {
			sciprintf("[SFX] Changing active song:");
			if (!self->song)
				sciprintf(" New song:");
			else
				sciprintf(" pausing %08lx, now playing",
				          self->song->handle);

			if (newsong)
				sciprintf(" %08lx\n", newsong->handle);
			else
				sciprintf(" none\n");
		}


		self->song = newsong;
		_thaw_time(self); /* Recover song delay time */

		if (newsong && player) {
			song_iterator_t *clonesong
			= songit_clone(newsong->it, newsong->delay);

			player->add_iterator(clonesong, newsong->wakeup_time);
		}
	}
}


static void _update_multi_song(sfx_state_t *self) {
	song_t *oldfirst = self->song;
	song_t *oldseeker;
	song_t *newsong = song_lib_find_active(self->songlib);
	song_t *newseeker;
	song_t not_playing_anymore; /* Dummy object, referenced by
				    ** songs which are no longer
				    ** active.  */

	/*	_dump_playing_list(self, "before");*/
	_freeze_time(self); /* Store song delay time */

	for (newseeker = newsong; newseeker;
	        newseeker = newseeker->next_playing) {
		if (!newseeker->it)
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
		= song_lib_find_next_active(self->songlib,
		                            newseeker);

		if (newseeker == newseeker->next_playing) { BREAKPOINT(); }
	}
	/* We now need to update the currently playing song list, because we're
	** going to use some functions that require this list to be in a sane
	** state (particularly is_playing(), indirectly */
	self->song = newsong;

	/* Third, stop all old songs */
	for (oldseeker = oldfirst; oldseeker;
	        oldseeker = oldseeker->next_stopping)
		if (oldseeker->next_playing == &not_playing_anymore) {
			_sfx_set_song_status(self, oldseeker,
			                     SOUND_STATUS_SUSPENDED);
			if (self->debug & SFX_DEBUG_SONGS) {
				sciprintf("[SFX] Stopping song %lx\n", oldseeker->handle);
			}
			if (player && oldseeker->it)
				player->iterator_message
				(songit_make_message(oldseeker->it->ID, SIMSG_STOP));
			oldseeker->next_playing = NULL; /* Clear this pointer; we don't need the tag anymore */
		}

	for (newseeker = newsong; newseeker;
	        newseeker = newseeker->next_playing) {
		if (newseeker->status != SOUND_STATUS_PLAYING && player) {
			if (self->debug & SFX_DEBUG_SONGS)
				sciprintf("[SFX] Adding song %lx\n", newseeker->it->ID);

			player->add_iterator(songit_clone(newseeker->it,
			                                  newseeker->delay),
			                     g_system->getMillis());
		}
		_sfx_set_song_status(self, newseeker,
		                     SOUND_STATUS_PLAYING);
	}

	self->song = newsong;
	_thaw_time(self);
	/*	_dump_playing_list(self, "after");*/
}

/* Update internal state */
static void _update(sfx_state_t *self) {
	if (self->flags & SFX_STATE_FLAG_MULTIPLAY)
		_update_multi_song(self);
	else
		_update_single_song(self);
}

int sfx_play_iterator_pcm(song_iterator_t *it, song_handle_t handle) {
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Playing PCM: %08lx\n", handle);
#endif
	if (g_system->getMixer()->isReady()) {
		sfx_pcm_feed_t *newfeed = it->get_pcm_feed(it);
		if (newfeed) {
			newfeed->debug_nr = (int) handle;
			mixer_subscribe(newfeed);
			return 1;
		}
	}
	return 0;
}

#define FREQ 60
#define DELAY (1000000 / FREQ)


static void _sfx_timer_callback(void *data) {
	/* First run the player, to give it a chance to fill
	** the audio buffer  */

	if (player)
		player->maintenance();

	mixer_process();
}

void sfx_init(sfx_state_t *self, ResourceManager *resmgr, int flags) {
	song_lib_init(&self->songlib);
	self->song = NULL;
	self->flags = flags;
	self->debug = 0; /* Disable all debugging by default */

	if (flags & SFX_STATE_FLAG_NOSOUND) {
		player = NULL;
		sciprintf("[SFX] Sound disabled.\n");
		return;
	}

	player = sfx_find_player(NULL);


#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Initialising: flags=%x\n", flags);
#endif

	/*-------------------*/
	/* Initialise player */
	/*-------------------*/

	if (!resmgr) {
		sciprintf("[SFX] Warning: No resource manager present, cannot initialise player\n");
		player = NULL;
	} else if (player->init(resmgr, DELAY / 1000)) {
		sciprintf("[SFX] Song player '%s' reported error, disabled\n", player->name);
		player = NULL;
	}

	if (!player)
		sciprintf("[SFX] No song player found\n");
	else
		sciprintf("[SFX] Using song player '%s', v%s\n", player->name, player->version);

	/*------------------*/
	/* Initialise timer */
	/*------------------*/

	// We initialise the timer last, so there is no possibility of the
	// timer callback being triggered while the mixer or player are
	// still being initialized.

	if (g_system->getMixer()->isReady() || (player && player->maintenance)) {
		if (!g_system->getTimerManager()->installTimerProc(&_sfx_timer_callback, DELAY, NULL)) {
			warning("[SFX] " __FILE__": Timer failed to initialize");
			warning("[SFX] Disabled sound support");
			player = NULL;
			return;
		}
	} /* With no PCM device and no player, we don't need a timer */

}

void sfx_exit(sfx_state_t *self) {
	g_system->getTimerManager()->removeTimerProc(&_sfx_timer_callback);

	// The timer API guarantees no more callbacks are running or will be
	// run from this point onward, so we can now safely exit the mixer and
	// player.

#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Uninitialising\n");
#endif

	song_lib_free(self->songlib);

	// WARNING: The mixer may hold feeds from the player, so we must
	// stop the mixer BEFORE stopping the player.
	g_system->getMixer()->stopAll();

	if (player)
		// See above: This must happen AFTER stopping the mixer
		player->exit();
}

void sfx_suspend(sfx_state_t *self, int suspend) {
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Suspending? = %d\n", suspend);
#endif
	if (suspend && (!self->suspended)) {
		/* suspend */

		_freeze_time(self);
		if (player)
			player->pause();
		/* Suspend song player */

	} else if (!suspend && (self->suspended)) {
		/* unsuspend */

		_thaw_time(self);
		if (player)
			player->resume();

		/* Unsuspend song player */
	}

	self->suspended = suspend;
}

int sfx_poll(sfx_state_t *self, song_handle_t *handle, int *cue) {
/* Polls the sound server for cues etc.
** Returns   : (int) 0 if the cue queue is empty, SI_LOOP, SI_CUE, or SI_FINISHED otherwise
**             (song_handle_t) *handle: The affected handle
**             (int) *cue: The sound cue number (if SI_CUE)
*/
	if (!self->song)
		return 0; /* No milk today */

	*handle = self->song->handle;

#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Polling any (%08lx)\n", *handle);
#endif
	return sfx_poll_specific(self, *handle, cue);
}

int sfx_poll_specific(sfx_state_t *self, song_handle_t handle, int *cue) {
	uint32 ctime = g_system->getMillis();
	song_t *song = self->song;

	while (song && song->handle != handle)
		song = song->next_playing;

	if (!song)
		return 0; /* Song not playing */

	if (self->debug & SFX_DEBUG_CUES) {
		fprintf(stderr, "[SFX:CUE] Polled song %08lx ", handle);
	}

	while (1) {
		unsigned char buf[8];
		int result;

		if (song->wakeup_time > ctime)
			return 0; /* Patience, young hacker! */

		result = songit_next(&(song->it), buf, cue, IT_READER_MASK_ALL);

		switch (result) {

		case SI_FINISHED:
			_sfx_set_song_status(self, song, SOUND_STATUS_STOPPED);
			_update(self);
			/* ...fall through... */
		case SI_LOOP:
		case SI_RELATIVE_CUE:
		case SI_ABSOLUTE_CUE:
			if (self->debug & SFX_DEBUG_CUES) {
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
				song->wakeup_time += result * SOUND_TICK;

			/* Delay */
			break;
		}
	}
	if (self->debug & SFX_DEBUG_CUES) {
		fprintf(stderr, "\n");
	}
}


/*****************/
/*  Song basics  */
/*****************/

int sfx_add_song(sfx_state_t *self, song_iterator_t *it, int priority, song_handle_t handle, int number) {
	song_t *song = song_lib_find(self->songlib, handle);

#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Adding song: %08lx at %d, it=%p\n", handle, priority, it);
#endif
	if (!it) {
		fprintf(stderr, "[SFX] Attempt to add empty song with handle %08lx\n", handle);
		return -1;
	}

	it->init(it);

	/* If we're already playing this, stop it */
	/* Tell player to shut up */
	_dump_songs(self);

	if (player)
		player->iterator_message(songit_make_message(handle, SIMSG_STOP));

	if (song) {
		_sfx_set_song_status(self, song, SOUND_STATUS_STOPPED);

		fprintf(stderr, "Overwriting old song (%08lx) ...\n", handle);
		if (song->status == SOUND_STATUS_PLAYING
		        || song->status == SOUND_STATUS_SUSPENDED) {
			warning("Unexpected (error): Song %ld still playing/suspended (%d)",
			        handle, song->status);
			songit_free(it);
			return -1;
		} else
			song_lib_remove(self->songlib, handle); /* No duplicates */

	}

	song = song_new(handle, it, priority);
	song->resource_num = number;
	song->hold = 0;
	song->loops = 0;
	song->wakeup_time = g_system->getMillis(); /* No need to delay */
	song_lib_add(self->songlib, song);
	self->song = NULL; /* As above */
	_update(self);

	return 0;
}

void sfx_remove_song(sfx_state_t *self, song_handle_t handle) {
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Removing song: %08lx\n", handle);
#endif
	if (self->song && self->song->handle == handle)
		self->song = NULL;

	song_lib_remove(self->songlib, handle);
	_update(self);
}



/**********************/
/* Song modifications */
/**********************/

#define ASSERT_SONG(s) if (!(s)) { warning("Looking up song handle %08lx failed in %s, L%d", handle, __FILE__, __LINE__); return; }

void sfx_song_set_status(sfx_state_t *self, song_handle_t handle, int status) {
	song_t *song = song_lib_find(self->songlib, handle);
	ASSERT_SONG(song);
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Setting song status to %d"
	        " (0:stop, 1:play, 2:susp, 3:wait): %08lx\n", status, handle);
#endif

	_sfx_set_song_status(self, song, status);

	_update(self);
}

void sfx_song_set_fade(sfx_state_t *self, song_handle_t handle,
	fade_params_t *params) {
#ifdef DEBUG_SONG_API
	static const char *stopmsg[] = {"??? Should not happen", "Do not stop afterwards", "Stop afterwards"};
#endif
	song_t *song = song_lib_find(self->songlib, handle);

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

void sfx_song_renice(sfx_state_t *self, song_handle_t handle, int priority) {
	song_t *song = song_lib_find(self->songlib, handle);
	ASSERT_SONG(song);
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Renicing song %08lx to %d\n",
	        handle, priority);
#endif

	song->priority = priority;

	_update(self);
}

void sfx_song_set_loops(sfx_state_t *self, song_handle_t handle, int loops) {
	song_t *song = song_lib_find(self->songlib, handle);
	song_iterator_message_t msg
	= songit_make_message(handle, SIMSG_SET_LOOPS(loops));
	ASSERT_SONG(song);

#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Setting loops on %08lx to %d\n",
	        handle, loops);
#endif
	songit_handle_message(&(song->it), msg);
	song->loops = ((base_song_iterator_t *) song->it)->loops;

	if (player/* && player->send_iterator_message*/)
		/* FIXME: The above should be optional! */
		player->iterator_message(msg);
}

void sfx_song_set_hold(sfx_state_t *self, song_handle_t handle, int hold) {
	song_t *song = song_lib_find(self->songlib, handle);
	song_iterator_message_t msg
	= songit_make_message(handle, SIMSG_SET_HOLD(hold));
	ASSERT_SONG(song);

	song->hold = hold;
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Setting hold on %08lx to %d\n",
	        handle, loops);
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

int sfx_send_midi(sfx_state_t *self, song_handle_t handle, int channel,
	int command, int arg1, int arg2) {
	byte buffer[5];
	tell_synth_func *tell = sfx_get_player_tell_func();

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
		return SFX_OK;
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
		return SFX_ERROR;
	}

	if (tell)
		tell(MIDI_cmdlen[command >> 4], buffer);
	return SFX_OK;
}

int sfx_get_volume(sfx_state_t *self) {
	warning("FIXME: Implement volume");
	return 0;
}

void sfx_set_volume(sfx_state_t *self, int volume) {
	warning("FIXME: Implement volume");
}

void sfx_all_stop(sfx_state_t *self) {
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] All stop\n");
#endif

	song_lib_free(self->songlib);
	_update(self);
}

} // End of namespace Sci
