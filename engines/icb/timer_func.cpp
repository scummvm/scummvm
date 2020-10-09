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

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/debug.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_vars.h"
#include "engines/icb/timer_func.h"
#include "engines/icb/common/ptr_util.h"

namespace ICB {

// whether the timer is on or off...
bool8 timerOn = FALSE8;

// hash of variable being followed...
uint32 timerVariable;

// divide the variable by this to get seconds (generally 12 for game cycles)
int32 timerMultiplier;

// below this value we display in red...
int32 timerDanger;

#define BAR_WIDTH 8
#define BAR_HEIGHT 6
#define DIG_SPACING (BAR_WIDTH + 4)
#define TIME_X (320 + 250)
#define TIME_Y (240 + 220)

// generic digits..............
#define TOP_BIT 1
#define TOP_RIGHT_BIT 2
#define TOP_LEFT_BIT 4
#define MID_BIT 8
#define BOT_RIGHT_BIT 16
#define BOT_LEFT_BIT 32
#define BOT_BIT 64

const uint8 digitBits[10] = {
    //	TOP_BIT  |   TOP_RIGHT_BIT   |   TOP_LEFT_BIT    |   MID_BIT |   BOT_RIGHT_BIT   |   BOT_LEFT_BIT    |   BOT_BIT
    TOP_BIT | TOP_RIGHT_BIT | TOP_LEFT_BIT | 0 | BOT_RIGHT_BIT | BOT_LEFT_BIT | BOT_BIT,       // 0
    0 | 0 | TOP_LEFT_BIT | 0 | 0 | BOT_LEFT_BIT | 0,                                           // 1
    TOP_BIT | TOP_RIGHT_BIT | 0 | MID_BIT | 0 | BOT_LEFT_BIT | BOT_BIT,                        // 2
    TOP_BIT | TOP_RIGHT_BIT | 0 | MID_BIT | BOT_RIGHT_BIT | 0 | BOT_BIT,                       // 3
    0 | TOP_RIGHT_BIT | TOP_LEFT_BIT | MID_BIT | BOT_RIGHT_BIT | 0 | 0,                        // 4
    TOP_BIT | 0 | TOP_LEFT_BIT | MID_BIT | BOT_RIGHT_BIT | 0 | BOT_BIT,                        // 5
    TOP_BIT | 0 | TOP_LEFT_BIT | MID_BIT | BOT_RIGHT_BIT | BOT_LEFT_BIT | BOT_BIT,             // 6
    TOP_BIT | TOP_RIGHT_BIT | 0 | 0 | BOT_RIGHT_BIT | 0 | 0,                                   // 7
    TOP_BIT | TOP_RIGHT_BIT | TOP_LEFT_BIT | MID_BIT | BOT_RIGHT_BIT | BOT_LEFT_BIT | BOT_BIT, // 8
    TOP_BIT | TOP_RIGHT_BIT | TOP_LEFT_BIT | MID_BIT | BOT_RIGHT_BIT | 0 | BOT_BIT,            // 9

};

const short barXY[7][2] = {
    {1, 0},                          // top
    {BAR_WIDTH + 1, 1},              // top/right
    {0, 1},                          // top/left
    {1, BAR_HEIGHT + 1},             // mid
    {BAR_WIDTH + 1, BAR_HEIGHT + 2}, // bot/right
    {0, BAR_HEIGHT + 2},             // bot/left
    {1, 2 * BAR_HEIGHT + 2}          // bot
};

// DRAW THE SEGMENT - different for psx and pc at the moment -----------------

#include "surface_manager.h"

void DrawSegment(int x, int y, int dx, int dy, int danger) {
	uint8 *surface_address;
	uint32 pitch;

	surface_address = surface_manager->Lock_surface(working_buffer_id);
	pitch = surface_manager->Get_pitch(working_buffer_id);

	_rgb drawColour;

	_rgb red = {255, 64, 64, 0};
	_rgb green = {64, 255, 64, 0};
	_rgb black = {0, 0, 0, 0};

	if (danger)
		drawColour = red;
	else
		drawColour = green;

	if (dx == 0) {
		Draw_vertical_line(x - 1, y, dy, &black, surface_address, pitch);
		Draw_vertical_line(x, y, dy, &drawColour, surface_address, pitch);
		Draw_vertical_line(x + 1, y, dy, &black, surface_address, pitch);
	} else {
		Draw_horizontal_line(x, y - 1, dx, &black, surface_address, pitch);
		Draw_horizontal_line(x, y, dx, &drawColour, surface_address, pitch);
		Draw_horizontal_line(x, y + 1, dx, &black, surface_address, pitch);
	}
	surface_manager->Unlock_surface(working_buffer_id);
}

// draw a digit using lcd display, -1 for the :
void DrawDigit(int x, int y, int d, int danger) {
	int sx, sy, dx, dy;
	int i, dig;

	// special dash...
	if (d == -1) {
		DrawSegment(x + 1 + BAR_WIDTH / 2, y + 1, 0, BAR_HEIGHT - 1, danger);
		DrawSegment(x + 1 + BAR_WIDTH / 2, y + BAR_HEIGHT + 2, 0, BAR_HEIGHT - 1, danger);
		return;
	}

	// move the 1 back 4 pixels
	if (d == 1)
		x += 1 + BAR_WIDTH / 2;

	for (i = 0; i < 7; i++) {
		dig = (1 << i);

		if (digitBits[d] & dig) {
			sx = x + barXY[i][0];
			sy = y + barXY[i][1];
			if ((dig == TOP_BIT) || (dig == BOT_BIT) || (dig == MID_BIT)) {
				dx = BAR_WIDTH;
				dy = 0;
			} else {
				dx = 0;
				dy = BAR_HEIGHT;
			}

			DrawSegment(sx, sy, dx, dy, danger);
		}
	}
}

// draw mm:ss
void DrawTime(int mins, int secs, int hs, int danger) {
	int x, y;

	x = TIME_X;
	y = TIME_Y;

	DrawDigit(x, y, mins / 10, danger);

	x += DIG_SPACING;

	DrawDigit(x, y, mins % 10, danger);

	x += DIG_SPACING;

	if (hs)
		DrawDigit(x, y, -1, danger); // dash

	x += DIG_SPACING;

	DrawDigit(x, y, secs / 10, danger);

	x += DIG_SPACING;

	DrawDigit(x, y, secs % 10, danger);
}

// call this to draw timer if it is on
void DrawTimer() {
	if (!timerOn)
		return;

	int mins, secs, cycles, totalSecs, hs, danger;

	cycles = g_globalScriptVariables.GetVariable(timerVariable);

	totalSecs = cycles / timerMultiplier;

	mins = totalSecs / 60;
	secs = totalSecs % 60;

	if ((cycles % timerMultiplier) < (timerMultiplier / 2))
		hs = 1;
	else
		hs = 0;

	if (cycles < timerDanger)
		danger = 1;
	else
		danger = 0;

	DrawTime(mins, secs, hs, danger);
}

void StopTimer() {
	// timer off
	timerOn = FALSE8;
}

mcodeFunctionReturnCodes fn_stop_timer(int32 &, int32 *) {
	StopTimer();

	return (IR_CONT);
}

mcodeFunctionReturnCodes fn_activate_timer(int32 &, int32 *params) {
	const char *timer_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// timer is now on
	timerOn = TRUE8;

	// variable
	timerVariable = HashString(timer_name);

	// test variable exists...
	int32 test = g_globalScriptVariables.FindVariable(timerVariable);
	if (test == -1) {
		Fatal_error("No global variable %s which fn_activate_timer() has been told to follow!", timer_name);
	}

	// set other values...
	timerMultiplier = (int32)params[1];
	timerDanger = (int32)params[2];

	return (IR_CONT);
}

} // End of namespace ICB
