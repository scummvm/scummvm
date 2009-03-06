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

/* Song library */

#ifndef SCI_SFX_SFX_SONGLIB_H
#define SCI_SFX_SFX_SONGLIB_H

#include "common/scummsys.h"

namespace Sci {

class SongIterator;

#define SOUND_STATUS_STOPPED   0
#define SOUND_STATUS_PLAYING   1
#define SOUND_STATUS_SUSPENDED 2
/* suspended: only if ordered from kernel space */
#define SOUND_STATUS_WAITING   3
/* "waiting" means "tagged for playing, but not active right now" */

typedef unsigned long song_handle_t;

enum RESTORE_BEHAVIOR {
	RESTORE_BEHAVIOR_CONTINUE, /* restart a song when restored from
				     a saved game */
	RESTORE_BEHAVIOR_RESTART /* continue it from where it was */
};

struct song_t {
	song_handle_t handle;
	int resource_num; /* Resource number */
	int priority; /* Song priority (more important if priority is higher) */
	int status;   /* See above */

	int restore_behavior;
	int restore_time;

	/* Grabbed from the sound iterator, for save/restore purposes */
	int loops;
	int hold;

	SongIterator *it;
	long delay; /* Delay before accessing the iterator, in microseconds */

	uint32 wakeup_time; /* Used by the sound core:
			      ** Playing -> time at which 'delay' has elapsed
			      ** Suspended/Waiting -> stopping time */

	song_t *next; /* Next song or NULL if this is the last one */
	song_t *next_playing; /* Next playing song; used by the
				    ** core song system */
	song_t *next_stopping; /* Next song pending stopping; used exclusively by
				     ** the core song system's _update_multi_song() */
};


struct songlib_t {
	song_t **lib;
	song_t *_s;
};

/**************************/
/* Song library commands: */
/**************************/

song_t *song_new(song_handle_t handle, SongIterator *it, int priority);
/* Initializes a new song
** Parameters: (song_handle_t) handle: The sound handle
**             (SongIterator *) it: The song
**             (int) priority: The song's priority
** Returns   : (song_t *) A freshly allocated song
** Other values are set to predefined defaults.
*/


void song_lib_init(songlib_t *songlib);
/* Initializes a static song library
** Parameters: (songlib_t *) songlib: Pointer to the library
**             to initialize
** Returns   : (void)
*/

void song_lib_free(songlib_t songlib);
/* Frees a song library
** Parameters: (songlib_t) songlib: The library to free
** Returns   : (void)
*/

void song_lib_add(songlib_t songlib, song_t *song);
/* Adds a song to a song library.
** Parameters: (songlib_t) songlib: An existing sound library, or NULL
**             (song_t *) song: The song to add
** Returns   : (void)
*/

song_t *song_lib_find(songlib_t songlib, song_handle_t handle);
/* Looks up the song with the specified handle
** Parameters: (songlib_t) songlib: An existing sound library, may point to NULL
**             (song_handle_t) handle: The sound handle to look for
** Returns   : (song_t *) The song or NULL if it wasn't found
*/

song_t *song_lib_find_active(songlib_t songlib);
/* Finds the first song playing with the highest priority
** Parameters: (songlib_t) songlib: An existing sound library
** Returns   : (song_t *) The song that should be played next, or NULL if there is none
*/

song_t *song_lib_find_next_active(songlib_t songlib, song_t *song);
/* Finds the next song playing with the highest priority
** Parameters: (songlib_t) songlib: The song library to operate on
**             (song_t *) song: A song previously returned from the song library
** Returns   : (song_t *) The next song to play relative to 'song', or
**                        NULL if none are left
** The functions 'song_lib_find_active' and 'song_lib_find_next_active
** allow to iterate over all songs that satisfy the requirement of
** being 'playable'.
*/

int song_lib_remove(songlib_t songlib, song_handle_t handle);
/* Removes a song from the library
** Parameters: (songlib_t) songlib: An existing sound library
**             (song_handle_t) handle: Handle of the song to remove
** Returns   : (int) The status of the song that was removed
*/

void song_lib_resort(songlib_t songlib, song_t *song);
/* Removes a song from the library and sorts it in again; for use after renicing
** Parameters: (songlib_t) songlib: An existing sound library
**             (song_t *) song: The song to work on
** Returns   : (void)
*/

int song_lib_count(songlib_t songlib);
/* Counts the number of songs in a song library
** Parameters: (songlib_t) songlib: The library to count
** Returns   : (int) The number of songs
*/

void song_lib_set_restore_behavior(songlib_t songlib, song_handle_t handle,
	RESTORE_BEHAVIOR action);
/* Determines what should be done with the song "handle" when
** restoring it from a saved game.
** Parameters: (songlib_t) songlib: The library that contains the song
**             (song_handle_t) handle: Its handle
**             (RESTORE_BEHAVIOR) action: The desired action
*/

} // End of namespace Sci

#endif // SCI_SSFX_SFX_SONGLIB_H
