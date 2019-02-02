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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/translation.h"
#include "common/system.h"
#include "graphics/scaler.h"
#include "gui/saveload.h"
#include "gui/message.h"

#include "mohawk/cursors.h"
#include "mohawk/installer_archive.h"
#include "mohawk/resource.h"
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_inventory.h"
#include "mohawk/riven_saveload.h"
#include "mohawk/riven_sound.h"
#include "mohawk/riven_stack.h"
#include "mohawk/riven_stacks/aspit.h"
#include "mohawk/riven_stacks/bspit.h"
#include "mohawk/riven_stacks/gspit.h"
#include "mohawk/riven_stacks/jspit.h"
#include "mohawk/riven_stacks/ospit.h"
#include "mohawk/riven_stacks/pspit.h"
#include "mohawk/riven_stacks/rspit.h"
#include "mohawk/riven_stacks/tspit.h"
#include "mohawk/riven_video.h"
#include "mohawk/dialogs.h"
#include "mohawk/console.h"

namespace Mohawk {

MohawkEngine_Riven::MohawkEngine_Riven(OSystem *syst, const MohawkGameDescription *gamedesc) :
		MohawkEngine(syst, gamedesc) {
	_showHotspots = false;
	_activatedPLST = false;
	_activatedSLST = false;
	_gameEnded = false;
	_extrasFile = nullptr;
	_stack = nullptr;
	_gfx = nullptr;
	_video = nullptr;
	_sound = nullptr;
	_rnd = nullptr;
	_scriptMan = nullptr;
	_console = nullptr;
	_saveLoad = nullptr;
	_optionsDialog = nullptr;
	_card = nullptr;
	_inventory = nullptr;
	_lastSaveTime = 0;

	_menuSavedCard = -1;
	_menuSavedStack = -1;

	DebugMan.addDebugChannel(kRivenDebugScript, "Script", "Track Script Execution");
	DebugMan.addDebugChannel(kRivenDebugPatches, "Patches", "Track Script Patching");

	// NOTE: We can never really support CD swapping. All of the music files
	// (*_Sounds.mhk) are stored on disc 1. They are copied to the hard drive
	// during install and used from there. The same goes for the extras.mhk
	// file. The following directories allow Riven to be played directly
	// from the DVD.

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "all");
	SearchMan.addSubDirectoryMatching(gameDataDir, "data");
	SearchMan.addSubDirectoryMatching(gameDataDir, "exe");
	SearchMan.addSubDirectoryMatching(gameDataDir, "assets1");
	SearchMan.addSubDirectoryMatching(gameDataDir, "program");
}

MohawkEngine_Riven::~MohawkEngine_Riven() {
	delete _card;
	delete _stack;
	delete _sound;
	delete _video;
	delete _gfx;
	delete _console;
	delete _extrasFile;
	delete _saveLoad;
	delete _scriptMan;
	delete _optionsDialog;
	delete _inventory;
	delete _rnd;

	DebugMan.clearAllDebugChannels();
}

GUI::Debugger *MohawkEngine_Riven::getDebugger() {
	return _console;
}

Common::Error MohawkEngine_Riven::run() {
	MohawkEngine::run();

	if (!_mixer->isReady()) {
		return Common::kAudioDeviceInitFailed;
	}

	// Let's try to open the installer file (it holds extras.mhk)
	// Though, we set a low priority to prefer the extracted version
	if (_installerArchive.open("arcriven.z"))
		SearchMan.add("arcriven.z", &_installerArchive, 0, false);

	_gfx = new RivenGraphics(this);
	_video = new RivenVideoManager(this);
	_sound = new RivenSoundManager(this);
	_console = new RivenConsole(this);
	_saveLoad = new RivenSaveLoad(this, _saveFileMan);
	_optionsDialog = new RivenOptionsDialog(this);
	_scriptMan = new RivenScriptManager(this);
	_inventory = new RivenInventory(this);

	_rnd = new Common::RandomSource("riven");

	// Create the cursor manager
	if (Common::File::exists("rivendmo.exe"))
		_cursor = new PECursorManager("rivendmo.exe");
	else if (Common::File::exists("riven.exe"))
		_cursor = new PECursorManager("riven.exe");
	else // last resort: try the Mac executable
		_cursor = new MacCursorManager("Riven");

	initVars();

	// Check the user has copied all the required datafiles
	if (!checkDatafiles()) {
		return Common::kNoGameDataFoundError;
	}

	// We need to have a cursor source, or the game won't work
	if (!_cursor->hasSource()) {
		Common::String message = _("You're missing a Riven executable. The Windows executable is 'riven.exe' or 'rivendmo.exe'. ");
		message += _("Using the 'arcriven.z' installer file also works. In addition, you can use the Mac 'Riven' executable.");
		GUIErrorMessage(message);
		warning("%s", message.c_str());
		return Common::kNoGameDataFoundError;
	}

	// Open extras.mhk for common images
	_extrasFile = new MohawkArchive();

	// We need extras.mhk for inventory images, marble images, and credits images
	if (!_extrasFile->openFile("extras.mhk")) {
		Common::String message = _("You're missing 'extras.mhk'. Using the 'arcriven.z' installer file also works.");
		GUIErrorMessage(message);
		warning("%s", message.c_str());
		return Common::kNoGameDataFoundError;
	}

	// Set the transition speed
	_gfx->setTransitionMode((RivenTransitionMode) _vars["transitionmode"]);

	// Start at main cursor
	_cursor->setCursor(kRivenMainCursor);
	_cursor->showCursor();

	// Let's begin, shall we?
	if (getFeatures() & GF_DEMO) {
		// Start the demo off with the videos
		changeToStack(kStackAspit);
		changeToCard(6);
	} else if (ConfMan.hasKey("save_slot")) {
		// Load game from launcher/command line if requested
		int gameToLoad = ConfMan.getInt("save_slot");

		// Attempt to load the game.
		Common::Error loadError = _saveLoad->loadGame(gameToLoad);
		if (loadError.getCode() != Common::kNoError) {
			return loadError;
		}
	} else {
		// Otherwise, start us off at aspit's card 1 (the main menu)
		changeToStack(kStackAspit);
		changeToCard(1);
	}


	while (!hasGameEnded())
		doFrame();

	return Common::kNoError;
}

void MohawkEngine_Riven::doFrame() {
	// Update background running things
	uint32 loopStart = _system->getMillis();
	_sound->updateSLST();
	_video->updateMovies();

	if (!_scriptMan->hasQueuedScripts()) {
		_stack->keyResetAction();
	}

	processInput();

	_stack->onFrame();

	if (!_scriptMan->runningQueuedScripts()) {
		// Don't run queued scripts if we are calling from a queued script
		// otherwise infinite looping will happen.
		_scriptMan->runQueuedScripts();
	}

	if (shouldPerformAutoSave(_lastSaveTime)) {
		tryAutoSaving();
	}

	_inventory->onFrame();

	// Update the screen once per frame
	_system->updateScreen();
	uint32 loopElapsed = _system->getMillis() - loopStart;

	// Cut down on CPU usage
	if (loopElapsed < 10)
		_system->delayMillis(10 - loopElapsed);
}

void MohawkEngine_Riven::processInput() {
	Common::Event event;
	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			_stack->onMouseMove(event.mouse);
			break;
		case Common::EVENT_LBUTTONDOWN:
			_stack->onMouseDown(_eventMan->getMousePos());
			break;
		case Common::EVENT_LBUTTONUP:
			_stack->onMouseUp(_eventMan->getMousePos());
			_inventory->checkClick(_eventMan->getMousePos());
			break;
		case Common::EVENT_KEYUP:
			_stack->keyResetAction();
			break;
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_d:
				if (event.kbd.flags & Common::KBD_CTRL) {
					_console->attach();
					_console->onFrame();
				}
				break;
			case Common::KEYCODE_SPACE:
				pauseGame();
				break;
			case Common::KEYCODE_F5:
				runOptionsDialog();
				break;
			case Common::KEYCODE_r:
				// Return to the main menu in the demo on ctrl+r
				if (event.kbd.flags & Common::KBD_CTRL && getFeatures() & GF_DEMO) {
					if (_stack->getId() != kStackAspit)
						changeToStack(kStackAspit);
					changeToCard(1);
				}
				break;
			case Common::KEYCODE_p:
				// Play the intro videos in the demo on ctrl+p
				if (event.kbd.flags & Common::KBD_CTRL && getFeatures() & GF_DEMO) {
					if (_stack->getId() != kStackAspit)
						changeToStack(kStackAspit);
					changeToCard(6);
				}
				break;
			case Common::KEYCODE_o:
				if (event.kbd.flags & Common::KBD_CTRL) {
					if (canLoadGameStateCurrently()) {
						runLoadDialog();
					}
				}
				break;
			case Common::KEYCODE_s:
				if (event.kbd.flags & Common::KBD_CTRL) {
					if (canSaveGameStateCurrently()) {
						runSaveDialog();
					}
				}
				break;
			case Common::KEYCODE_ESCAPE:
				if (!_scriptMan->hasQueuedScripts() && getFeatures() & GF_25TH) {
					// Check if we haven't jumped to menu
					if (_menuSavedStack == -1) {
						goToMainMenu();
					} else {
						resumeFromMainMenu();
					}
				} else {
					_stack->onKeyPressed(event.kbd);
				}
				break;
			default:
				if (event.kbdRepeat) {
					continue;
				}
				_stack->onKeyPressed(event.kbd);
				break;
			}
			break;
		case Common::EVENT_QUIT:
		case Common::EVENT_RTL:
			// Attempt to autosave before exiting
			tryAutoSaving();
			break;
		default:
			break;
		}
	}
}

void MohawkEngine_Riven::goToMainMenu() {
	if (isInMainMenu()) {
		return;
	}

	_menuSavedStack = _stack->getId();
	_menuSavedCard = _card->getId();

	_menuThumbnail.reset(new Graphics::Surface());
	createThumbnailFromScreen(_menuThumbnail.get());

	RivenCommand *go = new RivenStackChangeCommand(this, kStackAspit, 1, true, true);
	RivenScriptPtr goScript = _scriptMan->createScriptWithCommand(go);
	_scriptMan->runScript(goScript, true);
}

void MohawkEngine_Riven::resumeFromMainMenu() {
	assert(_menuSavedStack != -1);

	RivenCommand *resume = new RivenStackChangeCommand(this, _menuSavedStack, _menuSavedCard, true, true);
	RivenScriptPtr resumeScript = _scriptMan->createScriptWithCommand(resume);
	_scriptMan->runScript(resumeScript, true);

	_menuSavedStack = -1;
	_menuSavedCard = -1;
	_menuThumbnail.reset();
}

bool MohawkEngine_Riven::isInMainMenu() const {
	static const uint16 kCardIdAspitAtrusJournal = 5;
	return _stack->getId() == kStackAspit && _card->getId() < kCardIdAspitAtrusJournal;
}

bool MohawkEngine_Riven::isGameStarted() const {
	return !isInMainMenu() || _menuSavedStack != -1;
}

void MohawkEngine_Riven::pauseEngineIntern(bool pause) {
	MohawkEngine::pauseEngineIntern(pause);

	if (pause) {
		_video->pauseVideos();
	} else {
		_video->resumeVideos();

		if (_stack) {
			// The mouse may have moved while the game was paused,
			// the mouse cursor needs to be updated.
			_stack->onMouseMove(_eventMan->getMousePos());
		}
	}
}

// Stack/Card-Related Functions

void MohawkEngine_Riven::changeToStack(uint16 stackId) {
	// Don't change stack to the current stack (if the files are loaded)
	if (_stack && _stack->getId() == stackId && !_mhk.empty())
		return;

	// Free resources that may rely on the current stack data being loaded
	if (_card) {
		_card->leave();
		delete _card;
		_card = nullptr;
	}
	_video->removeVideos();
	_sound->stopAllSLST();

	// Clear the graphics cache; images aren't used across stack boundaries
	_gfx->clearCache();

	// Clear the old stack files out
	for (uint32 i = 0; i < _mhk.size(); i++)
		delete _mhk[i];
	_mhk.clear();

	// Get the prefix character for the destination stack
	char prefix = RivenStacks::getName(stackId)[0];

	// Load the localization override file if any
	if (getFeatures() & GF_LANGUAGE_FILES) {
		loadLanguageDatafile(prefix, stackId);
	}

	// Load files that start with the prefix
	const char **datafiles = listExpectedDatafiles();
	for (int i = 0; datafiles[i] != nullptr; i++) {
		if (datafiles[i][0] == prefix) {
			MohawkArchive *mhk = new MohawkArchive();
			if (mhk->openFile(datafiles[i]))
				_mhk.push_back(mhk);
			else
				delete mhk;
		}
	}

	// Make sure we have loaded files
	if (_mhk.empty())
		error("Could not load stack %s", RivenStacks::getName(stackId));

	delete _stack;
	_stack = constructStackById(stackId);

	// Set the mouse position to the correct value so the mouse
	// cursor can be computed accurately when loading a card.
	_stack->onMouseMove(getEventManager()->getMousePos());
}

const char **MohawkEngine_Riven::listExpectedDatafiles() const {
	// The files are in reverse order because of the way the 1.02 patch works.
	// The only "Data3" file is j_Data3.mhk from that patch. Patch files have higher
	// priorities over the regular files and are therefore loaded and checked first.
	static const char *datafilesDVD[] = {
			"a_Data.mhk",                  "a_Sounds.mhk",
			"b_Data.mhk",                  "b_Sounds.mhk",
			"g_Data.mhk",                  "g_Sounds.mhk",
			"j_Data2.mhk", "j_Data1.mhk",  "j_Sounds.mhk",
			"o_Data.mhk",                  "o_Sounds.mhk",
			"p_Data.mhk",                  "p_Sounds.mhk",
			"r_Data.mhk",                  "r_Sounds.mhk",
			"t_Data2.mhk", "t_Data1.mhk",  "t_Sounds.mhk",
			nullptr
	};

	static const char *datafilesCD[] = {
			"a_Data.mhk",                                "a_Sounds.mhk",
			"b_Data1.mhk", "b_Data.mhk",                 "b_Sounds.mhk",
			"g_Data.mhk",                                "g_Sounds.mhk",
			"j_Data3.mhk", "j_Data2.mhk", "j_Data1.mhk", "j_Sounds.mhk",
			"o_Data.mhk",                                "o_Sounds.mhk",
			"p_Data.mhk",                                "p_Sounds.mhk",
			"r_Data.mhk",                                "r_Sounds.mhk",
			"t_Data.mhk",                                "t_Sounds.mhk",
			nullptr
	};

	static const char *datafilesDemo[] = {
			"a_Data.mhk", "a_Sounds.mhk",
			"j_Data.mhk", "j_Sounds.mhk",
			"t_Data.mhk", "t_Sounds.mhk",
			nullptr
	};

	const char **datafiles;
	if (getFeatures() & GF_DEMO) {
		datafiles = datafilesDemo;
	} else if (getFeatures() & GF_DVD) {
		datafiles = datafilesDVD;
	} else {
		datafiles = datafilesCD;
	}
	return datafiles;
}

bool MohawkEngine_Riven::checkDatafiles() {
	Common::String missingFiles;

	const char **datafiles = listExpectedDatafiles();
	for (int i = 0; datafiles[i] != nullptr; i++) {
		if (!SearchMan.hasFile(datafiles[i])) {
			if (strcmp(datafiles[i], "j_Data3.mhk") == 0
					|| strcmp(datafiles[i], "b_Data1.mhk") == 0) {
				// j_Data3.mhk and b_Data1.mhk come from the 1.02 patch. They are not required to play.
				continue;
			}

			if (!missingFiles.empty()) {
				missingFiles += ", ";
			}
			missingFiles += datafiles[i];
		}
	}

	if (missingFiles.empty()) {
		return true;
	}

	Common::String message = _("You are missing the following required Riven data files:\n") + missingFiles;
	warning("%s", message.c_str());
	GUIErrorMessage(message);

	return false;
}

void MohawkEngine_Riven::loadLanguageDatafile(char prefix, uint16 stackId) {
	Common::String language = getDatafileLanguageName("a_data_");
	if (language.empty()) {
		return;
	}

	Common::String languageDatafile = Common::String::format("%c_data_%s.mhk", prefix, language.c_str());

	MohawkArchive *mhk = new MohawkArchive();
	if (mhk->openFile(languageDatafile)) {

		if (stackId == kStackOspit && getLanguage() != Common::EN_ANY && getLanguage() != Common::RU_RUS) {
			// WORKAROUND: The international CD versions were repacked for the 25th anniversary release
			// so they share the same resources as the English DVD version. The resource IDs for the DVD
			// version resources have a delta of 1 in their numbering when compared the the CD version
			// resources for Gehn's office. Unfortunately this delta was not compensated when repacking
			// the archives. We need to do it here at run time...
			mhk->offsetResourceIDs(ID_TBMP, 196, 1);
		} else if (stackId == kStackJspit && getLanguage() != Common::EN_ANY && getLanguage() != Common::RU_RUS) {
			// WORKAROUND: Same thing with Gehn's imager in the School in Jungle Island.
			mhk->offsetResourceIDs(ID_TMOV, 342, -2);
		} else if (stackId == kStackGspit && getLanguage() == Common::PL_POL) {
			// WORKAROUND: Same thing for the advertisement easter egg on Garden Island.
			mhk->offsetResourceIDs(ID_TMOV, 148, 2);
		}

		_mhk.push_back(mhk);
	} else {
		delete mhk;
	}
}

RivenStack *MohawkEngine_Riven::constructStackById(uint16 id) {
	switch (id) {
		case kStackAspit:
			return new RivenStacks::ASpit(this);
		case kStackBspit:
			return new RivenStacks::BSpit(this);
		case kStackGspit:
			return new RivenStacks::GSpit(this);
		case kStackJspit:
			return new RivenStacks::JSpit(this);
		case kStackOspit:
			return new RivenStacks::OSpit(this);
		case kStackPspit:
			return new RivenStacks::PSpit(this);
		case kStackRspit:
			return new RivenStacks::RSpit(this);
		case kStackTspit:
			return new RivenStacks::TSpit(this);
		default:
			error("Unknown stack id '%d'", id);
	}
}

// Riven uses some hacks to change stacks for linking books
// Otherwise, script command 27 changes stacks
struct RivenSpecialChange {
	byte startStack;
	uint32 startCardRMAP;
	byte targetStack;
	uint32 targetCardRMAP;
};

static const RivenSpecialChange rivenSpecialChange[] = {
	{ kStackAspit,  0x1f04, kStackOspit,  0x44ad }, // Trap Book
	{ kStackBspit, 0x1c0e7, kStackOspit,  0x2e76 }, // Dome Linking Book
	{ kStackGspit, 0x111b1, kStackOspit,  0x2e76 }, // Dome Linking Book
	{ kStackJspit, 0x28a18, kStackRspit,   0xf94 }, // Tay Linking Book
	{ kStackJspit, 0x26228, kStackOspit,  0x2e76 }, // Dome Linking Book
	{ kStackOspit,  0x5f0d, kStackPspit,  0x3bf0 }, // Return from 233rd Age
	{ kStackOspit,  0x470a, kStackJspit, 0x1508e }, // Return from 233rd Age
	{ kStackOspit,  0x5c52, kStackGspit, 0x10bea }, // Return from 233rd Age
	{ kStackOspit,  0x5d68, kStackBspit, 0x1adfd }, // Return from 233rd Age
	{ kStackOspit,  0x5e49, kStackTspit,   0xe87 }, // Return from 233rd Age
	{ kStackPspit,  0x4108, kStackOspit,  0x2e76 }, // Dome Linking Book
	{ kStackRspit,  0x32d8, kStackJspit, 0x1c474 }, // Return from Tay
	{ kStackTspit, 0x21b69, kStackOspit,  0x2e76 }  // Dome Linking Book
};

void MohawkEngine_Riven::changeToCard(uint16 dest) {
	debug (1, "Changing to card %d", dest);

	// Clear the graphics cache (images typically aren't used
	// on different cards).
	_gfx->clearCache();

	if (!(getFeatures() & GF_DEMO)) {
		for (byte i = 0; i < ARRAYSIZE(rivenSpecialChange); i++)
			if (_stack->getId() == rivenSpecialChange[i].startStack && dest == _stack->getCardStackId(
					rivenSpecialChange[i].startCardRMAP)) {
				changeToStack(rivenSpecialChange[i].targetStack);
				dest = _stack->getCardStackId(rivenSpecialChange[i].targetCardRMAP);
			}
	}

	// Clear any timer still floating around
	_stack->removeTimer();

	if (_card) {
		_card->leave();
		delete _card;
	}
	_card = new RivenCard(this, dest);
	_card->enter(true);

	// Now we need to redraw the cursor if necessary and handle mouse over scripts
	_stack->queueMouseCursorRefresh();

	// Finally, install any hardcoded timer
	_stack->installCardTimer();
}

Common::SeekableReadStream *MohawkEngine_Riven::getExtrasResource(uint32 tag, uint16 id) {
	return _extrasFile->getResource(tag, id);
}

Common::Array<uint16> MohawkEngine_Riven::getResourceIDList(uint32 type) const {
	Common::Array<uint16> ids;

	for (uint i = 0; i < _mhk.size(); i++) {
		ids.push_back(_mhk[i]->getResourceIDList(type));
	}

	return ids;
}


void MohawkEngine_Riven::delay(uint32 ms) {
	uint32 startTime = _system->getMillis();

	while (_system->getMillis() < startTime + ms && !hasGameEnded()) {
		doFrame();
	}
}

void MohawkEngine_Riven::startNewGame() {
	// Clear all the state data
	_menuSavedStack = -1;
	_menuSavedCard = -1;
	_menuThumbnail.reset();

	_vars.clear();
	initVars();

	_zipModeData.clear();

	setTotalPlayTime(0);
}

void MohawkEngine_Riven::runLoadDialog() {
	GUI::SaveLoadChooser slc(_("Load game:"), _("Load"), false);

	pauseEngine(true);
	int slot = slc.runModalWithCurrentTarget();
	pauseEngine(false);

	if (slot >= 0) {
		loadGameStateAndDisplayError(slot);
	}
}

void MohawkEngine_Riven::runSaveDialog() {
	GUI::SaveLoadChooser slc(_("Save game:"), _("Save"), true);

	pauseEngine(true);
	int slot = slc.runModalWithCurrentTarget();
	pauseEngine(false);

	if (slot >= 0) {
		Common::String result(slc.getResultString());
		if (result.empty()) {
			// If the user was lazy and entered no save name, come up with a default name.
			result = slc.createDefaultSaveDescription(slot);
		}

		saveGameStateAndDisplayError(slot, result);
	}
}

Common::Error MohawkEngine_Riven::loadGameState(int slot) {
	Common::Error loadError = _saveLoad->loadGame(slot);

	if (loadError.getCode() == Common::kNoError) {
		_menuSavedStack = -1;
		_menuSavedCard = -1;
		_menuThumbnail.reset();
	}

	return loadError;
}

void MohawkEngine_Riven::loadGameStateAndDisplayError(int slot) {
	assert(slot >= 0);

	Common::Error loadError = loadGameState(slot);

	if (loadError.getCode() != Common::kNoError) {
		GUI::MessageDialog dialog(loadError.getDesc());
		dialog.runModal();
	}
}

Common::Error MohawkEngine_Riven::saveGameState(int slot, const Common::String &desc) {
	return saveGameState(slot, desc, false);
}

Common::Error MohawkEngine_Riven::saveGameState(int slot, const Common::String &desc, bool autosave) {
	if (_menuSavedStack != -1) {
		_vars["CurrentStackID"] = _menuSavedStack;
		_vars["CurrentCardID"] = _menuSavedCard;
	}

	const Graphics::Surface *thumbnail = _menuSavedStack != -1 ? _menuThumbnail.get() : nullptr;
	Common::Error error = _saveLoad->saveGame(slot, desc, thumbnail, autosave);

	if (_menuSavedStack != -1) {
		_vars["CurrentStackID"] = 1;
		_vars["CurrentCardID"] = 1;
	}

	return error;
}

void MohawkEngine_Riven::saveGameStateAndDisplayError(int slot, const Common::String &desc) {
	assert(slot >= 0 && !desc.empty());

	Common::Error saveError = saveGameState(slot, desc);

	if (saveError.getCode() != Common::kNoError) {
		GUI::MessageDialog dialog(saveError.getDesc());
		dialog.runModal();
	}
}

void MohawkEngine_Riven::tryAutoSaving() {
	if (!canSaveGameStateCurrently() || _gameEnded) {
		return; // Can't save right now, try again on the next frame
	}

	_lastSaveTime = _system->getMillis();

	if (!_saveLoad->isAutoSaveAllowed()) {
		return; // Can't autosave ever, try again after the next autosave delay
	}

	Common::Error saveError = saveGameState(RivenSaveLoad::kAutoSaveSlot, "Autosave", true);
	if (saveError.getCode() != Common::kNoError)
		warning("Attempt to autosave has failed.");
}


void MohawkEngine_Riven::addZipVisitedCard(uint16 cardId, uint16 cardNameId) {
	Common::String cardName = getStack()->getName(kCardNames, cardNameId);
	if (cardName.empty())
		return;
	ZipMode zip;
	zip.name = cardName;
	zip.id = cardId;
	if (Common::find(_zipModeData.begin(), _zipModeData.end(), zip) == _zipModeData.end())
		_zipModeData.push_back(zip);
}

bool MohawkEngine_Riven::isZipVisitedCard(const Common::String &hotspotName) const {
	bool foundMatch = false;

	if (!hotspotName.empty())
		for (uint16 j = 0; j < _zipModeData.size(); j++)
			if (_zipModeData[j].name == hotspotName) {
				foundMatch = true;
				break;
			}

	return foundMatch;
}

bool MohawkEngine_Riven::canLoadGameStateCurrently() {
	if (getFeatures() & GF_DEMO) {
		return false;
	}

	if (_scriptMan->hasQueuedScripts()) {
		return false;
	}

	return true;
}

bool MohawkEngine_Riven::canSaveGameStateCurrently() {
	return canLoadGameStateCurrently() && isGameStarted();
}

bool MohawkEngine_Riven::hasGameEnded() const {
	return _gameEnded || shouldQuit();
}

void MohawkEngine_Riven::setGameEnded() {
	_gameEnded = true;
}

void MohawkEngine_Riven::runOptionsDialog() {
	runDialog(*_optionsDialog);

	if (hasGameEnded()) {
		// Attempt to autosave before exiting
		tryAutoSaving();
	}

	_gfx->setTransitionMode((RivenTransitionMode) _vars["transitionmode"]);
	_card->initializeZipMode();
}

bool ZipMode::operator== (const ZipMode &z) const {
	return z.name == name && z.id == id;
}

} // End of namespace Mohawk
