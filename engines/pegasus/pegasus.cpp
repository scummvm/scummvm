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
#include "pegasus/gamestate.h"
#include "pegasus/menu.h"
#include "pegasus/movie.h"
#include "pegasus/pegasus.h"
#include "pegasus/timers.h"
#include "pegasus/items/itemlist.h"
#include "pegasus/items/biochips/biochipitem.h"
#include "pegasus/items/inventory/inventoryitem.h"

namespace Pegasus {

PegasusEngine::PegasusEngine(OSystem *syst, const PegasusGameDescription *gamedesc) : Engine(syst), InputHandler(0), _gameDescription(gamedesc),
		_shellNotification(kJMPDCShellNotificationID, this), _returnHotspot(kInfoReturnSpotID) {
	_continuePoint = 0;
	_saveAllowed = _loadAllowed = true;
	_gameMenu = 0;
}

PegasusEngine::~PegasusEngine() {
	delete _gfx;
	delete _resFork;
	delete _console;
	delete _cursor;
	delete _continuePoint;
	delete _gameMenu;
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
	_items.setWeightLimit(0);
	_items.setOwnerID(kPlayerID);
	_biochips.setWeightLimit(8);
	_biochips.setOwnerID(kPlayerID);

	// Start up the first notification
	_shellNotification.notifyMe(this, kJMPShellNotificationFlags, kJMPShellNotificationFlags);
	_shellNotification.setNotificationFlags(kGameStartingFlag, kGameStartingFlag);

	_returnHotspot.setArea(Common::Rect(kNavAreaLeft, kNavAreaTop, 512 + kNavAreaLeft, 256 + kNavAreaTop));
	_returnHotspot.setHotspotFlags(kInfoReturnSpotFlag);
	g_allHotspots.push_back(&_returnHotspot);

	while (!shouldQuit()) {
		checkCallBacks();
		checkNotifications();
		InputHandler::pollForInput();
		giveIdleTime();
		_gfx->updateDisplay();
	}

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
		// Unused in game, but still in the data - no need to load it
		break;
	case kMapBiochip:
	case kAIBiochip:
	case kPegasusBiochip:
	case kRetinalScanBiochip:
	case kShieldBiochip:
	case kOpticalBiochip:
		// TODO: Specialized biochip classes
		new BiochipItem(itemID, neighborhoodID, roomID, direction);
		break;
	case kAirMask:
	case kKeyCard:
	case kGasCanister:
		// TODO: Specialized inventory item classes
		new InventoryItem(itemID, neighborhoodID, roomID, direction);
		break;
	default:
		// Everything else is a normal inventory item
		new InventoryItem(itemID, neighborhoodID, roomID, direction);
		break;
	}
}

void PegasusEngine::runIntro() {
	Video::SeekableVideoDecoder *video = new Video::QuickTimeDecoder();
	if (video->loadFile(_introDirectory + "/BandaiLogo.movie")) {
		while (!shouldQuit() && !video->endOfVideo()) {
			if (video->needsUpdate()) {
				const Graphics::Surface *frame = video->decodeNextFrame();
				_system->copyRectToScreen((byte *)frame->pixels, frame->pitch, 0, 0, frame->w, frame->h);
				_system->updateScreen();
			}

			Common::Event event;
			while (_eventMan->pollEvent(event))
				;
		}
	}

	delete video;

	if (shouldQuit())
		return;

	video = new Video::QuickTimeDecoder();

	if (!video->loadFile(_introDirectory + "/Big Movie.movie"))
		error("Could not load intro movie");

	video->seekToTime(Audio::Timestamp(0, 10 * 600, 600));

	while (!shouldQuit() && !video->endOfVideo()) {
		if (video->needsUpdate()) {
			const Graphics::Surface *frame = video->decodeNextFrame();

			// Scale up the frame doing some simple scaling
			Graphics::Surface scaledFrame;
			scaledFrame.create(frame->w * 2, frame->h * 2, frame->format);
			const byte *src = (const byte *)frame->pixels;
			byte *dst1 = (byte *)scaledFrame.pixels;
			byte *dst2 = (byte *)scaledFrame.pixels + scaledFrame.pitch;

			for (int y = 0; y < frame->h; y++) {
				for (int x = 0; x < frame->w; x++) {
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

			_system->copyRectToScreen((byte *)scaledFrame.pixels, scaledFrame.pitch, 0, 0, scaledFrame.w, scaledFrame.h);
			_system->updateScreen();
			scaledFrame.free();
		}

		Common::Event event;
		while (_eventMan->pollEvent(event))
			;
	}

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
	// TODO: Dispose currently running stuff (neighborhood, etc.)

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

	// TODO: Energy
	stream->readUint32BE();

	// TODO: Death reason
	stream->readByte();

	// TODO: This is as far as we can go right now
	return true;

	// Items
	g_allItems.readFromStream(stream);

	// TODO: Player Inventory
	// TODO: Player BioChips
	// TODO: Disc check
	// TODO: Jump to environment
	// TODO: AI rules

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

	// TODO: Energy
	stream->writeUint32BE(0);

	// TODO: Death reason
	stream->writeByte(0);

	// Items
	g_allItems.writeToStream(stream);

	// TODO: Player Inventory
	// TODO: Player BioChips
	// TODO: Jump to environment
	// TODO: AI rules
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

			if (shouldQuit())
				return;

			useMenu(new MainMenu());
			_gfx->invalRect(Common::Rect(0, 0, 640, 480));
			_gfx->updateDisplay();
			((MainMenu *)_gameMenu)->startMainMenuLoop();
#endif
			break;
		}
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
		error("Start new game (adventure mode)");
		break;
	case kMenuCmdCredits:
		error("Show credits");
		break;
	case kMenuCmdQuit:
		_system->quit();
		break;
	case kMenuCmdOverview:
		error("Show overview");
		break;
	case kMenuCmdStartWalkthrough:
		GameState.setWalkthroughMode(true);
		error("Start new game (walkthrough mode)");
		break;
	case kMenuCmdRestore:
		error("Load game");
		break;
	case kMenuCmdNoCommand:
		break;
	default:
		error("Unknown menu command %d", command);
	}
}

void PegasusEngine::handleInput(const Input &input, const Hotspot *cursorSpot) {
	if (!checkGameMenu())
		; // TODO: Other input

	// TODO: Quit request
	// TODO: Save request
	// TODO: Load request
}

} // End of namespace Pegasus
