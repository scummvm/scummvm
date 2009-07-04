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

class Song {
public:
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

public:

	Song();

	/**
	 * Initializes a new song.
	 * @param handle	the sound handle
	 * @param it		the song
	 * @param priority	the song's priority
	 * @return a freshly allocated song
	 */
	Song(SongHandle handle, SongIterator *it, int priority);
};


class SongLibrary {
public:
	Song *_lib;

public:
	SongLibrary() : _lib(0) {}

	/** Frees a song library. */
	void freeSounds();

	/**
	 * Adds a song to a song library.
	 * @param song		song to add
	 */
	void addSong(Song *song);

	/**
	 * Looks up the song with the specified handle.
	 * @param handle	sound handle to look for
	 * @return the song or NULL if it wasn't found
	 */
	Song *findSong(SongHandle handle);

	/**
	 * Finds the first song playing with the highest priority.
	 * @return the song that should be played next, or NULL if there is none
	 */
	Song *findFirstActive();

	/**
	 * Finds the next song playing with the highest priority.
	 *
	 * The functions 'findFirstActive' and 'findNextActive'
	 * allow to iterate over all songs that satisfy the requirement of
	 * being 'playable'.
	 *
	 * @param song		a song previously returned from the song library
	 * @return the next song to play relative to 'song', or NULL if none are left
	 */
	Song *findNextActive(Song *song);

	/**
	 * Removes a song from the library.
	 * @param handle		handle of the song to remove
	 * @return the status of the song that was removed
	 */
	int removeSong(SongHandle handle);

	/**
	 * Removes a song from the library and sorts it in again; for use after renicing.
	 * @param son		song to work on
	 */
	void resortSong(Song *song);

	/**
	 * Counts the number of songs in a song library.
	 * @return the number of songs
	 */
	int countSongs();

	/**
	 * Determines what should be done with the song "handle" when restoring
	 * it from a saved game.
	 * @param handle	sound handle being restored
	 * @param action	desired action
	 */
	void setSongRestoreBehavior(SongHandle handle,
		RESTORE_BEHAVIOR action);
};

} // End of namespace Sci

#endif // SCI_SSFX_SFX_SONGLIB_H
