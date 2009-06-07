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
#include "sound/timestamp.h"

namespace Sci {

class SongIterator;

#define SOUND_STATUS_STOPPED   0
#define SOUND_STATUS_PLAYING   1
#define SOUND_STATUS_SUSPENDED 2
/* suspended: only if ordered from kernel space */
#define SOUND_STATUS_WAITING   3
/* "waiting" means "tagged for playing, but not active right now" */

typedef unsigned long SongHandle;

enum RESTORE_BEHAVIOR {
	RESTORE_BEHAVIOR_CONTINUE, /* restart a song when restored from
				     a saved game */
	RESTORE_BEHAVIOR_RESTART /* continue it from where it was */
};

struct Song {
	SongHandle _handle;
	int _resourceNum; /**<! Resource number */
	int _priority; /**!< Song priority (more important if priority is higher) */
	int _status;   /* See above */

	int _restoreBehavior;
	int _restoreTime;

	/* Grabbed from the sound iterator, for save/restore purposes */
	int _loops;
	int _hold;

	SongIterator *_it;
	int _delay; /**!< Delay before accessing the iterator, in ticks */

	Audio::Timestamp _wakeupTime; /**!< Timestamp indicating the next MIDI event */

	Song *_next; /**!< Next song or NULL if this is the last one */

	/**
	 * Next playing song. Used by the core song system.
	 */
	Song *_nextPlaying;

	/**
	 * Next song pending stopping. Used exclusively by the core song system's
	 * _update_multi_song()
	 */
	Song *_nextStopping;
};


struct SongLibrary {
	Song **_lib;
	Song *_s;
};

/**************************/
/* Song library commands: */
/**************************/

/* Initializes a new song
** Parameters: (SongHandle) handle: The sound handle
**             (SongIterator *) it: The song
**             (int) priority: The song's priority
** Returns   : (Song *) A freshly allocated song
** Other values are set to predefined defaults.
*/
Song *song_new(SongHandle handle, SongIterator *it, int priority);


/* Initializes a static song library
** Parameters: (SongLibrary *) songlib: Pointer to the library
**             to initialize
** Returns   : (void)
*/
void song_lib_init(SongLibrary *songlib);

/* Frees a song library
** Parameters: (SongLibrary) songlib: The library to free
** Returns   : (void)
*/
void song_lib_free(const SongLibrary &songlib);

/* Adds a song to a song library.
** Parameters: (SongLibrary) songlib: An existing sound library, or NULL
**             (Song *) song: The song to add
** Returns   : (void)
*/
void song_lib_add(const SongLibrary &songlib, Song *song);

/* Looks up the song with the specified handle
** Parameters: (SongLibrary) songlib: An existing sound library, may point to NULL
**             (SongHandle) handle: The sound handle to look for
** Returns   : (Song *) The song or NULL if it wasn't found
*/
Song *song_lib_find(const SongLibrary &songlib, SongHandle handle);

/* Finds the first song playing with the highest priority
** Parameters: (SongLibrary) songlib: An existing sound library
** Returns   : (Song *) The song that should be played next, or NULL if there is none
*/
Song *song_lib_find_active(const SongLibrary &songlib);

/* Finds the next song playing with the highest priority
** Parameters: (SongLibrary) songlib: The song library to operate on
**             (Song *) song: A song previously returned from the song library
** Returns   : (Song *) The next song to play relative to 'song', or
**                        NULL if none are left
** The functions 'song_lib_find_active' and 'song_lib_find_next_active
** allow to iterate over all songs that satisfy the requirement of
** being 'playable'.
*/
Song *song_lib_find_next_active(const SongLibrary &songlib, Song *song);

/* Removes a song from the library
** Parameters: (SongLibrary) songlib: An existing sound library
**             (SongHandle) handle: Handle of the song to remove
** Returns   : (int) The status of the song that was removed
*/
int song_lib_remove(const SongLibrary &songlib, SongHandle handle);

/* Removes a song from the library and sorts it in again; for use after renicing
** Parameters: (SongLibrary) songlib: An existing sound library
**             (Song *) song: The song to work on
** Returns   : (void)
*/
void song_lib_resort(const SongLibrary &songlib, Song *song);

/* Counts the number of songs in a song library
** Parameters: (SongLibrary) songlib: The library to count
** Returns   : (int) The number of songs
*/
int song_lib_count(const SongLibrary &songlib);

/* Determines what should be done with the song "handle" when
** restoring it from a saved game.
** Parameters: (SongLibrary) songlib: The library that contains the song
**             (SongHandle) handle: Its handle
**             (RESTORE_BEHAVIOR) action: The desired action
*/
void song_lib_set_restore_behavior(const SongLibrary &songlib, SongHandle handle,
	RESTORE_BEHAVIOR action);

} // End of namespace Sci

#endif // SCI_SSFX_SFX_SONGLIB_H
