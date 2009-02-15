/***************************************************************************
  Copyright (C) 2008 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantability,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <creichen@gmail.com>

***************************************************************************/

/* Implementation of SCI instrument maps for GM and MT-32. */

#ifndef SCI_INSTRUMENT_MAP_
#define SCI_INSTRUMENT_MAP_

#include <stdlib.h>
#include "resource.h"
#include "../device.h"

#define SFX_INSTRUMENTS_NR	0x80
#define SFX_RHYTHM_NR		0x80
#define SFX_VELOCITIES_NR	0x80
#define SFX_NO_VELOCITY_MAP	-1 /* use in velocity_map_index to indicate that no map should be used */

/* Instrument map types */
#define SFX_MAP_UNKNOWN 0
#define SFX_MAP_MT32 1 /* Original MT-32 map format */
#define SFX_MAP_MT32_GM 2 /* More recent map format used for both MT-32 and GM */

/* Patch not mapped */
#define SFX_UNMAPPED -1
/* Patch mapped to rhythm key */
#define SFX_MAPPED_TO_RHYTHM -2

/* Maximum velocity (used for scaling) */
#define SFX_MAX_VELOCITY 128

typedef struct {
	int patch; /* Native instrument, SFX_UNMAPPED or SFX_MAPPED_TO_RHYTHM */
	int rhythm; /* Rhythm key when patch == SFX_MAPPED_TO_RHYTHM */
} sfx_patch_map_t;

typedef struct {
	sfx_patch_map_t patch_map[SFX_INSTRUMENTS_NR]; /* Map patch nr to which native instrument or rhythm key */
	int patch_key_shift[SFX_INSTRUMENTS_NR]; /* Shift patch key by how much? */
	int patch_volume_adjust[SFX_INSTRUMENTS_NR]; /* Adjust controller 7 by how much? */
	int patch_bend_range[SFX_INSTRUMENTS_NR]; /* Bend range in semitones or SFX_UNMAPPED for default */

	int percussion_map[SFX_RHYTHM_NR]; /* Map percussion instrument (RHYTH_CHANNEL) to what native 'key'? */
	int percussion_volume_adjust; /* unused in SCI patches */

	int velocity_map_index[SFX_INSTRUMENTS_NR]; /* Velocity translation map to use for that instrument */
	int velocity_maps_nr; /* How many velocity translation maps do we have? */
	byte **velocity_map; /* velocity_maps_nr entries, each of size SFX_VELOCITIES_NR */
	int percussion_velocity_map_index; /* Special index for the percussion map */
	int percussion_velocity_scale[SFX_INSTRUMENTS_NR]; /* Velocity scale (0 - SFX_PERC_MAX_VOL) */

	size_t initialisation_block_size;
	byte *initialisation_block; /* Initial MIDI commands to set up the device */
} sfx_instrument_map_t;

sfx_instrument_map_t *
sfx_instrument_map_new(int velocity_maps_nr);
/* Constructs a new default-initialised velocity map
** Parameters: (int) velocity_maps_nr: Number of velocity maps to allocate
** Returns   : (sfx_instrument_map *) an initialised instrument map
*/

void
sfx_instrument_map_free(sfx_instrument_map_t *map);
/* Deallocates an instrument map
** Parameters: (sfx_instrument_map *) map: The map to deallocate, or NULL for a no-op
*/

sfx_instrument_map_t *
sfx_instrument_map_load_sci(byte *data, size_t length);
/* Allocate and initialise an instrument map from SCI data
** Parameters: (byte *) Pointer to the data to initialise from
**             (size_t) Number of bytes to expect within
** Returns   : (sfx_instrument_map_t *) An initialised instrument map for these settings, or NULL
**              if `data' is NULL or `data' and `length' do not permit a valid instrument map
** If `data' is null, the function will return NULL quietly.
*/

sfx_instrument_map_t *
sfx_instrument_map_mt32_to_gm(byte *data, size_t size);
/* Allocate and initialise an instrument map from MT-32 patch data
** Parameters: (byte *) Pointer to the MT-32 patch data to initialise from
**             (size_t) Number of bytes to expect within
** Returns   : (sfx_instrument_map_t *) An initialised instrument map for these settings
** If `data' is null or invalid, the function will return a default MT-32 to GM map.
*/

int
sfx_instrument_map_detect(byte *data, size_t size);
/* Detects the type of patch data
** Parameters: (byte *) Pointer to the patch data
**             (size_t) Number of bytes to expect within
** Returns   : (int) SFX_MAP_SCI1 for an SCI1 instrument map, SFX_MAP_SCI0_MT32 for SCI0 MT-32 patch data,
**	       or SFX_MAP_UNKNOWN for unknown.
*/

midi_writer_t *
sfx_mapped_writer(midi_writer_t *writer, sfx_instrument_map_t *map);
/* Wrap a midi_writer_t into an instrument map
** Parameters: (midi_writer_t *) writer: The writer to wrap
**             (sfx_instrument_map_t *) map: The map to apply to all commands going into the writer, or NULL
** Returns   : (midi_writer_t *) A MIDI writer that preprocesses all data by `map' and otherwise relies on `writer'
** Effects   : If successful and neccessary, this operation will send initialisation messages to the writer, as needed.
** If `map' is NULL, this returns `writer'.  Otherwise it sets up a Decorator that handles translation and automatically
** deallocates the instrument map when the writer is closed.
*/


#endif /* !defined(SCI_INSTRUMENT_MAP_) */
