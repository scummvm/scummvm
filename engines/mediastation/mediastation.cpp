/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "graphics/framelimiter.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"

#include "mediastation/mediastation.h"
#include "mediastation/debugchannels.h"
#include "mediastation/detection.h"
#include "mediastation/boot.h"
#include "mediastation/context.h"
#include "mediastation/asset.h"
#include "mediastation/assets/movie.h"

namespace MediaStation {

MediaStationEngine *g_engine;

MediaStationEngine::MediaStationEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc),
	_randomSource("MediaStation") {
	g_engine = this;
	_mixer = g_system->getMixer();

	_gameDataDir = Common::FSNode(ConfMan.getPath("path"));
	SearchMan.addDirectory(_gameDataDir, 0, 3);
	for (uint i = 0; MediaStation::directoryGlobs[i]; i++) {
		Common::String directoryGlob = directoryGlobs[i];
		SearchMan.addSubDirectoryMatching(_gameDataDir, directoryGlob, 0, 5);
	}
}

MediaStationEngine::~MediaStationEngine() {
	_mixer = nullptr;

	delete _screen;
	_screen = nullptr;

	delete _boot;
	_boot = nullptr;

	for (auto it = _assets.begin(); it != _assets.end(); ++it) {
		delete it->_value;
	}
	_assets.clear();
	_assetsByChunkReference.clear();

	for (auto it = _functions.begin(); it != _functions.end(); ++it) {
		delete it->_value;
	}
	_functions.clear();

	for (auto it = _variables.begin(); it != _variables.end(); ++it) {
		delete it->_value;
	}
	_variables.clear();

	delete _root;
	_root = nullptr;
}

uint32 MediaStationEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String MediaStationEngine::getGameId() const {
	return _gameDescription->gameId;
}

bool MediaStationEngine::isFirstGenerationEngine() {
	if (_boot == nullptr) {
		error("Attempted to get engine version before BOOT.STM was read");
	} else {
		return (_boot->_versionInfo == nullptr);
	}
}

Common::Error MediaStationEngine::run() {
	// INITIALIZE SUBSYSTEMS.
	// All Media Station games run at 640x480.
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);
	_screen = new Graphics::Screen();
	// TODO: Determine if all titles blank the screen to 0xff.
	_screen->fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0xff);

	// LOAD BOOT.STM.
	Common::Path bootStmFilepath = Common::Path("BOOT.STM");
	_boot = new Boot(bootStmFilepath);

	// LOAD THE ROOT CONTEXT.
	// This is because we might have assets that always need to be loaded.
	//Context *root = nullptr;
	uint32 rootContextId = _boot->getRootContextId();
	if (rootContextId != 0) {
		_root = loadContext(rootContextId);
	} else {
		warning("MediaStation::run(): Title has no root context");
	}

	branchToScreen(_boot->_entryContextId);
	while (true) {
		processEvents();
		if (shouldQuit()) {
			break;
		}

		// PROCESS ANY ASSETS CURRENTLY PLAYING.
		// TODO: Implement a dirty-rect based rendering system rather than
		// redrawing the screen each time. This will require keeping track of
		// all the images on screen at any given time, rather than just letting
		// the movies handle their own drawing.
		//
		// First, they all need to be sorted by z-coordinate.
		debugC(5, kDebugGraphics, "***** START RENDERING ***");
		Common::sort(_assetsPlaying.begin(), _assetsPlaying.end(), [](Asset * a, Asset * b) {
			return a->zIndex() > b->zIndex();
		});
		for (auto it = _assetsPlaying.begin(); it != _assetsPlaying.end();) {
			(*it)->process();
			if (!(*it)->isActive()) {
				it = _assetsPlaying.erase(it);
			} else {
				++it;
			}
		}
		debugC(5, kDebugGraphics, "***** END RENDERING ***");

		// UPDATE THE SCREEN.
		g_engine->_screen->update();
		g_system->delayMillis(10);
	}

	// CLEAN UP.
	return Common::kNoError;
}

void MediaStationEngine::processEvents() {
	while (g_system->getEventManager()->pollEvent(e)) {
		debugC(9, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		debugC(9, kDebugEvents, "@@@@   Processing events");
		debugC(9, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

		switch (e.type) {
		case Common::EVENT_QUIT: {
			// TODO: Do any necessary clean-up.
			return;
		}

		case Common::EVENT_KEYDOWN: {
			break;
		}

		case Common::EVENT_LBUTTONDOWN: {
			break;
		}

		case Common::EVENT_RBUTTONDOWN: {
			// We are using the right button as a quick exit since the Media
			// Station engine doesn't seem to use the right button itself.
			warning("EVENT_RBUTTONDOWN: Quitting for development purposes");
			quitGame();
		}

		default: {
			break;
		}
		}
	}
}

Context *MediaStationEngine::loadContext(uint32 contextId) {
	if (_boot == nullptr) {
		error("Cannot load contexts before BOOT.STM is read");
	}

	// GET THE FILE ID.
	SubfileDeclaration *subfileDeclaration = _boot->_subfileDeclarations.getValOrDefault(contextId);
	if (subfileDeclaration == nullptr) {
		warning("MediaStationEngine::loadContext(): Couldn't find subfile declaration with ID 0x%x", contextId);
		return nullptr;
	}
	// The subfile declarations have other assets too, so we need to make sure
	if (subfileDeclaration->_startOffsetInFile != 16) {
		warning("MediaStationEngine::loadContext(): Requested ID wasn't for a context.");
		return nullptr;
	}
	uint32 fileId = subfileDeclaration->_fileId;

	// GET THE FILENAME.
	FileDeclaration *fileDeclaration = _boot->_fileDeclarations.getValOrDefault(fileId);
	if (fileDeclaration == nullptr) {
		warning("MediaStationEngine::loadContext(): Couldn't find file declaration with ID 0x%x", fileId);
		return nullptr;
	}
	Common::String *fileName = fileDeclaration->_name;

	// LOAD THE CONTEXT.
	Common::Path entryCxtFilepath = Common::Path(*fileName);
	Context *context = new Context(entryCxtFilepath);

	// SET THE VARIABLES.
	return context;
}

void MediaStationEngine::setPalette(Asset *palette) {
	assert(palette != nullptr);
	setPaletteFromHeader(palette->getHeader());
}

void MediaStationEngine::setPaletteFromHeader(AssetHeader *header) {
	assert(header != nullptr);
	if (header->_palette != nullptr) {
		_screen->setPalette(*header->_palette);
	} else {
		warning("MediaStationEngine::setPaletteFromHeader(): Asset %d does not have a palette. Current palette will be unchanged.", header->_id);
	}
}

void MediaStationEngine::addPlayingAsset(Asset *assetToAdd) {
	// If we're already marking the asset as played, we don't need to mark it
	// played again.
	for (Asset *asset : g_engine->_assetsPlaying) {
		if (asset == assetToAdd) {
			return;
		}
	}
	g_engine->_assetsPlaying.push_back(assetToAdd);
}

Operand MediaStationEngine::callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) {
	switch (methodId) {
		case kBranchToScreenMethod: {
			assert(args.size() == 1);
			uint32 contextId = args[0].getAssetId();
			branchToScreen(contextId);
			return Operand();
		}

		default: {
			error("MediaStationEngine::callMethod(): Got unimplemented method ID %d", static_cast<uint>(methodId));
		}
	}
}

void MediaStationEngine::branchToScreen(uint32 contextId) {
	Context *context = loadContext(contextId);
	if (context->_screenAsset != nullptr) {
		setPaletteFromHeader(context->_screenAsset);
		
		// TODO: Make the screen an asset just like everything else so we can
		// run event handlers with runEventHandlerIfExists.
		EventHandler *entryEvent = context->_screenAsset->_eventHandlers.getValOrDefault(MediaStation::kEntryEvent);
		if (entryEvent != nullptr) {
			debugC(5, kDebugScript, "Executing context entry event handler");
			entryEvent->execute(context->_screenAsset->_id);
		} else {
			debugC(5, kDebugScript, "No context entry event handler");
		}
	}
	_currentContext = context;
}

} // End of namespace MediaStation
