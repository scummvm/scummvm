/***************************************************************************
 sfx_songlib.h (C) 2002..04 Christoph Reichenbach

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

		Christoph Reichenbach (CJR) [reichenb@colorado.edu]

***************************************************************************/
/* Song library */

#ifndef _SCI_SFX_SONGLIB_H_
#define _SCI_SFX_SONGLIB_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <scitypes.h>
#include <sfx_iterator.h>

#define SOUND_STATUS_STOPPED   0
#define SOUND_STATUS_PLAYING   1
#define SOUND_STATUS_SUSPENDED 2
/* suspended: only if ordered from kernel space */
#define SOUND_STATUS_WAITING   3
/* "waiting" means "tagged for playing, but not active right now" */

typedef unsigned long song_handle_t;

typedef enum {
	RESTORE_BEHAVIOR_CONTINUE, /* restart a song when restored from
				     a saved game */
	RESTORE_BEHAVIOR_RESTART /* continue it from where it was */
} RESTORE_BEHAVIOR;

typedef struct _song {
	song_handle_t handle;
	int resource_num; /* Resource number */
	int priority; /* Song priority (more important if priority is higher) */
	int status;   /* See above */

	int restore_behavior;
	int restore_time;

/* Grabbed from the sound iterator, for save/restore purposes */
	int loops; 
	int hold; 

	song_iterator_t *it;
	long delay; /* Delay before accessing the iterator, in microseconds */

	GTimeVal wakeup_time; /* Used by the sound core:
			      ** Playing -> time at which 'delay' has elapsed
			      ** Suspended/Waiting -> stopping time */

	struct _song *next; /* Next song or NULL if this is the last one */
	struct _song *next_playing; /* Next playing song; used by the
				    ** core song system */
	struct _song *next_stopping; /* Next song pending stopping; used exclusively by
				     ** the core song system's _update_multi_song() */
} song_t;


typedef struct {
	song_t **lib;
	song_t *_s;
} songlib_t;

/**************************/
/* Song library commands: */
/**************************/

song_t *
song_new(song_handle_t handle, song_iterator_t *it, int priority);
/* Initializes a new song
** Parameters: (song_handle_t) handle: The sound handle
**             (song_iterator_t *) it: The song
**             (int) priority: The song's priority
** Returns   : (song_t *) A freshly allocated song
** Other values are set to predefined defaults.
*/


void
song_lib_init(songlib_t *songlib);
/* Initializes a static song library
** Parameters: (songlib_t *) songlib: Pointer to the library
**             to initialize
** Returns   : (void)
*/

void
song_lib_free(songlib_t songlib);
/* Frees a song library
** Parameters: (songlib_t) songlib: The library to free
** Returns   : (void)
*/

void
song_lib_add(songlib_t songlib, song_t *song);
/* Adds a song to a song library.
** Parameters: (songlib_t) songlib: An existing sound library, or NULL
**             (song_t *) song: The song to add
** Returns   : (void)
*/

song_t *
song_lib_find(songlib_t songlib, song_handle_t handle);
/* Looks up the song with the specified handle
** Parameters: (songlib_t) songlib: An existing sound library, may point to NULL
**             (song_handle_t) handle: The sound handle to look for
** Returns   : (song_t *) The song or NULL if it wasn't found
*/

song_t *
song_lib_find_active(songlib_t songlib);
/* Finds the first song playing with the highest priority
** Parameters: (songlib_t) songlib: An existing sound library
** Returns   : (song_t *) The song that should be played next, or NULL if there is none
*/

song_t *
song_lib_find_next_active(songlib_t songlib, song_t *song);
/* Finds the next song playing with the highest priority
** Parameters: (songlib_t) songlib: The song library to operate on
**             (song_t *) song: A song previously returned from the song library
** Returns   : (song_t *) The next song to play relative to 'song', or
**                        NULL if none are left
** The functions 'song_lib_find_active' and 'song_lib_find_next_active
** allow to iterate over all songs that satisfy the requirement of
** being 'playable'.
*/

int
song_lib_remove(songlib_t songlib, song_handle_t handle);
/* Removes a song from the library
** Parameters: (songlib_t) songlib: An existing sound library
**             (song_handle_t) handle: Handle of the song to remove
** Returns   : (int) The status of the song that was removed
*/

void
song_lib_resort(songlib_t songlib, song_t *song);
/* Removes a song from the library and sorts it in again; for use after renicing
** Parameters: (songlib_t) songlib: An existing sound library
**             (song_t *) song: The song to work on
** Returns   : (void)
*/

int
song_lib_count(songlib_t songlib);
/* Counts the number of songs in a song library
** Parameters: (songlib_t) songlib: The library to count
** Returns   : (int) The number of songs
*/

GTimeVal
song_sleep_time(GTimeVal *lastslept, long ticks);
/* Caluculates the amount of seconds and microseconds to sleep.
** Parameters: (GTimeVal *) lastslept: The time to start counting on
**             (long) ticks: Number of ticks to sleep
** Returns   : (GTimeVal) The amount of time to sleep
*/

GTimeVal
song_next_wakeup_time(GTimeVal *lastslept, long ticks);
/* Calculates the time at which "ticks" have passed, counting from "lastslept".
** Parameters: (GTimeVal *) lastslept: The base to start counting on
**             (long) ticks: Number of ticks to count
** Returns   : (GTimeVal) A structure describing the time at which the
**                              specified number of ticks has passed
*/

void
song_lib_set_restore_behavior(songlib_t songlib, song_handle_t handle, 
			      RESTORE_BEHAVIOR action);
/* Determines what should be done with the song "handle" when
** restoring it from a saved game.
** Parameters: (songlib_t) songlib: The library that contains the song
**             (song_handle_t) handle: Its handle
**             (RESTORE_BEHAVIOR) action: The desired action
*/

#endif /* !_SCI_SOUND_SERVER_H_ */
