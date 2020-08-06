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
#include "dsoptions.h"
#include "blitters.h"
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
static bool scaledMode;
static int scX;
static int scY;

static int subScX;
static int subScY;
static int subScTargetX;
static int subScTargetY;
static int subScreenWidth = SCUMM_GAME_WIDTH;
static int subScreenHeight = SCUMM_GAME_HEIGHT;
static int subScreenScale = 256;


// Saved buffers
static bool highBuffer;
static bool displayModeIs8Bit = false;

static bool consoleEnable = false;
static bool gameScreenSwap = false;
bool isCpuScalerEnabled();
//#define HEAVY_LOGGING

static int storedMouseX = 0;
static int storedMouseY = 0;

// Sprites
static SpriteEntry sprites[128];
static SpriteEntry spritesMain[128];

// Shake
static int s_shakeXOffset = 0;
static int s_shakeYOffset = 0;

// Touch
static int touchScX, touchScY, touchX, touchY;
static int mouseHotspotX, mouseHotspotY;
static bool cursorEnable = false;
static bool mouseCursorVisible = true;
static bool touchPadStyle = false;

// 8-bit surface size
static int gameWidth = 320;
static int gameHeight = 200;

// Scale
static bool twoHundredPercentFixedScale = false;
static bool cpuScalerEnable = false;

static u8 *scalerBackBuffer = NULL;

static u16 savedPalEntry255 = RGB15(31, 31, 31);

void setIcon(int num, int x, int y, int imageNum, int flags, bool enable);
void setIconMain(int num, int x, int y, int imageNum, int flags, bool enable);

bool isCpuScalerEnabled() {
	return cpuScalerEnable || !displayModeIs8Bit;
}


void setCpuScalerEnable(bool enable) {
	cpuScalerEnable = enable;
}

void setTrackPadStyleEnable(bool enable) {
	touchPadStyle = enable;
}

void setGameScreenSwap(bool enable) {
	gameScreenSwap = enable;
}

void setGamma(int gamma) {
	OSystem_DS::instance()->setGammaValue(gamma);
}

void setTopScreenZoom(int percentage) {
	s32 scale = (percentage << 8) / 100;
	subScreenScale = (256 * 256) / scale;
}

void updateOAM() {
	DC_FlushAll();

	if (gameScreenSwap) {
		dmaCopy(sprites, OAM, 128 * sizeof(SpriteEntry));
		dmaCopy(spritesMain, OAM_SUB, 128 * sizeof(SpriteEntry));
	} else {
		dmaCopy(sprites, OAM_SUB, 128 * sizeof(SpriteEntry));
		dmaCopy(spritesMain, OAM, 128 * sizeof(SpriteEntry));
	}
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

void initSprites() {
	for (int i = 0; i < 128; i++) {
		sprites[i].attribute[0] = ATTR0_DISABLED;
		sprites[i].attribute[1] = 0;
		sprites[i].attribute[2] = 0;
		sprites[i].filler = 0;
	}

	for (int i = 0; i < 128; i++) {
		spritesMain[i].attribute[0] = ATTR0_DISABLED;
		spritesMain[i].attribute[1] = 0;
		spritesMain[i].attribute[2] = 0;
		spritesMain[i].filler = 0;
	}

	updateOAM();
}


void saveGameBackBuffer() {

	// Sometimes the only copy of the game screen is in video memory.
	// So, I lock the video memory here, as if I'm going to modify it.  This
	// forces OSystem_DS to create a system memory copy if one doesn't exist.
	// This will be automatially restored by OSystem_DS::updateScreen().

	OSystem_DS::instance()->lockScreen();
	OSystem_DS::instance()->unlockScreen();
}

void set200PercentFixedScale(bool on) {
	twoHundredPercentFixedScale = on;
}

void setUnscaledMode(bool enable) {
	scaledMode = !enable;
}

void displayMode8Bit() {

#ifdef HEAVY_LOGGING
	printf("displayMode8Bit...");
#endif
	u16 buffer[32 * 32];

	setOptions();

	if (!displayModeIs8Bit) {
		for (int r = 0; r < 32 * 32; r++) {
			buffer[r] = ((u16 *) SCREEN_BASE_BLOCK_SUB(4))[r];
		}
	} else {
		for (int r = 0; r < 32 * 32; r++) {
			buffer[r] = ((u16 *) SCREEN_BASE_BLOCK(2))[r];
		}
	}

	displayModeIs8Bit = true;

	if (isCpuScalerEnabled()) {
		videoSetMode(MODE_5_2D | (consoleEnable ? DISPLAY_BG0_ACTIVE : 0) | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP);
		videoSetModeSub(MODE_3_2D /*| DISPLAY_BG0_ACTIVE*/ | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP); //sub bg 0 will be used to print text

		vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
		vramSetBankB(VRAM_B_MAIN_BG_0x06020000);

		vramSetBankC(VRAM_C_SUB_BG_0x06200000);
		vramSetBankD(VRAM_D_SUB_SPRITE);

		vramSetBankH(VRAM_H_LCD);

		REG_BG3CNT = BG_BMP16_256x256 | BG_BMP_BASE(8);

		REG_BG3PA = 256;
		REG_BG3PB = 0;
		REG_BG3PC = 0;
		REG_BG3PD = (int) ((200.0f / 192.0f) * 256);

	} else {
		videoSetMode(MODE_5_2D | (consoleEnable ? DISPLAY_BG0_ACTIVE : 0) | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP);
		videoSetModeSub(MODE_3_2D /*| DISPLAY_BG0_ACTIVE*/ | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP); //sub bg 0 will be used to print text

		vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
		vramSetBankB(VRAM_B_MAIN_BG_0x06020000);

		vramSetBankC(VRAM_C_SUB_BG_0x06200000);
		vramSetBankD(VRAM_D_SUB_SPRITE);

		vramSetBankH(VRAM_H_LCD);

		REG_BG3CNT = BG_BMP8_512x256 | BG_BMP_BASE(8);

		REG_BG3PA = (int) (((float) (gameWidth) / 256.0f) * 256);
		REG_BG3PB = 0;
		REG_BG3PC = 0;
		REG_BG3PD = (int) ((200.0f / 192.0f) * 256);
	}

	REG_BG3CNT_SUB = BG_BMP8_512x256;

	REG_BG3PA_SUB = (int) (subScreenWidth / 256.0f * 256);
	REG_BG3PB_SUB = 0;
	REG_BG3PC_SUB = 0;
	REG_BG3PD_SUB = (int) (subScreenHeight / 192.0f * 256);



	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, true, true);

	// Set this again because consoleinit resets it
	videoSetMode(MODE_5_2D | (consoleEnable ? DISPLAY_BG0_ACTIVE : 0) | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP);

	// Move the cursor to the bottom of the screen using ANSI escape code
	printf("\033[23;0f");


	for (int r = 0; r < 32 * 32; r++) {
		((u16 *) SCREEN_BASE_BLOCK(2))[r] = buffer[r];
	}

	// ConsoleInit destroys the hardware palette :-(
	if (OSystem_DS::instance()) {
		OSystem_DS::instance()->restoreHardwarePalette();
	}

	#ifdef HEAVY_LOGGING
	printf("done\n");
	#endif

	if (gameScreenSwap) {
		lcdMainOnTop();
	} else {
		lcdMainOnBottom();
	}
}

void setShowCursor(bool enable) {
	cursorEnable = enable;
	updateMouse();
}

void setMouseCursorVisible(bool enable) {
	mouseCursorVisible = enable;
	updateMouse();
}

void setCursorIcon(const u8 *icon, uint w, uint h, byte keycolor, int hotspotX, int hotspotY) {

	int off;

	mouseHotspotX = hotspotX;
	mouseHotspotY = hotspotY;

	off = 128*64;


	memset(SPRITE_GFX + off, 0, 32 * 32 * 2);
	memset(SPRITE_GFX_SUB + off, 0, 32 * 32 * 2);


	for (uint y=0; y<h; y++) {
		for (uint x=0; x<w; x++) {
			int color = icon[y*w+x];

			if (color == keycolor) {
				SPRITE_GFX[off+(y)*32+x] = 0x0000; // black background
				SPRITE_GFX_SUB[off+(y)*32+x] = 0x0000; // black background
			} else {
				SPRITE_GFX[off+(y)*32+x] = OSystem_DS::instance()->getDSCursorPaletteEntry(color) | 0x8000;
				SPRITE_GFX_SUB[off+(y)*32+x] = OSystem_DS::instance()->getDSCursorPaletteEntry(color) | 0x8000;
			}
		}
	}
}




void displayMode16Bit() {
	#ifdef HEAVY_LOGGING
	printf("displayMode16Bit...");
	#endif

	u16 buffer[32 * 32 * 2];

	if (!displayModeIs8Bit) {
		for (int r = 0; r < 32 * 32; r++) {
			buffer[r] = ((u16 *) SCREEN_BASE_BLOCK_SUB(4))[r];
		}
	} else {
		saveGameBackBuffer();
		for (int r = 0; r < 32 * 32; r++) {
			buffer[r] = ((u16 *) SCREEN_BASE_BLOCK(2))[r];
		}
	}


	videoSetMode(MODE_5_2D | /*DISPLAY_BG0_ACTIVE |*/ DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP);
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP); //sub bg 0 will be used to print text

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_BG);
	vramSetBankC(VRAM_C_MAIN_BG);
	vramSetBankD(VRAM_D_MAIN_BG);
	vramSetBankH(VRAM_H_SUB_BG);

	REG_BG3CNT = BG_BMP16_512x256;
	highBuffer = false;


	memset(BG_GFX, 0, 512 * 256 * 2);

	savedPalEntry255 = BG_PALETTE_SUB[255];
	BG_PALETTE_SUB[255] = RGB15(31,31,31);//by default font will be rendered with color 255

	// Do text stuff
	REG_BG0CNT_SUB = BG_MAP_BASE(4) | BG_TILE_BASE(0);
	REG_BG0VOFS_SUB = 0;

	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 4, 0, false, true);

	for (int r = 0; r < 32 * 32; r++) {
		((u16 *) SCREEN_BASE_BLOCK_SUB(4))[r] = buffer[r];
	}

	consoleSetWindow(NULL, 0, 0, 32, 24);

	lcdMainOnBottom();

	displayModeIs8Bit = false;

	// ConsoleInit destroys the hardware palette :-(
	OSystem_DS::instance()->restoreHardwarePalette();

	REG_BG3PA = isCpuScalerEnabled() ? 256 : (int) (1.25f * 256);
	REG_BG3PB = 0;
	REG_BG3PC = 0;
	REG_BG3PD = (int) ((200.0f / 192.0f) * 256);

	#ifdef HEAVY_LOGGING
	printf("done\n");
	#endif

	BG_PALETTE_SUB[255] = RGB15(31,31,31);//by default font will be rendered with color 255

}


void displayMode16BitFlipBuffer() {
	#ifdef HEAVY_LOGGING
	printf("Flip %s...", displayModeIs8Bit ? "8bpp" : "16bpp");
	#endif
	if (!displayModeIs8Bit) {
		u16 *back = get16BitBackBuffer();

		if (isCpuScalerEnabled()) {
			Rescale_320x256x1555_To_256x256x1555(BG_GFX, back, 512, 512);
		} else {
			for (int r = 0; r < 512 * 256; r++) {
				*(BG_GFX + r) = *(back + r);
			}
		}
	} else if (isCpuScalerEnabled()) {
		const u8 *back = (const u8*)get8BitBackBuffer();
		u16 *base = BG_GFX + 0x10000;
		Rescale_320x256xPAL8_To_256x256x1555(
			base,
			back,
			256,
			get8BitBackBufferStride(),
			BG_PALETTE,
			getGameHeight() );
	}
	#ifdef HEAVY_LOGGING
	printf("done\n");
	#endif
}

void setShakePos(int shakeXOffset, int shakeYOffset) {
	s_shakeXOffset = shakeXOffset;
	s_shakeYOffset = shakeYOffset;
}


u16 *get16BitBackBuffer() {
	return BG_GFX + 0x20000;
}

s32 get8BitBackBufferStride() {
	// When the CPU scaler is enabled, the back buffer is in system RAM and is
	// 320 pixels wide. When the CPU scaler is disabled, the back buffer is in
	// video memory and therefore must have a 512 pixel stride.

	if (isCpuScalerEnabled()){
		return 320;
	} else {
		return 512;
	}
}

u16 *getScalerBuffer() {
	return (u16 *) scalerBackBuffer;
}

u16 *get8BitBackBuffer() {
	if (isCpuScalerEnabled())
		return (u16 *) scalerBackBuffer;
	else
		return BG_GFX + 0x10000;		// 16bit qty!
}

void doTimerCallback() {
	if (callback) {
		if (callbackTimer <= 0) {
			callbackTimer += callbackInterval;
			callback(callbackInterval);
		}
	}
}

bool getIsDisplayMode8Bit() {
	return displayModeIs8Bit;
}

void setIcon(int num, int x, int y, int imageNum, int flags, bool enable) {
	sprites[num].attribute[0] = ATTR0_BMP | (enable ? (y & 0xFF) : 192) | (!enable ? ATTR0_DISABLED : 0);
	sprites[num].attribute[1] = ATTR1_SIZE_32 | (x & 0x1FF) | flags;
	sprites[num].attribute[2] = ATTR2_ALPHA(1)| (imageNum * 16);
}

void setIconMain(int num, int x, int y, int imageNum, int flags, bool enable) {
	spritesMain[num].attribute[0] = ATTR0_BMP | (y & 0xFF) | (!enable ? ATTR0_DISABLED : 0);
	spritesMain[num].attribute[1] = ATTR1_SIZE_32 | (x & 0x1FF) | flags;
	spritesMain[num].attribute[2] = ATTR2_ALPHA(1)| (imageNum * 16);
}

void updateMouse() {
	if ((cursorEnable) && (mouseCursorVisible)) {
		if (gameScreenSwap && touchPadStyle) {
			setIcon(3, storedMouseX - mouseHotspotX, storedMouseY - mouseHotspotY, 8, 0, true);
			setIconMain(3, 0, 0, 0, 0, false);
		} else {
			setIconMain(3, storedMouseX - mouseHotspotX, storedMouseY - mouseHotspotY, 8, 0, true);
			setIcon(3, 0, 0, 0, 0, false);
		}
	} else {
		setIconMain(3, 0, 0, 0, 0, false);
		setIcon(3, 0, 0, 0, 0, false);
	}
}

void warpMouse(int penX, int penY) {
	storedMouseX = ((penX - touchX) << 8) / touchScX;
	storedMouseY = ((penY - touchY) << 8) / touchScY;
	updateMouse();
}

void setMainScreenScroll(int x, int y) {
		REG_BG3X = x + (((frameCount & 1) == 0)? 64: 0);
		REG_BG3Y = y;

		if ((!gameScreenSwap) || (touchPadStyle)) {
			touchX = x >> 8;
			touchY = y >> 8;
		}
}

void setMainScreenScale(int x, int y) {
		if (isCpuScalerEnabled() && (x==320)) {
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

		if ((!gameScreenSwap) || (touchPadStyle)) {
			touchScX = x;
			touchScY = y;
		}
}

void setZoomedScreenScroll(int x, int y, bool shake) {
		if ((gameScreenSwap) && (!touchPadStyle)) {
			touchX = x >> 8;
			touchY = y >> 8;
		}


		REG_BG3X_SUB = x + ((shake && (frameCount & 1) == 0)? 64: 0);
		REG_BG3Y_SUB = y;
}

void setZoomedScreenScale(int x, int y) {
		if ((gameScreenSwap) && (!touchPadStyle)) {
			touchScX = x;
			touchScY = y;
		}

		REG_BG3PA_SUB = x;
		REG_BG3PB_SUB = 0;
		REG_BG3PC_SUB = 0;
		REG_BG3PD_SUB = y;
}

Common::Point transformPoint(uint16 x, uint16 y) {
	x = ((x * touchScX) >> 8) + touchX;
	x = CLIP<uint16>(x, 0, gameWidth  - 1);

	y = ((y * touchScY) >> 8) + touchY;
	y = CLIP<uint16>(y, 0, gameHeight - 1);

	return Common::Point(x, y);
}

void VBlankHandler(void) {
	frameCount++;

	if (callback) {
		callbackTimer -= FRAME_TIME;
	}

	int xCenter = subScTargetX + ((subScreenWidth >> 1) << 8);
	int yCenter = subScTargetY + ((subScreenHeight >> 1) << 8);


	if (twoHundredPercentFixedScale) {
		subScreenWidth = 256 >> 1;
		subScreenHeight = 192 >> 1;
	} else {
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
	}


	subScTargetX = xCenter - ((subScreenWidth  >> 1) << 8);
	subScTargetY = yCenter - ((subScreenHeight >> 1) << 8);

	subScTargetX = CLIP(subScTargetX, 0, (gameWidth  - subScreenWidth)  << 8);
	subScTargetY = CLIP(subScTargetY, 0, (gameHeight - subScreenHeight) << 8);

	subScX += (subScTargetX - subScX) >> 2;
	subScY += (subScTargetY - subScY) >> 2;

	if (displayModeIs8Bit) {

		if (!scaledMode) {

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
	} else {
		setZoomedScreenScroll(0, 0, true);
		setZoomedScreenScale(320, 256);

		setMainScreenScroll(0, 0);
		setMainScreenScale(320, 256);		// 1:1 scale
	}

	updateOAM();
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
	vramSetBankD(VRAM_D_SUB_SPRITE);
	vramSetBankE(VRAM_E_MAIN_SPRITE);

	for (int r = 0; r < 255; r++) {
		BG_PALETTE[r] = 0;
	}

	BG_PALETTE[255] = RGB15(0,31,0);


	for (int r = 0; r < 255; r++) {
		BG_PALETTE_SUB[r] = 0;
	}

	BG_PALETTE_SUB[255] = RGB15(0,31,0);

	// Allocate save buffer for game screen
	displayMode16Bit();

	memset(BG_GFX, 0, 512 * 256 * 2);
	scaledMode = true;
	scX = 0;
	scY = 0;
	subScX = 0;
	subScY = 0;
	subScTargetX = 0;
	subScTargetY = 0;

	lcdMainOnBottom();

	frameCount = 0;
	callback = NULL;

	BG_PALETTE[255] = RGB15(31,31,31);//by default font will be rendered with color 255

	//irqs are nice
	irqSet(IRQ_VBLANK, VBlankHandler);
	irqEnable(IRQ_VBLANK);

	// Set up a millisecond timer
	currentTimeMillis = 0;
	timerStart(0, ClockDivider_1, (u16)TIMER_FREQ(1000), timerTickHandler);
	REG_IME = 1;

	BG_PALETTE[255] = RGB15(0,0,31);

	initSprites();

	// If the software scaler's back buffer has not been allocated, do it now
	scalerBackBuffer = (u8 *) malloc(320 * 256);

	// This is a bodge to get around the fact that the cursor is turned on before it's image is set
	// during startup in Sam & Max.  This bodge moves the cursor offscreen so it is not seen.
	sprites[1].attribute[1] = ATTR1_SIZE_64 | 192;

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
		printf("FastRam (ITCM) allocation failed!\n");
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
#ifndef DISABLE_TEXT_CONSOLE
	consoleDebugInit(DebugDevice_NOCASH);
	nocashMessage("startup\n");
#endif

	DS::initHardware();

	defaultExceptionHandler();

	if (!nitroFSInit(NULL)) {
		printf("nitroFSInit failure: terminating\n");
		return(1);
	}

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
