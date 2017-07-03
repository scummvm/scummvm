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
#include "gui/saveload.h"

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

	DebugMan.addDebugChannel(kRivenDebugScript, "Script", "Track Script Execution");

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
	_system->updateScreen();

	// Let's begin, shall we?
	if (getFeatures() & GF_DEMO) {
		// Start the demo off with the videos
		changeToStack(kStackAspit);
		changeToCard(6);
	} else if (ConfMan.hasKey("save_slot")) {
		// Load game from launcher/command line if requested
		int gameToLoad = ConfMan.getInt("save_slot");

		// Attempt to load the game. On failure, just send us to the main menu.
		if (_saveLoad->loadGame(gameToLoad).getCode() != Common::kNoError) {
			changeToStack(kStackAspit);
			changeToCard(1);
		}
	} else {
		// Otherwise, start us off at aspit's card 1 (the main menu)
		changeToStack(kStackAspit);
		changeToCard(1);
	}


	while (!shouldQuit())
		doFrame();

	return Common::kNoError;
}

void MohawkEngine_Riven::doFrame() {
	// Update background running things
	_sound->updateSLST();
	_video->updateMovies();

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
			_stack->keyForceUp();
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
				runDialog(*_optionsDialog);
				if (_optionsDialog->getLoadSlot() >= 0)
					loadGameState(_optionsDialog->getLoadSlot());
				_gfx->setTransitionMode((RivenTransitionMode) _vars["transitionmode"]);
				_card->initializeZipMode();
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
			default:
				_stack->onKeyPressed(event.kbd.keycode);
				break;
			}
			break;
		default:
			break;
		}
	}

	_stack->onFrame();

	if (!_scriptMan->runningQueuedScripts()) {
		// Don't run queued scripts if we are calling from a queued script
		// otherwise infinite looping will happen.
		_scriptMan->runQueuedScripts();
	}

	_inventory->onFrame();

	// Update the screen once per frame
	_system->updateScreen();

	// Cut down on CPU usage
	_system->delayMillis(10);
}

void MohawkEngine_Riven::pauseEngineIntern(bool pause) {
	MohawkEngine::pauseEngineIntern(pause);

	if (pause) {
		_video->pauseVideos();
	} else {
		_video->resumeVideos();
		_system->updateScreen();
	}
}

// Stack/Card-Related Functions

void MohawkEngine_Riven::changeToStack(uint16 n) {
	// The endings are in reverse order because of the way the 1.02 patch works.
	// The only "Data3" file is j_Data3.mhk from that patch. Patch files have higher
	// priorities over the regular files and are therefore loaded and checked first.
	static const char *endings[] = { "_Data3.mhk", "_Data2.mhk", "_Data1.mhk", "_Data.mhk", "_Sounds.mhk" };

	// Don't change stack to the current stack (if the files are loaded)
	if (_stack && _stack->getId() == n && !_mhk.empty())
		return;

	// Stop any videos playing
	_video->removeVideos();

	// Clear the graphics cache; images aren't used across stack boundaries
	_gfx->clearCache();

	// Clear the old stack files out
	for (uint32 i = 0; i < _mhk.size(); i++)
		delete _mhk[i];
	_mhk.clear();

	// Get the prefix character for the destination stack
	char prefix = RivenStacks::getName(n)[0];

	// Load any file that fits the patterns
	for (int i = 0; i < ARRAYSIZE(endings); i++) {
		Common::String filename = Common::String(prefix) + endings[i];

		MohawkArchive *mhk = new MohawkArchive();
		if (mhk->openFile(filename))
			_mhk.push_back(mhk);
		else
			delete mhk;
	}

	// Make sure we have loaded files
	if (_mhk.empty())
		error("Could not load stack %s", RivenStacks::getName(n));

	// Stop any currently playing sounds
	_sound->stopAllSLST();

	delete _stack;
	_stack = constructStackById(n);
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
	_stack->onMouseMove(_stack->getMousePosition());

	// Finally, install any hardcoded timer
	_stack->installCardTimer();
}

Common::SeekableReadStream *MohawkEngine_Riven::getExtrasResource(uint32 tag, uint16 id) {
	return _extrasFile->getResource(tag, id);
}

void MohawkEngine_Riven::delay(uint32 ms) {
	uint32 startTime = _system->getMillis();

	while (_system->getMillis() < startTime + ms && !shouldQuit()) {
		doFrame();
	}
}

void MohawkEngine_Riven::runLoadDialog() {
	GUI::SaveLoadChooser slc(_("Load game:"), _("Load"), false);

	int slot = slc.runModalWithCurrentTarget();
	if (slot >= 0)
		loadGameState(slot);
}

Common::Error MohawkEngine_Riven::loadGameState(int slot) {
	return _saveLoad->loadGame(slot);
}

Common::Error MohawkEngine_Riven::saveGameState(int slot, const Common::String &desc) {
	return _saveLoad->saveGame(slot, desc);
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
	return canLoadGameStateCurrently();
}

bool ZipMode::operator== (const ZipMode &z) const {
	return z.name == name && z.id == id;
}

} // End of namespace Mohawk
