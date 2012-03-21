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
#include "mortevielle/graphics.h"
#include "mortevielle/menu.h"
#include "mortevielle/mouse.h"
#include "mortevielle/saveload.h"
#include "mortevielle/sound.h"
#include "mortevielle/speech.h"

namespace Mortevielle {

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

const int kAcha = 492;
const int asoul = 154;
const int aouvr = 282;
const int achai = 387;
const int arcf = 1272;
const int arep = 1314;
const int amzon = 1650;
const int fleche = 1758;
const int arega = 0;

struct nhom {
	byte _id;     /* number between 0 and 32 */
	byte _hom[4];
};

struct t_pcga {
	byte _p;
	nhom _a[16];
};

class MortevielleEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::Stack<int> _keypresses;
	uint32 _lastGameFrame;
	bool _mouseClick;
	Common::Point _mousePos;
	bool _inMainGameLoop;	// Flag when the main game loop is active
	Common::StringArray _engineStrings;
	Common::StringArray _gameStrings;

	Pattern _patternArr[15];

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
	void cinq_huit(char &c, int &idx, byte &pt, bool &the_end);
	void copcha();
	void displayCGAPattern(int n, Pattern p, nhom *pal);
	void loadPalette();
	void loadTexts();
	void loadBRUIT5();
	void loadCFIEC();
	void loadCFIPH();
	void showTitleScreen();

public:
	Common::String _hintPctMessage;
	Common::Point _prevPos;

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
	bool _soundOff;
	bool _largestClearScreen;
	bool _hiddenHero;
	bool _heroSearching;
	bool _keyPressedEsc;
	bool _reloadCFIEC;

	bool _blo;
	bool _col;
	bool _syn;
	bool _obpart;
	bool _okdes;
	bool _anyone;
	bool _brt;

	int _textColor;
	int _currGraphicalDevice;
	int _newGraphicalDevice;
	int _place;
	int _manorDistance;
	int _currBitIndex;
	int _currDay;
	int _currHour;
	int _currHalfHour;
	int _day;
	int _hour;
	int _minute;
	float _addfix;

	Common::Point _stdPal[91][17];
	t_pcga _cgaPal[91];
	int _c_zzz;
	int ptr_word;
	byte _v_lieu[7][25];
	SaveStruct g_s, g_s1;
	byte g_is;

	ScreenSurface _screenSurface;
	PaletteManager _paletteManager;
	GfxSurface _backgroundSurface;
	Common::RandomSource _randomSource;
	SoundManager _soundManager;
	SavegameManager _savegameManager;
	SpeechManager _speechManager;
	Menu _menu;
	MouseHandler _mouse;

	bool _quitGame;			// Quit game flag. Originally called 'arret'
	bool _endGame;			// End game flag. Originally called 'solu'
	bool _loseGame;			// Lose game flag. Originally called 'perdu'
	bool _txxFileFl;        // Flag used to determine if texts are from the original files or from a DAT file

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

	bool keyPressed();
	int getChar();
	Common::Point getMousePos() const { return _mousePos; }
	void setMousePos(const Common::Point &pt);
	bool getMouseClick() const { return _mouseClick; }
	void setMouseClick(bool v) { _mouseClick = v; }
	void delay(int amount);
	Common::String getEngineString(int idx) const { return _engineStrings[idx]; }
	Common::String getGameString(int idx) const { return _gameStrings[idx]; }
	void handleOpcode();

	void endGame();
	void askRestart();
	void gameLoaded();

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
	int setPresenceDiningRoom(int hour);
	int setPresenceBureau(int hour);
	int setPresenceKitchen();
	int setPresenceLanding();
	int setPresenceChapel(int hour);
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
	void initGame();
	void resetPresenceInRooms(int roomId);
	void showPeoplePresent(int bitIndex);
	int selectCharacters(int min, int max);
	void displayAloneText();

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
	void charToHour();
	void hourToChar();

	void changeGraphicalDevice(int newDevice);
	void startDialog(int16 rep);
	void endSearch();
	int convertCharacterIndexToBitIndex(int characterIndex);
	int convertBitIndexToCharacterIndex(int bitIndex);
	void clearScreenType1();
	void clearScreenType2();
	void clearScreenType3();
	void clearScreenType10();
	int getRandomNumber(int minval, int maxval);
	void showMoveMenuAlert();
	void showConfigScreen();
	void decodeNumber(byte *pStart, int count);
	Common::String getString(int num);
	void resetVariables();
	void setPal(int n);
	void music();
	void draw(int ad, int x, int y);
	void drawRightFrame();

};

extern MortevielleEngine *g_vm;

#define CHECK_QUIT if (g_vm->shouldQuit()) { return; }
#define CHECK_QUIT0 if (g_vm->shouldQuit()) { return 0; }

} // End of namespace Mortevielle

#endif
