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


#ifndef SCI_SFX_SEQUENCER_H
#define SCI_SFX_SEQUENCER_H

#include "common/scummsys.h"

#include "common/error.h"

namespace Sci {

#define SFX_SEQ_PATCHFILE_NONE -1

struct sfx_sequencer_t {
	const char *name;    /* Sequencer name */
	const char *version; /* Sequencer version */

	int device;  /* Type of device the sequencer depends on, may be SFX_DEVICE_NONE. */

	Common::Error (*set_option)(char *name, char *value);
	/* Sets an option for the sequencing mechanism
	** Parameters: (char *) name: The name describing what to set
	**             (char *) value: The value to set
	** Returns   : (int) Common::kNoError, or Common::kUnknownError if the name wasn't understood
	*/

	Common::Error (*open)(int patch_len, byte *patch, int patch2_len, byte *patch2, void *device);
	/* Opens the sequencer for writing
	** Parameters: (int) patch_len, patch2_len: Length of the patch data
	**             (byte *) patch, patch2: Bulk patch data
	**             (void *) device: A device matching the 'device' property, or NULL
	**                              if the property is null.
	** Returns   : (int) Common::kNoError on success, Common::kUnknownError otherwise
	** The device should be initialized to a tick frequency of 60 Hz.
	** 'patch' and 'patch_len' refer to the patch resource passed to open,
	** as specified by the 'patchfile' property. 'patch' may be NULL if the
	** resource wasn't found.
	** For more information regarding patch resources, please refer to the
	** FreeSCI documentation, particularly the part regarding 'patch.*' resource
	** data.
	*/

	Common::Error (*close)();
	/* Closes the sequencer
	** Returns   : Common::kNoError on success, Common::kUnknownError otherwise
	*/

	Common::Error (*event)(byte command, int argc, byte *argv);
	/* Plays a MIDI event
	** Parameters: (byte) command: MIDI command to play
	**             (int) argc: Number of arguments to the command
	**             (byte *) argv: Pointer to additional arguments
	** Returns   : Common::kNoError on success, Common::kUnknownError otherwise
	** argv is guaranteed to point to a sufficiently large number of
	** arguments, as indicated by 'command' and the MIDI standard.
	** No 'running status' will be passed, 'command' will always be
	** explicit.
	*/
	Common::Error (*delay)(int ticks); /* OPTIONAL -- may be NULL, but highly recommended */
	/* Inserts a delay (delta time) into the sequencer cue
	** Parameters: (int) ticks: Number of 60 Hz ticks to delay
	** Returns   : Common::kNoError on success, Common::kUnknownError otherwise
	*/

	Common::Error (*reset_timer)(uint32 ts);
	/* OPTIONAL -- may be NULL, but highly recommended in combination with delay() */
	/* Resets the timer counter associated with the 'delay()' function
	** Parameters: (uint32) ts: Timestamp of the base time
	** Returns   : Common::kNoError on success, Common::kUnknownError otherwise
	*/

	Common::Error (*allstop)(); /* OPTIONAL -- may be NULL */
	/* Stops playing everything in the sequencer queue
	** Returns   : Common::kNoError on success, Common::kUnknownError otherwise
	*/

	Common::Error (*volume)(uint8 volume);  /* OPTIONAL -- can be NULL */
	/* Sets the sequencer volume
	** Parameters; (byte) volume: The volume to set, with 0 being mute and 127 full volume
	** Returns   : Common::kNoError on success, Common::kUnknownError otherwise
	*/

	Common::Error (*reverb)(int param); /* OPTIONAL -- may be NULL */
	/* Sets the device reverb
	** Parameters; (int) param: The reverb to set
	** Returns   : Common::kNoError on success, Common::kUnknownError otherwise
	*/

	int patchfile, patchfile2; /* Patch resources to pass into the call to open(),
		       ** if present, or SFX_SEQ_PATCHFILE_NONE  */
	uint8 playmask; /* SCI 'playflag' mask to determine which SCI song channels
			 ** this sequencer should play */
	/* 0x01	-- MT-32
	** 0x02	-- Yamaha FB-01
	** 0x04	-- CMS or Game Blaster
	** 0x08	-- Casio MT540 or CT460
	** 0x10	-- Tandy 3-voice
	** 0x20 -- PC speaker
	*/
	uint8 play_rhythm; /* Plays the rhythm channel? */
	int8 polyphony; /* Device polyphony (# of voices) */

	int min_write_ahead_ms; /* Minimal write-ahead, in milliseconds */
	/* Note that write-ahead is tuned automatically; this enforces a lower limit */

};


sfx_sequencer_t *sfx_find_sequencer(char *name);
/* Finds a sequencer by name
** Parameters: (char *) name: Name of the sequencer to look up, or NULL for default
** Returns   : (sfx_sequencer_t *) The sequencer of matching name, or NULL
**                                 if not found
*/

} // End of namespace Sci

#endif // SCI_SFX_SEQUENCER_H
