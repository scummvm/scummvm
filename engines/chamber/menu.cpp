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

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/menu.h"
#include "chamber/cga.h"
#include "chamber/input.h"
#include "chamber/cursor.h"
#include "chamber/room.h"
#include "chamber/sound.h"
#include "chamber/script.h"

namespace Chamber {


unsigned char act_menu_x = 0;
unsigned char act_menu_y = 0;

rect_t *act_dot_rects_cur;
rect_t *act_dot_rects_end;

/*choice dots placement on actions menu*/
rect_t act_dot_rects[8 + 1];
struct {
	unsigned char x, y;
} act_dot_coords[8] = {
	{ 2,  0},
	{ 8, 32},
	{ 2, 32},
	{10,  8},
	{ 0, 24},
	{ 8,  0},
	{ 0,  8},
	{10, 24}
};

unsigned char act_menu_x;
unsigned char act_menu_y;

/*Handle keyboard keys in actions menu (to cycle through choices with directional keys)*/
unsigned char PollKeyboardInActionsMenu(void) {
	if (!key_direction) {
		key_held = 0;
		return key_code;
	}

	if (key_held)
		return key_code;

	key_held = 1;

	/*cycle through menu choices*/

	cursor_x = act_dot_rects_cur->sx * 4 + 1;
	cursor_y = act_dot_rects_cur->sy + 4;

	if (++act_dot_rects_cur == act_dot_rects_end)
		act_dot_rects_cur = act_dot_rects;

	return key_code;
}

/*Handle player input in actions menu*/
void PollInputInActionsMenu(void) {
	unsigned char keys;
	if (have_mouse)
		keys = PollMouse();
	else
		keys = PollKeyboardInActionsMenu();
	SetInputButtons(keys);
}

/*Draw actions menu and process its choices*/
void ActionsMenu(unsigned char **pinfo) {
	unsigned char x, y;
	unsigned char choices;
	int i, choice, numchoices;
	unsigned char *menurecs;

	last_object_hint = object_hint;

	if (act_menu_x == 0xFF) {
		/*recalc menu pos*/
		x = cursor_x / 4;
		if ((int)(x + 48 / 4) >= 296 / 4)
			x -= x + 48 / 4 - 296 / 4;
		if (x < 32 / 4)
			x = 32 / 4;
		act_menu_x = x;

		y = cursor_y;
		if ((int)(y + 45) >= 180)
			y -= y + 45 - 180;
		act_menu_y = y;
	}

	x = act_menu_x;
	y = act_menu_y;

	/*menu sprite*/
	BackupAndShowSprite(0, x, y);
	PlaySound(18);

	choices = *((*pinfo)++);
	menurecs = *pinfo;

	numchoices = 0;
	for (i = 0; i < 8; i++) {
		if (choices & (1 << i)) {
			act_dot_rects[numchoices].sx = x + act_dot_coords[i].x;
			act_dot_rects[numchoices].ex = act_dot_rects[numchoices].sx + 8 / 4;
			act_dot_rects[numchoices].sy = y + act_dot_coords[i].y;
			act_dot_rects[numchoices].ey = act_dot_rects[numchoices].sy + 8;
			numchoices++;
		}
	}
	act_dot_rects[numchoices].sx = x + 24 / 4;
	act_dot_rects[numchoices].sy = y + 22;
	/*TODO: no ex/ey?*/
	act_dot_rects_end = act_dot_rects + numchoices + 1;

	for (i = 0; i < numchoices; i++)
		DrawSpriteN(1, act_dot_rects[i].sx, act_dot_rects[i].sy, CGA_SCREENBUFFER);

	SelectCursor(CURSOR_0);
	ProcessInput();

	choice = 0;
	act_dot_rects_cur = act_dot_rects;
	do {
		PollInputInActionsMenu();

		for (choice = 0; choice < numchoices; choice++) {
			if (IsCursorInRect(&act_dot_rects[choice]))
				break;
		}
		if (choice < numchoices) {
			cursor_color = 0xAA;
			command_hint = menurecs[choice * 3];
			the_command = (menurecs[choice * 3 + 1] << 8) | menurecs[choice * 3 + 2];
		} else {
			cursor_color = 0xFF;
			command_hint = 100;
			the_command = 0xFFFF;
		}

		if (command_hint != last_command_hint)
			DrawCommandHint();  /*to backbuffer*/
		DrawHintsAndCursor(CGA_SCREENBUFFER);
	} while (buttons == 0);
	UndrawCursor(CGA_SCREENBUFFER);

	if (the_command != 0xFFFF) {
		PlaySound(19);
		WaitVBlank();

		/*draw dot explosion animation*/
		DrawSpriteN(24, act_dot_rects[choice].sx, act_dot_rects[choice].sy, CGA_SCREENBUFFER);
		for (i = 0; i < 0xFFF; i++) ; /*TODO: weak delay*/
		DrawSpriteN(2, act_dot_rects[choice].sx, act_dot_rects[choice].sy, CGA_SCREENBUFFER);
		for (i = 0; i < 0xFFF; i++) ; /*TODO: weak delay*/
		DrawSpriteN(25, act_dot_rects[choice].sx, act_dot_rects[choice].sy, CGA_SCREENBUFFER);
		for (i = 0; i < 0xFFF; i++) ; /*TODO: weak delay*/
	}
	CGA_RestoreBackupImage(CGA_SCREENBUFFER);

	*pinfo += numchoices * 3;
}

/*TODO: maybe rename to SpotsLoop*/
void MenuLoop(unsigned char spotmask, unsigned char spotvalue) {
	ProcessInput();
	do {
		PollInput();
		CheckHotspots(spotmask, spotvalue);
		if (object_hint != last_object_hint)
			DrawObjectHint();
		DrawHintsAndCursor(frontbuffer);
	} while (buttons == 0);
	UndrawCursor(frontbuffer);
}

void ProcessMenu(void) {
	SelectCursor(CURSOR_6);
	MenuLoop(SPOTFLG_80 | SPOTFLG_20 | SPOTFLG_10 | SPOTFLG_8, SPOTFLG_80 | SPOTFLG_10);
}


rect_t menu_buttons_rects[] = {
	{296 / 4, 312 / 4,  15,  30},   /*Room's Objects*/
	{296 / 4, 312 / 4,  40,  56},   /*Psi Powers*/
	{296 / 4, 312 / 4,  56,  72},   /*Posessions*/
	{296 / 4, 312 / 4,  72,  88},   /*Energy Level*/
	{296 / 4, 312 / 4,  88, 104},   /*Wait*/
	{296 / 4, 312 / 4, 104, 120},   /*Load*/
	{296 / 4, 312 / 4, 120, 136},   /*Save*/
	{296 / 4, 312 / 4, 136, 152}    /*Time*/
};

void CheckMenuCommandHover(void) {
	int i;
	for (i = 0; i < 8; i++) {
		if (IsCursorInRect(&menu_buttons_rects[i])) {
			the_command = 0xA001 + i;
			command_hint = i ? (i + 3) : 101;
			cursor_color = 0xAA;
			return;
		}
	}
	/*nothing found*/
	command_hint = 100;
	cursor_color = 0xFF;
	the_command = 0;
}

rect_t psi_buttons_rects[] = {
	{280 / 4, 296 / 4,  40,  56},   /*Solar Eyes*/
	{280 / 4, 296 / 4,  56,  72},   /*Sticky Fingers*/
	{280 / 4, 296 / 4,  72,  88},   /*Know Mind*/
	{280 / 4, 296 / 4,  88, 104},   /*Brainwarp*/
	{280 / 4, 296 / 4, 104, 120},   /*Zone Scan*/
	{280 / 4, 296 / 4, 120, 136},   /*Psi Shift*/
	{280 / 4, 296 / 4, 136, 152},   /*Extreme Violence*/
	{280 / 4, 296 / 4, 152, 168}    /*Tune In*/
};

void CheckPsiCommandHover(void) {
	/*TODO: maybe merge it with CheckMenuCommandHover()*/
	int i;
	for (i = 0; i < 8; i++) {
		if (IsCursorInRect(&psi_buttons_rects[i])) {
			the_command = 0xA00A + i;
			command_hint = i + 12;
			cursor_color = 0xAA;
			return;
		}
	}
	/*nothing found*/
	command_hint = 100;
	cursor_color = 0xFF;
	the_command = 0;
}

} // End of namespace Chamber
