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

#ifndef IMMORTAL_IMMORTAL_H
#define IMMORTAL_IMMORTAL_H

#include "audio/mixer.h"

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "common/platform.h"

#include "engines/engine.h"
#include "engines/savestate.h"

#include "graphics/screen.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "immortal/detection.h"
#include "immortal/disk.h"

#include "immortal/sprite_list.h"						// This is an enum of all available sprites
#include "immortal/story.h"

namespace Immortal {

// There is a lot of bit masking that needs to happen, so this enum makes it a little easier to read
enum BitMask16 : uint16 {
	kMaskLow   = 0x00FF,
	kMaskHigh  = 0xFF00,
	kMaskLast  = 0xF000,
	kMaskFirst = 0x000F,
	kMaskHLow  = 0x0F00,
	kMaskLHigh = 0x00F0,
	kMaskNeg   = 0x8000,
	kMask12Bit = 0x0F9F									// Compression code (pos, 00, len) is stored in lower 12 bits of word
};

enum BitMask8 : uint8 {
	kMaskASCII = 0x7F,                                  // The non-extended ASCII table uses 7 bits, this makes a couple of things easier
	kMask8High = 0xF0,
	kMask8Low  = 0x0F
};

enum ColourBitMask : uint16 {
	kMaskRed   = 0x0F00,
	kMaskGreen = 0x00F0,
	kMaskBlue  = 0x000F
};

enum ChrMask : uint16 {
	kChr0  = 0x0000,
	kChrL  = 0x0001,
	kChrR  = 0xFFFF,
	kChrLD = 0x0002,
	kChrRD = 0xFFFE
};

enum Screen {											// These are constants that are used for defining screen related arrays
	kMaxRooms 	  = 16,									// Should probably put this in a different enum
	kMaxSprites   = 32,									// Number of sprites allowed at once
	kViewPortCW   = 256 / 64,
	kViewPortCH   = 128 / kMaxSprites,
	kMaxDrawItems = kViewPortCH + 1 + kMaxSprites
};

enum InputAction {
	kActionNothing,
	kActionKey,
	kActionRestart,										// Key "R" <-- Debug?
	kActionSound,
	kActionFire,
	kActionButton,										// Does this just refer to whatever is not the fire button?
	kActionDBGStep										// Debug key for moving engine forward one frame at a time
};

enum InputDirection {
	kDirectionUp,
	kDirectionLeft,
	kDirectionDown,
	kDirectionRight
};

enum CertIndex : uint8 {
	kCertHits,
	kCertLevel,
	kCertLoGameFlags,
	kCertHiGameFlags,
	kCertQuickness,
	kCertInvLo,
	kCertInvHi,
	kCertGoldLo,
	kCertGoldHi
};

enum Song {
	kSongNothing,
	kSongMaze,
	kSongCombat,
	kSongText
};

enum GameFlags : uint8 {
	kSavedNone = 0,
	kSavedKing = 1,
	kSavedAna  = 2
};

struct Frame {
	uint16  _deltaX;
	uint16  _deltaY;
	uint16  _rectX;
	uint16  _rectY;
	  byte *_bitmap;
};

struct DataSprite {
	uint16  _cenX;                                      // These are the base center positions
	uint16  _cenY;
	uint16  _numFrames;
Common::Array<Frame> _frames;
};

struct Sprite {
	   int  _frame;										// Index of _dSprite._frames[]
	uint16  _X;
	uint16  _Y;
	uint16  _on;										// 1 = active
	uint16  _priority;
DataSprite *_dSprite;
};

struct ImmortalGameDescription;

// Forward declaration because we will need the Disk class
class ProDosDisk;
class Room;

class ImmortalEngine : public Engine {
private:
	Common::RandomSource _randomSource;

protected:
	// Engine APIs
	Common::Error run() override;

public:
	ImmortalEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~ImmortalEngine() override;

	const ADGameDescription *_gameDescription;

	/* Terrible functions because C doesn't like
	 * bit manipulation enough
	 */
	uint16 xba(uint16 ab);								// This just replicates the XBA command from the 65816, because flipping the byte order is somehow not a common library function???
	uint16 rol(uint16 ab, int n);						// Rotate bits left by n
	uint16 ror(uint16 ab, int n);						// Rotate bits right by n
	uint16 mult16(uint16 a, uint16 b);					// Just avoids using (uint16) everywhere, and is slightly closer to the original

	/*
	 * --- Members ---
	 *
	 */

	/*
	 * Constants
	 */

	// Misc constants
	const int kNumLengths = 21;
	const int kNiceTime = 36;
	const int kMaxCertificate = 16;

	// This should really be a char array, but inserting frame values will be stupid so it's just a string instead
	const Common::String genStr[11] = {"New game?%", "Enter certificate:&-=", "Invalid certificate.@",
					   	   			  "End of level!&Here is your certificate:&&=", "&@",
						   			  "   Electronic Arts presents&&       The Immortal&&&&      1990 Will Harvey|]]]]]]]]]=", // Might need \ for something
						  		 	  "          written by&&         Will Harvey&         Ian Gooding&      Michael Marcantel&       Brett G. Durrett&        Douglas Fulton|]]]]]]]/=",
						   			  "#" + Common::String(kGoldBigFrame) + "$0 gold@",
						   			  "Congratulations!&&Play again?@",
						   			  "Enter certificate:&-=",
						   			  "Game Over&&Play again?@"};

	// Screen constants
	const int kResH 	  = 320;
	const int kResV 	  = 200;
	const int kScreenW__  = 128;						// ??? labeled in source as SCREENWIDTH
	const int kScreenH__  = 128;						// ???
	const int kViewPortW  = 256;
	const int kViewPortH  = 128;
	const int kScreenSize = (kResH * kResV) * 2; 		// The size of the screen buffer is 320x200
	const int kScreenLeft = 32;
	const int kScreenTop  = 20;
	const int kTextLeft   = 8;
	const int kTextTop    = 4;
	const int kGaugeX     = 0;
	const int kGaugeY     = -13;						// ???
	const int kScreenBMW  = 160;						// Literally no idea yet
	const uint16 kChrW 	  = 64;
	const uint16 kChrH    = 32;
	const uint16 kChrH2   = kChrH * 2;
	const uint16 kChrH3   = kChrH * 3;
	const int kChrLen	  = (kChrW / 2) * kChrH;
	const int kChrBMW	  = kChrW / 2;
	const int kLCutaway   = 4;

	const uint16 kChrDy[19] = {kChr0, kChrH, kChrH2, kChrH, kChrH2,
						   	   kChrH2, kChrH, kChrH2, kChrH2, kChr0,
						       kChr0, kChrH2, kChrH, kChrH2, kChrH2,
						       kChrH2, kChrH, kChrH2, kChrH2};

	const uint16 kChrMask[19] = {kChr0, kChr0,  kChr0,  kChr0,
								 kChrR, kChrL,  kChr0,  kChrL,
								 kChrR, kChr0,  kChr0,  kChrLD,
								 kChr0, kChrR,  kChrLD, kChrRD,
								 kChr0, kChrRD, kChrL};

	const uint16 kIsBackground[36] = {1, 0, 0, 0, 0, 0,
									  0, 0, 0, 1, 1, 0,
									  0, 0, 0, 0, 0, 0,
									  0, 0, 0, 0, 0, 0,
							  		  0, 0, 0, 0, 0, 0,
							  		  0, 0, 0, 0, 0, 0};

	// Disk offsets
	const int kPaletteOffset = 21205;					// This is the byte position of the palette data in the disk

	// Sprite constants
	const int kMaxSpriteAbove = 48;						// Maximum sprite extents from center
	const int kMaxSpriteBelow = 16;
	const int kMaxSpriteLeft  = 16;
	const int kMaxSpriteRight = 16;
	const int kMaxSpriteW 	  = 64;
	const int kMaxSpriteH 	  = 64;
	const int kSpriteDY		  = 32;
	const int kVSX			  = kMaxSpriteW;
	const int kVSY 			  = kSpriteDY;
	const int kVSBMW		  = (kViewPortW + kMaxSpriteW) / 2;
	const int kVSLen		  = kVSBMW * (kViewPortH + kMaxSpriteH);
	const int kVSDY			  = 32; 					// difference from top of screen to top of viewport in the virtual screen buffer
	const int kMySuperBottom  = kVSDY + kViewPortH;
	const int kSuperBottom 	  = 200;
	const int kMySuperTop	  = kVSDY;
	const int kSuperTop	  	  = 0;
	const int kViewPortSpX	  = 32;
	const int kViewPortSpY	  = 0;
	const int kWizardX 		  = 28;						// Common sprite center for some reason
	const int kWizardY 		  = 37;

	// Asset constants
	const char kGaugeOn	   	  = 1;						// On uses the sprite at index 1 of the font spriteset
	const char kGaugeOff      = 0;						// Off uses the sprite at index 0 of the font spriteset
	const char kGaugeStop     = 1;						// Literally just means the final kGaugeOn char to draw
	const char kGaugeStart    = 1;						// First kGaugeOn char to draw


	/* 
	 * 'global' members
	 */

	// Misc
	Common::ErrorCode _err;								// If this is not kNoError at any point, the engine will stop
	uint8 _certificate[16];								// The certificate (password) is basically the inventory/equipment array
	uint8 _lastCertLen = 0;
	 bool _draw 	    = 0;							// Whether the screen should draw this frame
	  int _zero 	    = 0;							// No idea what this is yet
	 bool _gameOverFlag = false;
	uint8 _gameFlags;									// Bitflag array of event flags, but only two were used (saving ana and saving the king) <-- why is gameOverFlag not in this? Lol
	 bool _themePaused;									// In the source, this is actually considered a bit flag array of 2 bits (b0 and b1). However, it only ever checks for non-zero, so it's effectively only 1 bit.
	  int _titlesShown  = 0;
	  int _time 		= 0;
	  int _promoting    = 0;							// I think promoting means the title stuff
	 bool _restart 	    = false;

	// Level members
	  int _maxLevels	= 0;							// This is determined when loading in story files
	  int _level 	    = 0;
	 bool _levelOver    = false;
	Room *_rooms[kMaxRooms];							// Rooms within the level

	// Debug members
	bool _singleStep;									// Flag for _singleStep mode

	// Input members
	int _pressedAction;
	int _heldAction;
	int _pressedDirection;
	int _heldDirection;

	// Music members
	Song _playing;										// Currently playing song
	int _themeID  = 0;									// Not sure yet tbh
	int _combatID = 0;

	// Asset members
		   int	_numSprites = 0;						// This is more accurately actually the index within the sprite array, so _numSprites + 1 is the current number of sprites
	DataSprite  _font;									// The font sprite data is loaded separate from other sprite stuff
		Sprite  _sprites[kMaxSprites];					// All the sprites shown on screen
	DataSprite  _dataSprites[kFont + 1];				// All the sprite data, indexed by SpriteFile

	// Screen members
	  byte *_window;									// Bitmap of the window around the game
	  byte *_screenBuff;								// The final buffer that will transfer to the screen
    uint16  _myCNM[(kViewPortCW + 1)][(kViewPortCH + 1)];
    uint16  _myModCNM[(kViewPortCW + 1)][(kViewPortCH + 1)];
    uint16  _myModLCNM[(kViewPortCW + 1)][(kViewPortCH + 1)];
    uint16  _columnX[kViewPortCW + 1];
    uint16  _columnTop[kViewPortCW + 1];
    uint16  _columnIndex[kViewPortCW + 1];				// Why the heck is this an entire array, when it's just an index that gets zeroed before it gets used anyway...
	uint16  _tIndex[kMaxDrawItems];
	uint16  _tPriority[kMaxDrawItems];
    uint16  _viewPortX;
    uint16  _viewPortY;
    uint16  _myViewPortX;								// Probably mirror of viewportX
    uint16  _myViewPortY;
	   int  _lastGauge = 0;								// Mirror for player health, used to update health gauge display
    uint16  _penX = 0;									// Basically where in the screen we are currently drawing
    uint16  _penY = 0;
    uint16  _myUnivPointX;
    uint16  _myUnivPointY;
	   int  _num2DrawItems = 0;
	Graphics::Surface *_mainSurface;

	// Palette members
	   int _dontResetColors = 0;						// Not sure yet
	  bool _usingNormal  	= 0;						// Whether the palette is using normal
	  bool _dim 			= 0;						// Whether the palette is dim
	uint16 _palDefault[16];
	uint16 _palWhite[16];
	uint16 _palBlack[16];
	uint16 _palDim[16];
	  byte _palRGB[48];									// Palette that ScummVM actually uses, which is an RGB conversion of the original


	/*
	 * --- Functions ---
	 *
	 */

	/*
	 * [Kernal.cpp] Functions from Kernal.gs and Driver.gs
	 */

	// Screen
	void clearScreen();									// Draws a black rectangle on the screen buffer but only inside the frame
	void whiteScreen();									// Draws a white rectanlge on the screen buffer (but does not do anything with resetColors)
	void rect(int x, int y, int w, int h);				// Draws a solid rectangle at x,y with size w,h. Also shadows for blit?
	void backspace();									// Moves draw position back and draws empty rect in place of char
	void printChr(char c);
	void loadWindow();									// Gets the window.bm file
	void drawUniv();									// Draw the background, add the sprites, determine draw order, draw the sprites
	void copyToScreen();								// If draw is 0, just check input, otherwise also copy the screen buffer to the scummvm surface and update screen
	void mungeBM();										// Put together final bitmap?
	void blit();										// Will probably want this to be it's own function
	void blit40();										// Uses macro blit 40 times
	void sBlit();
	void scroll();
	void makeMyCNM();									// ?
	void drawBGRND();									// Draw floor parts of leftmask rightmask and maskers
	void addRows();										// Add rows to drawitem array
	void addSprites();									// Add all active sprites that are in the viewport, into a list that will be sorted by priority
	void sortDrawItems();								// Sort said items
	void drawItems();									// Draw the items over the background

	// Music
	void toggleSound();									// Actually pauses the sound, doesn't just turn it off/mute
	void fixPause();
	Song getPlaying();
	void playMazeSong();
	void playCombatSong();
	void doGroan();
	void musicPause(int sID);
	void musicUnPause(int sID);
	void loadSingles(Common::String songName);			// Loads and then parse the maze song

	// Palette
	void loadPalette();									// Get the static palette data from the disk
	void setColors(uint16 pal[]);						// Applies the current palette to the ScummVM surface palette
	void fixColors();									// Determine whether the screen should be dim or normal
	void useNormal();
	void useDim();
	void useBlack();
	void useWhite();
	void pump();										// Alternates between white and black with delays in between (flashes screen)
	void fadePal(uint16 pal[], int count, uint16 target[]); // Fades the palette except the frame
	void fade(uint16 pal[], int dir, int delay);		// Calls fadePal() by a given delay each iteration
	void fadeOut(int j);								// Calls Fade with a delay of j jiffies and direction 1
	void fadeIn(int j);									// || and direction 0
	void normalFadeOut();
	void slowFadeOut();
	void normalFadeIn();

	// Assets
	Common::SeekableReadStream *loadIFF(Common::String fileName); // Loads a file and uncompresses if it is compressed
	//void loadMazeGraphics();							// Creates a universe with a maze
	void loadFont();									// Gets the font.spr file, and centers the sprite
	void clearSprites();								// Clears all sprites before drawing the current frame
	void loadSprites();									// Loads all the sprite files and centers their sprites (in spritelist, but called from kernal)
	void addSprite(uint16 x, uint16 y, SpriteName n, int frame, uint16 p);

	// Input
	void userIO();										// Get input
	void pollKeys();									// Buffer input
	void noNetwork();									// Setup input mirrors
	void waitKey();										// Waits until a key is pressed (until getInput() returns true)
	void blit8();										// This is actually just input, but it is called blit because it does a 'paddle blit' 8 times

	// These will replace the myriad of hardware input handling from the source
	bool getInput();									// True if there was input, false if not
	void addKeyBuffer();
	void clearKeyBuff();


	/*
	 * [Sprites.cpp] Functions from Sprites.GS and spriteList.GS
	 */

	// Init
	void initDataSprite(Common::SeekableReadStream *f, DataSprite *d, int index, uint16 cenX, uint16 cenY); // Initializes the data sprite
	
	// Main
	void superSprite(int s, uint16 x, uint16 y, Frame f, int bmw, byte *dst, int sT, int sB);


	/* 
	 * [Logic.cpp] Functions from Logic.GS
	 */

	// Debug
	void doSingleStep();								// Let the user advance the engine one frame at a time

	// Main
	void trapKeys();									// Poorly named, this checks if the player wants to restart/pause music/use debug step
	 int keyOrButton();									// Returns value based on whether it was a keyboard key or a button press
	void logicInit();
	void logic();										// Keeps time, handles win and lose conditions, then general logic
	void restartLogic();								// This is the actual logic init
	 int logicFreeze();									// Overcomplicated way to check if game over or level over
	void updateHitGauge();
	void drawGauge(int h);
	void makeCertificate();
	void calcCheckSum(int l, uint8 checksum[]);			// Checksum is one word, but the source called it CheckSum
	bool getCertificate();
	void printCertificate();

	// Misc
	bool printAnd(Str s);
	bool fromOldGame();
	void setGameFlags(uint16 f);
	uint16 getGameFlags();
	void setSavedKing();
	bool isSavedKing();
	void setSavedAna();
	bool isSavedAna();
	 int getLevel();									// Literally just return _level...
	void gameOverDisplay();
	void gameOver();
	void levelOver();


	/*
	 * [Misc.cpp] Functions from Misc
	 */

	// Misc
	void delay(int j);									// Delay engine by j jiffies (from driver originally, but makes more sense grouped with misc)
	void delay4(int j);									// || /4
	void delay8(int j);									// || /8
	void miscInit();
	void setRandomSeed();
	void getRandom();
	void myDelay();

	// Text printing
	bool textPrint(Str s);
	void textSub();
	void textEnd(Str s);
	void textMiddle(Str s);
	void textBeginning(Str s);
	void yesNo();

	// Input related
	void buttonPressed();
	void firePressed();

	// Screen related
	void inside(int p, int p2, int a);
	void insideRect(int p, int r);


	/*
	 * [Compression.cpp] Functions from Compression.GS
	 */

	// Main routines
	Common::SeekableReadStream *unCompress(Common::File *src, int srcLen);

	// Subroutines called by unCompress
	void   setupDictionary(uint16 start[], uint16 ptk[], uint16 &findEmpty);
	int    getInputCode(bool &carry, Common::File *src, int &srcLen, uint16 &evenOdd);
	uint16 getMember(uint16 codeW, uint16 k, uint16 &findEmpty, uint16 start[], uint16 ptk[]);
	void   appendList(uint16 codeW, uint16 k, uint16 &hash, uint16 &findEmpty, uint16 start[], uint16 ptk[], uint16 &tmp);


	/*
	 * [Music.cpp] Functions from Music.cpp
	 */

	// Misc


	/*
	 * [DrawChr.cpp] Functions from DrawChr.cpp
	 */

	// Main
	int mungeCBM(int numChrs);
	void storeAddr();
	void mungeSolid();
	void mungeLRHC();
	void mungeLLHC();
	void mungeULHC();
	void mungeURHC();
	void drawSolid(int chr, int x, int y);
	void drawULHC(int chr, int x, int y);
	void drawURHC(int chr, int x, int y);
	void drawLLHC(int chr, int x, int y);
	void drawLRHC(int chr, int x, int y);


	/*
	 * --- ScummVM general engine Functions ---
	 *
	 */

	Common::ErrorCode initDisks();						// Opens and parses IMMORTAL.dsk and IMMORTAL_GFX.dsk
	uint32 getFeatures() const;							// Returns the game description flags
	Common::String getGameId() const;					// Returns the game Id

	/* Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return
			(f == kSupportsLoadingDuringRuntime) ||
			(f == kSupportsSavingDuringRuntime) ||
			(f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently() override {
		return true;
	}
	bool canSaveGameStateCurrently() override {
		return true;
	}

	/* Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	/* Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}

	Common::Error loadGameStream(Common::SeekableReadStream *stream) {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	} */
};

extern ImmortalEngine *g_engine;
#define SHOULD_QUIT ::Immortal::g_engine->shouldQuit()

} // namespace Immortal

#endif
