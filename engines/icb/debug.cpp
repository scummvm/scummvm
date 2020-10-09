/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/p4.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/keyboard.h"
#include "engines/icb/debug.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/session.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/mouse.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/common/px_array.h"
#include "engines/icb/text.h"

namespace ICB {

#define LIGHT_RECT_WIDTH (16)
#define LIGHT_RECT_HEIGHT (16)
#define LIGHT_TIE_WIDTH (10)
#define LIGHT_TIE_HEIGHT (10)

// Pink
#define LIGHT_TIE_PEN (0xFF7080)

// 2m below the light source
#define LIGHT_TIE_Y_OFFSET (-200)

// PSX specifics are in "debug_psx.cpp"
// PC specifics are in "debug_pc.cpp"

// zdebug logging on or off
bool8 terminate_debugging = FALSE8;

// This flag indicates whether or not the debug simulated feature is activated or not.
bool8 debug_auto_save = FALSE8;

#if CD_MODE == 0

void _game_session::Show_lit_unlit_diagnostics() {
	uint32 j;
	uint32 pitch; // backbuffer pitch
	uint8 *ad;

	_rgb pen = {// rgb
	            255, 0, 0, 0};

	// cross hair is now a development option
	if (px.cross_hair == FALSE8)
		return;

	ad = surface_manager->Lock_surface(working_buffer_id);
	pitch = surface_manager->Get_pitch(working_buffer_id);

	// setup camera
	PXcamera &camera = GetCamera();
	// set up nico world coords
	PXvector pos;

	for (j = 0; j < number_of_voxel_ids; j++) {
		pos.x = logic_structs[voxel_id_list[j]]->mega->actor_xyz.x;
		pos.y = logic_structs[voxel_id_list[j]]->mega->actor_xyz.y;
		pos.z = logic_structs[voxel_id_list[j]]->mega->actor_xyz.z;

		// screen pos
		PXvector filmpos;

		// yesno
		bool8 result = FALSE8;

		// compute screen coord
		PXWorldToFilm(pos, camera, result, filmpos);

		// print name if on screen
		if (result) {
			if (!logic_structs[voxel_id_list[j]]->mega->in_shade)
				Clip_text_print(&pen, (int32)(filmpos.x + (SCREEN_WIDTH / 2)), (int32)((SCREEN_DEPTH / 2) - filmpos.y), ad, pitch, ">");
		}
	}

	// Unlock to use Fill_rect !
	surface_manager->Unlock_surface(working_buffer_id);

	_rgb tiecolour = {0xFF, 0x70, 0x80, 0};
	_rgb dcolour = {0x00, 0x00, 0xFF, 0};
	_rgb tcol = {0xFF, 0xFF, 0xFF, 0};
	_rgb lcol = {0xFF, 0x70, 0x80, 0};

	// Draw the lights at their screen positions and with their base colours
	if (SetOK()) {
		rlp_API *lights = set.GetPRig();
		if (lights != NULL) {
			for (j = 0; j < lights->nLamps; j++) {
				PSXLamp *pLamp = (PSXLamp *)lights->GetLamp(j);
				uint32 state = pLamp->nStates - 1;
				if (state > 0) {
					state = Fetch_prop_state(pLamp->prop_name);
				}
				// Really shoud write a PSXLamp::GetState( uint32 n ) !
				PSXLampState *plampstate = pLamp->states + state;

				int r = plampstate->c.r;
				int g = plampstate->c.g;
				int b = plampstate->c.b;
				int m = plampstate->m;

				r = (r * m) >> 7; // m has 1.0 = 128
				g = (g * m) >> 7; // m has 1.0 = 128
				b = (b * m) >> 7; // m has 1.0 = 128

				// Convert from 0-4096 -> 0-256
				r >>= 4;
				g >>= 4;
				b >>= 4;

				// Clamp the colours
				if (r > 255)
					r = 255;
				if (g > 255)
					g = 255;
				if (b > 255)
					b = 255;

				pos.x = (PXfloat)plampstate->pos.vx;
				pos.y = (PXfloat)plampstate->pos.vy;
				pos.z = (PXfloat)plampstate->pos.vz;

				// screen pos
				PXvector lightpos, tiepos;

				// yesno
				bool8 result = FALSE8;

				// compute screen coord
				PXWorldToFilm(pos, camera, result, lightpos);

				// draw rectangle of the correct colour
				int x0 = (int32)(lightpos.x + (SCREEN_WIDTH / 2));
				int y0 = (int32)(SCREEN_DEPTH / 2 - lightpos.y);
				uint32 penrgb = (r << 16) | (g << 8) | (b << 0);

				Fill_rect(x0 - LIGHT_RECT_WIDTH / 2, y0 - LIGHT_RECT_HEIGHT / 2, x0 + LIGHT_RECT_WIDTH / 2, y0 + LIGHT_RECT_HEIGHT / 2, penrgb, (int32)-lightpos.z);

				// Draw a 'shadow' away from light ! - to try and guess z-distance
				pos.y += LIGHT_TIE_Y_OFFSET;
				PXWorldToFilm(pos, camera, result, tiepos);
				int x1 = (int32)(tiepos.x + (SCREEN_WIDTH / 2));
				int y1 = (int32)(SCREEN_DEPTH / 2 - tiepos.y);
				penrgb = LIGHT_TIE_PEN;
				Fill_rect(x1 - LIGHT_TIE_WIDTH / 2, y1 - LIGHT_TIE_HEIGHT / 2, x1 + LIGHT_TIE_WIDTH / 2, y1 + LIGHT_TIE_HEIGHT / 2, penrgb, (int32)-tiepos.z);

				// Lock to use General_draw_line !
				ad = surface_manager->Lock_surface(working_buffer_id);

				// Draw a pink line from light to its tie !
				General_draw_line_24_32((short)x0, (short)y0, (short)x1, (short)y1, &tiecolour, ad, pitch);

				// Print the name of the light at the light pos and the tie pos
				Clip_text_print(&tcol, x0, y0, ad, pitch, "%s", pLamp->lamp_name);
				Clip_text_print(&tcol, x1, y1, ad, pitch, "%s", pLamp->lamp_name);

				// Draw a line to show the light direction
				if ((pLamp->type == DIRECT_LIGHT) || (pLamp->type == SPOT_LIGHT)) {
					// plampstate direciton is normalised to be 4096 int32
					// Take that down to 128cm int32 (>>5)
					int dx = plampstate->vx >> 5;
					int dy = plampstate->vy >> 5;
					int dz = plampstate->vz >> 5;

					// Remove the tie position offset
					pos.y -= LIGHT_TIE_Y_OFFSET;

					pos.x += dx;
					pos.y += dy;
					pos.z += dz;
					PXWorldToFilm(pos, camera, result, tiepos);
					// Draw a line to show the direction of the light
					x1 = (int32)(tiepos.x + (SCREEN_WIDTH / 2));
					y1 = (int32)(SCREEN_DEPTH / 2 - tiepos.y);
					General_draw_line_24_32((short)x0, (short)y0, (short)x1, (short)y1, &dcolour, ad, pitch);
				}

				// Unlock to use Fill_rect !
				surface_manager->Unlock_surface(working_buffer_id);
			}
			// Lock to use Clip_text_print !
			ad = surface_manager->Lock_surface(working_buffer_id);

			Clip_text_print(&lcol, 0, 12, ad, pitch, "%d Lamps", lights->nLamps);
			// Unlock to use Fill_rect !
			surface_manager->Unlock_surface(working_buffer_id);
		}
	}
}

#else

void _game_session::Show_lit_unlit_diagnostics() {}

#endif // #if CD_MODE == 0

} // End of namespace ICB
