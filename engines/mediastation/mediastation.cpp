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

#include "common/config-manager.h"
#include "engines/util.h"

#include "mediastation/mediastation.h"
#include "mediastation/debugchannels.h"
#include "mediastation/detection.h"
#include "mediastation/boot.h"
#include "mediastation/context.h"
#include "mediastation/asset.h"
#include "mediastation/assets/hotspot.h"
#include "mediastation/assets/movie.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

MediaStationEngine *g_engine;

MediaStationEngine::MediaStationEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc),
	_randomSource("MediaStation") {
	g_engine = this;

	_gameDataDir = Common::FSNode(ConfMan.getPath("path"));
	SearchMan.addDirectory(_gameDataDir, 0, 3);
	for (uint i = 0; MediaStation::directoryGlobs[i]; i++) {
		Common::String directoryGlob = directoryGlobs[i];
		SearchMan.addSubDirectoryMatching(_gameDataDir, directoryGlob, 0, 5);
	}
}

MediaStationEngine::~MediaStationEngine() {
	delete _screen;
	_screen = nullptr;

	delete _cursor;
	_cursor = nullptr;

	delete _boot;
	_boot = nullptr;

	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		delete it->_value;
	}
	_loadedContexts.clear();

	for (auto it = _variables.begin(); it != _variables.end(); ++it) {
		delete it->_value;
	}
	_variables.clear();
}

Asset *MediaStationEngine::getAssetById(uint assetId) {
	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		Asset *asset = it->_value->getAssetById(assetId);
		if (asset != nullptr) {
			return asset;
		}
	}
	return nullptr;
}

Asset *MediaStationEngine::getAssetByChunkReference(uint chunkReference) {
	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		Asset *asset = it->_value->getAssetByChunkReference(chunkReference);
		if (asset != nullptr) {
			return asset;
		}
	}
	return nullptr;
}

Function *MediaStationEngine::getFunctionById(uint functionId) {
	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		Function *function = it->_value->getFunctionById(functionId);
		if (function != nullptr) {
			return function;
		}
	}
	return nullptr;
}

uint32 MediaStationEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String MediaStationEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Platform MediaStationEngine::getPlatform() const {
	return _gameDescription->platform;
}

const char *MediaStationEngine::getAppName() const {
	return _gameDescription->filesDescriptions[0].fileName;
}

bool MediaStationEngine::isFirstGenerationEngine() {
	if (_boot == nullptr) {
		error("Attempted to get engine version before BOOT.STM was read");
	} else {
		return (_boot->_versionInfo == nullptr);
	}
}

Common::Error MediaStationEngine::run() {
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);
	_screen = new Graphics::Screen();
	// TODO: Determine if all titles blank the screen to 0xff.
	_screen->fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0xff);

	Common::Path bootStmFilepath = Common::Path("BOOT.STM");
	_boot = new Boot(bootStmFilepath);

	if (getPlatform() == Common::kPlatformWindows) {
		_cursor = new WindowsCursorManager(getAppName());
	} else if (getPlatform() == Common::kPlatformMacintosh) {
		_cursor = new MacCursorManager(getAppName());
	} else {
		error("MediaStationEngine::run(): Attempted to use unsupported platform %s", Common::getPlatformDescription(getPlatform()));
	}
	_cursor->showCursor();

    if (ConfMan.hasKey("entry_context")) {
		// For development purposes, we can choose to start at an arbitrary context
		// in this title. This might not work in all cases.
        uint entryContextId = ConfMan.get("entry_context").asUint64();
        warning("Starting at user-requested context %d", entryContextId);
		_requestedScreenBranchId = entryContextId;
	} else {
		_requestedScreenBranchId = _boot->_entryContextId;
	}
	doBranchToScreen();

	while (true) {
		processEvents();
		if (shouldQuit()) {
			break;
		}

		if (!_requestedContextReleaseId.empty()) {
			for (uint contextId : _requestedContextReleaseId) {
				releaseContext(contextId);
			}
			_requestedContextReleaseId.clear();
		}

		debugC(5, kDebugGraphics, "***** START SCREEN UPDATE ***");
		for (auto it = _assetsPlaying.begin(); it != _assetsPlaying.end();) {
			(*it)->process();

			// If we're changing screens, exit out now so we don't try to access
			// any assets that no longer exist.
			if (_requestedScreenBranchId != 0) {
				doBranchToScreen();
				_requestedScreenBranchId = 0;
				break;
			}

			if (_needsHotspotRefresh) {
				refreshActiveHotspot();
				_needsHotspotRefresh = false;
			}

			if (!(*it)->isActive()) {
				it = _assetsPlaying.erase(it);
			} else {
				++it;
			}
		}
		redraw();
		debugC(5, kDebugGraphics, "***** END SCREEN UPDATE ***");

		_screen->update();
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

void MediaStationEngine::processEvents() {
	while (g_system->getEventManager()->pollEvent(_event)) {
		debugC(9, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		debugC(9, kDebugEvents, "@@@@   Processing events");
		debugC(9, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

		switch (_event.type) {
		case Common::EVENT_QUIT: {
			// TODO: Do any necessary clean-up.
			return;
		}

		case Common::EVENT_MOUSEMOVE: {
			_mousePos = g_system->getEventManager()->getMousePos();
			_needsHotspotRefresh = true;
			break;
		}

		case Common::EVENT_KEYDOWN: {
			// Even though this is a keydown event, we need to look at the mouse position.
			Asset *hotspot = findAssetToAcceptMouseEvents();
			if (hotspot != nullptr) {
				debugC(1, kDebugEvents, "EVENT_KEYDOWN (%d): Sent to hotspot %d", _event.kbd.ascii, hotspot->getHeader()->_id);
				hotspot->runKeyDownEventHandlerIfExists(_event.kbd);
			}
			break;
		}

		case Common::EVENT_LBUTTONDOWN: {
			Asset *hotspot = findAssetToAcceptMouseEvents();
			if (hotspot != nullptr) {
				debugC(1, kDebugEvents, "EVENT_LBUTTONDOWN (%d, %d): Sent to hotspot %d", _mousePos.x, _mousePos.y, hotspot->getHeader()->_id);
				hotspot->runEventHandlerIfExists(kMouseDownEvent);
			}
			break;
		}

		case Common::EVENT_RBUTTONDOWN: {
			// We are using the right button as a quick exit since the Media
			// Station engine doesn't seem to use the right button itself.
			warning("EVENT_RBUTTONDOWN: Quitting for development purposes");
			quitGame();
			break;
		}

		default:
			break;
		}
	}
}

void MediaStationEngine::setCursor(uint id) {
	// The cursor ID is not a resource ID in the executable, but a numeric ID
	// that's a lookup into BOOT.STM, which gives actual name the name of the
	// resource in the executable.
	if (id != 0) {
		CursorDeclaration *cursorDeclaration = _boot->_cursorDeclarations.getValOrDefault(id);
		if (cursorDeclaration == nullptr) {
			error("MediaStationEngine::setCursor(): Cursor %d not declared", id);
		}
		_cursor->setCursor(*cursorDeclaration->_name);
	}
}

void MediaStationEngine::refreshActiveHotspot() {
	Asset *hotspot = findAssetToAcceptMouseEvents();
	if (hotspot != _currentHotspot) {
		if (_currentHotspot != nullptr) {
			_currentHotspot->runEventHandlerIfExists(kMouseExitedEvent);
			debugC(5, kDebugEvents, "refreshActiveHotspot(): (%d, %d): Exited hotspot %d", _mousePos.x, _mousePos.y, _currentHotspot->getHeader()->_id);
		}
		_currentHotspot = hotspot;
		if (hotspot != nullptr) {
			debugC(5, kDebugEvents, "refreshActiveHotspot(): (%d, %d): Entered hotspot %d", _mousePos.x, _mousePos.y, hotspot->getHeader()->_id);
			setCursor(hotspot->getHeader()->_cursorResourceId);
			hotspot->runEventHandlerIfExists(kMouseEnteredEvent);
		} else {
			// There is no hotspot, so set the default cursor for this screen instead.
			setCursor(_currentContext->_screenAsset->_cursorResourceId);
		}
	}

	if (hotspot != nullptr) {
		debugC(5, kDebugEvents, "refreshActiveHotspot(): (%d, %d): Sent to hotspot %d", _mousePos.x, _mousePos.y, hotspot->getHeader()->_id);
		hotspot->runEventHandlerIfExists(kMouseMovedEvent);
	}
}

void MediaStationEngine::redraw() {
	if (_dirtyRects.empty()) {
		return;
	}

	Common::sort(_assetsPlaying.begin(), _assetsPlaying.end(), [](Asset * a, Asset * b) {
		return a->zIndex() > b->zIndex();
	});

	for (Common::Rect dirtyRect : _dirtyRects) {
		for (Asset *asset : _assetsPlaying) {
			Common::Rect *bbox = asset->getBbox();
			if (bbox != nullptr) {
				if (dirtyRect.intersects(*bbox)) {
					asset->redraw(dirtyRect);
				}
			}
		}
	}

	_screen->update();
	_dirtyRects.clear();
}

Context *MediaStationEngine::loadContext(uint32 contextId) {
	if (_boot == nullptr) {
		error("Cannot load contexts before BOOT.STM is read");
	}

	debugC(5, kDebugLoading, "MediaStationEngine::loadContext(): Loading context %d", contextId);
	if (_loadedContexts.contains(contextId)) {
		warning("MediaStationEngine::loadContext(): Context %d already loaded, returning existing context", contextId);
		return _loadedContexts.getVal(contextId);
	}

	// Get the file ID.
	SubfileDeclaration *subfileDeclaration = _boot->_subfileDeclarations.getValOrDefault(contextId);
	if (subfileDeclaration == nullptr) {
		error("MediaStationEngine::loadContext(): Couldn't find subfile declaration with ID %d", contextId);
		return nullptr;
	}
	// There are other assets in a subfile too, so we need to make sure we're
	// referencing the screen asset, at the start of the file.
	if (subfileDeclaration->_startOffsetInFile != 16) {
		warning("MediaStationEngine::loadContext(): Requested ID wasn't for a context.");
		return nullptr;
	}
	uint32 fileId = subfileDeclaration->_fileId;

	// Get the filename.
	FileDeclaration *fileDeclaration = _boot->_fileDeclarations.getValOrDefault(fileId);
	if (fileDeclaration == nullptr) {
		warning("MediaStationEngine::loadContext(): Couldn't find file declaration with ID 0x%x", fileId);
		return nullptr;
	}
	Common::Path entryCxtFilepath(*fileDeclaration->_name);

	// Load any child contexts before we actually load this one. The child
	// contexts must be unloaded explicitly later.
	ContextDeclaration *contextDeclaration = _boot->_contextDeclarations.getValOrDefault(contextId);
	for (uint32 childContextId : contextDeclaration->_fileReferences) {
		// The root context is referred to by an ID of 0, regardless of what its
		// actual ID is. The root context is already always loaded.
		if (childContextId != 0) {
			debugC(5, kDebugLoading, "MediaStationEngine::loadContext(): Loading child context %d", childContextId);
			loadContext(childContextId);
		}
	}
	Context *context = new Context(entryCxtFilepath);

	// Some contexts have a built-in palette that becomes active when the
	// context is loaded, and some rely on scripts to set
	// the palette later.
	if (context->_palette != nullptr) {
		_screen->setPalette(*context->_palette);
	}

	context->registerActiveAssets();
	_loadedContexts.setVal(contextId, context);
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
		assert(args.size() >= 1);
		if (args.size() > 1) {
			// TODO: Figure out what the rest of the args can be.
			warning("MediaStationEngine::callMethod(): branchToScreen got more than one arg");
		}
		uint32 contextId = args[0].getAssetId();
		_requestedScreenBranchId = contextId;
		return Operand();
	}

	case kReleaseContextMethod: {
		assert(args.size() == 1);
		uint32 contextId = args[0].getAssetId();
		_requestedContextReleaseId.push_back(contextId);
		return Operand();
	}

	default:
		error("MediaStationEngine::callMethod(): Got unimplemented method ID %s (%d)", builtInMethodToStr(methodId), static_cast<uint>(methodId));
	}
}

void MediaStationEngine::doBranchToScreen() {
	if (_currentContext != nullptr) {
		EventHandler *exitEvent = _currentContext->_screenAsset->_eventHandlers.getValOrDefault(kExitEvent);
		if (exitEvent != nullptr) {
			debugC(5, kDebugScript, "Executing context exit event handler");
			exitEvent->execute(_currentContext->_screenAsset->_id);
		} else {
			debugC(5, kDebugScript, "No context exit event handler");
		}

		releaseContext(_currentContext->_screenAsset->_id);
	}

	Context *context = loadContext(_requestedScreenBranchId);
	_currentContext = context;
	_dirtyRects.push_back(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT));
	_currentHotspot = nullptr;

	if (context->_screenAsset != nullptr) {
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

	_requestedScreenBranchId = 0;
}

void MediaStationEngine::releaseContext(uint32 contextId) {
	debugC(5, kDebugScript, "MediaStationEngine::releaseContext(): Releasing context %d", contextId);
	Context *context = _loadedContexts.getValOrDefault(contextId);
	if (context == nullptr) {
		error("MediaStationEngine::releaseContext(): Attempted to unload context %d that is not currently loaded", contextId);
	}

	// Make sure nothing is still using this context.
	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		uint id = it->_key;
		ContextDeclaration *contextDeclaration = _boot->_contextDeclarations.getValOrDefault(id);
		for (uint32 childContextId : contextDeclaration->_fileReferences) {
			if (childContextId == contextId) {
				return;
			}
		}
	}

	// Unload any assets currently playing from this context. They should have
	// already been stopped by scripts, but this is a last check.
	for (auto it = _assetsPlaying.begin(); it != _assetsPlaying.end();) {
		uint assetId = (*it)->getHeader()->_id;
		Asset *asset = context->getAssetById(assetId);
		if (asset != nullptr) {
			it = _assetsPlaying.erase(it);
		} else {
			++it;
		}
	}

	delete context;
	_loadedContexts.erase(contextId);
}

Asset *MediaStationEngine::findAssetToAcceptMouseEvents() {
	Asset *intersectingAsset = nullptr;
	// The z-indices seem to be reversed, so the highest z-index number is
	// actually the lowest asset.
	int lowestZIndex = INT_MAX;

	for (Asset *asset : _assetsPlaying) {
		if (asset->type() == kAssetTypeHotspot) {
			debugC(5, kDebugGraphics, "findAssetToAcceptMouseEvents(): Hotspot %d (z-index %d)", asset->getHeader()->_id, asset->zIndex());
			if (asset->isActive() && static_cast<Hotspot *>(asset)->isInside(_mousePos)) {
				if (asset->zIndex() < lowestZIndex) {
					lowestZIndex = asset->zIndex();
					intersectingAsset = asset;
				}
			}
		}
	}
	return intersectingAsset;
}

Operand MediaStationEngine::callBuiltInFunction(BuiltInFunction function, Common::Array<Operand> &args) {
	switch (function) {
	case kEffectTransitionFunction:
	case kEffectTransitionOnSyncFunction: {
		// TODO: effectTransitionOnSync should be split out into its own function.
		effectTransition(args);
		return Operand();
	}

	case kDrawingFunction: {
		// Not entirely sure what this function does, but it seems like a way to
		// call into some drawing functions built into the IBM/Crayola executable.
		warning("MediaStationEngine::callBuiltInFunction(): Built-in drawing function not implemented");
		return Operand();
	}

	case kUnk1Function: {
		warning("MediaStationEngine::callBuiltInFunction(): Function 10 not implemented");
		Operand returnValue = Operand(kOperandTypeLiteral1);
		returnValue.putInteger(1);
		return returnValue;
	}

	default:
		error("MediaStationEngine::callBuiltInFunction(): Got unknown built-in function %s (%d)", builtInFunctionToStr(function), static_cast<uint>(function));
	}
}

} // End of namespace MediaStation
