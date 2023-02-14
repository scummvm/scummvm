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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <setjmp.h>

#include "common/error.h"
#include "common/system.h"
#include "engines/util.h"

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
#include "chamber/ifgm.h"

namespace Chamber {

uint16 cpu_speed_delay;

/*
Prompt user to insert disk #2 to any drive
*/
void askDisk2(void) {
	drawMessage(seekToString(vepci_data, 179), frontbuffer);
}

void saveToFile(char *filename, void *data, uint16 size) {
	warning("STUB: SaveToFile(%s, data, %d)", filename, size);
#if 0
	FILE *f = fopen(filename, "wb");
	fwrite(data, 1, size, f);
	fclose(f);
#endif
}

int16 loadSplash(const char *filename) {
	if (!loadFile(filename, scratch_mem1))
		return 0;
	decompress(scratch_mem1 + 8, backbuffer);   /* skip compressed/decompressed size fields */
	return 1;
}

uint16 benchmarkCpu(void) {
	byte t;
	uint16 cycles = 0;
	for (t = script_byte_vars.timer_ticks; t == script_byte_vars.timer_ticks;) ;
	for (t = script_byte_vars.timer_ticks; t == script_byte_vars.timer_ticks;) cycles++;
	return cycles;
}

void randomize(void) {
	warning("STUB: Randomize()");
#if 0
	union REGS reg;

	reg.h.ah = 0;
	int86(0x1A, &reg, &reg);
	rand_seed = reg.h.dl;
	Rand();
#endif
}

void TRAP() {
	promptWait();
	for (;;) ;
}

/* Main Game Loop */
void gameLoop(byte *target) {
	for (;;) {
		animateSpots(target);

		/* Update/check live things */
		updateProtozorqs();
		checkGameTimeLimit();
		cleanupDroppedItems();

		/* Get player input */
		pollInput();

		if (g_vm->_shouldQuit)
			return;

		the_command = 0;
		if (isCursorInRect(&room_bounds_rect)) {
			selectCursor(CURSOR_TARGET);
			command_hint = 100;
			selectSpotCursor();
		} else {
			selectCursor(CURSOR_FINGER);
			object_hint = 117;
			checkMenuCommandHover();
		}

		if (object_hint != last_object_hint)
			drawObjectHint();

		if (command_hint != last_command_hint)
			drawCommandHint();

		drawHintsAndCursor(target);

		if (!buttons || !the_command) {
			/*Pending / AI commands*/

			if (script_byte_vars.check_used_commands < script_byte_vars.used_commands) {
				the_command = Swap16(script_word_vars.next_aspirant_cmd);
				if (the_command)
					goto process;
			}

			if (script_byte_vars.bvar_45)
				continue;

			the_command = Swap16(script_word_vars.next_protozorqs_cmd);
			if (the_command)
				goto process;

			if (Swap16(next_vorts_ticks) < script_word_vars.timer_ticks2) { /*TODO: is this ok? ticks2 is BE, ticks3 is LE*/
				the_command = next_vorts_cmd;
				if (the_command)
					goto process;
			}

			if (Swap16(next_turkey_ticks) < script_word_vars.timer_ticks2) { /*TODO: is this ok? ticks2 is BE, ticks4 is LE*/
				the_command = next_turkey_cmd;
				if (the_command)
					goto process;
			}

			continue;

process:
			;
			updateUndrawCursor(target);
			refreshSpritesData();
			runCommand();
			blitSpritesToBackBuffer();
			processInput();
			drawSpots(target);
		} else {
			/*Player action*/

			updateUndrawCursor(target);
			refreshSpritesData();
			runCommandKeepSp();
			script_byte_vars.used_commands++;
			if (script_byte_vars.dead_flag) {
				if (--script_byte_vars.tries_left == 0)
					resetAllPersons();
			}
			blitSpritesToBackBuffer();
			processInput();
			drawSpots(target);
		}
	}
}


void exitGame(void) {
	switchToTextMode();
	uninitTimer();
}

jmp_buf restart_jmp;

#ifdef DEBUG_ENDING
extern theEnd(void);
#endif

Common::Error ChamberEngine::run() {
	byte c;

	// Initialize graphics using following:
	initGraphics(320, 200);
	initSound();

	/*TODO: DetectCPU*/

	IFGM_Init();

	switchToGraphicsMode();

	/* Install timer callback */
	initTimer();

	if (g_vm->getLanguage() == Common::EN_USA) {
		/* Load title screen */
		if (!loadSplash("PRESCGA.BIN"))
			exitGame();

		if (ifgm_loaded) {
			/*TODO*/
		}
	} else {
		/* Load title screen */
		if (!loadSplash("PRES.BIN"))
			exitGame();
	}

	/* Select intense cyan-mageta palette */
	cga_ColorSelect(0x30);

	/* Show the title screen */
	cga_BackBufferToRealFull();

	if (g_vm->getLanguage() == Common::EN_USA) {
		if (ifgm_loaded) {
			/*TODO*/
		}

		/* Force English language */
		c = 'E';
	} else {
		/* Load language selection screen */
		if (!loadSplash("DRAP.BIN"))
			exitGame();

		/* Wait for a keypress and show the language selection screen */
		clearKeyboard();
		readKeyboardChar();

		if (_shouldQuit)
			return Common::kNoError;

		cga_BackBufferToRealFull();
		clearKeyboard();

		/* Wait for a valid language choice */
		do {
			c = readKeyboardChar();
			if (c > 'F')
				c -= ' ';
		} while (c < 'D' || c > 'F');
	}

	if (_shouldQuit)
		return Common::kNoError;

	/* Patch resource names for choosen language */
	res_texts[0].name[4] = c;
	res_texts[1].name[4] = c;
	res_desci[0].name[4] = c;
	res_diali[0].name[4] = c;

	if (g_vm->getLanguage() != Common::EN_USA)
		cga_BackBufferToRealFull();

	/* Load script and other static resources */
	/* Those are normally embedded in the executable, but here we load extracted ones*/
	if (!loadStaticData())
		exitGame();

	/* Load text resources */
	if (!loadVepciData() || !loadDesciData() || !loadDialiData())
		exitGame();

	/* Detect/Initialize input device */
	initInput();

	/* Load graphics resources */
	while (!loadFond() || !loadSpritesData() || !loadPersData())
		askDisk2();

	/*TODO: is this neccessary?*/
	cga_BackBufferToRealFull();

	/* Create clean game state snapshot */
	saveRestartGame();

	/* Detect CPU speed for delay routines */
	cpu_speed_delay = benchmarkCpu() / 8;

	if (g_vm->getLanguage() == Common::EN_USA) {
		if (ifgm_loaded) {
			/*TODO*/
		}
	}

	/*restart game from here*/
//restart:;
	setjmp(restart_jmp);

	randomize();

	/* Set start zone */
	script_byte_vars.zone_index = 7;

	/* Begin the game */
	script_byte_vars.game_paused = 0;

#ifdef DEBUG_SCRIPT
	unlink(DEBUG_SCRIPT_LOG);
#endif

#ifdef DEBUG_SKIP_INTRO
	/*bypass characters introduction*/
	script_byte_vars.load_flag = DEBUG_SKIP_INTRO;
#endif

	/* Discard all pending input */
	//ResetInput();

	/* Play introduction sequence and initialize game */
	the_command = 0xC001;
	runCommand();

	if (_shouldQuit)
		return Common::kNoError;

	/* Sync graphics */
	blitSpritesToBackBuffer();

	/* Initialize cursor backup */
	processInput();

#ifdef DEBUG_ENDING
	script_byte_vars.game_paused = 5;
	theEnd();
	for (;;) ;
#endif

	/* Main game loop */
	gameLoop(frontbuffer);

	/*TODO: the following code is never executed since gameLoop is infinite (or the whole game is restarted) */

	/* Release hardware */
	uninitInput();

	exitGame();

	return Common::kNoError;
}

} // End of namespace Chamber
