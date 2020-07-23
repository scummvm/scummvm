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
#include <nds/registers_alt.h>
#include <filesystem.h>

#include <stdlib.h>
#include <string.h>

#include "dsmain.h"
#include "osystem_ds.h"
#include "icons_raw.h"
#include "keyboard_raw.h"
#include "keyboard_pal_raw.h"
#define V16(a, b) ((a << 12) | b)
#include "touchkeyboard.h"
#include "dsoptions.h"
#include "blitters.h"
#include "engines/engine.h"

#include "backends/plugins/ds/ds-provider.h"
#include "base/main.h"
#include "base/version.h"
#include "common/util.h"

namespace DS {

// From console.c in NDSLib

//location of cursor
extern u8 row;
extern u8 col;

// Mouse mode
enum MouseMode {
	MOUSE_LEFT, MOUSE_RIGHT, MOUSE_HOVER, MOUSE_NUM_MODES
};

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


// Events
static int lastEventFrame;
static bool indyFightState;
static bool indyFightRight;

// Saved buffers
static bool highBuffer;
static bool displayModeIs8Bit = false;

// Game id
static u8 gameID;

static bool snapToBorder = false;
static bool consoleEnable = false;
static bool gameScreenSwap = false;
bool isCpuScalerEnabled();
//#define HEAVY_LOGGING

static MouseMode mouseMode = MOUSE_LEFT;

static int storedMouseX = 0;
static int storedMouseY = 0;

// Sprites
static SpriteEntry sprites[128];
static SpriteEntry spritesMain[128];

// Shake
static int s_shakeXOffset = 0;
static int s_shakeYOffset = 0;

// Keyboard
static bool keyboardEnable = false;
static bool leftHandedMode = false;
static bool keyboardIcon = false;

// Touch
static int touchScX, touchScY, touchX, touchY;
static int mouseHotspotX, mouseHotspotY;
static bool cursorEnable = false;
static bool mouseCursorVisible = true;
static bool leftButtonDown = false;
static bool rightButtonDown = false;
static bool touchPadStyle = false;
static int touchPadSensitivity = 8;
static bool tapScreenClicks = true;

static int tapCount = 0;
static int tapTimeout = 0;
static int tapComplete = 0;

// Dragging
static int dragStartX, dragStartY;
static bool dragging = false;
static int dragScX, dragScY;

// Interface styles
static char gameName[32];

// 8-bit surface size
static int gameWidth = 320;
static int gameHeight = 200;

// Scale
static bool twoHundredPercentFixedScale = false;
static bool cpuScalerEnable = false;

static u8 *scalerBackBuffer = NULL;

#define NUM_SUPPORTED_GAMES 21

static const gameListType gameList[NUM_SUPPORTED_GAMES] = {
	// Unknown game - use normal SCUMM controls
	{"unknown", 	CONT_SCUMM_ORIGINAL},

	// SCUMM games
	{"maniac",		CONT_SCUMM_ORIGINAL},
	{"zak",			CONT_SCUMM_ORIGINAL},
	{"loom",		CONT_SCUMM_ORIGINAL},
	{"indy3",		CONT_SCUMM_ORIGINAL},
	{"atlantis",		CONT_SCUMM_ORIGINAL},
	{"monkey",		CONT_SCUMM_ORIGINAL},
	{"monkey2",		CONT_SCUMM_ORIGINAL},
	{"tentacle",		CONT_SCUMM_ORIGINAL},
	{"samnmax",		CONT_SCUMM_SAMNMAX},

	// Non-SCUMM games
	{"sky",			CONT_SKY},
	{"simon1",		CONT_SIMON},
	{"simon2",		CONT_SIMON},
	{"gob",			CONT_GOBLINS},
	{"queen",		CONT_SCUMM_ORIGINAL},
	{"cine",		CONT_FUTURE_WARS},
	{"agi",			CONT_AGI},
	{"elvira2",		CONT_SIMON},
	{"elvira1",		CONT_SIMON},
	{"waxworks",		CONT_SIMON},
	{"parallaction",	CONT_NIPPON},
};

static const gameListType *s_currentGame = NULL;

// Stylus
static bool penDown = FALSE;
static bool penHeld = FALSE;
static bool penReleased = FALSE;
static bool penDownLastFrame = FALSE;
static s32 penX = 0, penY = 0;
static s32 penDownX = 0, penDownY = 0;
static int keysDownSaved = 0;
static int keysReleasedSaved = 0;
static int keysChangedSaved = 0;

static bool penDownSaved = FALSE;
static bool penReleasedSaved = FALSE;
static int penDownFrames = 0;
static int touchXOffset = 0;
static int touchYOffset = 0;

static int triggeredIcon = 0;
static int triggeredIconTimeout = 0;

static u16 savedPalEntry255 = RGB15(31, 31, 31);

Common::EventType getKeyEvent(int key);
int getKeysChanged();

void updateStatus();
void triggerIcon(int imageNum);
void setIcon(int num, int x, int y, int imageNum, int flags, bool enable);
void setIconMain(int num, int x, int y, int imageNum, int flags, bool enable);
void uploadSpriteGfx();

static bool isScrollingWithDPad() {
	return (getKeysHeld() & (KEY_L | KEY_R)) != 0;
}

bool isCpuScalerEnabled() {
	return cpuScalerEnable || !displayModeIs8Bit;
}


void setCpuScalerEnable(bool enable) {
	cpuScalerEnable = enable;
}

void setTrackPadStyleEnable(bool enable) {
	touchPadStyle = enable;
}

void setTapScreenClicksEnable(bool enable) {
	tapScreenClicks = enable;
}

void setGameScreenSwap(bool enable) {
	gameScreenSwap = enable;
}

void setSensitivity(int sensitivity) {
	touchPadSensitivity = sensitivity;
}

void setGamma(int gamma) {
	OSystem_DS::instance()->setGammaValue(gamma);
}

void setTopScreenZoom(int percentage) {
	s32 scale = (percentage << 8) / 100;
	subScreenScale = (256 * 256) / scale;
}

controlType getControlType() {
	return s_currentGame->control;
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

void exitGame() {
	s_currentGame = NULL;
}

void initGame() {
	// This is a good time to check for left handed mode since the mode change is done as the game starts.
	// There's probably a better way, but hey.
	#ifdef HEAVY_LOGGING
	printf("initing game...");
	#endif

	setOptions();

	if (s_currentGame == NULL) {

		strcpy(gameName, ConfMan.get("gameid").c_str());

		s_currentGame = &gameList[0];		// Default game

		for (int r = 0; r < NUM_SUPPORTED_GAMES; r++) {
			if (!scumm_stricmp(gameName, gameList[r].gameId)) {
				s_currentGame = &gameList[r];
			}
		}
	}

	#ifdef HEAVY_LOGGING
	printf("done\n");
	#endif

}

void setLeftHanded(bool enable) {
	leftHandedMode = enable;
}

void setSnapToBorder(bool enable) {
	snapToBorder = enable;
}

void setTouchXOffset(int x) {
	touchXOffset = x;
}

void setTouchYOffset(int y) {
	touchYOffset = y;
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

	initGame();

	setKeyboardEnable(false);

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

		BG3_CR = BG_BMP16_256x256 | BG_BMP_BASE(8);

		BG3_XDX = 256;
		BG3_XDY = 0;
		BG3_YDX = 0;
		BG3_YDY = (int) ((200.0f / 192.0f) * 256);

	} else {
		videoSetMode(MODE_5_2D | (consoleEnable ? DISPLAY_BG0_ACTIVE : 0) | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP);
		videoSetModeSub(MODE_3_2D /*| DISPLAY_BG0_ACTIVE*/ | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP); //sub bg 0 will be used to print text

		vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
		vramSetBankB(VRAM_B_MAIN_BG_0x06020000);

		vramSetBankC(VRAM_C_SUB_BG_0x06200000);
		vramSetBankD(VRAM_D_SUB_SPRITE);

		vramSetBankH(VRAM_H_LCD);

		BG3_CR = BG_BMP8_512x256 | BG_BMP_BASE(8);

		BG3_XDX = (int) (((float) (gameWidth) / 256.0f) * 256);
		BG3_XDY = 0;
		BG3_YDX = 0;
		BG3_YDY = (int) ((200.0f / 192.0f) * 256);
	}

	SUB_BG3_CR = BG_BMP8_512x256;

	SUB_BG3_XDX = (int) (subScreenWidth / 256.0f * 256);
	SUB_BG3_XDY = 0;
	SUB_BG3_YDX = 0;
	SUB_BG3_YDY = (int) (subScreenHeight / 192.0f * 256);



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
		POWER_CR |= POWER_SWAP_LCDS;
	} else {
		POWER_CR &= ~POWER_SWAP_LCDS;
	}

	uploadSpriteGfx();

	keyboardEnable = false;

}

void setGameID(int id) {
	gameID = id;
}

void setShowCursor(bool enable) {
	if ((s_currentGame) && (s_currentGame->control == CONT_SCUMM_SAMNMAX)) {
		if (cursorEnable) {
			sprites[1].attribute[0] = ATTR0_BMP | 150;
		} else {
			sprites[1].attribute[0] = ATTR0_DISABLED;
		}

	}

	cursorEnable = enable;
}

void setMouseCursorVisible(bool enable) {
	mouseCursorVisible = enable;
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

	if (s_currentGame->control != CONT_SCUMM_SAMNMAX)
		return;

	uint16 border = RGB15(24,24,24) | 0x8000;


	off = 176*64;
	memset(SPRITE_GFX_SUB+off, 0, 64*64*2);
	memset(SPRITE_GFX+off, 0, 64*64*2);

	int pos = 190 - (w+2);



	// make border
	for (uint i=0; i<w+2; i++) {
		SPRITE_GFX[off+i] = border;
		SPRITE_GFX[off+(31)*64+i] = border;

		SPRITE_GFX_SUB[off+i] = border;
		SPRITE_GFX_SUB[off+(31)*64+i] = border;
	}
	for (uint i=1; i<31; i++) {
		SPRITE_GFX[off+(i*64)] = border;
		SPRITE_GFX[off+(i*64)+(w+1)] = border;

		SPRITE_GFX_SUB[off+(i*64)] = border;
		SPRITE_GFX_SUB[off+(i*64)+(w+1)] = border;
	}

	int offset = (32 - h) >> 1;

	for (uint y=0; y<h; y++) {
		for (uint x=0; x<w; x++) {
			int color = icon[y*w+x];

			if (color == keycolor) {
				SPRITE_GFX[off+(y+1+offset)*64+(x+1)] = 0x8000; // black background
				SPRITE_GFX_SUB[off+(y+1+offset)*64+(x+1)] = 0x8000; // black background
			} else {
				SPRITE_GFX[off+(y+1+offset)*64+(x+1)] = BG_PALETTE[color] | 0x8000;
				SPRITE_GFX_SUB[off+(y+1+offset)*64+(x+1)] = BG_PALETTE[color] | 0x8000;
			}
		}
	}


	if (cursorEnable) {
		sprites[1].attribute[0] = ATTR0_BMP | 150;
		sprites[1].attribute[1] = ATTR1_SIZE_64 | pos;
		sprites[1].attribute[2] = ATTR2_ALPHA(1) | 176;
	} else {
		sprites[1].attribute[0] = ATTR0_DISABLED | 150;
		sprites[1].attribute[1] = ATTR1_SIZE_64 | pos;
		sprites[1].attribute[2] = ATTR2_ALPHA(1) | 176;
	}

}




void displayMode16Bit() {
	#ifdef HEAVY_LOGGING
	printf("displayMode16Bit...");
	#endif

	u16 buffer[32 * 32 * 2];

	releaseAllKeys();

	setKeyboardEnable(false);

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
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE |/* DISPLAY_BG1_ACTIVE |*/ DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP); //sub bg 0 will be used to print text

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_BG);
	vramSetBankC(VRAM_C_MAIN_BG);
	vramSetBankD(VRAM_D_MAIN_BG);
	vramSetBankH(VRAM_H_SUB_BG);

	BG3_CR = BG_BMP16_512x256;
	highBuffer = false;


	memset(BG_GFX, 0, 512 * 256 * 2);

	savedPalEntry255 = BG_PALETTE_SUB[255];
	BG_PALETTE_SUB[255] = RGB15(31,31,31);//by default font will be rendered with color 255

	// Do text stuff
	SUB_BG0_CR = BG_MAP_BASE(4) | BG_TILE_BASE(0);
	SUB_BG0_Y0 = 0;

	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 4, 0, false, true);

	for (int r = 0; r < 32 * 32; r++) {
		((u16 *) SCREEN_BASE_BLOCK_SUB(4))[r] = buffer[r];
	}

	consoleSetWindow(NULL, 0, 0, 32, 24);

	// Show keyboard
	SUB_BG1_CR = BG_TILE_BASE(1) | BG_MAP_BASE(12);

	POWER_CR &= ~POWER_SWAP_LCDS;

	displayModeIs8Bit = false;

	// ConsoleInit destroys the hardware palette :-(
	OSystem_DS::instance()->restoreHardwarePalette();

	BG3_XDX = isCpuScalerEnabled() ? 256 : (int) (1.25f * 256);
	BG3_XDY = 0;
	BG3_YDX = 0;
	BG3_YDY = (int) ((200.0f / 192.0f) * 256);

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
		//#define SCALER_PROFILE

		#ifdef SCALER_PROFILE
		TIMER1_CR = TIMER_ENABLE | TIMER_DIV_1024;
		u16 t0 = TIMER1_DATA;
		#endif
		const u8 *back = (const u8*)get8BitBackBuffer();
		u16 *base = BG_GFX + 0x10000;
		Rescale_320x256xPAL8_To_256x256x1555(
			base,
			back,
			256,
			get8BitBackBufferStride(),
			BG_PALETTE,
			getGameHeight() );

		#ifdef SCALER_PROFILE
		// 10 pixels : 1ms
		u16 t1 = TIMER1_DATA;
		TIMER1_CR &= ~TIMER_ENABLE;
		u32 dt = t1 - t0;
		u32 dt_us = (dt * 10240) / 334;
		u32 dt_10ms = dt_us / 100;
		int i;
		for(i=0; i<dt_10ms; ++i)
			base[i] = ((i/10)&1) ? 0xFFFF : 0x801F;
		for(; i<256; ++i)
			base[i] = 0x8000;
		#endif
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

void memoryReport() {
	int r = 0;
	int *p;
	do {
		p = (int *) malloc(r * 8192);
		free(p);
		r++;
	} while ((p) && (r < 512));

	int t = -1;
	void *block[1024];
	do {
		t++;
		block[t] = (int *) malloc(4096);
	} while ((t < 1024) && (block[t]));

	for (int q = 0; q < t; q++) {
		free(block[q]);
	}

	printf("Free: %dK, Largest: %dK\n", t * 4, r * 8);
}


void addIndyFightingKeys() {
	OSystem_DS *system = OSystem_DS::instance();
	Common::Event event;

	event.type = Common::EVENT_KEYDOWN;
	event.kbd.flags = 0;

	if ((getKeysDown() & KEY_L)) {
		indyFightRight = false;
	}

	if ((getKeysDown() & KEY_R)) {
		indyFightRight = true;
	}

	if ((getKeysChanged() & KEY_UP)) {
		event.type = getKeyEvent(KEY_UP);
		event.kbd.keycode = Common::KEYCODE_8;
		event.kbd.ascii = '8';
		system->addEvent(event);
	}
	if ((getKeysChanged() & KEY_LEFT)) {
		event.type = getKeyEvent(KEY_LEFT);
		event.kbd.keycode = Common::KEYCODE_4;
		event.kbd.ascii = '4';
		system->addEvent(event);
	}
	if ((getKeysChanged() & KEY_RIGHT)) {
		event.type = getKeyEvent(KEY_RIGHT);
		event.kbd.keycode = Common::KEYCODE_6;
		event.kbd.ascii = '6';
		system->addEvent(event);
	}
	if ((getKeysChanged() & KEY_DOWN)) {
		event.type = getKeyEvent(KEY_DOWN);
		event.kbd.keycode = Common::KEYCODE_2;
		event.kbd.ascii = '2';
		system->addEvent(event);
	}

	if (indyFightRight) {

		if ((getKeysChanged() & KEY_X)) {
			event.type = getKeyEvent(KEY_X);
			event.kbd.keycode = Common::KEYCODE_9;
			event.kbd.ascii = '9';
			system->addEvent(event);
		}
		if ((getKeysChanged() & KEY_A)) {
			event.type = getKeyEvent(KEY_A);
			event.kbd.keycode = Common::KEYCODE_6;
			event.kbd.ascii = '6';
			system->addEvent(event);
		}
		if ((getKeysChanged() & KEY_B)) {
			event.type = getKeyEvent(KEY_B);
			event.kbd.keycode = Common::KEYCODE_3;
			event.kbd.ascii = '3';
			system->addEvent(event);
		}

	} else {

		if ((getKeysChanged() & KEY_X)) {
			event.type = getKeyEvent(KEY_X);
			event.kbd.keycode = Common::KEYCODE_7;
			event.kbd.ascii = '7';
			system->addEvent(event);
		}
		if ((getKeysChanged() & KEY_A)) {
			event.type = getKeyEvent(KEY_A);
			event.kbd.keycode = Common::KEYCODE_4;
			event.kbd.ascii = '4';
			system->addEvent(event);
		}
		if ((getKeysChanged() & KEY_B)) {
			event.type = getKeyEvent(KEY_B);
			event.kbd.keycode = Common::KEYCODE_1;
			event.kbd.ascii = '1';
			system->addEvent(event);
		}

	}


	if ((getKeysChanged() & KEY_Y)) {
		event.type = getKeyEvent(KEY_Y);
		event.kbd.keycode = Common::KEYCODE_5;
		event.kbd.ascii = '5';
		system->addEvent(event);
	}
}


void setKeyboardEnable(bool en) {
	if (en == keyboardEnable) return;
	keyboardEnable = en;
	u16 *backupBank = (u16 *) 0x6040000;

	if (keyboardEnable) {


		DS::drawKeyboard(1, 15, backupBank);


		SUB_BG1_CR = BG_TILE_BASE(1) | BG_MAP_BASE(15);

		if (displayModeIs8Bit) {
			SUB_DISPLAY_CR |= DISPLAY_BG1_ACTIVE;	// Turn on keyboard layer
			SUB_DISPLAY_CR &= ~DISPLAY_BG3_ACTIVE;	// Turn off game layer
		} else {
			SUB_DISPLAY_CR |= DISPLAY_BG1_ACTIVE;	// Turn on keyboard layer
			SUB_DISPLAY_CR &= ~DISPLAY_BG0_ACTIVE;	// Turn off console layer
		}

		// Ensure the keyboard is on the lower screen
		POWER_CR |= POWER_SWAP_LCDS;


	} else {

		DS::releaseAllKeys();
		// Restore the palette that the keyboard has used
		for (int r = 0; r < 256; r++) {
			BG_PALETTE_SUB[r] = BG_PALETTE[r];
		}

		if (displayModeIs8Bit) {
			// Copy the sub screen VRAM from the top screen - they should always be
			// the same.
			u16 *buffer = get8BitBackBuffer();
			s32 stride = get8BitBackBufferStride();

			for (int y = 0; y < gameHeight; y++) {
				for (int x = 0; x < gameWidth; x++) {
					BG_GFX_SUB[y * 256 + x] = buffer[(y * (stride / 2)) + x];
				}
			}

			SUB_DISPLAY_CR &= ~DISPLAY_BG1_ACTIVE;	// Turn off keyboard layer
			SUB_DISPLAY_CR |= DISPLAY_BG3_ACTIVE;	// Turn on game layer
		} else {
			SUB_DISPLAY_CR &= ~DISPLAY_BG1_ACTIVE;	// Turn off keyboard layer
			SUB_DISPLAY_CR |= DISPLAY_BG0_ACTIVE;	// Turn on console layer
		}

		// Restore the screens so they're the right way round
		if (gameScreenSwap) {
			POWER_CR |= POWER_SWAP_LCDS;
		} else {
			POWER_CR &= ~POWER_SWAP_LCDS;
		}
	}
}

bool getKeyboardEnable() {
	return keyboardEnable;
}

bool getIsDisplayMode8Bit() {
	return displayModeIs8Bit;
}

void doScreenTapMode(OSystem_DS *system) {
	Common::Event event;
	static bool left = false, right = false;

	if (left) {
		event.type = Common::EVENT_LBUTTONUP;
		event.mouse = Common::Point(getPenX(), getPenY());
		system->addEvent(event);
		left = false;
	}

	if (right) {
		event.type = Common::EVENT_RBUTTONUP;
		event.mouse = Common::Point(getPenX(), getPenY());
		system->addEvent(event);
		right = false;
	}


	if (tapComplete == 1) {
		event.type = Common::EVENT_LBUTTONDOWN;
		event.mouse = Common::Point(getPenX(), getPenY());
		system->addEvent(event);
		tapComplete = 0;
		left = true;
	} else if (tapComplete == 2) {
		event.type = Common::EVENT_RBUTTONDOWN;
		event.mouse = Common::Point(getPenX(), getPenY());
		system->addEvent(event);
		tapComplete = 0;
		right = true;
	}

	if (!isScrollingWithDPad()) {

		if (getKeysDown() & KEY_LEFT) {
			event.type = Common::EVENT_LBUTTONDOWN;
			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
		}

		if (getKeysReleased() & KEY_LEFT) {
			event.type = Common::EVENT_LBUTTONUP;
			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
		}


		if (getKeysDown() & KEY_RIGHT) {
			event.type = Common::EVENT_RBUTTONDOWN;
			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
		}

		if (getKeysReleased() & KEY_RIGHT) {
			event.type = Common::EVENT_RBUTTONUP;
			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
		}
	}

	event.type = Common::EVENT_MOUSEMOVE;
	event.mouse = Common::Point(getPenX(), getPenY());
	system->addEvent(event);
}

void doButtonSelectMode(OSystem_DS *system) {
	Common::Event event;


	if (!isScrollingWithDPad()) {
		event.type = Common::EVENT_MOUSEMOVE;
		event.mouse = Common::Point(getPenX(), getPenY());
		system->addEvent(event);
	}

	if (getPenReleased() && (leftButtonDown || rightButtonDown)) {
		if (leftButtonDown) {
			event.type = Common::EVENT_LBUTTONUP;
			leftButtonDown = false;
			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
		} else if (rightButtonDown) {
			event.type = Common::EVENT_RBUTTONUP;
			rightButtonDown = false;
			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
		}
	}


	if ((mouseMode != MOUSE_HOVER) || (!displayModeIs8Bit)) {
		if (getPenDown() && !isScrollingWithDPad()) {
			if (mouseMode == MOUSE_LEFT) {
				event.type = Common::EVENT_LBUTTONDOWN;
				leftButtonDown = true;
			} else {
				event.type = Common::EVENT_RBUTTONDOWN;
				rightButtonDown = true;
			}

			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
		}

	} else {
		// In hover mode, D-pad left and right click the mouse when the pen is on the screen

		if (getPenHeld()) {
			if (getKeysDown() & KEY_LEFT) {
				event.type = Common::EVENT_LBUTTONDOWN;
				event.mouse = Common::Point(getPenX(), getPenY());
				system->addEvent(event);
			}
			if (getKeysReleased() & KEY_LEFT) {
				event.type = Common::EVENT_LBUTTONUP;
				event.mouse = Common::Point(getPenX(), getPenY());
				system->addEvent(event);
			}


			if (getKeysDown() & KEY_RIGHT) {
				event.type = Common::EVENT_RBUTTONDOWN;
				event.mouse = Common::Point(getPenX(), getPenY());
				system->addEvent(event);
			}
			if (getKeysReleased() & KEY_RIGHT) {
				event.type = Common::EVENT_RBUTTONUP;
				event.mouse = Common::Point(getPenX(), getPenY());
				system->addEvent(event);
			}
		}
	}

	if (!isScrollingWithDPad() && !getIndyFightState() && !getKeyboardEnable()) {

		if (!getPenHeld() || (mouseMode != MOUSE_HOVER)) {
			if (getKeysDown() & KEY_LEFT) {
				mouseMode = MOUSE_LEFT;
			}

			if (rightButtonDown) {
				event.mouse = Common::Point(getPenX(), getPenY());
				event.type = Common::EVENT_RBUTTONUP;
				system->addEvent(event);
				rightButtonDown = false;
			}


			if (getKeysDown() & KEY_RIGHT) {
				if ((s_currentGame->control != CONT_SCUMM_SAMNMAX) && (s_currentGame->control != CONT_FUTURE_WARS) && (s_currentGame->control != CONT_GOBLINS)) {
					mouseMode = MOUSE_RIGHT;
				} else {
					// If we're playing sam and max, click and release the right mouse
					// button to change verb

					if (s_currentGame->control == CONT_FUTURE_WARS) {
						event.mouse = Common::Point(320 - 128, 200 - 128);
						event.type = Common::EVENT_MOUSEMOVE;
						system->addEvent(event);
					} else {
						event.mouse = Common::Point(getPenX(), getPenY());
					}

					rightButtonDown = true;


					event.type = Common::EVENT_RBUTTONDOWN;
					system->addEvent(event);
				}
			}



			if (getKeysDown() & KEY_UP) {
				mouseMode = MOUSE_HOVER;
			}
		}
	}
}

void addEventsToQueue() {
	#ifdef HEAVY_LOGGING
	printf("addEventsToQueue\n");
	#endif
	OSystem_DS *system = OSystem_DS::instance();
	Common::Event event;

	if (system->isEventQueueEmpty()) {

		if ((keysHeld() & KEY_L) && (keysHeld() & KEY_R)) {
			memoryReport();
		}

		if (displayModeIs8Bit) {

			if (!indyFightState) {

				if (!isScrollingWithDPad() && (getKeysDown() & KEY_B)) {
					if (s_currentGame->control == CONT_AGI) {
						event.kbd.keycode = Common::KEYCODE_RETURN;
						event.kbd.ascii = 13;
						event.kbd.flags = 0;
					} else {
						event.kbd.keycode = Common::KEYCODE_ESCAPE;
						event.kbd.ascii = 27;
						event.kbd.flags = 0;
					}

					event.type = Common::EVENT_KEYDOWN;
					system->addEvent(event);

					event.type = Common::EVENT_KEYUP;
					system->addEvent(event);
				}

			}



			if ((!getIndyFightState()) && (getKeysDown() & KEY_Y)) {
				consoleEnable = !consoleEnable;
				printf("Console enable: %d\n", consoleEnable);
				if (displayModeIs8Bit) {
					displayMode8Bit();
				} else {
					displayMode16Bit();
				}
			}

			if ((getKeyboardEnable())) {
				event.kbd.flags = 0;

				bool down = getKeysDown() & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN);
				bool release = getKeysReleased() & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN);
				bool shoulders = getKeysHeld() & (KEY_L | KEY_R);

				if ( (down && (!shoulders)) || release) {

					if (getKeysChanged() & KEY_LEFT) {
						event.kbd.keycode = Common::KEYCODE_LEFT;
						event.kbd.ascii = 0;
						event.type = getKeyEvent(KEY_LEFT);
						system->addEvent(event);
					}

					if (getKeysChanged() & KEY_RIGHT) {
						event.kbd.keycode = Common::KEYCODE_RIGHT;
						event.kbd.ascii = 0;
						event.type = getKeyEvent(KEY_RIGHT);
						system->addEvent(event);
					}

					if (getKeysChanged() & KEY_UP) {
						event.kbd.keycode = Common::KEYCODE_UP;
						event.kbd.ascii = 0;
						event.type = getKeyEvent(KEY_UP);
						system->addEvent(event);
					}

					if (getKeysChanged() & KEY_DOWN) {
						event.kbd.keycode = Common::KEYCODE_DOWN;
						event.kbd.ascii = 0;
						event.type = getKeyEvent(KEY_DOWN);
						system->addEvent(event);
					}
				}

			}

			if (!isScrollingWithDPad() && !getIndyFightState() && !getKeyboardEnable()) {

				if ((getKeysDown() & KEY_A) && (!indyFightState)) {
					gameScreenSwap = !gameScreenSwap;

					if (gameScreenSwap) {
						POWER_CR |= POWER_SWAP_LCDS;
					} else {
						POWER_CR &= ~POWER_SWAP_LCDS;
					}

				}
			}


			static int selectTimeDown = -1;
			static const int SELECT_HOLD_TIME = 1000;

			if (getKeysDown() & KEY_SELECT) {
				selectTimeDown = getMillis();
			}

			if (selectTimeDown != -1) {
				if (getKeysHeld() & KEY_SELECT) {
					if (getMillis() - selectTimeDown >= SELECT_HOLD_TIME) {
						// Hold select down for one second - show GMM
						g_engine->openMainMenuDialog();
					}
				}

				if (getKeysReleased() & KEY_SELECT) {
					if (getMillis() - selectTimeDown < SELECT_HOLD_TIME) {
						// Just pressed select - show DS options screen
						showOptionsDialog();
					}
				}
			}
		}

		if (!getIndyFightState() && !isScrollingWithDPad() && (getKeysDown() & KEY_X)) {
			setKeyboardEnable(!keyboardEnable);
		}

		updateStatus();


		if ((tapScreenClicks) && (getIsDisplayMode8Bit())) {
			if ((!keyboardEnable) || (!isInsideKeyboard(penDownX, penDownY))) {
				doScreenTapMode(system);
			}
		} else {
			if (!keyboardEnable) {
				doButtonSelectMode(system);
			} else if ((!keyboardEnable) || (!isInsideKeyboard(penDownX, penDownY))) {
				doScreenTapMode(system);
			}
		}


		if (!keyboardEnable) {
			if ((!isScrollingWithDPad() || (indyFightState)) && (displayModeIs8Bit)) {
				// Controls specific to the control method

				if (s_currentGame->control == CONT_SKY) {
					// Extra controls for Beneath a Steel Sky
					if ((getKeysDown() & KEY_DOWN)) {
						penY = 0;
						penX = 160;		// Show inventory by moving mouse onto top line
					}
				}

				if (s_currentGame->control == CONT_AGI) {
					// Extra controls for Leisure Suit Larry and KQ4
					if ((getKeysHeld() & KEY_UP) && (getKeysHeld() & KEY_START)) {
						printf("Cheat key!\n");
						event.type = Common::EVENT_KEYDOWN;
						event.kbd.keycode = (Common::KeyCode)'X';		// Skip age test in LSL
						event.kbd.ascii = 'X';
						event.kbd.flags = Common::KBD_ALT;
						system->addEvent(event);

						event.type = Common::EVENT_KEYUP;
						system->addEvent(event);
					}
				}

				if (s_currentGame->control == CONT_SIMON) {
					// Extra controls for Simon the Sorcerer
					if ((getKeysDown() & KEY_DOWN)) {
						event.type = Common::EVENT_KEYDOWN;
						event.kbd.keycode = Common::KEYCODE_F10;		// F10 or # - show hotspots
						event.kbd.ascii = Common::ASCII_F10;
						event.kbd.flags = 0;
						system->addEvent(event);

						event.type = Common::EVENT_KEYUP;
						system->addEvent(event);
					}
				}

				if (s_currentGame->control == CONT_SCUMM_ORIGINAL) {
					// Extra controls for Scumm v1-5 games
					if ((getKeysDown() & KEY_DOWN)) {
						event.type = Common::EVENT_KEYDOWN;
						event.kbd.keycode = Common::KEYCODE_PERIOD;		// Full stop - skips current dialogue line
						event.kbd.ascii = '.';
						event.kbd.flags = 0;
						system->addEvent(event);

						event.type = Common::EVENT_KEYUP;
						system->addEvent(event);
					}

					if (indyFightState) {
						addIndyFightingKeys();
					}

				}

			}
		}

		if (!displayModeIs8Bit) {
			// Front end controls

			if (leftHandedSwap(getKeysChanged()) & KEY_UP) {
				event.type = getKeyEvent(leftHandedSwap(KEY_UP));
				event.kbd.keycode = Common::KEYCODE_UP;
				event.kbd.ascii = 0;
				event.kbd.flags = 0;
				system->addEvent(event);
			}

			if (leftHandedSwap(getKeysChanged()) & KEY_DOWN) {
				event.type = getKeyEvent(leftHandedSwap(KEY_DOWN));
				event.kbd.keycode = Common::KEYCODE_DOWN;
				event.kbd.ascii = 0;
				event.kbd.flags = 0;
				system->addEvent(event);
			}

			if (leftHandedSwap(getKeysDown()) & KEY_A) {
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_RETURN;
				event.kbd.ascii = 0;
				event.kbd.flags = 0;
				system->addEvent(event);

				event.type = Common::EVENT_KEYUP;
				system->addEvent(event);
			}

		}


		if ((getKeysChanged() & KEY_START)) {
			event.kbd.flags = 0;
			event.type = getKeyEvent(KEY_START);
			if (s_currentGame->control == CONT_FUTURE_WARS) {
				event.kbd.keycode = Common::KEYCODE_F10;
				event.kbd.ascii = Common::ASCII_F10;
			} else if (s_currentGame->control == CONT_GOBLINS) {
				event.kbd.keycode = Common::KEYCODE_F1;
				event.kbd.ascii = Common::ASCII_F1;
			} else if (s_currentGame->control == CONT_AGI) {
				event.kbd.keycode = Common::KEYCODE_ESCAPE;
				event.kbd.ascii = 27;
			} else {
				event.kbd.keycode = Common::KEYCODE_F5;		// F5
				event.kbd.ascii = Common::ASCII_F5;
			}
			system->addEvent(event);
		}


		if (keyboardEnable) {
			DS::addKeyboardEvents();
		}

		consumeKeys();
		consumePenEvents();
	}
}



void triggerIcon(int imageNum) {
	triggeredIcon = imageNum;
	triggeredIconTimeout = 120;
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

void updateStatus() {
	int offs;

	if (displayModeIs8Bit) {
		if (!tapScreenClicks) {
			switch (mouseMode) {
			case MOUSE_LEFT:
				offs = 1;
				break;
			case MOUSE_RIGHT:
				offs = 2;
				break;
			case MOUSE_HOVER:
				offs = 0;
				break;
			default:
				// Nothing!
				offs = 0;
				break;
			}

			setIcon(0, 208, 150, offs, 0, true);
		}

		if (indyFightState) {
			setIcon(1, (190 - 32), 150, 3, (indyFightRight ? 0 : ATTR1_FLIP_X), true);
		}

		if (triggeredIconTimeout > 0) {
			triggeredIconTimeout--;
			setIcon(4, 16, 150, triggeredIcon, 0, true);
		} else {
			setIcon(4, 0, 0, 0, 0, false);
		}

	} else {
		setIcon(0, 0, 0, 0, 0, false);
		setIcon(1, 0, 0, 0, 0, false);
		setIcon(2, 0, 0, 0, 0, false);
		setIcon(3, 0, 0, 0, 0, false);
		setIcon(4, 0, 0, 0, 0, false);
	}

	if ((keyboardIcon) && (!keyboardEnable) && (!displayModeIs8Bit)) {
		setIconMain(0, 0, 160, 4, 0, true);
	} else {
		setIconMain(0, 0, 0, 0, 0, false);
	}

}

void setMainScreenScroll(int x, int y) {
		BG3_CX = x + (((frameCount & 1) == 0)? 64: 0);
		BG3_CY = y;

		if ((!gameScreenSwap) || (touchPadStyle)) {
			touchX = x >> 8;
			touchY = y >> 8;
		}
}

void setMainScreenScale(int x, int y) {
		if (isCpuScalerEnabled() && (x==320)) {
			BG3_XDX = 256;
			BG3_XDY = 0;
			BG3_YDX = 0;
			BG3_YDY = y;
		} else {
			BG3_XDX = x;
			BG3_XDY = 0;
			BG3_YDX = 0;
			BG3_YDY = y;
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


		SUB_BG3_CX = x + ((shake && (frameCount & 1) == 0)? 64: 0);
		SUB_BG3_CY = y;
}

void setZoomedScreenScale(int x, int y) {
		if ((gameScreenSwap) && (!touchPadStyle)) {
			touchScX = x;
			touchScY = y;
		}

		SUB_BG3_XDX = x;
		SUB_BG3_XDY = 0;
		SUB_BG3_YDX = 0;
		SUB_BG3_YDY = y;
}

void VBlankHandler(void) {
	if ((!gameScreenSwap) && !isScrollingWithDPad()) {
		if (s_currentGame) {
			if (s_currentGame->control != CONT_SCUMM_SAMNMAX) {
				if (getPenHeld() && (getPenY() < SCUMM_GAME_HEIGHT)) {
					setTopScreenTarget(getPenX(), getPenY());
				}
			} else {
				if (getPenHeld()) {
					setTopScreenTarget(getPenX(), getPenY());
				}
			}
		}
	}


	penUpdate();
	keysUpdate();


	frameCount++;

	if ((cursorEnable) && (mouseCursorVisible)) {
		storedMouseX = penX;
		storedMouseY = penY;

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


	if (callback) {
		callbackTimer -= FRAME_TIME;
	}

	if (isScrollingWithDPad()) {

		if ((!dragging) && (getPenHeld()) && (penDownFrames > 5)) {
			dragging = true;
			dragStartX = penX;
			dragStartY = penY;

			if (gameScreenSwap) {
				dragScX = subScTargetX;
				dragScY = subScTargetY;
			} else {
				dragScX = scX;
				dragScY = scY;
			}


		}

		if ((dragging) && (!getPenHeld())) {
			dragging = false;
		}

		if (dragging) {

			if (gameScreenSwap) {
				subScTargetX = dragScX + ((dragStartX - penX) << 8);
				subScTargetY = dragScY + ((dragStartY - penY) << 8);
			} else {
				scX = dragScX + ((dragStartX - penX));
				scY = dragScY + ((dragStartY - penY));
			}
		}
	}

	static int ratio = (320 << 8) / SCUMM_GAME_WIDTH;

	bool zooming = false;

	if (isScrollingWithDPad()) {
		if ((getKeysHeld() & KEY_A) && (subScreenScale < ratio)) {
			subScreenScale += 1;
			zooming = true;
		}

		if ((getKeysHeld() & KEY_B) && (subScreenScale > 128)) {
			subScreenScale -=1;
			zooming = true;
		}
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
			if (zooming) {
				subScX = subScTargetX;
				subScY = subScTargetY;
			 	triggerIcon(5);
			}
		} else if ( ((subScreenWidth) > 128 - 8) && ((subScreenWidth) < 128 + 8) ) {
			subScreenWidth = 128;
			subScreenHeight = 96;
			if (zooming) {
				subScX = subScTargetX;
				subScY = subScTargetY;
				triggerIcon(6);
			}
		} else if (subScreenWidth > 256) {
			subScreenWidth = 320;
			subScreenHeight = 200;
			if (zooming) {
				subScX = subScTargetX;
				subScY = subScTargetY;
				triggerIcon(7);
			}
		}
	}


	subScTargetX = xCenter - ((subScreenWidth  >> 1) << 8);
	subScTargetY = yCenter - ((subScreenHeight >> 1) << 8);

	subScTargetX = CLIP(subScTargetX, 0, (gameWidth  - subScreenWidth)  << 8);
	subScTargetY = CLIP(subScTargetY, 0, (gameHeight - subScreenHeight) << 8);

	subScX += (subScTargetX - subScX) >> 2;
	subScY += (subScTargetY - subScY) >> 2;

	if (displayModeIs8Bit) {

		if (isScrollingWithDPad()) {

			int offsX = 0, offsY = 0;


			if ((getKeysHeld() & KEY_LEFT)) {
				offsX -= 2;
			}

			if ((getKeysHeld() & KEY_RIGHT)) {
				offsX += 2;
			}

			if ((getKeysHeld() & KEY_UP)) {
				offsY -= 2;
			}

			if ((getKeysHeld() & KEY_DOWN)) {
				offsY += 2;
			}

			if (((gameScreenSwap) && (getKeysHeld() & KEY_L)) || ((!gameScreenSwap) && (getKeysHeld() & KEY_R))) {
				subScTargetX += offsX << 8;
				subScTargetY += offsY << 8;
			} else {
				scX += offsX;
				scY += offsY;
			}
		}

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

	// Enable on screen keyboard when pen taps icon
	if ((keyboardIcon) && (penX < 32) && (penY > 160) && (penHeld)) {
		setKeyboardEnable(true);
	}

	if (keyboardEnable) {
		if (DS::getKeyboardClosed()) {
			setKeyboardEnable(false);
		}
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

void uploadSpriteGfx() {
	vramSetBankD(VRAM_D_SUB_SPRITE);
	vramSetBankE(VRAM_E_MAIN_SPRITE);

	// Convert texture from 24bit 888 to 16bit 1555, remembering to set top bit!
	const u8 *srcTex = (const u8 *) ::icons_raw;
	for (int r = 32 * 256 ; r >= 0; r--) {
		SPRITE_GFX_SUB[r] = 0x8000 | (srcTex[r * 3] >> 3) | ((srcTex[r * 3 + 1] >> 3) << 5) | ((srcTex[r * 3 + 2] >> 3) << 10);
		SPRITE_GFX[r] = 0x8000 | (srcTex[r * 3] >> 3) | ((srcTex[r * 3 + 1] >> 3) << 5) | ((srcTex[r * 3 + 2] >> 3) << 10);
	}

}

void initHardware() {
	penInit();
	indyFightState = false;
	indyFightRight = true;

	lastEventFrame = 0;
	mouseMode = MOUSE_LEFT;

	powerOn(POWER_ALL);
	vramSetBankD(VRAM_D_SUB_SPRITE);
	vramSetBankE(VRAM_E_MAIN_SPRITE);

	currentTimeMillis = 0;

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

	POWER_CR &= ~POWER_SWAP_LCDS;

	frameCount = 0;
	callback = NULL;

	BG_PALETTE[255] = RGB15(31,31,31);//by default font will be rendered with color 255

	//irqs are nice
	irqSet(IRQ_VBLANK, VBlankHandler);
	irqSet(IRQ_TIMER0, timerTickHandler);

	irqEnable(IRQ_VBLANK);
	irqEnable(IRQ_TIMER0);

	// Set up a millisecond timer
	#ifdef HEAVY_LOGGING
	printf("Setting up timer...");
	#endif
	TIMER0_CR = 0;
	TIMER0_DATA = (u32) TIMER_FREQ(1000);
	TIMER0_CR = TIMER_ENABLE | TIMER_DIV_1 | TIMER_IRQ_REQ;
	REG_IME = 1;
	#ifdef HEAVY_LOGGING
	printf("done\n");
	#endif

	BG_PALETTE[255] = RGB15(0,0,31);

	initSprites();

	// If the software scaler's back buffer has not been allocated, do it now
	scalerBackBuffer = (u8 *) malloc(320 * 256);


	WAIT_CR &= ~(0x0080);

	uploadSpriteGfx();

	// This is a bodge to get around the fact that the cursor is turned on before it's image is set
	// during startup in Sam & Max.  This bodge moves the cursor offscreen so it is not seen.
	sprites[1].attribute[1] = ATTR1_SIZE_64 | 192;

}




void setKeyboardIcon(bool enable) {
	keyboardIcon = enable;
}

bool getKeyboardIcon() {
	return keyboardIcon;
}


////////////////////
// Pen stuff
////////////////////


void penInit() {
	penDown = false;
	penHeld = false;
	penReleased = false;
	penDownLastFrame = false;
	penDownSaved = false;
	penReleasedSaved = false;
	penDownFrames = 0;
	consumeKeys();
}

void penUpdate() {

	touchPosition touchPos;
	touchRead(&touchPos);

	bool penDownThisFrame = (keysCurrent() & KEY_TOUCH) && (touchPos.px > 0) && (touchPos.py > 0);
	static bool moved = false;

	if (( (tapScreenClicks) || getKeyboardEnable() ) && (getIsDisplayMode8Bit())) {


		if ((tapTimeout >= 0)) {
			tapTimeout++;

			if (((tapTimeout > 15) || (tapCount == 2)) && (tapCount > 0)) {
				tapComplete = tapCount;
				tapCount = 0;
			}
		}



		if ((penHeld) && (!penDownThisFrame)) {
			if ((touchPadStyle) || (getKeyboardEnable() && (!isInsideKeyboard(penDownX, penDownY))) || (moved) || (tapCount == 1)) {
				if ((penDownFrames > 0) && (penDownFrames < 6) && ((tapTimeout == -1) || (tapTimeout > 2))) {
					tapCount++;
					tapTimeout = 0;
					moved = false;
				}
			}
		}
	}


	if ( ((keyboardEnable) || (touchPadStyle)) && (getIsDisplayMode8Bit()) ) {
		// Relative positioning mode


		if ((penDownFrames > 0) ) {

			if ((penHeld)) {

				if (penDownThisFrame) {
					if (penDownFrames >= 2) {

						touchRead(&touchPos);
						if ((!keyboardEnable) || (!isInsideKeyboard(touchPos.px, touchPos.py))) {
							int diffX = touchPos.px - penDownX;
							int diffY = touchPos.py - penDownY;

							int speed = ABS(diffX) + ABS(diffY);

							if ((ABS(diffX) < 35) && (ABS(diffY) < 35)) {

								if (speed >= 8)	{
									diffX *= ((speed >> 3) * touchPadSensitivity) >> 3;
									diffY *= ((speed >> 3) * touchPadSensitivity) >> 3;
								}

								penX += diffX;
								penY += diffY;

								if (penX > 255) {
									scX -= 255 - penX;
									penX = 255;
								}

								if (penX < 0) {
									scX -= -penX;
									penX = 0;
								}

								if (penY > 191) {
									scY += penY - 191;
									penY = 191;
								}

								if (penY < 0) {
									scY -= -penY;
									penY = 0;
								}
							}
						}
						penDownX = touchPos.px;
						penDownY = touchPos.py;

					}
				}

			} else {
				penDown = true;
				penHeld = true;
				penDownSaved = true;

				// First frame, so save pen positions
				if (penDownThisFrame) {
					touchRead(&touchPos);
					penDownX = touchPos.px;
					penDownY = touchPos.py;
				}
			}
		} else {
			if (penHeld) {
				penReleased = true;
				penReleasedSaved = true;
			} else {
				penReleased = false;
			}

			penDown = false;
			penHeld = false;
		}


	} else {	// Absolute positioning mode
		if ((penDownFrames > 1)) {			// Is this right?  Dunno, but it works for me.

			if ((penHeld)) {
				penHeld = true;
				penDown = false;
			} else {
				if (penDownFrames == 2) {
					touchRead(&touchPos);
					penDownX = touchPos.px;
					penDownY = touchPos.py;
				}
				penDown = true;
				penHeld = true;
				penDownSaved = true;
			}

			touchRead(&touchPos);
			if ((keysCurrent() & KEY_TOUCH) && (touchPos.px > 0) && (touchPos.py > 0)) {
				penX = touchPos.px + touchXOffset;
				penY = touchPos.py + touchYOffset;
				moved = true;
			}


		} else {
			if (penHeld) {
				penReleased = true;
				penReleasedSaved = true;
			} else {
				penReleased = false;
			}

			penDown = false;
			penHeld = false;
		}
	}



	if ((keysCurrent() & KEY_TOUCH) || ((penDownFrames == 2)) ) {
		penDownLastFrame = true;
		penDownFrames++;
	} else {
		penDownLastFrame = false;
		penDownFrames = 0;
	}
}

int leftHandedSwap(int keys) {
	// Start and select are unchanged
	if (leftHandedMode) {
		int result = keys & (~(KEY_R | KEY_L | KEY_Y | KEY_A | KEY_B | KEY_X | KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN));

		if (keys & KEY_L) result |= KEY_R;
		if (keys & KEY_R) result |= KEY_L;

		if (keys & KEY_LEFT) result |= KEY_Y;
		if (keys & KEY_RIGHT) result |= KEY_A;
		if (keys & KEY_DOWN) result |= KEY_B;
		if (keys & KEY_UP) result |= KEY_X;

		if (keys & KEY_Y) result |= KEY_LEFT;
		if (keys & KEY_A) result |= KEY_RIGHT;
		if (keys & KEY_B) result |= KEY_DOWN;
		if (keys & KEY_X) result |= KEY_UP;

		return result;
	} else {
		return keys;
	}
}

void keysUpdate() {
	scanKeys();
	keysDownSaved |= leftHandedSwap(keysDown());
	keysReleasedSaved |= leftHandedSwap(keysUp());
	keysChangedSaved = keysDownSaved | keysReleasedSaved;
}

int getKeysDown() {
	return keysDownSaved;
}

int getKeysHeld() {
	return leftHandedSwap(keysHeld());
}

int getKeysReleased() {
	return keysReleasedSaved;
}

int getKeysChanged() {
	return keysChangedSaved;
}

Common::EventType getKeyEvent(int key) {
	if (getKeysDown() & key) {
		return Common::EVENT_KEYDOWN;
	} else if (getKeysReleased() & key) {
		return Common::EVENT_KEYUP;
	} else {
		return (Common::EventType) 0;
	}
}

void consumeKeys() {
	keysDownSaved = 0;
	keysReleasedSaved = 0;
	keysChangedSaved = 0;
}

bool getPenDown() {
	return penDownSaved;
}

bool getPenHeld() {
	return penHeld;
}

bool getPenReleased() {
	return penReleasedSaved;
}

void consumePenEvents() {
	penDownSaved = false;
	penReleasedSaved = false;
}

int getPenX() {
	int x = ((penX * touchScX) >> 8) + touchX;
	x = x < 0? 0: (x > gameWidth - 1? gameWidth - 1: x);

	if (snapToBorder) {
		if (x < 8) x = 0;
		if (x > gameWidth - 8) x = gameWidth - 1;
	}

	return x;
}

int getPenY() {
	int y = ((penY * touchScY) >> 8) + touchY;
	y = y < 0? 0: (y > gameHeight - 1? gameHeight - 1: y);

	if (snapToBorder) {
		if (y < 8) y = 0;
		if (y > gameHeight - 8) y = gameHeight - 1;
	}

	return y;
}

GLvector getPenPos() {
	GLvector v;

	v.x = (penX * inttof32(1)) / SCREEN_WIDTH;
	v.y = (penY * inttof32(1)) / SCREEN_HEIGHT;

	return v;
}

void setIndyFightState(bool st) {
	indyFightState = st;
	indyFightRight = true;
}

bool getIndyFightState() {
	return indyFightState;
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

	printf("-------------------------------\n");
	printf("ScummVM DS\n");
	printf("Ported by Neil Millstone\n");
	printf("Version %s ", gScummVMVersion);
	printf("-------------------------------\n");
	printf("L/R + D-pad/pen:    Scroll view\n");
	printf("D-pad left:   Left mouse button\n");
	printf("D-pad right: Right mouse button\n");
	printf("D-pad up:           Hover mouse\n");
	printf("B button:        Skip cutscenes\n");
	printf("Select:         DS Options menu\n");
	printf("Start:   Game menu (some games)\n");
	printf("Y (in game):     Toggle console\n");
	printf("X:              Toggle keyboard\n");
	printf("A:                 Swap screens\n");
	printf("L+R (on start):      Clear SRAM\n");
	printf("\n");

	if (!nitroFSInit(NULL)) {
		printf("nitroFSInit failure: terminating\n");
		return(1);
	}

	DS::updateStatus();

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
