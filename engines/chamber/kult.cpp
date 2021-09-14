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
#include "chamber/decompr.h"
#include "chamber/cga.h"
#include "chamber/anim.h"
#include "chamber/cursor.h"
#include "chamber/input.h"
#include "chamber/timer.h"
#include "chamber/portrait.h"
#include "chamber/room.h"
#include "chamber/savegame.h"
#include "chamber/resdata.h"
#include "chamber/script.h"
#include "chamber/print.h"
#include "chamber/dialog.h"
#include "chamber/menu.h"

namespace Chamber {


unsigned short cpu_speed_delay;

/*
Prompt user to insert disk #2 to any drive
*/
void AskDisk2(void) {
	DrawMessage(SeekToString(vepci_data, 179), frontbuffer);
}

void SaveToFile(char *filename, void *data, unsigned int size) {
	FILE *f = fopen(filename, "wb");
	fwrite(data, 1, size, f);
	fclose(f);
}

int LoadSplash(char *filename) {
	if (!LoadFile(filename, scratch_mem1))
		return 0;
	decompress(scratch_mem1 + 8, backbuffer);   /* skip compressed/decompressed size fields */
	return 1;
}

unsigned int BenchmarkCpu(void) {
	unsigned char t;
	unsigned int cycles = 0;
	for (t = script_byte_vars.timer_ticks; t == script_byte_vars.timer_ticks;) ;
	for (t = script_byte_vars.timer_ticks; t == script_byte_vars.timer_ticks;) cycles++;
	return cycles;
}

void Randomize(void) {
	union REGS reg;

	reg.h.ah = 0;
	int86(0x1A, &reg, &reg);
	rand_seed = reg.h.dl;
	Rand();
}

void TRAP() {
	PromptWait();
	for (;;) ;
}

/* Main Game Loop */
void GameLoop(unsigned char *target) {
	for (;;) {
#if 1
		AnimateSpots(target);
#endif
		/* Update/check live things */
		UpdateTimedRects1();
		UpdateTimedRects2();
		UpdateTimedInventoryItems();

		/* Get player input */
		PollInput();

		the_command = 0;
		if (IsCursorInRect(&room_bounds_rect)) {
			SelectCursor(CURSOR_1);
			command_hint = 100;
			SelectSpotCursor();
		} else {
			SelectCursor(CURSOR_0);
			object_hint = 117;
			CheckMenuCommandHover();
		}

		if (object_hint != last_object_hint)
			DrawObjectHint();

		if (command_hint != last_command_hint)
			DrawCommandHint();

		DrawHintsAndCursor(target);

		if (!buttons || !the_command) {
			/*Pending / AI commands*/

			if (script_byte_vars.check_used_commands < script_byte_vars.used_commands) {
				the_command = Swap16(script_word_vars.next_command1);
				if (the_command)
					goto process;
			}

			if (script_byte_vars.flag_179FB)
				continue;

			the_command = Swap16(script_word_vars.next_command2);
			if (the_command)
				goto process;

			if (Swap16(next_ticks3) < script_word_vars.timer_ticks2) { /*TODO: is this ok? ticks2 is BE, ticks3 is LE*/
				the_command = next_command3;
				if (the_command)
					goto process;
			}

			if (Swap16(next_ticks4) < script_word_vars.timer_ticks2) { /*TODO: is this ok? ticks2 is BE, ticks4 is LE*/
				the_command = next_command4;
				if (the_command)
					goto process;
			}

			continue;

process:
			;
			UpdateUndrawCursor(target);
			RefreshSpritesData();
			RunCommand();
			BlitSpritesToBackBuffer();
			ProcessInput();
			DrawSpots(target);
		} else {
			/*Player action*/

			UpdateUndrawCursor(target);
			RefreshSpritesData();
			RunCommandKeepSp();
			script_byte_vars.used_commands++;
			if (script_byte_vars.dead_flag) {
				if (--script_byte_vars.tries_left == 0)
					ResetAllPersons();
			}
			BlitSpritesToBackBuffer();
			ProcessInput();
			DrawSpots(target);
		}
	}
}


void ExitGame(void) {
	SwitchToTextMode();
	exit(0);
}

void main(void) {
	unsigned char c;

	SwitchToGraphicsMode();

	/* Load title screen */
	if (!LoadSplash("PRES.BIN"))
		ExitGame();

	/* Select intense cyan-mageta palette */
	CGA_ColorSelect(0x30);

	/* Show the title screen */
	CGA_BackBufferToRealFull();

#ifdef COPYPROT
	/* Check if a valid floppy disk is present in any drive */
	if (!CheckCopyProtection()) for (;;) ;
#endif

	/* Load language selection screen */
	if (!LoadSplash("DRAP.BIN"))
		ExitGame();

	/* Wait for a keypress and show the language selection screen */
	ClearKeyboard();
	ReadKeyboardChar();
	CGA_SwapRealBackBuffer();
	ClearKeyboard();

	/* Wait for a valid language choice */
	do {
		c = ReadKeyboardChar();
		if (c > 'F')
			c -= ' ';
	} while (c < 'D' || c > 'F');

	/* Patch resource names for choosen language */
	res_texts[0].name[4] = c;
	res_texts[1].name[4] = c;
	res_desci[0].name[4] = c;
	res_diali[0].name[4] = c;

	CGA_BackBufferToRealFull();

	/* Load script and other static resources */
	/* Those are normally embedded in the executable, but here we load extracted ones*/
	if (!LoadStaticData())
		ExitGame();

	/* Load text resources */
	if (!LoadVepciData() || !LoadDesciData() || !LoadDialiData())
		ExitGame();

	/* Load graphics resources */
	while (!LoadFond() || !LoadSpritesData() || !LoadPersData())
		AskDisk2();

	/* Detect/Initialize input device */
	InitInput();

	/*TODO: is this neccessary?*/
	CGA_BackBufferToRealFull();

	/* Create clean game state snapshot */
	SaveRestartGame();

	/* Install timer callback */
	InitTimer();

	/* Detect CPU speed for delay routines */
	cpu_speed_delay = BenchmarkCpu() / 8;

	/*restart game from here*/
restart:
	;
	Randomize();

	/* Set start zone */
	script_byte_vars.zone_index = 7;

	/* Begin the game */
	script_byte_vars.game_paused = 0;

#ifdef DEBUG_SCRIPT
	{
		FILE *f = fopen(DEBUG_SCRIPT_LOG, "wt+");
		if (f) {
			fclose(f);
		}
	}
#endif

#if 1
	/* Play introduction sequence */
	the_command = 0xC001;
	RunCommand();
#endif

	/* Sync graphics */
	BlitSpritesToBackBuffer();

	/* Initialize cursor backup */
	ProcessInput();

	/* Main game loop */
	GameLoop(frontbuffer);

	/*TODO: the following code is never executed since GameLoop is infinite (or the whole game is restarted) */

	/* Release hardware */
	UninitInput();
	UninitTimer();

	ExitGame();
}

} // End of namespace Chamber
