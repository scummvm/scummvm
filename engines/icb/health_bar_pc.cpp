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
#include "engines/icb/debug.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/common/px_game_object.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/object_structs.h"

namespace ICB {

#define BAR_X 500
#define BAR_Y 25
#define BAR_W 100
#define BAR_H 7
#define MAX_HEALTH 10

void _game_session::Draw_health_bar() {
	static int targetHealth = -1;
	static int barEnd;
	static int barTarget;
	static int healthStep = 0;
	static _rgb healthColours[MAX_HEALTH + 1] = {{254, 0, 0, 0},   //  0
	                                             {254, 0, 0, 0},   //  1
	                                             {254, 0, 0, 0},   //  2
	                                             {254, 64, 0, 0},  //  3
	                                             {254, 128, 0, 0}, //  4
	                                             {254, 196, 0, 0}, //  5
	                                             {254, 254, 0, 0}, //  6
	                                             {196, 254, 0, 0}, //  7
	                                             {128, 254, 0, 0}, //  8
	                                             {32, 254, 0, 0},  //  9
	                                             {0, 254, 0, 0}};  // 10

	// Lock the surface
	uint8 *ad = (uint8 *)surface_manager->Lock_surface(working_buffer_id);
	uint32 pitch = surface_manager->Get_pitch(working_buffer_id);

	// Draw the glass behind the health bar
	_rgb pen;
	pen.red = 96;
	pen.green = 96;
	pen.blue = 128;
	for (uint16 y = BAR_Y + BAR_H; (y > BAR_Y); y--)
		BlendedLine(BAR_X - 1, y, BAR_X + BAR_W, y, pen, SCREEN_WIDTH, SCREEN_DEPTH, pitch, ad);

	// Draw a border around the health bar
	General_draw_line_24_32(BAR_X - 2, BAR_Y - 1, BAR_X + BAR_W + 2, BAR_Y - 1, &pen, ad, pitch);
	General_draw_line_24_32(BAR_X - 2, BAR_Y + BAR_H + 2, BAR_X + BAR_W + 2, BAR_Y + BAR_H + 2, &pen, ad, pitch);
	General_draw_line_24_32(BAR_X - 2, BAR_Y - 1, BAR_X - 2, BAR_Y + BAR_H + 2, &pen, ad, pitch);
	General_draw_line_24_32(BAR_X + BAR_W + 2, BAR_Y - 1, BAR_X + BAR_W + 2, BAR_Y + BAR_H + 2, &pen, ad, pitch);

	// Find out how much health the player has (0 dead -- 10 full)
	c_game_object *ob = (c_game_object *)objects->Fetch_item_by_number(player.Fetch_player_id());
	int32 ret = ob->GetVariable("hits");
	int32 health = ob->GetIntegerVariable(ret);

	// Sort out the speed the health bar is moving at
	if (targetHealth == -1) {
		targetHealth = health;
		barEnd = barTarget = (BAR_W * health) / MAX_HEALTH;
		healthStep = 0;
	}
	if (targetHealth != health) {
		// Re calculate the speed we are moving the helath bar at
		healthStep = (((BAR_W * health) / MAX_HEALTH) - barEnd) / 6;
		barTarget = (BAR_W * health) / MAX_HEALTH;
		targetHealth = health;
	}

	if (abs(barEnd - barTarget) < abs(healthStep)) {
		barEnd = barTarget;
		healthStep = 0;
	}

	// Draw the solid part of the health bar
	if (healthStep > 0) {
		for (uint16 y = BAR_Y + BAR_H; (health != 0) && (y > BAR_Y); y--)
			General_draw_line_24_32(BAR_X, y, (uint16)(BAR_X + barEnd), y, &healthColours[health], ad, pitch);

		pen.red = 255;
		pen.green = 255;
		pen.blue = 255;
		BlendedLine((uint16)(BAR_X - 1), BAR_Y + 2, (uint16)(BAR_X + barEnd), BAR_Y + 2, pen, SCREEN_WIDTH, SCREEN_DEPTH, pitch, ad);
		pen.red = 0;
		pen.green = 0;
		pen.blue = 0;
		BlendedLine((uint16)(BAR_X - 1), (BAR_Y + BAR_H), (uint16)(BAR_X + barEnd), (BAR_Y + BAR_H), pen, SCREEN_WIDTH, SCREEN_DEPTH, pitch, ad);
	} else {
		for (uint16 y = BAR_Y + BAR_H; (health != 0) && (y > BAR_Y); y--)
			General_draw_line_24_32(BAR_X, y, (uint16)(BAR_X + barTarget), y, &healthColours[health], ad, pitch);

		pen.red = 255;
		pen.green = 255;
		pen.blue = 255;
		BlendedLine((uint16)(BAR_X - 1), BAR_Y + 2, (uint16)(BAR_X + barTarget), BAR_Y + 2, pen, SCREEN_WIDTH, SCREEN_DEPTH, pitch, ad);
		pen.red = 0;
		pen.green = 0;
		pen.blue = 0;
		BlendedLine((uint16)(BAR_X - 1), (BAR_Y + BAR_H), (uint16)(BAR_X + barTarget), (BAR_Y + BAR_H), pen, SCREEN_WIDTH, SCREEN_DEPTH, pitch, ad);
	}

	// Draw the fading part of the health bar
	if (barEnd != barTarget) {
		for (uint16 y = BAR_Y + BAR_H; y > BAR_Y; y--)
			BlendedLine((uint16)(BAR_X + barTarget - 1), y, (uint16)(BAR_X + barEnd), y, healthColours[health], SCREEN_WIDTH, SCREEN_DEPTH, pitch, ad);
	}

	// Move the bar along
	barEnd += healthStep;

	// Unlock the surface
	surface_manager->Unlock_surface(working_buffer_id);
}

} // End of namespace ICB
