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

/* Sound engine */
#ifndef SCI_SFX_SFX_ENGINE_H
#define SCI_SFX_SFX_ENGINE_H

#include "sci/sfx/sfx_core.h"
#include "sci/sfx/sfx_songlib.h"
#include "sci/sfx/sfx_iterator.h"
#include "sci/scicore/resource.h"

namespace Sci {

#define SOUND_TICK 1000 / 60
/* Approximately 17 milliseconds */


#define SFX_STATE_FLAG_MULTIPLAY (1 << 0) /* More than one song playable
** simultaneously ? */
#define SFX_STATE_FLAG_NOSOUND	 (1 << 1) /* Completely disable sound playing */


#define SFX_DEBUG_SONGS		(1 << 0) /* Debug song changes */
#define SFX_DEBUG_CUES		(1 << 1) /* Debug cues, loops, and
** song completions */

struct sfx_state_t {
	song_iterator_t *it; /* The song iterator at the heart of things */
	unsigned int flags; /* SFX_STATE_FLAG_* */
	songlib_t songlib; /* Song library */
	song_t *song; /* Active song, or start of active song chain */
	int suspended; /* Whether we are suspended */
	unsigned int debug; /* Debug flags */

};

/***********/
/* General */
/***********/

void sfx_init(sfx_state_t *self, ResourceManager *resmgr, int flags);
/* Initializes the sound engine
** Parameters: (ResourceManager *) resmgr: Resource manager for initialization
**             (int) flags: SFX_STATE_FLAG_*
*/

void sfx_exit(sfx_state_t *self);
/* Deinitializes the sound subsystem
*/

void sfx_suspend(sfx_state_t *self, int suspend);
/* Suspends/unsuspends the sound sybsystem
** Parameters: (int) suspend: Whether to suspend (non-null) or to unsuspend
*/

int sfx_poll(sfx_state_t *self, song_handle_t *handle, int *cue);
/* Polls the sound server for cues etc.
** Returns   : (int) 0 if the cue queue is empty, SI_LOOP, SI_CUE, or SI_FINISHED otherwise
**             (song_handle_t) *handle: The affected handle
**             (int) *cue: The sound cue number (if SI_CUE), or the loop number (if SI_LOOP)
*/

int sfx_poll_specific(sfx_state_t *self, song_handle_t handle, int *cue);
/* Polls the sound server for cues etc.
** Parameters: (song_handle_t) handle: The handle to poll
** Returns   : (int) 0 if the cue queue is empty, SI_LOOP, SI_CUE, or SI_FINISHED otherwise
**             (int) *cue: The sound cue number (if SI_CUE), or the loop number (if SI_LOOP)
*/

int sfx_get_volume(sfx_state_t *self);
/* Determines the current global volume settings
** Returns   : (int) The global volume, between 0 (silent) and 127 (max. volume)
*/

void sfx_set_volume(sfx_state_t *self, int volume);
/* Determines the current global volume settings
** Parameters: (int) volume: The new global volume, between 0 and 127 (see above)
*/

void sfx_all_stop(sfx_state_t *self);
/* Stops all songs currently playing, purges song library
*/


/*****************/
/*  Song basics  */
/*****************/

int sfx_add_song(sfx_state_t *self, song_iterator_t *it, int priority, song_handle_t handle, int resnum);
/* Adds a song to the internal sound library
** Parameters: (song_iterator_t *) it: The iterator describing the song
**             (int) priority: Initial song priority (higher <-> more important)
**             (song_handle_t) handle: The handle to associate with the song
** Returns   : (int) 0 on success, nonzero on error
*/


void sfx_remove_song(sfx_state_t *self, song_handle_t handle);
/* Deletes a song and its associated song iterator from the song queue
** Parameters: (song_handle_t) handle: The song to remove
*/


/**********************/
/* Song modifications */
/**********************/


void sfx_song_set_status(sfx_state_t *self, song_handle_t handle, int status);
/* Sets the song status, i.e. whether it is playing, suspended, or stopped.
** Parameters: (song_handle_t) handle: Handle of the song to modify
**             (int) status: The song status the song should assume
** WAITING and PLAYING are set implicitly and essentially describe the same state
** as far as this function is concerned.
*/

void sfx_song_renice(sfx_state_t *self, song_handle_t handle, int priority);
/* Sets the new song priority
** Parameters: (song_handle_t) handle: The handle to modify
**             (int) priority: The priority to set
*/

void sfx_song_set_loops(sfx_state_t *self, song_handle_t handle, int loops);
/* Sets the number of loops for the specified song
** Parameters: (song_handle_t) handle: The song handle to reference
**             (int) loops: Number of loops to set
*/

void sfx_song_set_hold(sfx_state_t *self, song_handle_t handle, int hold);
/* Sets the number of loops for the specified song
** Parameters: (song_handle_t) handle: The song handle to reference
**             (int) hold: Number of loops to setn
*/

void sfx_song_set_fade(sfx_state_t *self, song_handle_t handle, fade_params_t *fade_setup);
/* Instructs a song to be faded out
** Parameters: (song_handle_t) handle: The song handle to reference
**             (fade_params_t *) fade_setup: The precise fade-out configuration to use
*/

} // End of namespace Sci

#endif // SCI_SFX_SFX_ENGINE_H
