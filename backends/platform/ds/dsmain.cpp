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


// - Remove scummconsole.c
// - Delete files

// - libcartreset
// - Alternative controls - tap for left click, double for right
// - Inherit the Earth?
// - Stereo audio?
// - Software scaler?
// - 100% scale

// - Arrow keys cause key events when keyboard enabled - Done
// - Mouse cursor display - Done
// - Disable scaler on options menu - Done
// - Fix scale icons on top screen - Done
// - Fseek optimisation? - No need
// - Fix agi hack to be cleaner - done
// - Fix not typing looong words - Done
// - Show keyboard by default in AGI games
// - Fix mouse moving when cursor on keyboard screen - Done
// - Fix 'fit' thingy always appearing - Done
// - check cine backbuffer code - Done
// - Add long filename support - Done
// - New icons
// - Add key config for gob engine: Start:F1, Shift-numbers: F keys - Done
// - Fix [ds] appearing in game menu

// - Find out what's going wrong when you turn the console off
// - enable console when asserting

// - Alternative controls?


// - Fix 512x256 backbuffer to 320x240 - Done
// - Fix keyboard appearing on wrong screen - Done
// - Volume amplify option
// - Make save/restore game screen use scaler buffer


// 1.0.0!
// - Fix text on tabs on config screen
// - Remove ini file debug msg
// - Memory size for ite
// - Try discworld?


// Allow use of stuff in <nds.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h


#include <nds.h>
#include <filesystem.h>

#include <stdlib.h>
#include <string.h>

#include "dsmain.h"
#include "osystem_ds.h"
#include "engines/engine.h"

#include "backends/plugins/ds/ds-provider.h"
#include "base/main.h"
#include "base/version.h"
#include "common/util.h"

namespace DS {

// From console.c in NDSLib

// Defines
#define FRAME_TIME 17
#define SCUMM_GAME_HEIGHT 142
#define SCUMM_GAME_WIDTH 227

static int frameCount;
static int currentTimeMillis;

// Timer Callback
static int callbackInterval;
static int callbackTimer;
static OSystem_DS::TimerProc callback;

// Scaled
static int scX;
static int scY;

static int subScX;
static int subScY;
static int subScTargetX;
static int subScTargetY;
static int subScreenWidth = SCUMM_GAME_WIDTH;
static int subScreenHeight = SCUMM_GAME_HEIGHT;
static int subScreenScale = 256;

static bool gameScreenSwap = false;

// Shake
static int s_shakeXOffset = 0;
static int s_shakeYOffset = 0;

// Touch
static int touchScX, touchScY, touchX, touchY;

// 8-bit surface size
static int gameWidth = 320;
static int gameHeight = 200;

void setGameScreenSwap(bool enable) {
	gameScreenSwap = enable;
}

void setTopScreenZoom(int percentage) {
	s32 scale = (percentage << 8) / 100;
	subScreenScale = (256 * 256) / scale;
}

void setGameSize(int width, int height) {
	gameWidth = width;
	gameHeight = height;
}

int getGameWidth() {
	return gameWidth;
}

int getGameHeight() {
	return gameHeight;
}

void displayMode8Bit() {
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000);

	if (g_system->getGraphicsMode() == GFX_SWSCALE) {
		REG_BG3CNT = BG_BMP16_256x256 | BG_BMP_BASE(8);

		REG_BG3PA = 256;
		REG_BG3PB = 0;
		REG_BG3PC = 0;
		REG_BG3PD = (int) ((200.0f / 192.0f) * 256);

	} else {
		REG_BG3CNT = BG_BMP8_512x256 | BG_BMP_BASE(8);

		REG_BG3PA = (int) (((float) (gameWidth) / 256.0f) * 256);
		REG_BG3PB = 0;
		REG_BG3PC = 0;
		REG_BG3PD = (int) ((200.0f / 192.0f) * 256);
	}

#ifdef DISABLE_TEXT_CONSOLE
	REG_BG3CNT_SUB = BG_BMP8_512x256;

	REG_BG3PA_SUB = (int) (subScreenWidth / 256.0f * 256);
	REG_BG3PB_SUB = 0;
	REG_BG3PC_SUB = 0;
	REG_BG3PD_SUB = (int) (subScreenHeight / 192.0f * 256);
#endif

	if (gameScreenSwap) {
		lcdMainOnTop();
	} else {
		lcdMainOnBottom();
	}
}

void setShakePos(int shakeXOffset, int shakeYOffset) {
	s_shakeXOffset = shakeXOffset;
	s_shakeYOffset = shakeYOffset;
}

void doTimerCallback() {
	if (callback) {
		if (callbackTimer <= 0) {
			callbackTimer += callbackInterval;
			callback(callbackInterval);
		}
	}
}

Common::Point warpMouse(int penX, int penY, bool isOverlayShown) {
	int storedMouseX, storedMouseY;
	if (!isOverlayShown) {
		storedMouseX = ((penX - touchX) << 8) / touchScX;
		storedMouseY = ((penY - touchY) << 8) / touchScY;
	} else {
		storedMouseX = penX;
		storedMouseY = penY;
	}

	return Common::Point(storedMouseX, storedMouseY);
}

void setMainScreenScroll(int x, int y) {
		REG_BG3X = x + (((frameCount & 1) == 0)? 64: 0);
		REG_BG3Y = y;

		if (!gameScreenSwap) {
			touchX = x >> 8;
			touchY = y >> 8;
		}
}

void setMainScreenScale(int x, int y) {
		if ((g_system->getGraphicsMode() == GFX_SWSCALE) && (x==320)) {
			REG_BG3PA = 256;
			REG_BG3PB = 0;
			REG_BG3PC = 0;
			REG_BG3PD = y;
		} else {
			REG_BG3PA = x;
			REG_BG3PB = 0;
			REG_BG3PC = 0;
			REG_BG3PD = y;
		}

		if (!gameScreenSwap) {
			touchScX = x;
			touchScY = y;
		}
}

void setZoomedScreenScroll(int x, int y, bool shake) {
		if (gameScreenSwap) {
			touchX = x >> 8;
			touchY = y >> 8;
		}

#ifdef DISABLE_TEXT_CONSOLE
		REG_BG3X_SUB = x + ((shake && (frameCount & 1) == 0)? 64: 0);
		REG_BG3Y_SUB = y;
#endif
}

void setZoomedScreenScale(int x, int y) {
		if (gameScreenSwap) {
			touchScX = x;
			touchScY = y;
		}

#ifdef DISABLE_TEXT_CONSOLE
		REG_BG3PA_SUB = x;
		REG_BG3PB_SUB = 0;
		REG_BG3PC_SUB = 0;
		REG_BG3PD_SUB = y;
#endif
}

Common::Point transformPoint(uint16 x, uint16 y, bool isOverlayShown) {
	if (!isOverlayShown) {
		x = ((x * touchScX) >> 8) + touchX;
		x = CLIP<uint16>(x, 0, gameWidth  - 1);

		y = ((y * touchScY) >> 8) + touchY;
		y = CLIP<uint16>(y, 0, gameHeight - 1);
	}

	return Common::Point(x, y);
}

void VBlankHandler(void) {
	frameCount++;

	if (callback) {
		callbackTimer -= FRAME_TIME;
	}

	int xCenter = subScTargetX + ((subScreenWidth >> 1) << 8);
	int yCenter = subScTargetY + ((subScreenHeight >> 1) << 8);

	subScreenWidth = (256 * subScreenScale) >> 8;
	subScreenHeight = (192 * subScreenScale) >> 8;

	if ( ((subScreenWidth) > 256 - 8) && ((subScreenWidth) < 256 + 8) ) {
		subScreenWidth = 256;
		subScreenHeight = 192;
	} else if ( ((subScreenWidth) > 128 - 8) && ((subScreenWidth) < 128 + 8) ) {
		subScreenWidth = 128;
		subScreenHeight = 96;
	} else if (subScreenWidth > 256) {
		subScreenWidth = 320;
		subScreenHeight = 200;
	}

	subScTargetX = xCenter - ((subScreenWidth  >> 1) << 8);
	subScTargetY = yCenter - ((subScreenHeight >> 1) << 8);

	subScTargetX = CLIP(subScTargetX, 0, (gameWidth  - subScreenWidth)  << 8);
	subScTargetY = CLIP(subScTargetY, 0, (gameHeight - subScreenHeight) << 8);

	subScX += (subScTargetX - subScX) >> 2;
	subScY += (subScTargetY - subScY) >> 2;

	if (g_system->getGraphicsMode() == GFX_NOSCALE) {
		if (scX + 256 > gameWidth - 1) {
			scX = gameWidth - 1 - 256;
		}

		if (scX < 0) {
			scX = 0;
		}

		if (scY + 192 > gameHeight - 1) {
			scY = gameHeight - 1 - 192;
		}

		if (scY < 0) {
			scY = 0;
		}

		setZoomedScreenScroll(subScX, subScY, (subScreenWidth != 256) && (subScreenWidth != 128));
		setZoomedScreenScale(subScreenWidth, ((subScreenHeight * (256 << 8)) / 192) >> 8);

		setMainScreenScroll((scX << 8) + (s_shakeXOffset << 8), (scY << 8) + (s_shakeYOffset << 8));
		setMainScreenScale(256, 256);		// 1:1 scale
	} else {
		if (scY > gameHeight - 192 - 1) {
			scY = gameHeight - 192 - 1;
		}

		if (scY < 0) {
			scY = 0;
		}

		setZoomedScreenScroll(subScX, subScY, (subScreenWidth != 256) && (subScreenWidth != 128));
		setZoomedScreenScale(subScreenWidth, ((subScreenHeight * (256 << 8)) / 192) >> 8);

		setMainScreenScroll(64 + (s_shakeXOffset << 8), (scY << 8) + (s_shakeYOffset << 8));
		setMainScreenScale(320, 256);		// 1:1 scale
	}
}

int getMillis(bool skipRecord) {
	return currentTimeMillis;
}

void setTimerCallback(OSystem_DS::TimerProc proc, int interval) {
	callback = proc;
	callbackInterval = interval;
	callbackTimer = interval;
}

void timerTickHandler() {
	if ((callback) && (callbackTimer > 0)) {
		callbackTimer--;
	}
	currentTimeMillis++;
}





void setTalkPos(int x, int y) {
	setTopScreenTarget(x, y);
}

void setTopScreenTarget(int x, int y) {
	subScTargetX = (x - (subScreenWidth >> 1));
	subScTargetY = (y - (subScreenHeight >> 1));

	if (subScTargetX < 0) subScTargetX = 0;
	if (subScTargetX > gameWidth - subScreenWidth) subScTargetX = gameWidth - subScreenWidth;

	if (subScTargetY < 0) subScTargetY = 0;
	if (subScTargetY > gameHeight - subScreenHeight) subScTargetY = gameHeight - subScreenHeight;

	subScTargetX <<=8;
	subScTargetY <<=8;
}

void initHardware() {
	powerOn(POWER_ALL);

	for (int r = 0; r < 255; r++) {
		BG_PALETTE[r] = 0;
	}

	videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	vramSetBankE(VRAM_E_MAIN_SPRITE);

	REG_BG2CNT = BG_BMP16_256x256;
	REG_BG2PA = 256;
	REG_BG2PB = 0;
	REG_BG2PC = 0;
	REG_BG2PD = 256;

	scX = 0;
	scY = 0;
	subScX = 0;
	subScY = 0;
	subScTargetX = 0;
	subScTargetY = 0;

	lcdMainOnBottom();

	frameCount = 0;
	callback = NULL;

	//irqs are nice
	irqSet(IRQ_VBLANK, VBlankHandler);
	irqEnable(IRQ_VBLANK);

	// Set up a millisecond timer
	currentTimeMillis = 0;
	timerStart(0, ClockDivider_1, (u16)TIMER_FREQ(1000), timerTickHandler);
	REG_IME = 1;

#ifndef DISABLE_TEXT_CONSOLE
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);
	vramSetBankH(VRAM_H_SUB_BG);
	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);
#else
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
#endif
}

///////////////////
// Fast Ram
///////////////////

#define FAST_RAM_SIZE (22500)
#define ITCM_DATA	__attribute__((section(".itcm")))

u8 *fastRamPointer;
u8 fastRamData[FAST_RAM_SIZE] ITCM_DATA;

void *fastRamAlloc(int size) {
	void *result = (void *) fastRamPointer;
	fastRamPointer += size;
	if(fastRamPointer > fastRamData + FAST_RAM_SIZE) {
		warning("FastRam (ITCM) allocation failed");
		return malloc(size);
	}
	return result;
}

void fastRamReset() {
	fastRamPointer = &fastRamData[0];
}

} // End of namespace DS

/////////////////
// Main
/////////////////

int main(int argc, char **argv) {
	g_system = new OSystem_DS();
	assert(g_system);

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new DSPluginProvider());
#endif

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);

	// Free OSystem
	g_system->destroy();

	return res;
}
