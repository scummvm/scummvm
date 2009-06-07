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

#include "sci/tools.h"
#include "sci/sfx/core.h"
#include "sci/sfx/iterator.h"

namespace Sci {

#define debug_stream stderr

Song *song_new(SongHandle handle, SongIterator *it, int priority) {
	Song *retval;
	retval = (Song *)malloc(sizeof(Song));

#ifdef SATISFY_PURIFY
	memset(retval, 0, sizeof(Song));
#endif

	retval->_handle = handle;
	retval->_priority = priority;
	retval->_next = NULL;
	retval->_delay = 0;
	retval->_wakeupTime = Audio::Timestamp();
	retval->_it = it;
	retval->_status = SOUND_STATUS_STOPPED;
	retval->_nextPlaying = NULL;
	retval->_nextStopping = NULL;
	retval->_restoreBehavior = RESTORE_BEHAVIOR_CONTINUE;
	retval->_restoreTime = 0;

	return retval;
}

void song_lib_add(const SongLibrary &songlib, Song *song) {
	Song **seeker = NULL;
	int pri	= song->_priority;

	if (NULL == song) {
		sciprintf("song_lib_add(): NULL passed for song\n");
		return;
	}

	if (*(songlib._lib) == NULL) {
		*(songlib._lib) = song;
		song->_next = NULL;

		return;
	}

	seeker = (songlib._lib);
	while (*seeker && ((*seeker)->_priority > pri))
		seeker = &((*seeker)->_next);

	song->_next = *seeker;
	*seeker = song;
}

static void _songfree_chain(Song *song) {
	/* Recursively free a chain of songs */
	if (song) {
		_songfree_chain(song->_next);
		delete song->_it;
		song->_it = NULL;
		free(song);
	}
}

void song_lib_init(SongLibrary *songlib) {
	songlib->_lib = &(songlib->_s);
	songlib->_s = NULL;
}

void song_lib_free(const SongLibrary &songlib) {
	_songfree_chain(*(songlib._lib));
	*(songlib._lib) = NULL;
}


Song *song_lib_find(const SongLibrary &songlib, SongHandle handle) {
	Song *seeker = *(songlib._lib);

	while (seeker) {
		if (seeker->_handle == handle)
			break;
		seeker = seeker->_next;
	}

	return seeker;
}

Song *song_lib_find_next_active(const SongLibrary &songlib, Song *other) {
	Song *seeker = other ? other->_next : *(songlib._lib);

	while (seeker) {
		if ((seeker->_status == SOUND_STATUS_WAITING) ||
		        (seeker->_status == SOUND_STATUS_PLAYING))
			break;
		seeker = seeker->_next;
	}

	/* Only return songs that have equal priority */
	if (other && seeker && other->_priority > seeker->_priority)
		return NULL;

	return seeker;
}

Song *song_lib_find_active(const SongLibrary &songlib) {
	return song_lib_find_next_active(songlib, NULL);
}

int song_lib_remove(const SongLibrary &songlib, SongHandle handle) {
	int retval;
	Song *goner = *(songlib._lib);

	if (!goner)
		return -1;

	if (goner->_handle == handle)
		*(songlib._lib) = goner->_next;

	else {
		while ((goner->_next) && (goner->_next->_handle != handle))
			goner = goner->_next;

		if (goner->_next) { /* Found him? */
			Song *oldnext = goner->_next;

			goner->_next = goner->_next->_next;
			goner = oldnext;
		} else return -1; /* No. */
	}

	retval = goner->_status;

	delete goner->_it;
	free(goner);

	return retval;
}

void song_lib_resort(const SongLibrary &songlib, Song *song) {
	if (*(songlib._lib) == song)
		*(songlib._lib) = song->_next;
	else {
		Song *seeker = *(songlib._lib);

		while (seeker->_next && (seeker->_next != song))
			seeker = seeker->_next;

		if (seeker->_next)
			seeker->_next = seeker->_next->_next;
	}

	song_lib_add(songlib, song);
}

int song_lib_count(const SongLibrary &songlib) {
	Song *seeker = *(songlib._lib);
	int retval = 0;

	while (seeker) {
		retval++;
		seeker = seeker->_next;
	}

	return retval;
}

void song_lib_set_restore_behavior(const SongLibrary &songlib, SongHandle handle, RESTORE_BEHAVIOR action) {
	Song *seeker = song_lib_find(songlib, handle);

	seeker->_restoreBehavior = action;
}

} // End of namespace Sci
