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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#ifndef MORTEVIELLE_H
#define MORTEVIELLE_H

#include "common/events.h"
#include "common/file.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/stack.h"
#include "engines/advancedDetector.h"
#include "engines/engine.h"
#include "common/error.h"
#include "graphics/surface.h"
#include "mortevielle/debugger.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/graphics.h"
#include "mortevielle/menu.h"
#include "mortevielle/mouse.h"
#include "mortevielle/saveload.h"
#include "mortevielle/sound.h"
#include "mortevielle/speech.h"
#include "mortevielle/outtext.h"

namespace Mortevielle {

/*---------------------------------------------------------------------------*/
/*-------------------           MEMORY  MAP          ------------------------*/
/*---------------------------------------------------------------------------*/
/* The following is a list of physical addresses in memory currently used
 * by the game.
 *
 * Address
 * -------
 * 5000:0 - Music data
 * 6000:0 - Decompressed current image
 * 7000:0+ - Compressed images
 * 7000:2 - 16 words representing palette map
 * 7000:4138 - width, height, x/y offset of decoded image
 */

#define ord(v) ((int) v)
#define chr(v) ((unsigned char) v)
#define lo(v) ((v) & 0xff)
#define hi(v) (((v) >> 8) & 0xff)
#define swap(v) (((lo(v)) << 8) | ((hi(v)) >> 8))
#define odd(v) (((v) % 2) == 1)

// Debug channels
enum {
	kMortevielleCore = 1 << 0,
	kMortevielleGraphics = 1 << 1
};

// Game languages
enum {
	LANG_FRENCH = 0,
	LANG_ENGLISH = 1,
	LANG_GERMAN = 2
};

// Static string list
enum {
	S_YES_NO = 0, S_GO_TO = 1, S_SOMEONE_ENTERS = 2, S_COOL = 3, S_LOURDE = 4,
	S_MALSAINE = 5, S_IDEM = 6,	S_YOU = 7, S_ARE = 8, S_ALONE = 9,
	S_HEAR_NOISE = 10, S_SHOULD_HAVE_NOTICED = 11, S_NUMBER_OF_HINTS = 12,
	S_WANT_TO_WAKE_UP = 13, S_OK = 14, S_SAVE_LOAD = 15, S_RESTART = 18, S_F3 = 19,
	S_F8 = 20, S_HIDE_SELF = 21, S_TAKE = 22, S_PROBE = 23, S_RAISE = 24, S_SUITE = 25,
	S_STOP = 26, S_USE_DEP_MENU = 27, S_LIFT = 28, S_READ = 29,
	S_LOOK = 30, S_SEARCH = 31, S_OPEN = 32, S_PUT = 33, S_TURN = 34, S_TIE = 35, S_CLOSE = 36,
	S_HIT = 37, S_POSE = 38, S_SMASH = 39,

	S_SMELL = 40, S_SCRATCH = 41, S_PROBE2 = 42, S_BEFORE_USE_DEP_MENU = 43, S_DAY = 44
};

enum DataType {
	kStaticStrings = 0,
	kGameStrings = 1
};

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 400
#define SCREEN_ORIG_HEIGHT 200
#define MORT_DAT_REQUIRED_VERSION 1
#define MORT_DAT "mort.dat"
#define GAME_FRAME_DELAY (1000 / 50)

const int kTime1 = 410;
const int kTime2 = 250;

const int kAcha = 492;
const int kAdrDes = 0x7000;
const int kFleche = 1758;

const int kAsoul = 154;
const int kAouvr = 282;
const int kAchai = 387;
const int kArcf = 1272;
const int kArep = 1314;
const int kAmzon = 1650;
const int kArega = 0;

const int kMaxDialogIndex = 9000;
const int kMaxDialogHint = 600;

const int kDescriptionStringIndex = 0;                // Unused
const int kInventoryStringIndex = 186;
const int kQuestionStringIndex = 247;
const int kDialogStringIndex = 292;
const int kMenuPlaceStringIndex = 435;
const int kMenuActionStringIndex = 476;
const int kMenuSelfStringIndex = 497;
const int kMenuSayStringIndex = 502;
const int kMaxPatt = 20;

const int OPCODE_NONE = 0;
enum verbs {OPCODE_ATTACH = 0x301, OPCODE_WAIT = 0x302,  OPCODE_FORCE = 0x303,   OPCODE_SLEEP = 0x304, OPCODE_LISTEN = 0x305,
            OPCODE_ENTER = 0x306,  OPCODE_CLOSE = 0x307, OPCODE_SEARCH = 0x308,  OPCODE_KNOCK = 0x309, OPCODE_SCRATCH = 0x30a,
			OPCODE_READ = 0x30b,   OPCODE_EAT = 0x30c,   OPCODE_PLACE = 0x30d,   OPCODE_OPEN = 0x30e,  OPCODE_TAKE = 0x30f,
			OPCODE_LOOK = 0x310,   OPCODE_SMELL = 0x311, OPCODE_SOUND = 0x312,   OPCODE_LEAVE = 0x313, OPCODE_LIFT = 0x314,
			OPCODE_TURN = 0x315,   OPCODE_SHIDE = 0x401, OPCODE_SSEARCH = 0x402, OPCODE_SREAD = 0x403, OPCODE_SPUT = 0x404,
			OPCODE_SLOOK = 0x405};

static const int _actionMenu[12] = { OPCODE_NONE,
		OPCODE_SHIDE, OPCODE_ATTACH, OPCODE_FORCE, OPCODE_SLEEP,
		OPCODE_ENTER, OPCODE_CLOSE,  OPCODE_KNOCK, OPCODE_EAT,
		OPCODE_PLACE, OPCODE_OPEN,   OPCODE_LEAVE
};

/*
9   "A glance at the forbidden$",
18  "It's already open$",
26  "A photograph$"
*/
enum Places {
	OWN_ROOM = 0,     GREEN_ROOM = 1,   PURPLE_ROOM = 2,     TOILETS = 3,      DARKBLUE_ROOM = 4,
	BLUE_ROOM = 5,    RED_ROOM = 6,     BATHROOM = 7,        GREEN_ROOM2 = 8,  ROOM9 = 9,
	DINING_ROOM = 10, BUREAU = 11,      KITCHEN = 12,        ATTIC = 13,       CELLAR = 14,
	LANDING = 15,     CRYPT = 16,       SECRET_PASSAGE = 17, ROOM18 = 18,      MOUNTAIN = 19,
	CHAPEL = 20,      MANOR_FRONT = 21, MANOR_BACK = 22,     INSIDE_WELL = 23, WELL = 24,
	DOOR = 25,        ROOM26 = 26,      COAT_ARMS = 27
};

enum GraphicModes { MODE_AMSTRAD1512 = 0, MODE_CGA = 1, MODE_EGA = 2, MODE_HERCULES = 3, MODE_TANDY = 4 };

struct nhom {
	byte _id;     /* number between 0 and 32 */
	byte _hom[4];
};

struct CgaPalette {
	byte _p;
	nhom _a[16];
};

struct Pattern {
	byte _tay, _tax;
	byte _des[kMaxPatt + 1][kMaxPatt + 1];
};

struct SaveStruct {
	int _faithScore;
	byte _pctHintFound[11];
	byte _availableQuestion[43];
	byte _inventory[31];
	int _currPlace;
	int _atticBallHoleObjectId;
	int _atticRodHoleObjectId;
	int _cellarObjectId;
	int _secretPassageObjectId;
	int _wellObjectId;
	int _selectedObjectId;
	int _purpleRoomObjectId;
	int _cryptObjectId;
	bool _alreadyEnteredManor;
	byte _fullHour;
};

struct Hint {
	int _hintId;
	byte _point;
};

class MortevielleEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::Stack<int> _keypresses;
	uint32 _lastGameFrame;
	Common::Point _mousePos;
	Common::StringArray _engineStrings;
	Common::StringArray _gameStrings;

	Pattern _patternArr[15];
	int _menuOpcode;

	bool _mouseClick;
	bool _inMainGameLoop;	// Flag when the main game loop is active
	bool _quitGame;			// Quit game flag. Originally called 'arret'
	bool _endGame;			// End game flag. Originally called 'solu'
	bool _loseGame;			// Lose game flag. Originally called 'perdu'
	bool _txxFileFl;        // Flag used to determine if texts are from the original files or from a DAT file
	bool _roomPresenceLuc;
	bool _roomPresenceIda;
	bool _purpleRoomPresenceLeo;
	bool _roomPresenceGuy;
	bool _roomPresenceEva;
	bool _roomPresenceMax;
	bool _roomPresenceBob;
	bool _roomPresencePat;
	bool _toiletsPresenceBobMax;
	bool _bathRoomPresenceBobMax;
	bool _room9PresenceLeo;
	bool _hiddenHero;
	bool _heroSearching;
	bool _keyPressedEsc;
	bool _reloadCFIEC;
	bool _col;
	bool _syn;
	bool _obpart;
	bool _anyone;
	bool _uptodatePresence;

	int  _textColor;
	int  _place;
	int  _manorDistance;
	int  _currBitIndex;
	int  _currDay;
	int  _currHour;
	int  _currHalfHour;
	int  _day;
	int  _hour;
	int  _minute;
	int  _mchai;
	int  _controlMenu;
	int  _startHour;
	int  _endHour;
	Common::Point _stdPal[91][17];
	CgaPalette _cgaPal[91];

	int  _x26KeyCount;
	int  _roomDoorId;
	int  _openObjCount;
	int  _takeObjCount;
	int  _num;
	int  _searchCount;
	bool _introSpeechPlayed;
	int  _inGameHourDuration;
	int  _x;
	int  _y;
	int  _currentHourCount;
	int  _currentDayHour;

	Common::String _hintPctMessage;
	byte  *_cfiecBuffer;
	int    _cfiecBufferSize;
	int    _openObjects[8];
	int    _nbrep[9];
	int    _nbrepm[9];
	uint16 _dialogIndexArray[kMaxDialogIndex + 1];
	Hint   _dialogHintArray[kMaxDialogHint + 1];

	Common::ErrorCode initialise();
	Common::ErrorCode loadMortDat();
	void readStaticStrings(Common::File &f, int dataSize, DataType dataType);
	void loadFont(Common::File &f);
	bool handleEvents();
	void addKeypress(Common::Event &evt);
	void initMouse();
	void showIntroduction();
	void mainGame();
	void playGame();
	void handleAction();
	void displayCGAPattern(int n, Pattern p, nhom *pal);
	void loadPalette();
	void loadTexts();
	void loadBRUIT5();
	void loadCFIEC();
	void loadCFIPH();
	void showTitleScreen();
	int  readclock();
	void palette(int v1);
	int checkLeoMaxRandomPresence();
	void interactNPC();
	void initCaveOrCellar();
	void displayControlMenu();
	void displayItemInHand(int objId);
	void resetRoomVariables(int roomId);
	int  getPresenceStats(int &rand, int cf, int roomId);
	void setPresenceFlags(int roomId);
	void testKey(bool d);
	void exitRoom();
	void getReadDescription(int objId);
	void getSearchDescription(int objId);
	int  checkLeaveSecretPassage();
	void changeGraphicalDevice(int newDevice);
	void startDialog(int16 rep);
	void endSearch();
	int convertCharacterIndexToBitIndex(int characterIndex);
	int convertBitIndexToCharacterIndex(int bitIndex);
	void clearUpperLeftPart();
	void clearDescriptionBar();
	void clearVerbBar();
	void clearUpperRightPart();
	int  getRandomNumber(int minval, int maxval);
	void showMoveMenuAlert();
	void showConfigScreen();
	void decodeNumber(byte *pStart, int count);
	void resetVariables();
	void music();
	void drawRightFrame();
	void prepareRoom();
	void drawClock();
	void checkManorDistance();
	void gotoManorFront();
	void gotoManorBack();
	void gotoDiningRoom();
	bool checkInventory(int objectId);
	void loseGame();
	void floodedInWell();
	void displayDiningRoom();
	void startMusicOrSpeech(int so);
	void setTextColor(int col);
	void prepareScreenType1();
	void prepareScreenType2();
	void prepareScreenType3();
	void updateHour(int &day, int &hour, int &minute);
	void getKnockAnswer();
	int  getPresenceStatsGreenRoom();
	int  getPresenceStatsPurpleRoom();
	int  getPresenceStatsToilets();
	int  getPresenceStatsBlueRoom();
	int  getPresenceStatsRedRoom();
	int  getPresenceStatsDiningRoom(int &hour);
	int  getPresenceStatsBureau(int &hour);
	int  getPresenceStatsKitchen();
	int  getPresenceStatsAttic();
	int  getPresenceStatsLanding();
	int  getPresenceStatsChapel(int &hour);
	int  getPresenceBitIndex(int roomId);
	void setPresenceGreenRoom(int roomId);
	void setPresencePurpleRoom();
	void setPresenceBlueRoom();
	void setPresenceRedRoom(int roomId);
	int  setPresenceDiningRoom(int hour);
	int  setPresenceBureau(int hour);
	int  setPresenceKitchen();
	int  setPresenceLanding();
	int  setPresenceChapel(int hour);
	void setRandomPresenceGreenRoom(int cf);
	void setRandomPresencePurpleRoom(int cf);
	void setRandomPresenceBlueRoom(int cf);
	void setRandomPresenceRedRoom(int cf);
	void setRandomPresenceRoom9(int cf);
	void setRandomPresenceDiningRoom(int cf);
	void setRandomPresenceBureau(int cf);
	void setRandomPresenceKitchen(int cf);
	void setRandomPresenceAttic(int cf);
	void setRandomPresenceLanding(int cf);
	void setRandomPresenceChapel(int cf);
	void loadPlaces();
	void resetPresenceInRooms(int roomId);
	void showPeoplePresent(int bitIndex);
	int  selectCharacters(int min, int max);
	void fctMove();
	void fctTake();
	void fctInventoryTake();
	void fctLift();
	void fctRead();
	void fctSelfRead();
	void fctLook();
	void fctSelftLook();
	void fctSearch();
	void fctSelfSearch();
	void fctOpen();
	void fctPlace();
	void fctTurn();
	void fctSelfHide();
	void fctAttach();
	void fctClose();
	void fctKnock();
	void fctSelfPut();
	void fctListen();
	void fctEat();
	void fctEnter();
	void fctSleep();
	void fctForce();
	void fctLeave();
	void fctWait();
	void fctSound();
	void fctDiscuss();
	void fctSmell();
	void fctScratch();
	void endGame();
	void askRestart();
	void handleOpcode();
	void displayStatusArrow();
	void displayStatusInDescriptionBar(char stat);

	void displayTextInDescriptionBar(int x, int y, int nb, int mesgId);
	void mapMessageId(int &mesgId);
	void resetOpenObjects();
	void setCoordinates(int sx);
	void displayTextInVerbBar(Common::String text);
	void drawPicture();
	void drawPictureWithText();
	void addObjectToInventory(int objectId);
	void putInHand(int &objId);
	void prepareDisplayText();

	void cinq_huit(char &c, int &idx, byte &pt, bool &the_end);
	void copcha();
	void adzon();
	void phaz(int &rand, int &p, int cf);
	void writetp(Common::String s, int t);
	void premtet();
	void ajchai();
	void ecr2(Common::String text);
	void init_nbrepm();
	void aniof(int ouf, int num);
	void tlu(int af, int ob);
	void mennor();
	void tsuiv();
	void treg(int objId);
	int  rechai();

public:
	Common::Point _prevPos;
	int  _msg[5];
	int  _fxxBuffer[108];
	byte _tabdon[4001];
	bool _soundOff;
	bool _blo;
	bool _destinationOk;
	bool _largestClearScreen;
	int  _currGraphicalDevice;
	int  _newGraphicalDevice;
	float _addFix;
	int  _savedBitIndex;
	int  _numpal;
	int  _key;
	SaveStruct _coreVar, _saveStruct;

	int _maff;
	int _caff;
	int _crep;

	int _resolutionScaler;
	byte _destinationArray[7][25];

	// TODO: Replace the following with proper implementations, or refactor out the code using them
	byte _mem[65536 * 16];

	Debugger _debugger;
	ScreenSurface _screenSurface;
	PaletteManager _paletteManager;
	GfxSurface _backgroundSurface;
	Common::RandomSource _randomSource;
	SoundManager _soundManager;
	SavegameManager _savegameManager;
	SpeechManager _speechManager;
	Menu _menu;
	MouseHandler _mouse;
	TextHandler _text;
	DialogManager _dialogManager;

	MortevielleEngine(OSystem *system, const ADGameDescription *gameDesc);
	~MortevielleEngine();
	virtual bool hasFeature(EngineFeature f) const;
	virtual bool canLoadGameStateCurrently();
	virtual bool canSaveGameStateCurrently();
	virtual Common::Error loadGameState(int slot);
	virtual Common::Error saveGameState(int slot, const Common::String &desc);
	virtual Common::Error run();
	uint32 getGameFlags() const;
	Common::Language getLanguage() const;
	static Common::String generateSaveFilename(const Common::String &target, int slot);
	Common::String generateSaveFilename(int slot) { return generateSaveFilename(_targetName, slot); }

	int getChar();
	bool keyPressed();
	Common::Point getMousePos() const { return _mousePos; }
	void setMousePos(const Common::Point &pt);
	bool getMouseClick() const { return _mouseClick; }
	void setMouseClick(bool v) { _mouseClick = v; }
	Common::String getEngineString(int idx) const { return _engineStrings[idx]; }
	Common::String getGameString(int idx) const { return _gameStrings[idx]; }

	void delay(int amount);
	void gameLoaded();
	void initGame();
	void displayAloneText();
	void draw(int ad, int x, int y);
	void charToHour();
	void hourToChar();
	Common::String getString(int num);
	void setPal(int n);
	Common::String copy(const Common::String &s, int idx, size_t size);
	void testKeyboard();
	int  getPresence(int roomId);
	void displayEmptyHand();

	void hirs();
	int  gettKeyPressed();
	void handleDescriptionText(int f, int mesgId);
	int  animof(int ouf, int num);
	void pictout(int seg, int dep, int x, int y);
};

extern MortevielleEngine *g_vm;

} // End of namespace Mortevielle

#endif
