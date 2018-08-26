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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/system.h"
#include "common/translation.h"
#include "graphics/thumbnail.h"
#include "graphics/scaler.h"
#include "gui/saveload.h"
#include "ultima/ultima.h"
#include "ultima/debugger.h"
#include "ultima/events.h"
#include "ultima/core/resources.h"
#include "ultima/core/mouse_cursor.h"
#include "ultima/gfx/screen.h"
#include "ultima/games/ultima1/game.h"

namespace Ultima {

UltimaEngine *g_vm;

UltimaEngine::UltimaEngine(OSystem *syst, const UltimaGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _randomSource("Ultima") {
	g_vm = this;
	_debugger = nullptr;
	_events = nullptr;
	_game = nullptr;
	_mouseCursor = nullptr;
	_screen = nullptr;
}

UltimaEngine::~UltimaEngine() {
	delete _debugger;
	delete _events;
	delete _game;
	delete _mouseCursor;
	delete _screen;
}

GameId UltimaEngine::getGameID() const {
	return _gameDescription->gameId;
}

Common::Language UltimaEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

uint32 UltimaEngine::getFeatures() const {
	return _gameDescription->features;
}

bool UltimaEngine::isVGAEnhanced() const {
	return getFeatures() & GF_VGA_ENHANCED;
}

bool UltimaEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool UltimaEngine::initialize() {
	DebugMan.addDebugChannel(kDebugLevelScript,      "scripts", "Script debug level");

	// Set up the resources datafile
	Resources *res = new Resources();
	if (!res->open()) {
		GUIErrorMessage("Could not find correct ultima.dat datafile");
		return false;
	}
	SearchMan.add("ultima", res);

	_debugger = Debugger::init(this);
	_events = new Events();
	_screen = new Gfx::Screen();
	_mouseCursor = new MouseCursor();

	// Create the game, and signal to it that the game is starting
	_game = createGame();
	_events->addTarget(_game);

	// If requested, load a savegame instead of showing the intro
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 999) {
			_game->starting(true);
			loadGameState(saveSlot);
			return true;
		}
	}

	_game->starting(false);
	return true;
}

void UltimaEngine::deinitialize() {
}

Common::Error UltimaEngine::run() {
	// Initialize the engine and play the game
	if (initialize())
		playGame();

	// Deinitialize and free the engine
	deinitialize();
	return Common::kNoError;
}

void UltimaEngine::playGame() {
	while (!shouldQuit()) {
		_events->pollEventsAndWait();
	}
}

Shared::Game *UltimaEngine::createGame() const {
	switch (getGameID()) {
	case GAME_ULTIMA1:
		return new Ultima1::Ultima1Game();
	default:
		error("Unknown game");
	}
}

void UltimaEngine::GUIError(const char *msg, ...) {
	char buffer[STRINGBUFLEN];
	va_list va;

	// Generate the full error message
	va_start(va, msg);
	vsnprintf(buffer, STRINGBUFLEN, msg, va);
	va_end(va);

	GUIErrorMessage(buffer);
}

Common::Error UltimaEngine::loadGameState(int slot) {
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(
		Common::String::format("%s.%.3d", _targetName.c_str(), slot));
	if (!saveFile)
		return Common::kReadingFailed;

	// Load the savaegame header
	UltimaSavegameHeader header;
	if (!readSavegameHeader(saveFile, header, false))
		return Common::kReadingFailed;

	if (header._gameId != getGameID() || header._language != getLanguage()
		|| header._videoMode != (isVGAEnhanced() ? 9 : 0))
		return Common::kReadingFailed;

	// Set the total play time
	_events->setFrameCounter(header._totalFrames);

	// Read in the game's data
	Common::Serializer s(saveFile, nullptr);
	_game->synchronize(s);

	delete saveFile;
	return Common::kNoError;
}

Common::Error UltimaEngine::saveGameState(int slot, const Common::String &desc) {
	Common::OutSaveFile *saveFile = g_system->getSavefileManager()->openForSaving(
		Common::String::format("%s.%.3d", _targetName.c_str(), slot));
	if (!saveFile)
		return Common::kCreatingFileFailed;

	// Write the savegame header
	writeSavegameHeader(saveFile, desc);

	// Write out the game's data
	Common::Serializer s(nullptr, saveFile);
	_game->synchronize(s);

	saveFile->finalize();
	delete saveFile;

	return Common::kNoError;
}

bool UltimaEngine::canLoadGameStateCurrently() {
	return _game->canLoadGameStateCurrently();
}

bool UltimaEngine::canSaveGameStateCurrently() {
	return _game->canSaveGameStateCurrently();
}

static const uint32 SAVEGAME_IDENT = MKTAG('U', 'L', 'T', 'S');
static const int SAVEGAME_VERSION = 1;

bool UltimaEngine::readSavegameHeader(Common::InSaveFile *in, UltimaSavegameHeader &header, bool skipThumbnail) {
	// Validate the header Id
	if (in->readUint32BE() != SAVEGAME_IDENT)
		return false;

	header._version = in->readByte();
	if (header._version > SAVEGAME_VERSION)
		return false;

	// Read in game, version and language fields
	header._gameId = in->readByte();
	header._language = in->readByte();
	header._videoMode = in->readByte();

	// Read in the string
	header._saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		header._saveName += ch;

	// Get the thumbnail
	if (!Graphics::loadThumbnail(*in, header._thumbnail, skipThumbnail)) {
		return false;
	}

	// Read in save date/time
	header._year = in->readSint16LE();
	header._month = in->readSint16LE();
	header._day = in->readSint16LE();
	header._hour = in->readSint16LE();
	header._minute = in->readSint16LE();
	header._totalFrames = in->readUint32LE();

	return true;
}

void UltimaEngine::writeSavegameHeader(Common::OutSaveFile *out, const Common::String &saveName) {
	out->writeUint32BE(SAVEGAME_IDENT);
	out->writeByte(SAVEGAME_VERSION);
	out->writeByte(getGameID());
	out->writeByte(getLanguage());
	out->writeByte(isVGAEnhanced() ? 9 : 0);
	out->writeString(saveName);
	out->writeByte(0);

	// Write a thumbnail of the screen
	uint8 thumbPalette[PALETTE_SIZE];
	_screen->getPalette(thumbPalette);
	Graphics::Surface saveThumb;
	::createThumbnail(&saveThumb, (const byte *)_screen->getPixels(),
		_screen->w, _screen->h, thumbPalette);
	Graphics::saveThumbnail(*out, saveThumb);
	saveThumb.free();

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);
	out->writeUint32LE(_events->getFrameCounter());
}

bool UltimaEngine::saveGame() {
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
	int slotNum = dialog->runModalWithCurrentTarget();
	Common::String saveName = dialog->getResultString();
	delete dialog;

	if (slotNum != -1)
		saveGameState(slotNum, saveName);

	return slotNum != -1;
}

bool UltimaEngine::loadGame() {
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"), false);
	int slotNum = dialog->runModalWithCurrentTarget();
	delete dialog;

	if (slotNum != -1)
		loadGameState(slotNum);

	return slotNum != -1;
}

} // End of namespace Ultima
