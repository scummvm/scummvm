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

#ifndef IMMORTAL_H
#define IMMORTAL_H

// Audio is only handled in kernal, therefore it is only needed here
#include "audio/mixer.h"

// Immortal.h is the engine, so it needs the engine headers
#include "engines/engine.h"
#include "engines/savestate.h"

// Theorectically, all graphics should be handled through driver, which is part of kernal, which is in immortal.h
#include "graphics/screen.h"
#include "graphics/surface.h"

// Detection is only needed by the main engine
#include "immortal/detection.h"

#include "common/formats/prodos.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "common/platform.h"

// Utilities.h contains many things used by all objects, not just immortal
#include "immortal/utilities.h"

// Room also includes story.h
#include "immortal/room.h"

namespace Immortal {

// Needed by kernal for input
enum InputAction {
	kActionNothing,
	kActionKey,
	kActionRestart,                                     // Key "R" <-- Debug?
	kActionSound,
	kActionFire,
	kActionButton,                                      // Does this just refer to whatever is not the fire button?
	kActionDBGStep                                      // Debug key for moving engine forward one frame at a time
};

enum ButtonHeldMask {
	kButton0Held = 2,
	kButton1Held = 4
};

enum InputDirection {
	kDirectionUp,
	kDirectionLeft,
	kDirectionDown,
	kDirectionRight
};

// Needed by kernal for text
enum FadeType {
	kTextFadeIn,
	kTextDontFadeIn
};

// Needed by kernal for music
enum Song {
	kSongNothing,
	kSongMaze,
	kSongCombat,
	kSongText
};

enum Sound {
	kSoundSwish = 6,
	kSoundAwe,
	kSoundHuh,
	kSoundClank,
	kSoundFireBall,
	kSoundDoor
};

// Needed by logic for various things
enum MonsterID {
	kPlayerID
};

// Needed by logic for certificate processing
enum CertificateIndex : uint8 {
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

// Needed by logic for various things
enum GameFlags : uint8 {
	kSavedNone,
	kSavedKing,
	kSavedAna
};

// Needed by level (maybe?)
enum LevelType {
	kRoomType,
	kMonsterType,
	kObjectType
};

// Basically the equivalent of the explosion from a projectile in other games I think
struct Spark {
};

// Generic sprites can be used anywhere, just sort of misc sprites
struct GenericSprite {
};

// Doors are a property of the level, not the room, they define the connections between rooms
struct Door {
	uint8 _x           = 0;
	uint8 _y           = 0;
	uint8 _fromRoom    = 0;
	uint8 _toRoom      = 0;
	uint8 _busyOnRight = 0;
	uint8 _on          = 0;
};

// Universe is a set of properties for the entire level, nor just the room
struct Univ {
	uint16  _rectX     = 0;
	uint16  _rectY     = 0;
	uint16  _numAnims  = 0;
	uint16  _numCols   = 0;
	uint16  _numRows   = 0;
	uint16  _numChrs   = 0;
	uint16  _num2Cols  = 0;
	uint16  _num2Rows  = 0;
	uint16  _num2Cells = 0;
	uint16  _num2Chrs  = 0;
};

struct Chr {
	byte *_scanlines[32];
};

struct ImmortalGameDescription;

// Forward declaration because we will need the Disk and Room classes
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
	uint16 xba(uint16 ab);                              // This just replicates the XBA command from the 65816, because flipping the byte order is somehow not a common library function???
	uint16 rol(uint16 ab, int n);                       // Rotate bits left by n
	uint16 ror(uint16 ab, int n);                       // Rotate bits right by n
	uint16 mult16(uint16 a, uint16 b);                  // Just avoids using (uint16) everywhere, and is slightly closer to the original

	/*
	 * --- Members ---
	 *
	 */

	/*
	 * Constants
	 */

	// Misc constants
	const int kNumLengths     = 21;
	const int kNiceTime       = 36;
	const int kMaxCertificate = 16;

	// Screen constants
	const int    kScreenW__   = 128;                    // ??? labeled in source as SCREENWIDTH
	const int    kScreenH__   = 128;                    // ???
	const int    kViewPortW   = 256;
	const int    kViewPortH   = 128;
	const int    kScreenSize  = (kResH * kResV) * 2;     // The size of the screen buffer is (320x200) * 2 byte words
	const uint16 kScreenLeft  = 32;
	const uint16 kScreenTop   = 20;
	const uint8  kTextLeft    = 8;
	const uint8  kTextTop     = 4;
	const uint8  kGaugeX      = 0;
	const uint8  kGaugeY      = static_cast<uint8>((-13) & 0xff);                    // ???
	const uint16 kScreenBMW   = 160;                    // Screen BitMap Width?
	const uint16 kChrW        = 64;
	const uint16 kChrH        = 32;
	const uint16 kChrH2       = kChrH * 2;
	const uint16 kChrH3       = kChrH * 3;
	const uint16 kChrLen      = (kChrW / 2) * kChrH;
	const uint16 kChrBMW      = kChrW / 2;
	const uint16 kLCutaway    = 4;
	const uint16 kLDrawSolid  = 32 * ((3 * 16) + 5);

	const uint16 kChrDy[19] = {kChr0, kChrH, kChrH2, kChrH, kChrH2,
	                           kChrH2, kChrH, kChrH2, kChrH2, kChr0,
	                           kChr0, kChrH2, kChrH, kChrH2, kChrH2,
	                           kChrH2, kChrH, kChrH2, kChrH2
	                          };

	const uint16 kChrMask[19] = {kChr0, kChr0,  kChr0,  kChr0,
	                             kChrR, kChrL,  kChr0,  kChrL,
	                             kChrR, kChr0,  kChr0,  kChrLD,
	                             kChr0, kChrR,  kChrLD, kChrRD,
	                             kChr0, kChrRD, kChrL
	                            };

	const uint16 kIsBackground[36] = {1, 0, 0, 0, 0, 0,
	                                  0, 0, 0, 1, 1, 0,
	                                  0, 0, 0, 0, 0, 0,
	                                  0, 0, 0, 0, 0, 0,
	                                  0, 0, 0, 0, 0, 0,
	                                  0, 0, 0, 0, 0, 0
	                                 };

	// Disk offsets
	const int kPaletteOffset    = 21205;                // This is the byte position of the palette data in the disk

	// Sprite constants
	const uint16 kMaxSpriteW    = 64;
	const uint16 kMaxSpriteH    = 64;
	const uint16 kSpriteDY      = 32;
	const uint16 kVSX           = kMaxSpriteW;
	const uint16 kVSY           = kSpriteDY;
	const uint16 kVSBMW         = (kViewPortW + kMaxSpriteW) / 2;
	const uint16 kVSLen         = kVSBMW * (kViewPortH + kMaxSpriteH);
	const uint16 kVSDY          = 32;                   // difference from top of screen to top of viewport in the virtual screen buffer
	const uint16 kMySuperBottom = kVSDY + kViewPortH;
	const uint16 kSuperBottom   = 200;
	const uint16 kMySuperTop    = kVSDY;
	const uint16 kSuperTop      = 0;
	const uint16 kViewPortSpX   = 32;
	const uint16 kViewPortSpY   = 0;
	const uint16 kWizardX       = 28;                   // Common sprite center for some reason
	const uint16 kWizardY       = 37;
	const uint16 kObjectY       = 24;
	const uint16 kObjectX       = 32;
	const uint16 kObjectHeight  = 48;
	const uint16 kObjectWidth   = 64;

	// Text constants
	const uint8 kMaxRows        = 5;
	const uint8 kMaxCollumns    = 26;

	const uint16 kYesNoY        = 88;
	const uint16 kYesNoX1       = 8;
	const uint16 kYesNoX2       = 182;

	// Asset constants
	const char kGaugeOn         = 1;                    // On uses the sprite at index 1 of the font spriteset
	const char kGaugeOff        = 0;                    // Off uses the sprite at index 0 of the font spriteset
	const char kGaugeStop       = 1;                    // Literally just means the final kGaugeOn char to draw
	const char kGaugeStart      = 1;                    // First kGaugeOn char to draw

	// Level constants
	const int kStoryNull        = 5;
	const int kMaxFilesPerLevel = 16;
	const int kMaxPartInstances = 4;
	const int kLevelToMaze[8]   = {0, 0, 1, 1, 2, 2, 2, 3};

	/*
	 * 'global' members
	 */

	// Misc
	Common::ErrorCode _err;                             // If this is not kNoError at any point, the engine will stop
	uint8 _certificate[16];                             // The certificate (password) is basically the inventory/equipment array
	uint8 _lastCertLen = 0;
	bool _draw         = 0;                         // Whether the screen should draw this frame
	int _zero          = 0;                         // No idea what this is yet
	bool _gameOverFlag = false;
	uint8 _gameFlags   = 0;                         // Bitflag array of event flags, but only two were used (saving ana and saving the king) <-- why is gameOverFlag not in this? Lol
	bool _themePaused  = false;                     // In the source, this is actually considered a bit flag array of 2 bits (b0 and b1). However, it only ever checks for non-zero, so it's effectively only 1 bit.
	int _titlesShown   = 0;
	int _time          = 0;
	int _promoting     = 0;                         // I think promoting means the title stuff
	bool _restart      = false;

	// Story members
	Story _stories[8];

	// Level members
	int _maxLevels       = 0;                         // This is determined when loading in story files
	int _level           = 0;
	bool _levelOver      = false;
	int _count           = 0;
	int _lastLevelLoaded = 0;
	int _lastSongLoaded  = 0;
	int _storyLevel      = 0;
	int _storyX          = 0;
	int _loadA           = 0;
	int _loadY           = 0;
	uint16 _initialX     = 0;
	uint16 _initialY     = 0;
	int _initialBX       = 0;
	int _initialBY       = 0;
	int _dRoomNum        = 0;
	int _initialRoom     = 0;
	int _currentRoom     = 0;
	int _lastType        = 0;
	int _roomCellX       = 0;
	int _roomCellY       = 0;
	Room *_rooms[kMaxRooms];                            // Rooms within the level
	Common::Array<SFlame> _allFlames[kMaxRooms];        // The level needs it's own set of flames so that the flames can be turned on/off permenantly. This is technically more like a hashmap in the source, but it could also be seen as a 2d array, just hashed together in the source

	// Door members
	Common::Array<Door> _doors;
	uint8 _numDoors        = 0;
	uint8 _doorRoom        = 0;
	uint8 _doorToNextLevel = 0;
	uint8 _doorCameInFrom  = 0;
	uint8 _ladders         = 0;
	uint8 _numLadders      = 0;
	uint8 _ladderInUse     = 0;
	uint8 _secretLadder    = 0;
	uint8 _secretCount     = 0;
	uint8 _secretDelta     = 0;

	// Debug members
	bool _singleStep = false;                           // Flag for _singleStep mode

	// Input members
	int _pressedAction    = 0;
	int _heldAction       = 0;
	int _pressedDirection = 0;
	int _heldDirection    = 0;

	// Text printing members
	uint8 _slowText  = 0;
	uint8 _formatted = 0;
	uint8 _collumn   = 0;
	uint8 _row       = 0;
	uint8 _myButton  = 0;
	uint8 _lastYes   = 0;

	// Music members
	Song _playing;                                      // Currently playing song
	int _themeID  = 0;                                  // Not sure yet tbh
	int _combatID = 0;

	// Asset members
	int _numSprites = 0;                                // This is more accurately actually the index within the sprite array, so _numSprites + 1 is the current number of sprites
	DataSprite _dataSprites[kFont + 1];                 // All the sprite data, indexed by SpriteName
	Sprite _sprites[kMaxSprites];						// All the sprites shown on screen
	Cycle _cycles[kMaxCycles];
	Common::Array<Common::String> _strPtrs;             // Str should really be a char array, but inserting frame values will be stupid so it's just a string instead
	Common::Array<Motive>  _motivePtrs;
	Common::Array<Damage>  _damagePtrs;
	Common::Array<Use>     _usePtrs;
	Common::Array<Pickup>  _pickupPtrs;
	Common::Array<SCycle>  _cycPtrs;                    // This is not actually a set of pointers, but it is serving the function of what was called cycPtrs in the source
	CArray2D<Motive>       _programPtrs;
	Common::Array<ObjType> _objTypePtrs;

	// Universe members
	Univ   *_univ;                                      // Pointer to the struct that contains the universe properties
	uint16 *_logicalCNM;                                // Draw-type data for the CNM (indexes into )
	uint16 *_CNM;                                       // Stands for CHARACTER NUMBER MAP, but really it should be TILE NUMBER MAP, because it points to tiles, which are made of characters
	byte   *_oldCBM;									// Stands for CHARACTER BIT MAP, but should probably be called like, TILE CHARACTER MAP, because it is the full gfx data for all tiles
	Common::Array<Chr> _Draw;                           // In the source this contained the Linear Coded Chr Routines, but here it just contains the expanded pixel data
	uint16 *_Solid;
	uint16 *_Right;
	uint16 *_Left;
	Common::SeekableReadStream *_dataBuffer;            // This contains the uncompressed CNM + CBM

	uint16 *_modCNM;
	uint16 *_modLogicalCNM;

	uint16  _myCNM[(kViewPortCW + 1)][(kViewPortCH + 1)];
	uint16  _myModCNM[(kViewPortCW + 1)][(kViewPortCH + 1)];
	uint16  _myModLCNM[(kViewPortCW + 1)][(kViewPortCH + 1)];

	// Screen members
	byte *_screenBuff;                                  // The final buffer that will transfer to the screen
	Graphics::Surface *_mainSurface;					// The ScummVM Surface

	uint16  _columnX[kViewPortCW + 1];
	uint16  _columnTop[kViewPortCW + 1];
	uint16  _columnIndex[kViewPortCW + 1];              // Why the heck is this an entire array, when it's just an index that gets zeroed before it gets used anyway...
	uint16  _tIndex[kMaxDrawItems];
	uint16  _tPriority[kMaxDrawItems];
	uint16  _viewPortX     = 0;
	uint16  _viewPortY     = 0;
	uint16  _myViewPortX   = 0;                         // Probably mirror of viewportX
	uint16  _myViewPortY   = 0;
	int     _lastGauge     = 0;                         // Mirror for player health, used to update health gauge display
	uint16  _lastBMW       = 0;                         // Mirrors used to determine where bitmap width needs to be re-calculated
	uint16  _lastY         = 0;
	uint16  _lastPoint     = 0;
	uint16  _penX          = 0;                         // Basically where in the screen we are currently drawing
	uint16  _penY          = 0;
	uint16  _myUnivPointX  = 0;
	uint16  _myUnivPointY  = 0;
	int     _num2DrawItems = 0;
	GenericSprite _genSprites[6];

	// Palette members
	int  _dontResetColors = 0;                          // Not sure yet
	bool _usingNormal     = 0;                          // Whether the palette is using normal
	bool _dim             = 0;                          // Whether the palette is dim
	uint16 _palUniv[16];
	uint16 _palDefault[16];
	uint16 _palWhite[16];
	uint16 _palBlack[16];
	uint16 _palDim[16];
	byte   _palRGB[48];                                 // Palette that ScummVM actually uses, which is an RGB conversion of the original


	/*
	 * --- Functions ---
	 *
	 */

	/*
	 * [Kernal.cpp] Functions from Kernal.gs and Driver.gs
	 */

	// Screen
	void clearScreen();                                 // Draws a black rectangle on the screen buffer but only inside the frame
	void whiteScreen();                                 // Draws a white rectanlge on the screen buffer (but does not do anything with resetColors)
	void rect(int x, int y, int w, int h);              // Draws a solid rectangle at x,y with size w,h. Also shadows for blit?
	void backspace();                                   // Moves draw position back and draws empty rect in place of char
	void printByte(int b);
	void printChr(char c);
	void loadWindow();                                  // Gets the window.bm file
	void drawUniv();                                    // Draw the background, add the sprites, determine draw order, draw the sprites
	void copyToScreen();                                // If draw is 0, just check input, otherwise also copy the screen buffer to the scummvm surface and update screen
	void mungeBM();                                     // Put together final bitmap?
	void blit();                                        // Will probably want this to be it's own function
	void blit40();                                      // Uses macro blit 40 times
	void sBlit();
	void scroll();
	void makeMyCNM();                                   // ?
	void drawBGRND();                                   // Draw floor parts of leftmask rightmask and maskers
	void addRows();                                     // Add rows to drawitem array
	void addSprite(uint16 vpX, uint16 vpY, SpriteName s, int img, uint16 x, uint16 y, uint16 p);
	void addSprites();                                  // Add all active sprites that are in the viewport, into a list that will be sorted by priority
	void sortDrawItems();                               // Sort said items
	void drawItems();                                   // Draw the items over the background
	void drawIcon(int img);
	void setPen(uint16 penX, uint16 penY);              // Sets the 'pen' x and y positions, including making y negative if above a certain point
	void center();
	void carriageReturn();

	// Music
	void toggleSound();                                 // Actually pauses the sound, doesn't just turn it off/mute
	void fixPause();
	Song getPlaying();
	void playMazeSong();
	void playCombatSong();
	void playTextSong();
	void doGroan();
	void stopMusic();
	void musicPause(int sID);
	void musicUnPause(int sID);
	void loadSingles(Common::String songName);          // Loads and then parse the maze song
	void standardBeep();

	// Palette
	void loadPalette();                                 // Get the static palette data from the disk
	void setColors(uint16 pal[]);                       // Applies the current palette to the ScummVM surface palette
	void fixColors();                                   // Determine whether the screen should be dim or normal
	void useNormal();
	void useDim();
	void useBlack();
	void useWhite();
	void pump();                                        // Alternates between white and black with delays in between (flashes screen)
	void fadePal(uint16 pal[], int count, uint16 target[]); // Fades the palette except the frame
	void fade(uint16 pal[], int dir, int delay);        // Calls fadePal() by a given delay each iteration
	void fadeOut(int j);                                // Calls Fade with a delay of j jiffies and direction 1
	void fadeIn(int j);                                 // || and direction 0
	void normalFadeOut();
	void slowFadeOut();
	void normalFadeIn();

	// Assets
	Common::SeekableReadStream *loadIFF(Common::String fileName); // Loads a file and uncompresses if it is compressed
	void initStoryStatic();                             // Sets up all of the global static story elements
	int loadUniv(char mazeNum);                        // Unpacks the .CNM and .UNV files into all the CNM stuff, returns the total length of everything
	void loadMazeGraphics(int m);                       // Creates a universe with a maze
	void makeBlisters(int povX, int povY);              // Turns the unmodified CNM/CBM/LCNM etc into the modified ones to actually be used for drawing the game
	void loadFont();                                    // Gets the font.spr file, and centers the sprite
	void clearSprites();                                // Clears all sprites before drawing the current frame
	void loadSprites();                                 // Loads all the sprite files and centers their sprites (in spritelist, but called from kernal)

	// Input
	void userIO();                                      // Get input
	void pollKeys();                                    // Buffer input
	void noNetwork();                                   // Setup input mirrors
	void waitKey();                                     // Waits until a key is pressed (until getInput() returns true)
	void waitClick();                                   // Waits until one of the two buttons is pressed
	void blit8();                                       // This is actually just input, but it is called blit because it does a 'paddle blit' 8 times

	// These will replace the myriad of hardware input handling from the source
	bool getInput();                                    // True if there was input, false if not
	void addKeyBuffer();
	void clearKeyBuff();


	/*
	 * [DrawChr.cpp] Functions from DrawChr.cpp
	 */

	// Main
	int mungeCBM(uint16 num2Chrs);
	void storeAddr(uint16 *drawType, uint16 chr2, uint16 drawIndex);
	void mungeSolid(int oldChr, uint16 &drawIndex);
	void mungeLRHC(int oldChr, uint16 &drawIndex);
	void mungeLLHC(int oldChr, uint16 &drawIndex);
	void mungeULHC(int oldChr, uint16 &drawIndex);
	void mungeURHC(int oldChr, uint16 &drawIndex);
	void drawSolid(int chr, int x, int y);
	void drawULHC(int chr, int x, int y);
	void drawURHC(int chr, int x, int y);
	void drawLLHC(int chr, int x, int y);
	void drawLRHC(int chr, int x, int y);


	/*
	 * [Logic.cpp] Functions from Logic.GS
	 */

	// Debug
	void doSingleStep();                                // Let the user advance the engine one frame at a time

	// Main
	void trapKeys();                                    // Poorly named, this checks if the player wants to restart/pause music/use debug step
	int keyOrButton();                                 // Returns value based on whether it was a keyboard key or a button press
	void logicInit();
	void logic();                                       // Keeps time, handles win and lose conditions, then general logic
	void restartLogic();                                // This is the actual logic init
	int logicFreeze();                                 // Overcomplicated way to check if game over or level over
	void updateHitGauge();
	void drawGauge(int h);
	void makeCertificate();
	void calcCheckSum(int l, uint8 checksum[]);         // Checksum is one word, but the source called it CheckSum
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
	int getLevel();                                    // Literally just return _level...
	void gameOverDisplay();
	void gameOver();
	void levelOver();


	/*
	 * [Misc.cpp] Functions from Misc
	 */

	// Misc
	void miscInit();
	void setRandomSeed();
	void getRandom();

	// Input related
	bool buttonPressed();
	bool firePressed();

	// Text printing
	void myFadeOut();
	void myFadeIn();
	bool textPrint(Str s, int n);
	bool textBeginning(Str s, int n);
	bool textSub(Str s, FadeType f, int n);
	void textEnd(Str s, int n);
	void textMiddle(Str s, int n);

	void textCR();
	void textPageBreak(Common::String s, int &index);
	void textAutoPageBreak();
	void textDoSpace(Common::String s, int index);
	void textBounceDelay();

	bool yesNo();
	void noOn();
	void yesOn();

	void myDelay(int j);


	/*
	 * [Level.cpp] Functions from level.GS
	 * < All functions implemented (in some capacity)! >
	 */
	// Init
	void levelInitAtStartOfGameOnly();
	void levelInit();
	//void levelGetCount <-- lda count

	// Main
	void levelStory(int l);
	void levelLoadFile(int l);
	void levelNew(int l);
	void levelDrawAll();
	void levelShowRoom(int r, int bX, int bY);
	bool levelIsShowRoom(int r);
	bool levelIsLoaded(int l);
	void univAtNew(int l);
	//void getLastType <-- lda lastType
	//void setLastType <-- sta lastType
	//void getShowRoom <-- lda currentRoom

	/*
	 * [Cycle.cpp] Functions from Cyc
	 */

	// Misc
	void cycleFreeAll();                            // Delete all cycles


	/*
	 * [Story.cpp] Functions related to Story.GS
	 */

	// Init
	void initStoryDynamic();

	/*
	 * [Sprites.cpp] Functions from Sprites.GS and spriteList.GS
	 */

	// Init
	void initDataSprite(Common::SeekableReadStream *f, DataSprite *d, int index, uint16 cenX, uint16 cenY); // Initializes the data sprite

	// Main
	void superSprite(DataSprite *dSprite, uint16 x, uint16 y, int img, uint16 bmw, byte *dst, uint16 superTop, uint16 superBottom);
	bool clipSprite(uint16 &height, uint16 &pointIndex, uint16 &skipY, DataSprite *dSprite, uint16 &pointX, uint16 &pointY, int img, uint16 bmw, uint16 superTop, uint16 superBottom);
	void spriteAligned(DataSprite *dSprite, Image &img, uint16 &skipY, uint16 &pointIndex, uint16 &height, uint16 bmw, byte *dst);

	/*
	 * [Compression.cpp] Functions from Compression.GS
	 */

	// Main routines
	Common::SeekableReadStream *unCompress(Common::File *source, int lSource);

	// Subroutines called by unCompress
	void setUpDictionary(uint16 *pCodes, uint16 *pTk, uint16 &findEmpty);
	int inputCode(uint16 &outCode, int &lSource, Common::File *source, uint16 &evenOdd);
	int member(uint16 &codeW, uint16 &k, uint16 *pCodes, uint16 *pTk, uint16 &findEmpty, uint16 &index);

	/*
	 * [door.cpp] Functions from Door.GS
	 */

	void roomTransfer(int r, int x, int y);     // Transfers the player from the current room to a new room at x,y
	void doorOpenSecret();
	void doorCloseSecret();
	//void doorToNextLevel();
	void doorInit();
	void doorClrLock();
	void doorNew(SDoor door);
	void doorDrawAll();
	void doorOnDoorMat();
	//void doorEnter(); // <-- this is actually a method of Player Monster, should probably move it there later
	int findDoorTop(int x, int y);
	int findDoor(int x, int y);
	bool doLockStuff(int d, MonsterID m, int top);
	bool inDoorTop(int x, int y, MonsterID m);
	bool inDoor(int x, int y, MonsterID m);
	int doorDoStep(MonsterID m, int d, int index);
	int doorSetOn(int d);
	int doorComeOut(MonsterID m);
	void doorSetLadders(MonsterID m);


	/*
	 * [Music.cpp] Functions from music.GS and sound.GS
	 */

	// Misc


	/*
	 * --- ScummVM general engine Functions ---
	 *
	 */

	Common::ErrorCode initDisks();                      // Opens and parses IMMORTAL.dsk and IMMORTAL_GFX.dsk
	uint32 getFeatures() const;                         // Returns the game description flags
	Common::String getGameId() const;                   // Returns the game Id

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

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
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

extern ImmortalEngine *g_immortal;
#define SHOULD_QUIT ::Immortal::g_immortal->shouldQuit()

} // namespace Immortal

#endif
