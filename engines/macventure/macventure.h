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

#ifndef MACVENTURE_H
#define MACVENTURE_H

#include "engines/engine.h"
#include "common/debug.h"
#include "common/random.h"
#include "common/macresman.h"
#include "common/huffman.h"

#include "gui/debugger.h"

#include "macventure/gui.h"
#include "macventure/world.h"
#include "macventure/hufflists.h"
#include "macventure/stringtable.h"
#include "macventure/script.h"

struct ADGameDescription;

namespace MacVenture {

class Console;
class World;
class ScriptEngine;

typedef uint32 ObjID;

enum {
	kScreenWidth = 512,
	kScreenHeight = 342
};

enum {
	kMacVentureDebugExample = 1 << 0,
	kMacVentureDebugExample2 = 1 << 1
	// next new level must be 1 << 2 (4)
	// the current limitation is 32 debug levels (1 << 31 is the last one)
};

enum {
	kGlobalSettingsID = 0x80,
	kDiplomaGeometryID = 0x81,
	kTextHuffmanTableID = 0x83
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


struct GlobalSettings {
	uint16 numObjects;    // number of game objects defined
	uint16 numGlobals;    // number of globals defined
	uint16 numCommands;   // number of commands defined
	uint16 numAttributes; // number of attributes
	uint16 numGroups;     // number of object groups
	uint16 invTop;        // inventory window bounds
	uint16 invLeft;
	uint16 invHeight;
	uint16 invWidth;
	uint16 invOffsetY;    // positioning offset for
	uint16 invOffsetX;    // new inventory windows
	uint16 defaultFont;   // default font
	uint16 defaultSize;   // default font size
	uint8  *attrIndices; // attribute indices into attribute table
	uint16 *attrMasks;   // attribute masks
	uint8  *attrShifts;  // attribute bit shifts
	uint8  *cmdArgCnts;  // command argument counts
	uint8  *commands;    // command buttons
};

enum GameState {
	kGameStateInit,
	kGameStatePlaying,
	kGameStateWinnig,
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

class MacVentureEngine : public Engine {

public:
	MacVentureEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~MacVentureEngine();

	virtual Common::Error run();

	void requestQuit();
	void requestUnpause();
	void selectControl(ControlAction action);
	void activateCommand(ControlAction id);
	void refreshReady();
	void preparedToRun();
	void gameChanged();
	void winGame();
	void loseGame();
	void clickToContinue();

	bool updateState();
	void revert();

	void enqueueObject(ObjectQueueID type, ObjID objID, ObjID target = 0);
	void enqueueText(TextQueueID type, ObjID target, ObjID source, ObjID text);

	void runObjQueue();
	bool printTexts();

	void handleObjectSelect(ObjID objID, WindowReference win, bool shiftPressed, bool isDoubleClick);
	void handleObjectDrop(ObjID objID, Common::Point delta, ObjID newParent);
	void updateDelta(Common::Point newPos);
	void focusObjWin(ObjID objID);
	void updateWindow(WindowReference winID);

	bool showTextEntry(ObjID text, ObjID srcObj, ObjID destObj);
	void setTextInput(Common::String content);
	Common::String getUserInput();

	// Data retrieval
	bool isPaused();
	bool needsClickToContinue();
	Common::String getCommandsPausedString() const;
	const GlobalSettings& getGlobalSettings() const;
	Common::String getFilePath(FilePathID id) const;
	bool isOldText() const;
	const HuffmanLists *getDecodingHuffman() const;
	uint32 randBetween(uint32 min, uint32 max);
	uint32 getInvolvedObjects();
	int findObjectInArray(ObjID objID, const Common::Array<ObjID> &list);
	uint getPrefixNdx(ObjID obj);
	Common::String getPrefixString(uint flag, ObjID obj);
	Common::String getNoun(ObjID ndx);

	// Attributes consult
	Common::Point getObjPosition(ObjID objID);
	bool isObjVisible(ObjID objID);
	bool isObjClickable(ObjID objID);
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

	void unselectAll();
	void selectObject(ObjID objID);
	void unselectObject(ObjID objID);
	void highlightExit(ObjID objID);
	void selectPrimaryObject(ObjID objID);
	void updateExits();

	// Object queue methods
	void focusObjectWindow(ObjID objID);
	void openObject(ObjID objID);
	void closeObject(ObjID objID);
	void checkObject(QueuedObject objID);
	void reflectSwap(ObjID fromID, ObjID toID);
	void toggleExits();
	void zoomObject(ObjID objID);

	bool isObjEnqueued(ObjID obj);

	// Data loading
	bool loadGlobalSettings();
	bool loadTextHuffman();

	const char* getGameFileName() const;

private: // Attributes

	const ADGameDescription *_gameDescription;
	Common::RandomSource *_rnd;

	Common::MacResManager *_resourceManager;

	Console *_debugger;
	Gui *_gui;
	World *_world;
	ScriptEngine *_scriptEngine;

	// String tables
	StringTable *_filenames;
	StringTable *_decodingDirectArticles;
	StringTable *_decodingNamingArticles;
	StringTable *_decodingIndirectArticles;

	// Engine state
	GameState _gameState;
	GlobalSettings _globalSettings;
	HuffmanLists *_textHuffman;
	bool _oldTextEncoding;
	bool _paused, _halted, _cmdReady, _prepared;
	bool _haltedAtEnd, _haltedInSelection;
	bool _gameChanged;
	bool _clickToContinue;

	Common::Array<QueuedObject> _objQueue;
	Common::Array<QueuedObject> _inQueue;
	Common::Array<QueuedObject> _soundQueue;
	Common::Array<QueuedText> _textQueue;

	// Selections
	ObjID _destObject;
	ControlAction _selectedControl;
	ControlAction _activeControl;
	Common::Array<ObjID> _currentSelection;
	Common::Array<ObjID> _selectedObjs;
	Common::Point _deltaPoint;
	Common::String _userInput;

};


class Console : public GUI::Debugger {
public:
	Console(MacVentureEngine *vm) {}
	virtual ~Console(void) {}
};
} // End of namespace MacVenture

#endif
