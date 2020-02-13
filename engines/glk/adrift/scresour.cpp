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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/adrift/scare.h"
#include "glk/adrift/scprotos.h"
#include "glk/adrift/scgamest.h"

namespace Glk {
namespace Adrift {

/* Assorted definitions and constants. */
static const sc_char NUL = '\0';


/*
 * res_has_sound()
 * res_has_graphics()
 *
 * Return TRUE if the game uses sound or graphics.
 */
sc_bool res_has_sound(sc_gameref_t game) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[2];
	sc_bool has_sound;
	assert(gs_is_game_valid(game));

	vt_key[0].string = "Globals";
	vt_key[1].string = "Sound";
	has_sound = prop_get_boolean(bundle, "B<-ss", vt_key);
	return has_sound;
}

sc_bool res_has_graphics(sc_gameref_t game) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[2];
	sc_bool has_graphics;
	assert(gs_is_game_valid(game));

	vt_key[0].string = "Globals";
	vt_key[1].string = "Graphics";
	has_graphics = prop_get_boolean(bundle, "B<-ss", vt_key);
	return has_graphics;
}


/*
 * res_set_resource()
 * res_clear_resource()
 * res_compare_resource()
 *
 * Convenience functions to set, clear, and compare resource fields.
 */
static void res_set_resource(sc_resourceref_t resource, const sc_char *name,
                 sc_int offset, sc_int length) {
	resource->name = name;
	resource->offset = offset;
	resource->length = length;
}

void res_clear_resource(sc_resourceref_t resource) {
	res_set_resource(resource, "", 0, 0);
}

sc_bool res_compare_resource(sc_resourceref_t from, sc_resourceref_t with) {
	return strcmp(from->name, with->name) == 0
	       && from->offset == with->offset && from->length == with->length;
}


/*
 * res_handle_resource()
 *
 * General helper for handling graphics and sound resources.  Supplied with a
 * partial key to the node containing resources, it identifies what resource
 * is appropriate, and sets this as the requested resource in the game, for
 * later use on sync'ing, using the handler appropriate for the game version.
 *
 * The partial format is something like "sis" (the bit to follow I<- or S<-
 * in prop_get), and the partial key is guaranteed to contain at least
 * strlen(partial_format) elements.
 */
void res_handle_resource(sc_gameref_t game, const sc_char *partial_format,
		const sc_vartype_t vt_partial[]) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[2], *vt_full;
	sc_int partial_length, resource_start_offset;
	sc_bool embedded;
	sc_char *format;
	assert(gs_is_game_valid(game));
	assert(partial_format && vt_partial);

	/*
	 * Check for resources.  If this game doesn't use any, exit now to avoid the
	 * overhead of pointless lookups and allocations.
	 */
	if (!(res_has_sound(game) || res_has_graphics(game)))
		return;

	/*
	 * Get the global offset for all resources.  For version 3.9 games this
	 * should be zero.  For version 4.0 games, it's the start of resource data
	 * in the TAF file where resources are embedded.
	 */
	vt_key[0].string = "ResourceOffset";
	resource_start_offset = prop_get_integer(bundle, "I<-s", vt_key);

	/*
	 * Get the flag that indicated embedded resources.  For version 3.9 games
	 * this should be false.  If not set, offset and length are forced to zero
	 * for interface functions.
	 */
	vt_key[0].string = "Globals";
	vt_key[1].string = "Embedded";
	embedded = prop_get_boolean(bundle, "B<-ss", vt_key);

	/*
	 * Allocate a format for use with properties calls, five characters longer
	 * than the partial passed in.  Build a key one element larger than the
	 * partial supplied, and copy over all supplied elements.
	 */
	partial_length = strlen(partial_format);
	format = (sc_char *)sc_malloc(partial_length + 5);

	vt_full = (sc_vartype_t *)sc_malloc((partial_length + 1) * sizeof(vt_partial[0]));
	memcpy(vt_full, vt_partial, partial_length * sizeof(vt_partial[0]));

	/* Search for sound resources, and offer if found. */
	if (res_has_sound(game)) {
		const sc_char *soundfile;
		sc_int soundoffset, soundlen;

		/* Get soundfile property from the node supplied. */
		vt_full[partial_length].string = "SoundFile";
		strcpy(format, "S<-");
		strcat(format, partial_format);
		strcat(format, "s");
		soundfile = prop_get_string(bundle, format, vt_full);

		/* If a sound is defined, handle it. */
		if (!sc_strempty(soundfile)) {
			if (embedded) {
				/* Retrieve offset and length. */
				vt_full[partial_length].string = "SoundOffset";
				strcpy(format, "I<-");
				strcat(format, partial_format);
				strcat(format, "s");
				soundoffset = prop_get_integer(bundle, format, vt_full)
				              + resource_start_offset;

				vt_full[partial_length].string = "SoundLen";
				strcpy(format, "I<-");
				strcat(format, partial_format);
				strcat(format, "s");
				soundlen = prop_get_integer(bundle, format, vt_full);
			} else {
				/* Coerce offset and length to zero. */
				soundoffset = 0;
				soundlen = 0;
			}

			/*
			 * If the sound is the special "##", latch stop, otherwise note
			 * details to play on sync.
			 */
			if (!strcmp(soundfile, "##")) {
				game->stop_sound = TRUE;
				res_clear_resource(&game->requested_sound);
			} else {
				res_set_resource(&game->requested_sound,
				                 soundfile, soundoffset, soundlen);
			}
		}
	}

	/* Now do the same thing for graphics resources. */
	if (res_has_graphics(game)) {
		const sc_char *graphicfile;
		sc_int graphicoffset, graphiclen;

		/* Get graphicfile property from the node supplied. */
		vt_full[partial_length].string = "GraphicFile";
		strcpy(format, "S<-");
		strcat(format, partial_format);
		strcat(format, "s");
		graphicfile = prop_get_string(bundle, format, vt_full);

		/* If a graphic is defined, handle it. */
		if (!sc_strempty(graphicfile)) {
			if (embedded) {
				/* Retrieve offset and length. */
				vt_full[partial_length].string = "GraphicOffset";
				strcpy(format, "I<-");
				strcat(format, partial_format);
				strcat(format, "s");
				graphicoffset = prop_get_integer(bundle, format, vt_full)
				                + resource_start_offset;

				vt_full[partial_length].string = "GraphicLen";
				strcpy(format, "I<-");
				strcat(format, partial_format);
				strcat(format, "s");
				graphiclen = prop_get_integer(bundle, format, vt_full);
			} else {
				/* Coerce offset and length to zero. */
				graphicoffset = 0;
				graphiclen = 0;
			}

			/* Graphics resource retrieved, note to show on sync. */
			res_set_resource(&game->requested_graphic,
			                 graphicfile, graphicoffset, graphiclen);
		}
	}

	/* Free allocated memory. */
	sc_free(format);
	sc_free(vt_full);
}


/*
 * res_sync_resources()
 *
 * Bring resources into line with the game; called on undo, restart,
 * restore, and so on.
 */
void res_sync_resources(sc_gameref_t game) {
	assert(gs_is_game_valid(game));

	/* Deal with any latched sound stop first. */
	if (game->stop_sound) {
		if (game->sound_active) {
			if_update_sound("", 0, 0, FALSE);
			game->sound_active = FALSE;

			res_clear_resource(&game->playing_sound);
		}
		game->stop_sound = FALSE;
	}

	/* Look for a change of sound, and pass to interface on change. */
	if (!res_compare_resource(&game->playing_sound,
	                          &game->requested_sound)) {
		const sc_char *name;
		sc_char *clean_name;
		sc_bool is_looping;

		/* If the sound name ends '##', this is a looping sound. */
		name = game->requested_sound.name;
		is_looping = !strcmp(name + strlen(name) - 2, "##");

		clean_name = (sc_char *)sc_malloc(strlen(name) + 1);
		strcpy(clean_name, name);
		if (is_looping)
			clean_name[strlen(clean_name) - 2] = NUL;

		if_update_sound(clean_name,
		                game->requested_sound.offset,
		                game->requested_sound.length, is_looping);
		game->playing_sound = game->requested_sound;
		game->sound_active = TRUE;

		sc_free(clean_name);
	}

	/* Look for a change of graphic, and pass to interface on change. */
	if (!res_compare_resource(&game->displayed_graphic,
	                          &game->requested_graphic)) {
		if_update_graphic(game->requested_graphic.name,
		                  game->requested_graphic.offset,
		                  game->requested_graphic.length);
		game->displayed_graphic = game->requested_graphic;
	}
}


/*
 * res_cancel_resources()
 *
 * Turn off sound and graphics, and reset the game's tracking of resources in
 * use to match.  Called on game restart or restore.
 */
void res_cancel_resources(sc_gameref_t game) {
	assert(gs_is_game_valid(game));

	/* Request that everything stops and clears. */
	game->stop_sound = FALSE;
	res_clear_resource(&game->requested_sound);
	res_clear_resource(&game->requested_graphic);

	/* Synchronize to have the above take effect. */
	res_sync_resources(game);
}

} // End of namespace Adrift
} // End of namespace Glk
