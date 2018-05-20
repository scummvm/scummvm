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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARTREK_H
#define STARTREK_H

#include "common/events.h"
#include "common/list.h"
#include "common/ptr.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/util.h"

#include "engines/engine.h"

#include "startrek/awaymission.h"
#include "startrek/filestream.h"
#include "startrek/graphics.h"
#include "startrek/items.h"
#include "startrek/object.h"
#include "startrek/room.h"
#include "startrek/sound.h"


using Common::SharedPtr;
using Common::String;

namespace Common {
	class MacResManager;
}

namespace StarTrek {

class StarTrekEngine;

typedef String (StarTrekEngine::*TextGetterFunc)(int, uintptr, String *);



const int MAX_MENUBUTTONS = 32;
const int TEXTBOX_WIDTH = 26;
const int MAX_TEXTBOX_LINES = 12;


enum StarTrekGameType {
	GType_ST25 = 1,
	GType_STJR = 2
};

enum StarTrekGameFeatures {
	GF_DEMO =    (1 << 0)
};

enum kDebugLevels {
	kDebugSound =     1 << 0,
	kDebugGraphics =  1 << 1
};

enum GameMode {
	GAMEMODE_START = 0,
	GAMEMODE_BRIDGE,
	GAMEMODE_AWAYMISSION,
	GAMEMODE_BEAMDOWN,
	GAMEMODE_BEAMUP
};

enum TextDisplayMode {
	TEXTDISPLAY_WAIT = 0,  // Wait for input before closing text
	TEXTDISPLAY_SUBTITLES, // Automatically continue when speech is done
	TEXTDISPLAY_NONE       // No text displayed
};

enum TextColor {
	TEXTCOLOR_YELLOW = 0xb0
};

// Keeps track of data for a list of buttons making up a menu
struct Menu {
	Sprite sprites[MAX_MENUBUTTONS];
	uint16 retvals[MAX_MENUBUTTONS];
	uint32 disabledButtons;
	SharedPtr<FileStream> menuFile;
	uint16 numButtons;
	int16 selectedButton;
	SharedPtr<Menu> nextMenu;
};

// Special events that can be returned by handleMenuEvents.
// (Normally it returns the "retval" of a pressed button, which is positive.)
enum MenuEvent {
	MENUEVENT_RCLICK_OFFBUTTON = -4,
	MENUEVENT_ENABLEINPUT,          // Makes buttons selectable (occurs after a delay)
	MENUEVENT_RCLICK_ONBUTTON,
	MENUEVENT_LCLICK_OFFBUTTON
};

// Buttons for standard text display
enum TextButtons {
	TEXTBUTTON_CONFIRM = 0,
	TEXTBUTTON_SCROLLUP,
	TEXTBUTTON_SCROLLDOWN,
	TEXTBUTTON_PREVCHOICE,
	TEXTBUTTON_NEXTCHOICE,
	TEXTBUTTON_SCROLLUP_ONELINE,
	TEXTBUTTON_SCROLLDOWN_ONELINE,
	TEXTBUTTON_GOTO_TOP,
	TEXTBUTTON_GOTO_BOTTOM,
	TEXTBUTTON_SPEECH_DONE // "Virtual" button?
};

// Buttons for option menu (corresponding to button indices, not button retvals, which are
// different for some reason)
enum OptionMenuButtons {
	OPTIONBUTTON_TEXT,
	OPTIONBUTTON_SAVE,
	OPTIONBUTTON_LOAD,
	OPTIONBUTTON_ENABLEMUSIC,
	OPTIONBUTTON_DISABLEMUSIC,
	OPTIONBUTTON_ENABLESFX,
	OPTIONBUTTON_DISABLESFX,
	OPTIONBUTTON_QUIT
};

enum TrekEventType {
	TREKEVENT_TICK = 0, // DOS clock changes
	TREKEVENT_LBUTTONDOWN = 1,
	TREKEVENT_MOUSEMOVE = 2,
	TREKEVENT_LBUTTONUP = 3,
	TREKEVENT_RBUTTONDOWN = 4,
	TREKEVENT_RBUTTONUP = 5,
	TREKEVENT_KEYDOWN = 6
};

struct TrekEvent {
	TrekEventType type;
	Common::KeyState kbd;
	Common::Point mouse;
	uint32 tick;
};

// Commands: Signals that can be passed to "handleAwayMissionCommands" or to room-specfiic
// code.
enum Commands {
	COMMAND_TICK = 0,
	COMMAND_WALK = 1, // Commands 1-5 correspond to Actions of the same number.
	COMMAND_USE = 2,
	COMMAND_GET = 3,
	COMMAND_LOOK = 4,
	COMMAND_TALK = 5,
	COMMAND_TOUCHED_WARP = 6,
	COMMAND_7 = 7, // Doors? (Or just hotspots activated by Kirk moving there?)
	COMMAND_FINISHED_BEAMING_IN = 10,
	COMMAND_FINISHED_ENTERING_ROOM = 12
};

struct Command {
	byte type;

	union { // FIXME: using unions in a dangeous way here...
		struct {
			byte b1;
			byte b2;
			byte b3;
		} gen;

		struct {
			byte activeObject;
			byte passiveObject;
		} action;
	};

	Command(byte _type, byte _b1, byte _b2, byte _b3)
		: type(_type) {
			gen.b1 = _b1;
			gen.b2 = _b2;
			gen.b3 = _b3;
		}
};

// Actions that can be used on away missions.
enum Acton {
	ACTION_WALK = 1,
	ACTION_USE = 2,
	ACTION_GET = 3,
	ACTION_LOOK = 4,
	ACTION_TALK = 5,
	ACTION_OPTIONS = 13 // Not really an action, but selectable from action menu
};


struct StarTrekGameDescription;
class Graphics;
class IWFile;
class Sound;

class StarTrekEngine : public ::Engine {
protected:
	Common::Error run();

private:
	// Game modes
	Common::Error runGameMode(int mode);

	// Away missions
	void initAwayMission();
	void runAwayMission();
	void cleanupAwayMission();
	void loadRoom(const Common::String &missionName, int roomIndex);
	void initAwayCrewPositions(int warpEntryIndex);
	void handleAwayMissionEvents();
	void unloadRoom();
	int loadActorAnimWithRoomScaling(int actorIndex, const Common::String &animName, int16 x, int16 y);
	uint16 getActorScaleAtPosition(int16 y);
	void addCommand(const Command &command);
	bool checkItemInteractionExists(int action, int activeItem, int passiveItem, int16 arg6);
	void handleAwayMissionCommand();

	bool isPointInPolygon(int16 *data, int16 x, int16 y);
	void checkTouchedLoadingZone(int16 x, int16 y);
	bool isPositionSolid(int16 x, int16 y);

public:
	SharedPtr<Room> getRoom();

private:
	// Transporter room
	void runTransportSequence(const Common::String &name);

public:
	StarTrekEngine(OSystem *syst, const StarTrekGameDescription *gamedesc);
	virtual ~StarTrekEngine();

	// Running the game
	void playSoundEffectIndex(int index);
	void playSpeech(const Common::String &filename);
	void stopPlayingSpeech();

	// Actors
	void initActors();
	int loadActorAnim(int actorIndex, const Common::String &animName, int16 x, int16 y, Fixed16 scale);
	bool actorWalkToPosition(int actorIndex, const Common::String &animFile, int16 srcX, int16 srcY, int16 destX, int16 destY);
	void updateActorAnimations();
	void removeActorFromScreen(int actorIndex);
	void actorFunc1();
	void drawActorToScreen(Actor *actor, const Common::String &animName, int16 x, int16 y, Fixed16 scale, bool addSprite);
	void releaseAnim(Actor *actor);
	void initStandAnim(int actorIndex);
	void updateActorPositionWhileWalking(Actor *actor, int16 x, int16 y);
	void chooseActorDirectionForWalking(Actor *actor, int16 srcX, int16 srcY, int16 destX, int16 destY);
	bool directPathExists(int16 srcX, int16 srcY, int16 destX, int16 destY);

	int findObjectAt(int x, int y);
	int findObjectAt(Common::Point p) { return findObjectAt(p.x, p.y); }
	SharedPtr<Bitmap> loadAnimationFrame(const Common::String &filename, Fixed16 scale);
	Common::String getCrewmanAnimFilename(int actorIndex, const Common::String &basename);
	void updateMouseBitmap();
	bool sub_2330c() { return false; } // TODO
	void showInventoryIcons(bool showItem);
	void hideInventoryIcons();
	int showInventoryMenu(int x, int y, bool restoreMouse);
	SharedPtr<Bitmap> scaleBitmap(SharedPtr<Bitmap> bitmap, Fixed16 scale);
	void scaleBitmapRow(byte *src, byte *dest, uint16 origWidth, uint16 scaledWidth);

	// Events
public:
	void pollSystemEvents();
	void initializeEventsAndMouse();
	bool getNextEvent(TrekEvent *e);
	void removeNextEvent();
	bool popNextEvent(TrekEvent *e);
	void addEventToQueue(const TrekEvent &e);
	void clearEventBuffer();
	void updateEvents();
	void updateTimerEvent();
	void updateMouseEvents();
	void updateKeyboardEvents();
	void updateClockTicks();
	bool checkKeyPressed();

	Common::EventManager *getEventMan() { return _eventMan; }

private:
	Common::List<TrekEvent> _eventQueue;
	bool _mouseMoveEventInQueue;
	bool _tickEventInQueue;
	uint32 _frameStartMillis;


	// text.cpp
public:
	const char *getNextTextLine(const char *text, char *line, int lineWidth);

	void getTextboxHeader(String *headerTextOutput, String speakerText, int choiceIndex);
	String readTextFromRdf(int choiceIndex, uintptr data, String *headerTextOutput);
	String readTextFromBuffer(int choiceIndex, uintptr data, String *headerTextOutput);

	void showTextbox(String headerText, const String &mainText, int xoffset, int yoffset, byte textColor, int maxTextLines); // TODO: better name. (return type?)

	String skipTextAudioPrompt(const String &str);
	String playTextAudio(const String &str);

	int showText(TextGetterFunc textGetter, uintptr var, int xoffset, int yoffset, int textColor, bool loopChoices, int maxTextLines, bool rclickCancelsChoice);

	int getNumTextboxLines(const String &str);
	String putTextIntoLines(const String &text);

	SharedPtr<TextBitmap> initTextSprite(int *xoffsetPtr, int *yoffsetPtr, byte textColor, int numTextLines, bool withHeader, Sprite *sprite);
	void drawMainText(SharedPtr<TextBitmap> bitmap, int numTextLines, int numTextboxLines, const String &text, bool withHeader);

	String readLineFormattedText(TextGetterFunc textGetter, uintptr var, int choiceIndex, SharedPtr<TextBitmap> textBitmap, int numTextboxLines, int *numLines);

	String readTextFromArray(int choiceIndex, uintptr data, String *headerTextOutput);

	// menu.cpp
public:
	int getMenuButtonAt(Sprite *sprites, int numSprites, int x, int y);
	void chooseMousePositionFromSprites(Sprite *sprites, int numSprites, int spriteIndex, int mode);
	void drawMenuButtonOutline(SharedPtr<Bitmap> bitmap, byte color);
	void showOptionsMenu(int x, int y);
	int showActionMenu();
	void loadMenuButtons(String mnuFilename, int xpos, int ypos);
	void setVisibleMenuButtons(uint32 bits);
	void disableMenuButtons(uint32 bits);
	void enableMenuButtons(uint32 bits);
	int handleMenuEvents(uint32 ticksUntilClickingEnabled, bool arg4);
	void unloadMenuButtons();

	void chooseMouseBitmapForAction(int action, bool withRedOutline);
	void showSaveMenu();
	void showLoadMenu();
	void showQuitGamePrompt(int x, int y);
	void showTextConfigurationMenu(bool fromOptionMenu);

	int loadTextDisplayMode();
	void saveTextDisplayMode(int value);


private:
	int16 _textDisplayMode;
	uint32 _textboxVar2;
	uint16 _textboxVar6;
	bool _textboxHasMultipleChoices;
	SharedPtr<Menu> _activeMenu;
	// Saved value of StarTrekEngine::_keyboardControlsMouse when menus are up
	bool _keyboardControlsMouseOutsideMenu;

	// Detection related functions
public:
	const StarTrekGameDescription *_gameDescription;
	uint32 getFeatures() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;
	uint8 getGameType();
	Common::Language getLanguage();

	// Resource related functions
	SharedPtr<FileStream> loadFile(Common::String filename, int fileIndex=0);

	// Movie related functions
	void playMovie(Common::String filename);
	void playMovieMac(Common::String filename);

	// Misc
	uint16 getRandomWord();
	Common::String getItemDescription(int itemIndex);


public:
	int _gameMode;
	int _lastGameMode;

	Common::String _missionToLoad;
	int _roomIndexToLoad;

	Common::String _missionName;
	int _roomIndex;
	Common::String _screenName; // _screenName = _missionName + _roomIndex
	Common::String _mapFilename; // Similar to _screenName, but used for .map files?
	SharedPtr<FileStream> _mapFile;
	int32 _playerActorScale;

	Common::String _txtFilename;
	Common::String _loadedText; // TODO: might be OK to delete this

	// Queue of "commands" (ie. next frame, clicked on object) for away mission or bridge
	Common::Queue<Command> _commandQueue;

	AwayMission _awayMission;
	bool _warpHotspotsActive;
	int16 _activeWarpHotspot;
	int16 _activeDoorWarpHotspot;
	int16 _lookActionBitmapIndex;

	Item _itemList[NUM_OBJECTS];

	Actor _actorList[NUM_ACTORS];
	Actor * const _kirkActor;
	Actor * const _spockActor;
	Actor * const _mccoyActor;
	Actor * const _redshirtActor;

	SharedPtr<FileStream> _actorBanFiles[NUM_ACTORS / 2];
	uint16 _actorBanVar2[NUM_ACTORS / 2]; // TODO: initialize?

	Sprite _inventoryIconSprite;
	Sprite _itemIconSprite;

	// _clockTicks is based on DOS interrupt 1A, AH=0; read system clock counter.
	// Updates 18.206 times every second.
	uint32 _clockTicks;
	uint32 _frameIndex;
	uint32 _roomFrameCounter; // Resets to 0 on loading a room

	bool _musicEnabled;
	bool _sfxEnabled;
	uint16 _word_467a6;
	uint16 _musicWorking;
	bool _sfxWorking;
	bool _finishedPlayingSpeech;

	bool _mouseControllingShip;

	// TODO: make this work.
	// When false, the keyboard generally acts in a more specific way (ie. move mouse
	// between items in a menu).
	bool _keyboardControlsMouse;

	bool _inQuitGameMenu;

	
	Graphics *_gfx;
	Sound *_sound;

private:
	Common::RandomSource _randomSource;

	Common::MacResManager *_macResFork;
	SharedPtr<Room> _room;
	SharedPtr<IWFile> _iwFile;
};

} // End of namespace StarTrek

#endif
