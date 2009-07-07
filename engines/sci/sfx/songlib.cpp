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

Song::Song() {
	_handle = 0;
	_priority = 0;
	_status = SOUND_STATUS_STOPPED;

	_restoreBehavior = RESTORE_BEHAVIOR_CONTINUE;
	_restoreTime = 0;

	_loops = 0;
	_hold = 0;

	_it = 0;
	_delay = 0;

	_next = NULL;
	_nextPlaying = NULL;
	_nextStopping = NULL;
}

Song::Song(SongHandle handle, SongIterator *it, int priority) {
	_handle = handle;
	_priority = priority;
	_status = SOUND_STATUS_STOPPED;

	_restoreBehavior = RESTORE_BEHAVIOR_CONTINUE;
	_restoreTime = 0;

	_loops = 0;
	_hold = 0;

	_it = it;
	_delay = 0;

	_next = NULL;
	_nextPlaying = NULL;
	_nextStopping = NULL;
}

void SongLibrary::addSong(Song *song) {
	Song **seeker = NULL;
	int pri	= song->_priority;

	if (NULL == song) {
		warning("addSong(): NULL passed for song");
		return;
	}

	seeker = &_lib;
	while (*seeker && ((*seeker)->_priority > pri))
		seeker = &((*seeker)->_next);

	song->_next = *seeker;
	*seeker = song;
}

void SongLibrary::freeSounds() {
	Song *next = _lib;
	while (next) {
		Song *song = next;
		delete song->_it;
		song->_it = NULL;
		next = song->_next;
		delete song;
	}
	_lib = NULL;
}


Song *SongLibrary::findSong(SongHandle handle) {
	Song *seeker = _lib;

	while (seeker) {
		if (seeker->_handle == handle)
			break;
		seeker = seeker->_next;
	}

	return seeker;
}

Song *SongLibrary::findNextActive(Song *other) {
	Song *seeker = other ? other->_next : _lib;

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

Song *SongLibrary::findFirstActive() {
	return findNextActive(NULL);
}

int SongLibrary::removeSong(SongHandle handle) {
	int retval;
	Song *goner = _lib;

	if (!goner)
		return -1;

	if (goner->_handle == handle)
		_lib = goner->_next;

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
	delete goner;

	return retval;
}

void SongLibrary::resortSong(Song *song) {
	if (_lib == song)
		_lib = song->_next;
	else {
		Song *seeker = _lib;

		while (seeker->_next && (seeker->_next != song))
			seeker = seeker->_next;

		if (seeker->_next)
			seeker->_next = seeker->_next->_next;
	}

	addSong(song);
}

int SongLibrary::countSongs() {
	Song *seeker = _lib;
	int retval = 0;

	while (seeker) {
		retval++;
		seeker = seeker->_next;
	}

	return retval;
}

void SongLibrary::setSongRestoreBehavior(SongHandle handle, RESTORE_BEHAVIOR action) {
	Song *seeker = findSong(handle);

	seeker->_restoreBehavior = action;
}

} // End of namespace Sci
