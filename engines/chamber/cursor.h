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

#ifndef CHAMBER_CURSOR_H
#define CHAMBER_CURSOR_H

namespace Chamber {

#define CURSOR_WIDTH 16
#define CURSOR_HEIGHT 16
#define CURSOR_WIDTH_SPR 20

enum Cursors {
	CURSOR_FINGER,
	CURSOR_TARGET,
	CURSOR_FLY,
	CURSOR_SNAKE,
	CURSOR_GRAB,
	CURSOR_POUR,
	CURSOR_BODY,
	CURSOR_ARROWS,
	CURSOR_CROSSHAIR,
	CURSOR_MAX
};

extern uint16 cursor_x;
extern byte cursor_y;
extern byte cursor_color;
extern byte *cursor_shape;
extern byte cursor_anim_ticks;
extern byte cursor_anim_phase;

void SelectCursor(uint16 num);
void UpdateCursor(void);
void DrawCursor(byte *target);
void UndrawCursor(byte *target);
void UpdateUndrawCursor(byte *target);

} // End of namespace Chamber

#endif
