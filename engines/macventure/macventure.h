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
	bool offsecreen;
	bool invisible;
};

class MacVentureEngine : public Engine {

public:
	MacVentureEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~MacVentureEngine();

	virtual Common::Error run();

	void requestQuit();
	void requestUnpause();
	void selectControl(ControlReference id);
	void activateCommand(ControlReference id);

	void enqueueObject(ObjID id);

	// Data retrieval
	bool isPaused();
	Common::String getCommandsPausedString() const;
	const GlobalSettings& getGlobalSettings() const;
	Common::String getFilePath(FilePathID id) const;

private:
	void processEvents();

	bool runScriptEngine();
	void endGame();
	bool updateState();
	void runObjQueue();
	void updateControls();
	void resetVars();

	// Data loading
	bool loadGlobalSettings();
	bool loadTextHuffman();

	// Utils
	ControlAction referenceToAction(ControlReference id);

private: // Attributes

	const ADGameDescription *_gameDescription;
	Common::RandomSource *_rnd;

	Common::MacResManager *_resourceManager;

	Console *_debugger;
	Gui *_gui;
	World *_world;
	ScriptEngine *_scriptEngine;

	// Engine state
	GameState _gameState;
	GlobalSettings _globalSettings;
	StringTable *_filenames;
	Common::Huffman *_textHuffman;
	bool _oldTextEncoding;
	bool _paused, _halted, _cmdReady;
	bool _haltedAtEnd, _haltedInSelection;
	bool _gameChanged;

	Common::List<QueuedObject> _objQueue;
	Common::List<QueuedObject> _soundQueue;
	Common::List<QueuedObject> _textQueue;

	// Selections
	ObjID _destObject;
	ControlAction _selectedControl;
	ControlAction _activeControl;
	Common::List<ObjID> _currentSelection;
	Common::Point _deltaPoint;

private: // Methods

	const char* getGameFileName() const;

};


class Console : public GUI::Debugger {
public:
	Console(MacVentureEngine *vm) {}
	virtual ~Console(void) {}
};
} // End of namespace MacVenture

#endif
