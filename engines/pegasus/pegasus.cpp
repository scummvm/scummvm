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
#include "common/file.h"
#include "common/fs.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "base/plugins.h"
#include "base/version.h"
#include "gui/saveload.h"

#include "pegasus/console.h"
#include "pegasus/cursor.h"
#include "pegasus/gamestate.h"
#include "pegasus/pegasus.h"
#include "pegasus/timers.h"
#include "pegasus/items/itemlist.h"
#include "pegasus/items/biochips/biochipitem.h"
#include "pegasus/items/inventory/inventoryitem.h"

//#define RUN_SUB_MOVIE // :D :D :D :D :D :D
//#define RUN_INTERFACE_TEST

#ifdef RUN_INTERFACE_TEST
#include "pegasus/sound.h"
#endif

namespace Pegasus {

PegasusEngine::PegasusEngine(OSystem *syst, const PegasusGameDescription *gamedesc) : Engine(syst), InputHandler(0), _gameDescription(gamedesc) {
	_continuePoint = 0;
	_saveAllowed = _loadAllowed = true;
}

PegasusEngine::~PegasusEngine() {
	delete _video;
	delete _gfx;
	delete _resFork;
	delete _inventoryLid;
	delete _biochipLid;
	delete _console;
	delete _cursor;
	delete _continuePoint;
}

Common::Error PegasusEngine::run() {
	_console = new PegasusConsole(this);
	_gfx = new GraphicsManager(this);
	_video = new VideoManager(this);
	_resFork = new Common::MacResManager();
	_inventoryLid = new Common::MacResManager();
	_biochipLid = new Common::MacResManager();
	_cursor = new Cursor();
	_gameMode = kIntroMode;
	_adventureMode = true;
	
	if (!_resFork->open("JMP PP Resources") || !_resFork->hasResFork())
		error("Could not load JMP PP Resources");

	if (!_inventoryLid->open("Images/Lids/Inventory Lid Sequence") || !_inventoryLid->hasResFork())
		error("Could not open Inventory Lid Sequence");

	if (!_biochipLid->open("Images/Lids/Biochip Lid Sequence") || !_biochipLid->hasResFork())
		error("Could not open Biochip Lid Sequence");

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

#if 0
	Common::MacResIDArray pictIds = _biochipLid->getResIDArray(MKID_BE('PICT'));
	for (uint32 i = 0; i < pictIds.size(); i++) {
		Common::String filename = Common::String::printf("PICT_%d.pict", pictIds[i]);
		Common::DumpFile file;
		assert(file.open(filename));
		Common::SeekableReadStream *res = _biochipLid->getResource(MKID_BE('PICT'), pictIds[i]);
		byte *data = new byte[res->size()];
		res->read(data, res->size());
		for (int j = 0; j < 512; j++)
			file.writeByte(0);
		file.write(data, res->size());
		file.close();
		delete res;
		delete[] data;
	}
#endif

#if defined(RUN_SUB_MOVIE)
	_video->playMovie("Images/Norad Alpha/Sub Chase Movie");
#elif defined(RUN_INTERFACE_TEST)
	_cursor->setCurrentFrameIndex(0);
	_cursor->show();
	drawInterface();
	Sound sound;
	sound.initFromAIFFFile("Sounds/Caldoria/Apartment Music.aiff");
	sound.loopSound();

	while (!shouldQuit()) {
		Common::Event event;
		// Ignore events for now
		while (_eventMan->pollEvent(event)) {
			if (event.type == Common::EVENT_MOUSEMOVE)
				_system->updateScreen();
		}
		
		_system->delayMillis(10);
	}
#else
	while (!shouldQuit()) {
		switch (_gameMode) {
		case kIntroMode:
			if (!isDemo())
				runIntro();
			_gameMode = kMainMenuMode;
			break;
		case kMainMenuMode:
			runMainMenu();
			break;
		case kMainGameMode:
			// NOTE: Prehistoric will be our testing location
			changeLocation(kPrehistoricID);
			mainGameLoop();
			break;
		case kQuitMode:
			return Common::kNoError;
		default:
			_gameMode = kMainMenuMode;
			break;
		}
	}
#endif

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
	_video->playMovieCentered(_introDirectory + "/BandaiLogo.movie");
	VideoHandle handle = _video->playBackgroundMovie(_introDirectory + "/Big Movie.movie");
	_video->seekToTime(handle, 10 * 600);
	_video->waitUntilMovieEnds(handle);
}

void PegasusEngine::drawInterface() {
	_gfx->drawPict("Images/Interface/3DInterface Top", 0, 0, false);
	_gfx->drawPict("Images/Interface/3DInterface Left", 0, kViewScreenOffset, false);
	_gfx->drawPict("Images/Interface/3DInterface Right", 640 - kViewScreenOffset, kViewScreenOffset, false);
	_gfx->drawPict("Images/Interface/3DInterface Bottom", 0, kViewScreenOffset + 256, false);
	//drawCompass();
	_system->updateScreen();
}

void PegasusEngine::mainGameLoop() {
	// TODO: Yeah...
	_system->fillScreen(0);
	_video->playMovieCentered("Images/Caldoria/Pullback.movie");
	drawInterface();

	Common::String navMovie = Common::String::format("Images/%s/%s.movie", getTimeZoneFolder(GameState.getCurrentNeighborhood()).c_str(), getTimeZoneDesc(GameState.getCurrentNeighborhood()).c_str());
	_video->playMovie(navMovie, kViewScreenOffset, kViewScreenOffset);

	_gameMode = kQuitMode;
}

void PegasusEngine::changeLocation(tNeighborhoodID neighborhood) {
	GameState.setCurrentNeighborhood(neighborhood);

	// Just a test...
	Neighborhood *neighborhoodPtr = new Neighborhood(this, this, getTimeZoneDesc(neighborhood), neighborhood);
	neighborhoodPtr->init();
	delete neighborhoodPtr;
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
		_gameMode = kMainGameMode;
	}

	slc.close();
}

Common::String PegasusEngine::getTimeZoneDesc(tNeighborhoodID neighborhood) {
	static const char *names[] = { "Caldoria", "Full TSA", "Full TSA", "Tiny TSA", "Prehistoric", "Mars", "WSC", "Norad Alpha", "Norad Delta" };
	return names[neighborhood];
}

Common::String PegasusEngine::getTimeZoneFolder(tNeighborhoodID neighborhood) {
	if (neighborhood == kFullTSAID || neighborhood == kTinyTSAID || neighborhood == kFinalTSAID)
		return "TSA";

	return getTimeZoneDesc(neighborhood);
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
	warning("pos = %d", loadFile->pos());
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

} // End of namespace Pegasus
