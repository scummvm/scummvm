/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "common/config-manager.h"
#include "common/error.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "base/plugins.h"
#include "base/version.h"
#include "gui/saveload.h"
#include "video/qt_decoder.h"

#include "pegasus/console.h"
#include "pegasus/cursor.h"
#include "pegasus/energymonitor.h"
#include "pegasus/gamestate.h"
#include "pegasus/interface.h"
#include "pegasus/menu.h"
#include "pegasus/movie.h"
#include "pegasus/pegasus.h"
#include "pegasus/timers.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/itemlist.h"
#include "pegasus/items/biochips/aichip.h"
#include "pegasus/items/biochips/biochipitem.h"
#include "pegasus/items/biochips/opticalchip.h"
#include "pegasus/items/biochips/pegasuschip.h"
#include "pegasus/items/biochips/retscanchip.h"
#include "pegasus/items/biochips/shieldchip.h"
#include "pegasus/items/inventory/airmask.h"
#include "pegasus/items/inventory/gascanister.h"
#include "pegasus/items/inventory/inventoryitem.h"
#include "pegasus/items/inventory/keycard.h"
#include "pegasus/neighborhood/neighborhood.h"
#include "pegasus/neighborhood/prehistoric/prehistoric.h"

namespace Pegasus {

PegasusEngine::PegasusEngine(OSystem *syst, const PegasusGameDescription *gamedesc) : Engine(syst), InputHandler(0), _gameDescription(gamedesc),
		_shellNotification(kJMPDCShellNotificationID, this), _returnHotspot(kInfoReturnSpotID), _itemDragger(this), _bigInfoMovie(kNoDisplayElement),
		_smallInfoMovie(kNoDisplayElement) {
	_continuePoint = 0;
	_saveAllowed = _loadAllowed = true;
	_gameMenu = 0;
	_deathReason = kDeathStranded;
	_neighborhood = 0;
	_FXLevel = 0x80;
	_ambientLevel = 0x80;
	_gameMode = kNoMode;
	_switchModesSync = false;
	_draggingItem = 0;
	_dragType = kDragNoDrag;
}

PegasusEngine::~PegasusEngine() {
	delete _resFork;
	delete _console;
	delete _cursor;
	delete _continuePoint;
	delete _gameMenu;
	delete _neighborhood;

	// NOTE: This must be deleted last!
	delete _gfx;
}

Common::Error PegasusEngine::run() {
	_console = new PegasusConsole(this);
	_gfx = new GraphicsManager(this);
	_resFork = new Common::MacResManager();
	_cursor = new Cursor();
	
	if (!_resFork->open("JMP PP Resources") || !_resFork->hasResFork())
		error("Could not load JMP PP Resources");

	// Initialize items
	createItems();

	// Initialize cursors
	_cursor->addCursorFrames(0x80); // Main
	_cursor->addCursorFrames(900);  // Mars Shuttle

	// Initialize the item dragger bounds
	_itemDragger.setHighlightBounds();

	if (!isDemo() && !detectOpeningClosingDirectory()) {
		Common::String message = "Missing intro directory. ";

		// Give Mac OS X a more specific message because we can
#ifdef MACOSX
		message += "Make sure \"Opening/Closing\" is present.";
#else
		message += "Be sure to rename \"Opening/Closing\" to \"Opening_Closing\".";
#endif

		GUIErrorMessage(message);
		warning("%s", message.c_str());
		return Common::kNoGameDataFoundError;
	}

	// Set up input
	InputHandler::setInputHandler(this);
	allowInput(true);

	// Set up inventories
	_items.setWeightLimit(9);
	_items.setOwnerID(kPlayerID);
	_biochips.setWeightLimit(8);
	_biochips.setOwnerID(kPlayerID);

	// Start up the first notification
	_shellNotification.notifyMe(this, kJMPShellNotificationFlags, kJMPShellNotificationFlags);
	_shellNotification.setNotificationFlags(kGameStartingFlag, kGameStartingFlag);

	_returnHotspot.setArea(Common::Rect(kNavAreaLeft, kNavAreaTop, 512 + kNavAreaLeft, 256 + kNavAreaTop));
	_returnHotspot.setHotspotFlags(kInfoReturnSpotFlag);
	g_allHotspots.push_back(&_returnHotspot);

	_screenDimmer.setBounds(Common::Rect(0, 0, 640, 480));
	_screenDimmer.setDisplayOrder(kScreenDimmerOrder);

	while (!shouldQuit())
		processShell();

	return Common::kNoError;
}

bool PegasusEngine::detectOpeningClosingDirectory() {
	// We need to detect what our Opening/Closing directory is listed as
	// On the original disc, it was 'Opening/Closing' but only HFS(+) supports the slash
	// Mac OS X will display this as 'Opening:Closing' and we can use that directly
	// On other systems, users will need to rename to "Opening_Closing"

	Common::FSNode gameDataDir(ConfMan.get("path"));
	gameDataDir = gameDataDir.getChild("Images");

	if (!gameDataDir.exists())
		return false;

	Common::FSList fsList;
	if (!gameDataDir.getChildren(fsList, Common::FSNode::kListDirectoriesOnly, true))
		return false;

	for (uint i = 0; i < fsList.size() && _introDirectory.empty(); i++) {
		Common::String name = fsList[i].getName();

		if (name.equalsIgnoreCase("Opening:Closing"))
			_introDirectory = name;
		else if (name.equalsIgnoreCase("Opening_Closing"))
			_introDirectory = name;
	}

	if (_introDirectory.empty())
		return false;

	debug(0, "Detected intro location as '%s'", _introDirectory.c_str());
	_introDirectory = Common::String("Images/") + _introDirectory;
	return true;
}

void PegasusEngine::createItems() {
	Common::SeekableReadStream *res = _resFork->getResource(MKTAG('N', 'I', 't', 'm'), 0x80);

	uint16 entryCount = res->readUint16BE();

	for (uint16 i = 0; i < entryCount; i++) {
		tItemID itemID = res->readUint16BE();
		tNeighborhoodID neighborhoodID = res->readUint16BE();
		tRoomID roomID = res->readUint16BE();
		tDirectionConstant direction = res->readByte();
		res->readByte(); // alignment

		createItem(itemID, neighborhoodID, roomID, direction);
	}

	delete res;
}

void PegasusEngine::createItem(tItemID itemID, tNeighborhoodID neighborhoodID, tRoomID roomID, tDirectionConstant direction) {
	switch (itemID) {
	case kInterfaceBiochip:
		// Unused in game, but still in the data and we need to create
		// it because it's saved/loaded from save files.
		new BiochipItem(itemID, neighborhoodID, roomID, direction);
		break;
	case kAIBiochip:
		new AIChip(itemID, neighborhoodID, roomID, direction);
		break;
	case kPegasusBiochip:
		new PegasusChip(itemID, neighborhoodID, roomID, direction);
		break;
	case kOpticalBiochip:
		new OpticalChip(itemID, neighborhoodID, roomID, direction);
		break;
	case kMapBiochip:
		// TODO: Implement this biochip
		new BiochipItem(itemID, neighborhoodID, roomID, direction);
		break;
	case kRetinalScanBiochip:
		new RetScanChip(itemID, neighborhoodID, roomID, direction);
		break;
	case kShieldBiochip:
		new ShieldChip(itemID, neighborhoodID, roomID, direction);
		break;		
	case kAirMask:
		new AirMask(itemID, neighborhoodID, roomID, direction);
		break;
	case kKeyCard:
		new KeyCard(itemID, neighborhoodID, roomID, direction);
		break;
	case kGasCanister:
		new GasCanister(itemID, neighborhoodID, roomID, direction);
		break;
	default:
		// Everything else is a normal inventory item
		new InventoryItem(itemID, neighborhoodID, roomID, direction);
		break;
	}
}

void PegasusEngine::runIntro() {
	bool skipped = false;

	Video::SeekableVideoDecoder *video = new Video::QuickTimeDecoder();
	if (video->loadFile(_introDirectory + "/BandaiLogo.movie")) {
		while (!shouldQuit() && !video->endOfVideo() && !skipped) {
			if (video->needsUpdate()) {
				const Graphics::Surface *frame = video->decodeNextFrame();
				_system->copyRectToScreen((byte *)frame->pixels, frame->pitch, 0, 0, frame->w, frame->h);
				_system->updateScreen();
			}

			Input input;
			InputHandler::getCurrentInputDevice()->getInput(input, kFilterAllInput);
			if (input.anyInput())
				skipped = true;

			_system->delayMillis(10);
		}
	}

	delete video;

	if (shouldQuit() || skipped)
		return;

	video = new Video::QuickTimeDecoder();

	if (!video->loadFile(_introDirectory + "/Big Movie.movie"))
		error("Could not load intro movie");

	video->seekToTime(Audio::Timestamp(0, 10 * 600, 600));

	playMovieScaled(video, 0, 0);

	delete video;
}

void PegasusEngine::showLoadDialog() {
	GUI::SaveLoadChooser slc(_("Load game:"), _("Load"));
	slc.setSaveMode(false);

	Common::String gameId = ConfMan.get("gameid");

	const EnginePlugin *plugin = 0;
	EngineMan.findGame(gameId, &plugin);

	int slot = slc.runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());

	if (slot >= 0) {
		warning("TODO: Load game");
	}

	slc.close();
}

GUI::Debugger *PegasusEngine::getDebugger() {
	return _console;
}

void PegasusEngine::addIdler(Idler *idler) {
	_idlers.push_back(idler);
}

void PegasusEngine::removeIdler(Idler *idler) {
	_idlers.remove(idler);
}

void PegasusEngine::giveIdleTime() {
	for (Common::List<Idler *>::iterator it = _idlers.begin(); it != _idlers.end(); it++)
		(*it)->useIdleTime();
}

void PegasusEngine::addTimeBase(TimeBase *timeBase) {
	_timeBases.push_back(timeBase);
}

void PegasusEngine::removeTimeBase(TimeBase *timeBase) {
	_timeBases.remove(timeBase);
}

bool PegasusEngine::loadFromStream(Common::ReadStream *stream) {
	// Dispose currently running stuff
	useMenu(0);
	useNeighborhood(0);
	removeAllItemsFromInventory();
	removeAllItemsFromBiochips();
	_currentItemID = kNoItemID;
	_currentBiochipID = kNoItemID;

	// Signature
	uint32 creator = stream->readUint32BE();
	if (creator != kPegasusPrimeCreator) {
		warning("Bad save creator '%s'", tag2str(creator));
		return false;
	}

	uint32 gameType = stream->readUint32BE();
	int saveType;

	switch (gameType) {
	case kPegasusPrimeDisk1GameType:
	case kPegasusPrimeDisk2GameType:
	case kPegasusPrimeDisk3GameType:
	case kPegasusPrimeDisk4GameType:
		saveType = kNormalSave;
		break;
	case kPegasusPrimeContinueType:
		saveType = kContinueSave;
		break;
	default:
		// There are five other possible game types on the Pippin
		// version, but hopefully we don't see any of those here
		warning("Unhandled pegasus game type '%s'", tag2str(gameType));
		return false;
	}

	uint32 version = stream->readUint32BE();
	if (version != kPegasusPrimeVersion) {
		warning("Where did you get this save? It's a beta (v%04x)!", version & 0x7fff);
		return false;
	}

	// Game State
	GameState.readGameState(stream);

	// Energy
	setLastEnergyValue(stream->readUint32BE() >> 16);

	// Death reason
	setEnergyDeathReason(stream->readByte());

	// TODO: This is as far as we can go right now (until I implement the mapping biochip and AI rules loading)
	return true;

	// Items
	g_allItems.readFromStream(stream);

	// Inventory
	uint32 itemCount = stream->readUint32BE();

	if (itemCount > 0) {
		for (uint32 i = 0; i < itemCount; i++) {
			InventoryItem *inv = (InventoryItem *)g_allItems.findItemByID((tItemID)stream->readUint16BE());
			addItemToInventory(inv);
		}

		g_interface->setCurrentInventoryItemID((tItemID)stream->readUint16BE());
	}

	// Biochips
	uint32 biochipCount = stream->readUint32BE();

	if (biochipCount > 0) {
		for (uint32 i = 0; i < biochipCount; i++) {
			BiochipItem *biochip = (BiochipItem *)g_allItems.findItemByID((tItemID)stream->readUint16BE());
			addItemToBiochips(biochip);
		}

		g_interface->setCurrentBiochipID((tItemID)stream->readUint16BE());
	}


	// TODO: Disc check

	// Jump to environment
	jumpToNewEnvironment(GameState.getCurrentNeighborhood(), GameState.getCurrentRoom(), GameState.getCurrentDirection());
	_shellNotification.setNotificationFlags(0, kNeedNewJumpFlag);
	performJump(GameState.getCurrentNeighborhood());

	// AI rules
	if (g_AIArea)
		g_AIArea->readAIRules(stream);

	startNeighborhood();

	// Make a new continue point if this isn't already one
	if (saveType == kNormalSave)
		makeContinuePoint();

	return true;
}

bool PegasusEngine::writeToStream(Common::WriteStream *stream, int saveType) {
	// Not ready yet! :P
	return false;

	// Signature
	stream->writeUint32BE(kPegasusPrimeCreator);

	if (saveType == kNormalSave) {
		// TODO: Disc check
		stream->writeUint32BE(kPegasusPrimeDisk1GameType);
	} else { // Continue
		stream->writeUint32BE(kPegasusPrimeContinueType);
	}

	stream->writeUint32BE(kPegasusPrimeVersion);

	// Game State
	GameState.writeGameState(stream);

	// Energy
	stream->writeUint32BE(getSavedEnergyValue() << 16);

	// Death reason
	stream->writeByte(getEnergyDeathReason());

	// Items
	g_allItems.writeToStream(stream);

	// Inventory
	uint32 itemCount = _items.getNumItems();
	stream->writeUint32BE(itemCount);

	if (itemCount > 0) {
		for (uint32 i = 0; i < itemCount; i++)
			stream->writeUint16BE(_items.getItemIDAt(i));

		stream->writeUint16BE(g_interface->getCurrentInventoryItem()->getObjectID());
	}

	// Biochips
	uint32 biochipCount = _biochips.getNumItems();
	stream->writeUint32BE(biochipCount);

	if (itemCount > 0) {
		for (uint32 i = 0; i < biochipCount; i++)
			stream->writeUint16BE(_biochips.getItemIDAt(i));

		stream->writeUint16BE(g_interface->getCurrentBiochip()->getObjectID());
	}

	// AI rules
	if (g_AIArea)
		g_AIArea->writeAIRules(stream);

	return true;
}

void PegasusEngine::makeContinuePoint() {
	delete _continuePoint;

	Common::MemoryWriteStreamDynamic newPoint(DisposeAfterUse::NO);
	writeToStream(&newPoint, kContinueSave);
	_continuePoint = new Common::MemoryReadStream(newPoint.getData(), newPoint.size(), DisposeAfterUse::YES);
}

void PegasusEngine::loadFromContinuePoint() {
	// Failure to load a continue point is fatal

	if (!_continuePoint)
		error("Attempting to load from non-existant continue point");

	if (!loadFromStream(_continuePoint))
		error("Failed loading continue point");
}

Common::Error PegasusEngine::loadGameState(int slot) {
	Common::StringArray filenames = _saveFileMan->listSavefiles("pegasus-*.sav");
	Common::InSaveFile *loadFile = _saveFileMan->openForLoading(filenames[slot]);
	if (!loadFile)
		return Common::kUnknownError;

	bool valid = loadFromStream(loadFile);
	delete loadFile;

	return valid ? Common::kNoError : Common::kUnknownError;
}

Common::Error PegasusEngine::saveGameState(int slot, const Common::String &desc) {
	Common::String output = Common::String::format("pegasus-%s.sav", desc.c_str());
	Common::OutSaveFile *saveFile = _saveFileMan->openForSaving(output);
	if (!saveFile)
		return Common::kUnknownError;

	bool valid = writeToStream(saveFile, kNormalSave);
	delete saveFile;

	return valid ? Common::kNoError : Common::kUnknownError;
}

void PegasusEngine::receiveNotification(Notification *notification, const tNotificationFlags flags) {
	if (&_shellNotification == notification) {
		switch (flags) {
		case kGameStartingFlag: {
#if 0
			// This is just some graphical test that I wrote; I'll
			// keep it around for reference.
			Movie opening(1);
			opening.initFromMovieFile(_introDirectory + "/Big Movie.movie");
			opening.setTime(10, 1);
			opening.setStart(10, 1);
			opening.startDisplaying();
			opening.show();
			opening.start();
			opening.setFlags(kLoopTimeBase);

			Input input;
			InputHandler::getCurrentInputDevice()->getInput(input, kFilterAllInput);

			while (opening.isRunning() && !shouldQuit()) {
				checkCallBacks();
				_gfx->updateDisplay();

				InputHandler::getCurrentInputDevice()->getInput(input, kFilterAllInput);
				if (input.anyInput())
					break;

				_system->delayMillis(10);
			}
#else
			if (!isDemo())
				runIntro();
			else
				showTempScreen("Images/Demo/NGsplashScrn.pict");

			if (shouldQuit())
				return;

			useMenu(new MainMenu());
			_gfx->invalRect(Common::Rect(0, 0, 640, 480));
			_gfx->updateDisplay();
			((MainMenu *)_gameMenu)->startMainMenuLoop();
#endif
			break;
		}
		case kPlayerDiedFlag:
			doDeath();
			break;
		case kNeedNewJumpFlag:
			performJump(GameState.getNextNeighborhood());
			startNeighborhood();
			break;
		default:
			break;
		}
	}
}

void PegasusEngine::checkCallBacks() {
	for (Common::List<TimeBase *>::iterator it = _timeBases.begin(); it != _timeBases.end(); it++)
		(*it)->checkCallBacks();
}

void PegasusEngine::resetIntroTimer() {
	// TODO
}

void PegasusEngine::delayShell(TimeValue time, TimeScale scale) {
	if (time == 0 || scale == 0)
		return;

	uint32 startTime = g_system->getMillis();
	uint32 timeInMillis = time * 1000 / scale;

	while (g_system->getMillis() < startTime + timeInMillis) {
		checkCallBacks();
		_gfx->updateDisplay();
	}
}

void PegasusEngine::useMenu(GameMenu *newMenu) {
	if (_gameMenu) {
		_gameMenu->restorePreviousHandler();
		delete _gameMenu;
	}

	_gameMenu = newMenu;

	if (_gameMenu)
		_gameMenu->becomeCurrentHandler();
}

bool PegasusEngine::checkGameMenu() {
	tGameMenuCommand command = kMenuCmdNoCommand;

	if (_gameMenu) {
		command = _gameMenu->getLastCommand();
		if (command != kMenuCmdNoCommand) {
			_gameMenu->clearLastCommand();
			doGameMenuCommand(command);
		}
	}

	return command != kMenuCmdNoCommand;
}

void PegasusEngine::doGameMenuCommand(const tGameMenuCommand command) {
	switch (command) {
	case kMenuCmdStartAdventure:
		GameState.setWalkthroughMode(false);

		// Only start the game in the demo for now
		// (until it works and I implement Caldoria)
		if (isDemo())
			startNewGame();
		else
			error("Start new game (adventure mode)");
		break;
	case kMenuCmdCredits:
		if (isDemo()) {
			showTempScreen("Images/Demo/DemoCredits.pict");
			_gfx->doFadeOutSync();
			_gfx->updateDisplay();
			_gfx->doFadeInSync();
		} else {
			// TODO: Stop intro timer
			_gfx->doFadeOutSync();
			useMenu(new CreditsMenu());
			_gfx->updateDisplay();
			_gfx->doFadeInSync();
		}
		break;
	case kMenuCmdQuit:
	case kMenuCmdDeathQuitDemo:
		if (isDemo())
			showTempScreen("Images/Demo/NGquitScrn.pict");
		_system->quit();
		break;
	case kMenuCmdOverview:
		// TODO: Stop intro timer
		doInterfaceOverview();
		resetIntroTimer();
		break;
	case kMenuCmdStartWalkthrough:
		GameState.setWalkthroughMode(true);
		error("Start new game (walkthrough mode)");
		break;
	case kMenuCmdRestore:
	case kMenuCmdDeathRestore:
		error("Load game");
		break;
	case kMenuCmdCreditsMainMenu:
		_gfx->doFadeOutSync();
		useMenu(new MainMenu());
		_gfx->updateDisplay();
		((MainMenu *)_gameMenu)->startMainMenuLoop();
		_gfx->doFadeInSync();
		resetIntroTimer();
		break;
	case kMenuCmdDeathContinue:
		if (((DeathMenu *)_gameMenu)->playerWon()) {
			if (isDemo()) {
				showTempScreen("Images/Demo/DemoCredits.pict");
				_gfx->doFadeOutSync();
				_gfx->updateDisplay();
				_gfx->doFadeInSync();
			} else {
				_gfx->doFadeOutSync();
				useMenu(0);
				_gfx->clearScreen();
				_gfx->updateDisplay();

				Video::SeekableVideoDecoder *video = new Video::QuickTimeDecoder();
				if (!video->loadFile(_introDirectory + "/Closing.movie"))
					error("Could not load closing movie");

				uint16 x = (640 - video->getWidth() * 2) / 2;
				uint16 y = (480 - video->getHeight() * 2) / 2;

				playMovieScaled(video, x, y);

				delete video;

				if (shouldQuit())
					return;

				useMenu(new MainMenu());
				_gfx->updateDisplay();
				((MainMenu *)_gameMenu)->startMainMenuLoop();
				_gfx->doFadeInSync();
				resetIntroTimer();
			}
		} else {
			loadFromContinuePoint();
		}
		break;
	case kMenuCmdDeathMainMenuDemo:
	case kMenuCmdDeathMainMenu:
		_gfx->doFadeOutSync();
		useMenu(new MainMenu());
		_gfx->updateDisplay();
		((MainMenu *)_gameMenu)->startMainMenuLoop();
		_gfx->doFadeInSync();
		if (!isDemo())
			resetIntroTimer();
		break;
	case kMenuCmdPauseSave:
		error("Save game");
		break;
	case kMenuCmdPauseContinue:
		pauseMenu(false);
		break;
	case kMenuCmdPauseRestore:
		error("Load game");
		break;
	case kMenuCmdPauseQuit:
		_gfx->doFadeOutSync();
		throwAwayEverything();
		pauseMenu(false);
		useMenu(new MainMenu());
		_gfx->updateDisplay();
		((MainMenu *)_gameMenu)->startMainMenuLoop();
		_gfx->doFadeInSync();
		if (!isDemo())
			resetIntroTimer();
		break;
	case kMenuCmdNoCommand:
		break;
	default:
		error("Unknown menu command %d", command);
	}
}

void PegasusEngine::handleInput(const Input &input, const Hotspot *cursorSpot) {
	if (!checkGameMenu())
		shellGameInput(input, cursorSpot);

	// Handle the console here
	if (input.isConsoleRequested()) {
		_console->attach();
		_console->onFrame();
	}

	// TODO: Save request
	// TODO: Load request
}

void PegasusEngine::doInterfaceOverview() {
	static const short kNumOverviewSpots = 11;
	static const Common::Rect overviewSpots[kNumOverviewSpots] = {
		Common::Rect(354, 318, 354 + 204, 318 + 12),
		Common::Rect(211, 34, 211 + 114, 34 + 28),
		Common::Rect(502, 344, 502 + 138, 344 + 120),
		Common::Rect(132, 40, 132 + 79, 40 + 22),
		Common::Rect(325, 40, 332 + 115, 40 + 22),
		Common::Rect(70, 318, 70 + 284, 318 + 12),
		Common::Rect(76, 334, 76 + 96, 334 + 96),
		Common::Rect(64, 64, 64 + 512, 64 + 256),
		Common::Rect(364, 334, 364 + 96, 334 + 96),
		Common::Rect(172, 334, 172 + 192, 334 + 96),
		Common::Rect(542, 36, 542 + 58, 36 + 20)
	};

	_gfx->doFadeOutSync();
	useMenu(0);

	Picture leftBackground(kNoDisplayElement);
	leftBackground.initFromPICTFile("Images/Interface/OVLeft.mac");
	leftBackground.setDisplayOrder(0);
	leftBackground.moveElementTo(kBackground1Left, kBackground1Top);
	leftBackground.startDisplaying();
	leftBackground.show();

	Picture topBackground(kNoDisplayElement);
	topBackground.initFromPICTFile("Images/Interface/OVTop.mac");
	topBackground.setDisplayOrder(0);
	topBackground.moveElementTo(kBackground2Left, kBackground2Top);
	topBackground.startDisplaying();
	topBackground.show();

	Picture rightBackground(kNoDisplayElement);
	rightBackground.initFromPICTFile("Images/Interface/OVRight.mac");
	rightBackground.setDisplayOrder(0);
	rightBackground.moveElementTo(kBackground3Left, kBackground3Top);
	rightBackground.startDisplaying();
	rightBackground.show();

	Picture bottomBackground(kNoDisplayElement);
	bottomBackground.initFromPICTFile("Images/Interface/OVBottom.mac");
	bottomBackground.setDisplayOrder(0);
	bottomBackground.moveElementTo(kBackground4Left, kBackground4Top);
	bottomBackground.startDisplaying();
	bottomBackground.show();

	Picture controllerHighlight(kNoDisplayElement);
	controllerHighlight.initFromPICTFile("Images/Interface/OVcontrollerHilite.mac");
	controllerHighlight.setDisplayOrder(0);
	controllerHighlight.moveElementTo(kOverviewControllerLeft, kOverviewControllerTop);
	controllerHighlight.startDisplaying();

	Movie overviewText(kNoDisplayElement);
	overviewText.initFromMovieFile("Images/Interface/Overview Mac.movie");
	overviewText.setDisplayOrder(0);
	overviewText.moveElementTo(kNavAreaLeft, kNavAreaTop);
	overviewText.startDisplaying();
	overviewText.show();
	overviewText.redrawMovieWorld();

	DropHighlight highlight(kNoDisplayElement);
	highlight.setDisplayOrder(1);
	highlight.startDisplaying();
	highlight.setHighlightThickness(4);
	highlight.setHighlightColor(g_system->getScreenFormat().RGBToColor(239, 239, 0));
	highlight.setHighlightCornerDiameter(8);

	Input input;
	InputHandler::getCurrentInputDevice()->getInput(input, kFilterAllInput);

	Common::Point cursorLoc;
	input.getInputLocation(cursorLoc);

	uint16 i;
	for (i = 0; i < kNumOverviewSpots; ++i)
		if (overviewSpots[i].contains(cursorLoc))
			break;

	TimeValue time;
	if (i == kNumOverviewSpots)
		time = 5;
	else if (i > 4)
		time = i + 1;
	else
		time = i;

	if (time == 2) {
		highlight.hide();
		controllerHighlight.show();
	} else if (i != kNumOverviewSpots) {
		controllerHighlight.hide();
		Common::Rect r = overviewSpots[i];
		r.grow(5);
		highlight.setBounds(r);
		highlight.show();
	} else {
		highlight.hide();
		controllerHighlight.hide();
	}

	overviewText.setTime(time * 3 + 2, 15);
	overviewText.redrawMovieWorld();

	_cursor->setCurrentFrameIndex(3);
	_cursor->show();

	_gfx->updateDisplay();
	_gfx->doFadeInSync();

	for (;;) {
		InputHandler::getCurrentInputDevice()->getInput(input, kFilterAllInput);

		if (input.anyInput() || shouldQuit()) // TODO: Check for save/load requests too
			break;

		input.getInputLocation(cursorLoc);
		for (i = 0; i < kNumOverviewSpots; ++i)
			if (overviewSpots[i].contains(cursorLoc))
				break;

		if (i == kNumOverviewSpots)
			time = 5;
		else if (i > 4)
			time = i + 1;
		else
			time = i;

		if (time == 2) {
			highlight.hide();
			controllerHighlight.show();
		} else if (i != kNumOverviewSpots) {
			controllerHighlight.hide();
			Common::Rect r = overviewSpots[i];
			r.grow(5);
			highlight.setBounds(r);
			highlight.show();
		} else {
			highlight.hide();
			controllerHighlight.hide();
		}

		overviewText.setTime(time * 3 + 2, 15);
		overviewText.redrawMovieWorld();

		refreshDisplay();
	}

	if (shouldQuit())
		return;

	highlight.hide();
	_cursor->hide();

	_gfx->doFadeOutSync();
	useMenu(new MainMenu());
	_gfx->updateDisplay();
	((MainMenu *)_gameMenu)->startMainMenuLoop();
	_gfx->doFadeInSync();

	// TODO: Cancel save/load requests?
}

void PegasusEngine::showTempScreen(const Common::String &fileName) {
	_gfx->doFadeOutSync();

	Picture picture(0);
	picture.initFromPICTFile(fileName);
	picture.setDisplayOrder(kMaxAvailableOrder);
	picture.startDisplaying();
	picture.show();
	_gfx->updateDisplay();

	_gfx->doFadeInSync();

	// Wait for the next event
	bool done = false;
	while (!shouldQuit() && !done) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONUP:
			case Common::EVENT_RBUTTONUP:
			case Common::EVENT_KEYDOWN:
				done = true;
				break;
			default:
				break;
			}
		}

		_system->delayMillis(10);
	}
}

void PegasusEngine::refreshDisplay() {
	giveIdleTime();
	_gfx->updateDisplay();
}

void PegasusEngine::resetEnergyDeathReason() {
	switch (getCurrentNeighborhoodID()) {
	case kMarsID:
		_deathReason = kDeathArrestedInMars;
		break;
	case kNoradAlphaID:
	case kNoradDeltaID:
		_deathReason = kDeathArrestedInNorad;
		break;
	case kWSCID:
		_deathReason = kDeathArrestedInWSC;
		break;
	default:
		_deathReason = kDeathStranded;
		break;
	}
}

bool PegasusEngine::playerHasItem(const Item *item) {
	return playerHasItemID(item->getObjectID());
}

bool PegasusEngine::playerHasItemID(const tItemID itemID) {
	return itemInInventory(itemID) || itemInBiochips(itemID);
}

InventoryItem *PegasusEngine::getCurrentInventoryItem() {
	if (g_interface)
		return g_interface->getCurrentInventoryItem();

	return 0;
}

bool PegasusEngine::itemInInventory(InventoryItem *item) {
	return _items.itemInInventory(item);
}

bool PegasusEngine::itemInInventory(tItemID id) {
	return _items.itemInInventory(id);
}

BiochipItem *PegasusEngine::getCurrentBiochip() {
	if (g_interface)
		return g_interface->getCurrentBiochip();

	return 0;
}

bool PegasusEngine::itemInBiochips(BiochipItem *item) {
	return _biochips.itemInInventory(item);
}

bool PegasusEngine::itemInBiochips(tItemID id) {
	return _biochips.itemInInventory(id);
}

bool PegasusEngine::playerAlive() {
	return (_shellNotification.getNotificationFlags() & kPlayerDiedFlag) == 0;
}

Common::String PegasusEngine::getBriefingMovie() {
	if (_neighborhood)
		return _neighborhood->getBriefingMovie();

	return Common::String();
}

Common::String PegasusEngine::getEnvScanMovie() {
	if (_neighborhood)
		return _neighborhood->getEnvScanMovie();

	return Common::String();
}

uint PegasusEngine::getNumHints() {
	if (_neighborhood)
		return _neighborhood->getNumHints();

	return 0;
}

Common::String PegasusEngine::getHintMovie(uint hintNum) {
	if (_neighborhood)
		return _neighborhood->getHintMovie(hintNum);

	return Common::String();
}

bool PegasusEngine::canSolve() {
	if (_neighborhood)
		return _neighborhood->canSolve();

	return false;
}

void PegasusEngine::prepareForAIHint(const Common::String &movieName) {
	if (g_neighborhood)
		g_neighborhood->prepareForAIHint(movieName);
}

void PegasusEngine::cleanUpAfterAIHint(const Common::String &movieName) {
	if (g_neighborhood)
		g_neighborhood->cleanUpAfterAIHint(movieName);
}

void PegasusEngine::jumpToNewEnvironment(const tNeighborhoodID neighborhoodID, const tRoomID roomID, const tDirectionConstant direction) {
	GameState.setNextLocation(neighborhoodID, roomID, direction);
	_shellNotification.setNotificationFlags(kNeedNewJumpFlag, kNeedNewJumpFlag);
}

void PegasusEngine::checkFlashlight() {
	if (_neighborhood)
		_neighborhood->checkFlashlight();
}

bool PegasusEngine::playMovieScaled(Video::SeekableVideoDecoder *video, uint16 x, uint16 y) {
	bool skipped = false;

	while (!shouldQuit() && !video->endOfVideo() && !skipped) {
		if (video->needsUpdate()) {
			const Graphics::Surface *frame = video->decodeNextFrame();

			// Scale up the frame doing some simple scaling
			Graphics::Surface scaledFrame;
			scaledFrame.create(frame->w * 2, frame->h * 2, frame->format);
			const byte *src = (const byte *)frame->pixels;
			byte *dst1 = (byte *)scaledFrame.pixels;
			byte *dst2 = (byte *)scaledFrame.pixels + scaledFrame.pitch;

			for (int i = 0; i < frame->h; i++) {
				for (int j = 0; j < frame->w; j++) {
					memcpy(dst1, src, frame->format.bytesPerPixel);
					dst1 += frame->format.bytesPerPixel;
					memcpy(dst1, src, frame->format.bytesPerPixel);
					dst1 += frame->format.bytesPerPixel;
					memcpy(dst2, src, frame->format.bytesPerPixel);
					dst2 += frame->format.bytesPerPixel;
					memcpy(dst2, src, frame->format.bytesPerPixel);
					dst2 += frame->format.bytesPerPixel;
					src += frame->format.bytesPerPixel;
				}

				src += frame->pitch - frame->format.bytesPerPixel * frame->w;
				dst1 += scaledFrame.pitch * 2 - scaledFrame.format.bytesPerPixel * scaledFrame.w;
				dst2 += scaledFrame.pitch * 2 - scaledFrame.format.bytesPerPixel * scaledFrame.w;
			}

			_system->copyRectToScreen((byte *)scaledFrame.pixels, scaledFrame.pitch, x, y, scaledFrame.w, scaledFrame.h);
			_system->updateScreen();
			scaledFrame.free();
		}

		Input input;
		InputHandler::getCurrentInputDevice()->getInput(input, kFilterAllInput);
		if (input.anyInput())
			skipped = true;

		_system->delayMillis(10);
	}

	return skipped;
}

void PegasusEngine::die(const tDeathReason reason) {
	Input dummy;
	if (isDragging())
		_itemDragger.stopTracking(dummy);

	_deathReason = reason;
	_shellNotification.setNotificationFlags(kPlayerDiedFlag, kPlayerDiedFlag);
}

void PegasusEngine::doDeath() {
	_gfx->doFadeOutSync();
	throwAwayEverything();
	useMenu(new DeathMenu(_deathReason));
	_gfx->updateDisplay();
	_gfx->doFadeInSync();
}

void PegasusEngine::throwAwayEverything() {
	if (_items.getNumItems() != 0)
		_currentItemID = g_interface->getCurrentInventoryItem()->getObjectID();
	else
		_currentItemID = kNoItemID;

	if (_biochips.getNumItems() != 0)
		_currentItemID = g_interface->getCurrentBiochip()->getObjectID();
	else
		_currentItemID = kNoItemID;

	useMenu(0);
	useNeighborhood(0);

	delete g_interface;
	g_interface = 0;
}

void PegasusEngine::processShell() {
	checkCallBacks();
	checkNotifications();
	InputHandler::pollForInput();
	refreshDisplay();
}

void PegasusEngine::createInterface() {
	if (!g_interface)
		new Interface();

	g_interface->createInterface();
}

void PegasusEngine::setGameMode(const tGameMode newMode) {
	if (newMode != _gameMode && canSwitchGameMode(newMode, _gameMode)) {
		switchGameMode(newMode, _gameMode);
		_gameMode = newMode;
	}
}

void PegasusEngine::switchGameMode(const tGameMode newMode, const tGameMode oldMode) {
	// Start raising panels before lowering panels, to give the activating panel time
	// to set itself up without cutting into the lowering panel's animation time.
	
	if (_switchModesSync) {
		if (newMode == kModeInventoryPick)
			raiseInventoryDrawerSync();
		else if (newMode == kModeBiochipPick)
			raiseBiochipDrawerSync();
		else if (newMode == kModeInfoScreen)
			showInfoScreen();
		
		if (oldMode == kModeInventoryPick)
			lowerInventoryDrawerSync();
		else if (oldMode == kModeBiochipPick)
			lowerBiochipDrawerSync();
		else if (oldMode == kModeInfoScreen)
			hideInfoScreen();
	} else {
		if (newMode == kModeInventoryPick)
			raiseInventoryDrawer();
		else if (newMode == kModeBiochipPick)
			raiseBiochipDrawer();
		else if (newMode == kModeInfoScreen)
			showInfoScreen();
		
		if (oldMode == kModeInventoryPick)
			lowerInventoryDrawer();
		else if (oldMode == kModeBiochipPick)
			lowerBiochipDrawer();
		else if (oldMode == kModeInfoScreen)
			hideInfoScreen();
	}
}

bool PegasusEngine::canSwitchGameMode(const tGameMode newMode, const tGameMode oldMode) {
	if (newMode == kModeInventoryPick && oldMode == kModeBiochipPick)
		return false;
	if (newMode == kModeBiochipPick && oldMode == kModeInventoryPick)
		return false;
	return true;
}

bool PegasusEngine::itemInLocation(const tItemID itemID, const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction) {
	tNeighborhoodID itemNeighborhood;
	tRoomID itemRoom;
	tDirectionConstant itemDirection;
	
	Item *item = g_allItems.findItemByID(itemID);
	item->getItemRoom(itemNeighborhood, itemRoom, itemDirection);

	return itemNeighborhood == neighborhood && itemRoom == room && itemDirection == direction;
}

tInventoryResult PegasusEngine::addItemToInventory(InventoryItem *item) {
	tInventoryResult result;
	
	do {
		if (g_interface)
			result = g_interface->addInventoryItem(item);
		else
			result = _items.addItem(item);

		// TODO
		if (result == kTooMuchWeight)
			error("Out of inventory space");
	} while (result != kInventoryOK);

	GameState.setTakenItem(item, true);
	if (g_neighborhood)
			g_neighborhood->pickedUpItem(item);

	g_AIArea->checkMiddleArea();

	return result;
}

void PegasusEngine::useNeighborhood(Neighborhood *neighborhood) {
	delete _neighborhood;
	_neighborhood = neighborhood;

	if (_neighborhood) {
		InputHandler::setInputHandler(_neighborhood);
		_neighborhood->init();
		_neighborhood->moveNavTo(kNavAreaLeft, kNavAreaTop);
		g_interface->setDate(_neighborhood->getDateResID());
	} else {
		InputHandler::setInputHandler(this);
	}
}

void PegasusEngine::performJump(const tNeighborhoodID neighborhoodID) {
	if (neighborhoodID == kNoradSubChaseID)
		error("TODO: Sub chase");

	if (_neighborhood)
		useNeighborhood(0);

	Neighborhood *neighborhood;
	makeNeighborhood(neighborhoodID, neighborhood);
	useNeighborhood(neighborhood);
}

void PegasusEngine::startNeighborhood() {
	if (_currentItemID != kNoItemID)
		g_interface->setCurrentInventoryItemID(_currentItemID);
	
	if (_currentBiochipID != kNoItemID)
		g_interface->setCurrentBiochipID(_currentBiochipID);
	
	setGameMode(kModeNavigation);
	
	if (_neighborhood)
		_neighborhood->start();
}

void PegasusEngine::startNewGame() {
	// WORKAROUND: The original game ignored the menu difficulty
	// setting. We're going to pass it through here so that
	// the menu actually makes sense now.
	bool isWalkthrough = GameState.getWalkthroughMode();
	GameState.resetGameState();
	GameState.setWalkthroughMode(isWalkthrough);

	// TODO: Enable erase
	_gfx->doFadeOutSync();
	useMenu(0);
	_gfx->updateDisplay();

	createInterface();

	if (isDemo()) {
		setLastEnergyValue(kFullEnergy);
		jumpToNewEnvironment(kPrehistoricID, kPrehistoric02, kSouth);
		GameState.setPrehistoricSeenTimeStream(false);
		GameState.setPrehistoricSeenFlyer1(false);
		GameState.setPrehistoricSeenFlyer2(false);
		GameState.setPrehistoricSeenBridgeZoom(false);
		GameState.setPrehistoricBreakerThrown(false);
	} else {
		jumpToNewEnvironment(kCaldoriaID, kCaldoria00, kEast);
	}

	removeAllItemsFromInventory();
	removeAllItemsFromBiochips();

	BiochipItem *biochip = (BiochipItem *)g_allItems.findItemByID(kAIBiochip);
	addItemToBiochips(biochip);

	if (isDemo()) {
		biochip = (BiochipItem *)g_allItems.findItemByID(kPegasusBiochip);
		addItemToBiochips(biochip);
		biochip = (BiochipItem *)g_allItems.findItemByID(kMapBiochip);
		addItemToBiochips(biochip);
		InventoryItem *item = (InventoryItem *)g_allItems.findItemByID(kKeyCard);
		addItemToInventory(item);
		item = (InventoryItem *)g_allItems.findItemByID(kJourneymanKey);
		addItemToInventory(item);
		_currentItemID = kJourneymanKey;
	} else {
		_currentItemID = kNoItemID;
	}

	_currentBiochipID = kAIBiochip;

	// Clear jump notification flags and just perform the jump...
	_shellNotification.setNotificationFlags(0, kNeedNewJumpFlag);
	
	performJump(GameState.getNextNeighborhood());
	
	startNeighborhood();
}

void PegasusEngine::makeNeighborhood(tNeighborhoodID neighborhoodID, Neighborhood *&neighborhood) {
	// TODO: CD check
	
	switch (neighborhoodID) {
	case kPrehistoricID:
		neighborhood = new Prehistoric(g_AIArea, this);
		break;
	}
}

bool PegasusEngine::wantsCursor() {
	return _gameMenu == 0;
}

void PegasusEngine::updateCursor(const Common::Point, const Hotspot *cursorSpot) {	
	if (_itemDragger.isTracking()) {
		_cursor->setCurrentFrameIndex(5);
	} else {
		if (!cursorSpot) {
			_cursor->setCurrentFrameIndex(0);
		} else {
			uint32 id = cursorSpot->getObjectID();

			switch (id) {
			case kCurrentItemSpotID:
				if (countInventoryItems() != 0)
					_cursor->setCurrentFrameIndex(4);
				else
					_cursor->setCurrentFrameIndex(0);
				break;
			default:
				tHotSpotFlags flags = cursorSpot->getHotspotFlags();

				if (flags & kZoomInSpotFlag)
					_cursor->setCurrentFrameIndex(1);
				else if (flags & kZoomOutSpotFlag)
					_cursor->setCurrentFrameIndex(2);
				else if (flags & (kPickUpItemSpotFlag | kPickUpBiochipSpotFlag))
					_cursor->setCurrentFrameIndex(4);
				else if (flags & kJMPClickingSpotFlags)
					_cursor->setCurrentFrameIndex(3);
				else
					_cursor->setCurrentFrameIndex(0);
			}
		}
	}
}

void PegasusEngine::toggleInventoryDisplay() {
	if (_gameMode == kModeInventoryPick)
		setGameMode(kModeNavigation);
	else
		setGameMode(kModeInventoryPick);
}

void PegasusEngine::toggleBiochipDisplay() {
	if (_gameMode == kModeBiochipPick)
		setGameMode(kModeNavigation);
	else
		setGameMode(kModeBiochipPick);
}

void PegasusEngine::showInfoScreen() {	
	if (g_neighborhood) {
		// Break the input handler chain...
		_savedHandler = InputHandler::getCurrentHandler();
		InputHandler::setInputHandler(this);
		
		Picture *pushPicture = ((Neighborhood *)g_neighborhood)->getTurnPushPicture();
		
		_bigInfoMovie.shareSurface(pushPicture);
		_smallInfoMovie.shareSurface(pushPicture);

		g_neighborhood->hideNav();

		_smallInfoMovie.initFromMovieFile("Images/Items/Info Right Movie");
		_smallInfoMovie.setDisplayOrder(kInfoSpinOrder);
		_smallInfoMovie.moveElementTo(kNavAreaLeft + 304, kNavAreaTop + 8);
		_smallInfoMovie.moveMovieBoxTo(304, 8);
		_smallInfoMovie.startDisplaying();
		_smallInfoMovie.show();

		TimeValue startTime, stopTime;
		g_AIArea->getSmallInfoSegment(startTime, stopTime);
		_smallInfoMovie.setSegment(startTime, stopTime);
		_smallInfoMovie.setFlags(kLoopTimeBase);

		_bigInfoMovie.initFromMovieFile("Images/Items/Info Left Movie");
		_bigInfoMovie.setDisplayOrder(kInfoBackgroundOrder);
		_bigInfoMovie.moveElementTo(kNavAreaLeft, kNavAreaTop);
		_bigInfoMovie.startDisplaying();
		_bigInfoMovie.show();
		_bigInfoMovie.setTime(g_AIArea->getBigInfoTime());

		_bigInfoMovie.redrawMovieWorld();
		_smallInfoMovie.redrawMovieWorld();
		_smallInfoMovie.start();
	}
}

void PegasusEngine::hideInfoScreen() {
	if (g_neighborhood) {
		InputHandler::setInputHandler(_savedHandler);

		_bigInfoMovie.hide();
		_bigInfoMovie.stopDisplaying();
		_bigInfoMovie.releaseMovie();

		_smallInfoMovie.hide();
		_smallInfoMovie.stopDisplaying();
		_smallInfoMovie.stop();
		_smallInfoMovie.releaseMovie();

		g_neighborhood->showNav();
	}
}

void PegasusEngine::raiseInventoryDrawer() {
	if (g_interface)
		g_interface->raiseInventoryDrawer();
}

void PegasusEngine::raiseBiochipDrawer() {
	if (g_interface)
		g_interface->raiseBiochipDrawer();
}

void PegasusEngine::lowerInventoryDrawer() {
	if (g_interface)
		g_interface->lowerInventoryDrawer();
}

void PegasusEngine::lowerBiochipDrawer() {
	if (g_interface)
		g_interface->lowerBiochipDrawer();
}

void PegasusEngine::raiseInventoryDrawerSync() {
	if (g_interface)
		g_interface->raiseInventoryDrawerSync();
}

void PegasusEngine::raiseBiochipDrawerSync() {
	if (g_interface)
		g_interface->raiseBiochipDrawerSync();
}

void PegasusEngine::lowerInventoryDrawerSync() {
	if (g_interface)
		g_interface->lowerInventoryDrawerSync();
}

void PegasusEngine::lowerBiochipDrawerSync() {
	if (g_interface)
		g_interface->lowerBiochipDrawerSync();
}

void PegasusEngine::toggleInfo() {
	if (_gameMode == kModeInfoScreen)
		setGameMode(kModeNavigation);
	else if (_gameMode == kModeNavigation)
		setGameMode(kModeInfoScreen);
}

void PegasusEngine::dragTerminated(const Input &) {
	Hotspot *finalSpot = _itemDragger.getLastHotspot();
	tInventoryResult result;

	if (_dragType == kDragInventoryPickup) {
		if (finalSpot && finalSpot->getObjectID() == kInventoryDropSpotID)
			result = addItemToInventory((InventoryItem *)_draggingItem);
		else
			result = kTooMuchWeight;

		if (result != kInventoryOK)
			autoDragItemIntoRoom(_draggingItem, _draggingSprite);
		else
			delete _draggingSprite;
	} else if (_dragType == kDragBiochipPickup) {
		if (finalSpot && finalSpot->getObjectID() == kBiochipDropSpotID)
			result = addItemToBiochips((BiochipItem *)_draggingItem);
		else
			result = kTooMuchWeight;

		if (result != kInventoryOK)
			autoDragItemIntoRoom(_draggingItem, _draggingSprite);
		else
			delete _draggingSprite;
	} else if (_dragType == kDragInventoryUse) {
		if (finalSpot && (finalSpot->getHotspotFlags() & kDropItemSpotFlag) != 0) {
			//	*** Need to decide on a case by case basis what to do here.
			//	the crowbar should break the cover off the Mars reactor if its frozen, the
			//	global transport card should slide through the slot, the oxygen mask should
			//	attach to the filling station, and so on...
			_neighborhood->dropItemIntoRoom(_draggingItem, finalSpot);
			delete _draggingSprite;
		} else {
			autoDragItemIntoInventory(_draggingItem, _draggingSprite);
		}
	}

	_dragType = kDragNoDrag;

	if (g_AIArea)
		g_AIArea->unlockAI();
}


void PegasusEngine::dragItem(const Input &input, Item *item, tDragType type) {	
	_draggingItem = item;
	_dragType = type;

	//	Create the sprite.
	_draggingSprite = _draggingItem->getDragSprite(kDraggingSpriteID);
	Common::Point where;
	input.getInputLocation(where);
	Common::Rect r1;
	_draggingSprite->getBounds(r1);
	r1 = Common::Rect::center(where.x, where.y, r1.width(), r1.height());
	_draggingSprite->setBounds(r1);

	//	Set up drag constraints.
	DisplayElement *navMovie = _gfx->findDisplayElement(kNavMovieID);
	Common::Rect r2;
	navMovie->getBounds(r2);
	r2.left -= r1.width() / 3;
	r2.right += r1.width() / 3;
	r2.top -= r1.height() / 3;
	r2.bottom += r2.height() / 3;

	r1 = Common::Rect(-30000, -30000, 30000, 30000);
	_itemDragger.setDragConstraints(r2, r1);

	// Start dragging.
	_draggingSprite->setDisplayOrder(kDragSpriteOrder);
	_draggingSprite->startDisplaying();
	_draggingSprite->show();
	_itemDragger.setDragSprite(_draggingSprite);
	_itemDragger.setNextHandler(_neighborhood);
	_itemDragger.startTracking(input);

	if (g_AIArea)
		g_AIArea->lockAIOut();
}

void PegasusEngine::shellGameInput(const Input &input, const Hotspot *cursorSpot) {
	if (_gameMode == kModeInfoScreen) {
		if (JMPPPInput::isToggleAIMiddleInput(input)) {
			tLowerClientSignature middleOwner = g_AIArea->getMiddleAreaOwner();
			g_AIArea->toggleMiddleAreaOwner();

			if (middleOwner != g_AIArea->getMiddleAreaOwner()) {
				_bigInfoMovie.setTime(g_AIArea->getBigInfoTime());
				_smallInfoMovie.stop();
				_smallInfoMovie.setFlags(0);

				TimeValue startTime, stopTime;
				g_AIArea->getSmallInfoSegment(startTime, stopTime);
				_smallInfoMovie.setSegment(startTime, stopTime);
				_smallInfoMovie.setFlags(kLoopTimeBase);

				_bigInfoMovie.redrawMovieWorld();
				_smallInfoMovie.redrawMovieWorld();
				_smallInfoMovie.start();
			}
		}
	} else {
		if (JMPPPInput::isRaiseInventoryInput(input))
			toggleInventoryDisplay();

		if (JMPPPInput::isRaiseBiochipsInput(input))
			toggleBiochipDisplay();

		if (JMPPPInput::isTogglePauseInput(input) && _neighborhood)
			pauseMenu(!isPaused());
	}

	if (JMPPPInput::isToggleInfoInput(input))
		toggleInfo();
}

void PegasusEngine::activateHotspots() {
	if (_gameMode == kModeInfoScreen) {
		g_allHotspots.activateOneHotspot(kInfoReturnSpotID);
	} else {
		// Set up hot spots.
		if (_dragType == kDragInventoryPickup)
			g_allHotspots.activateOneHotspot(kInventoryDropSpotID);
		else if (_dragType == kDragBiochipPickup)
			g_allHotspots.activateOneHotspot(kBiochipDropSpotID);
		else if (_dragType == kDragNoDrag)
			g_allHotspots.activateMaskedHotspots(kShellSpotFlag);
	}
}

bool PegasusEngine::isClickInput(const Input &input, const Hotspot *cursorSpot) {
	if (_cursor->isVisible() && cursorSpot)
		return JMPPPInput::isClickInput(input);
	else
		return false;
}

tInputBits PegasusEngine::getClickFilter() {
	return JMPPPInput::getClickInputFilter();
}

void PegasusEngine::clickInHotspot(const Input &input, const Hotspot *clickedSpot) {
	if (clickedSpot->getObjectID() == kCurrentItemSpotID) {
		InventoryItem *currentItem = getCurrentInventoryItem();
		if (currentItem) {
			removeItemFromInventory(currentItem);
			dragItem(input, currentItem, kDragInventoryUse);
		}
	} else if (clickedSpot->getObjectID() == kInfoReturnSpotID) {
		toggleInfo();
	}
}

tInventoryResult PegasusEngine::removeItemFromInventory(InventoryItem *item) {
	tInventoryResult result;
	
	if (g_interface)
		result = g_interface->removeInventoryItem(item);
	else
		result = _items.removeItem(item);

	// This should never happen
	assert(result == kInventoryOK);

	return result;
}

void PegasusEngine::removeAllItemsFromInventory() {
	if (g_interface)
		g_interface->removeAllItemsFromInventory();
	else
		_items.removeAllItems();
}

/////////////////////////////////////////////
//
// Biochip handling.

// Adding biochips to the biochip drawer is a little funny, because of two things:
//      --  We get the map biochip and pegasus biochip at the same time by dragging
//          one sprite in TSA
//      --  We can drag in more than one copy of the various biochips.
// Because of this we need to make sure that no more than one copy of each biochip
// is ever added.

tInventoryResult PegasusEngine::addItemToBiochips(BiochipItem *biochip) {
	tInventoryResult result;

	if (g_interface)
		result = g_interface->addBiochip(biochip);
	else
		result = _biochips.addItem(biochip);

	// This can never happen
	assert(result == kInventoryOK);

	GameState.setTakenItem(biochip, true);

	if (g_neighborhood)
		g_neighborhood->pickedUpItem(biochip);

	g_AIArea->checkMiddleArea();

	return result;
}

void PegasusEngine::removeAllItemsFromBiochips() {
	if (g_interface)
		g_interface->removeAllItemsFromBiochips();
	else
		_biochips.removeAllItems();
}

void PegasusEngine::setSoundFXLevel(uint16 fxLevel) {
	_FXLevel = fxLevel;
	if (_neighborhood)
		_neighborhood->setSoundFXLevel(fxLevel);
	if (g_AIArea)
		g_AIArea->setAIVolume(fxLevel);
}

void PegasusEngine::setAmbienceLevel(uint16 ambientLevel) {
	_ambientLevel = ambientLevel;
	if (_neighborhood)
		_neighborhood->setAmbienceLevel(ambientLevel);
}

void PegasusEngine::pauseMenu(bool menuUp) {
	if (menuUp) {
		// TODO: Pause engine
		_screenDimmer.startDisplaying();
		_screenDimmer.show();
		_gfx->updateDisplay();
		useMenu(new PauseMenu());
	} else {
		// TODO: Resume engine
		_screenDimmer.hide();
		_screenDimmer.stopDisplaying();
		useMenu(0);
		g_AIArea->checkMiddleArea();
	}
}

void PegasusEngine::autoDragItemIntoRoom(Item *item, Sprite *draggingSprite) {	
	if (g_AIArea)
		g_AIArea->lockAIOut();

	Common::Point start, stop;
	draggingSprite->getLocation(start.x, start.y);

	Hotspot *dropSpot = _neighborhood->getItemScreenSpot(item, draggingSprite);

	if (dropSpot) {
		dropSpot->getCenter(stop.x, stop.y);
	} else {
		stop.x = kNavAreaLeft + 256;
		stop.y = kNavAreaTop + 128;
	}

	Common::Rect bounds;
	draggingSprite->getBounds(bounds);
	stop.x -= bounds.width() >> 1;
	stop.y -= bounds.height() >> 1;

	int dx = ABS(stop.x - start.x);
	int dy = ABS(stop.y - start.y);
	TimeValue time = MAX(dx, dy);

	allowInput(false);
	_autoDragger.autoDrag(draggingSprite, start, stop, time, kDefaultTimeScale);

	while (_autoDragger.isDragging()) {
		checkCallBacks();
		refreshDisplay();
		_system->delayMillis(10);
	}

	_neighborhood->dropItemIntoRoom(_draggingItem, dropSpot);
	allowInput(true);
	delete _draggingSprite;

	if (g_AIArea)
		g_AIArea->unlockAI();
}

void PegasusEngine::autoDragItemIntoInventory(Item *, Sprite *draggingSprite) {	
	if (g_AIArea)
		g_AIArea->lockAIOut();

	Common::Point start;
	draggingSprite->getLocation(start.x, start.y);

	Common::Rect r;
	draggingSprite->getBounds(r);

	Common::Point stop((76 + 172 - r.width()) / 2, 334 - (2 * r.height() / 3));

	int dx = ABS(stop.x - start.x);
	int dy = ABS(stop.y - start.y);
	TimeValue time = MAX(dx, dy);

	allowInput(false);
	_autoDragger.autoDrag(draggingSprite, start, stop, time, kDefaultTimeScale);

	while (_autoDragger.isDragging()) {
		checkCallBacks();
		refreshDisplay();
		_system->delayMillis(10);
	}

	addItemToInventory((InventoryItem *)_draggingItem);
	allowInput(true);
	delete _draggingSprite;

	if (g_AIArea)
		g_AIArea->unlockAI();
}

tNeighborhoodID PegasusEngine::getCurrentNeighborhoodID() const {
	if (_neighborhood)
		return _neighborhood->getObjectID();

	return kNoNeighborhoodID;
}

} // End of namespace Pegasus
