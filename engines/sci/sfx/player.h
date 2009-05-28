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

/* song player structure */

#ifndef SCI_SFX_SFX_PLAYER_H
#define SCI_SFX_SFX_PLAYER_H

#include "common/scummsys.h"

#include "sci/resource.h"
#include "sci/sfx/iterator.h"

namespace Sci {

class SfxPlayer {
public:
	/** Number of voices that can play simultaneously */
	int _polyphony;

public:
	SfxPlayer() : _polyphony(0) {}
	virtual ~SfxPlayer() {}

	virtual Common::Error init(ResourceManager *resmgr, int expected_latency) = 0;
	/* Initializes the player
	** Parameters: (ResourceManager *) resmgr: A resource manager for driver initialization
	**             (int) expected_latency: Expected delay in between calls to 'maintenance'
	**                   (in microseconds)
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	*/

	virtual Common::Error add_iterator(SongIterator *it, uint32 start_time) = 0;
	/* Adds an iterator to the song player
	** Parameters: (songx_iterator_t *) it: The iterator to play
	**             (uint32) start_time: The time to assume as the
	**                        time the first MIDI command executes at
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	** The iterator should not be cloned (to avoid memory leaks) and
	** may be modified according to the needs of the player.
	** Implementors may use the 'sfx_iterator_combine()' function
	** to add iterators onto their already existing iterators
	*/

	virtual Common::Error stop() = 0;
	/* Stops the currently playing song and deletes the associated iterator
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	*/

	virtual Common::Error iterator_message(const SongIterator::Message &msg) = 0;
	/* Transmits a song iterator message to the active song
	** Parameters: (SongIterator::Message) msg: The message to transmit
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	** OPTIONAL -- may be NULL
	** If this method is not present, sending messages will stop
	** and re-start playing, so it is preferred that it is present
	*/

	virtual Common::Error pause() = 0;
	/* Pauses song playing
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	*/

	virtual Common::Error resume() = 0;
	/* Resumes song playing after a pause
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	*/

	virtual Common::Error exit() = 0;
	/* Stops the player
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError on failure
	*/

	virtual void tell_synth(int buf_nr, byte *buf) = 0;
	/* Pass a raw MIDI event to the synth
	Parameters: (int) argc: Length of buffer holding the midi event
	           (byte *) argv: The buffer itself
	*/
};

int sfx_get_player_polyphony();
/* Determines the polyphony of the player in use
** Returns   : (int) Number of voices the active player can emit
*/

void sfx_reset_player();
/* Tells the player to stop its internal iterator
** Parameters: None.
** Returns: Nothing.
 */

SongIterator *sfx_iterator_combine(SongIterator *it1, SongIterator *it2);
/* Combines two song iterators into one
** Parameters: (sfx_iterator_t *) it1: One of the two iterators, or NULL
**             (sfx_iterator_t *) it2: The other iterator, or NULL
** Returns   : (sfx_iterator_t *) A combined iterator
** If a combined iterator is returned, it will be flagged to be allowed to
** dispose of 'it1' and 'it2', where applicable. This means that this
** call should be used by song players, but not by the core sound system
*/

} // End of namespace Sci

#endif // SCI_SFX_SFX_PLAYER_H
