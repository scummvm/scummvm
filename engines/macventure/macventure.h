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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#ifndef MACVENTURE_MACVENTURE_H
#define MACVENTURE_MACVENTURE_H

#include "engines/engine.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/debug.h"
#include "common/random.h"
#include "common/macresman.h"
#include "common/compression/huffman.h"
#include "common/savefile.h"

#include "gui/debugger.h"

#include "macventure/debug.h"
#include "macventure/gui.h"
#include "macventure/world.h"
#include "macventure/hufflists.h"
#include "macventure/stringtable.h"
#include "macventure/script.h"
#include "macventure/controls.h"
#include "macventure/windows.h"
#include "macventure/sound.h"

struct ADGameDescription;

namespace MacVenture {

class SaveFileManager;

class Console;
class World;
class ScriptEngine;

class SoundManager;

typedef uint32 ObjID;

enum {
	kScreenWidth = 512,
	kScreenHeight = 342
};

enum {
	kGlobalSettingsID = 0x80,
	kDiplomaGeometryID = 0x81,
	kTextHuffmanTableID = 0x83,
	kDialogBoxTextID = 0x84,
	kDialogBoxDiplomaID = 0x87
};

enum {
	kSaveGameStrID = 0x82,
	kDiplomaFilenameID = 0x83,
	kClickToContinueTextID = 0x84,
	kStartGameFilenameID = 0x85
};

enum FilePathID {
	kMCVID = 1,
	kTitlePathID = 2,
	kSubdirPathID = 3,
	kObjectPathID = 4,
	kFilterPathID = 5,
	kTextPathID = 6,
	kGraphicPathID = 7,
	kSoundPathID = 8
};

const uint kMaxConsoleTextLength = 200000;

class GlobalSettings {
public:
	GlobalSettings();
	~GlobalSettings();

	void loadSettings(Common::SeekableReadStream *dataStream);

// HACK MAybe this should be private, but the class is only here to handle
// memory allocation/deallocation
public:
	uint16 _numObjects;    // number of game objects defined
	uint16 _numGlobals;    // number of globals defined
	uint16 _numCommands;   // number of commands defined
	uint16 _numAttributes; // number of attributes
	uint16 _numGroups;     // number of object groups
	uint16 _invTop;        // inventory window bounds
	uint16 _invLeft;
	uint16 _invHeight;
	uint16 _invWidth;
	uint16 _invOffsetY;    // positioning offset for
	uint16 _invOffsetX;    // new inventory windows
	uint16 _defaultFont;   // default font
	uint16 _defaultSize;   // default font size
	Common::Array<uint8> _attrIndices; // attribute indices into attribute table
	Common::Array<uint16> _attrMasks;   // attribute masks
	Common::Array<uint8> _attrShifts;  // attribute bit shifts
	Common::Array<uint8> _cmdArgCnts;  // command argument counts
	Common::Array<uint8> _commands;    // command buttons
};

enum GameState {
	kGameStateInit,
	kGameStatePlaying,
	kGameStateWinning,
	kGameStateLosing,
	kGameStateQuitting
};

enum ObjectQueueID {
	kFocusWindow = 2,
	kOpenWindow = 3,
	kCloseWindow = 4,
	kUpdateObject = 7,
	kUpdateWindow = 8,
	kSetToPlayerParent = 12,
	kHightlightExits = 13,
	kAnimateBack = 14
};

struct QueuedObject {
	ObjectQueueID id;
	ObjID object;
	ObjID parent;
	uint x;
	uint y;
	uint exitx;
	uint exity;
	bool hidden;
	bool offscreen;
	bool invisible;
	ObjID target; // For swapping
};

enum TextQueueID {
	kTextNumber = 1,
	kTextNewLine = 2,
	kTextPlain = 3
};

struct QueuedText {
	TextQueueID id;
	ObjID source;
	ObjID destination;
	ObjID asset;
};

enum SoundQueueID {
	kSoundPlay = 1,
	kSoundPlayAndWait = 2,
	kSoundWait = 3
};

struct QueuedSound {
	SoundQueueID id;
	ObjID reference;
};

struct Item {
	ObjID id;
	Common::Rect bounds;
};

class Layout {
public:
	void append(Item item) {
		_items.push_back(item);
		_width += 8 + item.bounds.width();
	}

	Item remove(uint index) {
		Item out = _items.remove_at(index);
		_width -= 8 + out.bounds.width();
		return out;
	}

	Item at(uint index) {
		return _items[index];
	}

	inline uint size() const { return _items.size(); }
	inline int width() const { return _width; }

private:
	int _width = 8;
	Common::Array<Item> _items;
};

class MacVentureEngine : public Engine {

public:
	MacVentureEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~MacVentureEngine() override;

	void initializePath(const Common::FSNode &gamePath) override;

	bool hasFeature(EngineFeature f) const override;

	Common::Error run() override;

	bool scummVMSaveLoadDialog(bool isSave);
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	void newGame();
	void setInitialFlags(GameState gameState = kGameStateInit);
	void setNewGameState();

	void reset();
	void resetInternals();
	void resetGui();
	void refreshScreen();

	// datafiles.cpp
	void loadDataBundle();
	Common::SeekableReadStream *getBorderFile(MVWindowType windowType, bool isActive);

	void requestQuit();
	void requestUnpause();
	void selectControl(ControlAction action);
	void refreshReady();
	void preparedToRun();
	void gameChanged();
	void winGame();
	void loseGame();
	void clickToContinue();

	void updateState(bool pause);
	void revert();

	void enqueueObject(ObjectQueueID type, ObjID objID, ObjID target = 0);
	void enqueueText(TextQueueID type, ObjID target, ObjID source, ObjID text);
	void enqueueSound(SoundQueueID type, ObjID target);

	void runObjQueue();
	void printTexts();
	void playSounds(bool pause);

	Item removeOutlier(Layout &layout, bool flag, Common::Rect rect);
	void cleanUp(WindowReference reference);
	void messUp(WindowReference reference);
	void moveItems(Common::Array<Item> &items, WindowReference reference);

	void unselectAll();
	void selectObject(ObjID objID);
	void handleObjectSelect(ObjID objID, WindowReference win, bool shiftPressed, bool isDoubleClick);
	void handleObjectDrop(ObjID objID, Common::Point delta, ObjID newParent);
	void setDeltaPoint(Common::Point newPos);
	void focusObjectWindow(ObjID objID);
	void updateWindow(WindowReference winID);

	bool showTextEntry(ObjID text, ObjID srcObj, ObjID destObj);
	void setTextInput(const Common::String &content);
	Common::String getUserInput();

	// Data retrieval
	Common::Path getDiplomaFileName();
	Common::Path getStartGameFileName();
	bool isPaused();
	bool needsClickToContinue();
	Common::String getCommandsPausedString() const;
	const GlobalSettings &getGlobalSettings() const;
	Common::Path getFilePath(FilePathID id) const;
	bool isOldText() const;
	const HuffmanLists *getDecodingHuffman() const;
	uint32 randBetween(uint32 min, uint32 max);
	uint32 getInvolvedObjects();
	int findObjectInArray(ObjID objID, const Common::Array<ObjID> &list);
	uint getPrefixNdx(ObjID obj);
	Common::String getPrefixString(uint flag, ObjID obj);
	Common::String getNoun(ObjID ndx);
	ScriptEngine *getScriptEngine() const { return _scriptEngine; }
	Common::Array<ObjID> &getSelectedObjects() { return _selectedObjs; }
	GameState getGameState() const { return _gameState; }

	Common::String getConsoleText() const;
	void setConsoleText(const Common::String &text);

	// Attributes consult
	Common::Point getObjPosition(ObjID objID);
	bool isObjVisible(ObjID objID);
	bool isObjClickable(ObjID objID);
	bool isObjDraggable(ObjID objID);
	bool isObjSelected(ObjID objID);
	bool isObjExit(ObjID objID);
	bool isHiddenExit(ObjID objID);
	Common::Point getObjExitPosition(ObjID objID);
	ObjID getParent(ObjID objID);

	// Utils
	ControlAction referenceToAction(ControlType id);

	// Encapsulation HACK
	Common::Rect getObjBounds(ObjID objID);
	uint getOverlapPercent(ObjID one, ObjID other);

	WindowReference getObjWindow(ObjID objID);
	WindowReference findParentWindow(ObjID objID);

	Common::Point getDeltaPoint();
	ObjID getDestObject();
	ControlAction getSelectedControl();

private:
	void processEvents();

	bool runScriptEngine();
	void endGame();
	void updateControls();
	void resetVars();

	void unselectObject(ObjID objID);
	void highlightExit(ObjID objID);
	void selectPrimaryObject(ObjID objID);
	void updateExits();

	// Object queue methods
	void openObject(ObjID objID);
	void closeObject(ObjID objID);
	void checkObject(QueuedObject objID);
	void reflectSwap(ObjID fromID, ObjID toID);
	void toggleExits();
	void zoomObject(ObjID objID);

	bool isObjEnqueued(ObjID obj);

	bool isGameRunning();

	// Data loading
	bool loadGlobalSettings();
	bool loadTextHuffman();

	const char *getGameFileName() const;

	Common::String capitalize(const Common::String &str) const;

private: // Attributes

	const ADGameDescription *_gameDescription;
	Common::RandomSource *_rnd;

	Common::MacResManager *_resourceManager;

	Gui *_gui;
	World *_world;
	ScriptEngine *_scriptEngine;

	// String tables
	StringTable *_filenames;
	StringTable *_decodingDirectArticles;
	StringTable *_decodingNamingArticles;
	StringTable *_decodingIndirectArticles;

	SoundManager *_soundManager;

	Common::FSNode _gamePath;
	Common::Archive *_dataBundle;

	// Engine state
	GameState _gameState;
	GlobalSettings *_globalSettings;
	HuffmanLists *_textHuffman;
	bool _oldTextEncoding;
	bool _paused, _halted, _cmdReady, _prepared;
	bool _haltedAtEnd, _haltedInSelection;
	bool _gameChanged;
	bool _clickToContinue;
	bool _enginePaused;

	Common::Array<QueuedObject> _objQueue;
	Common::Array<QueuedObject> _inQueue;
	Common::Array<QueuedSound> _soundQueue;
	Common::Array<QueuedText> _textQueue;

	// Selections
	ObjID _destObject;
	ControlAction _selectedControl;
	Common::Array<ObjID> _currentSelection;
	Common::Array<ObjID> _selectedObjs;
	Common::Point _deltaPoint;
	Common::String _userInput;

	Common::String _currentConsoleText;
};


class Console : public GUI::Debugger {
public:
	Console(MacVentureEngine *vm) {}
	~Console(void) override {}
};
} // End of namespace MacVenture

#endif
