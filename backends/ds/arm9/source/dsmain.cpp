/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 Neil Millstone
 * Copyright (C) 2006 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
 

// - Turn off when quit - Done
// - Simon and Kyrandia - Done
// - 200% scale option - Done
// - Change zoom range - Done
// - Speed increase! - Done
// - Fixed bugs in Sky - Done
// - Change name of ini file and intro screen for build c - Done
// - Check for existance of zip file in batch file - Done
// - Add new support - Done
// - Fix help screen

// - Remove scummconsole.c
// - Delete files
// - Fatlib conversion

#include <nds.h>

#include <ARM9/console.h> //basic print funcionality

#include <stdlib.h>
#include "dsmain.h"
#include "string.h"
#include "system.h"
#include "osystem_ds.h"
#include "icons_raw.h"
#include "gba_nds_fat.h"
#include "disc_io.h"
#include "config-manager.h"
#include "engines/scumm/scumm.h"
#include "keyboard_raw.h"
#include "keyboard_pal_raw.h"
#define V16(a, b) ((a << 12) | b)
#include "touchkeyboard.h"
#include "registers_alt.h"
//#include "compact_flash.h"
#include "dsoptions.h"
#include "user_debugger.h"
#include "ramsave.h"
#include "disc_io.h"

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

int textureID;
u16* texture;

int frameCount;
int currentTimeMillis;

// Timer Callback
int callbackInterval;
int callbackTimer;
OSystem::TimerProc callback;

// Scaled
bool scaledMode;
int scX;
int scY;

int subScX;
int subScY;
int subScTargetX;
int subScTargetY;
int subScreenWidth = SCUMM_GAME_WIDTH;
int subScreenHeight = SCUMM_GAME_HEIGHT;
int subScreenScale = 256;



// Sound
int bufferSize;
s16* soundBuffer;
int bufferFrame;
int bufferRate;
int bufferSamples;
bool soundHiPart;
int soundFrequency;

// Events
int lastEventFrame;
bool indyFightState;
bool indyFightRight;

OSystem::SoundProc soundCallback;
void* soundParam;
int lastCallbackFrame;
bool bufferFirstHalf;
bool bufferSecondHalf;

// Saved buffers
u8* savedBuffer = NULL;
bool highBuffer;
bool displayModeIs8Bit = false;

// Game id
u8 gameID;

bool consoleEnable = false;
bool gameScreenSwap = false;

MouseMode mouseMode;

// Sprites
SpriteEntry sprites[128];
SpriteEntry spritesMain[128];
int tweak;

// Shake
int shakePos = 0;

// Keyboard
bool keyboardEnable = false;
bool leftHandedMode = false;
bool keyboardIcon = false;

// Touch
int touchScX, touchScY, touchX, touchY;

// Dragging
int dragStartX, dragStartY;
bool dragging = false;
int dragScX, dragScY;

// Interface styles
char gameName[32];

// 8-bit surface size
int gameWidth = 320;
int gameHeight = 200;

// Scale
bool twoHundredPercentFixedScale = false;

enum controlType {
	CONT_SCUMM_ORIGINAL,
	CONT_SCUMM_SAMNMAX,
	CONT_SKY,
	CONT_SIMON,
};

struct gameListType {
	char 			gameId[16];
	controlType 	control;
};

#define NUM_SUPPORTED_GAMES 15

gameListType gameList[NUM_SUPPORTED_GAMES] = {
	// Unknown game - use normal SCUMM controls
	{"unknown", 	CONT_SCUMM_ORIGINAL},
	
	// SCUMM games
	{"maniac",		CONT_SCUMM_ORIGINAL},
	{"zak",			CONT_SCUMM_ORIGINAL},
	{"loom",		CONT_SCUMM_ORIGINAL},
	{"indy3",		CONT_SCUMM_ORIGINAL},
	{"atlantis",	CONT_SCUMM_ORIGINAL},
	{"monkey",		CONT_SCUMM_ORIGINAL},
	{"monkey2",		CONT_SCUMM_ORIGINAL},
	{"tentacle",	CONT_SCUMM_ORIGINAL},
	{"samnmax",		CONT_SCUMM_SAMNMAX},
	
	// Non-SCUMM games
	{"sky",			CONT_SKY},
	{"simon1",		CONT_SIMON},
	{"simon2",		CONT_SIMON},
	{"gob1",		CONT_SCUMM_ORIGINAL},
	{"queen",		CONT_SCUMM_ORIGINAL}
};

gameListType* currentGame = NULL;

// Stylus
#define ABS(x) ((x)>0?(x):-(x))

bool penDown;
bool penHeld;
bool penReleased;
bool penDownLastFrame;
f32 penX, penY;
int keysDownSaved;
int keysReleasedSaved;

bool penDownSaved;
bool penReleasedSaved;
int penDownFrames;
int touchXOffset = 0;
int touchYOffset = 0;

u16 savedPalEntry255 = RGB15(31, 31, 31);


extern "C" int scummvm_main(int argc, char *argv[]);
void updateStatus();

TransferSound soundControl;

//plays an 8 bit mono sample at 11025Hz
void playSound(const void* data, u32 length, bool loop, bool adpcm, int rate)
{
	
	if (!IPC->soundData) {
		soundControl.count = 0;
	}
	
	soundControl.data[soundControl.count].data = data;
	soundControl.data[soundControl.count].len = length | (loop? 0x80000000: 0x00000000);
	soundControl.data[soundControl.count].rate = rate;		// 367 samples per frame
	soundControl.data[soundControl.count].pan = 64;
	soundControl.data[soundControl.count].vol = 127;
	soundControl.data[soundControl.count].format = adpcm? 2: 0;

	soundControl.count++;

	DC_FlushAll();
	IPC->soundData = &soundControl;
}

void stopSound(int channel) {
	playSound(NULL, 0, false, false, -channel);
}

void updateOAM() {
	DC_FlushAll();
    dmaCopy(sprites, OAM_SUB, 128 * sizeof(SpriteEntry));
    dmaCopy(spritesMain, OAM, 128 * sizeof(SpriteEntry));
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
	for(int i = 0; i < 128; i++) {
	   sprites[i].attribute[0] = ATTR0_DISABLED;
	   sprites[i].attribute[1] = 0;
	   sprites[i].attribute[2] = 0;
	   sprites[i].attribute[3] = 0;
    }
	
	for(int i = 0; i < 128; i++) {
	   spritesMain[i].attribute[0] = ATTR0_DISABLED;
	   spritesMain[i].attribute[1] = 0;
	   spritesMain[i].attribute[2] = 0;
	   spritesMain[i].attribute[3] = 0;
    }
	
	updateOAM();
}


void saveGameBackBuffer() {
#ifdef DISABLE_SCUMM
	if (savedBuffer == NULL) savedBuffer = new u8[gameWidth * gameHeight];
	for (int r = 0; r < 200; r++) {
		memcpy(savedBuffer + (r * gameWidth), ((u8 *) (get8BitBackBuffer())) + (r * 512), gameWidth);
	}
#endif
}

void restoreGameBackBuffer() {
#ifdef DISABLE_SCUMM
	if (savedBuffer) {
		for (int r = 0; r < 200; r++) {
			memcpy(((u8 *) (BG_GFX_SUB)) + (r * 512), savedBuffer + (r * gameWidth), gameWidth);
			memcpy(((u8 *) (get8BitBackBuffer())) + (r * 512), savedBuffer + (r * gameWidth), gameWidth);
		}
		delete savedBuffer;
		savedBuffer = NULL;
	}
#endif

#ifndef DISABLE_SCUMM	
	memset(get8BitBackBuffer(), 0, 512 * 256);
	memset(BG_GFX_SUB, 0, 512 * 256);
	if (Scumm::g_scumm) {
		Scumm::g_scumm->markRectAsDirty(Scumm::kMainVirtScreen, 0, gameWidth - 1, 0, gameHeight - 1, 1);
		Scumm::g_scumm->markRectAsDirty(Scumm::kTextVirtScreen, 0, gameWidth - 1, 0, gameHeight - 1, 1);
		Scumm::g_scumm->markRectAsDirty(Scumm::kVerbVirtScreen, 0, gameWidth - 1, 0, gameHeight - 1, 1);
	}
#endif

}


void startSound(int freq, int buffer) {
	bufferRate = freq * 2;
	bufferFrame = 0;
	bufferSamples = 4096;

	bufferFirstHalf = false;
	bufferSecondHalf = true;
	
	int bytes = (2 * (bufferSamples)) + 100;
	
	soundBuffer = (s16 *) malloc(bytes * 2);


	soundHiPart = true;
	
	for (int r = 0; r < bytes; r++) {
		soundBuffer[r] = 0;
	}

	soundFrequency = freq;
	

	swiWaitForVBlank();
	swiWaitForVBlank();
	playSound(soundBuffer, (bufferSamples * 2), true, false, freq * 2);
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
}

int getSoundFrequency() {
	return soundFrequency;
}


void initGame() {
	// This is a good time to check for left handed mode since the mode change is done as the game starts.
	// There's probably a better way, but hey.
//	consolePrintf("initing game\n");

	static bool firstTime = true;


	setOptions();

	//strcpy(gameName, ConfMan.getActiveDomain().c_str());
	strcpy(gameName, ConfMan.get("gameid").c_str());
//	consolePrintf("\n\n\n\nCurrent game: '%s' %d\n", gameName, gameName[0]);

	currentGame = &gameList[0];		// Default game
	
	for (int r = 0; r < NUM_SUPPORTED_GAMES; r++) {
		if (!stricmp(gameName, gameList[r].gameId)) {
			currentGame = &gameList[r];
//			consolePrintf("Game list num: %d\n", currentGame);
		}
	}
		
	if (firstTime) {
		firstTime = false;

		if (ConfMan.hasKey("22khzaudio", "ds") && ConfMan.getBool("22khzaudio", "ds")) {
			startSound(22050, 8192);
		} else {
			startSound(11025, 4096);
		}

	}

}

void setLeftHanded(bool enable) {
	leftHandedMode = enable;
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

	u16 buffer[32 * 32];
	
	setKeyboardEnable(false);

	if (!displayModeIs8Bit) {
		for (int r = 0; r < 32 * 32; r++) {
			buffer[r] = ((u16 *) SCREEN_BASE_BLOCK_SUB(4))[r];
		}
	}
	
	

	videoSetMode(MODE_5_2D | (consoleEnable? DISPLAY_BG0_ACTIVE: 0) | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP); 
	videoSetModeSub(MODE_3_2D /*| DISPLAY_BG0_ACTIVE*/ | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP); //sub bg 0 will be used to print text

	vramSetBankA(VRAM_A_MAIN_BG_0x6000000);
	vramSetBankB(VRAM_B_MAIN_BG_0x6020000);

	vramSetBankC(VRAM_C_SUB_BG_0x6200000);
	vramSetBankD(VRAM_D_MAIN_BG_0x6040000);
	
	vramSetBankH(VRAM_H_LCD);

	BG3_CR = BG_BMP8_512x256 | BG_BMP_BASE(8);
	
	
	
	BG3_XDX = (int) (((float) (gameWidth) / 256.0f) * 256);
    BG3_XDY = 0;
    BG3_YDX = 0;
    BG3_YDY = (int) ((200.0f / 192.0f) * 256);

	SUB_BG3_CR = BG_BMP8_512x256;
	
	
	
	SUB_BG3_XDX = (int) (subScreenWidth / 256.0f * 256);
    SUB_BG3_XDY = 0;
    SUB_BG3_YDX = 0;
    SUB_BG3_YDY = (int) (subScreenHeight / 192.0f * 256);


	// Do text stuff
	BG0_CR = BG_MAP_BASE(0) | BG_TILE_BASE(1);
	BG0_Y0 = 0;
	
	// Restore palette entry used by text in the front-end	
	PALETTE_SUB[255] = savedPalEntry255;
	
	consoleInitDefault((u16*)SCREEN_BASE_BLOCK(0), (u16*)CHAR_BASE_BLOCK(1), 16);
	consolePrintSet(0, 23);
	
	if (!displayModeIs8Bit) {
		for (int r = 0; r < 32 * 32; r++) {
			((u16 *) SCREEN_BASE_BLOCK(0))[r] = buffer[r];
		}
//		dmaCopyHalfWords(3, (u16 *) SCREEN_BASE_BLOCK(0), buffer, 32 * 32 * 2);
	}
	
	
	if (!displayModeIs8Bit) restoreGameBackBuffer();
	displayModeIs8Bit = true;
	
	POWER_CR &= ~POWER_SWAP_LCDS;
	
	keyboardEnable = false;
	initGame();
	
}

void setGameID(int id) {
	gameID = id;
}

void dummyHandler() {
	REG_IF = IRQ_VBLANK;
}

void checkSleepMode() {
	if (IPC->performArm9SleepMode) {
	
		consolePrintf("ARM9 Entering sleep mode\n");
		
		int intSave = REG_IE;
		irqSet(IRQ_VBLANK, dummyHandler);
//		int irqHandlerSave = (int) IRQ_HANDLER;
		REG_IE = IRQ_VBLANK;
		//IRQ_HANDLER = dummyHandler;
		
		int powerSave = POWER_CR;
		POWER_CR &= ~POWER_ALL;
		
		while (IPC->performArm9SleepMode) {
			swiWaitForVBlank();
		}
		
		POWER_CR = powerSave;
//		IRQ_HANDLER = (void (*)()) irqHandlerSave;
		irqSet(IRQ_VBLANK, VBlankHandler);
		REG_IE = intSave;

		consolePrintf("ARM9 Waking from sleep mode\n");
	}
}

void setCursorIcon(const u8* icon, uint w, uint h, byte keycolor) {
	if (currentGame->control != CONT_SCUMM_SAMNMAX)
		return;

	uint16 border = RGB15(24,24,24) | 0x8000;
	
	
	int off = 48*64;
	memset(SPRITE_GFX_SUB+off, 0, 64*64*2);
	
	int pos = 190 - (w+2);
	

	
	// make border
	for (uint i=0; i<w+2; i++) {
		SPRITE_GFX_SUB[off+i] = border;
		SPRITE_GFX_SUB[off+(31)*64+i] = border;
	}
	for (uint i=1; i<31; i++) {
		SPRITE_GFX_SUB[off+(i*64)] = border;
		SPRITE_GFX_SUB[off+(i*64)+(w+1)] = border;
	}
	
	int offset = (32 - h) >> 1;

	for (uint y=0; y<h; y++) {
		for (uint x=0; x<w; x++) {
			int color = icon[y*w+x];
			if (color == keycolor) {
				SPRITE_GFX_SUB[off+(y+1+offset)*64+(x+1)] = 0x8000; // black background
			} else {
				SPRITE_GFX_SUB[off+(y+1+offset)*64+(x+1)] = BG_PALETTE[color] | 0x8000;
			}
		}
	}

	sprites[1].attribute[0] = ATTR0_BMP | 150;
	sprites[1].attribute[1] = ATTR1_SIZE_64 | pos;
	sprites[1].attribute[2] = ATTR2_ALPHA(1) | 48;
}




void displayMode16Bit() {

	u16 buffer[32 * 32 * 2];


	if (displayModeIs8Bit) {
		saveGameBackBuffer();
		for (int r = 0; r < 32 * 32; r++) {
			buffer[r] = ((u16 *) SCREEN_BASE_BLOCK(0))[r];
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
	
	BG3_XDX = (int) (1.25f * 256);
    BG3_XDY = 0;
    BG3_YDX = 0;
    BG3_YDY = (int) ((200.0f / 192.0f) * 256);

	memset(BG_GFX, 0, 512 * 256 * 2);
	
	savedPalEntry255 = PALETTE_SUB[255];
	PALETTE_SUB[255] = RGB15(31,31,31);//by default font will be rendered with color 255

	// Do text stuff
	SUB_BG0_CR = BG_MAP_BASE(4) | BG_TILE_BASE(0);
	SUB_BG0_Y0 = 0;

	consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(4), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);

	if (displayModeIs8Bit) {
		//dmaCopyHalfWords(3, (u16 *) SCREEN_BASE_BLOCK_SUB(0), buffer, 32 * 32 * 2);
		for (int r = 0; r < 32 * 32; r++) {
			((u16 *) SCREEN_BASE_BLOCK_SUB(4))[r] = buffer[r];
		}
	}

	consolePrintSet(0, 23);
	consolePrintf("\n");
	
	// Show keyboard
	SUB_BG1_CR = BG_TILE_BASE(1) | BG_MAP_BASE(12);
	//drawKeyboard(1, 12);
	
	POWER_CR &= ~POWER_SWAP_LCDS;



	displayModeIs8Bit = false;
}


void displayMode16BitFlipBuffer() {
	if (!displayModeIs8Bit) {
		u16* back = get16BitBackBuffer();
	
//		highBuffer = !highBuffer;
//		BG3_CR = BG_BMP16_512x256 |	BG_BMP_RAM(highBuffer? 1: 0);
		
		for (int r = 0; r < 512 * 256; r++) {
			*(BG_GFX + r) = *(back + r);
		}
	}
}

void setShakePos(int shakePos) {
	shakePos = shakePos;
}


u16* get16BitBackBuffer() {
	return BG_GFX + 0x20000;
}

u16* get8BitBackBuffer() {
	return BG_GFX + 0x10000;		// 16bit qty!
}

void setSoundProc(OSystem::SoundProc proc, void* param) {
//	consolePrintf("Set sound callback");
	soundCallback = proc;
	soundParam = param;
}

// The sound system in ScummVM seems to always return stereo interleaved samples.
// Here, I'm treating an 11Khz stereo stream as a 22Khz mono stream, which works sorta ok, but is
// a horrible bodge.  Any advice on how to change the engine to output mono would be greatly
// appreciated.
void doSoundCallback() {
	if ((soundCallback)) {
		lastCallbackFrame = frameCount;
		
		for (int r = IPC->playingSection; r < IPC->playingSection + 4; r++) {
			int chunk = r & 3;
			
			if (IPC->fillNeeded[chunk]) {
				IPC->fillNeeded[chunk] = false;
				DC_FlushAll();
				soundCallback(soundParam, (byte *) (soundBuffer + ((bufferSamples >> 2) * chunk)), bufferSamples >> 1);
				IPC->fillNeeded[chunk] = false;
				DC_FlushAll();
			}
		
		}
		
	}
}

void doTimerCallback() {
	if (callback) {
		if (callbackTimer <= 0) {
			callbackTimer += callbackInterval;
			callback(callbackInterval);
		}	
	}
}

void soundUpdate() {
	if ((bufferFrame == 0)) {
//		playSound(soundBuffer, (bufferSamples * 2), true);
	}
//	consolePrintf("%x\n", IPC->test);
	
	
	if (bufferFrame == 0) {
//		bufferFirstHalf = true;
	}	
	if (bufferFrame == bufferSize >> 1) {
	//bufferSecondHalf = true;
	}	
	
	bufferFrame++;
	if (bufferFrame == bufferSize) {
		bufferFrame = 0;
	}
}

void memoryReport() {
	int r = 0;
	int* p;
	do {
		p = (int *) malloc(r * 8192);
		free(p);
		r++;		
	} while ((p) && (r < 512));
	
	int t = -1;
	void* block[1024];
	do {
		t++;
		block[t] = (int *) malloc(4096);
	} while ((t < 1024) && (block[t]));		
	
	for (int q = 0; q < t; q++) {
		free(block[q]);
	}
	
	consolePrintf("Free: %dK, Largest: %dK\n", t * 4, r * 8);
}


void addIndyFightingKeys() {
	OSystem_DS* system = OSystem_DS::instance();
	OSystem::Event event;

	event.type = OSystem::EVENT_KEYDOWN;
	event.kbd.flags = 0;

	if ((getKeysDown() & KEY_L)) {
		indyFightRight = false;
	}

	if ((getKeysDown() & KEY_R)) {
		indyFightRight = true;
	}

	if ((getKeysDown() & KEY_UP)) {
		event.kbd.keycode = '8';
		event.kbd.ascii = '8';
		system->addEvent(event);
	}
	if ((getKeysDown() & KEY_LEFT)) {
		event.kbd.keycode = '4';
		event.kbd.ascii = '4';
		system->addEvent(event);
	}
	if ((getKeysDown() & KEY_RIGHT)) {
		event.kbd.keycode = '6';
		event.kbd.ascii = '6';
		system->addEvent(event);
	}	
	if ((getKeysDown() & KEY_DOWN)) {
		event.kbd.keycode = '2';
		event.kbd.ascii = '2';
		system->addEvent(event);
	}
	
	if (indyFightRight) {
	
		if ((getKeysDown() & KEY_X)) {
			event.kbd.keycode = '9';
			event.kbd.ascii = '9';
			system->addEvent(event);
		}
		if ((getKeysDown() & KEY_A)) {
			event.kbd.keycode = '6';
			event.kbd.ascii = '6';
			system->addEvent(event);
		}
		if ((getKeysDown() & KEY_B)) {
			event.kbd.keycode = '3';
			event.kbd.ascii = '3';
			system->addEvent(event);
		}	

	} else {

		if ((getKeysDown() & KEY_X)) {
			event.kbd.keycode = '7';
			event.kbd.ascii = '7';
			system->addEvent(event);
		}
		if ((getKeysDown() & KEY_A)) {
			event.kbd.keycode = '4';
			event.kbd.ascii = '4';
			system->addEvent(event);
		}
		if ((getKeysDown() & KEY_B)) {
			event.kbd.keycode = '1';
			event.kbd.ascii = '1';
			system->addEvent(event);
		}	
	
	}
	
	
	if ((getKeysDown() & KEY_Y)) {
		event.kbd.keycode = '5';
		event.kbd.ascii = '5';
		system->addEvent(event);
	}
} 			


void setKeyboardEnable(bool en) {
	if (en == keyboardEnable) return;
	keyboardEnable = en;
	u16* backupBank = (u16 *) 0x6040000;

	if (keyboardEnable) {


		DS::drawKeyboard(1, 12, backupBank);
		
		
		SUB_BG1_CR = BG_TILE_BASE(1) | BG_MAP_BASE(12);

		if (displayModeIs8Bit) {
			SUB_DISPLAY_CR |= DISPLAY_BG1_ACTIVE;	// Turn on keyboard layer
			SUB_DISPLAY_CR &= ~DISPLAY_BG3_ACTIVE;	// Turn off game layer
		} else {
			SUB_DISPLAY_CR |= DISPLAY_BG1_ACTIVE;	// Turn on keyboard layer
			SUB_DISPLAY_CR &= ~DISPLAY_BG0_ACTIVE;	// Turn off console layer
		}
		lcdSwap();
	} else {


		// Restore the palette that the keyboard has used
		for (int r = 0; r < 256; r++) {
			BG_PALETTE_SUB[r] = BG_PALETTE[r];
		}
		
		
		//restoreVRAM(1, 12, backupBank);
		
		if (displayModeIs8Bit) {
			// Copy the sub screen VRAM from the top screen - they should always be
			// the same.
			for (int r = 0; r < (512 * 256) >> 1; r++) {
				BG_GFX_SUB[r] = get8BitBackBuffer()[r];
			}
			
			SUB_DISPLAY_CR &= ~DISPLAY_BG1_ACTIVE;	// Turn off keyboard layer
			SUB_DISPLAY_CR |= DISPLAY_BG3_ACTIVE;	// Turn on game layer
		} else {
			SUB_DISPLAY_CR &= ~DISPLAY_BG1_ACTIVE;	// Turn off keyboard layer
			SUB_DISPLAY_CR |= DISPLAY_BG0_ACTIVE;	// Turn on console layer
		}
		
		lcdSwap();
	}
}

bool getKeyboardEnable() {
	return keyboardEnable;
}

bool getIsDisplayMode8Bit() {
	return displayModeIs8Bit;
}

void addEventsToQueue() {
	OSystem_DS* system = OSystem_DS::instance();
	OSystem::Event event;

	

	
	if (system->isEventQueueEmpty()) {

/*
		if (getKeysDown() & KEY_L) {
			tweak--;
			consolePrintf("Tweak: %d\n", tweak);
			IPC->tweakChanged = true;
		}
		

		if (getKeysDown() & KEY_R) {
			tweak++;
			consolePrintf("Tweak: %d\n", tweak);
			IPC->tweakChanged = true;
		}
	*/
		if ((keysHeld() & KEY_L) && (keysHeld() & KEY_R)) {
			memoryReport();
		}

		if (displayModeIs8Bit) {

			if (!indyFightState) {

				
				if ((getKeysDown() & KEY_B) && (!(getKeysHeld() & KEY_L)) && (!(getKeysHeld() & KEY_R))) {
		//			consolePrintf("Pressing Esc");
		
					event.type = OSystem::EVENT_KEYDOWN;
					event.kbd.keycode = 27;		
					event.kbd.ascii = 27;		
					event.kbd.flags = 0;
					system->addEvent(event);
				}
		
			}
			
		
			
			if ((!getIndyFightState()) && (getKeysDown() & KEY_Y)) {
				consoleEnable = !consoleEnable;
				if (displayModeIs8Bit) {
					displayMode8Bit();
				} else {
					displayMode16Bit();
				}
			}
	
	
			if (!((getKeysHeld() & KEY_L) || (getKeysHeld() & KEY_R)) && (!getIndyFightState())) {

				if ((getKeysDown() & KEY_A) && (!indyFightState)) {
					gameScreenSwap = !gameScreenSwap;
				}
	
				if (!getPenHeld() || (mouseMode != MOUSE_HOVER)) {
					if (getKeysDown() & KEY_LEFT) {
						mouseMode = MOUSE_LEFT;
					}
					if (getKeysDown() & KEY_RIGHT) {
						if (currentGame->control != CONT_SCUMM_SAMNMAX) {
							mouseMode = MOUSE_RIGHT;
						} else {
							// If we're playing sam and max, click and release the right mouse
							// button to change verb
							OSystem::Event event;
		
							event.type = OSystem::EVENT_RBUTTONDOWN;
							event.mouse = Common::Point(getPenX(), getPenY());
							system->addEvent(event);
		
							event.type = OSystem::EVENT_RBUTTONUP;
							system->addEvent(event);
						}
					}
					if (getKeysDown() & KEY_UP) {
						mouseMode = MOUSE_HOVER;
					}
				}
	
					
				
			}

			if ((getKeysDown() & KEY_SELECT)) {
				//scaledMode = !scaledMode;
				//scY = 4;
				showOptionsDialog();
			}

			
		}
	
		if (!getIndyFightState() && !((getKeysHeld() & KEY_L) || (getKeysHeld() & KEY_R)) && (getKeysDown() & KEY_X)) {
			setKeyboardEnable(!keyboardEnable);
		}
		
		updateStatus();			
		
		OSystem::Event event;

		if ((!(getKeysHeld() & KEY_L)) && (!(getKeysHeld() & KEY_R))) {
			event.type = OSystem::EVENT_MOUSEMOVE;
			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
			//consolePrintf("x=%d   y=%d  \n", getPenX(), getPenY());
		}
	
		if (!keyboardEnable) {
			if ((mouseMode != MOUSE_HOVER) || (!displayModeIs8Bit)) {
					if (getPenDown() && (!(getKeysHeld() & KEY_L)) && (!(getKeysHeld() & KEY_R))) {	
						event.type = ((mouseMode == MOUSE_LEFT) || (!displayModeIs8Bit))? OSystem::EVENT_LBUTTONDOWN: OSystem::EVENT_RBUTTONDOWN;
						event.mouse = Common::Point(getPenX(), getPenY());
						system->addEvent(event);
					}
					
					if (getPenReleased()) {
						event.type = mouseMode == MOUSE_LEFT? OSystem::EVENT_LBUTTONUP: OSystem::EVENT_RBUTTONUP;
						event.mouse = Common::Point(getPenX(), getPenY());
						system->addEvent(event);
					}
			} else {
				// In hover mode, D-pad left and right click the mouse when the pen is on the screen
	
				if (getPenHeld()) {
					if (getKeysDown() & KEY_LEFT) {
						event.type = OSystem::EVENT_LBUTTONDOWN;
						event.mouse = Common::Point(getPenX(), getPenY());
						system->addEvent(event);
					}
				/*	if (getKeysReleased() & KEY_LEFT) {
						event.type = OSystem::EVENT_LBUTTONUP;
						event.mouse = Common::Point(getPenX(), getPenY());
						system->addEvent(event);
					}*/
	
					if (getKeysDown() & KEY_RIGHT) {
						event.type = OSystem::EVENT_RBUTTONDOWN;
						event.mouse = Common::Point(getPenX(), getPenY());
						system->addEvent(event);
					}
					/*if (getKeysReleased() & KEY_RIGHT) {
						event.type = OSystem::EVENT_RBUTTONUP;
						event.mouse = Common::Point(getPenX(), getPenY());
						system->addEvent(event);
					}*/
				}
			}
			
			if ((!(getKeysHeld() & KEY_L)) && (!(getKeysHeld() & KEY_R))  && (displayModeIs8Bit)) {
				// Controls specific to the control method
			
			
				if (currentGame->control == CONT_SKY) {
					// Extra controls for Benieth a Steel Sky
					if ((getKeysDown() & KEY_DOWN)) {
						penY = 0;
						penX = 160;		// Show inventory by moving mouse onto top line
					}
				}

				if (currentGame->control == CONT_SIMON) {
					// Extra controls for Simon the Sorcerer
					if ((getKeysDown() & KEY_DOWN)) {
						OSystem::Event event;
					
						event.type = OSystem::EVENT_KEYDOWN;
						event.kbd.keycode = '#';		// F10 or # - show hotspots
						event.kbd.ascii = '#';
						event.kbd.flags = 0;
						system->addEvent(event);
//						consolePrintf("F10\n");
					}
				}
	
				if (currentGame->control == CONT_SCUMM_ORIGINAL) {
					// Extra controls for Scumm v1-5 games
					if ((getKeysDown() & KEY_DOWN)) {
						OSystem::Event event;
					
						event.type = OSystem::EVENT_KEYDOWN;
						event.kbd.keycode = '.';		// Full stop - skips current dialogue line
						event.kbd.ascii = '.';
						event.kbd.flags = 0;
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
			
			if (leftHandedSwap(getKeysDown()) & KEY_UP) {
				event.type = OSystem::EVENT_KEYDOWN;
				event.kbd.keycode = SDLK_UP;
				event.kbd.ascii = 0;
				event.kbd.flags = 0;
				system->addEvent(event);

				event.type = OSystem::EVENT_KEYUP;
				system->addEvent(event);
			}

			if (leftHandedSwap(getKeysDown()) & KEY_DOWN) {
				event.type = OSystem::EVENT_KEYDOWN;
				event.kbd.keycode = SDLK_DOWN;
				event.kbd.ascii = 0;
				event.kbd.flags = 0;
				system->addEvent(event);

				event.type = OSystem::EVENT_KEYUP;
				system->addEvent(event);
			}

			if (leftHandedSwap(getKeysDown()) & KEY_A) {
				event.type = OSystem::EVENT_KEYDOWN;
				event.kbd.keycode = SDLK_RETURN;
				event.kbd.ascii = 0;
				event.kbd.flags = 0;
				system->addEvent(event);

				event.type = OSystem::EVENT_KEYUP;
				system->addEvent(event);
			}
		
		}

		
		if ((getKeysDown() & KEY_START)) {
			event.type = OSystem::EVENT_KEYDOWN;
			event.kbd.keycode = 319;		// F5
			event.kbd.ascii = 319;
			event.kbd.flags = 0;
			system->addEvent(event);
/*
			event.type = OSystem::EVENT_KEYUP;
			event.kbd.keycode = 319;		// F5
			event.kbd.ascii = 319;
			system->addEvent(event);*/
			
//			consolePrintf("Pressing F5");
		}

		
		if (keyboardEnable) {
			DS::addKeyboardEvents();
		}
		
		consumeKeys();
		
		consumePenEvents();

	}
}

void updateStatus() {
	int offs;

	if (displayModeIs8Bit) {
		switch (mouseMode) {
			case MOUSE_LEFT: {
				offs = 16;
				break;
			}
			case MOUSE_RIGHT: {
				offs = 32;
				break;
			}
			case MOUSE_HOVER: {
				offs = 0;
				break;
			}
			default: {
				// Nothing!
				offs = 0;
				break;
			}
		}
	
		
		sprites[0].attribute[0] = ATTR0_BMP | 150; 
		sprites[0].attribute[1] = ATTR1_SIZE_32 | 208;
		sprites[0].attribute[2] = ATTR2_ALPHA(1)| offs;
	
		if (indyFightState) {
			sprites[2].attribute[0] = ATTR0_BMP | 150; 
			sprites[2].attribute[1] = ATTR1_SIZE_32 | (190 - 32) | (indyFightRight? 0: ATTR1_FLIP_X);
			sprites[2].attribute[2] = ATTR2_ALPHA(1)| 48;
		} else {
			sprites[2].attribute[0] = ATTR0_DISABLED; 
			sprites[2].attribute[1] = 0;
			sprites[2].attribute[2] = 0;
		}
	} else {
		sprites[0].attribute[0] = ATTR0_DISABLED; 
		sprites[1].attribute[0] = ATTR0_DISABLED; 
		sprites[2].attribute[0] = ATTR0_DISABLED; 
		sprites[3].attribute[0] = ATTR0_DISABLED; 
	}

	if ((keyboardIcon) && (!keyboardEnable) && (!displayModeIs8Bit)) {
		spritesMain[0].attribute[0] = ATTR0_BMP | 160;
		spritesMain[0].attribute[1] = ATTR1_SIZE_32 | 0;
		spritesMain[0].attribute[2] = ATTR2_ALPHA(1) | 64;
	} else {
		spritesMain[0].attribute[0] = ATTR0_DISABLED;
		spritesMain[0].attribute[1] = 0;
		spritesMain[0].attribute[2] = 0;
		spritesMain[0].attribute[3] = 0;
	}

}

void soundBufferEmptyHandler() {
	REG_IF = IRQ_TIMER2;

	if (soundHiPart) {
//		bufferSecondHalf = true;
	} else {
//		bufferFirstHalf = true;
	}
	
	soundHiPart = !soundHiPart;
}

void setMainScreenScroll(int x, int y) {
	if (gameScreenSwap) {
		SUB_BG3_CX = x + (((frameCount & 1) == 0)? 64: 0);
		SUB_BG3_CY = y;
	} else {
		BG3_CX = x + (((frameCount & 1) == 0)? 64: 0);
		BG3_CY = y;
		
		touchX = x >> 8;
		touchY = y >> 8;
	}
}

void setMainScreenScale(int x, int y) {
	if (gameScreenSwap) {
		SUB_BG3_XDX = x;
		SUB_BG3_XDY = 0;
		SUB_BG3_YDX = 0;
		SUB_BG3_YDY = y;
	} else {
		BG3_XDX = x;
		BG3_XDY = 0;
		BG3_YDX = 0;
		BG3_YDY = y;
		
		touchScX = x;
		touchScY = y;
	}
}

void setZoomedScreenScroll(int x, int y) {
	if (gameScreenSwap) {
		BG3_CX = x + (((frameCount & 1) == 0)? 64: 0);
		BG3_CY = y;
		
		touchX = x >> 8;
		touchY = y >> 8;
	} else {
		SUB_BG3_CX = x + (((frameCount & 1) == 0)? 64: 0);
		SUB_BG3_CY = y;
	}
}

void setZoomedScreenScale(int x, int y) {
	if (gameScreenSwap) {
		BG3_XDX = x;
		BG3_XDY = 0;
		BG3_YDX = 0;
		BG3_YDY = y;

		touchScX = x;
		touchScY = y;
	} else {
		SUB_BG3_XDX = x;
		SUB_BG3_XDY = 0;
		SUB_BG3_YDX = 0;
		SUB_BG3_YDY = y;
	}
}

void VBlankHandler(void) {
//	BG_PALETTE[0] = RGB15(31, 31, 31);
//	if (*((int *) (0x023FFF00)) != 0xBEEFCAFE) {
	//	consolePrintf("Guard band overwritten!");
//  }

//	consolePri ntf("X:%d Y:%d\n", getPenX(), getPenY());

	static bool firstTime = true;

	// This is to ensure that the ARM7 vblank handler runs before this one.
	// Fixes the problem with the MMD when the screens swap over on load.
	if (firstTime) {
		firstTime = false;
		return;
	}


	IPC->tweak = tweak;
	soundUpdate();


	

	if ((!gameScreenSwap) && (!(getKeysHeld() & KEY_L) && !(getKeysHeld() & KEY_R))) {
		if (currentGame) {
			if (currentGame->control != CONT_SCUMM_SAMNMAX) {
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
	
//	consolePrintf("%d\n", IPC->buttons);
	


	if (callback) {
		callbackTimer -= FRAME_TIME;
	}
	
	if ((getKeysHeld() & KEY_L) || (getKeysHeld() & KEY_R)) {
	
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
			
//			consolePrintf("X:%d Y:%d\n", dragStartX - penX, dragStartY - penY);
		}
	}	
	
	
/*	if ((frameCount & 1) == 0) {
		SUB_BG3_CX = subScX;
	} else {
		SUB_BG3_CX = subScX + 64;
	}
	
	SUB_BG3_CY = subScY + (shakePos << 8);*/

	/*SUB_BG3_XDX = (int) (subScreenWidth / 256.0f * 256);
    SUB_BG3_XDY = 0;
    SUB_BG3_YDX = 0;
    SUB_BG3_YDY = (int) (subScreenHeight / 192.0f * 256);*/

	static int ratio = ( 320 << 8) / SCUMM_GAME_WIDTH;
	
	
	if ((getKeysHeld() & KEY_L) || (getKeysHeld() & KEY_R)) {
		if ((getKeysHeld() & KEY_A) && (subScreenScale < ratio)) {
			subScreenScale += 2;
		}
		
		if ((getKeysHeld() & KEY_B) && (subScreenScale > 128)) {
			subScreenScale -=2;
		}
	}


	int xCenter = subScTargetX + ((subScreenWidth >> 1) << 8);
	int yCenter = subScTargetY + ((subScreenHeight >> 1) << 8);
	
	if (twoHundredPercentFixedScale) {
		subScreenWidth = 256 >> 1;
		subScreenHeight = 192 >> 1;
	} else {
		subScreenWidth = SCUMM_GAME_WIDTH * subScreenScale >> 8;
		subScreenHeight = SCUMM_GAME_HEIGHT * subScreenScale >> 8;
	}
	
	subScTargetX = xCenter - ((subScreenWidth >> 1) << 8);
	subScTargetY = yCenter - ((subScreenHeight >> 1) << 8);
	

	

	if (subScTargetX < 0) subScTargetX = 0;
	if (subScTargetX > (gameWidth - subScreenWidth) << 8) subScTargetX = (gameWidth - subScreenWidth) << 8;

	if (subScTargetY < 0) subScTargetY = 0;
	if (subScTargetY > (gameHeight - subScreenHeight) << 8) subScTargetY = (gameHeight - subScreenHeight) << 8;



	subScX += (subScTargetX - subScX) >> 2;
	subScY += (subScTargetY - subScY) >> 2;
	
	if (displayModeIs8Bit) {
	
		if ((getKeysHeld() & KEY_L) || (getKeysHeld() & KEY_R)) {
			
			int offsX = 0, offsY = 0;


			if (getKeysHeld() & KEY_LEFT) {
				offsX -= 1;
			}
			
			if (getKeysHeld() & KEY_RIGHT) {
				offsX += 1;
			}
	
			if (getKeysHeld() & KEY_UP) {
				offsY -= 1;
			}
	
			if (getKeysHeld() & KEY_DOWN) {
				offsY += 1;
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
			
			setZoomedScreenScroll(subScX, subScY);
			setZoomedScreenScale(subScreenWidth, (subScreenHeight * 256) / 192);
	
		
			setMainScreenScroll(scX << 8, (scY << 8) + (shakePos << 8));
			setMainScreenScale(256, 256);		// 1:1 scale
	
		} else {
		
			if (scY > gameHeight - 192 - 1) {
				scY = gameHeight - 192 - 1;
			}

			if (scY < 0) {
				scY = 0;
			}
		
			setZoomedScreenScroll(subScX, subScY);
			setZoomedScreenScale(subScreenWidth, (subScreenHeight * 256) / 192);
	
			setMainScreenScroll(64, (scY << 8) + (shakePos << 8));
			setMainScreenScale(320, 256);		// 1:1 scale
			
		}
	} else {
		setZoomedScreenScroll(0, 0);
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

	//PALETTE[0] = RGB15(0, 0, 0);
	REG_IF = IRQ_VBLANK;
}

int getMillis() {
	return currentTimeMillis;
//	return frameCount * FRAME_TIME;
}

void setTimerCallback(OSystem::TimerProc proc, int interval) {
//	consolePrintf("Set timer proc %x, int %d\n", proc, interval);
	callback = proc;
	callbackInterval = interval;
	callbackTimer = interval;
}

void timerTickHandler() {
	REG_IF = IRQ_TIMER0;
	if ((callback) && (callbackTimer > 0)) {
		callbackTimer--;
	}
	currentTimeMillis++;
}

void setTalkPos(int x, int y) {
//	if (gameID != Scumm::GID_SAMNMAX) {
//		setTopScreenTarget(x, 0);
//	} else {
		setTopScreenTarget(x, y);
//	}
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
	// Guard band
//((int *) (0x023FFF00)) = 0xBEEFCAFE;


	penInit();

	powerON(POWER_ALL);
/*	vramSetBankA(VRAM_A_MAIN_BG); 
	vramSetBankB(VRAM_B_MAIN_BG); 
	vramSetBankC(VRAM_C_SUB_BG); */
	vramSetBankI(VRAM_I_SUB_SPRITE); 
	vramSetBankG(VRAM_G_MAIN_SPRITE); 
	
	currentTimeMillis = 0;


/*
	// Set up a millisecond counter
	TIMER0_CR = 0;
	TIMER0_DATA = 0xFFFF;
	TIMER0_CR = TIMER_ENABLE | TIMER_CASCADE;
*/


	for (int r = 0; r < 255; r++) {
		PALETTE[r] = 0;
	}

	PALETTE[255] = RGB15(0,31,0);


	for (int r = 0; r < 255; r++) {
		PALETTE_SUB[r] = 0;
	}

	PALETTE_SUB[255] = RGB15(0,31,0);

	// Allocate save buffer for game screen
//	savedBuffer = new u8[320 * 200];
	displayMode16Bit();
	
	memset(BG_GFX, 0, 512 * 256 * 2);	
	scaledMode = true;
	scX = 0;
	scY = 0;
	subScX = 0;
	subScY = 0;
	subScTargetX = 0;
	subScTargetY = 0;
	
	//lcdSwap();
	POWER_CR &= ~POWER_SWAP_LCDS;
	
	frameCount = 0;
	callback = NULL;
	
//	vramSetBankH(VRAM_H_SUB_BG); 
	

//	// Do text stuff
	//BG0_CR = BG_MAP_BASE(0) | BG_TILE_BASE(1);
//	BG0_Y0 = 48;
	
	PALETTE[255] = RGB15(31,31,31);//by default font will be rendered with color 255
	
	//consoleInit() is a lot more flexible but this gets you up and running quick
//	consoleInitDefault((u16*)SCREEN_BASE_BLOCK(0), (u16*)CHAR_BASE_BLOCK(1), 16);
	//consolePrintSet(0, 6);
	
	//irqs are nice
	irqInit();
//	irqInitHandler();
	irqSet(IRQ_VBLANK, VBlankHandler);
	irqSet(IRQ_TIMER0, timerTickHandler);
	irqSet(IRQ_TIMER2, soundBufferEmptyHandler);
	
	irqEnable(IRQ_VBLANK);
	irqEnable(IRQ_TIMER0);
	irqEnable(IRQ_TIMER2);
	
	
	// Set up a millisecond timer
	TIMER0_CR = 0;
	TIMER0_DATA = (u32) TIMER_FREQ(1000);
	TIMER0_CR = TIMER_ENABLE | TIMER_DIV_1 | TIMER_IRQ_REQ;	
	REG_IME = 1;

	PALETTE[255] = RGB15(0,0,31);

	initSprites();
	
//	videoSetModeSub(MODE_3_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP); //sub bg 0 will be used to print text
	
	// Convert texture from 24bit 888 to 16bit 1555, remembering to set top bit!
	u8* srcTex = (u8 *) icons_raw;
	for (int r = 32 * 160 ; r >= 0; r--) {
		SPRITE_GFX_SUB[r] = 0x8000 | (srcTex[r * 3] >> 3) | ((srcTex[r * 3 + 1] >> 3) << 5) | ((srcTex[r * 3 + 2] >> 3) << 10);
		SPRITE_GFX[r] = 0x8000 | (srcTex[r * 3] >> 3) | ((srcTex[r * 3 + 1] >> 3) << 5) | ((srcTex[r * 3 + 2] >> 3) << 10);
	}

	WAIT_CR &= ~(0x0080);
	REG_WRAM_CNT = 0;

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

//	if (getKeysHeld() & KEY_L) consolePrintf("%d, %d   penX=%d, penY=%d tz=%d\n", IPC->touchXpx, IPC->touchYpx, penX, penY, IPC->touchZ1);

	if ((penDownFrames > 1)) {			// Is this right?  Dunno, but it works for me.

		if ((penHeld)) {
			penHeld = true;
			penDown = false;

			if ((IPC->touchZ1 > 0) && (IPC->touchXpx > 0) && (IPC->touchYpx > 0)) {
				penX = IPC->touchXpx + touchXOffset;
				penY = IPC->touchYpx + touchYOffset;
			}

		} else {
			penDown = true;
			penHeld = true;
			penDownSaved = true;

			//if ( (ABS(penX - IPC->touchXpx) < 10) && (ABS(penY - IPC->touchYpx) < 10) ) {
			if ((IPC->touchZ1 > 0) && (IPC->touchXpx > 0) && (IPC->touchYpx > 0)) {
				penX = IPC->touchXpx;
				penY = IPC->touchYpx;
			}
			//}
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


	if ((IPC->touchZ1 > 0) || ((penDownFrames == 2)) ) {
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

void consumeKeys() {
	keysDownSaved = 0;
	keysReleasedSaved = 0;
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
	return x;
}

int getPenY() {
	int y = ((penY * touchScY) >> 8) + touchY;
	y = y < 0? 0: (y > gameHeight - 1? gameHeight - 1: y);
	return y;
}

GLvector getPenPos() {
	GLvector v;
	
	v.x = (penX * inttof32(1)) / SCREEN_WIDTH;
	v.y = (penY * inttof32(1)) / SCREEN_HEIGHT;
	
	return v;
}

void formatSramOption() {
	consolePrintf("The following files are present in save RAM:\n");
	DSSaveFileManager::instance()->listFiles();
	
	consolePrintf("\nAre you sure you want to\n");
	consolePrintf("DELETE all files?\n");
	consolePrintf("A = Yes, X = No\n");
	
	while (true) {
		if (keysHeld() & KEY_A) {
			DSSaveFileManager::instance()->formatSram();
			consolePrintf("SRAM cleared!\n");
			return;
		}
	
		if (keysHeld() & KEY_X) {
			consolePrintf("Whew, that was close!\n");
			return;
		}
	}
}


void setIndyFightState(bool st) {
	indyFightState = st;
	indyFightRight = true;
}

bool getIndyFightState() {
	return indyFightState;
}

/////////////////
// GBAMP
/////////////////

bool GBAMPAvail = false;

void initGBAMP(int mode) {	
	if (FAT_InitFiles()) {
		if (mode == 2)	{
			disc_IsInserted();
		}
		GBAMPAvail = true;
		consolePrintf("Found flash card reader!\n");
	} else {
		GBAMPAvail = false;
		consolePrintf("Flash card reader not found!\n");
	}
}
	 
bool isGBAMPAvailable() {
	return GBAMPAvail;
}


#ifdef USE_DEBUGGER
void initDebugger() {
	set_verbosity(VERBOSE_INFO | VERBOSE_ERROR);
	wireless_init(0);
	wireless_connect();
	
	// This is where the address of the computer running the Java
	// stub goes.
	debugger_connect_tcp(192, 168, 0, 1);
	debugger_init();	
	
	// Update function - should really call every frame
	user_debugger_update();	
}


// Ensure the function is processed with C linkage
extern "C" void debug_print_stub(char* string);

void debug_print_stub(char *string) {
	consolePrintf(string);
}
#endif

void powerOff() {
	while (keysHeld() != 0) {		// Wait for all keys to be released.
		swiWaitForVBlank();			// Allow you to read error before the power
	}								// is turned off.

	for (int r = 0; r < 60; r++) {
		swiWaitForVBlank();
	}

	if (ConfMan.hasKey("disablepoweroff", "ds") && ConfMan.getBool("disablepoweroff", "ds")) {
		while (true);
	} else {
		IPC->reset = true;				// Send message to ARM7 to turn power off
		while (true);		// Stop the program continuing beyond this point
	}
}

/////////////////
// Main
/////////////////


int main(void)
{
	soundCallback = NULL;
	

	initHardware();
	
#ifdef USE_DEBUGGER
	swiWaitForVBlank();
	if (!(keysHeld() & KEY_Y)) {
		initDebugger();
	}
#endif
	
	// Let arm9 read cartridge
	*((u16 *) (0x04000204)) &= ~0x0080;
	
	lastCallbackFrame = 0;
	tweak = 0;
	
	indyFightState = false;
	indyFightRight = true;

	// CPU speed = 67108864
	// 8 frames = 2946   368.5 bytes per fr

//	playSound(stretch, 47619, false);
//	playSound(twang, 11010, true);   // 18640

//	bufferSize = 10;

	
	/*bufferRate = 44100;
	bufferFrame = 0;
	bufferSamples = 8192;

	bufferFirstHalf = false;
	bufferSecondHalf = true;
	
	int bytes = (2 * (bufferSamples)) + 100;
	
	soundBuffer = (s16 *) malloc(bytes * 2);


	soundHiPart = true;
	
	for (int r = 0; r < bytes; r++) {
		soundBuffer[r] = 0;
	}
	

	swiWaitForVBlank();
	swiWaitForVBlank();
	playSound(soundBuffer, (bufferSamples * 2), true);
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
*/
	

	lastEventFrame = 0;
	mouseMode = MOUSE_LEFT;
	

/*
	TIMER1_CR = 0;
	TIMER1_DATA = TIMER_FREQ(bufferRate);
	TIMER1_CR = TIMER_ENABLE | TIMER_DIV_1;
	
	TIMER2_CR = 0;
	TIMER2_DATA = 0xFFFF - (bufferSamples / 2);
	TIMER2_CR = TIMER_ENABLE | TIMER_IRQ_REQ | TIMER_CASCADE;
	*/
	// 2945 - 2947
	

	
//	for (int r = 2946; r < 3000; r++) {
//		soundBuffer[r] = 30000;
//	}
	

	
	consolePrintf("---------------------------\n");
	consolePrintf("ScummVM DS\n");
	consolePrintf("Ported by Neil Millstone\n");
	consolePrintf("Version 0.9.1beta6 ");
#if defined(DS_BUILD_A)
	consolePrintf("build A\n");
	consolePrintf("Supports: Lucasarts SCUMM\n");
	consolePrintf("---------------------------\n");
#elif defined(DS_BUILD_B)
	consolePrintf("build B\n");
	consolePrintf("Supports: BASS, QUEEN\n");
	consolePrintf("---------------------------\n");
#elif defined(DS_BUILD_C)
	consolePrintf("build C\n");
	consolePrintf("---------------------------\n");
	consolePrintf("Supports: SIMON, KYRA, GOB\n");
#endif
	consolePrintf("L/R + D-pad/pen: Scroll view\n");
	consolePrintf("D-pad left:  Left mouse button\n");
	consolePrintf("D-pad right: Right mouse button\n");
	consolePrintf("D-pad up:    Hover mouse\n");
	consolePrintf("B button:    Skip cutscenes\n");
	consolePrintf("Select:		DS Options menu\n");
	consolePrintf("Start:       Game menu\n");
	consolePrintf("Y (in game): Toggle console\n");
	consolePrintf("X:           Toggle keyboard\n");
	consolePrintf("A:			Swap screens\n");
	consolePrintf("L+R (on start): Clear SRAM\n");

#if defined(DS_BUILD_A)
	consolePrintf("For a complete key list see the\n");
	consolePrintf("help screen.\n\n");
#endif

	
	// Do M3 detection selectioon
	int extraData = DSSaveFileManager::getExtraData();
	bool present = DSSaveFileManager::isExtraDataPresent();

	for (int r = 0; r < 30; r++) {
		swiWaitForVBlank();
	}

	int mode = extraData & 0x03;

	if (mode == 0) {
		if ((keysHeld() & KEY_L) && !(keysHeld() & KEY_R)) {
			mode = 1;
		} else if (!(keysHeld() & KEY_L) && (keysHeld() & KEY_R)) {
			mode = 2;
		}
	} else {
		if ((keysHeld() & KEY_L) && !(keysHeld() & KEY_R)) {
			mode = 0;
		}
	}


	if (mode == 0) {
		consolePrintf("On startup hold L if you have\n");
		consolePrintf("an M3 SD or R for an SC SD\n");
	} else if (mode == 1) {
		consolePrintf("Using M3 SD Mode.\n");
		consolePrintf("Hold L on startup to disable.\n");
	} else if (mode == 2) {
		consolePrintf("Using SC SD Mode.\n");
		consolePrintf("Hold L on startup to disable.\n");
	}

	disc_setEnable(mode);
	DSSaveFileManager::setExtraData(mode);


/*
	if ((present) && (extraData & 0x00000001)) {

		if (keysHeld() & KEY_L) {
			extraData &= ~0x00000001;
			consolePrintf("M3 SD Detection: OFF\n");
			DSSaveFileManager::setExtraData(extraData);
		} else {
			consolePrintf("M3 SD Detection: ON\n");
			consolePrintf("Hold L on startup to disable.\n");
		}

	} else if (keysHeld() & KEY_L) {
		consolePrintf("M3 SD Detection: ON\n");
		extraData |= 0x00000001;
		DSSaveFileManager::setExtraData(extraData);
	} else {
		consolePrintf("M3 SD Detection: OFF\n");
		consolePrintf("Hold L on startup to enable.\n");
	}

	disc_setM3SDEnable(extraData & 0x00000001);
*/
	// Create a file system node to force search for a zip file in GBA rom space

	DSFileSystemNode* node = new DSFileSystemNode();
	if (!node->getZip() || (!node->getZip()->isReady())) {
		// If not found, init CF/SD driver
		initGBAMP(mode);
	}
	delete node;

	

	updateStatus();
	
	
//	OSystem_DS::instance();

	g_system = new OSystem_DS();
	assert(g_system);

	if ((keysHeld() & KEY_L) && (keysHeld() & KEY_R)) {
		formatSramOption();
	}

//	printf("'%s'", Common::ConfigManager::kTransientDomain.c_str());
	//printf("'%s'", Common::ConfigManager::kApplicationDomain.c_str());

#if defined(DS_BUILD_A)
	char* argv[2] = {"/scummvmds", "--config=scummvm.ini"};
#elif defined(DS_BUILD_B)
	char* argv[2] = {"/scummvmds", "--config=scummvmb.ini"};
#elif defined(DS_BUILD_C)
	char* argv[2] = {"/scummvmds", "--config=scummvmc.ini"};
#endif


#ifdef DS_NON_SCUMM_BUILD	

	while (1) {
		scummvm_main(2, (char **) &argv);
		powerOff();
	}
#else
	while (1) {
		scummvm_main(1, (char **) &argv);
		powerOff();
	}
#endif

	return 0;
}

}

int main() {
	DS::main();
}

