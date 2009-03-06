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
#include "sci/sci_memory.h"

namespace Sci {

#define debug_stream stderr

song_t *song_new(song_handle_t handle, SongIterator *it, int priority) {
	song_t *retval;
	retval = (song_t*) sci_malloc(sizeof(song_t));

#ifdef SATISFY_PURIFY
	memset(retval, 0, sizeof(song_t));
#endif

	retval->handle = handle;
	retval->priority = priority;
	retval->next = NULL;
	retval->delay = 0;
	retval->it = it;
	retval->status = SOUND_STATUS_STOPPED;
	retval->next_playing = NULL;
	retval->next_stopping = NULL;
	retval->restore_behavior = RESTORE_BEHAVIOR_CONTINUE;
	retval->restore_time = 0;

	return retval;
}

void song_lib_add(songlib_t songlib, song_t *song) {
	song_t **seeker = NULL;
	int pri	= song->priority;

	if (NULL == song) {
		sciprintf("song_lib_add(): NULL passed for song\n");
		return;
	}

	if (*(songlib.lib) == NULL) {
		*(songlib.lib) = song;
		song->next = NULL;

		return;
	}

	seeker = (songlib.lib);
	while (*seeker && ((*seeker)->priority > pri))
		seeker = &((*seeker)->next);

	song->next = *seeker;
	*seeker = song;
}

static void _songfree_chain(song_t *song) {
	/* Recursively free a chain of songs */
	if (song) {
		_songfree_chain(song->next);
		delete song->it;
		song->it = NULL;
		free(song);
	}
}

void song_lib_init(songlib_t *songlib) {
	songlib->lib = &(songlib->_s);
	songlib->_s = NULL;
}

void song_lib_free(songlib_t songlib) {
	_songfree_chain(*(songlib.lib));
	*(songlib.lib) = NULL;
}


song_t *song_lib_find(songlib_t songlib, song_handle_t handle) {
	song_t *seeker = *(songlib.lib);

	while (seeker) {
		if (seeker->handle == handle)
			break;
		seeker = seeker->next;
	}

	return seeker;
}

song_t *song_lib_find_next_active(songlib_t songlib, song_t *other) {
	song_t *seeker = other ? other->next : *(songlib.lib);

	while (seeker) {
		if ((seeker->status == SOUND_STATUS_WAITING) ||
		        (seeker->status == SOUND_STATUS_PLAYING))
			break;
		seeker = seeker->next;
	}

	/* Only return songs that have equal priority */
	if (other && seeker && other->priority > seeker->priority)
		return NULL;

	return seeker;
}

song_t *song_lib_find_active(songlib_t songlib) {
	return song_lib_find_next_active(songlib, NULL);
}

int song_lib_remove(songlib_t songlib, song_handle_t handle) {
	int retval;
	song_t *goner = *(songlib.lib);

	if (!goner)
		return -1;

	if (goner->handle == handle)
		*(songlib.lib) = goner->next;

	else {
		while ((goner->next) && (goner->next->handle != handle))
			goner = goner->next;

		if (goner->next) { /* Found him? */
			song_t *oldnext = goner->next;

			goner->next = goner->next->next;
			goner = oldnext;
		} else return -1; /* No. */
	}

	retval = goner->status;

	delete goner->it;
	free(goner);

	return retval;
}

void song_lib_resort(songlib_t songlib, song_t *song) {
	if (*(songlib.lib) == song)
		*(songlib.lib) = song->next;
	else {
		song_t *seeker = *(songlib.lib);

		while (seeker->next && (seeker->next != song))
			seeker = seeker->next;

		if (seeker->next)
			seeker->next = seeker->next->next;
	}

	song_lib_add(songlib, song);
}

int song_lib_count(songlib_t songlib) {
	song_t *seeker = *(songlib.lib);
	int retval = 0;

	while (seeker) {
		retval++;
		seeker = seeker->next;
	}

	return retval;
}

void song_lib_set_restore_behavior(songlib_t songlib, song_handle_t handle, RESTORE_BEHAVIOR action) {
	song_t *seeker = song_lib_find(songlib, handle);

	seeker->restore_behavior = action;
}

void song_lib_dump(songlib_t songlib, int line) {
	song_t *seeker = *(songlib.lib);

	fprintf(debug_stream, "L%d:", line);
	do {
		fprintf(debug_stream, "    %p", (void *)seeker);

		if (seeker) {
			fprintf(debug_stream, "[%04lx,p=%d,s=%d]->", seeker->handle, seeker->priority, seeker->status);
			seeker = seeker->next;
		}
		fprintf(debug_stream, "\n");
	} while (seeker);
	fprintf(debug_stream, "\n");

}

} // End of namespace Sci
