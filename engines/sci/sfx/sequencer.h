/***************************************************************************
 sfx_sequencer.h, from
	 midi_device.h Copyright (C) 2001 Solomon Peachy
		       Copytight (C) 2002 Christoph Reichenbach

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

***************************************************************************/


#ifndef _SFX_SEQUENCER_H_
#define _SFX_SEQUENCER_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */
#include <sfx_core.h>
#include <stdio.h>
#include "device.h"
#include <scitypes.h>

#define SFX_SEQ_PATCHFILE_NONE -1

typedef struct _sfx_sequencer {
	const char *name;    /* Sequencer name */
	const char *version; /* Sequencer version */

	int device;  /* Type of device the sequencer depends on, may be SFX_DEVICE_NONE. */

	int
	(*set_option)(char *name, char *value);
	/* Sets an option for the sequencing mechanism
	** Parameters: (char *) name: The name describing what to set
	**             (char *) value: The value to set
	** Returns   : (int) SFX_OK, or SFX_ERROR if the name wasn't understood
	*/

	int
	(*open)(int patch_len, byte *patch, int patch2_len, byte *patch2, void *device);
	/* Opens the sequencer for writing
	** Parameters: (int) patch_len, patch2_len: Length of the patch data
	**             (byte *) patch, patch2: Bulk patch data
	**             (void *) device: A device matching the 'device' property, or NULL
	**                              if the property is null.
	** Returns   : (int) SFX_OK on success, SFX_ERROR otherwise
	** The device should be initialized to a tick frequency of 60 Hz.
	** 'patch' and 'patch_len' refer to the patch resource passed to open,
	** as specified by the 'patchfile' property. 'patch' may be NULL if the
	** resource wasn't found.
	**   For more information regarding patch resources, please refer to the
	** FreeSCI documentation, particularly the part regarding 'patch.*' resource
	** data.
	*/

	int (*close)(void);
	/* Closes the sequencer
	** Returns   : SFX_OK on success, SFX_ERROR otherwise
	*/

	int (*event)(byte command, int argc, byte *argv);
	/* Plays a MIDI event
	** Parameters: (byte) command: MIDI command to play
	**             (int) argc: Number of arguments to the command
	**             (byte *) argv: Pointer to additional arguments
	** Returns   : SFX_OK on success, SFX_ERROR otherwise
	** argv is guaranteed to point to a sufficiently large number of
	** arguments, as indicated by 'command' and the MIDI standard.
	** No 'running status' will be passed, 'command' will always be
	** explicit.
	*/
	int (*delay)(int ticks); /* OPTIONAL -- may be NULL, but highly recommended */
	/* Inserts a delay (delta time) into the sequencer cue
	** Parameters: (int) ticks: Number of 60 Hz ticks to delay
	** Returns   : SFX_OK on success, SFX_ERROR otherwise
	*/

	int (*reset_timer)(GTimeVal ts);
	/* OPTIONAL -- may be NULL, but highly recommended in combination with delay() */
	/* Resets the timer counter associated with the 'delay()' function
	** Parameters: (GTimeVal) ts: Timestamp of the base time
	** Returns   : SFX_OK on success, SFX_ERROR otherwise
	*/

	int (*allstop)(void); /* OPTIONAL -- may be NULL */
	/* Stops playing everything in the sequencer queue
	** Returns   : SFX_OK on success, SFX_ERROR otherwise
	*/

        int (*volume)(guint8 volume);  /* OPTIONAL -- can be NULL */
	/* Sets the sequencer volume
	** Parameters; (byte) volume: The volume to set, with 0 being mute and 127 full volume
	** Returns   : SFX_OK on success, SFX_ERROR otherwise
	*/

	int (*reverb)(int param); /* OPTIONAL -- may be NULL */
	/* Sets the device reverb
	** Parameters; (int) param: The reverb to set
	** Returns   : SFX_OK on success, SFX_ERROR otherwise
	*/

	int patchfile, patchfile2; /* Patch resources to pass into the call to open(),
		       ** if present, or SFX_SEQ_PATCHFILE_NONE  */
	guint8 playmask; /* SCI 'playflag' mask to determine which SCI song channels
			 ** this sequencer should play */
	/* 0x01	-- MT-32
	** 0x02	-- Yamaha FB-01
	** 0x04	-- CMS or Game Blaster
	** 0x08	-- Casio MT540 or CT460
	** 0x10	-- Tandy 3-voice
	** 0x20 -- PC speaker
	*/
	guint8 play_rhythm; /* Plays the rhythm channel? */
	gint8 polyphony; /* Device polyphony (# of voices) */

	int min_write_ahead_ms; /* Minimal write-ahead, in milliseconds */
	/* Note that write-ahead is tuned automatically; this enforces a lower limit */

} sfx_sequencer_t;


sfx_sequencer_t *
sfx_find_sequencer(char *name);
/* Finds a sequencer by name
** Parameters: (char *) name: Name of the sequencer to look up, or NULL for default
** Returns   : (sfx_sequencer_t *) The sequencer of matching name, or NULL
**                                 if not found
*/


#endif /* _SFX_SEQUENCER_H_ */
