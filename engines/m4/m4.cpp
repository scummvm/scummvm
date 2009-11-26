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
 * $URL$
 * $Id$
 *
 */

//#define SCRIPT_TEST
//#define INTRO_TEST

#include "m4/globals.h"
#include "m4/burger_data.h"
#include "m4/m4.h"
#include "m4/resource.h"
#include "m4/sprite.h"
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
#include "m4/rails.h"
#include "m4/script.h"
#include "m4/compression.h"
#include "m4/animation.h"
#include "m4/m4_menus.h"
#include "m4/m4_views.h"
#include "m4/mads_anim.h"
#include "m4/mads_menus.h"

#include "common/file.h"
#include "common/events.h"
#include "common/EventRecorder.h"
#include "common/endian.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "engines/engine.h"
#include "graphics/surface.h"
#include "sound/mididrv.h"

namespace M4 {

// FIXME: remove global
M4Engine *_vm;

void escapeHotkeyHandler(M4Engine *vm, View *view, uint32 key) {
	// For now, simply exit the game
	vm->_events->quitFlag = true;
}

// Temporary hotkey handler for use in testing the TextviewView class

void textviewHotkeyHandler(M4Engine *vm, View *view, uint32 key) {
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

void saveGameHotkeyHandler(M4Engine *vm, View *view, uint32 key) {
	// TODO: See CreateF2SaveMenu - save menu should only be activated when
	// certain conditions are met, such as player_commands_allowed, and isInterfaceVisible
	vm->loadMenu(SAVE_MENU, true);
}

void loadGameHotkeyHandler(M4Engine *vm, View *view, uint32 key) {
	// TODO: See CreateF3LoadMenu - save menu should only be activated when
	// certain conditions are met, such as player_commands_allowed, and isInterfaceVisible
	vm->loadMenu(LOAD_MENU, true);
}

void gameMenuHotkeyHandler(M4Engine *vm, View *view, uint32 key) {
	vm->loadMenu(GAME_MENU);
}

M4Engine::M4Engine(OSystem *syst, const M4GameDescription *gameDesc) :
	Engine(syst), _gameDescription(gameDesc) {

	// FIXME
	_vm = this;

	SearchMan.addSubDirectoryMatching(_gameDataDir, "goodstuf");
	SearchMan.addSubDirectoryMatching(_gameDataDir, "resource");

	Common::addDebugChannel(kDebugScript, "script", "Script debug level");
	Common::addDebugChannel(kDebugConversations, "conversations", "Conversations debugging");
}


M4Engine::~M4Engine() {
	delete _globals;
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
	delete _converse;
	delete _script;
	delete _ws;
	delete _random;
	delete _animation;
	delete _palette;
	delete _resourceManager;
}

Common::Error M4Engine::run() {
	// Initialize backend
	if (isM4())
		initGraphics(640, 480, true);
	else
		initGraphics(320, 200, false);

	_screen = new M4Surface(true); // Special form for creating screen reference

	int midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	bool native_mt32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));

	MidiDriver *driver = MidiDriver::createMidi(midiDriver);
	if (native_mt32)
		driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	_midi = new MidiPlayer(this, driver);
	_midi->setGM(true);
	_midi->setNativeMT32(native_mt32);

	_globals = new Globals(this);
	if (isM4())
		_resourceManager = new M4ResourceManager(this);
	else
		_resourceManager = new MADSResourceManager(this);
	_saveLoad = new SaveLoad(this);
	_palette = new Palette(this);
	_mouse = new Mouse(this);
	_events = new Events(this);
	_kernel = new Kernel(this);
	_player = new Player(this);
	_font = new Font(this);
	if (getGameType() == GType_Burger) {
		_actor = new Actor(this);
		_interfaceView = new GameInterfaceView(this);
		_conversationView = new ConversationView(this);
	} else {
		_actor = NULL;
	}
	_rails = new Rails();	// needs to be initialized before _scene
	_scene = new Scene(this);
	_actionsView = new ActionsView(this);
	_dialogs = new Dialogs();
	_viewManager = new ViewManager(this);
	_inventory = new Inventory(this);
	_sound = new Sound(this, _mixer, 255);
	_converse = new Converse(this);
	_script = new ScriptInterpreter(this);
	_ws = new WoodScript(this);
	_animation = new Animation(this);
	//_callbacks = new Callbacks(this);
	_random = new Common::RandomSource();
	g_eventRec.registerRandomSource(*_random, "m4");

	if (isM4())
		return goM4();
	else
		return goMADS();
}

void M4Engine::eventHandler() {
	M4EventType event;
	uint32 keycode = 0;

	if ((event = _events->handleEvents()) != MEVENT_NO_EVENT) {
		if (_viewManager->containsViews())
			_viewManager->handleMouseEvents(event);
	}

	if (_events->kbdCheck(keycode))
		_viewManager->handleKeyboardEvents(keycode);
}

bool M4Engine::delay(int duration, bool keyAborts, bool clickAborts) {
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

void M4Engine::loadMenu(MenuType menuType, bool loadSaveFromHotkey, bool calledFromMainMenu) {
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

Common::Error M4Engine::goMADS() {
	_palette->setMadsSystemPalette();

	_mouse->init("cursor.ss", NULL);
	_mouse->setCursorNum(0);

	// Load MADS data files
	_globals->loadMadsVocab();			// vocab.dat
	_globals->loadMadsQuotes();			// quotes.dat
	_globals->loadMadsMessagesInfo();	// messages.dat
	_globals->loadMadsObjects();

	// Setup globals
	_vm->_globals->easyMouse = true;
	_vm->_globals->invObjectsStill = false;
	_vm->_globals->textWindowStill = false;
	_vm->_globals->storyMode = 0;

	// Test code to dump all messages to the console
	//for (int i = 0; i < _globals->getMessagesSize(); i++)
	//printf("%s\n----------\n", _globals->loadMessage(i));

	if ((getGameType() == GType_RexNebular) || (getGameType() == GType_DragonSphere)) {
		loadMenu(MAIN_MENU);

	} else {
		if (getGameType() == GType_DragonSphere) {
			_scene->loadScene(FIRST_SCENE);
		} else if (getGameType() == GType_Phantom) {
			//_scene->loadScene(FIRST_SCENE);
			_scene->loadScene(106);		// a more interesting scene
		}

		_viewManager->addView(_scene);
		_viewManager->addView(_actionsView);

		_font->setFont(FONT_MAIN_MADS);
		_font->setColors(2, 1, 3);
		_font->writeString(_scene->getBackgroundSurface(), "Testing the M4/MADS ScummVM engine", 5, 160, 310, 2);
		_font->writeString(_scene->getBackgroundSurface(), "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 5, 180, 310, 2);

		if (getGameType() == GType_DragonSphere) {
			//_scene->showMADSV2TextBox("Test", 10, 10, NULL);
		}

		_mouse->cursorOn();
	}

	_viewManager->systemHotkeys().add(Common::KEYCODE_ESCAPE, &escapeHotkeyHandler);
	_viewManager->systemHotkeys().add(Common::KEYCODE_KP_MULTIPLY, &textviewHotkeyHandler);

	// Load the general game SFX/voices
	if (getGameType() == GType_RexNebular) {
		_sound->loadDSRFile("rex009.dsr");
	} else if (getGameType() == GType_Phantom) {
		_sound->loadDSRFile("phan009.dsr");
	} else if (getGameType() == GType_DragonSphere) {
		_sound->loadDSRFile("drag009.dsr");
	}

	uint32 nextFrame = g_system->getMillis();
	while (!_events->quitFlag) {
		eventHandler();

		_animation->updateAnim();

		// Call the updateState method of all views
		_viewManager->updateState();

		if (g_system->getMillis() >= nextFrame) {

			_viewManager->refreshAll();
			nextFrame = g_system->getMillis();// + GAME_FRAME_DELAY;
		}

		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

Common::Error M4Engine::goM4() {

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
		printf("=================================\n");
		fflush(stdout);
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

		_viewManager->addView(_interfaceView);
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
			_ws->setInverseColorTable(_scene->getInverseColorTable());

			_kernel->loadSectionScriptFunctions();
			_kernel->loadRoomScriptFunctions();

			_kernel->roomInit();

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

void M4Engine::dumpFile(const char* filename, bool uncompress) {
	Common::SeekableReadStream *fileS = res()->get(filename);
	byte buffer[256];
	FILE *destFile = fopen(filename, "wb");
	int bytesRead = 0;
	printf("Dumping %s, size: %i\n", filename, fileS->size());

	if (!uncompress) {
		while (!fileS->eos()) {
			bytesRead = fileS->read(buffer, 256);
			fwrite(buffer, bytesRead, 1, destFile);
		}
	} else {
		MadsPack packData(fileS);
		Common::MemoryReadStream *sourceUnc;
		for (int i = 0; i < packData.getCount(); i++) {
			sourceUnc = packData.getItemStream(i);
			printf("Dumping compressed chunk %i of %i, size is %i\n", i + 1, packData.getCount(), sourceUnc->size());
			while (!sourceUnc->eos()) {
				bytesRead = sourceUnc->read(buffer, 256);
				fwrite(buffer, bytesRead, 1, destFile);
			}
			delete sourceUnc;
		}
	}

	fclose(destFile);
	res()->toss(filename);
	res()->purge();
}

} // End of namespace M4
