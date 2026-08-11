/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"
#include "common/error.h"
#include "common/file.h"
#include "common/system.h"
#include "engines/advancedDetector.h"
#include "engines/util.h"

#include "chamber/chamber.h"
#include "chamber/detection.h"
#include "chamber/common.h"
#include "chamber/decompr.h"
#include "chamber/cga.h"
#include "chamber/ega.h"
#include "chamber/amiga.h"
#include "chamber/ega_resource.h"
#include "chamber/anim.h"
#include "chamber/cursor.h"
#include "chamber/input.h"
#include "chamber/timer.h"
#include "chamber/portrait.h"
#include "chamber/room.h"
#include "chamber/saveload.h"
#include "chamber/resdata.h"
#include "chamber/script.h"
#include "chamber/print.h"
#include "chamber/dialog.h"
#include "chamber/menu.h"
#include "chamber/ifgm.h"
#include "graphics/surface.h"
#include "graphics/paletteman.h"

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

Graphics::Surface *loadSplash(const char *filename) {
	if (!loadFile(filename, scratch_mem1))
		return nullptr;

	Graphics::Surface *surface = new Graphics::Surface();
	int width = (g_vm->_videoMode == Common::kRenderHercG) ? 640 : 320;
	int height = 200;
	surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	decompress(scratch_mem1 + 8, backbuffer);

	for (int y = 0; y < CGA_HEIGHT; ++y) {
		byte *dst = (byte *)surface->getBasePtr(0, y);
		for (int x = 0; x < CGA_WIDTH; x += 4) {
			int cga_offset = (y % 2) * 8192 + (y / 2) * 80 + (x / 4);
			byte cga_byte = backbuffer[cga_offset];

			if (g_vm->_videoMode == Common::RenderMode::kRenderHercG) {
				byte colors = cga_byte;
				for (int i = 0; i < 8; i++) {
					byte bit = (colors & 0x80) >> 7;
					colors <<= 1;
					dst[x * 2 + i] = bit;
				}
			} else{
				for (int i = 0; i < 4; i++) {
					byte color = (cga_byte >> (6 - i * 2)) & 0x03;
					dst[x + i] = color;
				}
			}
		}
	}
	return surface;
}

uint16 benchmarkCpu(void) {
	byte t;
	uint16 cycles = 0;
	for (t = script_byte_vars.timer_ticks; t == script_byte_vars.timer_ticks;) ;
	for (t = script_byte_vars.timer_ticks; t == script_byte_vars.timer_ticks;) cycles++;
	return cycles;
}

void randomize(void) {
	// Original read the low byte of the BIOS timer-tick count (int 0x1A) into
	// rand_seed, the starting offset into the fixed aleat_data[] table that
	// getRand() walks. Keep that table (it reproduces the DOS game's exact
	// random distribution) but derive the offset from ScummVM's RandomSource:
	// its constructor already honors the "random_seed" config key for
	// reproducible runs and otherwise seeds from the time/date.
	rand_seed = (byte)g_vm->_rnd->getRandomNumber(255);
	getRand();
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
			g_vm->_renderer->selectCursor(CURSOR_TARGET);
			command_hint = 100;
			selectSpotCursor();
		} else {
			g_vm->_renderer->selectCursor(CURSOR_FINGER);
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
			if (the_command) {
				// Consume the queued command so a terminal command (e.g. the
				// "failed the ordeals" death scene queued by checkGameTimeLimit)
				// fires once instead of every frame. updateProtozorqs() re-queues
				// live protozorq AI each frame, but it early-returns once
				// bvar_26 >= 63 and would otherwise leave this set forever.
				script_word_vars.next_protozorqs_cmd = 0;
				goto process;
			}

			// next_vorts_ticks is a plain numeric value (always set as
			// Swap16(timer_ticks2) + 5), but timer_ticks2 is stored big-endian.
			// Byteswap the timer side so both operands are numeric; the old
			// Swap16(next_vorts_ticks) compared two BE representations, so the
			// vort command fired at the wrong times (vorts popping in/out of
			// rooms and leaving artifacts from the interrupted walk anim).
			if (next_vorts_ticks < Swap16(script_word_vars.timer_ticks2)) {
				the_command = next_vorts_cmd;
				if (the_command)
					goto process;
			}

			if (next_turkey_ticks < Swap16(script_word_vars.timer_ticks2)) {
				the_command = next_turkey_cmd;
				if (the_command)
					goto process;
			}

			continue;

process:
			;
			updateUndrawCursor(target);
			refreshSpritesData();
			// Drain priority commands at this main-loop baseline too: a queued
			// AI command (e.g. the timed "failed the ordeals" death scene) may
			// fire a priority command, which runCommand now propagates up to a
			// runCommandKeepSp anchor instead of running it nested.
			runCommandKeepSp();
			if (g_vm->_shouldRestart)
				return;
			blitSpritesToBackBuffer();
			processInput();
			drawSpots(target);
		} else {
			/*Player action*/

			updateUndrawCursor(target);
			refreshSpritesData();
			uint16 restart = runCommandKeepSp();
			clearButtons();
			if (restart == RUNCOMMAND_RESTART && g_vm->_shouldRestart)
				return;
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

#ifdef DEBUG_ENDING
extern theEnd(void);
#endif

Common::Error ChamberEngine::init() {
	byte c;

	// Initialize graphics using following:
	bool isCustomHerc = false;
	if (isEgaLikeRenderer()) {
		// EGA and Amiga use the chunky 8bpp pipeline (1 byte/pixel, linear lines)
		_screenW = 320;
		_screenH = 200;
		_screenBits = 8;
		_screenPPB = 1;
		_screenBPL = _screenW;
		_line_offset = 0;
		_line_offset2 = 0;
		_fontHeight = 6;
		_fontWidth = 4;
		initGraphics(_screenW, _screenH);
	} else {
		if (_videoMode == Common::RenderMode::kRenderHercG) {
			isCustomHerc = true;
			_videoMode = Common::RenderMode::kRenderCGA;
		}
		_screenW = 320;
		_screenH = 200;
		_screenBits = 2;
		_screenPPB = 8 / _screenBits;
		_screenBPL = _screenW / _screenPPB;
		_line_offset = 0x2000;
		_line_offset2 = 0x2000;
		_fontHeight = 6;
		_fontWidth = 4;
		if (isCustomHerc) {
			initGraphics(720, 348);
		} else {
			initGraphics(_screenW, _screenH);
		}
	}

	initSound();

	/*TODO: DetectCPU*/

	IFGM_Init();

	g_vm->_renderer->switchToGraphicsMode();

	/* Install timer callback */
	initTimer();

	Graphics::Surface *splash = nullptr;

	if (getPlatform() == Common::kPlatformAmiga) {
		// Amiga title: static resources (incl. palette) live in KULT, load them first
		loadAmigaStaticData();
		if (getLanguage() == Common::EN_USA) {
			splash = ega_loadFond("INTRO.PIA");
			if (splash) {
				g_vm->_renderer->colorSelect(AMIGA_NUM_PALETTES - 1);
				g_vm->_renderer->backBufferToRealFull();
				// Splash wraps ega_backbuffer: the surface does not own its pixels
				delete splash;
				splash = nullptr;
				clearKeyboard();
				readKeyboardChar();
				if (_shouldQuit)
					return Common::kNoError;
			}
		}
		splash = ega_loadFond("PRES.BIN");
		if (!splash) {
			_shouldQuit = true;
			return Common::kNoError;
		}
		// Last palette index is the full-brightness end of the title fade ramp
		g_vm->_renderer->colorSelect(AMIGA_NUM_PALETTES - 1);
		g_vm->_renderer->backBufferToRealFull();
	} else if (_videoMode == Common::RenderMode::kRenderEGA) {
		/* EGA title screen */
		splash = ega_loadFond("PRESEGA.EGA");
		if (!splash) {
			_shouldQuit = true;
			return Common::kNoError;
		}
		g_vm->_renderer->colorSelect(0x30);
		g_vm->_renderer->backBufferToRealFull();
	} else if (_gameDescription->flags & GF_SPLASH_PRESCGA) {
		/* EN_USA CGA title screen */
		splash = loadSplash("PRESCGA.BIN");
		if (!splash) {
			_shouldQuit = true;
			return Common::kNoError;
		}

		if (ifgm_loaded) {
			/*TODO*/
		}

		if (!isCustomHerc) {
			g_vm->_renderer->colorSelect(0x30);
			g_vm->_renderer->backBufferToRealFull();
		} else {
			if (_renderMode == Common::kRenderHercG)
				g_system->getPaletteManager()->setPalette(Graphics::HGC_G_PALETTE, 0, 2);
			else
				g_system->getPaletteManager()->setPalette(Graphics::HGC_A_PALETTE, 0, 2);

			g_vm->_renderer->backBufferToRealFull();
		}
	} else {
		/* Multilingual CGA title screen */
		splash = loadSplash("PRES.BIN");
		if (!splash) {
			_shouldQuit = true;
			return Common::kNoError;
		}

		if (!isCustomHerc) {
			/* Select intense cyan-magenta palette */
			g_vm->_renderer->colorSelect(0x30);
			g_vm->_renderer->backBufferToRealFull();
		} else {
			if (_renderMode == Common::kRenderHercG)
				g_system->getPaletteManager()->setPalette(Graphics::HGC_G_PALETTE, 0, 2);
			else
				g_system->getPaletteManager()->setPalette(Graphics::HGC_A_PALETTE, 0, 2);

			g_vm->_renderer->backBufferToRealFull();
		}
	}

	if (splash) {
		// EGA/Amiga splashes wrap ega_backbuffer; only CGA/Herc own their pixels
		if (g_vm->_videoMode != Common::RenderMode::kRenderEGA &&
		    g_vm->_videoMode != Common::kRenderAmiga)
			splash->free();
		delete splash;
	}

	/* Wait for a keypress */
	clearKeyboard();
	readKeyboardChar();


	if (!(_gameDescription->flags & GF_SPLASH2_DRAP)) {
		if (ifgm_loaded) {
			/*TODO*/
		}

		/* Single-language variant — force English */
		c = 'E';
	} else {
		/* Load language selection screen */
		Graphics::Surface *drap = loadSplash("DRAP.BIN");
		if (!drap) {
			warning("File DRAP.BIN not found");
			_shouldQuit = true;
			return Common::kReadingFailed;
		}
		drap->free();
		delete drap;

		/* Wait for a keypress and show the language selection screen */
		clearKeyboard();
		readKeyboardChar();

		if (_shouldQuit)
			return Common::kNoError;

		g_vm->_renderer->backBufferToRealFull();
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

	/* Patch resource names for chosen language */
	res_texts[0].name[4] = c;
	res_texts[1].name[4] = c;
	res_desci[0].name[4] = c;
	res_diali[0].name[4] = c;

	if (g_vm->getLanguage() != Common::EN_USA)
		g_vm->_renderer->backBufferToRealFull();

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
	if (g_vm->_videoMode == Common::RenderMode::kRenderEGA) {
		/* EGA: load decoded sprite banks from external .EGA files */
		ega_sprit_res = new EgaSpriteResource();
		ega_sprit_res->appendFromFile("SPRIT.EGA");

		ega_puzzl_res = new EgaSpriteResource();
		ega_puzzl_res->appendFromFile("PUZZL.EGA");
		ega_puzzl_res->appendFromFile("PUZZ1.EGA");

		ega_perso_res = new EgaSpriteResource();
		ega_perso_res->appendFromFile("PERSO.EGA");

		Graphics::Surface *fond = loadFond();
		if (!fond)
			exitGame();
		/* fond wraps ega_backbuffer via init() — surface does not own pixel data, safe to delete directly */
		delete fond;
	} else if (g_vm->getPlatform() == Common::kPlatformAmiga) {
		// Amiga sprite banks (appendFromFileAmiga); PUZZL/perso banks are merged here
		ega_sprit_res = new EgaSpriteResource();
		ega_sprit_res->appendFromFileAmiga("SPRIT.BIN");

		ega_puzzl_res = new EgaSpriteResource();
		ega_puzzl_res->appendFromFileAmiga("PUZZL.BIN");

		ega_perso_res = new EgaSpriteResource();
		ega_perso_res->appendFromFileAmiga("A.BIN");
		ega_perso_res->appendFromFileAmiga("B.BIN");

		Graphics::Surface *fond = loadFond();
		if (!fond)
			exitGame();
		delete fond;

		// Early intro screens set no palette of their own; apply the room palette now
		amigaApplyRoomPalette(0);
	} else {
		Graphics::Surface *fond;
		while (!(fond = loadFond()) || !loadSpritesData() || !loadPersData()) {
			delete fond;
			askDisk2();
		}
		delete fond;
	}

	/*TODO: is this necessary?*/
	g_vm->_renderer->backBufferToRealFull();

	/* Create clean game state snapshot */
	saveRestartGame();

	/* Detect CPU speed for delay routines */
	cpu_speed_delay = benchmarkCpu() / 8;

	if (g_vm->getLanguage() == Common::EN_USA) {
		if (ifgm_loaded) {
			/*TODO*/
		}
	}

	return Common::kNoError;
}

Common::Error ChamberEngine::execute() {
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
	// Freeze the ordeal timer during the intro/setup: it is installed before this
	// point (initTimer), so the non-interactive intro would otherwise start the
	// player's one-hour ordeal budget early.
	script_byte_vars.game_paused = 1;
	the_command = 0xC001;
	runCommand();
	script_byte_vars.game_paused = 0;

	if (_shouldQuit)
		return Common::kNoError;

	/* Sync graphics */
	blitSpritesToBackBuffer();

	/* Initialize cursor backup */
	processInput();

	if (ConfMan.hasKey("save_slot") && ConfMan.getInt("save_slot") >= 0) {
		int slot = ConfMan.getInt("save_slot");
		ConfMan.set("save_slot", "-1");
		loadGameState(slot);
	}

#ifdef DEBUG_ENDING
	script_byte_vars.game_paused = 5;
	theEnd();
	for (;;) ;
#endif

	/* Main game loop */
	gameLoop(frontbuffer);
	if (g_vm->_shouldRestart)
		run();

	/*TODO: the following code is never executed since gameLoop is infinite (or the whole game is restarted) */

	/* Release hardware */
	uninitInput();

	exitGame();

	return Common::kNoError;
}
Common::Error ChamberEngine::run() {
	if (!g_vm->_shouldRestart)
		init();

	do {
		g_vm->_shouldRestart = false;
		execute();
	} while (g_vm->_shouldRestart);

	return Common::kNoError;
}

} // End of namespace Chamber
