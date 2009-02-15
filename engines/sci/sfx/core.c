/***************************************************************************
 core.c Copyright (C) 2002 Christoph Reichenbach


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
/* Sound subsystem core: Event handler, sound player dispatching */

#include <stdio.h>
#include <sfx_timer.h>
#include <sfx_iterator_internal.h>
#include <sfx_player.h>
#include "mixer.h"
#include <sci_midi.h>


/*#define DEBUG_SONG_API*/
/*#define DEBUG_CUES*/
#ifdef DEBUG_CUES
int sciprintf(char *msg, ...);
#endif

static sfx_player_t *player = NULL;
sfx_pcm_mixer_t *mixer = NULL;
static sfx_pcm_device_t *pcm_device = NULL;
static sfx_timer_t *timer = NULL;

#define MILLION 1000000

int
sfx_pcm_available()
{
	return (pcm_device != NULL);
}

void
sfx_reset_player(void)
{
	if (player)
		player->stop();
}

tell_synth_func *
sfx_get_player_tell_func(void)
{
	if (player)
		return player->tell_synth;
	else
		return NULL;
}

int
sfx_get_player_polyphony(void)
{
	if (player)
		return player->polyphony;
	else
		return 0;
}

static long
time_minus(GTimeVal t1, GTimeVal t2)
{
	return (t1.tv_sec - t2.tv_sec) * MILLION
		+ (t1.tv_usec - t2.tv_usec);
}

static GTimeVal
time_plus(GTimeVal t1, long delta)
{
	if (delta > 0)
		t1.tv_usec += delta % MILLION;
	else
		t1.tv_usec -= (-delta) % MILLION;

	t1.tv_sec += delta / MILLION;

	if (t1.tv_usec > MILLION) {
		t1.tv_sec++;
		t1.tv_usec -= MILLION;
	}

	return t1;
}


static void
_freeze_time(sfx_state_t *self)
{
	/* Freezes the top song delay time */
	GTimeVal ctime;
	long delta;
	
	song_t *song = self->song;
	sci_get_current_time(&ctime);

	while (song) {
		delta = time_minus(song->wakeup_time, ctime);
		if (delta < 0)
			delta = 0;

		song->delay = delta;

		song = song->next_playing;
	}
}


static void
_dump_playing_list(sfx_state_t *self, char *msg)
{
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

static void
_dump_songs(sfx_state_t *self)
{
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


static void
_thaw_time(sfx_state_t *self)
{
	/* inverse of _freeze_time() */
	GTimeVal ctime;
	song_t *song = self->song;

	sci_get_current_time(&ctime);

	while (song) {
		song->wakeup_time = time_plus(ctime, song->delay);

		song = song->next_playing;
	}
}

static int
is_playing(sfx_state_t *self, song_t *song)
{
	song_t *playing_song = self->song;

/*	_dump_playing_list(self, "is-playing");*/

	while (playing_song) {
		if (playing_song == song)
			return 1;
		playing_song = playing_song->next_playing;
	}
	return 0;
}

static void
_sfx_set_song_status(sfx_state_t *self, song_t *song, int status)
{
	switch (status) {

	case SOUND_STATUS_STOPPED:
		/* Reset */
		song->it->init(song->it);
		break;

	case SOUND_STATUS_SUSPENDED:
	case SOUND_STATUS_WAITING:

		if (song->status == SOUND_STATUS_PLAYING) {
			/* Update delay, set wakeup_time */
			GTimeVal time;
			long delta;
			sci_get_current_time(&time);
			delta = time_minus(time, song->wakeup_time);

			song->delay -= delta;
			song->wakeup_time = time;
		}
		if (status == SOUND_STATUS_SUSPENDED)
			break;

		/* otherwise... */

	case SOUND_STATUS_PLAYING:
		if (song->status == SOUND_STATUS_STOPPED)
			/* Starting anew */
			sci_get_current_time(&song->wakeup_time);

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
static void
_update_single_song(sfx_state_t *self)
{
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
		
			player->add_iterator(clonesong,
					     newsong->wakeup_time);
		}
	}
}


static void
_update_multi_song(sfx_state_t *self)
{
	song_t *oldfirst = self->song;
	song_t *oldseeker;
	song_t *newsong = song_lib_find_active(self->songlib);
	song_t *newseeker;
	song_t not_playing_anymore; /* Dummy object, referenced by
				    ** songs which are no longer
				    ** active.  */
	GTimeVal tv;
	sci_get_current_time(&tv);
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
					     tv);
		}
		_sfx_set_song_status(self, newseeker,
				     SOUND_STATUS_PLAYING);
	}

	self->song = newsong;
	_thaw_time(self);
/*	_dump_playing_list(self, "after");*/
}

/* Update internal state */
static void
_update(sfx_state_t *self)
{
	if (self->flags & SFX_STATE_FLAG_MULTIPLAY)
		_update_multi_song(self);
	else
		_update_single_song(self);
}


static int _sfx_timer_active = 0; /* Timer toggle */

int
sfx_play_iterator_pcm(song_iterator_t *it, song_handle_t handle)
{
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Playing PCM: %08lx\n", handle);
#endif
	if (mixer) {
		sfx_pcm_feed_t *newfeed = it->get_pcm_feed(it);
		if (newfeed) {
			newfeed->debug_nr = (int) handle;
			mixer->subscribe(mixer, newfeed);
			return 1;
		}
	}
	return 0;
}

static void
_sfx_timer_callback(void *data)
{
	if (_sfx_timer_active) {
		/* First run the player, to give it a chance to fill
		** the audio buffer  */

		if (player)
			player->maintenance();

		if (mixer)
			mixer->process(mixer);
	}
}

void
sfx_init(sfx_state_t *self, resource_mgr_t *resmgr, int flags)
{
	song_lib_init(&self->songlib);
	self->song = NULL;
	self->flags = flags;
	self->debug = 0; /* Disable all debugging by default */

	if (flags & SFX_STATE_FLAG_NOSOUND) {
		mixer = NULL;
		pcm_device = NULL;
		player = NULL;
		sciprintf("[SFX] Sound disabled.\n");
		return;
	}

	mixer = sfx_pcm_find_mixer(NULL);
	pcm_device = sfx_pcm_find_device(NULL);
	player = sfx_find_player(NULL);


#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Initialising: flags=%x\n", flags);
#endif

	/*------------------*/
	/* Initialise timer */
	/*------------------*/

	if (pcm_device || player->maintenance) {
		if (pcm_device && pcm_device->timer)
			timer = pcm_device->timer;
		else
			timer = sfx_find_timer(NULL);

		if (!timer) {
			fprintf(stderr, "[SFX] " __FILE__": Could not find timing mechanism\n");
			fprintf(stderr, "[SFX] Disabled sound support\n");
			pcm_device = NULL;
			player = NULL;
			mixer = NULL;
			return;
		}

		if (timer->init(_sfx_timer_callback, NULL)) {
			fprintf(stderr, "[SFX] " __FILE__": Timer failed to initialize\n");
			fprintf(stderr, "[SFX] Disabled sound support\n");
			timer = NULL;
			pcm_device = NULL;
			player = NULL;
			mixer = NULL;
			return;
		}

		sciprintf("[SFX] Initialised timer '%s', v%s\n",
			  timer->name, timer->version);
	} /* With no PCM device and no player, we don't need a timer */

	/*----------------*/
	/* Initialise PCM */
	/*----------------*/

	if (!pcm_device) {
		sciprintf("[SFX] No PCM device found, disabling PCM support\n");
		mixer = NULL;
	} else {
		if (pcm_device->init(pcm_device)) {
			sciprintf("[SFX] Failed to open PCM device, disabling PCM support\n");
			mixer = NULL;
			pcm_device = NULL;
		} else {
			if (mixer->init(mixer, pcm_device)) {
				sciprintf("[SFX] Failed to initialise PCM mixer; disabling PCM support\n");
				mixer = NULL;
				pcm_device->exit(pcm_device);
				pcm_device = NULL;
			}
		}
	}

	/*-------------------*/
	/* Initialise player */
	/*-------------------*/

	if (!resmgr) {
		sciprintf("[SFX] Warning: No resource manager present, cannot initialise player\n");
		player = NULL;
	} else if (player->init(resmgr, timer? timer->delay_ms : 0)) {
		sciprintf("[SFX] Song player '%s' reported error, disabled\n", player->name);
		player = NULL;
	}

	if (!player)
		sciprintf("[SFX] No song player found\n");
	else
		sciprintf("[SFX] Using song player '%s', v%s\n", player->name, player->version);

	_sfx_timer_active = 1;
}

void
sfx_exit(sfx_state_t *self)
{
	_sfx_timer_active = 0;
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Uninitialising\n");
#endif

	song_lib_free(self->songlib);

	if (pcm_device) {
		pcm_device->exit(pcm_device);
		pcm_device = NULL;
	}
	if (timer && timer->exit())
		fprintf(stderr, "[SFX] Timer reported error on exit\n");

	/* WARNING: The mixer may hold feeds from the
	** player, so we must stop the mixer BEFORE
	** stopping the player. */
	if (mixer) {
		mixer->exit(mixer);
		mixer = NULL;
	}

	if (player)
		/* See above: This must happen AFTER stopping the mixer */
		player->exit();

}

static inline int
time_le(GTimeVal a, GTimeVal b)
{
	return time_minus(a, b) <= 0;
}

void
sfx_suspend(sfx_state_t *self, int suspend)
{
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

int
sfx_poll(sfx_state_t *self, song_handle_t *handle, int *cue)
/* Polls the sound server for cues etc.
** Returns   : (int) 0 if the cue queue is empty, SI_LOOP, SI_CUE, or SI_FINISHED otherwise
**             (song_handle_t) *handle: The affected handle
**             (int) *cue: The sound cue number (if SI_CUE)
*/
{
	if (!self->song)
		return 0; /* No milk today */

	*handle = self->song->handle;

#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Polling any (%08lx)\n", *handle);
#endif
	return sfx_poll_specific(self, *handle, cue);
}

int
sfx_poll_specific(sfx_state_t *self, song_handle_t handle, int *cue)
{
	GTimeVal ctime;
	song_t *song = self->song;

	sci_get_current_time(&ctime);

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

		if (!time_le(song->wakeup_time, ctime))
			return 0; /* Patience, young hacker! */
		result = songit_next(&(song->it), buf, cue,
				     IT_READER_MASK_ALL);

		switch (result) {

		case SI_FINISHED:
			_sfx_set_song_status(self, song,
					     SOUND_STATUS_STOPPED);
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
				song->wakeup_time =
					time_plus(song->wakeup_time,
						  result * SOUND_TICK);
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

int
sfx_add_song(sfx_state_t *self, song_iterator_t *it, int priority, song_handle_t handle, int number)
{
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
			fprintf(stderr, "Unexpected (error): Song %ld still playing/suspended (%d)\n",
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
	sci_get_current_time(&song->wakeup_time); /* No need to delay */
	song_lib_add(self->songlib, song);
	self->song = NULL; /* As above */
	_update(self);

	return 0;
}


void
sfx_remove_song(sfx_state_t *self, song_handle_t handle)
{
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

#define ASSERT_SONG(s) if (!(s)) { fprintf(stderr, "Looking up song handle %08lx failed in %s, L%d\n", handle, __FILE__, __LINE__); return; }

void
sfx_song_set_status(sfx_state_t *self, song_handle_t handle, int status)
{
	song_t *song = song_lib_find(self->songlib, handle);
	ASSERT_SONG(song);
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Setting song status to %d"
		" (0:stop, 1:play, 2:susp, 3:wait): %08lx\n", status, handle);
#endif

	 _sfx_set_song_status(self, song, status);

	_update(self);
}

void
sfx_song_set_fade(sfx_state_t *self, song_handle_t handle, 
		  fade_params_t *params)
{
#ifdef DEBUG_SONG_API
	static const char *stopmsg[] = {"??? Should not happen", "Do not stop afterwards","Stop afterwards"};
#endif
	song_t *song = song_lib_find(self->songlib, handle);

	ASSERT_SONG(song);

#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Setting fade params of %08lx to "
		"final volume %d in steps of %d per %d ticks. %s.\n",
		handle, fade->final_volume, fade->step_size, fade->ticks_per_step,
		stopmsg[fade->action]);
#endif

	SIMSG_SEND_FADE(song->it, params);

	_update(self);
}

void
sfx_song_renice(sfx_state_t *self, song_handle_t handle, int priority)
{
	song_t *song = song_lib_find(self->songlib, handle);
	ASSERT_SONG(song);
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] Renicing song %08lx to %d\n",
		handle, priority);
#endif

	song->priority = priority;

	_update(self);
}

void
sfx_song_set_loops(sfx_state_t *self, song_handle_t handle, int loops)
{
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

void
sfx_song_set_hold(sfx_state_t *self, song_handle_t handle, int hold)
{
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
				    3, 3, 0, 3, 2, 0, 3, 0};

static const song_handle_t midi_send_base = 0xffff0000;

static song_handle_t midi_send_handle = 0xffff0000;

int
sfx_send_midi(sfx_state_t *self, song_handle_t handle, int channel,
	      int command, int arg1, int arg2)
{
	byte buffer[5];
	tell_synth_func *tell = sfx_get_player_tell_func();

	/* Yes, in that order. SCI channel mutes are actually done via
	   a counting semaphore. 0 means to decrement the counter, 1
	   to increment it. */
	static char *channel_state[] = {"ON","OFF"}; 

	if (command == 0xb0 &&
	    arg1 == SCI_MIDI_CHANNEL_MUTE)
	{
		sciprintf("TODO: channel mute (channel %d %s)!\n", channel,
			  channel_state[arg2]);
		/* We need to have a GET_PLAYMASK interface to use
		   here. SET_PLAYMASK we've got.
		*/
		return SFX_OK;
	}   

	buffer[0] = channel | command; /* No channel remapping yet */

	switch (command)
	{
	case 0x80 :
	case 0x90 :
	case 0xb0 :
		buffer[1] = arg1&0xff;
		buffer[2] = arg2&0xff;
		break;
	case 0xc0 :
		buffer[1] = arg1&0xff;
		break;
	case 0xe0 :
		buffer[1] = (arg1&0x7f) | 0x80;
		buffer[2] = (arg1&0xff00) >> 7;
		break;
	default:
		sciprintf("Unexpected explicit MIDI command %02x\n", command);
		return SFX_ERROR;
	}

	if (tell)
		tell(MIDI_cmdlen[command >> 4], buffer);
	return SFX_OK;
}

int
sfx_get_volume(sfx_state_t *self)
{
	fprintf(stderr, "FIXME: Implement volume\n");
	return 0;
}

void
sfx_set_volume(sfx_state_t *self, int volume)
{
	fprintf(stderr, "FIXME: Implement volume\n");
}

void
sfx_all_stop(sfx_state_t *self)
{
#ifdef DEBUG_SONG_API
	fprintf(stderr, "[sfx-core] All stop\n");
#endif

	song_lib_free(self->songlib);
	_update(self);
}
