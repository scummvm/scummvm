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

//#define SCRIPT_TEST
//#define INTRO_TEST

#include "m4/globals.h"
#include "m4/burger_data.h"
#include "m4/m4.h"
#include "m4/resource.h"
#include "m4/hotspot.h"
#include "m4/font.h"
#include "m4/rails.h"
#include "m4/midi.h"
#include "m4/events.h"
#include "m4/graphics.h"
#include "m4/viewmgr.h"
#include "m4/gui.h"
#include "m4/woodscript.h"
#include "m4/actor.h"
#include "m4/sound.h"
#include "m4/script.h"
#include "m4/compression.h"
#include "m4/animation.h"
#include "m4/m4_menus.h"
#include "m4/m4_views.h"
#include "m4/mads_anim.h"
#include "m4/mads_menus.h"

#include "common/error.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/textconsole.h"
#include "engines/util.h"

namespace M4 {

// FIXME: remove global
MadsM4Engine *_vm;
MadsEngine *_madsVm;
M4Engine *_m4Vm;

void escapeHotkeyHandler(MadsM4Engine *vm, View *view, uint32 key) {
	// For now, simply exit the game
	vm->_events->quitFlag = true;
}

// Temporary hotkey handler for use in testing the TextviewView class

void textviewHotkeyHandler(MadsM4Engine *vm, View *view, uint32 key) {
	// Deactivate the scene if it's currently active
	View *sceneView = vm->_viewManager->getView(VIEWID_SCENE);
	if (sceneView != NULL)
		vm->_viewManager->deleteView(sceneView);

	// Activate the textview view
	vm->_font->setFont(FONT_CONVERSATION_MADS);
	TextviewView *textView = new TextviewView(vm);
	vm->_viewManager->addView(textView);
	textView->setScript("quotes", NULL);
}

void saveGameHotkeyHandler(MadsM4Engine *vm, View *view, uint32 key) {
	// TODO: See CreateF2SaveMenu - save menu should only be activated when
	// certain conditions are met, such as player_commands_allowed, and isInterfaceVisible
	vm->loadMenu(SAVE_MENU, true);
}

void loadGameHotkeyHandler(MadsM4Engine *vm, View *view, uint32 key) {
	// TODO: See CreateF3LoadMenu - save menu should only be activated when
	// certain conditions are met, such as player_commands_allowed, and isInterfaceVisible
	vm->loadMenu(LOAD_MENU, true);
}

void gameMenuHotkeyHandler(MadsM4Engine *vm, View *view, uint32 key) {
	vm->loadMenu(GAME_MENU);
}

MadsM4Engine::MadsM4Engine(OSystem *syst, const M4GameDescription *gameDesc) :
	Engine(syst), _gameDescription(gameDesc) {
	// Setup mixer
	syncSoundSettings();

	// FIXME
	_vm = this;
	_madsVm = NULL;

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	SearchMan.addSubDirectoryMatching(gameDataDir, "goodstuf");
	SearchMan.addSubDirectoryMatching(gameDataDir, "resource");
	SearchMan.addSubDirectoryMatching(gameDataDir, "option1");

	DebugMan.addDebugChannel(kDebugScript, "script", "Script debug level");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics debug level");
	DebugMan.addDebugChannel(kDebugConversations, "conversations", "Conversations debugging");
	DebugMan.addDebugChannel(kDebugSound, "sound", "Sounds debug level");
	DebugMan.addDebugChannel(kDebugCore, "core", "Core debug level");

	_resourceManager = NULL;
	_globals = NULL;
}


MadsM4Engine::~MadsM4Engine() {
	delete _midi;
	delete _saveLoad;
	delete _kernel;
	delete _player;
	delete _mouse;
	delete _events;
	delete _font;
	delete _actor;
//	delete _scene;		// deleted by the viewmanager
	delete _dialogs;
	delete _screen;
	delete _inventory;
	delete _viewManager;
	delete _rails;
	delete _script;
	delete _ws;
	delete _random;
	delete _palette;
	delete _globals;
	delete _sound;
	delete _resourceManager;
}

Common::Error MadsM4Engine::run() {
	// Initialize backend
	_screen = new M4Surface(true); // Special form for creating screen reference

	_midi = new MidiPlayer(this);
	_midi->setGM(true);	// FIXME: Really? Always?

	_saveLoad = new SaveLoad(this);
	_palette = new Palette(this);
	_mouse = new Mouse(this);
	_events = new Events(this);
	_kernel = new Kernel(this);
	_player = new Player(this);
	_font = new FontManager(this);
	if (getGameType() == GType_Burger) {
		_actor = new Actor(this);
		_conversationView = new ConversationView(this);
	} else {
		_actor = NULL;
	}
	_rails = new Rails();	// needs to be initialized before _scene
	_dialogs = new Dialogs();
	_viewManager = new ViewManager(this);
	_inventory = new Inventory(this);
	_sound = new Sound(this, _mixer, 255);
	_script = new ScriptInterpreter(this);
	_ws = new WoodScript(this);
	//_callbacks = new Callbacks(this);
	_random = new Common::RandomSource("m4");

	return Common::kNoError;
}

void MadsM4Engine::eventHandler() {
	M4EventType event;
	uint32 keycode = 0;

	if ((event = _events->handleEvents()) != MEVENT_NO_EVENT) {
		if (_viewManager->containsViews())
			_viewManager->handleMouseEvents(event);
	}

	if (_events->kbdCheck(keycode))
		_viewManager->handleKeyboardEvents(keycode);
}

bool MadsM4Engine::delay(int duration, bool keyAborts, bool clickAborts) {
	uint32 endTime = g_system->getMillis() + duration;
	M4EventType event;
	uint32 keycode = 0;

	while (!_events->quitFlag && (g_system->getMillis() < endTime)) {
		event = _events->handleEvents();
		if (clickAborts && ((event == MEVENT_LEFT_RELEASE) || (event == MEVENT_RIGHT_RELEASE)))
			return true;

		if (_events->kbdCheck(keycode)) {
			if (keyAborts)
				return true;
		}

		g_system->delayMillis(10);
	}

	return false;
}

void MadsM4Engine::loadMenu(MenuType menuType, bool loadSaveFromHotkey, bool calledFromMainMenu) {
	if (isM4() && (menuType != MAIN_MENU)) {
		bool menuActive = _viewManager->getView(VIEWID_MENU) != NULL;

		if (!menuActive)
			_palette->fadeToGreen(M4_DIALOG_FADE_STEPS, M4_DIALOG_FADE_DELAY);
	}

	View *view;

	switch (menuType) {
	case MAIN_MENU:
		if (getGameType() == GType_RexNebular)
			view = new RexMainMenuView(this);
		else if (getGameType() == GType_DragonSphere)
			view = new DragonMainMenuView(this);
		else
			view = new MadsMainMenuView(this);
		break;
	case GAME_MENU:
		view = new OrionMenuView(this, 200, 100, GAME_MENU, calledFromMainMenu, loadSaveFromHotkey);
		break;
	case OPTIONS_MENU:
		view = new OrionMenuView(this, 172, 160, OPTIONS_MENU, calledFromMainMenu, loadSaveFromHotkey);
		break;
	case LOAD_MENU:
	case SAVE_MENU:
		view = new OrionMenuView(this, 145, 10, menuType, calledFromMainMenu, loadSaveFromHotkey);
		break;
	default:
		error("Unknown menu type");
		break;
	}

	_viewManager->addView(view);
	_viewManager->moveToFront(view);
}

#define DUMP_BUFFER_SIZE 1024

void MadsM4Engine::dumpFile(const char *filename, bool uncompress) {
	Common::DumpFile f;
	byte buffer[DUMP_BUFFER_SIZE];
	Common::SeekableReadStream *fileS = res()->get(filename);

	if (!f.open(filename))
		error("Could not open '%s' for writing", filename);

	int bytesRead = 0;
	warning("Dumping %s, size: %i\n", filename, fileS->size());

	if (!uncompress) {
		while (!fileS->eos()) {
			bytesRead = fileS->read(buffer, DUMP_BUFFER_SIZE);
			f.write(buffer, bytesRead);
		}
	} else {
		MadsPack packData(fileS);
		Common::SeekableReadStream *sourceUnc;
		for (int i = 0; i < packData.getCount(); i++) {
			sourceUnc = packData.getItemStream(i);
			debugCN(kDebugCore, "Dumping compressed chunk %i of %i, size is %i\n", i + 1, packData.getCount(), sourceUnc->size());
			while (!sourceUnc->eos()) {
				bytesRead = sourceUnc->read(buffer, DUMP_BUFFER_SIZE);
				f.write(buffer, bytesRead);
			}
			delete sourceUnc;
		}
	}

	f.close();
	res()->toss(filename);
	res()->purge();
}

/*--------------------------------------------------------------------------*/

M4Engine::M4Engine(OSystem *syst, const M4GameDescription *gameDesc): MadsM4Engine(syst, gameDesc) {
	// FIXME
	_m4Vm = this;

	_globals = new M4Globals(this);
}

M4Engine::~M4Engine() {
	delete _converse;
}

Common::Error M4Engine::run() {
	// Set up the graphics mode
	initGraphics(640, 480, true);

	// Necessary pre-initialisation
	_resourceManager = new M4ResourceManager(this);

	// Set up needed common functionality
	MadsM4Engine::run();

	// M4 specific initialisation
	_converse = new Converse(this);

	_scene = new M4Scene(this);
	_script->open("m4.dat");

#ifdef SCRIPT_TEST

#if 0
	ScriptFunction *func = _script->loadFunction("room_parser_142");
	_script->runFunction(func);
#endif

#if 1
	ScriptFunction *func = _script->loadFunction("room_daemon_951");
	for (int i = 1; i < 58; i++) {
		_vm->_kernel->trigger = i;
		_script->runFunction(func);
		debugCN(kDebugCore, "=================================\n");
	}
#endif

	return Common::kNoError;
#endif

	// Set up the inventory

	// Set up the game interface view
	//_interfaceView->inventoryAdd("Money", "", 55);	// Sample item

	if (getGameType() == GType_Burger) {
		for (int i = 0; i < ARRAYSIZE(burger_inventory); i++) {
			char* itemName = strdup(burger_inventory[i].name);
			_inventory->registerObject(itemName, burger_inventory[i].scene,
									   burger_inventory[i].icon);
			_inventory->addToBackpack(i);	// debug: this adds ALL objects to the player's backpack
		}
	}

	// Show intro

	if (getGameType() == GType_Burger) {
		_kernel->newRoom = TITLE_SCENE_BURGER;
	} else {
		_scene->getBackgroundSurface()->loadBackgroundRiddle("main menu");
		_ws->setBackgroundSurface(_scene->getBackgroundSurface());
	}

	_viewManager->addView(_scene);

	// Setup game wide hotkeys. Note that Orion Burger used F2/F3 for Save/Restore,
	// but for standardisation with most other games, F5/F7 are also mapped

	_viewManager->systemHotkeys().add(Common::KEYCODE_ESCAPE, &escapeHotkeyHandler);
	_viewManager->systemHotkeys().add(Common::KEYCODE_F2, &saveGameHotkeyHandler);
	_viewManager->systemHotkeys().add(Common::KEYCODE_F3, &loadGameHotkeyHandler);
	_viewManager->systemHotkeys().add(Common::KEYCODE_F5, &saveGameHotkeyHandler);
	_viewManager->systemHotkeys().add(Common::KEYCODE_F7, &loadGameHotkeyHandler);
	_viewManager->systemHotkeys().add(Common::KEYCODE_F9, &gameMenuHotkeyHandler);

	// Start playing Orion Burger intro music
	//_midi->playMusic("999intro", 255, false, -1, -1);

	// TODO: start playing intro animations

	// TODO: Master Lu

	// Test for mouse
	_mouse->init("cursor", NULL);
	_mouse->setCursorNum(0);
	_mouse->cursorOn();

	_ws->assets()->loadAsset("SHOW SCRIPT", NULL);
	_ws->assets()->loadAsset("STREAM SCRIPT", NULL);

#ifdef INTRO_TEST
	int curPart = 0;
	Machine *mach = NULL;
#endif

	_ws->setSurfaceView(_scene);

	uint32 nextFrame = g_system->getMillis();
	while (!_events->quitFlag) {

		// This should probably be moved to either Scene or Kernel
		if (_kernel->currentRoom != _kernel->newRoom) {

			_ws->clear();

			_kernel->currentSection = _kernel->newRoom / 100;
			_kernel->currentRoom = _kernel->newRoom;

			_scene->loadScene(_kernel->currentRoom);

			_ws->setBackgroundSurface(_scene->getBackgroundSurface());
			_ws->setInverseColorTable(scene()->getInverseColorTable());

			_kernel->loadSectionScriptFunctions();
			_kernel->loadRoomScriptFunctions();

			_kernel->roomInit();

			_scene->show();

#ifdef INTRO_TEST
			if (_kernel->currentRoom == 951) {
				curPart = 0;
				mach = _ws->streamSeries("PLANET X HILLTOP A", 1, 0x1000, 0);
			}
#endif

		}

		eventHandler();

		// Call the updateState method of all views
		_viewManager->updateState();

		// Handle frame updates
		if (g_system->getMillis() >= nextFrame) {
#ifdef INTRO_TEST
			// Orion Burger intro test (scene 951)
			// This is ugly and bad, machine is not deleted so there's a huge memory
			// leak too. But hey, we can see some of the intro!
			if (mach && mach->getState() == -1) {
				if (curPart == 0)
					mach = _ws->streamSeries("Planet X Low Ground Shot", 1, 0x1000, 0);
				else if (curPart == 1)
					mach = _ws->streamSeries("Planet X Hilltop B", 1, 0x1000, 0);
				else if (curPart == 2)
					mach = _ws->streamSeries("Space Station Panorama A", 1, 0x1000, 0);
				else if (curPart == 3)
					mach = _ws->streamSeries("Cargo Transfer Area A", 1, 0x1000, 0);
				else if (curPart == 4)
					mach = _ws->streamSeries("VP's Office A", 1, 0x1000, 0);
				else if (curPart == 5)
					mach = _ws->streamSeries("Hologram", 1, 0x1000, 0);
				else if (curPart == 6)
					mach = _ws->streamSeries("VP's Office B", 1, 0x1000, 0);
				else if (curPart == 7)
					mach = _ws->streamSeries("Cargo Transfer Area B", 1, 0x1000, 0);
				else if (curPart == 8)
					mach = _ws->streamSeries("Cargo Transfer Controls", 1, 0x1000, 0);
				else if (curPart == 9)
					mach = _ws->streamSeries("Space Station Panorama B", 1, 0x1000, 0);
				// This last scene is from the rolling demo
				//else if (curPart == 10)
				//	mach = _ws->streamSeries("Call To Action", 1, 0x1000, 0);
				curPart++;
			}
#endif
			_ws->update();
			_viewManager->refreshAll();
			nextFrame = g_system->getMillis();// + GAME_FRAME_DELAY;

			// TEST STUFF ONLY
			if (_player->commandReady) {
				_kernel->roomParser();
				_player->commandReady = false;
			}

		}

		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

/*--------------------------------------------------------------------------*/

MadsEngine::MadsEngine(OSystem *syst, const M4GameDescription *gameDesc): MadsM4Engine(syst, gameDesc) {
	// FIXME
	_madsVm = this;

	_globals = new MadsGlobals(this);
	_currentTimer = 0;
}

MadsEngine::~MadsEngine() {
}

Common::Error MadsEngine::run() {
	// Set up the graphics mode
	initGraphics(320, 200, false);

	// Necessary pre-initialisation
	_resourceManager = new MADSResourceManager(this);

	// Set up needed common functionality
	MadsM4Engine::run();

	_palette->setMadsSystemPalette();

	_mouse->init("cursor.ss", NULL);
	_mouse->setCursorNum(0);

	// Load MADS data files
	MadsGlobals *globs = (MadsGlobals *)_globals;
	globs->loadMadsVocab();			// vocab.dat
	globs->loadQuotes();			// quotes.dat
	globs->loadMadsMessagesInfo();	// messages.dat
	globs->loadMadsObjects();

	// Setup globals
	globs->_config.easyMouse = true;
	globs->_config.invObjectsStill = false;
	globs->_config.textWindowStill = false;
	globs->_config.storyMode = 1;	// Naughty
	globs->_config.screenFades = 0;

	// Test code to dump all messages to the console
	//for (int i = 0; i < _globals->getMessagesSize(); i++)
	//debugCN(kDebugCore, "%s\n----------\n", _globals->loadMessage(i));

	if (getGameType() == GType_RexNebular) {
		MadsGameLogic::initializeGlobals();

		_scene = NULL;
		loadMenu(MAIN_MENU);
	} else {
		// Test code
		_scene = new MadsScene(this);

		startScene(FIRST_SCENE);
		RGBList *_bgPalData;
		_scene->loadBackground(FIRST_SCENE, &_bgPalData);
		_palette->addRange(_bgPalData);
		_scene->translate(_bgPalData);

		_scene->show();

		_font->setFont(FONT_MAIN_MADS);
		_font->current()->setColors(2, 1, 3);
		_font->current()->writeString(_scene->getBackgroundSurface(), "Testing the M4/MADS ScummVM engine", 5, 160, 310, 2);
		_font->current()->writeString(_scene->getBackgroundSurface(), "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 5, 180, 310, 2);

		if (getGameType() == GType_DragonSphere) {
			//_scene->showMADSV2TextBox("Test", 10, 10, NULL);
		}

		_mouse->cursorOn();
	}

	_viewManager->systemHotkeys().add(Common::KEYCODE_ESCAPE, &escapeHotkeyHandler);
	_viewManager->systemHotkeys().add(Common::KEYCODE_KP_MULTIPLY, &textviewHotkeyHandler);

	uint32 nextFrame = g_system->getMillis();
	while (!_events->quitFlag) {
		eventHandler();

		if (g_system->getMillis() >= nextFrame) {
			nextFrame = g_system->getMillis() + GAME_FRAME_DELAY;
			++_currentTimer;

			// Call the updateState method of all views
			_viewManager->updateState();

			// Refresh the display
			_viewManager->refreshAll();
		}

		g_system->delayMillis(10);

		if (globals()->dialogType != DIALOG_NONE)
			showDialog();
	}

	return Common::kNoError;
}

void MadsEngine::showDialog() {
	// Switch to showing the given dialog
	RexDialogView *dlg = NULL;
	switch (globals()->dialogType) {
	case DIALOG_GAME_MENU:
		dlg = new RexGameMenuDialog();
		break;
	case DIALOG_OPTIONS:
		dlg = new RexOptionsDialog();
		break;
	default:
		error("Unknown dialog type");
	};

	globals()->dialogType = DIALOG_NONE;
	_viewManager->addView(dlg);
}

} // End of namespace M4
