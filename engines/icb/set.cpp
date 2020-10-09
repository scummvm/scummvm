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

#include "engines/icb/common/px_common.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/set.h"
#include "engines/icb/debug.h"
#include "engines/icb/mission.h"
#include "engines/icb/session.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/res_man.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/surface_manager.h"
#include "engines/icb/p4.h"
#include "engines/icb/text.h"

namespace ICB {

int32 _game_session::GetSelectedMegaId(void) {
	selected_mega_id = -1;
	if ((px.mega_hilite) && (player.interact_selected)) {
		int sel_id = player.cur_interact_id;
		if (logic_structs[sel_id]->image_type != PROP) {
			selected_mega_id = player.cur_interact_id;
		}
	}
	return selected_mega_id;
}

void _game_session::GetSelectedMegaRGB(u_char &r, u_char &g, u_char &b) {
	int newr = (u_char)g_mega_select_r + r;
	int newg = (u_char)g_mega_select_g + g;
	int newb = (u_char)g_mega_select_b + b;

	if (newr > 150)
		newr = 150;
	if (newg > 150)
		newg = 150;
	if (newb > 150)
		newb = 150;

	r = (u_char)newr;
	g = (u_char)newg;
	b = (u_char)newb;

	g_mega_select_r += 8;
	g_mega_select_g += 8;
	g_mega_select_b += 8;

	if (g_mega_select_r > 128)
		g_mega_select_r = 40;
	if (g_mega_select_g > 128)
		g_mega_select_g = 40;
	if (g_mega_select_b > 128)
		g_mega_select_b = 40;
}

int32 _game_session::GetSelectedPropId(void) {
	selected_prop_id = -1;
	if ((px.prop_hilite) && (player.interact_selected)) {
		int sel_id = player.cur_interact_id;
		if (logic_structs[sel_id]->image_type == PROP) {
			selected_prop_id = player.cur_interact_id;
		}
	}
	return selected_prop_id;
}

void _game_session::GetSelectedPropRGB(u_char &r, u_char &g, u_char &b) {
	r = (u_char)g_prop_select_r;
	g = (u_char)g_prop_select_g;
	b = (u_char)g_prop_select_b;

	g_prop_select_r += 4;
	g_prop_select_g += 4;
	g_prop_select_b += 4;

	if (g_prop_select_r > 255)
		g_prop_select_r = 100;
	if (g_prop_select_g > 255)
		g_prop_select_g = 100;
	if (g_prop_select_b > 255)
		g_prop_select_b = 100;
}

bool8 _game_session::IsPropSelected(const char *propName) {
	uint32 prop_number = objects->Fetch_item_number_by_name(propName);

	if (prop_number == 0xFFFFFFFF)
		return FALSE8;

	// Does this prop's id match the seleced_prop's id ?
	if (prop_number == (uint)selected_prop_id)
		return TRUE8;

	return FALSE8;
}

#if CD_MODE == 0

void _game_session::Render_3d_nicos() {
	_feature_info *feature;
	uint32 j;
	uint32 pitch; // backbuffer pitch
	uint8 *ad;

	_rgb pen = {// rgb
	            0, 230, 255, 0};

	// is this mode switched on
	if (px.nicos_displayed == FALSE8)
		return;

	ad = surface_manager->Lock_surface(working_buffer_id);
	pitch = surface_manager->Get_pitch(working_buffer_id);

	for (j = 0; j < features->Fetch_number_of_items(); j++) {
		// get nico
		feature = (_feature_info *)MS->features->Fetch_item_by_number(j);

		// setup camera : have to do this once per frame because
		// clip_text_print does a Res_open and in principal this
		// could move the camera about due to defragging etc.
		PXcamera &camera = GetCamera();

		// set up nico world coords
		PXvector pos;
		pos.x = feature->x;
		pos.y = feature->y;
		pos.z = feature->z;

		// yesno
		bool8 result = FALSE8;

		// screen pos
		PXvector filmpos;

		// compute screen coord
		PXWorldToFilm(pos, camera, result, filmpos);

		// print name if on screen
		if (result) {
			Clip_text_print(&pen, (uint32)(filmpos.x + (SCREEN_WIDTH / 2)), (uint32)((SCREEN_DEPTH / 2) - filmpos.y), ad, pitch, "%s %3.1f",
			                (char *)features->Fetch_items_name_by_number(j), feature->direction);
		}
	}

	surface_manager->Unlock_surface(working_buffer_id);
}

#else

void _game_session::Render_3d_nicos() {}

#endif // #if CD_MODE == 0

} // End of namespace ICB
