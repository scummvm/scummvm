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
#include "mohawk/dialogs.h"
#include "mohawk/video.h"
#include "mohawk/console.h"

namespace Mohawk {

Common::Rect *g_atrusJournalRect1;
Common::Rect *g_atrusJournalRect2;
Common::Rect *g_cathJournalRect2;
Common::Rect *g_atrusJournalRect3;
Common::Rect *g_cathJournalRect3;
Common::Rect *g_trapBookRect3;
Common::Rect *g_demoExitRect;

MohawkEngine_Riven::MohawkEngine_Riven(OSystem *syst, const MohawkGameDescription *gamedesc) :
		MohawkEngine(syst, gamedesc) {
	_showHotspots = false;
	_gameOver = false;
	_activatedPLST = false;
	_activatedSLST = false;
	_extrasFile = nullptr;
	_stack = nullptr;
	_gfx = nullptr;
	_sound = nullptr;
	_rnd = nullptr;
	_scriptMan = nullptr;
	_console = nullptr;
	_saveLoad = nullptr;
	_optionsDialog = nullptr;
	_card = nullptr;
	removeTimer();

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

	g_atrusJournalRect1 = new Common::Rect(295, 402, 313, 426);
	g_atrusJournalRect2 = new Common::Rect(259, 402, 278, 426);
	g_cathJournalRect2 = new Common::Rect(328, 408, 348, 419);
	g_atrusJournalRect3 = new Common::Rect(222, 402, 240, 426);
	g_cathJournalRect3 = new Common::Rect(291, 408, 311, 419);
	g_trapBookRect3 = new Common::Rect(363, 396, 386, 432);
	g_demoExitRect = new Common::Rect(291, 408, 317, 419);
}

MohawkEngine_Riven::~MohawkEngine_Riven() {
	delete _card;
	delete _stack;
	delete _sound;
	delete _gfx;
	delete _console;
	delete _extrasFile;
	delete _saveLoad;
	delete _scriptMan;
	delete _optionsDialog;
	delete _rnd;
	delete g_atrusJournalRect1;
	delete g_atrusJournalRect2;
	delete g_cathJournalRect2;
	delete g_atrusJournalRect3;
	delete g_cathJournalRect3;
	delete g_trapBookRect3;
	delete g_demoExitRect;
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
	_sound = new RivenSoundManager(this);
	_console = new RivenConsole(this);
	_saveLoad = new RivenSaveLoad(this, _saveFileMan);
	_optionsDialog = new RivenOptionsDialog(this);
	_scriptMan = new RivenScriptManager(this);

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
	_gfx->setTransitionSpeed(_vars["transitionmode"]);

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


	while (!_gameOver && !shouldQuit())
		handleEvents();

	return Common::kNoError;
}

void MohawkEngine_Riven::handleEvents() {
	// Update background running things
	checkTimer();
	_sound->updateSLST();
	_gfx->runFliesEffect();
	bool needsUpdate = _gfx->runScheduledWaterEffects();
	needsUpdate |= _video->updateMovies();

	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			_card->onMouseMove(event.mouse);

			if (!(getFeatures() & GF_DEMO)) {
				// Check to show the inventory, but it is always "showing" in the demo
				if (_eventMan->getMousePos().y >= 392)
					_gfx->showInventory();
				else
					_gfx->hideInventory();
			}

			needsUpdate = true;
			break;
		case Common::EVENT_LBUTTONDOWN:
			if (_card->getCurHotspot()) {
				checkSunnerAlertClick();
			}
			_card->onMouseDown(_eventMan->getMousePos());
			break;
		case Common::EVENT_LBUTTONUP:
			_card->onMouseUp(_eventMan->getMousePos());
			checkInventoryClick();
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
			case Common::KEYCODE_F4:
				_showHotspots = !_showHotspots;
				if (_showHotspots) {
					_card->drawHotspotRects();
					needsUpdate = true;
				} else
					refreshCard();
				break;
			case Common::KEYCODE_F5:
				runDialog(*_optionsDialog);
				if (_optionsDialog->getLoadSlot() >= 0)
					loadGameState(_optionsDialog->getLoadSlot());
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
				break;
			}
			break;
		default:
			break;
		}
	}

	_card->onMouseUpdate();

	// Update the screen if we need to
	if (needsUpdate)
		_system->updateScreen();

	// Cut down on CPU usage
	_system->delayMillis(10);
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
	_video->stopVideos();
	_video->clearMLST();

	// Clear the graphics cache; images aren't used across stack boundaries
	_gfx->clearCache();

	// Clear the old stack files out
	for (uint32 i = 0; i < _mhk.size(); i++)
		delete _mhk[i];
	_mhk.clear();

	// Get the prefix character for the destination stack
	char prefix = getStackName(n)[0];

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
		error("Could not load stack %s", getStackName(n).c_str());

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
		for (byte i = 0; i < 13; i++)
			if (_stack->getId() == rivenSpecialChange[i].startStack && dest == _stack->getCardStackId(
					rivenSpecialChange[i].startCardRMAP)) {
				changeToStack(rivenSpecialChange[i].targetStack);
				dest = _stack->getCardStackId(rivenSpecialChange[i].targetCardRMAP);
			}
	}

	if (_card) {
		_card->leave();
		delete _card;
	}
	_card = new RivenCard(this, dest);

	refreshCard(); // Handles hotspots and scripts
}

void MohawkEngine_Riven::refreshCard() {
	// Clear any timer still floating around
	removeTimer();

	_card->enter();

	if (_showHotspots)
		_card->drawHotspotRects();

	// Now we need to redraw the cursor if necessary and handle mouse over scripts
	updateCurrentHotspot();

	// Finally, install any hardcoded timer
	installCardTimer();
}

void MohawkEngine_Riven::updateCurrentHotspot() {
	_card->onMouseMove(_eventMan->getMousePos());
}

void MohawkEngine_Riven::checkInventoryClick() {
	Common::Point mousePos = _eventMan->getMousePos();

	// Don't even bother. We're not in the inventory portion of the screen.
	if (mousePos.y < 392)
		return;

	// In the demo, check if we've clicked the exit button
	if (getFeatures() & GF_DEMO) {
		if (g_demoExitRect->contains(mousePos)) {
			if (_stack->getId() == kStackAspit && _card->getId() == 1) {
				// From the main menu, go to the "quit" screen
				changeToCard(12);
			} else if (_stack->getId() == kStackAspit && _card->getId() == 12) {
				// From the "quit" screen, just quit
				_gameOver = true;
			} else {
				// Otherwise, return to the main menu
				if (_stack->getId() != kStackAspit)
					changeToStack(kStackAspit);
				changeToCard(1);
			}
		}
		return;
	}

	// No inventory shown on aspit
	if (_stack->getId() == kStackAspit)
		return;

	// Set the return stack/card id's.
	_vars["returnstackid"] = _stack->getId();
	_vars["returncardid"] = _stack->getCardGlobalId(_card->getId());

	// See RivenGraphics::showInventory() for an explanation
	// of the variables' meanings.
	bool hasCathBook = _vars["acathbook"] != 0;
	bool hasTrapBook = _vars["atrapbook"] != 0;

	// Go to the book if a hotspot contains the mouse
	if (!hasCathBook) {
		if (g_atrusJournalRect1->contains(mousePos)) {
			_gfx->hideInventory();
			changeToStack(kStackAspit);
			changeToCard(5);
		}
	} else if (!hasTrapBook) {
		if (g_atrusJournalRect2->contains(mousePos)) {
			_gfx->hideInventory();
			changeToStack(kStackAspit);
			changeToCard(5);
		} else if (g_cathJournalRect2->contains(mousePos)) {
			_gfx->hideInventory();
			changeToStack(kStackAspit);
			changeToCard(6);
		}
	} else {
		if (g_atrusJournalRect3->contains(mousePos)) {
			_gfx->hideInventory();
			changeToStack(kStackAspit);
			changeToCard(5);
		} else if (g_cathJournalRect3->contains(mousePos)) {
			_gfx->hideInventory();
			changeToStack(kStackAspit);
			changeToCard(6);
		} else if (g_trapBookRect3->contains(mousePos)) {
			_gfx->hideInventory();
			changeToStack(kStackAspit);
			changeToCard(7);
		}
	}
}

Common::SeekableReadStream *MohawkEngine_Riven::getExtrasResource(uint32 tag, uint16 id) {
	return _extrasFile->getResource(tag, id);
}

void MohawkEngine_Riven::delayAndUpdate(uint32 ms) {
	uint32 startTime = _system->getMillis();

	while (_system->getMillis() < startTime + ms && !shouldQuit()) {
		_sound->updateSLST();
		_gfx->runFliesEffect();
		bool needsUpdate = _gfx->runScheduledWaterEffects();
		needsUpdate |= _video->updateMovies();

		Common::Event event;
		while (_system->getEventManager()->pollEvent(event))
			;

		if (needsUpdate)
			_system->updateScreen();

		_system->delayMillis(10); // Ease off the CPU
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

Common::String MohawkEngine_Riven::getStackName(uint16 stack) const {
	static const char *rivenStackNames[] = {
		"<unknown>",
		"ospit",
		"pspit",
		"rspit",
		"tspit",
		"bspit",
		"gspit",
		"jspit",
		"aspit"
	};

	// Sanity check.
	assert(stack < ARRAYSIZE(rivenStackNames));

	return rivenStackNames[stack];
}

void MohawkEngine_Riven::installTimer(TimerProc proc, uint32 time) {
	removeTimer();
	_timerProc = proc;
	_timerTime = time + getTotalPlayTime();
}

void MohawkEngine_Riven::checkTimer() {
	if (!_timerProc)
		return;

	// NOTE: If the specified timer function is called, it is its job to remove the timer!
	if (getTotalPlayTime() >= _timerTime) {
		TimerProc proc = _timerProc;
		proc(this);
	}
}

void MohawkEngine_Riven::removeTimer() {
	_timerProc = 0;
	_timerTime = 0;
}

static void catherineIdleTimer(MohawkEngine_Riven *vm) {
	uint32 &cathCheck = vm->_vars["pcathcheck"];
	uint32 &cathState = vm->_vars["acathstate"];
	uint16 movie;

	// Choose a random movie based on where Catherine is
	if (cathCheck == 0) {
		static const int movieList[] = { 5, 6, 7, 8 };
		cathCheck = 1;
		movie = movieList[vm->_rnd->getRandomNumber(3)];
	} else if (cathState == 1) {
		static const int movieList[] = { 11, 14 };
		movie = movieList[vm->_rnd->getRandomBit()];
	} else {
		static const int movieList[] = { 9, 10, 12, 13 };
		movie = movieList[vm->_rnd->getRandomNumber(3)];
	}

	// Update her state if she moves from left/right or right/left, resp.
	if (movie == 5 || movie == 7 || movie == 11 || movie == 14)
		cathState = 2;
	else
		cathState = 1;

	// Play the movie, blocking
	vm->_video->activateMLST(vm->getCard()->getMovie(movie));
	vm->_cursor->hideCursor();
	vm->_video->playMovieBlockingRiven(movie);
	vm->_cursor->showCursor();
	vm->_system->updateScreen();

	// Install the next timer for the next video
	uint32 timeUntilNextMovie = vm->_rnd->getRandomNumber(120) * 1000;

	vm->_vars["pcathtime"] = timeUntilNextMovie + vm->getTotalPlayTime();

	vm->installTimer(&catherineIdleTimer, timeUntilNextMovie);
}

static void sunnersTopStairsTimer(MohawkEngine_Riven *vm) {
	// If the sunners are gone, we have no video to play
	if (vm->_vars["jsunners"] != 0) {
		vm->removeTimer();
		return;
	}

	// Play a random sunners video if the script one is not playing already
	// and then set a new timer for when the new video should be played

	VideoEntryPtr oldVideo = vm->_video->findVideoRiven(1);
	uint32 timerTime = 500;

	if (!oldVideo || oldVideo->endOfVideo()) {
		uint32 &sunnerTime = vm->_vars["jsunnertime"];

		if (sunnerTime == 0) {
			timerTime = vm->_rnd->getRandomNumberRng(2, 15) * 1000;
		} else if (sunnerTime < vm->getTotalPlayTime()) {
			VideoEntryPtr video = vm->_video->playMovieRiven(vm->_rnd->getRandomNumberRng(1, 3));

			timerTime = video->getDuration().msecs() + vm->_rnd->getRandomNumberRng(2, 15) * 1000;
		}

		sunnerTime = timerTime + vm->getTotalPlayTime();
	}

	vm->installTimer(&sunnersTopStairsTimer, timerTime);
}

static void sunnersMidStairsTimer(MohawkEngine_Riven *vm) {
	// If the sunners are gone, we have no video to play
	if (vm->_vars["jsunners"] != 0) {
		vm->removeTimer();
		return;
	}

	// Play a random sunners video if the script one is not playing already
	// and then set a new timer for when the new video should be played

	VideoEntryPtr oldVideo = vm->_video->findVideoRiven(1);
	uint32 timerTime = 500;

	if (!oldVideo || oldVideo->endOfVideo()) {
		uint32 &sunnerTime = vm->_vars["jsunnertime"];

		if (sunnerTime == 0) {
			timerTime = vm->_rnd->getRandomNumberRng(1, 10) * 1000;
		} else if (sunnerTime < vm->getTotalPlayTime()) {
			// Randomize the video
			int randValue = vm->_rnd->getRandomNumber(5);
			uint16 movie = 4;
			if (randValue == 4)
				movie = 2;
			else if (randValue == 5)
				movie = 3;

			VideoEntryPtr video = vm->_video->playMovieRiven(movie);

			timerTime = video->getDuration().msecs() + vm->_rnd->getRandomNumberRng(1, 10) * 1000;
		}

		sunnerTime = timerTime + vm->getTotalPlayTime();
	}

	vm->installTimer(&sunnersMidStairsTimer, timerTime);
}

static void sunnersLowerStairsTimer(MohawkEngine_Riven *vm) {
	// If the sunners are gone, we have no video to play
	if (vm->_vars["jsunners"] != 0) {
		vm->removeTimer();
		return;
	}

	// Play a random sunners video if the script one is not playing already
	// and then set a new timer for when the new video should be played

	VideoEntryPtr oldVideo = vm->_video->findVideoRiven(1);
	uint32 timerTime = 500;

	if (!oldVideo || oldVideo->endOfVideo()) {
		uint32 &sunnerTime = vm->_vars["jsunnertime"];

		if (sunnerTime == 0) {
			timerTime = vm->_rnd->getRandomNumberRng(1, 30) * 1000;
		} else if (sunnerTime < vm->getTotalPlayTime()) {
			VideoEntryPtr video = vm->_video->playMovieRiven(vm->_rnd->getRandomNumberRng(3, 5));

			timerTime = video->getDuration().msecs() + vm->_rnd->getRandomNumberRng(1, 30) * 1000;
		}

		sunnerTime = timerTime + vm->getTotalPlayTime();
	}

	vm->installTimer(&sunnersLowerStairsTimer, timerTime);
}

static void sunnersBeachTimer(MohawkEngine_Riven *vm) {
	// If the sunners are gone, we have no video to play
	if (vm->_vars["jsunners"] != 0) {
		vm->removeTimer();
		return;
	}

	// Play a random sunners video if the script one is not playing already
	// and then set a new timer for when the new video should be played

	VideoEntryPtr oldvideo = vm->_video->findVideoRiven(3);
	uint32 timerTime = 500;

	if (!oldvideo || oldvideo->endOfVideo()) {
		uint32 &sunnerTime = vm->_vars["jsunnertime"];

		if (sunnerTime == 0) {
			timerTime = vm->_rnd->getRandomNumberRng(1, 30) * 1000;
		} else if (sunnerTime < vm->getTotalPlayTime()) {
			// Unlike the other cards' scripts which automatically
			// activate the MLST, we have to set it manually here.
			uint16 mlstID = vm->_rnd->getRandomNumberRng(3, 8);
			vm->_video->activateMLST(vm->getCard()->getMovie(mlstID));
			VideoEntryPtr video = vm->_video->playMovieRiven(mlstID);

			timerTime = video->getDuration().msecs() + vm->_rnd->getRandomNumberRng(1, 30) * 1000;
		}

		sunnerTime = timerTime + vm->getTotalPlayTime();
	}

	vm->installTimer(&sunnersBeachTimer, timerTime);
}

void MohawkEngine_Riven::installCardTimer() {
	switch (_stack->getCurrentCardGlobalId()) {
	case 0x3a85: // Top of elevator on prison island
		// Handle Catherine hardcoded videos
		installTimer(&catherineIdleTimer, _rnd->getRandomNumberRng(1, 33) * 1000);
		break;
	case 0x77d6: // Sunners, top of stairs
		installTimer(&sunnersTopStairsTimer, 500);
		break;
	case 0x79bd: // Sunners, middle of stairs
		installTimer(&sunnersMidStairsTimer, 500);
		break;
	case 0x7beb: // Sunners, bottom of stairs
		installTimer(&sunnersLowerStairsTimer, 500);
		break;
	case 0xb6ca: // Sunners, shoreline
		installTimer(&sunnersBeachTimer, 500);
		break;
	}
}

void MohawkEngine_Riven::doVideoTimer(VideoHandle handle, bool force) {
	assert(handle);

	uint16 id = _scriptMan->getStoredMovieOpcodeID();

	if (handle != _video->findVideoRiven(id)) // Check if we've got a video match
		return;

	// Run the opcode if we can at this point
	if (force || handle->getTime() >= _scriptMan->getStoredMovieOpcodeTime())
		_scriptMan->runStoredMovieOpcode();
}

void MohawkEngine_Riven::checkSunnerAlertClick() {
	// We need to do a manual hardcoded check for the sunners'
	// alert movies.

	uint32 &sunners = _vars["jsunners"];

	// If the sunners are gone, there's nothing for us to do
	if (sunners != 0)
		return;

	uint32 rmapCode = _stack->getCurrentCardGlobalId();

	// This is only for the mid/lower staircase sections
	if (rmapCode != 0x79bd && rmapCode != 0x7beb)
		return;

	// Only set the sunners variable on the forward hotspot
	if (_card->getCurHotspot()->getBlstId() != 3)
		return;

	// If the alert video is no longer playing, we have nothing left to do
	VideoEntryPtr video = _video->findVideoRiven(1);
	if (!video || video->endOfVideo())
		return;

	sunners = 1;
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

bool ZipMode::operator== (const ZipMode &z) const {
	return z.name == name && z.id == id;
}

} // End of namespace Mohawk
