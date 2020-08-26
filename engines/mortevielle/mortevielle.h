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

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#ifndef MORTEVIELLE_MORTEVIELLE_H
#define MORTEVIELLE_MORTEVIELLE_H

#include "common/events.h"
#include "common/file.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/stack.h"
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
#include "mortevielle/outtext.h"
#include "mortevielle/detection.h"

namespace Mortevielle {

// Debug channels
enum {
	kMortevielleCore = 1 << 0,
	kMortevielleGraphics = 1 << 1,
	kMortevielleSounds = 1 << 2
};

// Game languages
enum {
	MORTDAT_LANG_FRENCH = 0,
	MORTDAT_LANG_ENGLISH = 1,
	MORTDAT_LANG_GERMAN = 2
};

// Static string list
enum {
	S_YES_NO = 0, S_GO_TO = 1, S_SOMEONE_ENTERS = 2, S_COOL = 3, S_LOURDE = 4,
	S_MALSAINE = 5, S_IDEM = 6,	S_YOU = 7, S_ARE = 8, S_ALONE = 9,
	S_HEAR_NOISE = 10, S_SHOULD_HAVE_NOTICED = 11, S_NUMBER_OF_HINTS = 12,
	S_WANT_TO_WAKE_UP = 13, S_OKAY = 14, S_SAVE_LOAD = 15, S_RESTART = 18, S_F3 = 19,
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
#define DISK_ACCESS_DELAY 1000

const int kTime1 = 410;
const int kTime2 = 250;

const int kAcha = 492;
const int kFleche = 1758;

const int kAsoul = 154;
const int kAouvr = 282;
const int kAsearch = 387;
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
const int kStartingScreenStringIndex = 456;
const int kMenuActionStringIndex = 476;
const int kMenuSelfStringIndex = 497;
const int kMenuSayStringIndex = 502;
const int kMaxPatt = 20;

const int kResolutionScaler = 2;
/*
18  "It's already open$",
26  "A photograph$"
*/
enum Places {
	OWN_ROOM = 0,     GREEN_ROOM = 1,   PURPLE_ROOM = 2,     TOILETS = 3,      DARKBLUE_ROOM = 4,
	BLUE_ROOM = 5,    RED_ROOM = 6,     BATHROOM = 7,        GREEN_ROOM2 = 8,  JULIA_ROOM = 9,
	DINING_ROOM = 10, BUREAU = 11,      KITCHEN = 12,        ATTIC = 13,       CELLAR = 14,
	LANDING = 15,     CRYPT = 16,       SECRET_PASSAGE = 17, ROOM18 = 18,      MOUNTAIN = 19,
	CHAPEL = 20,      MANOR_FRONT = 21, MANOR_BACK = 22,     INSIDE_WELL = 23, WELL = 24,
	DOOR = 25,        ROOM26 = 26,      COAT_ARMS = 27
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
	const MortevielleGameDescription *_gameDescription;
	Common::Stack<int> _keypresses;
	uint32 _lastGameFrame;
	Common::Point _mousePos;
	Common::StringArray _engineStrings;
	Common::StringArray _gameStrings;

	int _menuOpcode;

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
	bool _juliaRoomPresenceLeo;
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
	int  _curSearchObjId;
	int  _controlMenu;
	int  _startTime;
	int  _endTime;
	Common::Point _stdPal[91][17];

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
	int  _currentTime;
	int  _pauseStartTime;

	Common::String _hintPctMessage;
	byte  *_cfiecBuffer;
	int    _cfiecBufferSize;
	int    _openObjects[7];
	uint16 _dialogIndexArray[kMaxDialogIndex + 1];
	Hint   _dialogHintArray[kMaxDialogHint + 1];

	Common::ErrorCode initialize();
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
	void loadPalette();
	void loadTexts();
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
	int  getPresenceStats(int &rand, int faithScore, int roomId);
	void setPresenceFlags(int roomId);
	void testKey(bool d);
	void exitRoom();
	void getReadDescription(int objId);
	void getSearchDescription(int objId);
	int  checkLeaveSecretPassage();
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
	void setRandomPresenceGreenRoom(int faithScore);
	void setRandomPresencePurpleRoom(int faithScore);
	void setRandomPresenceBlueRoom(int faithScore);
	void setRandomPresenceRedRoom(int faithScore);
	void setRandomPresenceJuliaRoom(int faithScore);
	void setRandomPresenceDiningRoom(int faithScore);
	void setRandomPresenceBureau(int faithScore);
	void setRandomPresenceKitchen(int faithScore);
	void setRandomPresenceAttic(int faithScore);
	void setRandomPresenceLanding(int faithScore);
	void setRandomPresenceChapel(int faithScore);
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
	void prepareDisplayText();
	bool decryptNextChar(char &c, int &idx, byte &pt);
	void displayStatusArrow();
	void displayStatusInDescriptionBar(char stat);
	void displayQuestionText(Common::String s, int cmd);
	void displayTextInDescriptionBar(int x, int y, int nb, int mesgId);
	void displayTextInVerbBar(Common::String text);
	void displayTextBlock(Common::String text);
	void mapMessageId(int &mesgId);
	void resetOpenObjects();
	void setCoordinates(int sx);
	void drawPicture();
	void drawPictureWithText();
	void addObjectToInventory(int objectId);
	void putInHand(int &objId);
	void initMaxAnswer();
	void displayAnimFrame(int frameNum, int animId);
	int  getFirstObject();
	void prepareNextObject();
	void putObject();
	void resetObjectPlace();
	void resetCoreVar();
	void drawDiscussionBox();
	void displayNarrativePicture(int af, int ob);
	void menuUp();
	void displayLookScreen(int objId);

	void adzon();

public:
	Common::Point _prevPos;
	int  _currMenu;
	int  _currAction;
	int  _drawingSizeArr[108];
	int  _charAnswerCount[9];
	int  _charAnswerMax[9];
	byte _tabdon[4001];
	bool _soundOff;
	bool _outsideOnlyFl;
	bool _destinationOk;
	bool _largestClearScreen;
	float _addFix;
	int  _savedBitIndex;
	int  _numpal;
	int  _key;
	bool _mouseClick;
	SaveStruct _coreVar, _saveStruct;

	int _maff;
	int _caff;
	int _crep;
	int _is;		// ???

	byte _destinationArray[7][25];

	byte *_curPict;
	byte *_curAnim;
	byte *_rightFramePict;

	PaletteManager _paletteManager;
	GfxSurface _backgroundSurface;
	Common::RandomSource _randomSource;

	ScreenSurface *_screenSurface;
	SoundManager *_soundManager;
	SavegameManager *_savegameManager;
	Menu *_menu;
	MouseHandler *_mouse;
	TextHandler *_text;
	DialogManager *_dialogManager;

	MortevielleEngine(OSystem *system, const MortevielleGameDescription *gameDesc);
	~MortevielleEngine() override;
	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	Common::Error run() override;
	void pauseEngineIntern(bool pause) override;
	uint32 getGameFlags() const;
	Common::Language getLanguage() const;
	Common::Language getOriginalLanguage() const;
	bool useOriginalData() const;
	static Common::String generateSaveFilename(const Common::String &target, int slot);
	Common::String getSaveStateName(int slot) const override {
		return generateSaveFilename(_targetName, slot);
	}

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
	void displayInterScreenMessage(int mesgId);
	void draw(int x, int y);
	void charToHour();
	void hourToChar();
	Common::String getString(int num);
	void setPal(int n);
	Common::String copy(const Common::String &s, int idx, size_t size);
	void testKeyboard();
	int  getPresence(int roomId);
	void displayEmptyHand();
	void displayPicture(const byte *pic, int x, int y);

	int  gettKeyPressed();
	void handleDescriptionText(int f, int mesgId);
	int  getAnimOffset(int frameNum, int animNum);

	void clearScreen();
};

extern MortevielleEngine *g_vm;

} // End of namespace Mortevielle

#endif
